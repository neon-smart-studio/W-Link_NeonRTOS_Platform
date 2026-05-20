
/**
  ******************************************************************************
  * @file           : ndef_type_uri.h
  * @brief          : NDEF RTD (well-known and external) types header file
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

#ifndef NDEF_TYPE_URI_H
#define NDEF_TYPE_URI_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

/*! RTD Type URI Protocols */
#define NDEF_URI_PREFIX_NONE          0x00U    /*!< No URI Protocol               */
#define NDEF_URI_PREFIX_HTTP_WWW      0x01U    /*!< URI Protocol http://www.      */
#define NDEF_URI_PREFIX_HTTPS_WWW     0x02U    /*!< URI Protocol https://www.     */
#define NDEF_URI_PREFIX_HTTP          0x03U    /*!< URI Protocol http://          */
#define NDEF_URI_PREFIX_HTTPS         0x04U    /*!< URI Protocol https://         */
#define NDEF_URI_PREFIX_TEL           0x05U    /*!< URI Protocol tel:             */
#define NDEF_URI_PREFIX_MAILTO        0x06U    /*!< URI Protocol mailto:          */
#define NDEF_URI_PREFIX_FTP_ANONYMOUS 0x07U    /*!< URI Protocol ftp://anonymous@ */
#define NDEF_URI_PREFIX_FTP_FTP       0x08U    /*!< URI Protocol ftp://ftp.       */
#define NDEF_URI_PREFIX_FTPS          0x09U    /*!< URI Protocol ftps://          */
#define NDEF_URI_PREFIX_SFTP          0x0AU    /*!< URI Protocol sftp://          */
#define NDEF_URI_PREFIX_SMB           0x0BU    /*!< URI Protocol smb://           */
#define NDEF_URI_PREFIX_NFS           0x0CU    /*!< URI Protocol nfs://           */
#define NDEF_URI_PREFIX_FTP           0x0DU    /*!< URI Protocol ftp://           */
#define NDEF_URI_PREFIX_DAV           0x0EU    /*!< URI Protocol dav://           */
#define NDEF_URI_PREFIX_NEWS          0x0FU    /*!< URI Protocol news:            */
#define NDEF_URI_PREFIX_TELNET        0x10U    /*!< URI Protocol telnet://        */
#define NDEF_URI_PREFIX_IMAP          0x11U    /*!< URI Protocol imap:            */
#define NDEF_URI_PREFIX_RTSP          0x12U    /*!< URI Protocol rtsp://          */
#define NDEF_URI_PREFIX_URN           0x13U    /*!< URI Protocol urn:             */
#define NDEF_URI_PREFIX_POP           0x14U    /*!< URI Protocol pop:             */
#define NDEF_URI_PREFIX_SIP           0x15U    /*!< URI Protocol sip:             */
#define NDEF_URI_PREFIX_SIPS          0x16U    /*!< URI Protocol sips:            */
#define NDEF_URI_PREFIX_TFTP          0x17U    /*!< URI Protocol tftp:            */
#define NDEF_URI_PREFIX_BTSPP         0x18U    /*!< URI Protocol btspp://         */
#define NDEF_URI_PREFIX_BTL2CAP       0x19U    /*!< URI Protocol btl2cap://       */
#define NDEF_URI_PREFIX_BTGOEP        0x1AU    /*!< URI Protocol btgoep://        */
#define NDEF_URI_PREFIX_TCPOBEX       0x1BU    /*!< URI Protocol tcpobex://       */
#define NDEF_URI_PREFIX_IRDAOBEX      0x1CU    /*!< URI Protocol irdaobex://      */
#define NDEF_URI_PREFIX_FILE          0x1DU    /*!< URI Protocol file://          */
#define NDEF_URI_PREFIX_URN_EPC_ID    0x1EU    /*!< URI Protocol urn:epc:id:      */
#define NDEF_URI_PREFIX_URN_EPC_TAG   0x1FU    /*!< URI Protocol urn:epc:tag      */
#define NDEF_URI_PREFIX_URN_EPC_PAT   0x20U    /*!< URI Protocol urn:epc:pat:     */
#define NDEF_URI_PREFIX_URN_EPC_RAW   0x21U    /*!< URI Protocol urn:epc:raw:     */
#define NDEF_URI_PREFIX_URN_EPC       0x22U    /*!< URI Protocol urn:epc:         */
#define NDEF_URI_PREFIX_URN_NFC       0x23U    /*!< URI Protocol urn:nfc:         */
#define NDEF_URI_PREFIX_AUTODETECT    0x24U    /*!< ST Protocol Autodetect        */
#define NDEF_URI_PREFIX_COUNT         0x25U    /*!< Number of URI protocols       */

/*! RTD URI Record Type buffer */
extern const NDef_Const_Buffer_8 bufRtdTypeURI;        /*! URI Record Type buffer                              */

/*! RTD Type URI */
typedef struct {
  uint8_t         protocol;     /*!< Protocol Identifier */
  NDef_Const_Buffer bufURIString; /*!< URI string buffer   */
} NDef_Type_Rtd_URI;

NFC_OpResult NDef_RtdURIInit(NDef_Type *uri, uint8_t protocol, const NDef_Const_Buffer *bufURIString);
NFC_OpResult NDef_GetRtdURI(const NDef_Type *uri, NDef_Const_Buffer *bufProtocol, NDef_Const_Buffer *bufURIString);
NFC_OpResult NDef_RecordToRtdURI(const NDef_Record *record, NDef_Type *uri);
NFC_OpResult NDef_RtdURIToRecord(const NDef_Type *uri, NDef_Record *record);

#endif /* NDEF_TYPE_URI_H */