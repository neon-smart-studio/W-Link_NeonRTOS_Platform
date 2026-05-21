
/**
  ******************************************************************************
  * @file           : NDef__type_vcard.h
  * @brief          : NDEF MIME vCard type header file
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

#ifndef NDEF_TYPE_VCARD_H
#define NDEF_TYPE_VCARD_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"
#include "NDef_Types.h"

#include "NFC/NFC_Def.h"

#define NDEF_VCARD_PROPERTY_COUNT       16U    /*!< Number of properties that can be decoded */

/*! vCard Record Type buffer */
extern const NDef_Const_Buffer_8 bufMediaTypeVCard; /*! vCard Record Type buffer */

/*! NDEF Type vCard */
typedef struct {
  const uint8_t *propertyBuffer[NDEF_VCARD_PROPERTY_COUNT]; /*!< vCard property buffers  */
  uint8_t        propertyLength[NDEF_VCARD_PROPERTY_COUNT]; /*!< vCard property buffers length */
} NDef_Type_VCard;

bool NDef_BufferMatch(const NDef_Const_Buffer *buf1, const NDef_Const_Buffer *buf2);
NFC_OpResult NDef_VCardParseProperty(const NDef_Const_Buffer *bufProperty, NDef_Const_Buffer *bufType, NDef_Const_Buffer *bufSubtype, NDef_Const_Buffer *bufValue);
NFC_OpResult NDef_VCardSetProperty(NDef_Type_VCard *vCard, const NDef_Const_Buffer *bufProperty);
NFC_OpResult NDef_VCardGetProperty(const NDef_Type_VCard *vCard, const NDef_Const_Buffer *bufType, NDef_Const_Buffer *bufProperty);
NFC_OpResult NDef_VCardReset(NDef_Type_VCard *vCard);
NFC_OpResult NDef_VCardInit(NDef_Type *type, const NDef_Type_VCard *vCard);
NFC_OpResult NDef_GetVCard(const NDef_Type *type, NDef_Type_VCard *vCard);
NFC_OpResult NDef_RecordToVCard(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_VCardToRecord(const NDef_Type *type, NDef_Record *record);

#endif /* NDEF_TYPE_VCARD_H */
