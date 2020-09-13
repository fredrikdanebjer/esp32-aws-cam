/*
* @file camera_service.c
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

#include "system_controller.h"
#include "camera_service.h"
#include "aws_service.h"

#include "esp_camera.h"
#include "esp_log.h"

// ESP32-S Camera Pins
#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

static camera_config_t camera_config = {
  .pin_pwdn  = CAM_PIN_PWDN,
  .pin_reset = CAM_PIN_RESET,
  .pin_xclk = CAM_PIN_XCLK,
  .pin_sscb_sda = CAM_PIN_SIOD,
  .pin_sscb_scl = CAM_PIN_SIOC,

  .pin_d7 = CAM_PIN_D7,
  .pin_d6 = CAM_PIN_D6,
  .pin_d5 = CAM_PIN_D5,
  .pin_d4 = CAM_PIN_D4,
  .pin_d3 = CAM_PIN_D3,
  .pin_d2 = CAM_PIN_D2,
  .pin_d1 = CAM_PIN_D1,
  .pin_d0 = CAM_PIN_D0,
  .pin_vsync = CAM_PIN_VSYNC,
  .pin_href = CAM_PIN_HREF,
  .pin_pclk = CAM_PIN_PCLK,

  //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
  .xclk_freq_hz = 20000000,
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,

  .pixel_format = PIXFORMAT_JPEG,
  .frame_size = FRAMESIZE_QVGA,

  .jpeg_quality = 12, //0-63 lower number means higher quality
  .fb_count = 1 //if more than one, i2s runs in continuous mode. Use only with JPEG
};

static uint8_t _initialized = 0;

static int CAM_SERVICE_camera_init()
{
  esp_err_t err = ESP_OK;

  if ((err = esp_camera_init(&camera_config)) != ESP_OK)
  {
    ESP_LOGI("CAM_SERVICE", "Camera init failed with %d\n", err);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int CAM_SERVICE_send_camera_capture()
{
  camera_fb_t *fb = esp_camera_fb_get();
  image_info_t image = {0};

  if (!fb)
  {
    ESP_LOGI("CAM_SERVICE", "Capture failed to acquire frame\n");
    return EXIT_FAILURE;
  }

  image.buf = fb->buf;
  image.len = fb->len;
  image.width = fb->width;
  image.height = fb->height;
  image.format = (uint8_t) fb->format;

  ESP_LOGI("CAM_SERVICE", "Sending Picture\n");
  SC_send_cmd(sc_service_aws, AWS_SERVICE_CMD_MQTT_PUBLISH_IMAGE, &image);

  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return(fb);

  return ESP_OK;
}

static int CAM_SERVICE_init()
{
  if (_initialized)
  {
    return EXIT_SUCCESS;
  }

  if (CAM_SERVICE_camera_init() != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }

  _initialized = 1;

  return EXIT_SUCCESS;
}

static int CAM_SERVICE_deinit()
{
  return EXIT_SUCCESS;
}

static int CAM_SERVICE_recv_msg(uint8_t cmd, void* arg)
{
  (void) arg;

  switch (cmd)
  {
    case (CAM_SERVICE_CMD_CAPTURE_SEND_IMAGE):
      return CAM_SERVICE_send_camera_capture();
  }

  return EXIT_FAILURE;
}

void CAM_SERVICE_register()
{
  sc_service_t cs;

  cs.init_service = CAM_SERVICE_init;
  cs.deinit_service = CAM_SERVICE_deinit;
  cs.recv_msg = CAM_SERVICE_recv_msg;
  cs.service_id = sc_service_camera;

  SC_register_service(&cs);
}
