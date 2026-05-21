
/**
  ******************************************************************************
  * @file           : ndef_type_wpcwlc.cpp
  * @brief          :NDEF RTD RTD Wireless Power Consortium WLC Record (WPCWLC) type
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
#include "NDef_Type_WPCWLC.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_RTD_WPCWLC_SUPPORT

/*! RTD Type strings */
static const uint8_t ndefRtdTypeWptWlc[] = "www.wirelesspowerconsortium.com:wlc"; /*!< External Type (Wireless Power Consortium WLC  Record)  */

const NDef_Const_Buffer_8 bufRtdTypeWpcWlc  = { ndefRtdTypeWptWlc, sizeof(ndefRtdTypeWptWlc) - 1U };    /*!< WPCWLC External Type Record buffer  */

/*****************************************************************************/
static uint32_t NDef_RtdWpcWlcPayloadGetLength(const NDef_Type *wpcWlc)
{
  const NDef_Type_Rtd_WpcWlc *rtdWpcWlc;

  if ((wpcWlc == NULL) || (wpcWlc->id != NDEF_TYPE_ID_RTD_WPCWLC)) {
    return 0;
  }

  rtdWpcWlc = &wpcWlc->data.wpcWlc;

  return rtdWpcWlc->bufPayload.length;
}


/*****************************************************************************/
static const uint8_t *NDef_RtdWpcWlcToPayloadItem(const NDef_Type *wpcWlc, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Rtd_WpcWlc *rtdWpcWlc;

  if ((wpcWlc  == NULL) || (wpcWlc->id != NDEF_TYPE_ID_RTD_WPCWLC) ||
      (bufItem == NULL)) {
    return NULL;
  }

  rtdWpcWlc = &wpcWlc->data.wpcWlc;

  if (begin == true) {
    item = 0;
  }

  switch (item) {
    case 0:
      /* Ki Payload */
      bufItem->buffer = rtdWpcWlc->bufPayload.buffer;
      bufItem->length = rtdWpcWlc->bufPayload.length;
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
NFC_OpResult NDef_RtdWpcWlcInit(NDef_Type *wpcWlc, const NDef_Const_Buffer *bufPayload)
{
  NDef_Type_Rtd_WpcWlc *rtdWpcWlc;

  if ((wpcWlc == NULL) || (bufPayload == NULL)) {
    return NFC_InvalidParameter;
  }

  wpcWlc->id               = NDEF_TYPE_ID_RTD_WPCWLC;
  wpcWlc->getPayloadLength = NDef_RtdWpcWlcPayloadGetLength;
  wpcWlc->getPayloadItem   = NDef_RtdWpcWlcToPayloadItem;
  wpcWlc->typeToRecord     = NDef_RtdWpcWlcToRecord;
  rtdWpcWlc                = &wpcWlc->data.wpcWlc;

  rtdWpcWlc->bufPayload.buffer = bufPayload->buffer;
  rtdWpcWlc->bufPayload.length = bufPayload->length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdWpcWlc(const NDef_Type *wpcWlc, NDef_Const_Buffer *bufWpcWlc)
{
  const NDef_Type_Rtd_WpcWlc *rtdWpcWlc;

  if ((wpcWlc    == NULL) || (wpcWlc->id != NDEF_TYPE_ID_RTD_WPCWLC) ||
      (bufWpcWlc == NULL)) {
    return NFC_InvalidParameter;
  }

  rtdWpcWlc = &wpcWlc->data.wpcWlc;

  bufWpcWlc->buffer = rtdWpcWlc->bufPayload.buffer;
  bufWpcWlc->length = rtdWpcWlc->bufPayload.length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdWpcWlc(const NDef_Record *record, NDef_Type *wpcWlc)
{
  if ((record == NULL) || (wpcWlc == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_Record_TypeMatch(record, NDEF_TNF_RTD_EXTERNAL_TYPE, &bufRtdTypeWpcWlc)) { /* "www.wirelesspowerconsortium.com:wlc" */
    return NFC_ProtocolError;
  }

  /* No constraint on payload length */

  return NDef_RtdWpcWlcInit(wpcWlc, &record->bufPayload);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdWpcWlcToRecord(const NDef_Type *wpcWlc, NDef_Record *record)
{
  if ((wpcWlc == NULL) || (wpcWlc->id != NDEF_TYPE_ID_RTD_WPCWLC) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  /* "www.wirelesspowerconsortium.com:wlc" */
  (void)NDef_Record_SetType(record, NDEF_TNF_RTD_EXTERNAL_TYPE, &bufRtdTypeWpcWlc);

  if (NDef_RecordSetNDefType(record, wpcWlc) < NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
