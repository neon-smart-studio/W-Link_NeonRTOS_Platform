
/**
  ******************************************************************************
  * @file           : ndef_type_wifi.h
  * @brief          : NDEF Wifi type header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/******************************************************************************
 * This file contains code derived from or based on software provided by
 * STMicroelectronics.
 *
 * Original source:
 * STMicroelectronics X-CUBE / BSP / Middleware component
 *
 * Modifications:
 * Copyright (c) 2026 Neon Smart Studio
 * Author: Neon / Neona
 *
 * Licensed under:
 * - Original ST license: ST MIX MYLIBERTY SOFTWARE LICENSE AGREEMENT
 * - Additional modifications may be licensed separately where applicable.
 *
 * The original ST copyright and license notice are preserved below.
 ******************************************************************************/

#ifndef NDEF_TYPE_WIFI_H
#define NDEF_TYPE_WIFI_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"
#include "NDef_Types.h"

#include "NFC/NFC_Def.h"

#define NDEF_WIFI_AUTHENTICATION_NONE       0U  /*!< WPS No Authentication (Should be 1, but set to 0 for Android native support) */
#define NDEF_WIFI_AUTHENTICATION_WPAPSK     2U  /*!< WPS Authentication based on WPAPSK  */
#define NDEF_WIFI_AUTHENTICATION_SHARED     3U  /*!< WPS Authentication                  */
#define NDEF_WIFI_AUTHENTICATION_WPA        4U  /*!< WPS Authentication based on WPA     */
#define NDEF_WIFI_AUTHENTICATION_WPA2       5U  /*!< WPS Authentication based on WPA2    */
#define NDEF_WIFI_AUTHENTICATION_WPA2PSK    6U  /*!< WPS Authentication based on WPA2PSK */


#define NDEF_WIFI_ENCRYPTION_NONE    0U  /*!< WPS No Encryption (Should be 1, but set to 0 for Android native support) */
#define NDEF_WIFI_ENCRYPTION_WEP     2U  /*!< WPS Encryption based on WEP  */
#define NDEF_WIFI_ENCRYPTION_TKIP    3U  /*!< WPS Encryption based on TKIP */
#define NDEF_WIFI_ENCRYPTION_AES     4U  /*!< WPS Encryption based on AES  */

/*! Structure to store Network SSID, Authentication Type, Encryption Type and Network Key */
typedef struct {
  NDef_Const_Buffer bufNetworkSSID;   /*!< Network SSID        */
  NDef_Const_Buffer bufNetworkKey;    /*!< Network Key         */
  uint8_t         authentication;   /*!< Authentication type */
  uint8_t         encryption;       /*!< Encryption          */
} NDef_Type_Wifi;

/*! Wifi Record Type buffers */
extern const NDef_Const_Buffer_8 bufMediaTypeWifi;  /*! Wifi Record Type buffer */

NFC_OpResult NDef_WifiInit(NDef_Type *wifi, const NDef_Type_Wifi *wifiConfig);
NFC_OpResult NDef_GetWifi(const NDef_Type *wifi, NDef_Type_Wifi *wifiConfig);
NFC_OpResult NDef_RecordToWifi(const NDef_Record *record, NDef_Type *wifi);
NFC_OpResult NDef_WifiToRecord(const NDef_Type *wifi, NDef_Record *record);

#endif /* NDEF_TYPE_WIFI_H */

/**
  * @}
  *
  */
