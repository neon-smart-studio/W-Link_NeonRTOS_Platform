
/**
  ******************************************************************************
  * @file           : ndef_type_media.cpp
  * @brief          : NDEF MIME types
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
#include "NDef_Type_Media.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_MEDIA_SUPPORT

/*****************************************************************************/
NFC_OpResult NDef_MediaInit(NDef_Type *media, const NDef_Const_Buffer_8 *bufType, const NDef_Const_Buffer *bufPayload)
{
  NDef_Type_Media *typeMedia;

  if ((media == NULL) || (bufType == NULL) || (bufPayload == NULL)) {
    return NFC_InvalidParameter;
  }

  media->id               = NDEF_TYPE_ID_MEDIA;
  media->getPayloadLength = NULL;
  media->getPayloadItem   = NULL;
  media->typeToRecord     = NDef_MediaToRecord;
  typeMedia               = &media->data.media;

  typeMedia->bufType.buffer    = bufType->buffer;
  typeMedia->bufType.length    = bufType->length;
  typeMedia->bufPayload.buffer = bufPayload->buffer;
  typeMedia->bufPayload.length = bufPayload->length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetMedia(const NDef_Type *media, NDef_Const_Buffer_8 *bufType, NDef_Const_Buffer *bufPayload)
{
  const NDef_Type_Media *typeMedia;

  if ((media   == NULL) || (media->id != NDEF_TYPE_ID_MEDIA) ||
      (bufType == NULL) || (bufPayload == NULL)) {
    return NFC_InvalidParameter;
  }

  typeMedia = &media->data.media;

  bufType->buffer    = typeMedia->bufType.buffer;
  bufType->length    = typeMedia->bufType.length;

  bufPayload->buffer = typeMedia->bufPayload.buffer;
  bufPayload->length = typeMedia->bufPayload.length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToMedia(const NDef_Record *record, NDef_Type *media)
{
  const NDef_Type *type;
  NDef_Const_Buffer_8 bufType;

  if ((record == NULL) || (media == NULL)) {
    return NFC_InvalidParameter;
  }

  if (NDef_Header_TNF(record) != NDEF_TNF_MEDIA_TYPE) {
    return NFC_ProtocolError;
  }

  type = NDef_RecordGetNDefType(record);
  if ((type != NULL) && (type->id == NDEF_TYPE_ID_MEDIA)) {
    (void)memcpy(media, type, sizeof(NDef_Type));
    return NFC_OK;
  }

  bufType.buffer = record->type;
  bufType.length = record->typeLength;

  return NDef_MediaInit(media, &bufType, &record->bufPayload);
}


/*****************************************************************************/
NFC_OpResult NDef_MediaToRecord(const NDef_Type *media, NDef_Record *record)
{
  const NDef_Type_Media *typeMedia;

  if ((media  == NULL) || (media->id != NDEF_TYPE_ID_MEDIA) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  typeMedia = &media->data.media;

  (void)NDef_Record_Reset(record);

  (void)NDef_Record_SetType(record, NDEF_TNF_MEDIA_TYPE, &typeMedia->bufType);

  (void)NDef_Record_SetPayload(record, &typeMedia->bufPayload);

  return NFC_OK;
}

#endif
