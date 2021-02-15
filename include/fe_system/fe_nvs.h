/*
* @file fe_nvs.h
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

#ifndef FE_NVS__H
#define FE_NVS__H

#include <stdlib.h>
#include <stdint.h>

/*
* @brief Initializes the NVS
* @retval EXIT_SUCCESS on success, otherwise EXIT_FAILURE
*/
int FE_NVS_init();

/*
* @brief Deinitializes the NVS
* @retval EXIT_SUCCESS on success, otherwise EXIT_FAILURE
*/
int FE_NVS_deinit();

/*
* @brief Erases the NVS
* @retval EXIT_SUCCESS on success, otherwise EXIT_FAILURE
*/
int FE_NVS_erase();

/*
* @brief Writes a Key-Value pair to the NVS
* @param section the namespace used to store the keypair
* @param key the key that links to the value
* @param data the value linked to the key
* @param data_sz the number of bytes of data
* @retval EXIT_SUCCESS on success, otherwise EXIT_FAILURE
*/
int FE_NVS_write_key_value(const char *section, const char *key, uint8_t *data, size_t data_sz);

/*
* @brief Reads a Key-Value pair to the NVS
* @param section the namespace used to store the keypair
* @param key the key that links to the value
* @param data address where to store the value linked to the key
* @param data_sz the number of bytes available in data
* @retval EXIT_SUCCESS on success, otherwise EXIT_FAILURE
*/
int FE_NVS_read_key_value(const char *section, const char *key, uint8_t *data, size_t data_sz);

/*
* @brief Checks if a Key-Value pair exists in the NVS
* @param section the namespace used to store the keypair
* @param key the key that links to the value
* @param data_sz the number of bytes available in data
* @retval EXIT_SUCCESS on success, otherwise EXIT_FAILURE
*/
int FE_NVS_verify_key(const char *section, const char *key);

#endif
