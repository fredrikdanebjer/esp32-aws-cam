/*
* @file wifi_service.c
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

#include "fe_wifi.h"
#include "system_controller.h"

#include "fsu_eye_wifi_credentials.h"

#include "esp_log.h"

static int WIFI_SERVICE_init()
{
  int status = FE_WIFI_init(FSU_EYE_WIFI_SSID,
                            FSU_EYE_WIFI_PASSWORD,
                            FSU_EYE_WIFI_SECURITY);

  if (EXIT_SUCCESS != status)
  {
    return status;
  }

  return FE_WIFI_connect();
}

static int WIFI_SERVICE_deinit()
{
  int status = FE_WIFI_disconnect();

  if (EXIT_SUCCESS != status)
  {
    return status;
  }

  return FE_WIFI_deinit();
}

static int WIFI_SERVICE_recv_msg(uint8_t cmd, void* arg)
{
  return EXIT_SUCCESS;
}

void WIFI_SERVICE_register()
{
  sc_service_t ws;

  ws.init_service = WIFI_SERVICE_init;
  ws.deinit_service = WIFI_SERVICE_deinit;
  ws.recv_msg = WIFI_SERVICE_recv_msg;
  ws.service_id = sc_service_wifi;

  SC_register_service(&ws);
}
