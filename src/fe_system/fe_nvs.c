/*
* @file fe_nvs.c
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

#include "fe_nvs.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "esp_log.h"

static uint8_t _initialized = 0;

int FE_NVS_init()
{
  if (_initialized)
  {
    return EXIT_SUCCESS;
  }

  esp_err_t ret = nvs_flash_init();

  if(ESP_OK != ret)
  {
    ESP_LOGW("FE NVS", "Erasing nvs flash, and reinit, due to %d\n", ret);

    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  _initialized = 1;

  return EXIT_SUCCESS;
}

int FE_NVS_deinit()
{
  if (nvs_flash_deinit() != ESP_OK)
  {
    return EXIT_FAILURE;
  }

  _initialized = 0;

  return EXIT_SUCCESS;
}

int FE_NVS_erase()
{
  ESP_ERROR_CHECK(nvs_flash_erase());

  return EXIT_SUCCESS;
}

int FE_NVS_write_key_value(const char *section, const char *key, uint8_t *data, size_t data_sz)
{
  nvs_handle hnvs;

  // Open the NVS
  if (nvs_open(section, NVS_READWRITE, &hnvs) != ESP_OK)
  {
    return EXIT_FAILURE;
  }

  // Write data
  if (nvs_set_blob(hnvs, key, data, data_sz) != ESP_OK)
  {
    return EXIT_FAILURE;
  }

  // Commit the write
  if (nvs_commit(hnvs) != ESP_OK)
  {
    return EXIT_FAILURE;
  }

  // Close
  nvs_close(hnvs);
  return EXIT_SUCCESS;
}

int FE_NVS_read_key_value(const char *section, const char *key, uint8_t *data, size_t data_sz)
{
  nvs_handle hnvs;

  // Open the NVS
  if (nvs_open(section, NVS_READWRITE, &hnvs) != ESP_OK)
  {
    return EXIT_FAILURE;
  }

  // Get required data size
  size_t required_sz = 0;
  esp_err_t err = ESP_OK;
  if ((err = nvs_get_blob(hnvs, key, NULL, &required_sz)) != ESP_OK)
  {
    ESP_LOGI("FE NVS", "Read error due to %d\n", err);
    return EXIT_FAILURE;
  }

  // Check that the caller provided enough data size
  if (data_sz < required_sz)
  {
    return EXIT_FAILURE;
  }

  // Read the data
  if (nvs_get_blob(hnvs, key, data, &required_sz) != ESP_OK)
  {
    return EXIT_FAILURE;
  }

  // Close
  nvs_close(hnvs);
  return EXIT_SUCCESS;
}

int FE_NVS_verify_key(const char *section, const char *key)
{
  nvs_handle hnvs;

  // Open the NVS
  if (nvs_open(section, NVS_READWRITE, &hnvs) != ESP_OK)
  {
    return EXIT_FAILURE;
  }

  // Get required data size
  size_t required_sz = 0;
  esp_err_t err = ESP_OK;
  if ((err = nvs_get_blob(hnvs, key, NULL, &required_sz)) != ESP_OK)
  {
    ESP_LOGI("FE NVS", "Verify read error due to %d\n", err);
    return EXIT_FAILURE;
  }

  // Close
  nvs_close(hnvs);
  return EXIT_SUCCESS;
}
