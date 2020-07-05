/*
* @file fe_wifi.c
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

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>
#include <limits.h>
#include "esp_wifi.h"

static WIFINetworkParams_t wifi_credentials;

static char _ssid[UCHAR_MAX];
static char _passwd[UCHAR_MAX];

static uint8_t _initialized = 0;

int FE_WIFI_init(char *ssid, char *passwd, WIFISecurity_t security)
{
  if (_initialized)
  {
    return EXIT_SUCCESS;
  }

  if (UCHAR_MAX < strlen(ssid) + 1 || UCHAR_MAX < strlen(passwd) + 1)
  {
    return EXIT_FAILURE;
  }

  memset(&wifi_credentials, '0', sizeof(WIFINetworkParams_t));

  memcpy(_ssid, ssid, strlen(ssid) + 1);
  memcpy(_passwd, passwd, strlen(passwd) + 1);

  wifi_credentials.pcSSID = _ssid;
  wifi_credentials.ucSSIDLength = strlen(_ssid) + 1;
  wifi_credentials.pcPassword = _passwd;
  wifi_credentials.ucPasswordLength = strlen(_passwd) + 1;
  wifi_credentials.xSecurity = security;

  if (WIFI_On() != eWiFiSuccess)
  {
    return EXIT_FAILURE;
  }

  _initialized = 1;

  return EXIT_SUCCESS;
}

int FE_WIFI_deinit()
{
  if (WIFI_Off() != eWiFiSuccess)
  {
    return EXIT_FAILURE;
  }

  _initialized = 0;

  return EXIT_SUCCESS;
}

int FE_WIFI_connect(void)
{
  uint32_t retry_period = 500;

  if (!_initialized)
  {
    return EXIT_FAILURE;
  }

  while (WIFI_ConnectAP(&(wifi_credentials)) != eWiFiSuccess)
  {
    vTaskDelay(pdMS_TO_TICKS(retry_period));
  }

  return EXIT_SUCCESS;
}

int FE_WIFI_disconnect(void)
{
  (void)WIFI_Disconnect();

  return EXIT_FAILURE;
}
