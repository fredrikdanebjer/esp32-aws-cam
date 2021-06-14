/*
* @file command_parser.c
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

#include "command_parser.h"

#include "fsu_eye_aws_credentials.h"

#include <string.h>

#include "jsmn.h"

#include "esp_log.h"

/**
 * /r/command messages are expected to have this form:
 * {
 *  "id":<thing_name>,      // The device thing name
 *  "service_id":<service>, // ID of the service to perform
 *  "command_id":<command>  // Command ID to perform on the service
 *  "..."                   // Optional extra tokens on service basis
 * }
**/

#define COMMAND_MESSAGE_TOKENS_NO_ARG   (7U)
#define COMMAND_ID_TOKEN_OFFSET         (1U)
#define COMMAND_SID_TOKEN_OFFSET        (3U)
#define COMMAND_CMD_TOKEN_OFFSET        (5U)
#define COMMAND_MESSAGE_ID_FIELD        "id"
#define COMMAND_MESSAGE_SERVICE_FIELD   "service_id"
#define COMMAND_MESSAGE_COMMAND_FIELD   "command_id"

/* KVS Command Defines */
/**
 * The KVS commands adds two extra fields as per defined below
**/
#define COMMAND_MESSAGE_TOKENS_KVS      (11U)
#define COMMAND_KVS_KEY_TOKEN_OFFSET    (7U)
#define COMMAND_KVS_VALUE_TOKEN_OFFSET  (9U)
#define COMMAND_MESSAGE_KVS_KEY_FIELD   "kvs key"
#define COMMAND_MESSAGE_KVS_VALUE_FIELD "kvs value"
/* End KVS Command*/

#define EYE_SUBSCRIBE_MAX_TOKENS      (0x10U)

#define LOG_TAG     "COMMAND PARSER"

// JSON Helper function, to check if a token is a sought after string
static int _jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
  if (JSMN_STRING == tok->type
    && (int)strlen(s) == (tok->end - tok->start)
    && strncmp(json + tok->start, s, tok->end - tok->start) == 0)
  {
    return true;
  }
  return false;
}

int CP_parse_upstream_json(cp_fsu_service_argument_t *arg, const char *json, size_t json_len)
{
  jsmn_parser parser;
  jsmntok_t tokens[EYE_SUBSCRIBE_MAX_TOKENS];
  int parsed_tokens = 0;
  uint32_t service = 0;
  uint32_t command = 0;
  const char *payload = json;

  jsmn_init(&parser);
  if ((parsed_tokens = jsmn_parse(&parser, payload, json_len, tokens, EYE_SUBSCRIBE_MAX_TOKENS)) < 0)
  {
    ESP_LOGW(LOG_TAG, "MQTT subscribe returned failed during parse with %d.", parsed_tokens);
  }

  if (COMMAND_MESSAGE_TOKENS_NO_ARG != parsed_tokens
      && COMMAND_MESSAGE_TOKENS_KVS != parsed_tokens)
  {
    ESP_LOGW(LOG_TAG, "MQTT subscribe invalid command message, found %d tokens.", parsed_tokens);
    return EXIT_FAILURE;
  }

  if (_jsoneq(payload, &tokens[COMMAND_ID_TOKEN_OFFSET], COMMAND_MESSAGE_ID_FIELD)
    && _jsoneq(payload, &tokens[COMMAND_SID_TOKEN_OFFSET], COMMAND_MESSAGE_SERVICE_FIELD)
    && _jsoneq(payload, &tokens[COMMAND_CMD_TOKEN_OFFSET], COMMAND_MESSAGE_COMMAND_FIELD))
  {
    // ID
    if (!_jsoneq(payload, &tokens[COMMAND_ID_TOKEN_OFFSET + 1], FSU_EYE_AWS_IOT_THING_NAME))
    {
      ESP_LOGW(LOG_TAG, "MQTT subscribe invalid ID received on commange message!");
      return EXIT_FAILURE;
    }

    // SID
    service = strtoul(&payload[tokens[COMMAND_SID_TOKEN_OFFSET + 1].start], NULL, 10);
    if (0 == service)
    {
      ESP_LOGW(LOG_TAG, "MQTT subscribe invalid service received on commange message.");
      return EXIT_FAILURE;
    }
    arg->sid = service;

    // Command
    command = strtoul(&payload[tokens[COMMAND_CMD_TOKEN_OFFSET + 1].start], NULL, 10);
    if (0 == command)
    {
      ESP_LOGW(LOG_TAG, "MQTT subscribe invalid command received on commange message.");
      return EXIT_FAILURE;
    }
    arg->cmd = command;
  }
  else
  {
    return EXIT_FAILURE;
  }

  // If KVS, continue parsing
  if (sc_service_kvs == arg->sid)
  {
    if (_jsoneq(payload, &tokens[COMMAND_KVS_KEY_TOKEN_OFFSET], COMMAND_MESSAGE_KVS_KEY_FIELD)
     && _jsoneq(payload, &tokens[COMMAND_KVS_VALUE_TOKEN_OFFSET], COMMAND_MESSAGE_KVS_VALUE_FIELD))
    {
      // KVS Key
      arg->as.kvs.key = strtoul(&payload[tokens[COMMAND_KVS_KEY_TOKEN_OFFSET + 1].start], NULL, 10);
      if (0 == arg->as.kvs.key)
      {
        ESP_LOGW(LOG_TAG, "MQTT subscribe invalid KVS Key field received on commange message.");
        return EXIT_FAILURE;
      }

      // KVS Value
      memcpy(arg->as.kvs.value, payload + tokens[COMMAND_KVS_VALUE_TOKEN_OFFSET + 1].start, tokens[COMMAND_KVS_VALUE_TOKEN_OFFSET + 1].end - tokens[COMMAND_KVS_VALUE_TOKEN_OFFSET + 1].start);
      arg->as.kvs.value_len = (tokens[COMMAND_KVS_VALUE_TOKEN_OFFSET + 1].end - tokens[COMMAND_KVS_VALUE_TOKEN_OFFSET + 1].start);
    }
  }

  if (service >= sc_service_count)
  {
    ESP_LOGW(LOG_TAG, "MQTT subscribe received unknown service id %d, discarding", service);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
