/**
  ******************************************************************************
  * @file           : ndef_types.h
  * @brief          : Common NDEF RTD (well-known and external) and Media types header file
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

#ifndef NDEF_TYPES_H
#define NDEF_TYPES_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC_Config.h"

/*
 * ============================================================================
 * Core
 * ============================================================================
 */

#ifndef CONFIG_NDEF_SUPPORT
#define NDEF_SUPPORT false
#else
#define NDEF_SUPPORT CONFIG_NDEF_SUPPORT
#endif

#ifndef CONFIG_NDEF_PARSER_SUPPORT
#define NDEF_PARSER_SUPPORT false
#else
#define NDEF_PARSER_SUPPORT CONFIG_NDEF_PARSER_SUPPORT
#endif

#ifndef CONFIG_NDEF_MESSAGE_SUPPORT
#define NDEF_MESSAGE_SUPPORT false
#else
#define NDEF_MESSAGE_SUPPORT CONFIG_NDEF_MESSAGE_SUPPORT
#endif

#ifndef CONFIG_NDEF_RECORD_SUPPORT
#define NDEF_RECORD_SUPPORT false
#else
#define NDEF_RECORD_SUPPORT CONFIG_NDEF_RECORD_SUPPORT
#endif

#ifndef CONFIG_NDEF_TLV_SUPPORT
#define NDEF_TLV_SUPPORT false
#else
#define NDEF_TLV_SUPPORT CONFIG_NDEF_TLV_SUPPORT
#endif

/*
 * ============================================================================
 * RTD Types
 * ============================================================================
 */

#ifndef CONFIG_NFC_TYPE_EMPTY_SUPPORT
#define NDEF_TYPE_EMPTY_SUPPORT false
#else
#define NDEF_TYPE_EMPTY_SUPPORT CONFIG_NFC_TYPE_EMPTY_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_FLAT_SUPPORT
#define NDEF_TYPE_FLAT_SUPPORT false
#else
#define NDEF_TYPE_FLAT_SUPPORT CONFIG_NFC_TYPE_FLAT_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_RTD_DEVICE_INFO_SUPPORT
#define NDEF_TYPE_RTD_DEVICE_INFO_SUPPORT false
#else
#define NDEF_TYPE_RTD_DEVICE_INFO_SUPPORT CONFIG_NFC_TYPE_RTD_DEVICE_INFO_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_RTD_TEXT_SUPPORT
#define NDEF_TYPE_RTD_TEXT_SUPPORT false
#else
#define NDEF_TYPE_RTD_TEXT_SUPPORT CONFIG_NFC_TYPE_RTD_TEXT_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_RTD_URI_SUPPORT
#define NDEF_TYPE_RTD_URI_SUPPORT false
#else
#define NDEF_TYPE_RTD_URI_SUPPORT CONFIG_NFC_TYPE_RTD_URI_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_RTD_AAR_SUPPORT
#define NDEF_TYPE_RTD_AAR_SUPPORT false
#else
#define NDEF_TYPE_RTD_AAR_SUPPORT CONFIG_NFC_TYPE_RTD_AAR_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_RTD_WLC_SUPPORT
#define NDEF_TYPE_RTD_WLC_SUPPORT false
#else
#define NDEF_TYPE_RTD_WLC_SUPPORT CONFIG_NFC_TYPE_RTD_WLC_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_RTD_WPCWLC_SUPPORT
#define NDEF_TYPE_RTD_WPCWLC_SUPPORT false
#else
#define NDEF_TYPE_RTD_WPCWLC_SUPPORT CONFIG_NFC_TYPE_RTD_WPCWLC_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_RTD_TNEP_SUPPORT
#define NDEF_TYPE_RTD_TNEP_SUPPORT false
#else
#define NDEF_TYPE_RTD_TNEP_SUPPORT CONFIG_NFC_TYPE_RTD_TNEP_SUPPORT
#endif

/*
 * ============================================================================
 * MIME Types
 * ============================================================================
 */

#ifndef CONFIG_NFC_TYPE_MEDIA_SUPPORT
#define NDEF_TYPE_MEDIA_SUPPORT false
#else
#define NDEF_TYPE_MEDIA_SUPPORT CONFIG_NFC_TYPE_MEDIA_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_BLUETOOTH_SUPPORT
#define NDEF_TYPE_BLUETOOTH_SUPPORT false
#else
#define NDEF_TYPE_BLUETOOTH_SUPPORT CONFIG_NFC_TYPE_BLUETOOTH_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_VCARD_SUPPORT
#define NDEF_TYPE_VCARD_SUPPORT false
#else
#define NDEF_TYPE_VCARD_SUPPORT CONFIG_NFC_TYPE_VCARD_SUPPORT
#endif

#ifndef CONFIG_NFC_TYPE_WIFI_SUPPORT
#define NDEF_TYPE_WIFI_SUPPORT false
#else
#define NDEF_TYPE_WIFI_SUPPORT CONFIG_NFC_TYPE_WIFI_SUPPORT
#endif

/*
 * ============================================================================
 * Optional Features
 * ============================================================================
 */

#ifndef CONFIG_NDEF_DEBUG_SUPPORT
#define NDEF_DEBUG_SUPPORT false
#else
#define NDEF_DEBUG_SUPPORT CONFIG_NDEF_DEBUG_SUPPORT
#endif

#ifndef CONFIG_NDEF_DYNAMIC_MEMORY_SUPPORT
#define NDEF_DYNAMIC_MEMORY_SUPPORT false
#else
#define NDEF_DYNAMIC_MEMORY_SUPPORT CONFIG_NDEF_DYNAMIC_MEMORY_SUPPORT
#endif

/* RTD types */
#if NDEF_TYPE_EMPTY_SUPPORT
  #include "NDef_Type_Empty.h"
#endif
#if NDEF_TYPE_FLAT_SUPPORT
  #include "NDef_Type_Flat.h"
#endif
#if NDEF_TYPE_RTD_DEVICE_INFO_SUPPORT
  #include "NDef_Type_DeviceInfo.h"
#endif
#if NDEF_TYPE_RTD_TEXT_SUPPORT
  #include "NDef_Type_Text.h"
#endif
#if NDEF_TYPE_RTD_URI_SUPPORT
  #include "NDef_Type_URI.h"
#endif
#if NDEF_TYPE_RTD_AAR_SUPPORT
  #include "NDef_Type_AAR.h"
#endif
#if NDEF_TYPE_RTD_WLC_SUPPORT
  #include "NDef_Type_WLC.h"
#endif
#if NDEF_TYPE_RTD_WPCWLC_SUPPORT
  #include "NDef_Type_WPCWLC.h"
#endif
#if NDEF_TYPE_RTD_TNEP_SUPPORT
  #include "NDef_Type_Tnep.h"
#endif

/* MIME types */
#if NDEF_TYPE_MEDIA_SUPPORT
  #include "NDef_Type_Media.h"
#endif
#if NDEF_TYPE_BLUETOOTH_SUPPORT
  #include "NDef_Type_Bluetooth.h"
#endif
#if NDEF_TYPE_VCARD_SUPPORT
  #include "NDef_Type_Vcard.h"
#endif
#if NDEF_TYPE_WIFI_SUPPORT
  #include "NDef_Type_WiFi.h"
#endif

/*****************************************************************************/

/*! NDEF Type Id enum */
typedef enum {
  NDEF_TYPE_ID_NONE = 0,
  NDEF_TYPE_ID_FLAT,
  NDEF_TYPE_ID_EMPTY,
  NDEF_TYPE_ID_RTD_DEVICE_INFO,
  NDEF_TYPE_ID_RTD_TEXT,
  NDEF_TYPE_ID_RTD_URI,
  NDEF_TYPE_ID_RTD_AAR,
  NDEF_TYPE_ID_RTD_WLCCAP,
  NDEF_TYPE_ID_RTD_WLCSTAI,
  NDEF_TYPE_ID_RTD_WLCINFO,
  NDEF_TYPE_ID_RTD_WLCCTL,
  NDEF_TYPE_ID_RTD_WPCWLC,
  NDEF_TYPE_ID_RTD_TNEP_SERVICE_PARAMETER,
  NDEF_TYPE_ID_RTD_TNEP_SERVICE_SELECT,
  NDEF_TYPE_ID_RTD_TNEP_STATUS,
  NDEF_TYPE_ID_MEDIA,
  NDEF_TYPE_ID_BLUETOOTH_BREDR,
  NDEF_TYPE_ID_BLUETOOTH_LE,
  NDEF_TYPE_ID_BLUETOOTH_SECURE_BREDR,
  NDEF_TYPE_ID_BLUETOOTH_SECURE_LE,
  NDEF_TYPE_ID_MEDIA_VCARD,
  NDEF_TYPE_ID_MEDIA_WIFI,
  NDEF_TYPE_ID_COUNT        /* Keep this one last */
} NDef_Type_ID;


/*! NDEF abstraction Struct */
typedef struct NDef_Type_Struct {
  NDef_Type_ID      id;                                       /*!< Type Id           */
  uint32_t (*getPayloadLength)(const struct NDef_Type_Struct *type);       /*!< Return payload length, specific to each type */
  const uint8_t *(*getPayloadItem)(const struct NDef_Type_Struct *type, NDef_Const_Buffer *item, bool begin); /*!< Payload Encoder, specific to each type */
  NFC_OpResult(*typeToRecord)(const struct NDef_Type_Struct *type, NDef_Record *record);      /*!< Type to Record convert function */
  union {
#if NDEF_TYPE_FLAT_SUPPORT
    NDef_Const_Buffer           bufPayload;       /*!< Flat/unknown type    */
#endif
#if NDEF_TYPE_RTD_DEVICE_INFO_SUPPORT
    NDef_Type_Rtd_DeviceInfo     deviceInfo;       /*!< Device Information   */
#endif
#if NDEF_TYPE_RTD_TEXT_SUPPORT
    NDef_Type_Rtd_Text           text;             /*!< Text                 */
#endif
#if NDEF_TYPE_RTD_URI_SUPPORT
    NDef_Type_Rtd_URI            uri;              /*!< URI                  */
#endif
#if NDEF_TYPE_RTD_AAR_SUPPORT
    NDef_Type_Rtd_AAR            aar;              /*!< AAR                  */
#endif
#if NDEF_TYPE_RTD_WLC_SUPPORT
    NDef_Type_Rtd_WlcCapability  wlcCapability; /*!< WLC Capability          */
    NDef_Type_Rtd_WlcStatusInfo  wlcStatusInfo; /*!< WLC Status and Info     */
    NDef_Type_Rtd_WlcPollInfo    wlcPollInfo;   /*!< WLC Poll Information    */
    NDef_Type_Rtd_WlcListenCtl   wlcListenCtl;  /*!< WLC Listen Control      */
#endif
#if NDEF_TYPE_RTD_WPCWLC_SUPPORT
    NDef_Type_Rtd_WpcWlc         wpcWlc;           /*!< WPC WLC              */
#endif
#if NDEF_TYPE_RTD_TNEP_SUPPORT
    NDef_Type_Rtd_TnepServiceParameter tnepServiceParameter; /*!< TNEP Service Parameter */
    NDef_Type_Rtd_TnepServiceSelect    tnepServiceSelect;    /*!< TNEP Service Select    */
    NDef_Type_Rtd_TnepStatus           tnepStatus;           /*!< TNEP Status            */
#endif
#if NDEF_TYPE_MEDIA_SUPPORT
    NDef_Type_Media             media;            /*!< Media                */
#endif
#if NDEF_TYPE_BLUETOOTH_SUPPORT
    NDef_Type_Bluetooth         bluetooth;        /*!< Bluetooth            */
#endif
#if NDEF_TYPE_VCARD_SUPPORT
    NDef_Type_VCard             vCard;            /*!< vCard                */
#endif
#if NDEF_TYPE_WIFI_SUPPORT
    NDef_Type_Wifi              wifi;             /*!< Wifi                 */
#endif
    uint8_t                   reserved;         /*!< Non-conditional field to avoid empty union when all types are disabled */
  } data;                      /*!< Type data union                         */
};


NFC_OpResult NDef_RecordToType(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_TypeToRecord(const NDef_Type *type, NDef_Record *record);
NFC_OpResult NDef_RecordSetNDefType(NDef_Record *record, const NDef_Type *type);
const NDef_Type *NDef_RecordGetNDefType(const NDef_Record *record);

#endif /* NDEF_TYPES_H */
