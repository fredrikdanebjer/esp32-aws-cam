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

#include <stdint.h>
#include "xtensa/core-macros.h"
#include "fe_sys.h"
#include "system_controller.h"
#include "wifi_service.h"

#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

#include "esp_log.h"

// CPU Ticks per second
#define CPU_SPEED_SECONDS (160000000U)

// Application entry-point
void app_main()
{
  // Initalize System Resources
  FE_SYS_init();

  SC_init();
  WIFI_SERVICE_register();

  uint32_t last_time = XTHAL_GET_CCOUNT();

  while (1)
  {
    // Inprecise wait using XT-HAL
    while (XTHAL_GET_CCOUNT() - last_time < CPU_SPEED_SECONDS * 5);
    last_time = XTHAL_GET_CCOUNT();
    ESP_LOGI("FSU_EYE", "Hello from FSU-Eye!\n");
  }
}

