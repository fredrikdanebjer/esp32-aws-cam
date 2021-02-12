/*
* @file fsu_eye_wifi_credential.c
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

#ifndef FSU_EYE_WIFI_CREDENTIALS__H
#define FSU_EYE_WIFI_CREDENTIALS__H

#include "iot_wifi.h"

#if (!AFR_ESP_LWIP)
/** \addtogroup FSU_EYE_MAC_ADDR
 *
 * Default MAC Address
 *  @{
 */
#define FSU_EYE_MAC_ADDR0                     0x00
#define FSU_EYE_MAC_ADDR1                     0x11
#define FSU_EYE_MAC_ADDR2                     0x22
#define FSU_EYE_MAC_ADDR3                     0x33
#define FSU_EYE_MAC_ADDR4                     0x44
#define FSU_EYE_MAC_ADDR5                     0x21
/** @}*/

/** \addtogroup FSU_EYE_IP_ADDR
 *
 * Default IP Address
 *  @{
 */
#define FSU_EYE_IP_ADDR0                      192
#define FSU_EYE_IP_ADDR1                      168
#define FSU_EYE_IP_ADDR2                      0
#define FSU_EYE_IP_ADDR3                      105
/** @}*/

/** \addtogroup FSU_EYE_GATEWAY_ADDR
 *
 * Default Gateway Address
 *  @{
 */
#define FSU_EYE_GATEWAY_ADDR0                 192
#define FSU_EYE_GATEWAY_ADDR1                 168
#define FSU_EYE_GATEWAY_ADDR2                 0
#define FSU_EYE_GATEWAY_ADDR3                 1
/** @}*/

/** \addtogroup FSU_EYE_DSN_SERVER_ADDR
 *
 * Default DNS Server Address
 *  @{
 */
#define FSU_EYE_DNS_SERVER_ADDR0              208
#define FSU_EYE_DNS_SERVER_ADDR1              67
#define FSU_EYE_DNS_SERVER_ADDR2              222
#define FSU_EYE_DNS_SERVER_ADDR3              222
/** @}*/

/** \addtogroup FSU_EYE_NET_MASK
 *
 * Default Net MASK
 *  @{
 */
#define FSU_EYE_NET_MASK0                     255
#define FSU_EYE_NET_MASK1                     255
#define FSU_EYE_NET_MASK2                     255
#define FSU_EYE_NET_MASK3                     0
/** @}*/
#endif

/*
 * @brief Wi-Fi credentials max length.
 */
#define FSU_EYE_WIFI_CREDENTIALS_MAX_LENGTH   (50U)

/*
 * @brief Wi-Fi network to join.
 */
#define FSU_EYE_WIFI_SSID                     ""

/*
 * @brief Password needed to join Wi-Fi network.
 */
#define FSU_EYE_WIFI_PASSWORD                 ""

/*
 * @brief Wi-Fi network security type.
 * @note Possible values are eWiFiSecurityOpen, eWiFiSecurityWEP, eWiFiSecurityWPA,
 * eWiFiSecurityWPA2 (depending on the support of your device Wi-Fi radio).
 */
#define FSU_EYE_WIFI_SECURITY                eWiFiSecurityWPA2

#endif /* ifndef FSU_EYE_WIFI_CREDENTIALS__H */
