
/**
  ******************************************************************************
  * @file           : NDef__type_aar.cpp
  * @brief          : NDEF RTD Android Application Record (AAR) type
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
#include "NDef_Type_AAR.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_RTD_AAR_SUPPORT

/*! RTD Type strings */
static const uint8_t NDef_RtdTypeAar[]            = "android.com:pkg"; /*!< External Type (Android Application Record)  */

const NDef_Const_Buffer_8 bufRtdTypeAAR          = { NDef_RtdTypeAar,        sizeof(NDef_RtdTypeAar) - 1U };        /*!< AAR External Type Record buffer       */

/*****************************************************************************/
NFC_OpResult NDef_RtdAARInit(NDef_Type *aar, const NDef_Const_Buffer *bufPayload)
{
  NDef_Type_Rtd_AAR *rtdAar;

  if ((aar == NULL) || (bufPayload == NULL)) {
    return NFC_InvalidParameter;
  }

  aar->id               = NDEF_TYPE_ID_RTD_AAR;
  aar->getPayloadLength = NULL;
  aar->getPayloadItem   = NULL;
  aar->typeToRecord     = NDef_RtdAARToRecord;
  rtdAar                = &aar->data.aar;

  rtdAar->bufType.buffer    = bufRtdTypeAAR.buffer;
  rtdAar->bufType.length    = bufRtdTypeAAR.length;
  rtdAar->bufPayload.buffer = bufPayload->buffer;
  rtdAar->bufPayload.length = bufPayload->length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdAAR(const NDef_Type *aar, NDef_Const_Buffer *bufAarString)
{
  const NDef_Type_Rtd_AAR *rtdAar;

  if ((aar          == NULL) || (aar->id != NDEF_TYPE_ID_RTD_AAR) ||
      (bufAarString == NULL)) {
    return NFC_InvalidParameter;
  }

  rtdAar = &aar->data.aar;

  bufAarString->buffer = rtdAar->bufPayload.buffer;
  bufAarString->length = rtdAar->bufPayload.length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdAAR(const NDef_Record *record, NDef_Type *aar)
{
  if ((record == NULL) || (aar == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_RecordTypeMatch(record, NDEF_TNF_RTD_EXTERNAL_TYPE, &bufRtdTypeAAR)) { /* "android.com:pkg" */
    return NFC_ProtocolError;
  }

  /* No constraint on payload length */

  return NDef_RtdAARInit(aar, &record->bufPayload);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdAARToRecord(const NDef_Type *aar, NDef_Record *record)
{
  const NDef_Type_Rtd_AAR *rtdAar;

  if ((aar    == NULL) || (aar->id != NDEF_TYPE_ID_RTD_AAR) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  rtdAar = &aar->data.aar;

  (void)NDef_RecordReset(record);

  /* "android.com:pkg" */
  (void)NDef_RecordSetType(record, NDEF_TNF_RTD_EXTERNAL_TYPE, &bufRtdTypeAAR);

  (void)NDef_RecordSetPayload(record, &rtdAar->bufPayload);

  return NFC_OK;
}

#endif
