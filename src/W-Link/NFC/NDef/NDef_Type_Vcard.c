
/**
  ******************************************************************************
  * @file           : NDef__type_vcard.cpp
  * @brief          : NDEF MIME vCard type
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
#include "NDef_Type_VCard.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_VCARD_SUPPORT

/*! vCard Type strings */
static const uint8_t NDef_MediaTypeVCard[]    = "text/x-vCard";       /*!< vCard Type */

const NDef_Const_Buffer_8 bufMediaTypeVCard     = { NDef_MediaTypeVCard, sizeof(NDef_MediaTypeVCard) - 1U     };  /*!< vCard Type buffer    */


/*! vCard delimiters */
static const uint8_t COLON[]     = ":";
static const uint8_t SEMICOLON[] = ";";
static const uint8_t NEWLINE[]   = "\r\n";
static const uint8_t LINEFEED[]  = "\n";

static const NDef_Const_Buffer bufColon     = { COLON,     sizeof(COLON)     - 1U }; /*!< ":"    */
static const NDef_Const_Buffer bufSemicolon = { SEMICOLON, sizeof(SEMICOLON) - 1U }; /*!< ";"    */
static const NDef_Const_Buffer bufNewLine   = { NEWLINE,   sizeof(NEWLINE)   - 1U }; /*!< "\r\n" */
static const NDef_Const_Buffer bufLineFeed  = { LINEFEED,  sizeof(LINEFEED)  - 1U }; /*!< "\n"   */

/*! vCard Payload minimal length (BEGIN:VCARD + VERSION:2.1 + END:VCARD) */
#define NDEF_VCARD_PAYLOAD_LENGTH_MIN    ( sizeof("BEGIN:VCARD") - 1U + sizeof("VERSION:2.1") - 1U + sizeof("END:VCARD") - 1U )

/*****************************************************************************/
bool NDef_BufferMatch(const NDef_Const_Buffer *buf1, const NDef_Const_Buffer *buf2)
{
  if ((buf1 == NULL) || (buf2 == NULL)) {
    return false;
  } else if ((buf1->buffer == buf2->buffer) &&
             (buf1->length == buf2->length)) {
    return true;
  } else {
    if ((buf1->length != 0U) &&
        (buf1->length == buf2->length) &&
        (memcmp(buf1->buffer, buf2->buffer, buf1->length) == 0)) {
      return true;
    }
  }

  return false;
}


/*****************************************************************************/
static NFC_OpResult NDef_BufferFind(const NDef_Const_Buffer *bufPayload, const NDef_Const_Buffer *bufMarker, uint32_t *offset)
{
  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (bufMarker  == NULL) || (bufMarker->buffer  == NULL)) {
    return NFC_InvalidParameter;
  }

  uint32_t i = 0;
  while ((i + bufMarker->length) <= bufPayload->length) {
    NDef_Const_Buffer bufOffset;
    bufOffset.buffer = &bufPayload->buffer[i];
    bufOffset.length = bufMarker->length;

    if (NDef_BufferMatch(&bufOffset, bufMarker) == true) {
      if (offset != NULL) {
        *offset = i;
      }
      return NFC_OK;
    }
    i++;
  }

  return NFC_NotFound;
}


/*****************************************************************************/
static NFC_OpResult NDef_VCardGetPropertyType(const NDef_Const_Buffer *bufProperty, NDef_Const_Buffer *bufType)
{
  NFC_OpResult err;

  if ((bufProperty == NULL) || (bufType == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Look for the type delimiter semicolon ":" */
  uint32_t colonOffset;
  err = NDef_BufferFind(bufProperty, &bufColon, &colonOffset);
  if (err < NFC_OK) {
    return err;
  }

  bufType->buffer = bufProperty->buffer;
  bufType->length = colonOffset;

  /* Look for the subtype delimiter semicolon ";", if any */
  uint32_t semicolonOffset;
  err = NDef_BufferFind(bufProperty, &bufSemicolon, &semicolonOffset);
  if (err == NFC_OK) {
    bufType->length = (semicolonOffset < colonOffset) ? semicolonOffset : colonOffset; /* Type is ahead ";" or ":" */
  }

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_VCardGetPropertySubtype(const NDef_Const_Buffer *bufProperty, NDef_Const_Buffer *bufSubtype)
{
  NFC_OpResult err;

  ///* These parameters check are not needed as long as this function is only
  //   called internally (being static) because these parameters are checked by the caller */
  //if ( (bufProperty == NULL) || (bufSubtype == NULL) )
  //{
  //    return NFC_InvalidParameter;
  //}

  /* Look for the type delimiter colon ":" */
  uint32_t colonOffset;
  err = NDef_BufferFind(bufProperty, &bufColon, &colonOffset);
  if (err < NFC_OK) {
    return err;
  }

  /* Look for the subtype delimiter semicolon ";" */
  uint32_t semicolonOffset;
  err = NDef_BufferFind(bufProperty, &bufSemicolon, &semicolonOffset);
  if (err < NFC_OK) {
    return err;
  }

  /* The subtype is between the first semicolon ";" delimiter and ":" delimiter */
  if (semicolonOffset < colonOffset) {
    bufSubtype->buffer = &bufProperty->buffer[semicolonOffset + bufSemicolon.length];
    bufSubtype->length = colonOffset - (semicolonOffset + bufSemicolon.length);
    return NFC_OK;
  }

  return NFC_NotFound;
}


/*****************************************************************************/
static NFC_OpResult NDef_VCardGetPropertyEOL(const NDef_Const_Buffer *bufProperty, NDef_Const_Buffer *bufEOL)
{
  NFC_OpResult err;
  uint32_t offset;

  if ((bufProperty == NULL) || (bufEOL == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Look for "\r\n" */
  err = NDef_BufferFind(bufProperty, &bufNewLine, &offset); /* "\r\n" */
  if (err == NFC_OK) {
    bufEOL->buffer = bufNewLine.buffer;
    bufEOL->length = bufNewLine.length;
  } else {
    /* Look for "\n" */
    err = NDef_BufferFind(bufProperty, &bufLineFeed, &offset); /* "\n" */
    if (err == NFC_OK) {
      bufEOL->buffer = bufLineFeed.buffer;
      bufEOL->length = bufLineFeed.length;
    } else {
      bufEOL->buffer = NULL;
      bufEOL->length = 0;
    }
  }

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_VCardGetPropertyValue(const NDef_Const_Buffer *bufProperty, NDef_Const_Buffer *bufValue)
{
  NFC_OpResult err;

  if ((bufProperty == NULL) || (bufProperty->buffer == NULL) ||
      (bufValue    == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Look for the type delimiter colon ":" */
  uint32_t colonOffset;
  err = NDef_BufferFind(bufProperty, &bufColon, &colonOffset);
  if (err < NFC_OK) {
    return err;
  }

  /* Look for the End-Of-Line */
  NDef_Const_Buffer bufEOL;
  err = NDef_VCardGetPropertyEOL(bufProperty, &bufEOL);
  if (err < NFC_OK) {
    return err;
  }

  /* Value between ":" and End-Of-Line */
  bufValue->buffer = &bufProperty->buffer[colonOffset + bufColon.length];
  bufValue->length =  bufProperty->length - (colonOffset + bufColon.length + bufEOL.length);

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_VCardParseProperty(const NDef_Const_Buffer *bufProperty, NDef_Const_Buffer *bufType, NDef_Const_Buffer *bufSubtype, NDef_Const_Buffer *bufValue)
{
  NFC_OpResult err;

  if ((bufProperty == NULL) ||
      (bufType     == NULL) || (bufSubtype == NULL) || (bufValue == NULL)) {
    return NFC_InvalidParameter;
  }

  err = NDef_VCardGetPropertyType(bufProperty, bufType);
  if (err < NFC_OK) {
    return err;
  }

  err = NDef_VCardGetPropertySubtype(bufProperty, bufSubtype);
  if (err < NFC_OK) {
    /* Not all properties have a subtype */
    bufSubtype->buffer = NULL;
    bufSubtype->length = 0;
  }

  err = NDef_VCardGetPropertyValue(bufProperty, bufValue);
  if (err < NFC_OK) {
    return err;
  }

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_VCardSetProperty(NDef_Type_VCard *vCard, const NDef_Const_Buffer *bufProperty)
{
  NFC_OpResult err;

  if ((vCard == NULL) || (bufProperty == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Check the property contains a type */
  NDef_Const_Buffer bufPropertyType;
  err = NDef_VCardGetPropertyType(bufProperty, &bufPropertyType);
  if (err < NFC_OK) {
    return err;
  }

  for (uint32_t i = 0; i < (uint32_t)(sizeof(vCard->propertyBuffer)/sizeof(vCard->propertyBuffer[0])); i++) {
    /* Find first free property */
    if (vCard->propertyBuffer[i] == NULL) {
      /* Append it */
      vCard->propertyBuffer[i] = bufProperty->buffer;
      vCard->propertyLength[i] = (uint8_t)bufProperty->length;
      return NFC_OK;
    } else {
      /* Or update existing one */
      NDef_Const_Buffer vCardProperty;
      vCardProperty.buffer = vCard->propertyBuffer[i];
      vCardProperty.length = (uint8_t)vCard->propertyLength[i];

      NDef_Const_Buffer vCardPropertyType;
      err = NDef_VCardGetPropertyType(&vCardProperty, &vCardPropertyType);
      if (err < NFC_OK) {
        return err;
      }

      if (NDef_BufferMatch(&vCardPropertyType, &bufPropertyType) == true) {
        vCard->propertyBuffer[i] = bufProperty->buffer;
        vCard->propertyLength[i] = (uint8_t)bufProperty->length;
        return NFC_OK;
      }
    }
  }

  return NFC_MemoryError;
}


/*****************************************************************************/
NFC_OpResult NDef_VCardGetProperty(const NDef_Type_VCard *vCard, const NDef_Const_Buffer *bufType, NDef_Const_Buffer *bufProperty)
{
  NFC_OpResult err;

  if ((vCard   == NULL) ||
      (bufType == NULL) || (bufType->buffer == NULL)) {
    return NFC_InvalidParameter;
  }

  for (uint32_t i = 0; i < sizeof(vCard->propertyBuffer)/sizeof(vCard->propertyBuffer[0]); i++) {
    NDef_Const_Buffer bufLine;
    bufLine.buffer = vCard->propertyBuffer[i];
    bufLine.length = vCard->propertyLength[i];

    NDef_Const_Buffer bufLineType;
    err = NDef_VCardGetPropertyType(&bufLine, &bufLineType);
    if (err < NFC_OK) {
      return NFC_NotFound;
    }

    if (NDef_BufferMatch(&bufLineType, bufType) == true) {
      if (bufProperty != NULL) {
        bufProperty->buffer = bufLine.buffer;
        bufProperty->length = bufLine.length;
      }
      return NFC_OK;
    }
  }

  return NFC_NotFound;
}


/*****************************************************************************/
static uint32_t NDef_VCardPayloadGetLength(const NDef_Type *type)
{
  const NDef_Type_VCard *NDef_Data;

  if ((type == NULL) || (type->id != NDEF_TYPE_ID_MEDIA_VCARD)) {
    return 0;
  }

  NDef_Data = &type->data.vCard;

  uint32_t payloadLength = 0;
  for (uint32_t i = 0; i < sizeof(NDef_Data->propertyBuffer)/sizeof(NDef_Data->propertyBuffer[0]); i++) {
    payloadLength += NDef_Data->propertyLength[i];
  }

  return payloadLength;
}


/*****************************************************************************/
NFC_OpResult NDef_VCardReset(NDef_Type_VCard *vCard)
{
  if (vCard == NULL) {
    return NFC_InvalidParameter;
  }

  /* Initialize every property */
  for (uint32_t i = 0; i < (uint32_t)sizeof(vCard->propertyBuffer)/sizeof(vCard->propertyBuffer[0]); i++) {
    vCard->propertyBuffer[i] = NULL;
    vCard->propertyLength[i] = 0;
  }

  return NFC_OK;
}


/*****************************************************************************/
static const uint8_t *NDef_VCardToPayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_VCard *NDef_Data;

  if ((type    == NULL) || (type->id != NDEF_TYPE_ID_MEDIA_VCARD) ||
      (bufItem == NULL)) {
    return NULL;
  }

  NDef_Data = &type->data.vCard;

  bufItem->buffer = NULL;
  bufItem->length = 0;

  /* Initialization */
  if (begin == true) {
    item = 0;
  }

  while (item < (uint32_t)sizeof(NDef_Data->propertyBuffer)/sizeof(NDef_Data->propertyBuffer[0])) {
    bufItem->buffer = NDef_Data->propertyBuffer[item];
    bufItem->length = NDef_Data->propertyLength[item];

    item++;
    return bufItem->buffer;
  }

  return bufItem->buffer;
}


/*****************************************************************************/
NFC_OpResult NDef_VCardInit(NDef_Type *type, const NDef_Type_VCard *vCard)
{
  NDef_Type_VCard *NDef_Data;

  if ((type == NULL) || (vCard == NULL)) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_MEDIA_VCARD;
  type->getPayloadLength = NDef_VCardPayloadGetLength;
  type->getPayloadItem   = NDef_VCardToPayloadItem;
  type->typeToRecord     = NDef_VCardToRecord;
  NDef_Data               = &type->data.vCard;

  /* Copy in a bulk */
  (void)memcpy(NDef_Data, vCard, sizeof(NDef_Type_VCard));

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetVCard(const NDef_Type *type, NDef_Type_VCard *vCard)
{
  const NDef_Type_VCard *NDef_Data;

  if ((type  == NULL) || (type->id != NDEF_TYPE_ID_MEDIA_VCARD) ||
      (vCard == NULL)) {
    return NFC_InvalidParameter;
  }

  NDef_Data = &type->data.vCard;

  /* Copy in a bulk */
  (void)memcpy(vCard, NDef_Data, sizeof(NDef_Type_VCard));

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_VCardGetLine(const NDef_Const_Buffer *bufPayload, NDef_Const_Buffer *bufLine)
{
  NFC_OpResult err;
  uint32_t offset;

  if ((bufPayload == NULL) || (bufLine == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Look for "\r\n" */
  err = NDef_BufferFind(bufPayload, &bufNewLine, &offset); /* "\r\n" */
  if (err == NFC_OK) {
    /* Return up to the marker */
    bufLine->buffer = bufPayload->buffer;
    bufLine->length = offset + bufNewLine.length;
  } else {
    /* Look for "\n" */
    err = NDef_BufferFind(bufPayload, &bufLineFeed, &offset); /* "\n" */
    if (err == NFC_OK) {
      /* Return up to the marker */
      bufLine->buffer = bufPayload->buffer;
      bufLine->length = offset + bufLineFeed.length;
    } else {
      /* Return up to the end of the payload */
      bufLine->buffer = bufPayload->buffer;
      bufLine->length = bufPayload->length;
    }
  }

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToVcard(const NDef_Const_Buffer *bufPayload, NDef_Type *type)
{
  /*! vCard keyword types */
  static const uint8_t TYPE_BEGIN[]   = "BEGIN";
  static const uint8_t TYPE_END[]     = "END";
  static const uint8_t TYPE_VERSION[] = "VERSION";
  /*static const uint8_t VALUE_VCARD[]  = "VCARD";*/
  /*static const uint8_t VALUE_2_1[]    = "2.1";*/

  static const NDef_Const_Buffer bufTypeBegin   = { TYPE_BEGIN,   sizeof(TYPE_BEGIN)   - 1U }; /*!< "BEGIN"   */
  static const NDef_Const_Buffer bufTypeEnd     = { TYPE_END,     sizeof(TYPE_END)     - 1U }; /*!< "END"     */
  static const NDef_Const_Buffer bufTypeVersion = { TYPE_VERSION, sizeof(TYPE_VERSION) - 1U }; /*!< "VERSION" */
  /*static const NDef_Const_Buffer bufValueVCard  = { VALUE_VCARD,  sizeof(VALUE_VCARD)  - 1U }; *//*!< "VCARD"   */
  /*static const NDef_Const_Buffer bufValue_2_1   = { VALUE_2_1,    sizeof(VALUE_2_1)    - 1U }; *//*!< "2.1"     */

  NFC_OpResult err;
  NDef_Type_VCard *NDef_Data;

  NDef_Const_Buffer bufRemaining;
  NDef_Const_Buffer bufLine;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (type       == NULL)) {
    return NFC_InvalidParameter;
  }

  if (bufPayload->length < NDEF_VCARD_PAYLOAD_LENGTH_MIN) { /* vCard Payload Min */
    return NFC_ProtocolError;
  }

  type->id               = NDEF_TYPE_ID_MEDIA_VCARD;
  type->getPayloadLength = NDef_VCardPayloadGetLength;
  type->getPayloadItem   = NDef_VCardToPayloadItem;
  type->typeToRecord     = NDef_VCardToRecord;
  NDef_Data               = &type->data.vCard;

  /* Reset the vCard before parsing the payload */
  if (NDef_VCardReset(NDef_Data) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  uint32_t offset = 0;
  while (offset < bufPayload->length) {
    /* Parse the remaining to find an "end of line" or reach the end of payload */
    bufRemaining.buffer = &bufPayload->buffer[offset];
    bufRemaining.length = bufPayload->length - offset;

    err = NDef_VCardGetLine(&bufRemaining, &bufLine);
    if (err < NFC_OK) {
      return err;
    }

    err = NDef_VCardSetProperty(NDef_Data, &bufLine);
    if (err < NFC_OK) {
      return err;
    }

    /* Move to the next line */
    offset += bufLine.length;
  }

  /* Check BEGIN, VERSION and END types were found */
  NFC_OpResult err_begin   = NDef_VCardGetProperty(NDef_Data, &bufTypeBegin, NULL);
  NFC_OpResult err_version = NDef_VCardGetProperty(NDef_Data, &bufTypeVersion, NULL);
  NFC_OpResult err_end     = NDef_VCardGetProperty(NDef_Data, &bufTypeEnd, NULL);
  if ((err_begin != NFC_OK) || (err_version != NFC_OK) || (err_end != NFC_OK)) {
    return NFC_Syntax ;
  }

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToVCard(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *NDef_Data;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_Record_TypeMatch(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeVCard)) { /* "text/x-vCard" */
    return NFC_ProtocolError;
  }

  NDef_Data = NDef_RecordGetNDefType(record);
  if ((NDef_Data != NULL) && (type->id == NDEF_TYPE_ID_MEDIA_VCARD)) {
    (void)memcpy(type, NDef_Data, sizeof(NDef_Type));
    return NFC_OK;
  }

  return NDef_PayloadToVcard(&record->bufPayload, type);
}


/*****************************************************************************/
NFC_OpResult NDef_VCardToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type   == NULL) || (type->id != NDEF_TYPE_ID_MEDIA_VCARD) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  (void)NDef_Record_SetType(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeVCard);

  if (NDef_RecordSetNDefType(record, type) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
