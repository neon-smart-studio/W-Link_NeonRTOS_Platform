
/**
  ******************************************************************************
  * @file           : ndef_type_empty.cpp
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
#include "NDef_Type_Empty.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_EMPTY_SUPPORT

/*****************************************************************************/
static uint32_t NDef_EmptyTypePayloadGetLength(const NDef_Type *empty)
{
  return 0;
}

/*****************************************************************************/
static const uint8_t *NDef_EmptyTypePayloadItem(const NDef_Type *empty, NDef_Const_Buffer *bufItem, bool begin)
{
  if ((empty == NULL) || (empty->id != NDEF_TYPE_ID_EMPTY)) {
    return NULL;
  }

  if (bufItem != NULL) {
    bufItem->buffer = NULL;
    bufItem->length = 0;
  }

  return NULL;
}


/*****************************************************************************/
NFC_OpResult NDef_EmptyTypeInit(NDef_Type *empty)
{
  if (empty == NULL) {
    return NFC_InvalidParameter;
  }

  empty->id               = NDEF_TYPE_ID_EMPTY;
  empty->getPayloadLength = NDef_EmptyTypePayloadGetLength;
  empty->getPayloadItem   = NDef_EmptyTypePayloadItem;
  empty->typeToRecord     = NDef_EmptyTypeToRecord;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToEmptyType(const NDef_Record *record, NDef_Type *empty)
{
  NDef_Const_Buffer_8 bufEmpty = { NULL, 0 };

  if ((record == NULL) || (empty == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_RecordTypeMatch(record, NDEF_TNF_EMPTY, &bufEmpty)) {
    return NFC_InvalidParameter;
  }

  if ((record->idLength          != 0U) || (record->id                != NULL) ||
      (record->bufPayload.length != 0U) || (record->bufPayload.buffer != NULL)) {
    return NFC_InvalidParameter;
  }

  return NDef_EmptyTypeInit(empty);
}


/*****************************************************************************/
NFC_OpResult NDef_EmptyTypeToRecord(const NDef_Type *empty, NDef_Record *record)
{
  if ((empty  == NULL) || (empty->id != NDEF_TYPE_ID_EMPTY) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_RecordReset(record);

  if (NDef_RecordSetNdefType(record, empty) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
