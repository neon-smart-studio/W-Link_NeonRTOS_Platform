
/**
  ******************************************************************************
  * @file           : ndef_message.cpp
  * @brief          : NDEF message
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

#include "ndef_record.h"
#include "ndef_message.h"

#include "NFC/NFC_Def.h"

#define NDEF_MAX_RECORD          10U    /*!< Maximum number of records */

static uint8_t NDef_Record_PoolIndex = 0;

/*****************************************************************************/
static NDef_Record *NDefAllocRecord(void)
{
  static NDef_Record NDef_Record_Pool[NDEF_MAX_RECORD];

  if (NDef_Record_PoolIndex >= NDEF_MAX_RECORD) {
    return NULL;
  }

  return &NDef_Record_Pool[NDef_Record_PoolIndex++];
}

NFC_OpResult NDef_Message_Init(NDef_Message *message)
{
  if (message == NULL) {
    return NFC_InvalidParameter;
  }

  message->record           = NULL;
  message->info.length      = 0;
  message->info.recordCount = 0;

  NDef_Record_PoolIndex = 0;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Message_GetInfo(const NDef_Message *message, NDef_Message_Info *info)
{
  NDef_Record *record;
  uint32_t    length      = 0;
  uint32_t    recordCount = 0;

  if ((message == NULL) || (info == NULL)) {
    return NFC_InvalidParameter;
  }

  record = message->record;

  while (record != NULL) {
    length += NDef_Record_GetLength(record);
    recordCount++;

    record = record->next;
  }

  info->length      = length;
  info->recordCount = recordCount;

  return NFC_OK;
}


/*****************************************************************************/
uint32_t NDef_Message_GetRecordCount(const NDef_Message *message)
{
  NDef_Message_Info info;

  if (NDef_Message_GetInfo(message, &info) == NFC_OK) {
    return info.recordCount;
  }

  return 0;
}


/*****************************************************************************/
NFC_OpResult NDef_Message_Append(NDef_Message *message, NDef_Record *record)
{
  if ((message == NULL) || (record == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Clear the Message Begin bit */
  NDef_Header_ClearMB(record);

  /* Record is appended so it is the last in the list, set the Message End bit */
  NDef_Header_SetME(record);

  record->next = NULL;

  if (message->record == NULL) {
    /* Set the Message Begin bit for the first record only */
    NDef_Header_SetMB(record);

    message->record = record;
  } else {
    NDef_Record *current = message->record;

    /* Go through the list of records */
    while (current->next != NULL) {
      current = current->next;
    }

    /* Clear the Message End bit to the record before the one being appended */
    NDef_Header_ClearME(current);

    /* Append to the last record */
    current->next = record;
  }

  message->info.length      += NDef_Record_GetLength(record);
  message->info.recordCount += 1U;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Message_Decode(const NDef_Const_Buffer *bufPayload, NDef_Message *message)
{
  NFC_OpResult err;
  uint32_t offset;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL)) {
    return NFC_InvalidParameter;
  }

  err = NDef_Message_Init(message);
  if (err != NFC_OK) {
    return err;
  }

  offset = 0;
  while (offset < bufPayload->length) {
    NDef_Const_Buffer bufRecord;
    NDef_Record *record = NDefAllocRecord();
    if (record == NULL) {
      return NFC_MemoryError;
    }
    bufRecord.buffer = &bufPayload->buffer[offset];
    bufRecord.length =  bufPayload->length - offset;
    err = NDef_Record_Decode(&bufRecord, record);
    if (err != NFC_OK) {
      return err;
    }
    offset += NDef_Record_GetLength(record);

    err = NDef_Message_Append(message, record);
    if (err != NFC_OK) {
      return err;
    }
  }

  return NFC_OK;
}

/*****************************************************************************/
NFC_OpResult NDef_Message_Encode(const NDef_Message *message, NDef_Buffer *bufPayload)
{
  NFC_OpResult      err;
  NDef_Message_Info info;
  NDef_Record     *record;
  uint32_t        offset;
  uint32_t        remainingLength;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL)) {
    return NFC_InvalidParameter;
  }

  err = NDef_Message_GetInfo(message, &info);
  if ((err != NFC_OK) || (bufPayload->length < info.length)) {
    bufPayload->length = info.length;
    return NFC_MemoryError;
  }

  /* Get the first record */
  record          = NDef_Message_GetFirstRecord(message);
  offset          = 0;
  remainingLength = bufPayload->length;

  while (record != NULL) {
    NDef_Buffer bufRecord;
    bufRecord.buffer = &bufPayload->buffer[offset];
    bufRecord.length = remainingLength;
    err = NDef_Record_Encode(record, &bufRecord);
    if (err != NFC_OK) {
      bufPayload->length = info.length;
      return err;
    }
    offset          += bufRecord.length;
    remainingLength -= bufRecord.length;

    record = NDef_Message_GetNextRecord(record);
  }

  bufPayload->length = offset;
  return NFC_OK;
}

/*****************************************************************************/
NDef_Record *NDef_Message_FindRecordType(NDef_Message *message, uint8_t tnf, const NDef_Const_Buffer_8 *bufType)
{
  NDef_Record *record;

  record = NDef_Message_GetFirstRecord(message);

  while (record != NULL) {
    if (NDef_Record_TypeMatch(record, tnf, bufType) == true) {
      return record;
    }

    record = NDef_Message_GetNextRecord(record);
  }

  return record;
}
