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

#include "fsu_eye_aws_credentials.h"
#include "private/iot_default_root_certificates.h"

#define FSU_EYE_NETWORK_INTERFACE     IOT_NETWORK_INTERFACE_AFR
#define FSU_EYE_AWS_IOT_ALPN_MQTT    "x-amzn-mqtt-ca"

#define KEEP_ALIVE_SECONDS            (60U)
#define MQTT_TIMEOUT_MS               (5000U)

#define FSU_EYE_TOPIC_ROOT            "fsu/eye"
#define FSU_EYE_TOPIC_LWT             (FSU_EYE_TOPIC_ROOT "/lwt")

#define LWT_MESSAGE                   (FSU_EYE_AWS_IOT_THING_NAME " lost connection.")

static IotMqttConnection_t _mqtt_connection;
static uint8_t _initialized = 0;
static uint8_t _connected = 0;

IotNetworkServerInfo_t aws_server_info = {
  .pHostName = FSU_EYE_AWS_MQTT_BROKER_ENDPOINT,
  .port = FSU_EYE_AWS_MQTT_BROKER_PORT  
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

  return EXIT_SUCCESS;
}

static int AWS_SERVICE_deinit()
{
  IotMqtt_Cleanup();

  return EXIT_SUCCESS;
}

static int AWS_SERVICE_recv_msg(uint8_t cmd)
{
  return EXIT_SUCCESS;
}

static int AWS_SERVICE_mqtt_connect()
{
  IotMqttError_t connect_status;
  IotMqttNetworkInfo_t network_info = IOT_MQTT_NETWORK_INFO_INITIALIZER;
  IotMqttConnectInfo_t connect_info = IOT_MQTT_CONNECT_INFO_INITIALIZER;
  IotMqttPublishInfo_t will_info = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
  _mqtt_connection = IOT_MQTT_CONNECTION_INITIALIZER;

  // Set the AWS Connection Credentials
  network_info.createNetworkConnection = true;
  network_info.u.setup.pNetworkServerInfo = &aws_server_info;
  network_info.u.setup.pNetworkCredentialInfo = &network_credentials;
  network_info.pNetworkInterface = FSU_EYE_NETWORK_INTERFACE;

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
  connect_status = IotMqtt_Connect(&network_info,
                                   &connect_info,
                                   MQTT_TIMEOUT_MS,
                                   &_mqtt_connection);

  if (IOT_MQTT_SUCCESS == connect_status)
  {
    return EXIT_FAILURE;
  }

  _connected = 1;
  return EXIT_SUCCESS;
}

void AWS_SERVICE_register()
{
  sc_service_t as;

  as.init_service = AWS_SERVICE_init;
  as.deinit_service = AWS_SERVICE_deinit;
  as.recv_msg = AWS_SERVICE_recv_msg;
  as.service_id = sc_service_aws;

  SC_register_service(&as);

  while (AWS_SERVICE_mqtt_connect() != EXIT_SUCCESS);
}
