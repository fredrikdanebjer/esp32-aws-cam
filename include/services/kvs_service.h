/*
* @file kvs_service.h
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

#ifndef KVS_SERVICE__H
#define KVS_SERVICE__H

#include <stdlib.h>

#define KVS_SERVICE_CMD_GET_KEY_VALUE           (0U)
#define KVS_SERVICE_CMD_PUT_KEY_VALUE           (1U)
#define KVS_SERVICE_CMD_VERIFY_KEY              (2U)

#define KVS_SERVICE_MAXIMUM_VALUE_SIZE          (0x100)

typedef enum {
  kvs_entry_wifi_ssid,
  kvs_entry_wifi_password,
  kvs_entry_eye_image_report_interval,
  kvs_entry_eye_info_report_interval,
  kvs_entry_count
} kvs_entry_id_t;

typedef struct key_value_pair {
  kvs_entry_id_t key;
  char *value;
  size_t value_len;
} kvs_entry_t;

/*
* @brief Registers the key-value-storage service to the system controller.
*/
void KVS_SERVICE_register();

#endif /* ifndef KVS_SERVICE__H */
