
/**
  ******************************************************************************
  * @file           : ndef_record.h
  * @brief          : NDEF record header file
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

#ifndef NDEF_RECORD_H
#define NDEF_RECORD_H

#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

#define NDEF_RECORD_HEADER_LEN       7U    /*!< Record header length (sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) */

#define NDEF_SHORT_RECORD_LENGTH_MAX 255U  /*!< Short record maximum length */


/*! Type Name Format aka TNF types */
#define NDEF_TNF_EMPTY               0U    /*!< TNF Empty             */
#define NDEF_TNF_RTD_WELL_KNOWN_TYPE 1U    /*!< TNF Well-known Type   */
#define NDEF_TNF_MEDIA_TYPE          2U    /*!< TNF Media Type        */
#define NDEF_TNF_URI                 3U    /*!< TNF URI               */
#define NDEF_TNF_RTD_EXTERNAL_TYPE   4U    /*!< TNF External Type     */
#define NDEF_TNF_UNKNOWN             5U    /*!< TNF Unknown           */
#define NDEF_TNF_UNCHANGED           6U    /*!< TNF Unchanged         */
#define NDEF_TNF_RESERVED            7U    /*!< TNF Reserved          */

#define NDEF_TNF_MASK                7U    /*!< Type Name Format mask */

/*! Build the record header byte, made of MB, ME, CF, SR, IL bits and TNF type */
#define NDef_Header(MB, ME, CF, SR, IL, TNF)  ((((MB) & 1U) << 7U) | (((ME) & 1U) << 6U) | (((CF) & 1U) << 5U) | (((SR) & 1U) << 4U) | (((IL) & 1U) << 3U) | ((uint8_t)(TNF) & NDEF_TNF_MASK) )   /*< Build the record header byte, made of MB, ME, CF, SR, IL bits and TNF type */

/*! Read bits in header byte */
#define NDef_Header_MB(record)             ( ((record)->header & 0x80U) >> 7 )    /*!< Return the MB bit from the record header byte */
#define NDef_Header_ME(record)             ( ((record)->header & 0x40U) >> 6 )    /*!< Return the ME bit from the record header byte */
#define NDef_Header_CF(record)             ( ((record)->header & 0x20U) >> 5 )    /*!< Return the CF bit from the record header byte */
#define NDef_Header_SR(record)             ( ((record)->header & 0x10U) >> 4 )    /*!< Return the SR bit from the record header byte */
#define NDef_Header_IL(record)             ( ((record)->header & 0x08U) >> 3 )    /*!< Return the IL bit from the record header byte */
#define NDef_Header_TNF(record)            (  (record)->header & NDEF_TNF_MASK )  /*!< Return the TNF type from the record header byte */

/*! Set bits in header byte */
#define NDef_Header_SetMB(record)          ( (record)->header |= (1U << 7) )      /*!< Set the MB bit in the record header byte */
#define NDef_Header_SetME(record)          ( (record)->header |= (1U << 6) )      /*!< Set the ME bit in the record header byte */
#define NDef_Header_SetCF(record)          ( (record)->header |= (1U << 5) )      /*!< Set the CF bit in the record header byte */
#define NDef_Header_SetSR(record)          ( (record)->header |= (1U << 4) )      /*!< Set the SR bit in the record header byte */
#define NDef_Header_SetIL(record)          ( (record)->header |= (1U << 3) )      /*!< Set the IL bit in the record header byte */
#define NDef_Header_SetTNF(record, value)  ( (record)->header |= (uint8_t)(value) & NDEF_TNF_MASK )  /*!< Set the TNF type in the record header byte */

/*! Clear bits in header byte */
#define NDef_Header_ClearMB(record)          ( (record)->header &= 0x7FU )        /*!< Clear the MB bit in the record header byte */
#define NDef_Header_ClearME(record)          ( (record)->header &= 0xBFU )        /*!< Clear the ME bit in the record header byte */
#define NDef_Header_ClearCF(record)          ( (record)->header &= 0xDFU )        /*!< Clear the CF bit in the record header byte */
#define NDef_Header_ClearSR(record)          ( (record)->header &= 0xEFU )        /*!< Clear the SR bit in the record header byte */
#define NDef_Header_ClearIL(record)          ( (record)->header &= 0xF7U )        /*!< Clear the IL bit in the record header byte */
#define NDef_Header_ClearTNF(record, value)  ( (record)->header &= 0xF8U )        /*!< Clear the TNF type in the record header byte */

/*! Set or Clear the MB/ME bit in header byte */
#define NDef_Header_SetValueMB(record, value)    do{ (record)->header &= 0x7FU; (record)->header |= (((uint8_t)(value)) & 1U) << 7; }while(0)   /*!< Write the value to the MB bit in the record header byte */
#define NDef_Header_SetValueME(record, value)    do{ (record)->header &= 0xBFU; (record)->header |= (((uint8_t)(value)) & 1U) << 6; }while(0)   /*!< Write the value to the ME bit in the record header byte */
#define NDef_Header_SetValueSR(record, value)    do{ (record)->header &= 0xEFU; (record)->header |= (((uint8_t)(value)) & 1U) << 4; }while(0)   /*!< Write the value to the SR bit in the record header byte */

/*! Test bit in header byte */
#define NDef_Header_IsSetMB(record)        ( NDef_Header_MB(record) == 1U )         /*!< Return true if the Message Begin bit is set */
#define NDef_Header_IsSetSR(record)        ( NDef_Header_SR(record) == 1U )         /*!< Return true if the Short Record bit is set  */
#define NDef_Header_IsSetIL(record)        ( NDef_Header_IL(record) == 1U )         /*!< Return true if the Id Length bit is set     */


typedef struct NDef_Type_Struct NDef_Type;       /*!< Forward declaration */
typedef struct NDef_Message_Struct NDef_Message; /*!< Forward declaration */

/*! Record type */
typedef struct NDef_Record_Struct {
  uint8_t  header;               /*!< Header byte made of MB:1 ME:1 CF:1 SR:1 IL:1 TNF:3 => 8 bits */
  uint8_t  typeLength;           /*!< Type length in bytes */
  uint8_t  idLength;             /*!< Id Length, presence depends on the IL bit */
  const uint8_t *type;           /*!< Type follows the structure implied by the value of the TNF field */
  const uint8_t *id;             /*!< Id (middle and terminating record chunks MUST NOT have an ID field) */
  NDef_Const_Buffer bufPayload;    /*!< Payload buffer. Payload length depends on the SR bit (either coded on 1 or 4 bytes) */

  const NDef_Type *ndeftype;      /*!< Well-known type data */

  struct NDef_Record_Struct *next; /*!< Pointer to the next record, if any */
} NDef_Record;


NFC_OpResult NDef_Record_Reset(NDef_Record *record);
NFC_OpResult NDef_Record_Init(NDef_Record *record, uint8_t tnf, const NDef_Const_Buffer_8 *bufType, const NDef_Const_Buffer_8 *bufId, const NDef_Const_Buffer *bufPayload);
uint32_t NDef_Record_GetHeaderLength(const NDef_Record *record);
uint32_t NDef_Record_GetLength(const NDef_Record *record);
NFC_OpResult NDef_Record_SetType(NDef_Record *record, uint8_t tnf, const NDef_Const_Buffer_8 *bufType);
NFC_OpResult NDef_Record_GetType(const NDef_Record *record, uint8_t *tnf, NDef_Const_Buffer_8 *bufType);
bool NDef_Record_TypeMatch(const NDef_Record *record, uint8_t tnf, const NDef_Const_Buffer_8 *bufType);
NFC_OpResult NDef_Record_SetId(NDef_Record *record, const NDef_Const_Buffer_8 *bufId);
NFC_OpResult NDef_Record_GetId(const NDef_Record *record, NDef_Const_Buffer_8 *bufId);
NFC_OpResult NDef_Record_SetPayload(NDef_Record *record, const NDef_Const_Buffer *bufPayload);
NFC_OpResult NDef_Record_GetPayload(const NDef_Record *record, NDef_Const_Buffer *bufPayload);
NFC_OpResult NDef_Record_Decode(const NDef_Const_Buffer *bufPayload, NDef_Record *record);
NFC_OpResult NDef_Record_EncodeHeader(const NDef_Record *record, NDef_Buffer *bufHeader);
NFC_OpResult NDef_Record_Encode(const NDef_Record *record, NDef_Buffer *bufRecord);
uint32_t NDef_Record_GetPayloadLength(const NDef_Record *record);
const uint8_t *NDef_Record_GetPayloadItem(const NDef_Record *record, NDef_Const_Buffer *bufPayloadItem, bool begin);

#endif /* NDEF_RECORD_H */
