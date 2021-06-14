/*
* @file main.c
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

#include <string.h>
#include <stdint.h>
#include "xtensa/core-macros.h"
#include "fe_sys.h"
#include "fe_ble.h"
#include "system_controller.h"
#include "kvs_service.h"
#include "wifi_service.h"
#include "aws_service.h"
#include "camera_service.h"
#include "platform/iot_threads.h"
#include "iot_logging_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

#include "iot_init.h"
#include "esp_log.h"
#include "esp_timer.h"

#include <esp_http_server.h>

#define LOG_TAG                   "FSU EYE"

// EYE App configs
#define EYE_APP_TASK_PRIORITY     (tskIDLE_PRIORITY + 5U)
#define EYE_APP_STACKSIZE         (0x1800U)

#define EYE_OTA_TASK_PRIORITY     (tskIDLE_PRIORITY + 4U)
#define EYE_OTA_STACKSIZE         (0x1200U)

#define MICROSECONDS              (1000000U)

#define EYE_APP_PUBLISH_INFO      ("{" \
                                      "\"fsu-eye version\":\"%u.%u.%u\"," \
                                      "\"webserver local ip\":\"%d.%d.%d.%d\"," \
                                      "\"info report freq\":\"%llu\"" \
                                      "\"image report freq\":\"%llu\"" \
                                      "\"uptime\":\"%llu\"" \
                                  "}")

#define EYE_APP_PUBLISH_INFO_LEN  (0x100U)

static message_info_t publish_msg;

static void eye_app(void * pArgument)
{
  uint64_t current_tic = 0;
  uint64_t last_time_camera = 0;
  uint64_t last_time_message = 0;
  uint64_t image_freq = UINT64_MAX;
  uint64_t info_freq = UINT64_MAX;

  ip_address_t ip = {0};
  char publish_info_msg[EYE_APP_PUBLISH_INFO_LEN] = {'\0'};

  kvs_entry_t freq_entry = {
    .key = kvs_entry_count,
    .value_len = KVS_SERVICE_MAXIMUM_VALUE_SIZE
  };

  while (1)
  {
    SC_send_cmd(sc_service_aws, AWS_SERVICE_CMD_MQTT_CONNECT_SUBSCRIBE, NULL);

    current_tic = esp_timer_get_time();

    memset(freq_entry.value, '\0', KVS_SERVICE_MAXIMUM_VALUE_SIZE);
    freq_entry.key = kvs_entry_eye_info_report_interval;
    SC_send_cmd(sc_service_kvs, KVS_SERVICE_CMD_GET_KEY_VALUE, &freq_entry);

    // We have already verified the value before allowing it to be put into KVS,
    // so just check base case
    if ((info_freq = strtoull(freq_entry.value, NULL, 10)) <= 0)
    {
      ESP_LOGI(LOG_TAG, "Error on fetching Info Report Frequency from KVS\n");
      info_freq = UINT64_MAX;
    }

    if (current_tic - last_time_message > MICROSECONDS * info_freq)
    {
      memset(&publish_msg, 0, sizeof(message_info_t));

      if (FE_SYS_get_ip(&ip) != EXIT_SUCCESS)
      {
        memset(&ip, 0, sizeof(ip_address_t));
      }

      snprintf(publish_info_msg, EYE_APP_PUBLISH_INFO_LEN, EYE_APP_PUBLISH_INFO, APP_VERSION_MAJOR,
                                                                                 APP_VERSION_MINOR,
                                                                                 APP_VERSION_BUILD,
                                                                                 ip.ip4_addr1,
                                                                                 ip.ip4_addr2,
                                                                                 ip.ip4_addr3,
                                                                                 ip.ip4_addr4,
                                                                                 image_freq,
                                                                                 info_freq,
                                                                                 (current_tic / MICROSECONDS));
      publish_msg.msg = publish_info_msg;
      publish_msg.msg_len = strlen(EYE_APP_PUBLISH_INFO);

      ESP_LOGI(LOG_TAG, "Sending Info!\n");
      SC_send_cmd(sc_service_aws, AWS_SERVICE_CMD_MQTT_PUBLISH_MESSAGE, &publish_msg);
      last_time_message = esp_timer_get_time();
    }

    memset(freq_entry.value, '\0', KVS_SERVICE_MAXIMUM_VALUE_SIZE);
    freq_entry.key = kvs_entry_eye_image_report_interval;
    SC_send_cmd(sc_service_kvs, KVS_SERVICE_CMD_GET_KEY_VALUE, &freq_entry);

    // We have already verified the value before allowing it to be put into KVS,
    // so just check base case
    if ((image_freq = strtoull(freq_entry.value, NULL, 10)) <= 0)
    {
      ESP_LOGI(LOG_TAG, "Error on fetching Image Report Frequency from KVS\n");
      image_freq = UINT64_MAX;
    }

    if (current_tic - last_time_camera > MICROSECONDS * image_freq)
    {
      ESP_LOGI(LOG_TAG, "Taking Picture!\n");
      SC_send_cmd(sc_service_camera, CAM_SERVICE_CMD_CAPTURE_SEND_IMAGE, NULL);
      last_time_camera = esp_timer_get_time();
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// Application entry-point
int app_main()
{
  // Initalize System Resources
  FE_SYS_init();

  // Initialize AWS IoT SDK
  IotSdk_Init();

  FE_BLE_init();

  // Create a detached thread for shutting down BLE
  Iot_CreateDetachedThread(FE_BLE_shutdown_runner,
                           NULL,
                           EYE_APP_TASK_PRIORITY,
                           EYE_APP_STACKSIZE);

  // Initialize the System Controller
  SC_init();

  // Registers services to System Controllers
  KVS_SERVICE_register();
  WIFI_SERVICE_register();
  AWS_SERVICE_register();
  CAM_SERVICE_register();

  // Create a detached thread of main app
  Iot_CreateDetachedThread(eye_app,
                           NULL,
                           EYE_APP_TASK_PRIORITY,
                           EYE_APP_STACKSIZE);

  // Create a detached thread of OTA task
  Iot_CreateDetachedThread(AWS_SERVICE_OTA_runner,
                           NULL,
                           EYE_OTA_TASK_PRIORITY,
                           EYE_OTA_STACKSIZE);

  return 0;
}

