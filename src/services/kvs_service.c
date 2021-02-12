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

#include "esp_log.h"

#define KVS_NAMESPACE             "KVS"

#define KVS_MAX_CHARS_IN_ENTRIES  (1U)

static int KVS_SERVICE_put_value(kvs_entry_t *entry)
{
  char id_str[KVS_MAX_CHARS_IN_ENTRIES + 1];

  // Transform key to string, as required by nvs
  snprintf(id_str, (sizeof(id_str) / sizeof(char)), "%u", entry->key);

  // Write
  return FE_NVS_write_key_value(KVS_NAMESPACE, id_str, entry->value, entry->value_len);
}

static int KVS_SERVICE_get_value(kvs_entry_t *entry)
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

static int KVS_SERVICE_verify_key(kvs_entry_t *entry)
{
  char id_str[KVS_MAX_CHARS_IN_ENTRIES + 1];

  // Transform key to string, as required by nvs
  snprintf(id_str, (sizeof(id_str) / sizeof(char)), "%u", entry->key);

  // Read
  if (FE_NVS_verify_key(KVS_NAMESPACE, id_str) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }

  return FE_NVS_read_key_value(KVS_NAMESPACE, id_str, entry->value, entry->value_len);
}

static int KVS_SERVICE_init()
{
  if (FE_NVS_init() != EXIT_SUCCESS)
  {
    ESP_LOGI("KVS SERVICE", "Could not initialize NVS library\n");

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int KVS_SERVICE_deinit()
{
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
