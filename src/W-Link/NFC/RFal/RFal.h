/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2021 STMicroelectronics</center></h2>
  *
  * Licensed under ST MIX MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/mix_myliberty
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/
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

#ifndef RFAL_H
#define RFAL_H

#include <stdbool.h>
#include <stdint.h>

#include "RFal_RF.h"

#include "NFC/NFC_Def.h"

#include "NFC_Config.h"

#ifdef __cplusplus
extern "C" {
#endif
/* -------------------------------------------------------------------------- */
/* Core */
/* -------------------------------------------------------------------------- */
NFC_OpResult RFal_Init(void);
NFC_OpResult RFal_Calibrate(void);
NFC_OpResult RFal_AdjustRegulators(uint16_t *result);
NFC_OpResult RFal_DeInit(void);
void RFal_SetObsvMode(uint32_t txMode, uint32_t rxMode);
void RFal_GetObsvMode(uint8_t *txMode, uint8_t *rxMode);
void RFal_DisableObsvMode(void);

/* -------------------------------------------------------------------------- */
/* Callback */
/* -------------------------------------------------------------------------- */
void RFal_SetUpperLayerCallback(RFal_UpperLayerCallback pFunc);
void RFal_SetPreTxRxCallback(RFal_PreTxRxCallback pFunc);
void RFal_SetSyncTxRxCallback(RFal_SyncTxRxCallback pFunc);
void RFal_SetPostTxRxCallback(RFal_PostTxRxCallback pFunc);
void RFal_SetLmEonCallback(RFal_LmEonCallback pFunc);

/* -------------------------------------------------------------------------- */
/* Mode / Bitrate / Timing / Field */
/* -------------------------------------------------------------------------- */
NFC_OpResult RFal_SetMode(RFal_Mode mode, RFal_BitRate txBR, RFal_BitRate rxBR);
RFal_Mode RFal_GetMode(void);
NFC_OpResult RFal_SetBitRate(RFal_BitRate txBR, RFal_BitRate rxBR);
NFC_OpResult RFal_GetBitRate(RFal_BitRate *txBR, RFal_BitRate *rxBR);
void RFal_SetErrorHandling(RFal_EHandling eHandling);
RFal_EHandling RFal_GetErrorHandling(void);
void RFal_SetFDTPoll(uint32_t FDTPoll);
uint32_t RFal_GetFDTPoll(void);
void RFal_SetFDTListen(uint32_t FDTListen);
uint32_t RFal_GetFDTListen(void);
void RFal_SetGT(uint32_t GT);
uint32_t RFal_GetGT(void);
bool RFal_IsGTExpired(void);
NFC_OpResult RFal_FieldOnAndStartGT(void);
NFC_OpResult RFal_FieldOff(void);
bool RFal_IsExtFieldOn(void);

/* -------------------------------------------------------------------------- */
/* Transceive */
/* -------------------------------------------------------------------------- */
NFC_OpResult RFal_StartTransceive(const RFal_TransceiveContext *ctx);
bool RFal_IsTransceiveInTx(void);
bool RFal_IsTransceiveInRx(void);
NFC_OpResult RFal_TransceiveBlockingTx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt);
NFC_OpResult RFal_TransceiveBlockingRx(void);
NFC_OpResult RFal_TransceiveBlockingTxRx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt);
RFal_TransceiveState RFal_GetTransceiveState(void);
NFC_OpResult RFal_GetTransceiveStatus(void);
NFC_OpResult RFal_GetTransceiveRSSI(uint16_t *rssi);
bool RFal_IsTransceiveSubcDetected(void);
void RFal_Worker(void);
void RFal_TransceiveTx(void);
void RFal_TransceiveRx(void);
NFC_OpResult RFal_TransceiveRunBlockingTx(void);

/* -------------------------------------------------------------------------- */
/* NFCA */
/* -------------------------------------------------------------------------- */
NFC_OpResult RFal_ISO14443ATransceiveShortFrame(RFal_14443AShortFrameCmd txCmd, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *rxRcvdLen, uint32_t fwt);
NFC_OpResult RFal_ISO14443ATransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt);
NFC_OpResult RFal_ISO14443AStartTransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt);
NFC_OpResult RFal_ISO14443AGetTransceiveAnticollisionFrameStatus(void);

/* -------------------------------------------------------------------------- */
/* NFCV */
/* -------------------------------------------------------------------------- */
NFC_OpResult RFal_ISO15693TransceiveAnticollisionFrame(uint8_t *txBuf, uint8_t txBufLen, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen);
NFC_OpResult RFal_ISO15693TransceiveEOFAnticollision(uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen);
NFC_OpResult RFal_ISO15693TransceiveEOF(uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen);

/* -------------------------------------------------------------------------- */
/* NFCF / FeliCa */
/* -------------------------------------------------------------------------- */
NFC_OpResult RFal_FeliCaPoll(RFal_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, RFal_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected);
NFC_OpResult RFal_StartFeliCaPoll(RFal_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, RFal_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected);
NFC_OpResult RFal_GetFeliCaPollStatus(void);

/* -------------------------------------------------------------------------- */
/* Listen Mode */
/* -------------------------------------------------------------------------- */
NFC_OpResult RFal_ListenStart(uint32_t lmMask, const RFal_LmConfPA *confA, const RFal_LmConfPB *confB, const RFal_LmConfPF *confF, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen);
NFC_OpResult RFal_ListenStop(void);
NFC_OpResult RFal_ListenSleepStart(RFal_LmState sleepSt, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen);
RFal_LmState RFal_ListenGetState(bool *dataFlag, RFal_BitRate *lastBR);
NFC_OpResult RFal_ListenSetState(RFal_LmState newSt);

/* -------------------------------------------------------------------------- */
/* Wake-Up Mode */
/* -------------------------------------------------------------------------- */
NFC_OpResult RFal_WakeUpModeStart(const RFal_WakeUpConfig *config);
bool RFal_WakeUpModeIsEnabled(void);
NFC_OpResult RFal_WakeUpModeGetInfo(bool force, RFal_WakeUpInfo *info);
bool RFal_WakeUpModeHasWoke(void);
NFC_OpResult RFal_WakeUpModeStop(void);

/* -------------------------------------------------------------------------- */
/* RF Chip */
/* -------------------------------------------------------------------------- */
bool RFal_ChipIsBusy(void);
NFC_OpResult RFal_ChipWriteReg(uint16_t reg, const uint8_t *values, uint8_t len);
NFC_OpResult RFal_ChipReadReg(uint16_t reg, uint8_t *values, uint8_t len);
NFC_OpResult RFal_ChipExecCmd(uint16_t cmd);
NFC_OpResult RFal_ChipWriteTestReg(uint16_t reg, uint8_t value);
NFC_OpResult RFal_ChipReadTestReg(uint16_t reg, uint8_t *value);
NFC_OpResult RFal_ChipChangeRegBits(uint16_t reg, uint8_t valueMask, uint8_t value);
NFC_OpResult RFal_ChipChangeTestRegBits(uint16_t reg, uint8_t valueMask, uint8_t value);
NFC_OpResult RFal_ChipSetRFO(uint8_t rfo);
NFC_OpResult RFal_ChipGetRFO(uint8_t *result);
NFC_OpResult RFal_ChipMeasureAmplitude(uint8_t *result);
NFC_OpResult RFal_ChipMeasurePhase(uint8_t *result);
NFC_OpResult RFal_ChipMeasureCapacitance(uint8_t *result);
NFC_OpResult RFal_ChipMeasurePowerSupply(uint8_t param, uint8_t *result);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_H */
