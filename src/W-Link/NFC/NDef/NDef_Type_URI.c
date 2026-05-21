
/**
  ******************************************************************************
  * @file           : NDef__type_uri.cpp
  * @brief          : NDEF RTD URI type
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
#include "NDef_Type_URI.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_RTD_URI_SUPPORT

/*! URI defines */
#define NDEF_RTD_URI_PROTOCOL_LEN        1U                        /*!< URI protocol length */
#define NDEF_RTD_URI_PAYLOAD_LENGTH_MIN  (NDEF_RTD_URI_PROTOCOL_LEN + sizeof(uint8_t)) /*!< URI minimum payload length */

/*! URI defines */
#define NDEF_RTD_URI_ID_CODE_OFFSET      0U    /*!< URI Id code offset */
#define NDEF_RTD_URI_FIELD_OFFSET        1U    /*!< URI field offset */

/*! RTD URI Type string */
static const uint8_t NDef_RtdTypeURI[]            = "U";               /*!< URI Record Type                {0x55}       */

const NDef_Const_Buffer_8 bufRtdTypeURI             = { NDef_RtdTypeURI,        sizeof(NDef_RtdTypeURI) - 1U };        /*!< URI Record Type buffer                */


/*! URI Type strings */
static const uint8_t NDef_URIPrefixNone[]         = "";
static const uint8_t NDef_URIPrefixHttpWww[]      = "http://www.";
static const uint8_t NDef_URIPrefixHttpsWww[]     = "https://www.";
static const uint8_t NDef_URIPrefixHttp[]         = "http://";
static const uint8_t NDef_URIPrefixHttps[]        = "https://";
static const uint8_t NDef_URIPrefixTel[]          = "tel:";
static const uint8_t NDef_URIPrefixMailto[]       = "mailto:";
static const uint8_t NDef_URIPrefixFtpAnonymous[] = "ftp://anonymous:anonymous@";
static const uint8_t NDef_URIPrefixFtpFtp[]       = "ftp://ftp.";
static const uint8_t NDef_URIPrefixFtps[]         = "ftps://";
static const uint8_t NDef_URIPrefixSftp[]         = "sftp://";
static const uint8_t NDef_URIPrefixSmb[]          = "smb://";
static const uint8_t NDef_URIPrefixNfs[]          = "nfs://";
static const uint8_t NDef_URIPrefixFtp[]          = "ftp://";
static const uint8_t NDef_URIPrefixDav[]          = "dav://";
static const uint8_t NDef_URIPrefixNews[]         = "news:";
static const uint8_t NDef_URIPrefixTelnet[]       = "telnet://";
static const uint8_t NDef_URIPrefixImap[]         = "imap:";
static const uint8_t NDef_URIPrefixRtsp[]         = "rtsp://";
static const uint8_t NDef_URIPrefixUrn[]          = "urn:";
static const uint8_t NDef_URIPrefixPop[]          = "pop:";
static const uint8_t NDef_URIPrefixSip[]          = "sip:";
static const uint8_t NDef_URIPrefixSips[]         = "sips:";
static const uint8_t NDef_URIPrefixTftp[]         = "tftp:";
static const uint8_t NDef_URIPrefixBtspp[]        = "btspp://";
static const uint8_t NDef_URIPrefixBtl2cap[]      = "btl2cap://";
static const uint8_t NDef_URIPrefixBtgoep[]       = "btgoep://";
static const uint8_t NDef_URIPrefixTcpobex[]      = "tcpobex://";
static const uint8_t NDef_URIPrefixIrdaobex[]     = "irdaobex://";
static const uint8_t NDef_URIPrefixFile[]         = "file://";
static const uint8_t NDef_URIPrefixUrnEpcId[]     = "urn:epc:id:";
static const uint8_t NDef_URIPrefixUrnEpcTag[]    = "urn:epc:tag";
static const uint8_t NDef_URIPrefixUrnEpcPat[]    = "urn:epc:pat:";
static const uint8_t NDef_URIPrefixUrnEpcRaw[]    = "urn:epc:raw:";
static const uint8_t NDef_URIPrefixUrnEpe[]       = "urn:epc:";
static const uint8_t NDef_URIPrefixUrnNfc[]       = "urn:nfc:";
static const uint8_t NDef_URIPrefixEmpty[]        = ""; /* Autodetect filler */

static const NDef_Const_Buffer NDef_URIPrefix[NDEF_URI_PREFIX_COUNT] = {
  { NDef_URIPrefixNone, sizeof(NDef_URIPrefixNone) - 1U },
  { NDef_URIPrefixHttpWww, sizeof(NDef_URIPrefixHttpWww) - 1U },
  { NDef_URIPrefixHttpsWww, sizeof(NDef_URIPrefixHttpsWww) - 1U },
  { NDef_URIPrefixHttp, sizeof(NDef_URIPrefixHttp) - 1U },
  { NDef_URIPrefixHttps, sizeof(NDef_URIPrefixHttps) - 1U },
  { NDef_URIPrefixTel, sizeof(NDef_URIPrefixTel) - 1U },
  { NDef_URIPrefixMailto, sizeof(NDef_URIPrefixMailto) - 1U },
  { NDef_URIPrefixFtpAnonymous, sizeof(NDef_URIPrefixFtpAnonymous) - 1U },
  { NDef_URIPrefixFtpFtp, sizeof(NDef_URIPrefixFtpFtp) - 1U },
  { NDef_URIPrefixFtps, sizeof(NDef_URIPrefixFtps) - 1U },
  { NDef_URIPrefixSftp, sizeof(NDef_URIPrefixSftp) - 1U },
  { NDef_URIPrefixSmb, sizeof(NDef_URIPrefixSmb) - 1U },
  { NDef_URIPrefixNfs, sizeof(NDef_URIPrefixNfs) - 1U },
  { NDef_URIPrefixFtp, sizeof(NDef_URIPrefixFtp) - 1U },
  { NDef_URIPrefixDav, sizeof(NDef_URIPrefixDav) - 1U },
  { NDef_URIPrefixNews, sizeof(NDef_URIPrefixNews) - 1U },
  { NDef_URIPrefixTelnet, sizeof(NDef_URIPrefixTelnet) - 1U },
  { NDef_URIPrefixImap, sizeof(NDef_URIPrefixImap) - 1U },
  { NDef_URIPrefixRtsp, sizeof(NDef_URIPrefixRtsp) - 1U },
  { NDef_URIPrefixUrn, sizeof(NDef_URIPrefixUrn) - 1U },
  { NDef_URIPrefixPop, sizeof(NDef_URIPrefixPop) - 1U },
  { NDef_URIPrefixSip, sizeof(NDef_URIPrefixSip) - 1U },
  { NDef_URIPrefixSips, sizeof(NDef_URIPrefixSips) - 1U },
  { NDef_URIPrefixTftp, sizeof(NDef_URIPrefixTftp) - 1U },
  { NDef_URIPrefixBtspp, sizeof(NDef_URIPrefixBtspp) - 1U },
  { NDef_URIPrefixBtl2cap, sizeof(NDef_URIPrefixBtl2cap) - 1U },
  { NDef_URIPrefixBtgoep, sizeof(NDef_URIPrefixBtgoep) - 1U },
  { NDef_URIPrefixTcpobex, sizeof(NDef_URIPrefixTcpobex) - 1U },
  { NDef_URIPrefixIrdaobex, sizeof(NDef_URIPrefixIrdaobex) - 1U },
  { NDef_URIPrefixFile, sizeof(NDef_URIPrefixFile) - 1U },
  { NDef_URIPrefixUrnEpcId, sizeof(NDef_URIPrefixUrnEpcId) - 1U },
  { NDef_URIPrefixUrnEpcTag, sizeof(NDef_URIPrefixUrnEpcTag) - 1U },
  { NDef_URIPrefixUrnEpcPat, sizeof(NDef_URIPrefixUrnEpcPat) - 1U },
  { NDef_URIPrefixUrnEpcRaw, sizeof(NDef_URIPrefixUrnEpcRaw) - 1U },
  { NDef_URIPrefixUrnEpe, sizeof(NDef_URIPrefixUrnEpe) - 1U },
  { NDef_URIPrefixUrnNfc, sizeof(NDef_URIPrefixUrnNfc) - 1U },
  { NDef_URIPrefixEmpty, sizeof(NDef_URIPrefixEmpty) - 1U }
};

/*****************************************************************************/
static uint32_t NDef_RtdURIPayloadGetLength(const NDef_Type *uri)
{
  const NDef_Type_Rtd_URI *rtdURI;

  if ((uri == NULL) || (uri->id != NDEF_TYPE_ID_RTD_URI)) {
    return 0;
  }

  rtdURI = &uri->data.uri;

  return sizeof(rtdURI->protocol) + rtdURI->bufURIString.length;
}


/*****************************************************************************/
static const uint8_t *NDef_RtdURIToPayloadItem(const NDef_Type *uri, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Rtd_URI *rtdURI;

  if ((uri     == NULL) || (uri->id != NDEF_TYPE_ID_RTD_URI) ||
      (bufItem == NULL)) {
    return NULL;
  }

  rtdURI = &uri->data.uri;

  if (begin == true) {
    item = 0;
  }

  switch (item) {
    case 0:
      /* Protocol byte */
      bufItem->buffer = &rtdURI->protocol;
      bufItem->length = sizeof(rtdURI->protocol);
      break;

    case 1:
      /* URI string */
      bufItem->buffer = rtdURI->bufURIString.buffer;
      bufItem->length = rtdURI->bufURIString.length;
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
static NFC_OpResult NDef_RtdURIProtocolAutodetect(uint8_t *protocol, NDef_Const_Buffer *bufURIString)
{
  if ((protocol  == NULL)                       ||
      (*protocol != NDEF_URI_PREFIX_AUTODETECT) ||
      (bufURIString == NULL)) {
    return NFC_InvalidParameter;
  }

  for (uint8_t i = 0; i < NDEF_URI_PREFIX_COUNT; i++) { /* Protocol fits in 1 byte => uint8_t */
    if (NDef_URIPrefix[i].length > 0U) {
      if (memcmp(bufURIString->buffer, NDef_URIPrefix[i].buffer, NDef_URIPrefix[i].length) == 0) {
        *protocol = i;
        /* Move after the protocol string */
        bufURIString->buffer  = &bufURIString->buffer[NDef_URIPrefix[i].length];
        bufURIString->length -= NDef_URIPrefix[i].length;
        return NFC_OK;
      }
    }
  }

  *protocol = NDEF_URI_PREFIX_NONE;

  return NFC_NotFound;
}


/*****************************************************************************/
NFC_OpResult NDef_RtdURIInit(NDef_Type *uri, uint8_t protocol, const NDef_Const_Buffer *bufURIString)
{
  NDef_Type_Rtd_URI *rtdURI;
  NDef_Const_Buffer bufURI;
  uint8_t protocolDetect;

  if ((uri == NULL) || (protocol >= NDEF_URI_PREFIX_COUNT) ||
      (bufURIString == NULL) || (bufURIString->buffer == NULL) || (bufURIString->length == 0U)) {
    return NFC_InvalidParameter;
  }

  uri->id               = NDEF_TYPE_ID_RTD_URI;
  uri->getPayloadLength = NDef_RtdURIPayloadGetLength;
  uri->getPayloadItem   = NDef_RtdURIToPayloadItem;
  uri->typeToRecord     = NDef_RtdURIToRecord;
  rtdURI                = &uri->data.uri;

  bufURI.buffer = bufURIString->buffer;
  bufURI.length = bufURIString->length;
  protocolDetect = protocol;
  if (protocol == NDEF_URI_PREFIX_AUTODETECT) {
    /* Update protocol and URI buffer */
    (void)NDef_RtdURIProtocolAutodetect(&protocolDetect, &bufURI);
  }
  rtdURI->protocol = protocolDetect;

  rtdURI->bufURIString.buffer = bufURI.buffer;
  rtdURI->bufURIString.length = bufURI.length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdURI(const NDef_Type *uri, NDef_Const_Buffer *bufProtocol, NDef_Const_Buffer *bufURIString)
{
  const NDef_Type_Rtd_URI *rtdURI;

  if ((uri         == NULL) || (uri->id != NDEF_TYPE_ID_RTD_URI) ||
      (bufProtocol == NULL) || (bufURIString == NULL)) {
    return NFC_InvalidParameter;
  }

  rtdURI = &uri->data.uri;

  bufProtocol->buffer   = NDef_URIPrefix[rtdURI->protocol].buffer;
  bufProtocol->length   = NDef_URIPrefix[rtdURI->protocol].length;

  bufURIString->buffer = rtdURI->bufURIString.buffer;
  bufURIString->length = rtdURI->bufURIString.length;

  return NFC_OK;
}

/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdURI(const NDef_Const_Buffer *bufURI, NDef_Type *uri)
{
  uint8_t protocol;

  if ((bufURI == NULL) || (bufURI->buffer == NULL) ||
      (uri    == NULL)) {
    return NFC_InvalidParameter;
  }

  if (bufURI->length < NDEF_RTD_URI_PAYLOAD_LENGTH_MIN) {
    return NFC_ProtocolError;
  }

  /* Extract info from the payload */
  protocol = bufURI->buffer[NDEF_RTD_URI_ID_CODE_OFFSET];

  NDef_Const_Buffer bufStringURI;
  bufStringURI.buffer = &bufURI->buffer[NDEF_RTD_URI_FIELD_OFFSET];
  bufStringURI.length =  bufURI->length - sizeof(protocol);

  return NDef_RtdURIInit(uri, protocol, &bufStringURI);
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdURI(const NDef_Record *record, NDef_Type *uri)
{
  const NDef_Type *type;

  if ((record == NULL) || (uri == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_Record_TypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeURI)) { /* "U" */
    return NFC_ProtocolError;
  }

  type = NDef_RecordGetNDefType(record);
  if ((type != NULL) && (type->id == NDEF_TYPE_ID_RTD_URI)) {
    (void)memcpy(uri, type, sizeof(NDef_Type));
    return NFC_OK;
  }

  return NDef_PayloadToRtdURI(&record->bufPayload, uri);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdURIToRecord(const NDef_Type *uri, NDef_Record *record)
{
  if ((uri    == NULL) || (uri->id != NDEF_TYPE_ID_RTD_URI) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  /* "U" */
  (void)NDef_Record_SetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeURI);

  if (NDef_RecordSetNDefType(record, uri) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
