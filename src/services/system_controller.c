/*
* @file system_controller.c
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

#include <stdlib.h>
#include <string.h>

#include "system_controller.h"

#include "esp_log.h"

static sc_service_t _services[sc_service_count];
static uint8_t _service_present_mask = 0;
static uint8_t _initialized = 0;

#define SC_IS_SERVICE_ACTIVE(id) ((1 << (id)) & _service_present_mask)
#define SC_ACTIVATE_SERVICE(id) (_service_present_mask |= (1 << (id)))
#define SC_DEACTIVATE__SERVICE(id) (_service_present_mask &= (0xFF - (1 << (id))))

int SC_init()
{
  if (_initialized)
  {
    return EXIT_SUCCESS;
  }

  memset(_services, '0', sizeof(_services));
  _service_present_mask = 0;
  _initialized = 1;

  return EXIT_SUCCESS;
}

int SC_deinit()
{
  for (uint8_t i = 0; i < sc_service_count; ++i)
  {
    SC_deregister_service(i);
  } 

  memset(_services, '0', sizeof(_services));
  _service_present_mask = 0;
  _initialized = 0;

  return EXIT_FAILURE;
}

int SC_register_service(sc_service_t *service)
{
  if (!_initialized)
  {
    ESP_LOGW("SYS_CTRL", "Could not initialize service %u, SystemController was"
                         " not initialized\n", service->service_id);
    return EXIT_FAILURE;
  }
  if (NULL == service)
  {
    ESP_LOGW("SYS_CTRL", "Could not initialize NULL service\n");
    return EXIT_FAILURE;
  }

  // Check if already present
  if (SC_IS_SERVICE_ACTIVE(service->service_id))
  { 
    ESP_LOGW("SYS_CTRL", "Service %u already active, cannot register device\n",
              service->service_id);
    return EXIT_SUCCESS;
  }

  memcpy(&_services[service->service_id], service, sizeof(sc_service_t));
  SC_ACTIVATE_SERVICE(service->service_id);

  ESP_LOGI("SYS_CTRL", "Invoking init for service %u\n", service->service_id);
  _services[service->service_id].init_service();

  return EXIT_SUCCESS;
}

int SC_deregister_service(uint8_t service_id)
{
  if (!SC_IS_SERVICE_ACTIVE(service_id))
  {
    return EXIT_SUCCESS;
  }

  if (_services[service_id].deinit_service(NULL) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }

  SC_DEACTIVATE_SERVICE(service->service_id);
  return EXIT_SUCCESS;
}

int SC_send_cmd(sc_service_list_t sid, uint8_t cmd, void* arg)
{
  if (SC_IS_SERVICE_ACTIVE(sid))
  {
    return _services[sid].recv_msg(cmd, arg);
  }

  return EXIT_FAILURE;
}
