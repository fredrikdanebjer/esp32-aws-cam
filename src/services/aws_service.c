/*
* @file aws_service.c
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

#include "aws_service.h"

#include <string.h>
#include "types/iot_mqtt_types.h"

#include "platform/iot_network_freertos.h"
#include "platform/iot_network.h"
#include "iot_mqtt.h"
#include "semphr.h"

#include "fsu_eye_aws_credentials.h"
#include "private/iot_default_root_certificates.h"

#include "aws_dev_mode_key_provisioning.h"
#include "esp_log.h"

#include "jsmn.h"
#include "stdbool.h"

#include "platform/iot_clock.h"
#include "aws_iot_ota_agent.h"

#include "iot_appversion32.h"

#define FSU_EYE_NETWORK_INTERFACE     IOT_NETWORK_INTERFACE_AFR
#define FSU_EYE_AWS_IOT_ALPN_MQTT     "x-amzn-mqtt-ca"

#define KEEP_ALIVE_SECONDS            (60U)
#define MQTT_TIMEOUT_MS               (5000U)

#define PUBLISH_RETRY_LIMIT           (1U)
#define PUBLISH_RETRY_MS              (1000U)

#define TOPIC_FILTER_COUNT            1

#define FSU_EYE_TOPIC_ROOT            "fsu/eye/" FSU_EYE_AWS_IOT_THING_NAME

#define FSU_EYE_SUBSCRIBE_COMMAND     (FSU_EYE_TOPIC_ROOT "/r/command")
/**
 * /r/command messages are expected to have this form:
 * {
 *  "id":"thing_name",      // The device thing name
 *  "service_id":"service", // ID of the service to perform
 *  "command_id":"command"  // Command ID to perform on the service
 * }
*/
#define COMMAND_MESSAGE_TOKENS        (7U)
#define COMMAND_MESSAGE_ID_FIELD      "id"
#define COMMAND_MESSAGE_SERVICE_FIELD "service_id"
#define COMMAND_MESSAGE_COMMAND_FIELD "command_id"

#define FSU_EYE_TOPIC_LWT             (FSU_EYE_TOPIC_ROOT "/lwt")
#define FSU_EYE_TOPIC_INFO            (FSU_EYE_TOPIC_ROOT "/info")
#define FSU_EYE_TOPIC_IMAGE           (FSU_EYE_TOPIC_ROOT "/image")

#define LWT_MESSAGE                   ("{"\
                                          "\"id\":\"" FSU_EYE_AWS_IOT_THING_NAME "\"" \
                                          "\"message\":\"LWT\"" \
                                       "}")

#define EYE_PUBLISH_MAX_LEN           (0x4C00U)
#define EYE_SUBSCRIBE_MAX_TOKENS      (0x10U)

#define EYE_MSG_ID_FORMAT             "%s"
#define EYE_MSG_INFO_FORMAT           "%s"

#define EYE_INFO_MSG                  ("{"\
                                         "\"id\":\"" EYE_MSG_ID_FORMAT "\","\
                                         "\"msg\":\"" EYE_MSG_INFO_FORMAT "\""\
                                       "}")

// App version struct, used by OTA Agent to decide if new firmware is an upgrade
const AppVersion32_t xAppFirmwareVersion =
{
    .u.x.ucMajor = APP_VERSION_MAJOR,
    .u.x.ucMinor = APP_VERSION_MINOR,
    .u.x.usBuild = APP_VERSION_BUILD,
};

static IotMqttConnection_t _mqtt_connection;
static uint8_t _initialized = 0;
static uint8_t _connected = 0;
static char _payload[EYE_PUBLISH_MAX_LEN];
static SemaphoreHandle_t _payload_mutex;
static uint8_t _publish_complete;

IotNetworkServerInfo_t aws_server_info = {
  .pHostName = FSU_EYE_AWS_MQTT_BROKER_ENDPOINT,
  .port = FSU_EYE_AWS_MQTT_BROKER_PORT  
};

static const char * _ota_state_dict[eOTA_AgentState_All] =
{
    "Init",
    "Ready",
    "RequestingJob",
    "WaitingForJob",
    "CreatingFile",
    "RequestingFileBlock",
    "WaitingForFileBlock",
    "ClosingFile",
    "Suspended",
    "ShuttingDown",
    "Stopped"
};

static IotNetworkCredentials_t network_credentials = {
  .pAlpnProtos = FSU_EYE_AWS_IOT_ALPN_MQTT,
  .maxFragmentLength = 0,
  .disableSni = false,
  .pRootCa =  FSU_EYE_AWS_ROOT_CA,
  .rootCaSize = sizeof(FSU_EYE_AWS_ROOT_CA),
  .pClientCert = FSU_EYE_AWS_CLIENT_CERT,
  .clientCertSize = sizeof(FSU_EYE_AWS_CLIENT_CERT),
  .pPrivateKey = FSU_EYE_AWS_PRIVATE_KEY,
  .privateKeySize = sizeof(FSU_EYE_AWS_PRIVATE_KEY)
};

// JSON Helper function, to check if a token is a sought after string
static int _jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
  if (JSMN_STRING == tok->type
    && (int)strlen(s) == (tok->end - tok->start)
    && strncmp(json + tok->start, s, tok->end - tok->start) == 0)
  {
    return true;
  }
  return false;
}

// Adds the Certificate and Private Key to the internal PKCS11 and mbedtls
// utilized lists. The credentials seems to be stored in NVS.
static int AWS_SERVICE_PKCS11_provision_key(void)
{
  ProvisioningParams_t provision_params;

  memset(&provision_params, 0, sizeof(provision_params));

  provision_params.pucClientPrivateKey = (uint8_t*) FSU_EYE_AWS_PRIVATE_KEY;
  provision_params.pucClientCertificate = (uint8_t*) FSU_EYE_AWS_CLIENT_CERT;
  provision_params.pucJITPCertificate = NULL;
  provision_params.ulClientPrivateKeyLength = sizeof(FSU_EYE_AWS_PRIVATE_KEY);
  provision_params.ulClientCertificateLength = sizeof(FSU_EYE_AWS_CLIENT_CERT);
  provision_params.ulJITPCertificateLength = 0;

  ESP_LOGI("AWS_SERVICE", "PKCS11 PrKey strlen & size: %d - %d.\n", sizeof(FSU_EYE_AWS_PRIVATE_KEY), strlen(FSU_EYE_AWS_PRIVATE_KEY));

  if (vAlternateKeyProvisioning(&provision_params) != CKR_OK)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int AWS_SERVICE_init()
{
  if (_initialized)
  {
    return EXIT_SUCCESS;
  }

  if (IotMqtt_Init() != IOT_MQTT_SUCCESS )
  {
    return EXIT_FAILURE;
  }

  _mqtt_connection = IOT_MQTT_CONNECTION_INITIALIZER;
  _connected = 0;
  _initialized = 1;
  memset(_payload, '\0', EYE_PUBLISH_MAX_LEN);
  _payload_mutex = xSemaphoreCreateMutex();

  // For some reason the MQTT Connect method does not utilize the private key,
  // it is instead always fetched from the internal PKCS11 provisioned list
  AWS_SERVICE_PKCS11_provision_key();

  return EXIT_SUCCESS;
}

static int AWS_SERVICE_deinit()
{
  IotMqtt_Cleanup();
  vSemaphoreDelete(_payload_mutex);
  _initialized = 0;

  return EXIT_SUCCESS;
}

static void _publish_complete_callback(void *param1,
                                       IotMqttCallbackParam_t *const param)
{
  ESP_LOGI("AWS_SERVICE", "MQTT publish complete!\n");
  _publish_complete = 1;
}

static void _mqtt_subscription_callback(void *param1,
                                       IotMqttCallbackParam_t *const param)
{
  ESP_LOGI("AWS_SERVICE", "MQTT subscribe received: %.*s\n", param->u.message.info.payloadLength,
                                                             (const char*) param->u.message.info.pPayload);

  jsmn_parser parser;
  jsmntok_t tokens[EYE_SUBSCRIBE_MAX_TOKENS];
  int status = 0;
  uint32_t service = 0;
  uint32_t command = 0;
  const char *payload = param->u.message.info.pPayload;


  jsmn_init(&parser);
  if ((status = jsmn_parse(&parser, payload, param->u.message.info.payloadLength, tokens, EYE_SUBSCRIBE_MAX_TOKENS)) < 0)
  {
    ESP_LOGW("AWS_SERVICE", "MQTT subscribe returned failed during parse with %d.", status);
  }

  if (strncmp(FSU_EYE_SUBSCRIBE_COMMAND, param->u.message.info.pTopicName, param->u.message.info.topicNameLength) == 0)
  {
    if (COMMAND_MESSAGE_TOKENS != status)
    {
      ESP_LOGW("AWS_SERVICE", "MQTT subscribe invalid command message, found %d tokens.", status);
      return;
    }
    for (int i = 1; i < COMMAND_MESSAGE_TOKENS; ++i)
    {
      if (_jsoneq(payload, &tokens[i], COMMAND_MESSAGE_ID_FIELD))
      {
        if (!_jsoneq(payload, &tokens[i + 1], FSU_EYE_AWS_IOT_THING_NAME))
        {
          ESP_LOGW("AWS_SERVICE", "MQTT subscribe invalid ID received on commange message!");
          return;
        }
      }
      else if (_jsoneq(payload, &tokens[i], COMMAND_MESSAGE_SERVICE_FIELD))
      {
        service = strtoul(&payload[tokens[i + 1].start], NULL, 10);
        if (0 == service)
        {
          ESP_LOGW("AWS_SERVICE", "MQTT subscribe invalid service received on commange message.");
          return;
        }
      }
      else if (_jsoneq(payload, &tokens[i], COMMAND_MESSAGE_COMMAND_FIELD))
      {
        command = strtoul(&payload[tokens[i + 1].start], NULL, 10);
        if (0 == command)
        {
          ESP_LOGW("AWS_SERVICE", "MQTT subscribe invalid command received on commange message.");
          return;
        }
      }
      else
      {
        ESP_LOGW("AWS_SERVICE", "MQTT subscribe invalid command message token at index %d.", i);
        return;
      }
      ++i;
    }
    ESP_LOGW("AWS_SERVICE", "MQTT subscribe issuing command %d to service %d.", command, service);
    SC_send_cmd((uint8_t)service, (uint8_t)command, NULL);
  }
}

static void _mqtt_disconnected_callback(void *param1,
                                       IotMqttCallbackParam_t *const param)
{
  ESP_LOGI("AWS_SERVICE", "MQTT disconnection!\n");
  _connected = 0;
}

static int AWS_SERVICE_subscribe(IotMqttConnection_t mqtt_connection,
                                 const char **topic_filters)
{
  int status = EXIT_SUCCESS;
  IotMqttError_t subscription_status = IOT_MQTT_STATUS_PENDING;
  IotMqttSubscription_t subscriptions[TOPIC_FILTER_COUNT] = {IOT_MQTT_SUBSCRIPTION_INITIALIZER};

  // Set the members of the subscription list
  for(int i = 0; i < TOPIC_FILTER_COUNT; ++i)
  {
    subscriptions[i].qos = IOT_MQTT_QOS_1;
    subscriptions[i].pTopicFilter = topic_filters[i];
    subscriptions[i].topicFilterLength = strlen(topic_filters[i]);
    subscriptions[i].callback.pCallbackContext = NULL;
    subscriptions[i].callback.function = _mqtt_subscription_callback;
  }

  subscription_status = IotMqtt_TimedSubscribe(mqtt_connection,
                                              subscriptions,
                                              TOPIC_FILTER_COUNT,
                                              0,
                                              MQTT_TIMEOUT_MS);

  // Verify subscription statuses
  switch(subscription_status)
  {
    case IOT_MQTT_SUCCESS:
      ESP_LOGI("AWS_SERVICE", "Subscription succeeded!");
      break;

    case IOT_MQTT_SERVER_REFUSED:

      // Check which subscriptions were rejected
      for(int i = 0; i < TOPIC_FILTER_COUNT; ++i)
      {
        if(IotMqtt_IsSubscribed(mqtt_connection,
                                subscriptions[i].pTopicFilter,
                                subscriptions[i].topicFilterLength,
                                NULL) == false)
        {
          ESP_LOGE("AWS_SERVICE", "Topic filter %.*s was rejected.",
                        subscriptions[i].topicFilterLength,
                        subscriptions[i].pTopicFilter);
        }
      }
      status = EXIT_FAILURE;
      break;

    default:
      status = EXIT_FAILURE;
      break;
  }

  return status;
}

static int AWS_SERVICE_mqtt_publish(const void *msg, size_t len, const char *topic, size_t topic_len)
{
  if (!_initialized || !_connected)
  {
    return EXIT_FAILURE;
  }

  IotMqttError_t status = IOT_MQTT_STATUS_PENDING;
  IotMqttPublishInfo_t publish_info = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
  IotMqttCallbackInfo_t publish_complete = IOT_MQTT_CALLBACK_INFO_INITIALIZER;
  _publish_complete = 0;

  publish_complete.function = _publish_complete_callback;
  publish_complete.pCallbackContext = NULL;

  publish_info.qos = IOT_MQTT_QOS_1;
  publish_info.pTopicName = topic;
  publish_info.topicNameLength = topic_len;
  publish_info.retryMs = PUBLISH_RETRY_MS;
  publish_info.retryLimit = PUBLISH_RETRY_LIMIT;
  publish_info.pPayload = msg;
  publish_info.payloadLength = len;

  status = IotMqtt_Publish(_mqtt_connection,
                           &publish_info,
                           0,
                           &publish_complete,
                           NULL);

  if(IOT_MQTT_STATUS_PENDING != status)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int AWS_SERVICE_publish_image(image_info_t *image_info)
{
  if (!_initialized || !_connected)
  {
    return EXIT_FAILURE;
  }

  if (NULL == image_info)
  {
    ESP_LOGW("AWS_SERVICE", "Provided image was NULL.\n");
    return EXIT_FAILURE;
  }

  if(xSemaphoreTake(_payload_mutex, (TickType_t) 10U) == pdTRUE)
  {
    AWS_SERVICE_mqtt_publish(image_info->buf, image_info->len, FSU_EYE_TOPIC_IMAGE, strlen(FSU_EYE_TOPIC_IMAGE));
    xSemaphoreGive(_payload_mutex);

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

static int AWS_SERVICE_publish_info(message_info_t *info)
{
  if (!_initialized || !_connected)
  {
    return EXIT_FAILURE;
  }

  if (NULL == info)
  {
    ESP_LOGW("AWS_SERVICE", "Could not create publish info message, NULL input.\n");
    return EXIT_FAILURE;
  }

  if ((EYE_PUBLISH_MAX_LEN - (sizeof(EYE_INFO_MSG) - strlen(EYE_MSG_ID_FORMAT) - strlen(EYE_MSG_INFO_FORMAT))) < info->msg_len)
  {
    ESP_LOGE("AWS_SERVICE", "Could not create publish info message, input too large.\n");
    return EXIT_FAILURE;
  }

  size_t len = 0;

  if(xSemaphoreTake(_payload_mutex, (TickType_t) 10U) == pdTRUE)
  {
    memset(_payload, '\0', EYE_PUBLISH_MAX_LEN);

    len = snprintf(_payload, EYE_PUBLISH_MAX_LEN, EYE_INFO_MSG, FSU_EYE_AWS_IOT_THING_NAME, info->msg);
    if (len > 0)
    {
      AWS_SERVICE_mqtt_publish(_payload, len, FSU_EYE_TOPIC_INFO, strlen(FSU_EYE_TOPIC_INFO));
      xSemaphoreGive(_payload_mutex);

      return EXIT_SUCCESS;
    }

    xSemaphoreGive(_payload_mutex);
  }

  return EXIT_FAILURE;
}

static int AWS_SERVICE_mqtt_connect()
{
  if (_connected)
  {
    return EXIT_SUCCESS;
  }

  IotMqttError_t connect_status = IOT_MQTT_STATUS_PENDING;
  IotMqttNetworkInfo_t network_info = IOT_MQTT_NETWORK_INFO_INITIALIZER;
  IotMqttConnectInfo_t connect_info = IOT_MQTT_CONNECT_INFO_INITIALIZER;
  IotMqttPublishInfo_t will_info = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
  IotMqttCallbackInfo_t callback_info = {
    .pCallbackContext = NULL,
    .function = _mqtt_disconnected_callback
  };
  _mqtt_connection = IOT_MQTT_CONNECTION_INITIALIZER;

  // Set the AWS Connection Credentials
  network_info.createNetworkConnection = true;
  network_info.u.setup.pNetworkServerInfo = &aws_server_info;
  network_info.u.setup.pNetworkCredentialInfo = &network_credentials;
  network_info.pNetworkInterface = FSU_EYE_NETWORK_INTERFACE;
  network_info.disconnectCallback = callback_info;

  // Set the members of the Last Will and Testament (LWT) message info
  will_info.pTopicName = FSU_EYE_TOPIC_LWT;
  will_info.topicNameLength = sizeof(FSU_EYE_TOPIC_LWT) - 1;
  will_info.pPayload = LWT_MESSAGE;
  will_info.payloadLength = sizeof(LWT_MESSAGE) - 1;

  // Set up connection information
  connect_info.awsIotMqttMode = true;
  connect_info.cleanSession = true;
  connect_info.keepAliveSeconds = KEEP_ALIVE_SECONDS;
  connect_info.pWillInfo = &will_info;
  connect_info.pClientIdentifier = FSU_EYE_AWS_IOT_THING_NAME;
  connect_info.clientIdentifierLength = strlen(FSU_EYE_AWS_IOT_THING_NAME);

  // Open MQTT connection
  ESP_LOGI("AWS_SERVICE", "Establishing MQTT Connection!\n");
  connect_status = IotMqtt_Connect(&network_info,
                                   &connect_info,
                                   MQTT_TIMEOUT_MS,
                                   &_mqtt_connection);

  if (IOT_MQTT_SUCCESS != connect_status)
  {
    ESP_LOGI("AWS_SERVICE", "MQTT Connection Failed!\n");
    return EXIT_FAILURE;
  }

  _connected = 1;
  ESP_LOGI("AWS_SERVICE", "MQTT Connected!\n");

  return EXIT_SUCCESS;
}

static int AWS_SERVICE_mqtt_connect_subscribe()
{
  if (_connected)
  {
    return EXIT_SUCCESS;
  }

  if (AWS_SERVICE_mqtt_connect() != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }

  const char *subscription_topics[TOPIC_FILTER_COUNT] =
  {
    FSU_EYE_SUBSCRIBE_COMMAND
  };

  if (AWS_SERVICE_subscribe(_mqtt_connection, subscription_topics) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

static int AWS_SERVICE_recv_msg(uint8_t cmd, void* arg)
{
  switch (cmd)
  {
    case(AWS_SERVICE_CMD_MQTT_CONNECT_SUBSCRIBE):
      return AWS_SERVICE_mqtt_connect_subscribe();

    case (AWS_SERVICE_CMD_MQTT_PUBLISH_MESSAGE):
      return AWS_SERVICE_publish_info((message_info_t*)arg);

    case (AWS_SERVICE_CMD_MQTT_PUBLISH_IMAGE):
      return AWS_SERVICE_publish_image((image_info_t*)arg);
  }
  return EXIT_FAILURE;
}

static void OTA_complete_callback(OTA_JobEvent_t event)
{
  ESP_LOGI("AWS SERVICE OTA", "Complete callback with %u\n", event);

  if (event == eOTA_JobEvent_Activate)
  {
    ESP_LOGI("AWS SERVICE OTA", "Received eOTA_JobEvent_Activate callback from OTA Agent.\n");

    // OTA job is completed, so we disconnect here before rebooting
    if(_mqtt_connection != NULL)
    {
      IotMqtt_Disconnect(_mqtt_connection, false);
    }

    // We activate the new firmware, which will cause a reset
    OTA_ActivateNewImage();

    while (1)
    {
      // Nothing to do - should not get here, but we wait for reboot ... 
    }
  }
  else if (event == eOTA_JobEvent_Fail)
  {
    ESP_LOGI("AWS SERVICE OTA", "Received eOTA_JobEvent_Fail callback from OTA Agent.\n");
  }
  else if (event == eOTA_JobEvent_StartTest)
  {
    ESP_LOGI("AWS SERVICE OTA", "Attempting to set image to Verified\n" );

    if (OTA_SetImageState(eOTA_ImageState_Accepted) != kOTA_Err_None)
    {
      ESP_LOGI("AWS SERVICE OTA", "Error! Failed to set image state as accepted.\n" );
    }
  }
}

void AWS_SERVICE_OTA_runner()
{
  OTA_State_t ota_state;
  static OTA_ConnectionContext_t ota_connection_context;

  ESP_LOGI("AWS SERVICE OTA Agent", "Init, running version %u.%u.%u\n", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD);

  while (1)
  {
    while (!_connected)
    {
      // Wait for connection to come up, yield meanwhile
      IotClock_SleepMs(1000);
    }

    // Copy connection context to OTA struct
    ota_connection_context.pxNetworkInterface = ( void * ) FSU_EYE_NETWORK_INTERFACE;
    ota_connection_context.pvNetworkCredentials = &network_credentials;
    ota_connection_context.pvControlClient = _mqtt_connection;

    // Check if OTA Agent was suspended, if so resume
    if((ota_state = OTA_GetAgentState()) == eOTA_AgentState_Suspended)
    {
      OTA_Resume(&ota_connection_context);
    }

    // Initialize the OTA Agent
    OTA_AgentInit((void*) (&ota_connection_context),
                  (const uint8_t*) FSU_EYE_AWS_IOT_THING_NAME,
                  OTA_complete_callback,
                  (TickType_t) ~0);

    while(((ota_state = OTA_GetAgentState()) != eOTA_AgentState_Stopped) && _connected)
    {
      IotClock_SleepMs(1000);
      ESP_LOGI("AWS SERVICE OTA Agent", "State: %s  Rx: %u  Queued: %u  Processed: %u  Dropped: %u\r\n", _ota_state_dict[ota_state], OTA_GetPacketsReceived(), OTA_GetPacketsQueued(), OTA_GetPacketsProcessed(), OTA_GetPacketsDropped());
    }

    // Check if were disconnected, if so, suspend the OTA Agent.
    if(!_connected)
    {
      if(OTA_Suspend() == kOTA_Err_None)
      {
        while((ota_state = OTA_GetAgentState()) != eOTA_AgentState_Suspended)
        {
          // Wait for OTA Agent to process the suspend event.
          IotClock_SleepMs( 1000 );
        }
      }
    }
  }
}

void AWS_SERVICE_register()
{
  sc_service_t as;

  as.init_service = AWS_SERVICE_init;
  as.deinit_service = AWS_SERVICE_deinit;
  as.recv_msg = AWS_SERVICE_recv_msg;
  as.service_id = sc_service_aws;

  SC_register_service(&as);
}
