
/**
  ******************************************************************************
  * @file           : ndef_poller.cpp
  * @brief          : Provides NDEF methods and definitions to access NFC Forum Tags
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

#include "NDef_Buffer.h"
#include "NDef_Poller.h"
#include "NDef_T2T.h"
#include "NDef_T3T.h"
#include "NDef_T4T.h"
#include "NDef_T5T.h"

#include "NFC/RFal/RFal_NFC.h"

#include "NFC/NFC_Def.h"

NDef_Device_Type NDef_GetDeviceType(const RFal_NFC_Device *dev)
{
  NDef_Device_Type type = NDEF_DEV_NONE;

  if (dev != NULL) {
    switch (dev->type) {
      case RFAL_NFC_LISTEN_TYPE_NFCA:
        switch (dev->dev.nfca.type) {
          case RFAL_NFCA_T1T:
            type = NDEF_DEV_T1T;
            break;
          case RFAL_NFCA_T2T:
            type = NDEF_DEV_T2T;
            break;
          case RFAL_NFCA_T4T:
            type = NDEF_DEV_T4T;
            break;
          default:
            break;
        }
        break;
      case RFAL_NFC_LISTEN_TYPE_NFCB:
        type = NDEF_DEV_T4T;
        break;
      case RFAL_NFC_LISTEN_TYPE_NFCF:
        type = NDEF_DEV_T3T;
        break;
      case RFAL_NFC_LISTEN_TYPE_NFCV:
        type = NDEF_DEV_T5T;
        break;
      default:
        break;
    }
  }

  return type;
}

/*******************************************************************************/
static NFC_OpResult NDef_Poller_WriteRecord(NDef_Context *ctx, const NDef_Record *record, uint32_t *recordOffset)
{
  NFC_OpResult      err;
  uint8_t         recordHeaderBuf[NDEF_RECORD_HEADER_LEN];
  NDef_Buffer      bufHeader;
  NDef_Const_Buffer bufPayloadItem;
  uint32_t        offset;
  bool            firstPayloadItem;

  if ((ctx == NULL) || (record == NULL) || (recordOffset == NULL)) {
    return NFC_InvalidParameter;
  }

  offset = *recordOffset;

  bufHeader.buffer = recordHeaderBuf;
  bufHeader.length = sizeof(recordHeaderBuf);
  (void)NDef_Record_EncodeHeader(record, &bufHeader);
  err = NDef_Poller_WriteBytes(ctx, offset, bufHeader.buffer, bufHeader.length);
  if (err < NFC_OK) {
    /* Conclude procedure */
    return err;
  }
  offset += bufHeader.length;

  NDef_Const_Buffer_8 bufType;
  NDef_Record_GetType(record, NULL, &bufType);
  if (bufType.length != 0U) {
    err = NDef_Poller_WriteBytes(ctx, offset, bufType.buffer, bufType.length);
    if (err < NFC_OK) {
      /* Conclude procedure */
      return err;
    }
    offset += record->typeLength;
  }

  NDef_Const_Buffer_8 bufId;
  NDef_Record_GetId(record, &bufId);
  if (bufId.length != 0U) {
    err = NDef_Poller_WriteBytes(ctx, offset, bufId.buffer, bufId.length);
    if (err < NFC_OK) {
      /* Conclude procedure */
      return err;
    }
    offset += record->idLength;
  }
  if (NDef_Record_GetPayloadLength(record) != 0U) {
    firstPayloadItem = true;
    while (NDef_Record_GetPayloadItem(record, &bufPayloadItem, firstPayloadItem) != NULL) {
      firstPayloadItem = false;
      err = NDef_Poller_WriteBytes(ctx, offset, bufPayloadItem.buffer, bufPayloadItem.length);
      if (err < NFC_OK) {
        /* Conclude procedure */
        return err;
      }
      offset += bufPayloadItem.length;
    }
  }

  *recordOffset = offset;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_WriteMessage(NDef_Context *ctx, const NDef_Message *message)
{
  NFC_OpResult      err;
  NDef_Message_Info info;
  NDef_Record     *record;
  uint32_t        offset;

  if ((ctx == NULL) || (message == NULL)) {
    return NFC_InvalidParameter;
  }

  if ((ctx->state != NDEF_STATE_INITIALIZED) && (ctx->state != NDEF_STATE_READWRITE)) {
    return NFC_WrongState;
  }

  (void)NDef_Message_GetInfo(message, &info);

  /* Verify length of the NDEF message */
  err = NDef_Poller_CheckAvailableSpace(ctx, info.length);
  if (err < NFC_OK) {
    /* Conclude procedure */
    return NFC_InvalidParameter;
  }

  /* Reset L-Field/NLEN field */
  err = NDef_Poller_BeginWriteMessage(ctx, info.length);
  if (err < NFC_OK) {
    /* Conclude procedure */
    ctx->state = NDEF_STATE_INVALID;
    return err;
  }

  if (info.length != 0U) {
    offset = ctx->messageOffset;

    record = NDef_Message_GetFirstRecord(message);
    while (record != NULL) {
      err = NDef_Poller_WriteRecord(ctx, record, &offset);
      if (err < NFC_OK) {
        /* Conclude procedure */
        ctx->state = NDEF_STATE_INVALID;
        return err;
      }

      record = NDef_Message_GetNextRecord(record);
    }

    err = NDef_Poller_EndWriteMessage(ctx, info.length);
    if (err < NFC_OK) {
      /* Conclude procedure */
      ctx->state = NDEF_STATE_INVALID;
      return err;
    }

    /* Procedure complete: Set Read/Write state */
    ctx->state = NDEF_STATE_READWRITE;
  } else {
    /* Procedure complete: Set Initialized state */
    ctx->state = NDEF_STATE_INITIALIZED;
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_ContextInitialization(NDef_Context *ctx, const RFal_NFC_Device *dev)
{
  NDef_Device_Type type;

  static const NDef_Poller_Wrapper ndefT1TWrapper = {
    NULL, /* ndefT1TPollerContextInitialization, */
    NULL, /* ndefT1TPollerNdefDetect,            */
    NULL, /* ndefT1TPollerReadBytes,             */
    NULL, /* ndefT1TPollerReadRawMessage,        */
    NULL, /* ndefT1TPollerWriteBytes,            */
    NULL, /* ndefT1TPollerWriteRawMessage,       */
    NULL, /* ndefT1TPollerTagFormat,             */
    NULL, /* ndefT1TPollerWriteRawMessageLen     */
    NULL, /* ndefT1TPollerCheckPresence          */
    NULL, /* ndefT1TPollerCheckAvailableSpace    */
    NULL, /* ndefT1TPollerBeginWriteMessage      */
    NULL, /* ndefT1TPollerEndWriteMessage        */
    NULL  /* ndefT1TPollerSetReadOnly            */
  };

  static const NDef_Poller_Wrapper NDef_T2T_Wrapper = {
    NDef_T2T_Poller_ContextInitialization,
    NDef_T2T_Poller_NdefDetect,
    NDef_T2T_Poller_ReadBytes,
    NDef_T2T_Poller_ReadRawMessage,
    NDef_T2T_Poller_WriteBytes,
    NDef_T2T_Poller_WriteRawMessage,
    NDef_T2T_Poller_TagFormat,
    NDef_T2T_Poller_WriteRawMessageLen,
    NDef_T2T_Poller_CheckPresence,
    NDef_T2T_Poller_CheckAvailableSpace,
    NDef_T2T_Poller_BeginWriteMessage,
    NDef_T2T_Poller_EndWriteMessage,
    NDef_T2T_Poller_SetReadOnly
  };

  static const NDef_Poller_Wrapper NDef_T3T_Wrapper = {
    NDef_T3T_Poller_ContextInitialization,
    NDef_T3T_Poller_NdefDetect,
    NDef_T3T_Poller_ReadBytes,
    NDef_T3T_Poller_ReadRawMessage,
    NDef_T3T_Poller_WriteBytes,
    NDef_T3T_Poller_WriteRawMessage,
    NDef_T3T_Poller_TagFormat,
    NDef_T3T_Poller_WriteRawMessageLen,
    NDef_T3T_Poller_CheckPresence,
    NDef_T3T_Poller_CheckAvailableSpace,
    NDef_T3T_Poller_BeginWriteMessage,
    NDef_T3T_Poller_EndWriteMessage,
    NDef_T3T_Poller_SetReadOnly
  };

  static const NDef_Poller_Wrapper NDef_T4T_Wrapper = {
    NDef_T4T_Poller_ContextInitialization,
    NDef_T4T_Poller_NdefDetect,
    NDef_T4T_Poller_ReadBytes,
    NDef_T4T_Poller_ReadRawMessage,
    NDef_T4T_Poller_WriteBytes,
    NDef_T4T_Poller_WriteRawMessage,
    NDef_T4T_Poller_TagFormat,
    NDef_T4T_Poller_WriteRawMessageLen,
    NDef_T4T_Poller_CheckPresence,
    NDef_T4T_Poller_CheckAvailableSpace,
    NDef_T4T_Poller_BeginWriteMessage,
    NDef_T4T_Poller_EndWriteMessage,
    NDef_T4T_Poller_SetReadOnly
  };

  static const NDef_Poller_Wrapper NDef_T5T_Wrapper = {
    NDef_T5T_Poller_ContextInitialization,
    NDef_T5T_Poller_NdefDetect,
    NDef_T5T_Poller_ReadBytes,
    NDef_T5T_Poller_ReadRawMessage,
    NDef_T5T_Poller_WriteBytes,
    NDef_T5T_Poller_WriteRawMessage,
    NDef_T5T_Poller_TagFormat,
    NDef_T5T_Poller_WriteRawMessageLen,
    NDef_T5T_Poller_CheckPresence,
    NDef_T5T_Poller_CheckAvailableSpace,
    NDef_T5T_Poller_BeginWriteMessage,
    NDef_T5T_Poller_EndWriteMessage,
    NDef_T5T_Poller_SetReadOnly
  };

  static const NDef_Poller_Wrapper *NDef_Poller_Wrappers[] = {
    NULL,            /* No device */
    &ndefT1TWrapper,
    &NDef_T2T_Wrapper,
    &NDef_T3T_Wrapper,
    &NDef_T4T_Wrapper,
    &NDef_T5T_Wrapper,
  };

  if ((ctx == NULL) || (dev == NULL)) {
    return NFC_InvalidParameter;
  }

  type = NDef_GetDeviceType(dev);

  if ((type == NDEF_DEV_NONE) || (type >= sizeof(NDef_Poller_Wrappers)/sizeof(NDef_Poller_Wrappers[0]))) {
    return NFC_InvalidParameter;
  }

  ctx->NDefPollWrapper = NDef_Poller_Wrappers[type];

  /* NDefPollWrapper is NULL when support of a given tag type is not enabled */
  if ((ctx->NDefPollWrapper == NULL) || (ctx->NDefPollWrapper->pollerContextInitialization == NULL)) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerContextInitialization)(ctx, dev);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_NdefDetect(NDef_Context *ctx, NDef_Info *info)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerNdefDetect == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerNdefDetect)(ctx, info);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_ReadRawMessage(NDef_Context *ctx, uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen, bool single)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerReadRawMessage == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerReadRawMessage)(ctx, buf, bufLen, rcvdLen, single);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_ReadBytes(NDef_Context *ctx, uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerReadBytes == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerReadBytes)(ctx, offset, len, buf, rcvdLen);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_WriteRawMessage(NDef_Context *ctx, const uint8_t *buf, uint32_t bufLen)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerWriteRawMessage == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerWriteRawMessage)(ctx, buf, bufLen);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_TagFormat(NDef_Context *ctx, const NDef_CapabilityContainer *cc, uint32_t options)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerTagFormat == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerTagFormat)(ctx, cc, options);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_WriteRawMessageLen(NDef_Context *ctx, uint32_t rawMessageLen)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerWriteRawMessageLen == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerWriteRawMessageLen)(ctx, rawMessageLen, true);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_WriteBytes(NDef_Context *ctx, uint32_t offset, const uint8_t *buf, uint32_t len)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerWriteBytes == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerWriteBytes)(ctx, offset, buf, len, false, false);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_CheckPresence(NDef_Context *ctx)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerCheckPresence == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerCheckPresence)(ctx);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_CheckAvailableSpace(const NDef_Context *ctx, uint32_t messageLen)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerCheckAvailableSpace == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerCheckAvailableSpace)(ctx, messageLen);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_BeginWriteMessage(NDef_Context *ctx, uint32_t messageLen)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerBeginWriteMessage == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerBeginWriteMessage)(ctx, messageLen);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_EndWriteMessage(NDef_Context *ctx, uint32_t messageLen)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerEndWriteMessage == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerEndWriteMessage)(ctx, messageLen, true);
}

/*******************************************************************************/
NFC_OpResult NDef_Poller_SetReadOnly(NDef_Context *ctx)
{
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  if (ctx->NDefPollWrapper == NULL) {
    return NFC_WrongState;
  }

  if (ctx->NDefPollWrapper->pollerSetReadOnly == NULL) {
    return NFC_Unsupport;
  }

  return (ctx->NDefPollWrapper->pollerSetReadOnly)(ctx);
}
