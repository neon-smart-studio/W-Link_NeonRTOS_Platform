
/**
  ******************************************************************************
  * @file           : ndef_t3t.h
  * @brief          : Provides NDEF methods and definitions to access NFC Forum T3T
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

#ifndef NDEF_T3T_H
#define NDEF_T3T_H

#include "NDef_Poller.h"

#include "NFC/RFal/RFal_NFC.h"

#include "NFC/NFC_Def.h"

NFC_OpResult NDef_T3T_Poller_ContextInitialization(NDef_Context *ctx, const RFal_NFC_Device *dev);
NFC_OpResult NDef_T3T_Poller_NdefDetect(NDef_Context *ctx, NDef_Info *info);
NFC_OpResult NDef_T3T_Poller_ReadBytes(NDef_Context *ctx, uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);
NFC_OpResult NDef_T3T_Poller_WriteBytes(NDef_Context *ctx, uint32_t offset, const uint8_t *buf, uint32_t len, bool pad, bool writeTerminator);
NFC_OpResult NDef_T3T_Poller_ReadRawMessage(NDef_Context *ctx, uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen, bool single);
NFC_OpResult NDef_T3T_Poller_WriteRawMessage(NDef_Context *ctx, const uint8_t *buf, uint32_t bufLen);
NFC_OpResult NDef_T3T_Poller_WriteRawMessageLen(NDef_Context *ctx, uint32_t rawMessageLen, bool writeTerminator);
NFC_OpResult NDef_T3T_Poller_TagFormat(NDef_Context *ctx, const NDef_CapabilityContainer *cc, uint32_t options);
NFC_OpResult NDef_T3T_Poller_CheckPresence(NDef_Context *ctx);
NFC_OpResult NDef_T3T_Poller_CheckAvailableSpace(const NDef_Context *ctx, uint32_t messageLen);
NFC_OpResult NDef_T3T_Poller_BeginWriteMessage(NDef_Context *ctx, uint32_t messageLen);
NFC_OpResult NDef_T3T_Poller_EndWriteMessage(NDef_Context *ctx, uint32_t messageLen, bool writeTerminator);
NFC_OpResult NDef_T3T_Poller_SetReadOnly(NDef_Context *ctx);

#endif /* NDEF_T3T_H */