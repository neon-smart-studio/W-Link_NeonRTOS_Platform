
/**
  ******************************************************************************
  * @file           : NDef__type_wifi.cpp
  * @brief          : NDEF Wifi type
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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "NDef_Record.h"
#include "NDef_Types.h"
#include "NDef_Type_WiFi.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_WIFI_SUPPORT

/*! Wifi Type strings */
static const uint8_t NDef_MediaTypeWifi[]     = "application/vnd.wfa.wsc";   /*!< Wi-Fi Simple Configuration Type */

const NDef_Const_Buffer_8 bufMediaTypeWifi      = { NDef_MediaTypeWifi,      sizeof(NDef_MediaTypeWifi) - 1U      };  /*!< Wifi Type buffer     */


/*! Wifi OBB (WPS) */

#define NDEF_WIFI_DEFAULT_NETWORK_KEY      "00000000"      /*! Network key to be used when the Authentication is set to None.
                                                               Although WPS defines a 0-length network key in such case,
                                                               a 8 digit value is required with tested phones. */

#define NDEF_WIFI_NETWORK_SSID_LENGTH           32U    /*!< Network SSID length        */
#define NDEF_WIFI_NETWORK_KEY_LENGTH            32U    /*!< Network Key length         */

#define NDEF_WIFI_ENCRYPTION_TYPE_LENGTH         2U    /*!< Encryption type length     */
#define NDEF_WIFI_AUTHENTICATION_TYPE_LENGTH     2U    /*!< Authentication type length */
#define WIFI_SSID_TYPE_LENGTH                    2U    /*!< SSID type length           */
#define WIFI_SSID_KEY_TYPE_LENGTH                2U    /*!< SSID key type length       */

#define NDEF_WIFI_ATTRIBUTE_ID_SSID_LSB       0x10U    /*!< SSID Attribute ID LSB      */
#define NDEF_WIFI_ATTRIBUTE_ID_SSID_MSB       0x45U    /*!< SSID Attribute ID MSB      */

#define NDEF_WIFI_ATTRIBUTE_ID_NETWORK_LSB    0x10U    /*!< Network Attribute ID LSB   */
#define NDEF_WIFI_ATTRIBUTE_ID_NETWORK_MSB    0x27U    /*!< Network Attribute ID MSB   */

#define NDEF_WIFI_ATTRIBUTE_ENCRYPTION        0x0FU    /*!< Encryption attribute       */
#define NDEF_WIFI_ATTRIBUTE_AUTHENTICATION    0x03U    /*!< Authentication attribute   */

#define NDEF_WIFI_ATTRIBUTE_ID_OFFSET                 0x01U    /*!< Attribute Id offset */
#define NDEF_WIFI_ATTRIBUTE_LENGTH_MSB_OFFSET         0x02U    /*!< Attribute length MSB offset     */
#define NDEF_WIFI_ATTRIBUTE_LENGTH_LSB_OFFSET         0x03U    /*!< Attribute length LSB offset     */
#define NDEF_WIFI_ATTRIBUTE_DATA_OFFSET               0x04U    /*!< Attribute data offset           */
#define NDEF_WIFI_ATTRIBUTE_ENCRYPTION_LSB_OFFSET     0x05U    /*!< Attribute encryption offset     */
#define NDEF_WIFI_ATTRIBUTE_AUTHENTICATION_LSB_OFFSET 0x05U    /*!< Attribute authentication offset */


static uint8_t wifiConfigToken1[] = {
  0x10, 0x4A, /* Attribute ID: Version       */
  0x00, 0x01, /* Attribute ID Length         */
  0x10,       /* Version 1.0                 */
  0x10, 0x0E, /* Attribute ID Credential     */
  0x00, 0x48, /* Attribute ID Length         */
  0x10, 0x26, /* Attribute ID: Network Index */
  0x00, 0x01, /* Attribute Length            */
  0x01,       /* Index                       */
  0x10, 0x45  /* Attribute ID: SSID          */
};

static uint8_t wifiConfigToken3[] = {
  0x10, 0x03, /* Attribute ID:Authentication Type */
  0x00, 0x02, /* Attribute Length                 */
  0x00, 0x01, /* Attribute Type: Open             */
  0x10, 0x0F, /* Attribute ID: Encryption Type    */
  0x00, 0x02, /* Attribute Length                 */
  0x00, 0x01, /* Encryption Type: None            */
  0x10, 0x27  /* Attribute ID: Network Key        */
};

static uint8_t wifiConfigToken5[] = {
  0x10, 0x20,       /* Attribute ID: MAC Address           */
  0x00, 0x06,       /* Attribute Length                    */
  0,                /* MAC-ADDRESS                         */
  0,                /* MAC-ADDRESS                         */
  0,                /* MAC-ADDRESS                         */
  0,                /* MAC-ADDRESS                         */
  0,                /* MAC-ADDRESS                         */
  0,                /* MAC-ADDRESS                         */
  0x10, 0x49,       /* Attribute ID: Vendor Extension      */
  0x00, 0x06,       /* Attribute Length                    */
  0x00, 0x37, 0x2A, /* Vendor ID: WFA                      */
  0x02,             /* Subelement ID:Network Key Shareable */
  0x01,             /* Subelement Length                   */
  0x01,             /* Network Key Shareable: TRUE         */
  0x10, 0x49,       /* Attribute ID: Vendor Extension      */
  0x00, 0x06,       /* Attribute Length                    */
  0x00, 0x37, 0x2A, /* Vendor ID: WFA                      */
  0x00,             /* Subelement ID: Version2             */
  0x01,             /* Subelement Length: 1                */
  0x20              /* Version 2                           */
};


/*
 ******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************
 */


/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */


/*****************************************************************************/
/*! Manage a Wifi Out-Of-Band NDEF message, to start a communication based on Wifi.
 *  The Wifi OOB format is described by the Wifi Protected Setup specification.
 *  It consists in a list of data elements formatted as type-length-value.

    The Wifi OOB in a NDEF record has the following structure:
        - Version
        - Credential
            - Network index
            - SSID
            - Authentication Type
            - Encryption Type
            - Network Key
            - MAC Address
            - Vendor Extension
                - Network Key Shareable
            - Vendor Extension
                - Version2

    Note: If the `Network key` is set to an empty buffer, the library sets it to "0x00000000"
          Even if 0-length Network Key is supposed to be supported, smartphones dont necessarily accept it.
  */


/*****************************************************************************/
static uint32_t NDef_WifiPayloadGetLength(const NDef_Type *wifi)
{
  const NDef_Type_Wifi *wifiData;
  uint32_t payloadLength;

  if ((wifi == NULL) || (wifi->id != NDEF_TYPE_ID_MEDIA_WIFI)) {
    return 0;
  }

  wifiData = &wifi->data.wifi;

  payloadLength = sizeof(wifiConfigToken1)
                  + WIFI_SSID_TYPE_LENGTH    + wifiData->bufNetworkSSID.length
                  + sizeof(wifiConfigToken3)
                  + WIFI_SSID_KEY_TYPE_LENGTH + wifiData->bufNetworkKey.length
                  + sizeof(wifiConfigToken5);

  return payloadLength;
}


/*****************************************************************************/
static const uint8_t *NDef_WifiToPayloadItem(const NDef_Type *wifi, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Wifi *wifiData;
  uint16_t credentialLength;

  uint8_t defaultKey[4] = {0, 0, 0, 0};
  NDef_Const_Buffer_8 bufDefaultNetworkKey = { defaultKey, sizeof(defaultKey) };

  const uint8_t CONFIG_TOKEN_1_CREDENTIAL_LENGTH_INDEX   =  7U;
  const uint8_t CONFIG_TOKEN_3_AUTHENTICATION_TYPE_INDEX =  5U;
  const uint8_t CONFIG_TOKEN_3_ENCRYPTION_TYPE_INDEX     = 11U;

  static uint8_t zero[] = { 0 };
  static NDef_Const_Buffer_8 bufZero = { zero, sizeof(zero) };

  if ((wifi    == NULL) || (wifi->id != NDEF_TYPE_ID_MEDIA_WIFI) ||
      (bufItem == NULL)) {
    return NULL;
  }

  wifiData = &wifi->data.wifi;

  if (begin == true) {
    item = 0;
  }

  bufItem->buffer = NULL;
  bufItem->length = 0;

  switch (item) {
    case 0:
      /* Config Token1 */

      /* Update Token1 with credential length */
      credentialLength = (uint16_t)(5U +                   /* Network index      */
                                    2U +                              /* SSID type          */
                                    2U +                              /* SSID key length    */
                                    wifiData->bufNetworkSSID.length + /* SSID key           */
                                    sizeof(wifiConfigToken3) +        /* Token3 length      */
                                    2U +                              /* Network key length */
                                    wifiData->bufNetworkKey.length +  /* Network key        */
                                    sizeof(wifiConfigToken5));        /* Token5 length      */

      wifiConfigToken1[CONFIG_TOKEN_1_CREDENTIAL_LENGTH_INDEX]      = (uint8_t)(credentialLength >>    8U);
      wifiConfigToken1[CONFIG_TOKEN_1_CREDENTIAL_LENGTH_INDEX + 1U] = (uint8_t)(credentialLength  & 0xFFU);

      bufItem->buffer = wifiConfigToken1;
      bufItem->length = sizeof(wifiConfigToken1);
      break;

    case 1:
      /* SSID Length (1st byte) */
      bufItem->buffer = bufZero.buffer;
      bufItem->length = bufZero.length;
      break;

    case 2:
      /* SSID Length (2nd byte) */
      bufItem->buffer = (const uint8_t *)&wifiData->bufNetworkSSID.length;
      bufItem->length = 1U;
      break;

    case 3:
      /* SSID Value */
      bufItem->buffer = wifiData->bufNetworkSSID.buffer;
      bufItem->length = wifiData->bufNetworkSSID.length;
      break;

    case 4:
      /* Config Token3 */

      /* Update Token3 with Authentication and Encryption Types */
      wifiConfigToken3[CONFIG_TOKEN_3_AUTHENTICATION_TYPE_INDEX] = wifiData->authentication;
      wifiConfigToken3[CONFIG_TOKEN_3_ENCRYPTION_TYPE_INDEX]     = wifiData->encryption;

      bufItem->buffer = wifiConfigToken3;
      bufItem->length = sizeof(wifiConfigToken3);
      break;

    case 5:
      /* SSID Key Length (1st byte) */
      bufItem->buffer = bufZero.buffer;
      bufItem->length = bufZero.length;
      break;

    case 6:
      /* SSID Key Length (2 bytes) */
      bufItem->buffer = (const uint8_t *)&wifiData->bufNetworkKey.length;
      bufItem->length = 1U;
      break;

    case 7:
      /* SSID Key Value */
      if (wifiData->bufNetworkKey.length == 0U) {
        /* Empty network key is not supported by Phones */
        bufItem->buffer = bufDefaultNetworkKey.buffer;
        bufItem->length = bufDefaultNetworkKey.length;
      } else {
        bufItem->buffer = wifiData->bufNetworkKey.buffer;
        bufItem->length = wifiData->bufNetworkKey.length;
      }
      break;

    case 8:
      /* Config Token5 */
      bufItem->buffer = wifiConfigToken5;
      bufItem->length = sizeof(wifiConfigToken5);
      break;

    default:
      bufItem->buffer = NULL;
      bufItem->length = 0;
      break;
  }

  /* Move to next item for next call */
  item++;

  return bufItem->buffer;
}


/*****************************************************************************/
NFC_OpResult NDef_WifiInit(NDef_Type *wifi, const NDef_Type_Wifi *wifiConfig)
{
  NDef_Type_Wifi *wifiData;

  if ((wifi == NULL) || (wifiConfig == NULL)) {
    return NFC_InvalidParameter;
  }

  wifi->id               = NDEF_TYPE_ID_MEDIA_WIFI;
  wifi->getPayloadLength = NDef_WifiPayloadGetLength;
  wifi->getPayloadItem   = NDef_WifiToPayloadItem;
  wifi->typeToRecord     = NDef_WifiToRecord;
  wifiData               = &wifi->data.wifi;

  wifiData->bufNetworkSSID = wifiConfig->bufNetworkSSID;
  wifiData->bufNetworkKey  = wifiConfig->bufNetworkKey;
  wifiData->authentication = wifiConfig->authentication;
  wifiData->encryption     = wifiConfig->encryption;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetWifi(const NDef_Type *wifi, NDef_Type_Wifi *wifiConfig)
{
  const NDef_Type_Wifi *wifiData;

  if ((wifi       == NULL) || (wifi->id != NDEF_TYPE_ID_MEDIA_WIFI) ||
      (wifiConfig == NULL)) {
    return NFC_InvalidParameter;
  }

  wifiData = &wifi->data.wifi;

  wifiConfig->bufNetworkSSID.buffer = wifiData->bufNetworkSSID.buffer;
  wifiConfig->bufNetworkSSID.length = wifiData->bufNetworkSSID.length;
  wifiConfig->bufNetworkKey.buffer  = wifiData->bufNetworkKey.buffer;
  wifiConfig->bufNetworkKey.length  = wifiData->bufNetworkKey.length;
  wifiConfig->authentication = wifiData->authentication;
  wifiConfig->encryption     = wifiData->encryption;

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToWifi(const NDef_Const_Buffer *bufPayload, NDef_Type *wifi)
{
  NDef_Type_Wifi wifiConfig;
  uint32_t offset;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (wifi       == NULL)) {
    return NFC_InvalidParameter;
  }

  wifiConfig.bufNetworkSSID.buffer = NULL;
  wifiConfig.bufNetworkSSID.length = 0;
  wifiConfig.bufNetworkKey.buffer  = NULL;
  wifiConfig.bufNetworkKey.length  = 0;
  wifiConfig.authentication        = 0;
  wifiConfig.encryption            = 0;

  offset = 0;
  while (offset < bufPayload->length) {
    uint8_t attribute = bufPayload->buffer[offset];
    if (attribute == NDEF_WIFI_ATTRIBUTE_ID_SSID_LSB) {
      uint8_t data1   = bufPayload->buffer[offset + NDEF_WIFI_ATTRIBUTE_LENGTH_MSB_OFFSET];
      uint8_t data2   = bufPayload->buffer[offset + NDEF_WIFI_ATTRIBUTE_LENGTH_LSB_OFFSET];
      uint32_t length = ((uint32_t)data1 << 8U) | data2;

      switch (bufPayload->buffer[offset + NDEF_WIFI_ATTRIBUTE_ID_OFFSET]) {
        case NDEF_WIFI_ATTRIBUTE_ID_SSID_MSB:
          /* Network SSID */
          if (length > NDEF_WIFI_NETWORK_SSID_LENGTH) {
            return NFC_ProtocolError;
          }
          wifiConfig.bufNetworkSSID.buffer = &bufPayload->buffer[offset + NDEF_WIFI_ATTRIBUTE_DATA_OFFSET];
          wifiConfig.bufNetworkSSID.length = length;
          offset += (NDEF_WIFI_ATTRIBUTE_DATA_OFFSET + length);
          break;
        case NDEF_WIFI_ATTRIBUTE_ID_NETWORK_MSB:
          /* Network key */
          if (length > NDEF_WIFI_NETWORK_KEY_LENGTH) {
            return NFC_ProtocolError;
          }
          wifiConfig.bufNetworkKey.buffer = &bufPayload->buffer[offset + NDEF_WIFI_ATTRIBUTE_DATA_OFFSET];
          wifiConfig.bufNetworkKey.length = length;
          offset += (NDEF_WIFI_ATTRIBUTE_DATA_OFFSET + length);
          break;
        case NDEF_WIFI_ATTRIBUTE_AUTHENTICATION:
          /* Authentication */
          if (length != NDEF_WIFI_AUTHENTICATION_TYPE_LENGTH) {
            return NFC_ProtocolError;
          }
          wifiConfig.authentication = bufPayload->buffer[offset + NDEF_WIFI_ATTRIBUTE_AUTHENTICATION_LSB_OFFSET];
          offset += (NDEF_WIFI_ATTRIBUTE_DATA_OFFSET + length);
          break;
        case NDEF_WIFI_ATTRIBUTE_ENCRYPTION:
          /* Encryption */
          if (length != NDEF_WIFI_ENCRYPTION_TYPE_LENGTH) {
            return NFC_ProtocolError;
          }
          wifiConfig.encryption = bufPayload->buffer[offset + NDEF_WIFI_ATTRIBUTE_ENCRYPTION_LSB_OFFSET];
          offset += (NDEF_WIFI_ATTRIBUTE_DATA_OFFSET + length);
          break;
        default:
          offset++;
          break;
      }
    } else {
      offset++;
    }
  }

  return NDef_WifiInit(wifi, &wifiConfig);
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToWifi(const NDef_Record *record, NDef_Type *wifi)
{
  const NDef_Type *type;

  if ((record == NULL) || (wifi == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_RecordTypeMatch(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeWifi)) { /* "application/vnd.wfa.wsc" */
    return NFC_ProtocolError;
  }

  type = NDef_RecordGetNdefType(record);
  if ((type != NULL) && (type->id == NDEF_TYPE_ID_MEDIA_WIFI)) {
    (void)memcpy(wifi, type, sizeof(NDef_Type));
    return NFC_OK;
  }

  return NDef_PayloadToWifi(&record->bufPayload, wifi);
}


/*****************************************************************************/
NFC_OpResult NDef_WifiToRecord(const NDef_Type *wifi, NDef_Record *record)
{
  if ((wifi   == NULL) || (wifi->id != NDEF_TYPE_ID_MEDIA_WIFI) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_RecordReset(record);

  (void)NDef_RecordSetType(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeWifi);

  if (NDef_RecordSetNdefType(record, wifi) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
