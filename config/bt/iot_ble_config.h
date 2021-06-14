/*
* @file iot_ble_config.h
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

#ifndef _IOT_BLE_CONFIG_H_
#define _IOT_BLE_CONFIG_H_

#include "fsu_eye_aws_credentials.h" 

/* BLE Namespace Device name */
#define IOT_BLE_DEVICE_COMPLETE_LOCAL_NAME        ("BLE-" FSU_EYE_AWS_IOT_THING_NAME)

/* Enable to use custom services  */
#define IOT_BLE_ADD_CUSTOM_SERVICES               (1U)

/* Disable the pre-configured device info service */
#define IOT_BLE_ENABLE_DEVICE_INFO_SERVICE        (0U)

/* Disable the pre-configured data transfer service*/
#define IOT_BLE_ENABLE_DATA_TRANSFER_SERVICE      (0U)

/* Connection settings, simplest and lowest security */
#define IOT_BLE_ENABLE_NUMERIC_COMPARISON        (0U)
#define IOT_BLE_ENABLE_SECURE_CONNECTION         (0U)
#define IOT_BLE_INPUT_OUTPUT                     (0U)
#define IOT_BLE_ENCRYPTION_REQUIRED              (0U)

/* Enable custom advertisement callback */
#define IOT_BLE_SET_CUSTOM_ADVERTISEMENT_MSG      (1U)

/* Finally load rest of BLE settings as defaults */
#include "iot_ble_config_defaults.h"

#endif /* _IOT_BLE_CONFIG_H_ */
