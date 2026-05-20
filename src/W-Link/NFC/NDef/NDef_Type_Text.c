
/**
  ******************************************************************************
  * @file           : ndef_type_text.cpp
  * @brief          : NDEF RTD Text type
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
#include "NDef_Type_Text.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_RTD_TEXT_SUPPORT

/*! Text defines */
#define NDEF_RTD_TEXT_STATUS_OFFSET              0U    /*!< Text status offset */
#define NDEF_RTD_TEXT_LANGUAGE_OFFSET            1U    /*!< Text language offset */

#define NDEF_RTD_TEXT_LANGUAGE_CODE_LEN_MASK  0x3FU    /*!< IANA language code mask (length coded on 6 bits) */

#define NDEF_RTD_TEXT_PAYLOAD_LENGTH_MIN         (sizeof(uint8_t) + sizeof(uint8_t))   /*!< Minimum Text Payload length */

/*! RTD Text Type string */
static const uint8_t ndefRtdTypeText[]           = "T";               /*!< Text Record Type               {0x54}       */

const NDef_Const_Buffer_8 bufRtdTypeText            = { ndefRtdTypeText,       sizeof(ndefRtdTypeText) - 1U };       /*!< Text Record Type buffer               */

/*****************************************************************************/
static uint32_t NDef_RtdTextPayloadGetLength(const NDef_Type *text)
{
  const NDef_Type_Rtd_Text *rtdText;

  if ((text == NULL) || (text->id != NDEF_TYPE_ID_RTD_TEXT)) {
    return 0;
  }

  rtdText = &text->data.text;

  return sizeof(rtdText->status) + rtdText->bufLanguageCode.length + rtdText->bufSentence.length;
}


/*****************************************************************************/
static const uint8_t *NDef_RtdTextToPayloadItem(const NDef_Type *text, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Rtd_Text *rtdText;

  if ((text    == NULL) || (text->id != NDEF_TYPE_ID_RTD_TEXT) ||
      (bufItem == NULL)) {
    return NULL;
  }

  rtdText = &text->data.text;

  if (begin == true) {
    item = 0;
  }

  switch (item) {
    case 0:
      /* Status byte */
      bufItem->buffer = &rtdText->status;
      bufItem->length = sizeof(rtdText->status);
      break;

    case 1:
      /* Language Code */
      bufItem->buffer = rtdText->bufLanguageCode.buffer;
      bufItem->length = rtdText->bufLanguageCode.length;
      break;

    case 2:
      /* Actual text */
      bufItem->buffer = rtdText->bufSentence.buffer;
      bufItem->length = rtdText->bufSentence.length;
      break;

    default:
      bufItem->buffer = NULL;
      bufItem->length = 0;
      break;
  }

  /* Move to next item for next call */
  item++;

  return bufItem->buffer;
}


/*****************************************************************************/
NFC_OpResult NDef_RtdTextInit(NDef_Type *text, uint8_t utfEncoding, const NDef_Const_Buffer_8 *bufLanguageCode, const NDef_Const_Buffer *bufSentence)
{
  NDef_Type_Rtd_Text *rtdText;

  if ((text            == NULL) ||
      (bufLanguageCode == NULL) || (bufLanguageCode->buffer == NULL) || (bufLanguageCode->length == 0U) ||
      (bufSentence     == NULL) || (bufSentence->buffer     == NULL) || (bufSentence->length     == 0U)) {
    return NFC_InvalidParameter;
  }

  if (bufLanguageCode->length > NDEF_RTD_TEXT_LANGUAGE_CODE_LEN_MASK) {
    return NFC_ProtocolError;
  }

  if ((utfEncoding != TEXT_ENCODING_UTF8) && (utfEncoding != TEXT_ENCODING_UTF16)) {
    return NFC_InvalidParameter;
  }

  text->id               = NDEF_TYPE_ID_RTD_TEXT;
  text->getPayloadLength = NDef_RtdTextPayloadGetLength;
  text->getPayloadItem   = NDef_RtdTextToPayloadItem;
  text->typeToRecord     = NDef_RtdTextToRecord;
  rtdText                = &text->data.text;

  rtdText->status = (utfEncoding << NDEF_TEXT_ENCODING_SHIFT) | (bufLanguageCode->length & NDEF_RTD_TEXT_LANGUAGE_CODE_LEN_MASK);

  rtdText->bufLanguageCode.buffer = bufLanguageCode->buffer;
  rtdText->bufLanguageCode.length = bufLanguageCode->length;

  rtdText->bufSentence.buffer = bufSentence->buffer;
  rtdText->bufSentence.length = bufSentence->length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdText(const NDef_Type *text, uint8_t *utfEncoding, NDef_Const_Buffer_8 *bufLanguageCode, NDef_Const_Buffer *bufSentence)
{
  const NDef_Type_Rtd_Text *rtdText;

  if ((text        == NULL) || (text->id != NDEF_TYPE_ID_RTD_TEXT) ||
      (utfEncoding == NULL) || (bufLanguageCode == NULL) || (bufSentence == NULL)) {
    return NFC_InvalidParameter;
  }

  rtdText = &text->data.text;

  *utfEncoding            = (rtdText->status >> NDEF_TEXT_ENCODING_SHIFT) & 1U;

  bufLanguageCode->buffer = rtdText->bufLanguageCode.buffer;
  bufLanguageCode->length = rtdText->bufLanguageCode.length;

  bufSentence->buffer     = rtdText->bufSentence.buffer;
  bufSentence->length     = rtdText->bufSentence.length;

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdText(const NDef_Const_Buffer *bufText, NDef_Type *text)
{
  NDef_Type_Rtd_Text *rtdText;
  uint8_t status;
  uint8_t languageCodeLength;

  if ((bufText == NULL) || (bufText->buffer == NULL) || (bufText->length == 0U) ||
      (text    == NULL)) {
    return NFC_InvalidParameter;
  }

  if (bufText->length < NDEF_RTD_TEXT_PAYLOAD_LENGTH_MIN) { /* Text Payload Min */
    return NFC_ProtocolError;
  }

  text->id               = NDEF_TYPE_ID_RTD_TEXT;
  text->getPayloadLength = NDef_RtdTextPayloadGetLength;
  text->getPayloadItem   = NDef_RtdTextToPayloadItem;
  text->typeToRecord     = NDef_RtdTextToRecord;
  rtdText                = &text->data.text;

  /* Extract info from the payload */
  status = bufText->buffer[NDEF_RTD_TEXT_STATUS_OFFSET];

  rtdText->status = status;

  /* Extract info from the status byte */
  //uint8_t textUtfEncoding          = (status & NDEF_TEXT_ENCODING_MASK) >> NDEF_TEXT_ENCODING_SHIFT;
  languageCodeLength = (status & NDEF_RTD_TEXT_LANGUAGE_CODE_LEN_MASK);

  rtdText->bufLanguageCode.buffer = &(bufText->buffer[NDEF_RTD_TEXT_LANGUAGE_OFFSET]);
  rtdText->bufLanguageCode.length = languageCodeLength;

  rtdText->bufSentence.buffer = &(bufText->buffer[NDEF_RTD_TEXT_LANGUAGE_OFFSET + languageCodeLength]);
  rtdText->bufSentence.length = bufText->length - sizeof(status) - languageCodeLength;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdText(const NDef_Record *record, NDef_Type *text)
{
  const NDef_Type *type;

  if ((record == NULL) || (text == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_RecordTypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeText)) { /* "T" */
    return NFC_ProtocolError;
  }

  type = NDef_RecordGetNdefType(record);
  if ((type != NULL) && (type->id == NDEF_TYPE_ID_RTD_TEXT)) {
    (void)memcpy(text, type, sizeof(NDef_Type));
    return NFC_OK;
  }

  return NDef_PayloadToRtdText(&record->bufPayload, text);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdTextToRecord(const NDef_Type *text, NDef_Record *record)
{
  if ((text   == NULL) || (text->id != NDEF_TYPE_ID_RTD_TEXT) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_RecordReset(record);

  /* "T" */
  (void)NDef_RecordSetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeText);

  if (NDef_RecordSetNdefType(record, text) < NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
