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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "NeonRTOS.h"

#include "ST25R95_IO.h"

#include "ST25R95.h"

#include "RFal_ST25R95_AnalogConfig.h"
#include "RFal_ST25R95.h"

#include "NFC/RFal/RFal.h"

#include "NFC_Config.h"

#ifdef CONFIG_NFC_READER_DEVICE_ST25R95

#define RFal_CreateByteFlagsTxRxContext( ctx, tB, tBL, rB, rBL, rdL, fl, t ) \
    (ctx).txBuf     = (uint8_t*)(tB);                                       \
    (ctx).txBufLen  = (uint16_t)RFal_ConvBytesToBits(tBL);                   \
    (ctx).rxBuf     = (uint8_t*)(rB);                                       \
    (ctx).rxBufLen  = (uint16_t)RFal_ConvBytesToBits(rBL);                   \
    (ctx).rxRcvdLen = (uint16_t*)(rdL);                                     \
    (ctx).flags     = (uint32_t)(fl);                                       \
    (ctx).fwt       = (uint32_t)(t);

static RFal_ST25R95 gRFAL;              /*!< RFAL module instance */

static NFC_OpResult RFal_RunListenModeWorker(void);
static NFC_OpResult RFal_RunWakeUpModeWorker(void);
static NFC_OpResult RFal_RunTransceiveWorker(void);


static bool    gST25R95ListenRunning = false;
static uint8_t gST25R95ACState       = ST25R95_ACSTATE_IDLE;

static RFal_LmState ST25R95_MapACStateToLmState(uint8_t acState)
{
    switch (acState) {
        case ST25R95_ACSTATE_IDLE:
            return RFAL_LM_STATE_IDLE;

        case ST25R95_ACSTATE_READYA:
            return RFAL_LM_STATE_READY_A;

        case ST25R95_ACSTATE_READYAX:
            return RFAL_LM_STATE_READY_Ax;

        case ST25R95_ACSTATE_ACTIVE:
            return RFAL_LM_STATE_ACTIVE_A;

        case ST25R95_ACSTATE_ACTIVEX:
            return RFAL_LM_STATE_ACTIVE_Ax;

        case ST25R95_ACSTATE_HALT:
            return RFAL_LM_STATE_SLEEP_A;

        default:
            return RFAL_LM_STATE_NOT_INIT;
    }
}

NFC_OpResult ST25R95_SetACFilter(const RFal_LmConfPA *confA)
{
    if (confA == NULL) {
        return NFC_InvalidParameter;
    }

    /*
     * 最小可編譯版本：
     * 目前先只保存 RFAL Listen state。
     *
     * 若你的 ST25R95.c / ST25R95_IO.c 已經有真正的 AC Filter command，
     * 可以在這裡接：
     *
     * return ST25R95_Set_ACFilter(confA);
     */
    gST25R95ACState = ST25R95_ACSTATE_IDLE;
    return NFC_OK;
}


bool ST25R95_IsInListen(void)
{
    return gST25R95ListenRunning;
}


NFC_OpResult ST25R95_Listen(void)
{
    /*
     * ST25R95 Listen command 的真正啟動點。
     *
     * 如果你底層已有 Listen command，這裡應接到底層：
     *
     * ret = ST25R95_IO_SPI_Listen(...);
     *
     * 目前先做 RFAL 狀態 glue，避免 linker fail。
     */
    gST25R95ListenRunning = true;
    return NFC_OK;
}


NFC_OpResult ST25R95_RearmListen(void)
{
    gST25R95ListenRunning = false;
    return ST25R95_Listen();
}


RFal_LmState ST25R95_GetLmState(void)
{
    return ST25R95_MapACStateToLmState(gST25R95ACState);
}


NFC_OpResult ST25R95_SetACState(uint8_t acState)
{
    switch (acState) {
        case ST25R95_ACSTATE_IDLE:
        case ST25R95_ACSTATE_READYA:
        case ST25R95_ACSTATE_READYAX:
        case ST25R95_ACSTATE_ACTIVE:
        case ST25R95_ACSTATE_ACTIVEX:
        case ST25R95_ACSTATE_HALT:
            gST25R95ACState = acState;
            return NFC_OK;

        default:
            return NFC_InvalidParameter;
    }
}

/*******************************************************************************/
NFC_OpResult RFal_Init(void)
{
  /* Initialize chip */
  if (ST25R95_IO_Init() != NFC_OK) {
    return (NFC_System);
  }

  /* Initialize Analog Configs */
  if (RFal_AnalogConfig_Init() != NFC_OK) {
    return (NFC_System);
  }

  /* Check expected chip: ST25R95 */
  if (!ST25R95_CheckChipID()) {
    return NFC_Hw_Mismatch;
  }

  /*******************************************************************************/
  /* Debug purposes */
  /*LogSetLevel(LOG_MODULE_DEFAULT, LOG_LEVEL_INFO); !!!!!!!!!!!!!!! */

  /*******************************************************************************/
  gRFAL.state              = RFAL_STATE_INIT;
  gRFAL.mode               = RFAL_MODE_NONE;
  gRFAL.protocol           = ST25R95_Protocol_FieldOff;
  gRFAL.field              = false;

  /* Disable all timings */
  gRFAL.timings.FDTListen  = RFAL_TIMING_NONE;
  gRFAL.timings.FDTPoll    = RFAL_TIMING_NONE;
  gRFAL.timings.GT         = RFAL_TIMING_NONE;

  gRFAL.tmr.GT             = RFAL_TIMING_NONE;
  gRFAL.tmr.FDTPoll        = RFAL_TIMING_NONE;

  gRFAL.callbacks.preTxRx  = NULL;
  gRFAL.callbacks.postTxRx = NULL;

  /* Initialize Wake-Up Mode */
  gRFAL.wum.state = RFAL_WUM_STATE_NOT_INIT;
  gRFAL.wum.CalTagDet = ST25R95_TAGDETECT_DEF_CALIBRATION;

  if (gRFAL.wum.CalTagDet == 0xFFU) {
    return NFC_System;
  }
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_Calibrate(void)
{
  return (NFC_OK);
}

/*******************************************************************************/
NFC_OpResult RFal_AdjustRegulators(uint16_t *result)
{
  return (NFC_OK);
}

/*******************************************************************************/
void RFal_SetUpperLayerCallback(RFal_UpperLayerCallback pFunc)
{
  return;
}

/*******************************************************************************/
void RFal_SetPreTxRxCallback(RFal_PreTxRxCallback pFunc)
{
  gRFAL.callbacks.preTxRx = pFunc;
}

/*******************************************************************************/
void RFal_SetSyncTxRxCallback(RFal_SyncTxRxCallback pFunc)
{
  return;   /* NFC_Unsupport */
}

/*******************************************************************************/
void RFal_SetPostTxRxCallback(RFal_PostTxRxCallback pFunc)
{
  gRFAL.callbacks.postTxRx = pFunc;
}

/*******************************************************************************/
void RFal_SetLmEonCallback(RFal_LmEonCallback pFunc)
{
  return;   /* NFC_Unsupport */
}

/*******************************************************************************/
NFC_OpResult RFal_DeInit(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }

  gRFAL.state = RFAL_STATE_IDLE;

  /* Deinitialize chip */
  return RFal_IO_DeInit();
}

/*******************************************************************************/
void RFal_SetObsvMode(uint32_t txMode, uint32_t rxMode)
{
  return;
}

/*******************************************************************************/
void RFal_GetObsvMode(uint8_t *txMode, uint8_t *rxMode)
{
  return;
}

/*******************************************************************************/
void RFal_DisableObsvMode(void)
{
  return;
}

/*******************************************************************************/
NFC_OpResult RFal_SetMode(RFal_Mode mode, RFal_BitRate txBR, RFal_BitRate rxBR)
{
  /* Check if RFAL is not initialized */
  if (gRFAL.state == RFAL_STATE_IDLE) {
    return NFC_WrongState;
  }

  /* Check allowed bit rate value */
  if ((txBR == RFAL_BR_KEEP) || (rxBR == RFAL_BR_KEEP)) {
    return NFC_InvalidParameter;
  }

  /*******************************************************************************/
  /* Ensure that no previous operation is still ongoing */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }


  switch (mode) {
    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCA:
    case RFAL_MODE_POLL_NFCA_T1T:
      gRFAL.protocol = ST25R95_Protocol_ISO14443A;
      gRFAL.nfcaData.NfcaSplitFrame = false;
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_POLL | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_TX));
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_POLL | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_RX));
      break;
    case RFAL_MODE_POLL_NFCB:
      gRFAL.protocol = ST25R95_Protocol_ISO14443B;
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_POLL | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCB | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_TX));
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_POLL | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCB | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_RX));
      break;
    case RFAL_MODE_POLL_NFCF:
      gRFAL.protocol = ST25R95_Protocol_ISO18092;
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_POLL | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCF | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_TX));
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_POLL | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCF | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_RX));
      break;
    case RFAL_MODE_POLL_NFCV:
      gRFAL.protocol = ST25R95_Protocol_ISO15693;
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_POLL | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCV | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_TX));
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_POLL | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCV | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_RX));
      break;
    case RFAL_MODE_LISTEN_NFCA:
      gRFAL.protocol = ST25R95_Protocol_CE_ISO14443A;
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_LISTEN | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_TX));
      RFal_AnalogConfig_Set((RFAL_ST25R95_ANALOG_CONFIG_LISTEN | RFAL_ST25R95_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R95_ANALOG_CONFIG_RX));
      break;
    /*******************************************************************************/
    case RFAL_MODE_POLL_B_PRIME:
    case RFAL_MODE_POLL_B_CTS:
    case RFAL_MODE_POLL_PICOPASS:
    case RFAL_MODE_POLL_ACTIVE_P2P:
    case RFAL_MODE_LISTEN_ACTIVE_P2P:
    case RFAL_MODE_LISTEN_NFCB:
    case RFAL_MODE_LISTEN_NFCF:
      return NFC_Unsupport;
      /*NOTREACHED*/
      break;

    default:
      return NFC_InvalidParameter;
  }

  /* Set state as STATE_MODE_SET only if not initialized yet (PSL) */
  gRFAL.state = ((gRFAL.state < RFAL_STATE_MODE_SET) ? RFAL_STATE_MODE_SET : gRFAL.state);
  gRFAL.mode  = mode;

  /* Apply the given bit rate and mode */
  return (RFal_SetBitRate(txBR, rxBR));
}

/*******************************************************************************/
RFal_Mode RFal_GetMode(void)
{
  return gRFAL.mode;
}

/*******************************************************************************/
NFC_OpResult RFal_SetBitRate(RFal_BitRate txBR, RFal_BitRate rxBR)
{
  NFC_OpResult retCode = NFC_OK;

  /* Check if RFAL is not initialized */
  if (gRFAL.state == RFAL_STATE_IDLE) {
    return NFC_WrongState;
  }

  /*******************************************************************************/
  /* Ensure that no previous operation is still ongoing */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }


  /* Store the new Bit Rates */
  gRFAL.txBR = ((txBR == RFAL_BR_KEEP) ? gRFAL.txBR : txBR);
  gRFAL.rxBR = ((rxBR == RFAL_BR_KEEP) ? gRFAL.rxBR : rxBR);

  retCode = ST25R95_Set_BitRate(gRFAL.protocol, txBR, rxBR);
  if ((retCode == NFC_OK) && (gRFAL.protocol != ST25R95_Protocol_FieldOff)) {
    /* If field on, update bitrate value through ProtocolSelect */
    retCode = ST25R95_Protocol_Select(gRFAL.protocol);
  }

  return (retCode);
}

/*******************************************************************************/
NFC_OpResult RFal_GetBitRate(RFal_BitRate *txBR, RFal_BitRate *rxBR)
{
  if ((gRFAL.state == RFAL_STATE_IDLE) || (gRFAL.mode == RFAL_MODE_NONE)) {
    return NFC_WrongState;
  }

  if (txBR != NULL) {
    *txBR = gRFAL.txBR;
  }

  if (rxBR != NULL) {
    *rxBR = gRFAL.rxBR;
  }

  return NFC_OK;
}

/*******************************************************************************/
void RFal_SetErrorHandling(RFal_EHandling eHandling)
{
  return;
}

/*******************************************************************************/
RFal_EHandling RFal_GetErrorHandling(void)
{
  return ERRORHANDLING_NONE;
}

/*******************************************************************************/
void RFal_SetFDTPoll(uint32_t FDTPoll)
{
  gRFAL.timings.FDTPoll = (FDTPoll > RFAL_ST25R95_GPT_MAX_1FC) ? RFAL_ST25R95_GPT_MAX_1FC : FDTPoll;
}

/*******************************************************************************/
uint32_t RFal_GetFDTPoll(void)
{
  return gRFAL.timings.FDTPoll;
}

/*******************************************************************************/
void RFal_SetFDTListen(uint32_t FDTListen)
{
  gRFAL.timings.FDTListen = (FDTListen > RFAL_ST25R95_MRT_MAX_1FC) ? RFAL_ST25R95_MRT_MAX_1FC : FDTListen;
}

/*******************************************************************************/
uint32_t RFal_GetFDTListen(void)
{
  return gRFAL.timings.FDTListen;
}

/*******************************************************************************/
void RFal_SetGT(uint32_t GT)
{
  gRFAL.timings.GT = (GT > RFAL_ST25R95_GT_MAX_1FC) ? RFAL_ST25R95_GT_MAX_1FC : GT;
}

/*******************************************************************************/
uint32_t RFal_GetGT(void)
{
  return gRFAL.timings.GT;
}

/*******************************************************************************/
bool RFal_IsGTExpired(void)
{
  if (gRFAL.tmr.GT != RFAL_TIMING_NONE) {
    if (!RFal_TimerisExpired(gRFAL.tmr.GT)) {
      return false;
    }
  }
  return true;
}

/*******************************************************************************/
NFC_OpResult RFal_FieldOnAndStartGT(void)
{
  NFC_OpResult ret;

  /* Check if RFAL has been initialized  */
  if ((gRFAL.state < RFAL_STATE_INIT)) {
    return NFC_WrongState;
  }

  /*******************************************************************************/
  /* Ensure that no previous operation is still ongoing */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }


  ret = NFC_OK;

  /*******************************************************************************/
  /* Turn field On if not already On */
  if (!gRFAL.field) {
    ret = ST25R95_Field_On(gRFAL.protocol);
    gRFAL.field = true;
  }

  /*******************************************************************************/
  /* Start GT timer in case the GT value is set */
  if ((gRFAL.timings.GT != RFAL_TIMING_NONE)) {
    /* Ensure that a SW timer doesn't have a lower value then the minimum  */
    RFal_TimerStart(gRFAL.tmr.GT, RFal_Conv1fcToMs(((gRFAL.timings.GT) > RFAL_ST25R95_GT_MIN_1FC) ? (gRFAL.timings.GT) : RFAL_ST25R95_GT_MIN_1FC));
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_FieldOff(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }

  RFal_WakeUpModeStop();
  gRFAL.field = false;
  gRFAL.protocol = ST25R95_Protocol_FieldOff;
  return (ST25R95_Field_Off());
}

/*******************************************************************************/
NFC_OpResult RFal_StartTransceive(const RFal_TransceiveContext *ctx)
{
  /* Ensure that RFAL is already Initialized and the mode has been set */
  if ((gRFAL.state >= RFAL_STATE_MODE_SET)) {
    /*******************************************************************************/
    /* Ensure that no previous operation is still ongoing */
    if (RFal_ChipIsBusy()) {
      return NFC_RequestError;
    }

    gRFAL.TxRx.ctx = *ctx;

    /*******************************************************************************/
    if (RFal_IsModePassiveComm(gRFAL.mode)) { /* Passive Comms */
      if ((gRFAL.TxRx.ctx.fwt != RFAL_FWT_NONE) && (gRFAL.TxRx.ctx.fwt != 0)) {
        ST25R95_Set_FWT(gRFAL.protocol, gRFAL.TxRx.ctx.fwt);
      } else {
        /* Since ST25R95 does not support, use max FWT available */
        ST25R95_Set_FWT(gRFAL.protocol, ST25R95_FWT_MAX);
      }
    }

    gRFAL.state       = RFAL_STATE_TXRX;
    gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_IDLE;
    gRFAL.TxRx.status = NFC_Busy;
    gRFAL.Lm.dataFlag = false;

    /*******************************************************************************/
    if ((RFAL_MODE_POLL_NFCV == gRFAL.mode) || (RFAL_MODE_POLL_PICOPASS == gRFAL.mode)) {
      /* In NFCV a TxRx with a valid txBuf and txBufSize==0 indicates to send an EOF */
      /* Skip logic below that would go directly into receive                        */
      if (gRFAL.TxRx.ctx.txBuf != NULL) {
        return  NFC_OK;
      }
    }

    /*******************************************************************************/
    /* Check if the Transceive start performing Tx or goes directly to Rx          */
    if ((gRFAL.TxRx.ctx.txBuf == NULL) || (gRFAL.TxRx.ctx.txBufLen == 0)) {
      return NFC_InvalidParameter;
    }

    return NFC_OK;
  }

  return NFC_WrongState;
}

/*******************************************************************************/
bool RFal_IsTransceiveInTx(void)
{
  return ((gRFAL.TxRx.state >= RFAL_TXRX_STATE_TX_IDLE) && (gRFAL.TxRx.state < RFAL_TXRX_STATE_RX_IDLE));
}

/*******************************************************************************/
bool RFal_IsTransceiveInRx(void)
{
  return (gRFAL.TxRx.state >= RFAL_TXRX_STATE_RX_IDLE);
}

/*******************************************************************************/
NFC_OpResult RFal_TransceiveBlockingTx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt)
{
  NFC_OpResult               ret;
  RFal_TransceiveContext    ctx;

  RFal_CreateByteFlagsTxRxContext(ctx, txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt);
  ret = RFal_StartTransceive(&ctx);
  if(ret < NFC_OK)
  {
      return ret;
  }

  return RFal_TransceiveRunBlockingTx();
}

/*******************************************************************************/
NFC_OpResult RFal_TransceiveBlockingRx(void)
{
  NFC_OpResult ret;

  do {
    RFal_Worker();
  } while (((ret = RFal_GetTransceiveStatus()) == NFC_Busy) && RFal_IsTransceiveInRx());

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_TransceiveBlockingTxRx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt)
{
  NFC_OpResult ret;

  ret = RFal_TransceiveBlockingTx(txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt);
  if(ret < NFC_OK)
  {
      return ret;
  }

  ret = RFal_TransceiveBlockingRx();

  /* Convert received bits to bytes */
  if (actLen != NULL) {
    *actLen = RFal_ConvBitsToBytes(*actLen);
  }

  return ret;
}

/*******************************************************************************/
static NFC_OpResult RFal_RunTransceiveWorker(void)
{
  if (gRFAL.state == RFAL_STATE_TXRX) {
    /* Run Tx or Rx state machines */
    if (RFal_IsTransceiveInTx()) {
      RFal_TransceiveTx();
      return RFal_GetTransceiveStatus();
    } else if (RFal_IsTransceiveInRx()) {
      RFal_TransceiveRx();
      return RFal_GetTransceiveStatus();
    }
  }
  return NFC_WrongState;
}

/*******************************************************************************/
RFal_TransceiveState RFal_GetTransceiveState(void)
{
  return gRFAL.TxRx.state;
}

NFC_OpResult RFal_GetTransceiveStatus(void)
{
  return ((gRFAL.TxRx.state == RFAL_TXRX_STATE_IDLE) ? gRFAL.TxRx.status : NFC_Busy);
}

/*******************************************************************************/
NFC_OpResult RFal_GetTransceiveRSSI(uint16_t *rssi)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
bool RFal_IsTransceiveSubcDetected(void)
{
  return false;
}

/*******************************************************************************/
void RFal_Worker(void)
{
  switch (gRFAL.state) {
    case RFAL_STATE_TXRX:
      RFal_RunTransceiveWorker();
      break;

    case RFAL_STATE_LM:
      RFal_RunListenModeWorker();
      break;

    case RFAL_STATE_WUM:
      RFal_RunWakeUpModeWorker();
      break;

    /* Nothing to be done */
    default:
      break;
  }
}

/*******************************************************************************/
void RFal_TransceiveTx(void)
{
  uint8_t transmitFlag = 0;

  if (gRFAL.TxRx.state != gRFAL.TxRx.lastState) {
    /* RFal_LogD("%s: lastSt: %d curSt: %d \r\n", __FUNCTION__, gRFAL.TxRx.lastState, gRFAL.TxRx.state);*/
    gRFAL.TxRx.lastState = gRFAL.TxRx.state;
  }

  switch (gRFAL.TxRx.state) {
    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_IDLE:
      /* Nothing to do */
      gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_WAIT_GT ;
    /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_GT:
      /* Wait for GT and FDT Poll */

      if (!RFal_IsGTExpired() || !RFal_TimerisExpired(gRFAL.tmr.FDTPoll)) {
        break;
      }
      gRFAL.tmr.GT = RFAL_TIMING_NONE;
      gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_TRANSMIT;
    /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_TRANSMIT:
      /*******************************************************************************/
      /* Execute Pre Transceive Callback                                             */
      /*******************************************************************************/
      if (gRFAL.callbacks.preTxRx != NULL) {
        gRFAL.callbacks.preTxRx();
      }
      /*******************************************************************************/
      /* Prepare Rx                                                                  */
      /*******************************************************************************/
      ST25R95_IO_SPI_PrepareRx(
        gRFAL.protocol,
        gRFAL.TxRx.ctx.rxBuf,
        RFal_ConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen),
        gRFAL.TxRx.ctx.rxRcvdLen,
        gRFAL.TxRx.ctx.flags,
        gRFAL.RxInformationBytes
      );
      /*******************************************************************************/
      /* Send the data                                                               */
      /*******************************************************************************/
      ST25R95_IO_SPI_Send_Data(gRFAL.TxRx.ctx.txBuf, RFal_ConvBitsToBytes(gRFAL.TxRx.ctx.txBufLen), gRFAL.protocol, gRFAL.TxRx.ctx.flags);

      /* Start FDTPoll SW timer */
      RFal_TimerStart(gRFAL.tmr.FDTPoll, (RFAL_ST25R95_SW_TMR_MIN_1MS + RFal_Conv1fcToMs(gRFAL.timings.FDTPoll)));

      gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_WAIT_TXE;
    /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_TXE:
      transmitFlag = gRFAL.TxRx.ctx.txBufLen % 8;
      if (transmitFlag == 0) {
        transmitFlag = 0x8;
      }
      if (!(gRFAL.TxRx.ctx.flags & RFAL_TXRX_FLAGS_CRC_TX_MANUAL)) {
        transmitFlag |= RFAL_ST25R95_ISO14443A_APPENDCRC;
      }
      if (gRFAL.nfcaData.NfcaSplitFrame) {
        transmitFlag |= RFAL_ST25R95_ISO14443A_SPLITFRAME;
      }
      if (gRFAL.mode == RFAL_MODE_POLL_NFCA_T1T) {
        transmitFlag |= RFAL_ST25R95_ISO14443A_TOPAZFORMAT;
      }
      ST25R95_IO_SPI_Send_Transmit_Flag(gRFAL.protocol, transmitFlag);
      if (gRFAL.protocol == ST25R95_Protocol_CE_ISO14443A) {
        ST25R95_RearmListen();
      }
      gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_DONE;
    /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_DONE:
      /* If no rxBuf is provided do not wait/expect Rx */
      if (gRFAL.TxRx.ctx.rxBuf == NULL) {
        /* Clean up Transceive */
        //RFal_CleanupTransceive();
        gRFAL.TxRx.status = NFC_OK;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_IDLE;
        break;
      }
      /* Goto Rx */
      gRFAL.TxRx.state  =  RFAL_TXRX_STATE_RX_IDLE;
      break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_FAIL:
      /* Error should be assigned by previous state */
      if (gRFAL.TxRx.status == NFC_Busy) {
        gRFAL.TxRx.status = NFC_System;
      }
      gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
      break;

    /*******************************************************************************/
    default:
      gRFAL.TxRx.status = NFC_System;
      gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_FAIL;
      break;
  }
}

/*******************************************************************************/
void RFal_TransceiveRx(void)
{
  NFC_OpResult retCode;

  if (gRFAL.TxRx.state != gRFAL.TxRx.lastState) {
    /*RFal_LogD("%s: lastSt: %d curSt: %d \r\n", __FUNCTION__, gRFAL.TxRx.lastState, gRFAL.TxRx.state);*/
    gRFAL.TxRx.lastState = gRFAL.TxRx.state;
  }

  switch (gRFAL.TxRx.state) {
    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_IDLE:

      /* Clear rx counters */
      if (gRFAL.TxRx.ctx.rxRcvdLen)  {
        *gRFAL.TxRx.ctx.rxRcvdLen = 0;
      }

      gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXE;
    /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_WAIT_RXE:
      if (ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_TIMEOUT) == NFC_SlaveTimeout) {
        break;
      }
      gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_DATA;
    /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_READ_DATA:
      retCode = ST25R95_IO_SPI_Complete_Rx();
      /* Re-Start FDTPoll SW timer */
      RFal_TimerStart(gRFAL.tmr.FDTPoll, (RFAL_ST25R95_SW_TMR_MIN_1MS + RFal_Conv1fcToMs(gRFAL.timings.FDTPoll)));

      if (gRFAL.TxRx.ctx.rxRcvdLen != NULL) {
        (*gRFAL.TxRx.ctx.rxRcvdLen) = RFal_ConvBytesToBits(*gRFAL.TxRx.ctx.rxRcvdLen);

        /*******************************************************************************/
        /* In case of Incomplete byte append the residual bits                         */
        /*******************************************************************************/
        if ((retCode <= NFC_ImcompleteByte_01) && (retCode >= NFC_ImcompleteByte_07)) {
          (*gRFAL.TxRx.ctx.rxRcvdLen) += (NFC_ImcompleteByte_00 - retCode);

          if ((*gRFAL.TxRx.ctx.rxRcvdLen) > 0) {
            (*gRFAL.TxRx.ctx.rxRcvdLen) -= RFAL_BITS_IN_BYTE;
          }
        }
      }

      /*******************************************************************************/
      /* Execute Post Transceive Callback                                            */
      /*******************************************************************************/
      if (gRFAL.callbacks.postTxRx != NULL) {
        gRFAL.callbacks.postTxRx();
      }

      if (retCode != NFC_OK) {
        gRFAL.TxRx.status = retCode;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
        break;
      }
      gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_DONE;
    /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_DONE:
      gRFAL.TxRx.status = NFC_OK;
      gRFAL.TxRx.state  = RFAL_TXRX_STATE_IDLE;
      break;


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_FAIL:
      gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
      break;

    /*******************************************************************************/
    default:
      gRFAL.TxRx.status = NFC_System;
      gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
      break;
  }
}

/*******************************************************************************/
NFC_OpResult RFal_TransceiveRunBlockingTx(void)
{
  NFC_OpResult ret;

  do {
    RFal_Worker();
  } while (((ret = RFal_GetTransceiveStatus()) == NFC_Busy) && RFal_IsTransceiveInTx());

  if (RFal_IsTransceiveInRx()) {
    return NFC_OK;
  }

  return ret;
}

/*******************************************************************************/
bool RFal_ChipIsBusy(void)
{
  /* ST25R95 cannot be interrupted while an operation is ongoing */

  /* Check whether a Transceive operation is still running */
  if ((gRFAL.state == RFAL_STATE_TXRX) && (gRFAL.TxRx.state > RFAL_TXRX_STATE_TX_IDLE)) {
    return (true);
  }

  return (false);
}

/*****************************************************************************
 *  NFCA Mode                                                              *
 *****************************************************************************/
/*******************************************************************************/
NFC_OpResult RFal_ISO14443ATransceiveShortFrame(RFal_14443AShortFrameCmd txCmd, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *rxRcvdLen, uint32_t fwt)
{
  NFC_OpResult ret;
  RFal_TransceiveContext ctx;
  uint8_t st95hShortFrameBuffer;
  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < RFAL_STATE_MODE_SET) || ((gRFAL.mode != RFAL_MODE_POLL_NFCA) && (gRFAL.mode != RFAL_MODE_POLL_NFCA_T1T)) || !gRFAL.field) {
    return NFC_WrongState;
  }

  /* Check for valid parameters */
  if ((rxBuf == NULL) || (rxRcvdLen == NULL) || (fwt == RFAL_FWT_NONE)) {
    return NFC_InvalidParameter;
  }

  gRFAL.nfcaData.NfcaSplitFrame = false;
  /*******************************************************************************/
  /* Update the short frame buffer with the REQA or WUPA command                 */
  st95hShortFrameBuffer =  txCmd;


  ctx.flags     = (RFAL_TXRX_FLAGS_CRC_TX_MANUAL | RFAL_TXRX_FLAGS_CRC_RX_KEEP);
  ctx.txBuf     = &st95hShortFrameBuffer;
  ctx.txBufLen  = 7;
  ctx.rxBuf     = rxBuf;
  ctx.rxBufLen  = rxBufLen;
  ctx.rxRcvdLen = rxRcvdLen;
  ctx.fwt       = fwt;

  RFal_StartTransceive(&ctx);

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = RFal_TransceiveRunBlockingTx();
  if (ret == NFC_OK) {
    ret = RFal_TransceiveBlockingRx();
  }

  /* ST25R95 has no means to disable CRC check, discard CRC errors */
  if (ret == NFC_CRC_Error) {
    ret = NFC_OK;
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO14443ATransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt)
{
  NFC_OpResult            ret;

  ret = RFal_ISO14443AStartTransceiveAnticollisionFrame(buf, bytesToSend, bitsToSend, rxLength, fwt);
  if(ret < NFC_OK)
  {
      return ret;
  }

  do{
    ret = RFal_ISO14443AGetTransceiveAnticollisionFrameStatus();
    RFal_Worker();
  }while( ret == NFC_Busy );

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO14443AStartTransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt)
{
  NFC_OpResult            ret;
  RFal_TransceiveContext ctx;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCA)) {
    return NFC_WrongState;
  }

  /* Check for valid parameters */
  if ((buf == NULL) || (bytesToSend == NULL) || (bitsToSend == NULL) || (rxLength == NULL)) {
    return NFC_InvalidParameter;
  }

  gRFAL.nfcaData.NfcaSplitFrame = true;

  /*******************************************************************************/
  /* Prepare for Transceive                                                      */
  ctx.flags     = (RFAL_TXRX_FLAGS_CRC_TX_MANUAL | RFAL_TXRX_FLAGS_CRC_RX_KEEP);
  ctx.txBuf     = buf;
  ctx.txBufLen  = (RFal_ConvBytesToBits(*bytesToSend) + *bitsToSend);
  ctx.rxBuf     = (buf + (*bytesToSend));
  ctx.rxBufLen  = RFal_ConvBytesToBits(RFAL_ISO14443A_SDD_RES_LEN);
  ctx.rxRcvdLen = rxLength;
  ctx.fwt       = fwt;

  ret = RFal_StartTransceive(&ctx);
  if(ret < NFC_OK)
  {
      return ret;
  }

  /*******************************************************************************/
  gRFAL.nfcaData.collByte = 0;

  /* save the collision byte */
  if ((*bitsToSend) > 0U) {
    buf[(*bytesToSend)] <<= (RFAL_BITS_IN_BYTE - (*bitsToSend));
    buf[(*bytesToSend)] >>= (RFAL_BITS_IN_BYTE - (*bitsToSend));
    gRFAL.nfcaData.collByte = buf[(*bytesToSend)];
  }

  gRFAL.nfcaData.buf         = buf;
  gRFAL.nfcaData.bytesToSend = bytesToSend;
  gRFAL.nfcaData.bitsToSend  = bitsToSend;
  gRFAL.nfcaData.rxLength    = rxLength;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO14443AGetTransceiveAnticollisionFrameStatus(void)
{
  NFC_OpResult   ret;

  /*******************************************************************************/
  /* Run Transceive blocking Tx*/
  ret = RFal_TransceiveRunBlockingTx();
  if (ret == NFC_OK) {
    ret = RFal_TransceiveBlockingRx();
    /* ignore CRC error */
    if (ret == NFC_CRC_Error) {
      ret = NFC_OK;
    }

    /*******************************************************************************/
    if ((*gRFAL.nfcaData.bitsToSend) > 0U) {
      gRFAL.nfcaData.buf[(*gRFAL.nfcaData.bytesToSend)] >>= (*gRFAL.nfcaData.bitsToSend);
      gRFAL.nfcaData.buf[(*gRFAL.nfcaData.bytesToSend)] <<= (*gRFAL.nfcaData.bitsToSend);
      gRFAL.nfcaData.buf[(*gRFAL.nfcaData.bytesToSend)] |= gRFAL.nfcaData.collByte;
    }

    if ((ret == NFC_RF_Collision)) {
      (*gRFAL.nfcaData.rxLength) = RFal_ConvBytesToBits(gRFAL.RxInformationBytes[1]);
      (*gRFAL.nfcaData.bytesToSend) = (gRFAL.RxInformationBytes[1] + (*gRFAL.nfcaData.bytesToSend)) & 0xF;
      (*gRFAL.nfcaData.bitsToSend)  = gRFAL.RxInformationBytes[2] & 0x7;
    }
  }
  gRFAL.nfcaData.NfcaSplitFrame = false;
  return ret;
}

/*****************************************************************************
 *  NFCV Mode                                                              *
 *****************************************************************************/
/*******************************************************************************/
NFC_OpResult RFal_ISO15693TransceiveAnticollisionFrame(uint8_t *txBuf, uint8_t txBufLen, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
  NFC_OpResult            ret;
  RFal_TransceiveContext ctx;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCV)) {
    return NFC_WrongState;
  }

  /*******************************************************************************/
  /* Prepare for Transceive  */
  ctx.flags     = (((txBufLen == 0) ? RFAL_TXRX_FLAGS_CRC_TX_MANUAL : RFAL_TXRX_FLAGS_CRC_TX_AUTO) | RFAL_TXRX_FLAGS_CRC_RX_KEEP | RFAL_TXRX_FLAGS_AGC_OFF | ((txBufLen == 0) ? RFAL_TXRX_FLAGS_NFCV_FLAG_MANUAL : RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO)); /* Disable Automatic Gain Control (AGC) for better detection of collision */
  ctx.txBuf     = txBuf;
  ctx.txBufLen  = RFal_ConvBytesToBits(txBufLen);
  ctx.rxBuf     = rxBuf;
  ctx.rxBufLen  = RFal_ConvBytesToBits(rxBufLen);
  ctx.rxRcvdLen = actLen;
  ctx.fwt       = RFAL_FWT_NONE;

  ret = RFal_StartTransceive(&ctx);
  if(ret < NFC_OK)
  {
      return ret;
  }

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = RFal_TransceiveRunBlockingTx();
  if (ret == NFC_OK) {
    ret = RFal_TransceiveBlockingRx();
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO15693TransceiveEOFAnticollision(uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
  uint8_t dummy;

  return RFal_ISO15693TransceiveAnticollisionFrame(&dummy, 0, rxBuf, rxBufLen, actLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO15693TransceiveEOF(uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen)
{
  NFC_OpResult ret;
  uint8_t    dummy;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCV)) {
    return NFC_WrongState;
  }

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = RFal_TransceiveBlockingTxRx(&dummy,
                                            0,
                                            rxBuf,
                                            rxBufLen,
                                            actLen,
                                            ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_AGC_ON),
                                            RFal_Conv64fcTo1fc(RFAL_FWT_NONE));
  return ret;
}

/*****************************************************************************
 *  NFCF Mode                                                              *
 *****************************************************************************/
/*******************************************************************************/
NFC_OpResult RFal_FeliCaPoll(RFal_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, RFal_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected)
{
  NFC_OpResult ret;

  ret = RFal_StartFeliCaPoll(slots, sysCode, reqCode, pollResList, pollResListSize, devicesDetected, collisionsDetected);
  if(ret < NFC_OK)
  {
      return ret;
  }
  
  do{
    ret = RFal_GetFeliCaPollStatus();
    RFal_Worker();
  }while( ret == NFC_Busy );

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_StartFeliCaPoll(RFal_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, RFal_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected)
{
  NFC_OpResult        ret;
  uint8_t           frame[RFAL_FELICA_POLL_REQ_LEN - RFAL_FELICA_LEN_LEN];  // LEN is added by ST25R95 automatically
  uint8_t           frameIdx;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCF)) {
    return NFC_WrongState;
  }

  frameIdx    = 0;
  gRFAL.nfcfData.colDetected = 0;
  gRFAL.nfcfData.devDetected = 0;

  /*******************************************************************************/
  /* Compute SENSF_REQ frame */
  frame[frameIdx++] = (uint8_t)FELICA_CMD_POLLING;       /* CMD: SENF_REQ                       */
  frame[frameIdx++] = (uint8_t)(sysCode >> 8);   /* System Code (SC)                    */
  frame[frameIdx++] = (uint8_t)(sysCode & 0xFFU); /* System Code (SC)                    */
  frame[frameIdx++] = reqCode;                   /* Communication Parameter Request (RC)*/
  frame[frameIdx++] = (uint8_t)slots;            /* TimeSlot (TSN)                      */

  ST25R95_Set_SlotCounter((uint8_t)slots);
  /*******************************************************************************/
  /* NRT should not stop on reception - Use EMVCo mode to run NRT in nrt_emv     *
   * ERRORHANDLING_EMVCO has no special handling for NFC-F mode                  */
  gRFAL.conf.eHandling       = ERRORHANDLING_EMD;

  /*******************************************************************************/
  /* Run transceive blocking,
   * Calculate Total Response Time in(64/fc):
   *                       512 PICC process time + (n * 256 Time Slot duration)  */
  ret = RFal_TransceiveBlockingTx(frame,
                                          (uint16_t)frameIdx,
                                          (uint8_t *)gRFAL.nfcfData.pollResponses,
                                          RFAL_FELICA_POLL_RES_LEN,
                                          &gRFAL.nfcfData.actLen,
                                          RFAL_TXRX_FLAGS_CRC_RX_REMV,
                                          RFal_Conv64fcTo1fc(RFAL_FELICA_POLL_DELAY_TIME + (RFAL_FELICA_POLL_SLOT_TIME * ((uint32_t)slots + 1U))));
  if(ret < NFC_OK)
  {
      return ret;
  }

  /* Store context */
  gRFAL.nfcfData.pollResList        = pollResList;
  gRFAL.nfcfData.pollResListSize    = pollResListSize;
  gRFAL.nfcfData.devicesDetected    = devicesDetected;
  gRFAL.nfcfData.collisionsDetected = collisionsDetected;
  gRFAL.nfcfData.slots              = slots;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_GetFeliCaPollStatus(void)
{
  NFC_OpResult ret;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state != RFAL_STATE_TXRX) || (gRFAL.mode != RFAL_MODE_POLL_NFCF)) {
    return NFC_WrongState;
  }

  /* Wait until transceive has terminated */
  ret = RFal_GetTransceiveStatus();
  if(ret == NFC_Busy)
  {
      return NFC_Busy;
  }

  /*******************************************************************************/
  /* If Tx OK, Wait for first response                                           */
  if (ret == NFC_OK) {
    ret = RFal_TransceiveBlockingRx();
    if (ret != NFC_SlaveTimeout) {
      /* If the reception was OK, new device found */
      if (ret == NFC_OK) {
        gRFAL.nfcfData.devDetected++;
      }
      /* If the reception was not OK, mark as collision */
      else {
        gRFAL.nfcfData.colDetected++;
      }
    }
  }
  ST25R95_Set_SlotCounter((uint8_t)RFAL_FELICA_1_SLOT);
  /*******************************************************************************/
  /* Restore NRT to normal mode - back to previous error handling */
  gRFAL.conf.eHandling = gRFAL.nfcfData.curHandling;

  /*******************************************************************************/
  /* Assign output parameters if requested                                       */

  if ((gRFAL.nfcfData.pollResList != NULL) && (gRFAL.nfcfData.pollResListSize > 0) && (gRFAL.nfcfData.devDetected > 0)) {
    memcpy(gRFAL.nfcfData.pollResList, gRFAL.nfcfData.pollResponses, (RFAL_FELICA_POLL_RES_LEN * (uint32_t)((gRFAL.nfcfData.pollResListSize < gRFAL.nfcfData.devDetected) ? gRFAL.nfcfData.pollResListSize : gRFAL.nfcfData.devDetected)));
  }

  if (gRFAL.nfcfData.devicesDetected != NULL) {
    *gRFAL.nfcfData.devicesDetected = gRFAL.nfcfData.devDetected;
  }

  if (gRFAL.nfcfData.collisionsDetected != NULL) {
    *gRFAL.nfcfData.collisionsDetected = gRFAL.nfcfData.colDetected;
  }

  return ((gRFAL.nfcfData.colDetected || gRFAL.nfcfData.devDetected) ? NFC_OK : ret);
}
/*****************************************************************************
 *  Listen Mode                                                              *
 *****************************************************************************/

/*******************************************************************************/
bool RFal_IsExtFieldOn(void)
{
  return (false);
}

/*******************************************************************************/
NFC_OpResult RFal_ListenStart(uint32_t lmMask, const RFal_LmConfPA *confA, const RFal_LmConfPB *confB, const RFal_LmConfPF *confF, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen)
{
  /* Check if RFAL is not initialized */
  if (gRFAL.state == RFAL_STATE_IDLE) {
    return NFC_WrongState;
  }

  gRFAL.cardEmulT4AT = false;

  /*******************************************************************************/
  /* Check whether a Transceive operation is still ongoing                       *
   * ST25R95 cannot be interrupted while a Transceive is ongoing, reject         */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }

  /*******************************************************************************/
  if ((lmMask & RFAL_LM_MASK_ACTIVE_P2P) || (lmMask & RFAL_LM_MASK_NFCB) || (lmMask & RFAL_LM_MASK_NFCF)) {
    return NFC_Unsupport;
  }

  if ((lmMask & RFAL_LM_MASK_NFCA)) {
    if (confA == NULL) {
      return (NFC_InvalidParameter);
    }

    RFal_SetMode(RFAL_MODE_LISTEN_NFCA, RFAL_BR_106, RFAL_BR_106);

    if (ST25R95_SetACFilter(confA) != NFC_OK) {
      return (NFC_InvalidParameter);
    }

    gRFAL.Lm.rxBuf    = rxBuf;
    gRFAL.Lm.rxBufLen = RFal_ConvBytesToBits(rxBufLen);
    gRFAL.Lm.rxLen    = rxLen;
    *gRFAL.Lm.rxLen   = 0;
    gRFAL.Lm.dataFlag = false;
    gRFAL.state       = RFAL_STATE_LM;

    return NFC_OK;
  }

  return NFC_Unsupport;
}

/*******************************************************************************/
static NFC_OpResult RFal_RunListenModeWorker(void)
{
  NFC_OpResult retCode = NFC_OK;

  if (!ST25R95_IsInListen()) {
    retCode = ST25R95_Listen();
  }

  if (retCode != NFC_OK) {
    return (retCode);
  }

  if (ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_TIMEOUT) == NFC_SlaveTimeout) {
    return (NFC_OK);
  }

  retCode = ST25R95_IO_SPI_Complete_Rx(
    gRFAL.protocol,
    gRFAL.Lm.rxBuf,
    RFal_ConvBitsToBytes(gRFAL.Lm.rxBufLen),
    gRFAL.Lm.rxLen,
    (gRFAL.TxRx.ctx.flags & RFAL_TXRX_FLAGS_CRC_RX_KEEP) != RFAL_TXRX_FLAGS_CRC_RX_KEEP,
    gRFAL.RxInformationBytes
  );
  if (!((retCode == NFC_LinkLoss) || ((retCode == NFC_OK) && (gRFAL.Lm.rxLen == 0)))) {
    *gRFAL.Lm.rxLen   = RFal_ConvBytesToBits(*gRFAL.Lm.rxLen);
    gRFAL.Lm.dataFlag = true;
    gRFAL.state       = RFAL_STATE_MODE_SET;
  }

  return (retCode);
}

/*******************************************************************************/
NFC_OpResult RFal_ListenStop(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }

  ST25R95_IO_SPI_Command_Echo(); /* kill listen command */
  ST25R95_Field_Off();
  gRFAL.state              = RFAL_STATE_INIT;
  gRFAL.mode               = RFAL_MODE_NONE;
  gRFAL.protocol           = ST25R95_Protocol_FieldOff;
  gRFAL.field              = false;
  gRFAL.Lm.dataFlag        = false;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ListenSleepStart(RFal_LmState sleepSt, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen)
{
  NFC_OpResult retCode = NFC_InvalidParameter;

  if (sleepSt == RFAL_LM_STATE_SLEEP_A) {
    gRFAL.state       = RFAL_STATE_LM;
    gRFAL.Lm.rxBuf    = rxBuf;
    gRFAL.Lm.rxBufLen = RFal_ConvBytesToBits(rxBufLen);
    gRFAL.Lm.rxLen    = rxLen;
    *gRFAL.Lm.rxLen   = 0;
    gRFAL.Lm.dataFlag = false;
    RFal_ListenSetState(sleepSt);
    retCode = NFC_OK;
  }

  return retCode;
}

/*******************************************************************************/
RFal_LmState RFal_ListenGetState(bool *dataFlag, RFal_BitRate *lastBR)
{
  RFal_LmState state;

  /* Allow state retrieval even if gRFAL.state != RFAL_STATE_LM so  *
   * that this Lm state can be used by caller after activation      */

  if (lastBR != NULL) {
    *lastBR = gRFAL.txBR;
  }

  if (dataFlag != NULL) {
    *dataFlag = gRFAL.Lm.dataFlag;
  }
  state = ST25R95_GetLmState();

  if (((state == RFAL_LM_STATE_ACTIVE_A) || (state == RFAL_LM_STATE_READY_Ax)) && gRFAL.cardEmulT4AT) {
    state = RFAL_LM_STATE_CARDEMU_4A;
  }

  return (state);
}

/*******************************************************************************/
NFC_OpResult RFal_ListenSetState(RFal_LmState newSt)
{
  NFC_OpResult retCode = NFC_OK;
  uint8_t st25r95State;
  bool WasInListen;

  WasInListen = ST25R95_IsInListen();
  ST25R95_IO_SPI_Command_Echo(); /* kill listen command */
  gRFAL.cardEmulT4AT = false;

  switch (newSt) {
    default:
    case RFAL_LM_STATE_NOT_INIT:
    case RFAL_LM_STATE_POWER_OFF:
    case RFAL_LM_STATE_READY_B:
    case RFAL_LM_STATE_READY_F:
    case RFAL_LM_STATE_CARDEMU_4B:
    case RFAL_LM_STATE_CARDEMU_3:
    case RFAL_LM_STATE_TARGET_A:
    case RFAL_LM_STATE_TARGET_F:
    case RFAL_LM_STATE_SLEEP_B:
    case RFAL_LM_STATE_SLEEP_AF:
      retCode = NFC_InvalidParameter;
      break;

    case RFAL_LM_STATE_IDLE:
      st25r95State = ST25R95_ACSTATE_IDLE;
      break;

    case RFAL_LM_STATE_READY_A:
      st25r95State = ST25R95_ACSTATE_READYA;
      break;

    case RFAL_LM_STATE_ACTIVE_A:
      st25r95State = ST25R95_ACSTATE_ACTIVE;
      break;

    case RFAL_LM_STATE_SLEEP_A:
      st25r95State = ST25R95_ACSTATE_HALT;
      break;

    case RFAL_LM_STATE_READY_Ax:
      st25r95State = ST25R95_ACSTATE_READYAX;
      break;

    case RFAL_LM_STATE_ACTIVE_Ax:
      st25r95State = ST25R95_ACSTATE_ACTIVEX;
      break;

    case RFAL_LM_STATE_CARDEMU_4A:
      st25r95State = ST25R95_GetLmState();
      if ((st25r95State != ST25R95_ACSTATE_ACTIVE) && (st25r95State != ST25R95_ACSTATE_ACTIVEX)) {
        st25r95State = ST25R95_ACSTATE_ACTIVE;
      } else {
        st25r95State = gST25R95ACState;
      }
      gRFAL.cardEmulT4AT = true;
      break;
  }
  if (retCode == NFC_OK) {
    ST25R95_SetACState(st25r95State);
  }
  if (WasInListen) {
    ST25R95_Listen();
  }

  return (retCode);
}

/*******************************************************************************
 *  Wake-Up Mode                                                               *
 *******************************************************************************/

/*******************************************************************************/
NFC_OpResult RFal_WakeUpModeStart(const RFal_WakeUpConfig *config)
{
  /* Check if RFAL is not initialized */
  if (gRFAL.state == RFAL_STATE_IDLE) {
    return NFC_WrongState;
  }

  if (config == NULL) {
    gRFAL.wum.cfg.period           = RFAL_WUM_PERIOD_300MS;
    gRFAL.wum.cfg.irqTout          = false;
    gRFAL.wum.cfg.swTagDetect      = false;

    gRFAL.wum.cfg.indAmp.enabled   = true;
    gRFAL.wum.cfg.indPha.enabled   = false;
    gRFAL.wum.cfg.cap.enabled      = false;
    gRFAL.wum.cfg.indAmp.delta     = 8U;
    gRFAL.wum.cfg.indAmp.reference = RFAL_WUM_REFERENCE_AUTO;
  } else {
    gRFAL.wum.cfg = *config;
  }

  /* Check for valid configuration */
  if (gRFAL.wum.cfg.cap.enabled || gRFAL.wum.cfg.indPha.enabled  || gRFAL.wum.cfg.swTagDetect || !gRFAL.wum.cfg.indAmp.enabled) {
    return NFC_InvalidParameter;
  }

  if (gRFAL.wum.cfg.indAmp.reference == RFAL_WUM_REFERENCE_AUTO) {
    gRFAL.wum.cfg.indAmp.reference = gRFAL.wum.CalTagDet;
  }
  if ((gRFAL.wum.cfg.indAmp.delta > gRFAL.wum.cfg.indAmp.reference) || ((((uint32_t)gRFAL.wum.cfg.indAmp.delta) + ((uint32_t)gRFAL.wum.cfg.indAmp.reference)) > 0xFCUL)) {
    return NFC_InvalidParameter;
  }

  /* Use a fixed period of ~300 ms */
  ST25R95_IO_SPI_Idle(gRFAL.wum.cfg.indAmp.reference - gRFAL.wum.cfg.indAmp.delta, gRFAL.wum.cfg.indAmp.reference + gRFAL.wum.cfg.indAmp.delta, RFAL_ST25R95_IDLE_DEFAULT_WUPERIOD);
  gRFAL.state     = RFAL_STATE_WUM;
  gRFAL.wum.state = RFAL_WUM_STATE_ENABLED;
  return NFC_OK;
}

/*******************************************************************************/
bool RFal_WakeUpModeIsEnabled(void)
{
  return NFC_Unsupport; /* NFC_Unsupport*/
}

/*******************************************************************************/
NFC_OpResult RFal_WakeUpModeGetInfo(bool force, RFal_WakeUpInfo *info)
{
  return NFC_Unsupport; /* NFC_Unsupport*/
}

/*******************************************************************************/
bool RFal_WakeUpModeHasWoke(void)
{
  return (gRFAL.wum.state >= RFAL_WUM_STATE_ENABLED_WOKE);
}

/*******************************************************************************/
NFC_OpResult RFal_WakeUpModeStop(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }


  if (gRFAL.wum.state == RFAL_WUM_STATE_NOT_INIT) {
    return NFC_WrongState;
  }

  gRFAL.wum.state = RFAL_WUM_STATE_NOT_INIT;
  ST25R95_IO_SPI_Kill_Idle();
  ST25R95_IO_SPI_Command_Echo();
  return NFC_OK;
}

/*******************************************************************************/
static NFC_OpResult RFal_RunWakeUpModeWorker(void)
{
  if (gRFAL.state != RFAL_STATE_WUM) {
    return NFC_WrongState;
  }

  switch (gRFAL.wum.state) {
    case RFAL_WUM_STATE_ENABLED:
    case RFAL_WUM_STATE_ENABLED_WOKE:
      if (ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_TIMEOUT) != NFC_SlaveTimeout) {
        ST25R95_IO_SPI_Get_Idle_Response();
        gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
      }

    default:
      return NFC_WrongState;
  }

  return NFC_OK;
}

/*******************************************************************************
 *  RF Chip                                                                    *
 *******************************************************************************/

/*******************************************************************************/
NFC_OpResult RFal_ChipWriteReg(uint16_t reg, const uint8_t *values, uint8_t len)
{
  NFC_OpResult retCode;

  /* Ensure that no previous operation is still ongoing */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }

  if (len != 1) {
    retCode = NFC_InvalidParameter;
  } else {
    retCode = ST25R95_Write_Reg(gRFAL.protocol, reg, values[0]);
  }
  return (retCode);
}

/*******************************************************************************/
NFC_OpResult RFal_ChipReadReg(uint16_t reg, uint8_t *values, uint8_t len)
{
  NFC_OpResult retCode;

  /* Ensure that no previous operation is still ongoing */
  if (RFal_ChipIsBusy()) {
    return NFC_RequestError;
  }


  if (len != 1) {
    retCode = NFC_InvalidParameter;
  } else {
    retCode = ST25R95_Read_Reg(reg, values);
  }
  return (retCode);
}

/*******************************************************************************/
NFC_OpResult RFal_ChipExecCmd(uint16_t cmd)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipWriteTestReg(uint16_t reg, uint8_t value)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipReadTestReg(uint16_t reg, uint8_t *value)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipChangeRegBits(uint16_t reg, uint8_t valueMask, uint8_t value)
{
  NFC_OpResult retCode;
  uint8_t tmp;

  retCode = ST25R95_Read_Reg(reg, &tmp);

  if (retCode == NFC_OK) {
    /* mask out the bits we don't want to change */
    tmp &= (uint8_t)(~((uint32_t)valueMask));
    /* set the new value */
    tmp |= (value & valueMask);
    retCode = ST25R95_Write_Reg(gRFAL.protocol, reg, tmp);
  }

  return retCode;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipChangeTestRegBits(uint16_t reg, uint8_t valueMask, uint8_t value)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipSetRFO(uint8_t rfo)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipGetRFO(uint8_t *result)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasureAmplitude(uint8_t *result)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasurePhase(uint8_t *result)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasureCapacitance(uint8_t *result)
{
  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasurePowerSupply(uint8_t param, uint8_t *result)
{
  return NFC_Unsupport;
}

#endif //CONFIG_NFC_READER_DEVICE_ST25R95