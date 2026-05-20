
/**
  ******************************************************************************
  * @file           : ndef_message.h
  * @brief          : NDEF message header file
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

#ifndef NDEF_MESSAGE_H
#define NDEF_MESSAGE_H

#include "NDef_Record.h"

#include "NFC/NFC_Def.h"

/*! Message scanning macros */
#define NDef_Message_GetFirstRecord(message)    (((message) == NULL) ? NULL : (message)->record)  /*!< Get first record */
#define NDef_Message_GetNextRecord(record)      (((record)  == NULL) ? NULL : (record)->next)     /*!< Get next record  */

/*! Message information */
typedef struct {
  uint32_t length;      /*!< Message length in bytes          */
  uint32_t recordCount; /*!< Number of records in the message */
} NDef_Message_Info;


/*! NDEF message */
struct NDef_Message_Struct {
  NDef_Record     *record; /*!< Pointer to a record */
  NDef_Message_Info info;   /*!< Message information, e.g. length in bytes, record count */
};

NFC_OpResult NDef_Message_Init(NDef_Message *message);
NFC_OpResult NDef_Message_GetInfo(const NDef_Message *message, NDef_Message_Info *info);
uint32_t NDef_Message_GetRecordCount(const NDef_Message *message);
NFC_OpResult NDef_Message_Append(NDef_Message *message, NDef_Record *record);
NFC_OpResult NDef_Message_Decode(const NDef_Const_Buffer *bufPayload, NDef_Message *message);
NFC_OpResult NDef_Message_Encode(const NDef_Message *message, NDef_Buffer *bufPayload);
NDef_Record *NDef_Message_FindRecordType(NDef_Message *message, uint8_t tnf, const NDef_Const_Buffer_8 *bufType);

#endif /* NDEF_MESSAGE_H */
