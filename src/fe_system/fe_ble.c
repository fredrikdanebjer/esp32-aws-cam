/*
* @file fe_ble.c
*
* The MIT License (MIT)
*
* Copyright (c) 2021 Fredrik Danebjer
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <string.h>

#include "kvs_service.h"
#include "system_controller.h"

#include "esp_log.h"
#include "iot_ble_config.h"
#include "iot_ble.h"

#define FE_BLE_UPTIME                           (1000 * 60 * 3) // 3 minutes

#define WIFI_CREDENTIAL_SERVICE_UUID            { 0x42, 0xe8, 0x4d, 0x84, 0x32, 0x1b, 0x46, 0x70, 0x93, 0xca, 0x57, 0x7f, 0xba, 0x71, 0x2e, 0xed }
#define WIFI_CREDENTIAL_SSID_CHARACTERISTIC     (0x10b5)  // Full UUID: { 0x38, 0x0a, 0x1c, 0xc3, 0xe4, 0x91, 0x40, 0xc6, 0x8c, 0xf2, 0x0d, 0x29, 0xb5, 0x10, 0x95, 0x39 }
#define WIFI_CREDENTIAL_PASSWORD_CHARACTERISTIC (0x13db)  // Full UUID: { 0xaf, 0x2d, 0xd3, 0x0a, 0x99, 0xe2, 0x4f, 0x0d, 0xa0, 0xed, 0x89, 0x50, 0xdb, 0x13, 0xd5, 0xbc }

#define _UUID128( value )        \
    {                            \
        .uu.uu128 = value,       \
        .ucType = eBTuuidType128 \
    }

#define _UUID16( value )        \
    {                           \
        .uu.uu16 = value,       \
        .ucType = eBTuuidType16 \
    }

typedef enum
{
    _ATTR_SERVICE,
    _ATTR_CHAR_WIFI_SSID,
    _ATTR_CHAR_WIFI_PASSWORD,
    _ATTR_NUMBER
} _attr_t;

static void _wifiSSIDCallback(IotBleAttributeEvent_t * pEventParam);
static void _wifiPasswordCallback(IotBleAttributeEvent_t * pEventParam);

static BTUuid_t _advUUID =
{
    .uu.uu128 = WIFI_CREDENTIAL_SERVICE_UUID,
    .ucType   = eBTuuidType128
};

static uint16_t _handlesBuffer[_ATTR_NUMBER];

static const BTAttribute_t _pAttributeTable[] =
{
    {
        .xServiceUUID = _UUID128(WIFI_CREDENTIAL_SERVICE_UUID)
    },
    {
        .xAttributeType = eBTDbCharacteristic,
        .xCharacteristic =
        {
            .xUuid        = _UUID16(WIFI_CREDENTIAL_SSID_CHARACTERISTIC),
            .xPermissions = (IOT_BLE_CHAR_READ_PERM | IOT_BLE_CHAR_WRITE_PERM),
            .xProperties = (eBTPropRead | eBTPropWrite)
        }
    },
    {
        .xAttributeType = eBTDbCharacteristic,
        .xCharacteristic =
        {
            .xUuid        = _UUID16(WIFI_CREDENTIAL_PASSWORD_CHARACTERISTIC),
            .xPermissions = IOT_BLE_CHAR_WRITE_PERM,
            .xProperties = eBTPropWrite
        }
    },
};

static const BTService_t _wifiCredentialsService =
{
  .xNumberOfAttributes = _ATTR_NUMBER,
  .ucInstId            = 0,
  .xType               = eBTServiceTypePrimary,
  .pusHandlesBuffer    = _handlesBuffer,
  .pxBLEAttributes     = ( BTAttribute_t * ) _pAttributeTable
};

static const IotBleAttributeEventCallback_t pxCallBackArray[ _ATTR_NUMBER ] =
{
  NULL,
  _wifiSSIDCallback,
  _wifiPasswordCallback,
};

static kvs_entry_t kvs_ssid = {0};
static kvs_entry_t kvs_password = {0};
static uint8_t _initialized = 0;

static void _wifiSSIDCallback(IotBleAttributeEvent_t * pEventParam)
{
  IotBleWriteEventParams_t * write_param;
  IotBleAttributeData_t attr_data = {0};
  IotBleEventResponse_t resp;

  resp.pAttrData = &attr_data;
  resp.rspErrorStatus = eBTRspErrorNone;
  resp.eventStatus = eBTStatusFail;
  resp.attrDataOffset = 0;

  if((pEventParam->xEventType == eBLEWrite) || (pEventParam->xEventType == eBLEWriteNoResponse))
  {
    write_param = pEventParam->pParamWrite;
    attr_data.handle = write_param->attrHandle;

    if(write_param->length < KVS_SERVICE_MAXIMUM_VALUE_SIZE && pEventParam->xEventType == eBLEWrite)
    {
      memset(&kvs_ssid, '\0', sizeof(kvs_ssid));
      kvs_ssid.key = kvs_entry_wifi_ssid;
      kvs_ssid.value_len = write_param->length;
      memcpy(kvs_ssid.value, write_param->pValue, write_param->length);
      SC_send_cmd(sc_service_kvs, KVS_SERVICE_CMD_PUT_KEY_VALUE, &kvs_ssid);
      ESP_LOGI("FE BLE", "Writing wifi ssid %s, of length %u\n", kvs_ssid.value, write_param->length);

      resp.eventStatus = eBTStatusSuccess;
      attr_data.pData = write_param->pValue;
      attr_data.size = write_param->length;
      resp.attrDataOffset = write_param->offset;

      IotBle_SendResponse( &resp, write_param->connId, write_param->transId );
    }
  }
  else if(pEventParam->xEventType == eBLERead)
  {
    memset(&kvs_ssid, '\0', sizeof(kvs_ssid));
    kvs_ssid.key = kvs_entry_wifi_ssid;
    SC_send_cmd(sc_service_kvs, KVS_SERVICE_CMD_GET_KEY_VALUE, &kvs_ssid);
    ESP_LOGI("FE BLE", "Reading wifi ssid %s, of length %u\n", kvs_ssid.value, kvs_ssid.value_len);

    attr_data.handle = pEventParam->pParamRead->attrHandle;
    attr_data.pData = (uint8_t*) kvs_ssid.value;
    attr_data.size = kvs_ssid.value_len;
    resp.attrDataOffset = 0;
    resp.eventStatus = eBTStatusSuccess;

    IotBle_SendResponse(&resp, pEventParam->pParamRead->connId, pEventParam->pParamRead->transId);
  }
}

static void _wifiPasswordCallback(IotBleAttributeEvent_t * pEventParam)
{
  IotBleWriteEventParams_t * write_param;
  IotBleAttributeData_t attr_data = { 0 };
  IotBleEventResponse_t resp;

  resp.pAttrData = &attr_data;
  resp.rspErrorStatus = eBTRspErrorNone;
  resp.eventStatus = eBTStatusFail;
  resp.attrDataOffset = 0;

  if((pEventParam->xEventType == eBLEWrite) || (pEventParam->xEventType == eBLEWriteNoResponse))
  {
    write_param = pEventParam->pParamWrite;
    attr_data.handle = write_param->attrHandle;

    if(pEventParam->xEventType == eBLEWrite)
    {
      memset(&kvs_password, '\0', sizeof(kvs_password));
      kvs_password.key = kvs_entry_wifi_password;
      kvs_password.value_len = write_param->length;
      memcpy(kvs_password.value, write_param->pValue, write_param->length);
      SC_send_cmd(sc_service_kvs, KVS_SERVICE_CMD_PUT_KEY_VALUE, &kvs_password);
      ESP_LOGI("FE BLE", "Writing wifi password %s, of length %u\n", kvs_password.value, write_param->length);

      resp.eventStatus = eBTStatusSuccess;
      attr_data.pData = write_param->pValue;
      attr_data.size = write_param->length;
      resp.attrDataOffset = write_param->offset;

      IotBle_SendResponse(&resp, write_param->connId, write_param->transId);
    }
  }
}

void IotBle_SetCustomAdvCb( IotBleAdvertisementParams_t * pAdvParams,  IotBleAdvertisementParams_t * pScanParams)
{
  ESP_LOGI("FE BLE", "Setting Advertisment Data\n");

  memset(pAdvParams, 0, sizeof(IotBleAdvertisementParams_t));
  memset(pScanParams, 0, sizeof(IotBleAdvertisementParams_t));

  pAdvParams->pUUID1 = &_advUUID;
  pAdvParams->name.xType = BTGattAdvNameNone;

  pScanParams->setScanRsp = true;
  pScanParams->name.xType = BTGattAdvNameComplete;
}

void IotBle_AddCustomServicesCb( void )
{
  ESP_LOGI("FE BLE", "Creating BLE Services\n");
  IotBle_CreateService((BTService_t*) &_wifiCredentialsService, (IotBleAttributeEventCallback_t*) pxCallBackArray);
}

int FE_BLE_init()
{
  if (_initialized)
  {
    return EXIT_SUCCESS;
  }

  BTStatus_t status;

  status = IotBle_Init();
  if (status != eBTStatusSuccess)
  {
    return EXIT_FAILURE;
  }

  status = IotBle_On();
  if (status != eBTStatusSuccess)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


int FE_BLE_deinit()
{
  IotBle_Off();
  IotBle_DeleteService((BTService_t*) &_wifiCredentialsService);

  _initialized = 0;

  return EXIT_SUCCESS;
}

void FE_BLE_shutdown_runner()
{
  vTaskDelay(FE_BLE_UPTIME / portTICK_PERIOD_MS);

  FE_BLE_deinit();
}
