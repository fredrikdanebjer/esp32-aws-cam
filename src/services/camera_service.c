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

#include "fsu_http_server_config.h"

#include "esp_http_server.h"
#include "esp_camera.h"
#include "esp_log.h"

#include "semphr.h"

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

#define PART_BOUNDARY "123456789000000000000987654321"

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static SemaphoreHandle_t _camera_mutex;

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
  .frame_size = FRAMESIZE_VGA,

  .jpeg_quality = 12, //0-63 lower number means higher quality
  .fb_count = 1 //if more than one, i2s runs in continuous mode. Use only with JPEG
};

static httpd_handle_t httpd_handle;

static uint8_t _service_initialized = 0;
static uint8_t _camera_initialized = 0;
static uint8_t _http_server_initialized = 0;

static int CAM_SERVICE_camera_init()
{
  esp_err_t err = ESP_OK;

  if (_camera_initialized)
  {
    return EXIT_SUCCESS;
  }

  if ((err = esp_camera_init(&camera_config)) != ESP_OK)
  {
    ESP_LOGI("CAM_SERVICE", "Camera init failed with %d\n", err);
    return EXIT_FAILURE;
  }

  _camera_initialized = 1;

  return EXIT_SUCCESS;
}

static void CAM_SERVICE_camera_deinit()
{
  esp_camera_deinit();

  _camera_initialized = 0;
}

/**
 *  Below function has been substantionally taken from random nerd tutorials, with below original copyright notice:
 *
 *  Rui Santos
 *  Complete project details at https://RandomNerdTutorials.com/esp32-cam-video-streaming-web-server-camera-home-assistant/
 *
 *  IMPORTANT!!!
 *   - Select Board "AI Thinker ESP32-CAM"
 *   - GPIO 0 must be connected to GND to upload a sketch
 *   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files.
 *
**/
static esp_err_t http_server_handler(httpd_req_t *req)
{
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK)
  {
    return res;
  }

  if(xSemaphoreTake(_camera_mutex, (TickType_t) 10U) == pdTRUE)
  {
    while (true)
    {
      fb = esp_camera_fb_get();
      if (!fb)
      {
        ESP_LOGI("CAM_SERVICE", "Camera capture failed");
        res = ESP_FAIL;
      }
      else
      {
        if (fb->width > 400)
        {
          if (fb->format != PIXFORMAT_JPEG)
          {
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            esp_camera_fb_return(fb);
            fb = NULL;
            if (!jpeg_converted)
            {
              ESP_LOGI("CAM_SERIVE", "JPEG compression failed");
              res = ESP_FAIL;
            }
          }
          else
          {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
          }
        }
      }
      if (res == ESP_OK)
      {
        size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
      }
      if (res == ESP_OK)
      {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
      }
      if (res == ESP_OK)
      {
        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
      }
      if (fb)
      {
        esp_camera_fb_return(fb);
        fb = NULL;
        _jpg_buf = NULL;
      }
      else if (_jpg_buf)
      {
        free(_jpg_buf);
        _jpg_buf = NULL;
      }
      if (res != ESP_OK)
      {
        break;
      }
    }
    xSemaphoreGive(_camera_mutex);
  }
  return res;
}

static int CAM_SERVICE_http_server_start()
{
  if (!_camera_initialized)
  {
    return EXIT_FAILURE;
  }

  if (_http_server_initialized)
  {
    return EXIT_SUCCESS;
  }

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = FSU_HTTP_SERVER_PORT;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = http_server_handler,
    .user_ctx  = NULL
  };

  ESP_LOGI("CAM_SERVICE", "Starting http server on port: '%d'\n", config.server_port);
  if (httpd_start(&httpd_handle, &config) == ESP_OK) {
    httpd_register_uri_handler(httpd_handle, &index_uri);
  }

  _http_server_initialized = 1;

  return EXIT_SUCCESS;
}

static int CAM_SERVICE_send_camera_capture()
{
  if(xSemaphoreTake(_camera_mutex, (TickType_t) 10U) == pdTRUE)
  {
    camera_fb_t *fb = esp_camera_fb_get();
    image_info_t image = {0};

    if (!fb)
    {
      ESP_LOGI("CAM_SERVICE", "Capture failed to acquire frame\n");
      xSemaphoreGive(_camera_mutex);
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

    xSemaphoreGive(_camera_mutex);

    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}

static int CAM_SERVICE_init()
{
  if (_service_initialized)
  {
    return EXIT_SUCCESS;
  }

  _camera_mutex = xSemaphoreCreateMutex();

  if (CAM_SERVICE_camera_init() != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }

  if (CAM_SERVICE_http_server_start() != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }

  _service_initialized = 1;

  return EXIT_SUCCESS;
}

static int CAM_SERVICE_deinit()
{
  CAM_SERVICE_camera_deinit();

  vSemaphoreDelete(_camera_mutex);
  _service_initialized = 0;

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
