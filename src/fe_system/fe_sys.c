/*
* @file fe_system.c
*
* The MIT License (MIT)
*
* Copyright (c) 2020 Fredrik Danebjer
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

#include "fe_sys.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#if (AFR_ESP_LWIP)
#include "tcpip_adapter.h"
#else
#include "FreeRTOS_IP.h"
#endif

#include "FreeRTOSConfig.h"

#include "nvs_flash.h"

#include "iot_system_init.h"
#include "iot_logging_task.h"

#include "fsu_eye_wifi_credentials.h"

// Logging Task Defines
#define FE_SYS_LOGGING_MESSAGE_QUEUE_LENGTH    (32U)
#define FE_SYS_LOGGING_TASK_STACK_SIZE         (configMINIMAL_STACK_SIZE * 4)

#if (!AFR_ESP_LWIP)
uint8_t _mac_addr[6] =
{
    FSU_EYE_MAC_ADDR0,
    FSU_EYE_MAC_ADDR1,
    FSU_EYE_MAC_ADDR2,
    FSU_EYE_MAC_ADDR3,
    FSU_EYE_MAC_ADDR4,
    FSU_EYE_MAC_ADDR5
};

static const uint8_t _ip_addr[4] =
{
    FSU_EYE_IP_ADDR0,
    FSU_EYE_IP_ADDR1,
    FSU_EYE_IP_ADDR2,
    FSU_EYE_IP_ADDR3
};
static const uint8_t _net_mask[4] =
{
    FSU_EYE_NET_MASK0,
    FSU_EYE_NET_MASK1,
    FSU_EYE_NET_MASK2,
    FSU_EYE_NET_MASK3
};
static const uint8_t _gateway_addr[4] =
{
    FSU_EYE_GATEWAY_ADDR0,
    FSU_EYE_GATEWAY_ADDR1,
    FSU_EYE_GATEWAY_ADDR2,
    FSU_EYE_GATEWAY_ADDR3
};
static const uint8_t _dns_server_addr[4] =
{
    FSU_EYE_DNS_SERVER_ADDR0,
    FSU_EYE_DNS_SERVER_ADDR1,
    FSU_EYE_DNS_SERVER_ADDR2,
    FSU_EYE_DNS_SERVER_ADDR3
};
#endif

static void FE_SYS_NVS_init()
{
  esp_err_t ret = nvs_flash_init();

  if(ESP_ERR_NVS_NO_FREE_PAGES != ret
   || ESP_ERR_NVS_NEW_VERSION_FOUND != ret)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

int FE_SYS_init()
{
  FE_SYS_NVS_init();

  // Create logging task
  xLoggingTaskInitialize(FE_SYS_LOGGING_TASK_STACK_SIZE,
                         tskIDLE_PRIORITY + 5,
                         FE_SYS_LOGGING_MESSAGE_QUEUE_LENGTH);

#if (AFR_ESP_LWIP)
  // Initialize the ESP LWIP TCP/IP Stack
  tcpip_adapter_init();
#else
  // Initialize the FreeRTOS IP Stack
  FreeRTOS_IPInit(_ip_addr,
                  _net_mask,
                  _gateway_addr,
                  _dns_server_addr,
                  _mac_addr);
#endif

  // Initialize rest of FreeRTOS 
  if (SYSTEM_Init() != pdPASS)
  {
    return EXIT_FAILURE;
  }

  // Start the scheduler should be done here, but is taken care of in the
  // ESP IDF.
  // Without ESP IDF: vTaskStartScheduler();

  return EXIT_SUCCESS;
}