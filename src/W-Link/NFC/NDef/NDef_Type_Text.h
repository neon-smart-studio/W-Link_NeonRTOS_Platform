
/**
  ******************************************************************************
  * @file           : ndef_type_text.h
  * @brief          : NDEF RTD Text type header file
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


#ifndef NDEF_TYPE_RTD_TEXT_H
#define NDEF_TYPE_RTD_TEXT_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"
#include "NDef_Types.h"

#include "NFC/NFC_Def.h"

/*! RTD Text Record Type buffer */
extern const NDef_Const_Buffer_8 bufRtdTypeText;       /*! Text Record Type buffer                             */

/*! RTD Type Text Encoding */
#define TEXT_ENCODING_UTF8               0U    /*!< UTF8  text encoding           */
#define TEXT_ENCODING_UTF16              1U    /*!< UTF16 text encoding           */

#define NDEF_TEXT_ENCODING_MASK       0x80U    /*!< Text encoding mask            */
#define NDEF_TEXT_ENCODING_SHIFT         7U    /*!< Text encoding bit shift       */


/*! RTD Type Text */
typedef struct {
  uint8_t          status;          /*!< Status byte                   */
  NDef_Const_Buffer_8 bufLanguageCode; /*!< ISO/IANA language code buffer */
  NDef_Const_Buffer  bufSentence;     /*!< Sentence buffer               */
} NDef_Type_Rtd_Text;

NFC_OpResult NDef_RtdTextInit(NDef_Type *text, uint8_t utfEncoding, const NDef_Const_Buffer_8 *bufLanguageCode, const NDef_Const_Buffer *bufSentence);
NFC_OpResult NDef_GetRtdText(const NDef_Type *text, uint8_t *utfEncoding, NDef_Const_Buffer_8 *bufLanguageCode, NDef_Const_Buffer *bufSentence);
NFC_OpResult NDef_RecordToRtdText(const NDef_Record *record, NDef_Type *text);
NFC_OpResult NDef_RtdTextToRecord(const NDef_Type *text, NDef_Record *record);

#endif /* NDEF_TYPE_RTD_TEXT_H */
