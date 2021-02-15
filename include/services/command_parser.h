/*
* @file command_parser.h
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

#ifndef COMMAND_PARSER__H
#define COMMAND_PARSER__H

#include "system_controller.h"
#include "kvs_service.h"
#include "aws_service.h"

#include <stdint.h>

typedef struct fsu_service_argument {
  sc_service_list_t sid;
  uint8_t cmd;
  union {
    message_info_t info_msg;
    kvs_entry_t kvs;
  } as;
} cp_fsu_service_argument_t;

/*
* @brief Parses an upstream JSON formatted message intended for the FSU-Eye and
* fills the argument struct accordingly.
* @param arg instant of cp_fsu_service_argument_t which to fill with parsed data
* @param json the string to parse
* @param json_len the length of the string to parse
*/
int CP_parse_upstream_json(cp_fsu_service_argument_t *arg, const char *json, size_t json_len);

#endif /* ifndef COMMAND_PARSER__H */
