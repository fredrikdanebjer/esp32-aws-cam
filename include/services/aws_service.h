/*
* @file aws_service.h
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

#ifndef AWS_SERVICE__H_
#define AWS_SERVICE__H_

#include "system_controller.h"
#include <stdlib.h>

#define AWS_SERVICE_CMD_MQTT_CONNECT_SUBSCRIBE  (0U)
#define AWS_SERVICE_CMD_MQTT_PUBLISH_MESSAGE    (1U)
#define AWS_SERVICE_CMD_MQTT_PUBLISH_IMAGE      (2U)

typedef struct message_info {
  char* msg;
  uint16_t msg_len; // Length of message, excluding terminator
} message_info_t;

typedef struct image_info {
  uint8_t* buf;
  size_t len;
  size_t width;
  size_t height;
  uint8_t format;
} image_info_t;

void AWS_SERVICE_register();

#endif // AWS_SERVICE__H_
