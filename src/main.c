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
#include "system_controller.h"
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

// EYE App configs
#define EYE_APP_TASK_PRIORITY     (tskIDLE_PRIORITY + 5U)
#define EYE_APP_STACKSIZE         (0x1200U)

#define EYE_OTA_TASK_PRIORITY     (tskIDLE_PRIORITY + 4U)
#define EYE_OTA_STACKSIZE         (0x1800U)

#define MICROSECONDS              (1000000U)

#define EYE_APP_PUBLISH_INFO  "Hello, from FSU App"

static message_info_t publish_msg;

static void eye_app(void * pArgument)
{
  uint64_t current_tic = 0;
  uint64_t last_time_camera = 0;
  uint64_t last_time_message = 0;

  while (1)
  {
    SC_send_cmd(sc_service_aws, AWS_SERVICE_CMD_MQTT_CONNECT_SUBSCRIBE, NULL);
    current_tic = esp_timer_get_time();

    if (current_tic - last_time_message > MICROSECONDS * 60 * 15)
    {
      ESP_LOGI("FSU_EYE", "Sending Info!\n");
      SC_send_cmd(sc_service_aws, AWS_SERVICE_CMD_MQTT_PUBLISH_MESSAGE, &publish_msg);
      last_time_message = esp_timer_get_time();
    }

    if (current_tic - last_time_camera > MICROSECONDS * 30)
    {
      ESP_LOGI("FSU_EYE", "Taking Picture!\n");
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

  // Initialize the System Controller
  SC_init();

  // Registers services to System Controllers
  WIFI_SERVICE_register();
  AWS_SERVICE_register();
  CAM_SERVICE_register();

  memset(&publish_msg, 0, sizeof(message_info_t));
  publish_msg.msg = EYE_APP_PUBLISH_INFO;
  publish_msg.msg_len = strlen(EYE_APP_PUBLISH_INFO);

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

