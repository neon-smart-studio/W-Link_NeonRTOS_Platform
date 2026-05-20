
/**
  ******************************************************************************
  * @file           : ndef_type_flat.cpp
  * @brief          : NDEF Empty type
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
#include "NDef_Type_Flat.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_FLAT_SUPPORT

/*****************************************************************************/
static uint32_t NDef_FlatPayloadTypePayloadGetLength(const NDef_Type *type)
{
  if ((type == NULL) || (type->id != NDEF_TYPE_ID_FLAT)) {
    return 0;
  }

  return type->data.bufPayload.length;
}


/*****************************************************************************/
static const uint8_t *NDef_FlatPayloadTypePayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  if ((type == NULL) || (type->id != NDEF_TYPE_ID_FLAT)) {
    return NULL;
  }

  if ((begin == true) && (bufItem != NULL)) {
    bufItem->buffer = type->data.bufPayload.buffer;
    bufItem->length = type->data.bufPayload.length;

    return bufItem->buffer;
  }

  return NULL;
}


/*****************************************************************************/
NFC_OpResult NDef_FlatPayloadTypeInit(NDef_Type *type, const NDef_Const_Buffer *bufPayload)
{
  if ((type == NULL) || (bufPayload == NULL)) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_FLAT;
  type->getPayloadLength = NDef_FlatPayloadTypePayloadGetLength;
  type->getPayloadItem   = NDef_FlatPayloadTypePayloadItem;
  type->typeToRecord     = NDef_FlatPayloadTypeToRecord;

  type->data.bufPayload.buffer = bufPayload->buffer;
  type->data.bufPayload.length = bufPayload->length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetFlatPayloadType(const NDef_Type *type, NDef_Const_Buffer *bufPayload)
{
  if ((type     == NULL) || (type->id != NDEF_TYPE_ID_FLAT) ||
      (bufPayload == NULL)) {
    return NFC_InvalidParameter;
  }

  bufPayload->buffer = type->data.bufPayload.buffer ;
  bufPayload->length = type->data.bufPayload.length ;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToFlatPayloadType(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *ndefData;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  ndefData = NDef_RecordGetNdefType(record);
  if ((ndefData != NULL) && (ndefData->id == NDEF_TYPE_ID_FLAT)) {
    (void)memcpy(type, ndefData, sizeof(NDef_Type));
    return NFC_OK;
  }

  NDef_Const_Buffer bufPayload;
  NFC_OpResult err = NDef_RecordGetPayload(record, &bufPayload);
  if (err != NFC_OK) {
    return err;
  }

  return NDef_FlatPayloadTypeInit(type, &bufPayload);
}


/*****************************************************************************/
NFC_OpResult NDef_FlatPayloadTypeToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type   == NULL) || (type->id != NDEF_TYPE_ID_FLAT) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_RecordReset(record);

  /* Do not initialize Type string */

  if (NDef_RecordSetNdefType(record, type) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
