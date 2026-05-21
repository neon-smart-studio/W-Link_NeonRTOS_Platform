
/**
  ******************************************************************************
  * @file           : ndef_record.cpp
  * @brief          : NDEF record
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
#include "NDef_Message.h"
#include "NDef_Types.h"

#include "NFC/NFC_Def.h"

#define NDef_Buffer_IsInvalid(buf)     (((buf)->buffer == NULL) && ((buf)->length != 0U))

/*****************************************************************************/
NFC_OpResult NDef_Record_Reset(NDef_Record *record)
{
  NDef_Const_Buffer_8 bufEmpty8 = { NULL, 0 };
  NDef_Const_Buffer  bufEmpty  = { NULL, 0 };

  if (record == NULL) {
    return NFC_InvalidParameter;
  }

  /* Set the MB and ME bits */
  record->header = NDef_Header(1U, 1U, 0U, 0U, 0U, NDEF_TNF_EMPTY);

  (void)NDef_Record_SetType(record, NDEF_TNF_EMPTY, &bufEmpty8);

  (void)NDef_Record_SetId(record, &bufEmpty8);

  /* Set the SR bit */
  (void)NDef_Record_SetPayload(record, &bufEmpty);

  record->ndeftype = NULL;

  record->next = NULL;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_Init(NDef_Record *record, uint8_t tnf, const NDef_Const_Buffer_8 *bufType, const NDef_Const_Buffer_8 *bufId, const NDef_Const_Buffer *bufPayload)
{
  if (record == NULL) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  (void)NDef_Record_SetType(record, tnf, bufType);

  (void)NDef_Record_SetId(record, bufId);

  (void)NDef_Record_SetPayload(record, bufPayload);

  return NFC_OK;
}


/*****************************************************************************/
uint32_t NDef_Record_GetHeaderLength(const NDef_Record *record)
{
  uint32_t length;

  if (record == NULL) {
    return 0;
  }

  length  = sizeof(uint8_t);      /* header (MB:1 + ME:1 + CF:1 + SR:1 + IL:1 + TNF:3 => 8 bits) */
  length += sizeof(uint8_t);      /* Type length */
  if (NDef_Header_IsSetSR(record)) {
    length += sizeof(uint8_t);  /* Short record */
  } else {
    length += sizeof(uint32_t); /* Standard record */
  }
  if (NDef_Header_IsSetIL(record)) {
    length += sizeof(uint8_t);  /* Id length */
  }
  length += record->typeLength;   /* Type */
  length += record->idLength;     /* Id */

  return length;
}


/*****************************************************************************/
uint32_t NDef_Record_GetLength(const NDef_Record *record)
{
  uint32_t length;

  length  = NDef_Record_GetHeaderLength(record);  /* Header */
  length += NDef_Record_GetPayloadLength(record); /* Payload */

  return length;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_SetType(NDef_Record *record, uint8_t tnf, const NDef_Const_Buffer_8 *bufType)
{
  if ((record  == NULL) ||
      (bufType == NULL) || NDef_Buffer_IsInvalid(bufType)) {
    return NFC_InvalidParameter;
  }

  NDef_Header_SetTNF(record, tnf);

  record->typeLength = bufType->length;
  record->type       = bufType->buffer;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_GetType(const NDef_Record *record, uint8_t *tnf, NDef_Const_Buffer_8 *bufType)
{
  /* Allow to get either tnf or bufType */
  if ((record == NULL) || ((tnf == NULL) && (bufType == NULL))) {
    return NFC_InvalidParameter;
  }

  if (tnf != NULL) {
    *tnf            = NDef_Header_TNF(record);
  }

  if (bufType != NULL) {
    bufType->buffer = record->type;
    bufType->length = record->typeLength;
  }

  return NFC_OK;
}


/*****************************************************************************/
bool NDef_Record_TypeMatch(const NDef_Record *record, uint8_t tnf, const NDef_Const_Buffer_8 *bufType)
{
  if ((record == NULL) || (bufType == NULL)) {
    return false;
  }

  if ((NDef_Header_TNF(record) == tnf)             &&
      (record->typeLength    == bufType->length) &&
      (memcmp(record->type, bufType->buffer, bufType->length) == 0)) {
    return true;
  }

  return false;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_SetId(NDef_Record *record, const NDef_Const_Buffer_8 *bufId)
{
  if ((record == NULL) ||
      (bufId  == NULL) || NDef_Buffer_IsInvalid(bufId)) {
    return NFC_InvalidParameter;
  }

  if (bufId->buffer != NULL) {
    NDef_Header_SetIL(record);
  } else {
    NDef_Header_ClearIL(record);
  }

  record->id       = bufId->buffer;
  record->idLength = bufId->length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_GetId(const NDef_Record *record, NDef_Const_Buffer_8 *bufId)
{
  if ((record == NULL) || (bufId == NULL)) {
    return NFC_InvalidParameter;
  }

  bufId->buffer = record->id;
  bufId->length = record->idLength;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_SetPayload(NDef_Record *record, const NDef_Const_Buffer *bufPayload)
{
  if ((record     == NULL) ||
      (bufPayload == NULL) || NDef_Buffer_IsInvalid(bufPayload)) {
    return NFC_InvalidParameter;
  }

  NDef_Header_SetValueSR(record, (bufPayload->length <= NDEF_SHORT_RECORD_LENGTH_MAX) ? 1 : 0);

  record->bufPayload.buffer = bufPayload->buffer;
  record->bufPayload.length = bufPayload->length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_GetPayload(const NDef_Record *record, NDef_Const_Buffer *bufPayload)
{
  if ((record == NULL) || (bufPayload == NULL)) {
    return NFC_InvalidParameter;
  }

  bufPayload->buffer = record->bufPayload.buffer;
  bufPayload->length = record->bufPayload.length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_Decode(const NDef_Const_Buffer *bufPayload, NDef_Record *record)
{
  uint32_t offset;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) || (record == NULL)) {
    return NFC_InvalidParameter;
  }

  if (NDef_Record_Reset(record) != NFC_OK) {
    return NFC_InternalError;
  }

  /* Get "header" byte */
  offset = 0;
  if ((offset + sizeof(uint8_t)) > bufPayload->length) {
    return NFC_ProtocolError;
  }
  record->header = bufPayload->buffer[offset];
  offset++;

  /* Get Type length */
  if ((offset + sizeof(uint8_t)) > bufPayload->length) {
    return NFC_ProtocolError;
  }
  record->typeLength = bufPayload->buffer[offset];
  offset++;

  /* Decode Payload length */
  if (NDef_Header_IsSetSR(record)) {
    /* Short record */
    if ((offset + sizeof(uint8_t)) > bufPayload->length) {
      return NFC_ProtocolError;
    }
    record->bufPayload.length = bufPayload->buffer[offset]; /* length stored on a single byte for Short Record */
    offset++;
  } else {
    /* Standard record */
    if ((offset + sizeof(uint32_t)) > bufPayload->length) {
      return NFC_ProtocolError;
    }
    record->bufPayload.length = (((uint32_t)(&bufPayload->buffer[offset])[0] << 24) | ((uint32_t)(&bufPayload->buffer[offset])[1] << 16) | ((uint32_t)(&bufPayload->buffer[offset])[2] << 8) | ((uint32_t)(&bufPayload->buffer[offset])[3]));
    offset += sizeof(uint32_t);
  }

  /* Get Id length */
  if (NDef_Header_IsSetIL(record)) {
    if ((offset + sizeof(uint8_t)) > bufPayload->length) {
      return NFC_ProtocolError;
    }
    record->idLength = bufPayload->buffer[offset];
    offset++;
  } else {
    record->idLength = 0;
  }

  /* Get Type */
  if (record->typeLength > 0U) {
    if ((offset + record->typeLength) > bufPayload->length) {
      return NFC_ProtocolError;
    }
    record->type = &bufPayload->buffer[offset];
    offset += record->typeLength;
  } else {
    record->type = NULL;
  }

  /* Get Id */
  if (record->idLength > 0U) {
    if ((offset + record->idLength) > bufPayload->length) {
      return NFC_ProtocolError;
    }
    record->id = &bufPayload->buffer[offset];
    offset += record->idLength;
  } else {
    record->id = NULL;
  }

  /* Get Payload */
  if (record->bufPayload.length > 0U) {
    if ((offset + record->bufPayload.length) > bufPayload->length) {
      return NFC_ProtocolError;
    }
    record->bufPayload.buffer = &bufPayload->buffer[offset];
  } else {
    record->bufPayload.buffer = NULL;
  }

  record->next = NULL;

  return NFC_OK;
}

/*****************************************************************************/
NFC_OpResult NDef_Record_EncodeHeader(const NDef_Record *record, NDef_Buffer *bufHeader)
{
  uint32_t offset;
  uint32_t payloadLength;

  if ((record == NULL) || (bufHeader == NULL) || (bufHeader->buffer == NULL)) {
    return NFC_InvalidParameter;
  }

  if (bufHeader->length < NDEF_RECORD_HEADER_LEN) {
    bufHeader->length = NDEF_RECORD_HEADER_LEN;
    return NFC_MemoryError;
  }

  /* Start encoding the record */
  offset = 0;
  bufHeader->buffer[offset] = record->header;
  offset++;

  /* Set Type length */
  bufHeader->buffer[offset] = record->typeLength;
  offset++;

  /* Encode Payload length */
  payloadLength = NDef_Record_GetPayloadLength(record);

  if (payloadLength <= NDEF_SHORT_RECORD_LENGTH_MAX) {
    /* Short record */
    bufHeader->buffer[offset] = (uint8_t)payloadLength;
    offset++;
  } else {
    /* Standard record */
    bufHeader->buffer[offset] = (uint8_t)(payloadLength >> 24);
    offset++;
    bufHeader->buffer[offset] = (uint8_t)(payloadLength >> 16);
    offset++;
    bufHeader->buffer[offset] = (uint8_t)(payloadLength >> 8);
    offset++;
    bufHeader->buffer[offset] = (uint8_t)(payloadLength);
    offset++;
  }

  /* Encode Id length */
  if (NDef_Header_IsSetIL(record)) {
    bufHeader->buffer[offset] = record->idLength;
    offset++;
  }

  bufHeader->length = offset;

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_Record_EncodePayload(const NDef_Record *record, NDef_Buffer *bufPayload)
{
  uint32_t payloadLength;
  uint32_t offset;
  bool     begin;
  NDef_Const_Buffer bufPayloadItem;

  if (bufPayload == NULL) {
    return NFC_InvalidParameter;
  }

  payloadLength = NDef_Record_GetPayloadLength(record);
  if (payloadLength > bufPayload->length) {
    return NFC_MemoryError;
  }

  begin  = true;
  offset = 0;
  while (NDef_Record_GetPayloadItem(record, &bufPayloadItem, begin) != NULL) {
    begin = false;
    if (bufPayloadItem.length > 0U) {
      memcpy(&bufPayload->buffer[offset], bufPayloadItem.buffer, bufPayloadItem.length);
    }
    offset += bufPayloadItem.length;
  }

  bufPayload->length = offset;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_Encode(const NDef_Record *record, NDef_Buffer *bufRecord)
{
  NFC_OpResult err;
  NDef_Buffer bufHeader;
  NDef_Buffer bufPayload;
  uint32_t   offset;

  if ((record == NULL) || (bufRecord == NULL) || (bufRecord->buffer == NULL)) {
    return NFC_InvalidParameter;
  }

  if (bufRecord->length < NDef_Record_GetLength(record)) {
    bufRecord->length = NDef_Record_GetLength(record);
    return NFC_MemoryError;
  }

  /* Encode header at the beginning of buffer provided */
  bufHeader = *bufRecord; /* Copy NDef_Buffer fields */
  err = NDef_Record_EncodeHeader(record, &bufHeader);
  if (err != NFC_OK) {
    return err;
  }

  offset = bufHeader.length;

  /* Set Type */
  if (record->typeLength > 0U) {
    (void)memcpy(&bufRecord->buffer[offset], record->type, record->typeLength);
    offset += record->typeLength;
  }

  /* Set Id */
  if (record->idLength > 0U) {
    (void)memcpy(&bufRecord->buffer[offset], record->id, record->idLength);
    offset += record->idLength;
  }

  /* Set Payload */
  bufPayload.buffer = &bufRecord->buffer[offset];
  bufPayload.length =  bufRecord->length - offset;
  err = NDef_Record_EncodePayload(record, &bufPayload);
  if (err != NFC_OK) {
    return err;
  }

  bufRecord->length = offset + bufPayload.length;

  return NFC_OK;
}

/*****************************************************************************/
uint32_t NDef_Record_GetPayloadLength(const NDef_Record *record)
{
  uint32_t payloadLength;

  if (record == NULL) {
    return 0;
  }

  if ((record->ndeftype != NULL) && (record->ndeftype->getPayloadLength != NULL)) {
    payloadLength = record->ndeftype->getPayloadLength(record->ndeftype);
  } else {
    payloadLength = record->bufPayload.length;
  }

  return payloadLength;
}

/*****************************************************************************/
const uint8_t *NDef_Record_GetPayloadItem(const NDef_Record *record, NDef_Const_Buffer *bufPayloadItem, bool begin)
{
  if ((record == NULL) || (bufPayloadItem == NULL)) {
    return NULL;
  }

  bufPayloadItem->buffer = NULL;
  bufPayloadItem->length = 0;

  if ((record->ndeftype != NULL) && (record->ndeftype->getPayloadItem != NULL)) {
    record->ndeftype->getPayloadItem(record->ndeftype, bufPayloadItem, begin);
  } else {
    if (begin == true) {
      (void)NDef_Record_GetPayload(record, bufPayloadItem);
    }
  }

  return bufPayloadItem->buffer;
}
