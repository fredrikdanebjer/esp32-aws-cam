/*
* @file kvs_service.c
*
* The MIT License (MIT)
*
* Copyright (c) 2021 Fredrik Danebjer
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
#include "kvs_service.h"
#include "fe_nvs.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "esp_log.h"

#include "fsu_eye_kvs_defaults.h"

#define KVS_NAMESPACE             "KVS"

#define KVS_MAX_CHARS_IN_ENTRIES  (1U)

/*
* @brief Dictionary of what types the KVS entries has to be validated as,
* internally they are still stored as strings.
*/
static char kvs_entry_type_dict[kvs_entry_count] = {
  's',    // WiFi SSID: String
  's',    // WiFi Password: String
  'u',    // Image Report Interval: Unsigned 64-bit int
  'u'     // Info Report Interval: Unsigned 64-bit int
};

static uint8_t _initialized = 0;
static SemaphoreHandle_t _flash_mutex;

static char _ram_kv_store[kvs_entry_count][KVS_SERVICE_MAXIMUM_VALUE_SIZE];

int _validate_string_as_uint64(const char *str)
{
  const char *itr = str;
  char *end;

  // strtoull accepts prepended spaces and +/- signs, we do not accept this in KVS, so reject
  if (*itr == ' ' || *itr == '+' || *itr == '-')
  {
    return 0;
  }

  errno = 0;

  strtoull(itr, &end, 10);

  if (end == itr
    || '\0' != *end
    || ERANGE == errno)
  {
    ESP_LOGI("KVS SERVICE", "String is not an uint\n");
    return 0;
  }
  return 1;
}

static int KVS_SERVICE_validate_type(kvs_entry_t *entry)
{
  if (kvs_entry_type_dict[entry->key] == 's')
  {
    return 1;
  }
  else if (kvs_entry_type_dict[entry->key] == 'u')
  {
    if (!_validate_string_as_uint64(entry->value))
    {
      return 0;
    }
    return 1;
  }
  ESP_LOGI("KVS SERVICE", "Provided entry has unknown type\n");

  return 0;
}

static int KVS_SERVICE_put_value(kvs_entry_t *entry)
{
  char id_str[KVS_MAX_CHARS_IN_ENTRIES + 1];


  if (!KVS_SERVICE_validate_type(entry))
  {
    return EXIT_FAILURE;
  }

  if(xSemaphoreTake(_flash_mutex, (TickType_t) 10U) == pdTRUE)
  {
    // Transform key to string, as required by nvs
    snprintf(id_str, (sizeof(id_str) / sizeof(char)), "%u", entry->key);

    // Write
    if (FE_NVS_write_key_value(KVS_NAMESPACE, id_str, entry->value, entry->value_len) == EXIT_SUCCESS)
    {
      // Update RAM KVS
      memset(_ram_kv_store[entry->key], '\0', KVS_SERVICE_MAXIMUM_VALUE_SIZE);
      memcpy(_ram_kv_store[entry->key], entry->value, entry->value_len);

      xSemaphoreGive(_flash_mutex);

      return EXIT_SUCCESS;
    }

    xSemaphoreGive(_flash_mutex);
  }
  return EXIT_FAILURE;
}

static int KVS_SERVICE_get_value_from_nvs(kvs_entry_t *entry)
{
  char id_str[KVS_MAX_CHARS_IN_ENTRIES + 1];

  // Transform key to string, as required by nvs
  snprintf(id_str, (sizeof(id_str) / sizeof(char)), "%u", entry->key);

  // Read
  if (FE_NVS_read_key_value(KVS_NAMESPACE, id_str, NULL, entry->value_len) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }

  return FE_NVS_read_key_value(KVS_NAMESPACE, id_str, entry->value, entry->value_len);
}

static int KVS_SERVICE_get_value(kvs_entry_t *entry)
{
  if(xSemaphoreTake(_flash_mutex, (TickType_t) 10U) == pdTRUE)
  {
    memcpy(entry->value, _ram_kv_store[entry->key], KVS_SERVICE_MAXIMUM_VALUE_SIZE);
    entry->value_len = strlen(entry->value);

    xSemaphoreGive(_flash_mutex);

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

static int KVS_SERVICE_verify_key(kvs_entry_t *entry)
{
  char id_str[KVS_MAX_CHARS_IN_ENTRIES + 1];

  if(xSemaphoreTake(_flash_mutex, (TickType_t) 10U) == pdTRUE)
  {
    // Transform key to string, as required by nvs
    snprintf(id_str, (sizeof(id_str) / sizeof(char)), "%u", entry->key);

    // Read
    if (FE_NVS_verify_key(KVS_NAMESPACE, id_str) != EXIT_SUCCESS)
    {
      xSemaphoreGive(_flash_mutex);

      return EXIT_FAILURE;
    }

    xSemaphoreGive(_flash_mutex);

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

static void KVS_SERVICE_init_store()
{
  kvs_entry_t entry;

  for (uint8_t i = 0; i < kvs_entry_count; ++i)
  {
    memset(_ram_kv_store[i], '\0', KVS_SERVICE_MAXIMUM_VALUE_SIZE);
    entry.key = (kvs_entry_id_t)i;
    if (KVS_SERVICE_verify_key(&entry) != EXIT_FAILURE)
    {
      entry.value = _ram_kv_store[i];
      entry.value_len = KVS_SERVICE_MAXIMUM_VALUE_SIZE;

      if (KVS_SERVICE_get_value_from_nvs(&entry) != EXIT_SUCCESS)
      {
        ESP_LOGI("KVS SERVICE", "Failure to initialize RAM KVS at element %u\n", i);
      }
    }
    else
    {
      entry.value = _kvs_defaults[i];
      entry.value_len = KVS_SERVICE_MAXIMUM_VALUE_SIZE;

      if (KVS_SERVICE_put_value(&entry) != EXIT_SUCCESS)
      {
        ESP_LOGI("KVS SERVICE", "Failure to initialize default NVS KVS at element %u\n", i);
      }
    }
  }
}

static void KVS_SERVICE_deinit_store()
{
  for (size_t i = 0; i < kvs_entry_count; ++i)
  {
    memset(_ram_kv_store[i], '\0', KVS_SERVICE_MAXIMUM_VALUE_SIZE);
  }
}

static int KVS_SERVICE_init()
{
  if (_initialized)
  {
    return EXIT_SUCCESS;
  }

  if (FE_NVS_init() != EXIT_SUCCESS)
  {
    ESP_LOGI("KVS SERVICE", "Could not initialize NVS library\n");

    return EXIT_FAILURE;
  }

  _flash_mutex = xSemaphoreCreateMutex();
  KVS_SERVICE_init_store();
  _initialized = 1;

  return EXIT_SUCCESS;
}

static int KVS_SERVICE_deinit()
{
  if (!_initialized)
  {
    return EXIT_SUCCESS;
  }

  vSemaphoreDelete(_flash_mutex);
  KVS_SERVICE_deinit_store();
  _initialized = 0;

  return EXIT_SUCCESS;
}

static int KVS_SERVICE_recv_msg(uint8_t cmd, void* arg)
{
  switch (cmd)
  {
    case(KVS_SERVICE_CMD_PUT_KEY_VALUE):
      return KVS_SERVICE_put_value((kvs_entry_t*)arg);

    case (KVS_SERVICE_CMD_GET_KEY_VALUE):
      return KVS_SERVICE_get_value((kvs_entry_t*)arg);

    case (KVS_SERVICE_CMD_VERIFY_KEY):
      return KVS_SERVICE_verify_key((kvs_entry_t*)arg);
  }

  return EXIT_FAILURE;
}

void KVS_SERVICE_register()
{
  sc_service_t kvss;

  kvss.init_service = KVS_SERVICE_init;
  kvss.deinit_service = KVS_SERVICE_deinit;
  kvss.recv_msg = KVS_SERVICE_recv_msg;
  kvss.service_id = sc_service_kvs;

  SC_register_service(&kvss);
}
