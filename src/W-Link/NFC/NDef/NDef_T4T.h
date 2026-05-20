
/**
  ******************************************************************************
  * @file           : ndef_t4t.h
  * @brief          : Provides NDEF methods and definitions to access NFC Forum T4T
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

#ifndef NDEF_T4T_H
#define NDEF_T4T_H

#include "NDef_Poller.h"

#include "NFC/RFal/RFal_NFC.h"

#include "NFC/NFC_Def.h"

#define NDEF_T4T_MAPPING_VERSION_2_0  0x20U                    /*!< Mapping version 2.0                   */
#define NDEF_T4T_MAPPING_VERSION_3_0  0x30U                    /*!< Mapping version 3.0                   */

/*! Minimum size for an APDU (corresponding to Select NDEF App)     */
#define NDEF_T4T_MIN_APDU_LEN                                   13U

/*! Maximum Response-APDU response body length (short field coding) */
#if RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN > (256 + RFAL_T4T_MAX_RAPDU_SW1SW2_LEN)
  #define NDEF_T4T_MAX_RAPDU_BODY_LEN                            256U
#else
  #define NDEF_T4T_MAX_RAPDU_BODY_LEN (RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN - RFAL_T4T_MAX_RAPDU_SW1SW2_LEN)
#endif

/*! Maximum Command-APDU data length (short field coding)           */
#if RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN > (255 + RFAL_T4T_MAX_CAPDU_PROLOGUE_LEN + RFAL_T4T_LC_LEN + RFAL_T4T_LE_LEN)
  #define NDEF_T4T_MAX_CAPDU_BODY_LEN                            255U
#else
  #define NDEF_T4T_MAX_CAPDU_BODY_LEN (RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN - (RFAL_T4T_MAX_CAPDU_PROLOGUE_LEN + RFAL_T4T_LC_LEN + RFAL_T4T_LE_LEN))
#endif

#define Ndef_T4T_IsReadAccessGranted(r)  ( ((r) == 0x00U) || (((r) >= 0x80U) && ((r) <= 0xFEU)) ) /*!< Read access status  */
#define Ndef_T4T_IsWriteAccessGranted(w) ( ((w) == 0x00U) || (((w) >= 0x80U) && ((w) <= 0xFEU)) ) /*!< Write access status */

NFC_OpResult NDef_T4T_Poller_ContextInitialization(NDef_Context *ctx, const RFal_NFC_Device *dev);
NFC_OpResult NDef_T4T_Poller_NdefDetect(NDef_Context *ctx, NDef_Info *info);
NFC_OpResult NDef_T4T_Poller_SelectNdefTagApplication(NDef_Context *ctx);
NFC_OpResult NDef_T4T_Poller_SelectFile(NDef_Context *ctx, const uint8_t *fileId);
NFC_OpResult NDef_T4T_Poller_ReadBinary(NDef_Context *ctx, uint16_t offset, uint8_t len);
NFC_OpResult NDef_T4T_Poller_ReadBinaryODO(NDef_Context *ctx, uint32_t offset, uint8_t len);
NFC_OpResult NDef_T4T_Poller_ReadBytes(NDef_Context *ctx, uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);
NFC_OpResult NDef_T4T_Poller_WriteBinary(NDef_Context *ctx, uint16_t offset, const uint8_t *data, uint8_t len);
NFC_OpResult NDef_T4T_Poller_WriteBinaryODO(NDef_Context *ctx, uint32_t offset, const uint8_t *data, uint8_t len);
NFC_OpResult NDef_T4T_Poller_WriteBytes(NDef_Context *ctx, uint32_t offset, const uint8_t *buf, uint32_t len, bool pad, bool writeTerminator);
NFC_OpResult NDef_T4T_Poller_ReadRawMessage(NDef_Context *ctx, uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen, bool single);
NFC_OpResult NDef_T4T_Poller_WriteRawMessage(NDef_Context *ctx, const uint8_t *buf, uint32_t bufLen);
NFC_OpResult NDef_T4T_Poller_WriteRawMessageLen(NDef_Context *ctx, uint32_t rawMessageLen, bool writeTerminator);
NFC_OpResult NDef_T4T_Poller_TagFormat(NDef_Context *ctx, const NDef_CapabilityContainer *cc, uint32_t options);
NFC_OpResult NDef_T4T_Poller_CheckPresence(NDef_Context *ctx);
NFC_OpResult NDef_T4T_Poller_CheckAvailableSpace(const NDef_Context *ctx, uint32_t messageLen);
NFC_OpResult NDef_T4T_Poller_BeginWriteMessage(NDef_Context *ctx, uint32_t messageLen);
NFC_OpResult NDef_T4T_Poller_EndWriteMessage(NDef_Context *ctx, uint32_t messageLen, bool writeTerminator);
NFC_OpResult NDef_T4T_Poller_SetReadOnly(NDef_Context *ctx);

#endif /* NDEF_T4T_H */
