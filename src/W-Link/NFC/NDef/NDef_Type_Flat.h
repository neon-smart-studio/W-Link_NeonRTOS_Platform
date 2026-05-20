
/**
  ******************************************************************************
  * @file           : ndef_type_flat.h
  * @brief          : NDEF Flat payload type header file
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



#ifndef NDEF_TYPE_FLAT_H
#define NDEF_TYPE_FLAT_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

NFC_OpResult NDef_FlatPayloadTypeInit(NDef_Type *type, const NDef_Const_Buffer *bufPayload);
NFC_OpResult NDef_GetFlatPayloadType(const NDef_Type *type, NDef_Const_Buffer *bufPayload);
NFC_OpResult NDef_RecordToFlatPayloadType(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_FlatPayloadTypeToRecord(const NDef_Type *type, NDef_Record *record);

#endif /* NDEF_TYPE_FLAT_H */
