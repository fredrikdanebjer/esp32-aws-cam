/*
* @file fe_wifi.h
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

#ifndef FE_WIFI__H
#define FE_WIFI__H

#include "iot_wifi.h"

/*
* @brief Initializes the WiFi in the FE_SYS space.
* @param ssid Null-terminated SSID string
* @param passwd Null-terminated Password String for the network
* @param security Wifi security model
*/
int FE_WIFI_init(char *ssid, char *passwd, WIFISecurity_t security);

/*
* @brief Deinitializes the WiFi
*/
int FE_WIFI_deinit();

/*
* @brief Connects to initialzed WiFi
*/
int FE_WIFI_connect(void);

#endif /* ifndef FE_WIFI__H */
