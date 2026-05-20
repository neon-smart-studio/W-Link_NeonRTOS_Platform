
/**
  ******************************************************************************
  * @file           : ndef_type_media.h
  * @brief          : NDEF MIME Media type header file
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


#ifndef NDEF_TYPE_MEDIA_H
#define NDEF_TYPE_MEDIA_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

/*! Media Type */
typedef struct {
  NDef_Const_Buffer_8 bufType;    /*!< Media type    */
  NDef_Const_Buffer  bufPayload; /*!< Media payload */
} NDef_Type_Media;

NFC_OpResult NDef_MediaInit(NDef_Type *media, const NDef_Const_Buffer_8 *bufType, const NDef_Const_Buffer *bufPayload);
NFC_OpResult NDef_GetMedia(const NDef_Type *media, NDef_Const_Buffer_8 *bufType, NDef_Const_Buffer *bufPayload);
NFC_OpResult NDef_RecordToMedia(const NDef_Record *record, NDef_Type *media);
NFC_OpResult NDef_MediaToRecord(const NDef_Type *media, NDef_Record *record);

#endif /* NDEF_TYPE_MEDIA_H */

/**
  * @}
  *
  */
