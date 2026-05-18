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

#include "ST25R95_Def.h"

#include "ST25R95_IO.h"

#include "ST25R95.h"

#include "ST25R95_AnalogConfig.h"

#include "ST25R95_RFal.h"

#define ST25R95_RFal_CreateByteFlagsTxRxContext( ctx, tB, tBL, rB, rBL, rdL, fl, t ) \
    (ctx).txBuf     = (uint8_t*)(tB);                                       \
    (ctx).txBufLen  = (uint16_t)rfalConvBytesToBits(tBL);                   \
    (ctx).rxBuf     = (uint8_t*)(rB);                                       \
    (ctx).rxBufLen  = (uint16_t)rfalConvBytesToBits(rBL);                   \
    (ctx).rxRcvdLen = (uint16_t*)(rdL);                                     \
    (ctx).flags     = (uint32_t)(fl);                                       \
    (ctx).fwt       = (uint32_t)(t);

static ST25R95_RFal gRFAL;              /*!< RFAL module instance */

static ST25R95_OpResult ST25R95_RFal_RunListenModeWorker(void);
static ST25R95_OpResult ST25R95_RFal_RunWakeUpModeWorker(void);
static ST25R95_OpResult ST25R95_RFal_RunTransceiveWorker(void);

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_Init(void)
{
  /* Initialize chip */
  if (ST25R95_IO_Init() != ST25R95_OK) {
    return (ST25R95_System);
  }

  /* Check expected chip: ST25R95 */
  if (!ST25R95_CheckChipID()) {
    return ST25R95_Hw_Mismatch;
  }

  /*******************************************************************************/
  /* Debug purposes */
  /*LogSetLevel(LOG_MODULE_DEFAULT, LOG_LEVEL_INFO); !!!!!!!!!!!!!!! */

  /*******************************************************************************/
  gRFAL.state              = ST25R95_State_Init;
  gRFAL.mode               = ST25R95_Mode_None;
  gRFAL.protocol           = ST25R95_Protocol_FieldOff;
  gRFAL.field              = false;

  /* Disable all timings */
  gRFAL.timings.FDTListen  = ST25R95_TIMING_NONE;
  gRFAL.timings.FDTPoll    = ST25R95_TIMING_NONE;
  gRFAL.timings.GT         = ST25R95_TIMING_NONE;

  gRFAL.tmr.GT             = ST25R95_TIMING_NONE;
  gRFAL.tmr.FDTPoll        = ST25R95_TIMING_NONE;

  gRFAL.callbacks.preTxRx  = NULL;
  gRFAL.callbacks.postTxRx = NULL;

  /* Initialize Wake-Up Mode */
  gRFAL.wum.state = ST25R95_WUM_STATE_NOT_INIT;
  gRFAL.wum.CalTagDet = ST25R95_TAGDETECT_DEF_CALIBRATION;

  if (gRFAL.wum.CalTagDet == 0xFFU) {
    return ST25R95_System;
  }
  return ST25R95_OK;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_Calibrate(void)
{
  return (ST25R95_OK);
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_AdjustRegulators(uint16_t *result)
{
  return (ST25R95_OK);
}

/*******************************************************************************/
void ST25R95_RFal_SetUpperLayerCallback(ST25R95_UpperLayerCallback pFunc)
{
  return;
}

/*******************************************************************************/
void ST25R95_RFal_SetPreTxRxCallback(ST25R95_PreTxRxCallback pFunc)
{
  gRFAL.callbacks.preTxRx = pFunc;
}

/*******************************************************************************/
void ST25R95_RFal_SetSyncTxRxCallback(ST25R95_SyncTxRxCallback pFunc)
{
  return;   /* ST25R95_Unsupport */
}

/*******************************************************************************/
void ST25R95_RFal_SetPostTxRxCallback(ST25R95_PostTxRxCallback pFunc)
{
  gRFAL.callbacks.postTxRx = pFunc;
}

/*******************************************************************************/
void ST25R95_RFal_SetLmEonCallback(ST25R95_LmEonCallback pFunc)
{
  return;   /* ST25R95_Unsupport */
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_Deinit(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }

  gRFAL.state = ST25R95_State_Idle;

  /* Deinitialize chip */
  return ST25R95_IO_DeInit();
}

/*******************************************************************************/
void ST25R95_RFal_SetObsvMode(uint32_t txMode, uint32_t rxMode)
{
  return;
}


/*******************************************************************************/
void ST25R95_RFal_GetObsvMode(uint8_t *txMode, uint8_t *rxMode)
{
  return;
}

/*******************************************************************************/
void ST25R95_RFal_DisableObsvMode(void)
{
  return;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_SetMode(ST25R95_Mode mode, ST25R95_BitRate txBR, ST25R95_BitRate rxBR)
{
  /* Check if RFAL is not initialized */
  if (gRFAL.state == ST25R95_State_Idle) {
    return ST25R95_WrongState;
  }

  /* Check allowed bit rate value */
  if ((txBR == ST25R95_BitRate_KEEP) || (rxBR == ST25R95_BitRate_KEEP)) {
    return ST25R95_InvalidParameter;
  }

  /*******************************************************************************/
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }


  switch (mode) {
    /*******************************************************************************/
    case ST25R95_Mode_Poll_NFCA:
    case ST25R95_Mode_Poll_NFCA_T1T:
      gRFAL.protocol = ST25R95_Protocol_ISO14443A;
      gRFAL.nfcaData.NfcaSplitFrame = false;
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCA | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCA | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
    case ST25R95_Mode_Poll_NFCB:
      gRFAL.protocol = ST25R95_Protocol_ISO14443B;
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCB | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCB | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
    case ST25R95_Mode_Poll_NFCF:
      gRFAL.protocol = ST25R95_Protocol_ISO18092;
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCF | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCF | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
    case ST25R95_Mode_Poll_NFCV:
      gRFAL.protocol = ST25R95_Protocol_ISO15693;
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCV | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCV | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
    case ST25R95_Mode_Listen_NFCA:
      gRFAL.protocol = ST25R95_Protocol_CE_ISO14443A;
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_LISTEN | ST25R95_ANALOG_CONFIG_TECH_NFCA | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_AnalogConfig_Set((ST25R95_ANALOG_CONFIG_LISTEN | ST25R95_ANALOG_CONFIG_TECH_NFCA | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
    /*******************************************************************************/
    case ST25R95_Mode_Poll_B_PRIME:
    case ST25R95_Mode_Poll_B_CTS:
    case ST25R95_Mode_Poll_PICOPASS:
    case ST25R95_Mode_Poll_Active_P2P:
    case ST25R95_Mode_Listen_Active_P2P:
    case ST25R95_Mode_Listen_NFCB:
    case ST25R95_Mode_Listen_NFCF:
      return ST25R95_Unsupport;
      /*NOTREACHED*/
      break;

    default:
      return ST25R95_InvalidParameter;
  }

  /* Set state as STATE_MODE_SET only if not initialized yet (PSL) */
  gRFAL.state = ((gRFAL.state < ST25R95_State_Mode_Set) ? ST25R95_State_Mode_Set : gRFAL.state);
  gRFAL.mode  = mode;

  /* Apply the given bit rate and mode */
  return (ST25R95_RFal_SetBitRate(txBR, rxBR));
}

/*******************************************************************************/
ST25R95_Mode ST25R95_RFal_GetMode(void)
{
  return gRFAL.mode;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_SetBitRate(ST25R95_BitRate txBR, ST25R95_BitRate rxBR)
{
  ST25R95_OpResult retCode = ST25R95_OK;

  /* Check if RFAL is not initialized */
  if (gRFAL.state == ST25R95_State_Idle) {
    return ST25R95_WrongState;
  }

  /*******************************************************************************/
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }


  /* Store the new Bit Rates */
  gRFAL.txBR = ((txBR == ST25R95_BitRate_KEEP) ? gRFAL.txBR : txBR);
  gRFAL.rxBR = ((rxBR == ST25R95_BitRate_KEEP) ? gRFAL.rxBR : rxBR);

  retCode = ST25R95_Set_BitRate(gRFAL.protocol, txBR, rxBR);
  if ((retCode == ST25R95_OK) && (gRFAL.protocol != ST25R95_Protocol_FieldOff)) {
    /* If field on, update bitrate value through ProtocolSelect */
    retCode = ST25R95_Protocol_Select(gRFAL.protocol);
  }

  return (retCode);
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_GetBitRate(ST25R95_BitRate *txBR, ST25R95_BitRate *rxBR)
{
  if ((gRFAL.state == ST25R95_State_Idle) || (gRFAL.mode == ST25R95_Mode_None)) {
    return ST25R95_WrongState;
  }

  if (txBR != NULL) {
    *txBR = gRFAL.txBR;
  }

  if (rxBR != NULL) {
    *rxBR = gRFAL.rxBR;
  }

  return ST25R95_OK;
}

/*******************************************************************************/
void ST25R95_RFal_SetErrorHandling(ST25R95_EHandling eHandling)
{
  NO_WARNING(eHandling);
  return;
}

/*******************************************************************************/
ST25R95_EHandling ST25R95_RFal_GetErrorHandling(void)
{
  return ERRORHANDLING_NONE;
}

/*******************************************************************************/
void ST25R95_RFal_SetFDTPoll(uint32_t FDTPoll)
{
  gRFAL.timings.FDTPoll = (FDTPoll > ST25R95_GPT_MAX_1FC) ? ST25R95_GPT_MAX_1FC : FDTPoll;
}

/*******************************************************************************/
uint32_t ST25R95_RFal_GetFDTPoll(void)
{
  return gRFAL.timings.FDTPoll;
}

/*******************************************************************************/
void ST25R95_RFal_SetFDTListen(uint32_t FDTListen)
{
  gRFAL.timings.FDTListen = (FDTListen > ST25R95_MRT_MAX_1FC) ? ST25R95_MRT_MAX_1FC : FDTListen;
}

/*******************************************************************************/
uint32_t ST25R95_RFal_GetFDTListen(void)
{
  return gRFAL.timings.FDTListen;
}

/*******************************************************************************/
void ST25R95_RFal_SetGT(uint32_t GT)
{
  gRFAL.timings.GT = (GT > ST25R95_GT_MAX_1FC) ? ST25R95_GT_MAX_1FC : GT;
}

/*******************************************************************************/
uint32_t ST25R95_RFal_GetGT(void)
{
  return gRFAL.timings.GT;
}

/*******************************************************************************/
bool ST25R95_RFal_IsGTExpired(void)
{
  if (gRFAL.tmr.GT != ST25R95_TIMING_NONE) {
    if (!ST25R95_TimerisExpired(gRFAL.tmr.GT)) {
      return false;
    }
  }
  return true;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_FieldOnAndStartGT(void)
{
  ST25R95_OpResult ret;

  /* Check if RFAL has been initialized  */
  if ((gRFAL.state < ST25R95_State_Init)) {
    return ST25R95_WrongState;
  }

  /*******************************************************************************/
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }


  ret = ST25R95_OK;

  /*******************************************************************************/
  /* Turn field On if not already On */
  if (!gRFAL.field) {
    ret = ST25R95_Field_On(gRFAL.protocol);
    gRFAL.field = true;
  }

  /*******************************************************************************/
  /* Start GT timer in case the GT value is set */
  if ((gRFAL.timings.GT != ST25R95_TIMING_NONE)) {
    /* Ensure that a SW timer doesn't have a lower value then the minimum  */
    ST25R95_TimerStart(gRFAL.tmr.GT, ST25R95_Conv1fcToMs(MAX((gRFAL.timings.GT), ST25R95_GT_MIN_1FC)));
  }

  return ret;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_FieldOff(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }

  ST25R95_RFal_WakeUpModeStop();
  gRFAL.field = false;
  gRFAL.protocol = ST25R95_Protocol_FieldOff;
  return (ST25R95_FieldOff());
}



/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_StartTransceive(const ST25R95_TransceiveContext *ctx)
{
  /* Ensure that RFAL is already Initialized and the mode has been set */
  if ((gRFAL.state >= ST25R95_State_Mode_Set)) {
    /*******************************************************************************/
    /* Ensure that no previous operation is still ongoing */
    if (ST25R95_RFal_ChipIsBusy()) {
      return ST25R95_RequestError;
    }

    gRFAL.TxRx.ctx = *ctx;

    /*******************************************************************************/
    if (ST25R95_RFal_IsModePassiveComm(gRFAL.mode)) { /* Passive Comms */
      if ((gRFAL.TxRx.ctx.fwt != ST25R95_FWT_NONE) && (gRFAL.TxRx.ctx.fwt != 0)) {
        ST25R95_Set_FWT(gRFAL.protocol, gRFAL.TxRx.ctx.fwt);
      } else {
        /* Since ST25R95 does not support, use max FWT available */
        ST25R95_Set_FWT(gRFAL.protocol, ST25R95_FWT_MAX);
      }
    }

    gRFAL.state       = ST25R95_State_TXRX;
    gRFAL.TxRx.state  = ST25R95_Transceive_State_TX_Idle;
    gRFAL.TxRx.status = ST25R95_Busy;
    gRFAL.Lm.dataFlag = false;

    /*******************************************************************************/
    if ((ST25R95_Mode_Poll_NFCV == gRFAL.mode) || (ST25R95_Mode_Poll_PICOPASS == gRFAL.mode)) {
      /* In NFCV a TxRx with a valid txBuf and txBufSize==0 indicates to send an EOF */
      /* Skip logic below that would go directly into receive                        */
      if (gRFAL.TxRx.ctx.txBuf != NULL) {
        return  ST25R95_OK;
      }
    }

    /*******************************************************************************/
    /* Check if the Transceive start performing Tx or goes directly to Rx          */
    if ((gRFAL.TxRx.ctx.txBuf == NULL) || (gRFAL.TxRx.ctx.txBufLen == 0)) {
      return ST25R95_InvalidParameter;
    }

    return ST25R95_OK;
  }

  return ST25R95_WrongState;
}


/*******************************************************************************/
bool ST25R95_RFal_IsTransceiveInTx(void)
{
  return ((gRFAL.TxRx.state >= ST25R95_Transceive_State_TX_Idle) && (gRFAL.TxRx.state < ST25R95_Transceive_State_RX_Idle));
}


/*******************************************************************************/
bool ST25R95_RFal_IsTransceiveInRx(void)
{
  return (gRFAL.TxRx.state >= ST25R95_Transceive_State_RX_Idle);
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_TransceiveBlockingTx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt)
{
  ST25R95_OpResult               ret;
  ST25R95_TransceiveContext    ctx;

  ST25R95_RFal_CreateByteFlagsTxRxContext(ctx, txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt);
  ret = ST25R95_RFal_StartTransceive(&ctx);
  if(ret < ST25R95_OK)
  {
      return ret;
  }

  return ST25R95_RFal_TransceiveRunBlockingTx();
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_TransceiveBlockingRx(void)
{
  ST25R95_OpResult ret;

  do {
    ST25R95_RFal_Worker();
  } while (((ret = ST25R95_RFal_GetTransceiveStatus()) == ST25R95_Busy) && ST25R95_IsTransceiveInRx());

  return ret;
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_TransceiveBlockingTxRx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt)
{
  ST25R95_OpResult ret;

  ret = ST25R95_RFal_TransceiveBlockingTx(txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt);
  if(ret < ST25R95_OK)
  {
      return ret;
  }

  ret = ST25R95_RFal_TransceiveBlockingRx();

  /* Convert received bits to bytes */
  if (actLen != NULL) {
    *actLen = ST25R95_ConvBitsToBytes(*actLen);
  }

  return ret;
}


/*******************************************************************************/
static ST25R95_OpResult ST25R95_RFal_RunTransceiveWorker(void)
{
  if (gRFAL.state == ST25R95_State_TXRX) {
    /* Run Tx or Rx state machines */
    if (ST25R95_IsTransceiveInTx()) {
      ST25R95_TransceiveTx();
      return ST25R95_RFal_GetTransceiveStatus();
    } else if (ST25R95_IsTransceiveInRx()) {
      ST25R95_TransceiveRx();
      return ST25R95_RFal_GetTransceiveStatus();
    }
  }
  return ST25R95_WrongState;
}

/*******************************************************************************/
ST25R95_TransceiveState ST25R95_RFal_GetTransceiveState(void)
{
  return gRFAL.TxRx.state;
}

ST25R95_OpResult ST25R95_RFal_GetTransceiveStatus(void)
{
  return ((gRFAL.TxRx.state == ST25R95_Transceive_State_Idle) ? gRFAL.TxRx.status : ST25R95_Busy);
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_GetTransceiveRSSI(uint16_t *rssi)
{
  NO_WARNING(rssi);

  return ST25R95_Unsupport;
}

/*******************************************************************************/
bool ST25R95_RFal_IsTransceiveSubcDetected(void)
{
  return false;
}


/*******************************************************************************/
void ST25R95_RFal_Worker(void)
{
  switch (gRFAL.state) {
    case ST25R95_State_TXRX:
      ST25R95_RFal_RunTransceiveWorker();
      break;

    case ST25R95_State_LM:
      ST25R95_RFal_RunListenModeWorker();
      break;

    case ST25R95_State_WUM:
      ST25R95_RFal_RunWakeUpModeWorker();
      break;

    /* Nothing to be done */
    default:
      break;
  }
}

/*******************************************************************************/
void ST25R95_RFal_TransceiveTx(void)
{
  uint8_t transmitFlag = 0;

  if (gRFAL.TxRx.state != gRFAL.TxRx.lastState) {
    /* ST25R95_LogD("%s: lastSt: %d curSt: %d \r\n", __FUNCTION__, gRFAL.TxRx.lastState, gRFAL.TxRx.state);*/
    gRFAL.TxRx.lastState = gRFAL.TxRx.state;
  }

  switch (gRFAL.TxRx.state) {
    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Idle:
      /* Nothing to do */
      gRFAL.TxRx.state = ST25R95_Transceive_State_TX_Wait_GT ;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Wait_GT:
      /* Wait for GT and FDT Poll */

      if (!ST25R95_IsGTExpired() || !ST25R95_TimerisExpired(gRFAL.tmr.FDTPoll)) {
        break;
      }
      gRFAL.tmr.GT = ST25R95_TIMING_NONE;
      gRFAL.TxRx.state = ST25R95_Transceive_State_TX_Transmit;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Transmit:
      /*******************************************************************************/
      /* Execute Pre Transceive Callback                                             */
      /*******************************************************************************/
      if (gRFAL.callbacks.preTxRx != NULL) {
        gRFAL.callbacks.preTxRx();
      }
      /*******************************************************************************/
      /* Prepare Rx                                                                  */
      /*******************************************************************************/
      st25r95SPIPrepareRx(
        gRFAL.protocol,
        gRFAL.TxRx.ctx.rxBuf,
        ST25R95_ConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen),
        gRFAL.TxRx.ctx.rxRcvdLen,
        gRFAL.TxRx.ctx.flags,
        gRFAL.RxInformationBytes
      );
      /*******************************************************************************/
      /* Send the data                                                               */
      /*******************************************************************************/
      st25r95SPISendData(gRFAL.TxRx.ctx.txBuf, ST25R95_ConvBitsToBytes(gRFAL.TxRx.ctx.txBufLen), gRFAL.protocol, gRFAL.TxRx.ctx.flags);

      /* Start FDTPoll SW timer */
      ST25R95_TimerStart(gRFAL.tmr.FDTPoll, (ST25R95_SW_TMR_MIN_1MS + ST25R95_Conv1fcToMs(gRFAL.timings.FDTPoll)));

      gRFAL.TxRx.state = ST25R95_Transceive_State_TX_Wait_TXE;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Wait_TXE:
      if (!st25r95SPIIsTransmitCompleted()) {
        break;
      }
      transmitFlag = gRFAL.TxRx.ctx.txBufLen % 8;
      if (transmitFlag == 0) {
        transmitFlag = 0x8;
      }
      if (!(gRFAL.TxRx.ctx.flags & ST25R95_TXRX_FLAGS_CRC_TX_MANUAL)) {
        transmitFlag |= ST25R95_ISO14443A_APPENDCRC;
      }
      if (gRFAL.nfcaData.NfcaSplitFrame) {
        transmitFlag |= ST25R95_ISO14443A_SPLITFRAME;
      }
      if (gRFAL.mode == ST25R95_Mode_Poll_NFCA_T1T) {
        transmitFlag |= ST25R95_ISO14443A_TOPAZFORMAT;
      }
      st25r95SPISendTransmitFlag(gRFAL.protocol, transmitFlag);
      if (gRFAL.protocol == ST25R95_Protocol_CE_ISO14443A) {
        st25r95RearmListen();
      }
      gRFAL.TxRx.state = ST25R95_Transceive_State_TX_Done;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Done:
      /* If no rxBuf is provided do not wait/expect Rx */
      if (gRFAL.TxRx.ctx.rxBuf == NULL) {
        /* Clean up Transceive */
        //ST25R95_CleanupTransceive();
        gRFAL.TxRx.status = ST25R95_OK;
        gRFAL.TxRx.state  = ST25R95_Transceive_State_Idle;
        break;
      }
      /* Goto Rx */
      gRFAL.TxRx.state  =  ST25R95_Transceive_State_RX_Idle;
      break;

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Fail:
      /* Error should be assigned by previous state */
      if (gRFAL.TxRx.status == ST25R95_Busy) {
        gRFAL.TxRx.status = ST25R95_System;
      }
      gRFAL.TxRx.state = ST25R95_Transceive_State_Idle;
      break;

    /*******************************************************************************/
    default:
      gRFAL.TxRx.status = ST25R95_System;
      gRFAL.TxRx.state  = ST25R95_Transceive_State_TX_Fail;
      break;
  }
}

/*******************************************************************************/
void ST25R95_RFal_TransceiveRx(void)
{
  ST25R95_OpResult retCode;

  if (gRFAL.TxRx.state != gRFAL.TxRx.lastState) {
    /*ST25R95_LogD("%s: lastSt: %d curSt: %d \r\n", __FUNCTION__, gRFAL.TxRx.lastState, gRFAL.TxRx.state);*/
    gRFAL.TxRx.lastState = gRFAL.TxRx.state;
  }

  switch (gRFAL.TxRx.state) {
    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Idle:

      /* Clear rx counters */
      if (gRFAL.TxRx.ctx.rxRcvdLen)  {
        *gRFAL.TxRx.ctx.rxRcvdLen = 0;
      }

      gRFAL.TxRx.state = ST25R95_Transceive_State_RX_Wait_RXE;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Wait_RXE:
      if (st25r95SPIPollRead(ST25R95_CONTROL_POLL_NO_TIMEOUT) == ST25R95_SlaveTimeout) {
        break;
      }
      gRFAL.TxRx.state = ST25R95_Transceive_State_RX_Read_Data;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Read_Data:
      retCode = st25r95SPICompleteRx();
      /* Re-Start FDTPoll SW timer */
      ST25R95_TimerStart(gRFAL.tmr.FDTPoll, (ST25R95_SW_TMR_MIN_1MS + ST25R95_Conv1fcToMs(gRFAL.timings.FDTPoll)));

      if (gRFAL.TxRx.ctx.rxRcvdLen != NULL) {
        (*gRFAL.TxRx.ctx.rxRcvdLen) = ST25R95_ConvBytesToBits(*gRFAL.TxRx.ctx.rxRcvdLen);

        /*******************************************************************************/
        /* In case of Incomplete byte append the residual bits                         */
        /*******************************************************************************/
        if ((retCode >= ST25R95_ERR_INCOMPLETE_BYTE_01) && (retCode <= ST25R95_ERR_INCOMPLETE_BYTE_07)) {
          (*gRFAL.TxRx.ctx.rxRcvdLen) += (retCode - ST25R95_ERR_INCOMPLETE_BYTE);

          if ((*gRFAL.TxRx.ctx.rxRcvdLen) > 0) {
            (*gRFAL.TxRx.ctx.rxRcvdLen) -= ST25R95_BITS_IN_BYTE;
          }

          retCode = ST25R95_ImcompleteByte;
        }
      }


      /*******************************************************************************/
      /* Execute Post Transceive Callback                                            */
      /*******************************************************************************/
      if (gRFAL.callbacks.postTxRx != NULL) {
        gRFAL.callbacks.postTxRx();
      }

      if (retCode != ST25R95_OK) {
        gRFAL.TxRx.status = retCode;
        gRFAL.TxRx.state  = ST25R95_Transceive_State_RX_Fail;
        break;
      }
      gRFAL.TxRx.state = ST25R95_Transceive_State_RX_Done;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Done:
      gRFAL.TxRx.status = ST25R95_OK;
      gRFAL.TxRx.state  = ST25R95_Transceive_State_Idle;
      break;


    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Fail:
      gRFAL.TxRx.state = ST25R95_Transceive_State_Idle;
      break;

    /*******************************************************************************/
    default:
      gRFAL.TxRx.status = ST25R95_System;
      gRFAL.TxRx.state  = ST25R95_Transceive_State_RX_Fail;
      break;
  }
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_TransceiveRunBlockingTx(void)
{
  ST25R95_OpResult ret;

  do {
    ST25R95_RFal_Worker();
  } while (((ret = ST25R95_RFal_GetTransceiveStatus()) == ST25R95_Busy) && ST25R95_RFal_IsTransceiveInTx());

  if (ST25R95_RFal_IsTransceiveInRx()) {
    return ST25R95_OK;
  }

  return ret;
}

/*******************************************************************************/
bool ST25R95_RFal_ChipIsBusy(void)
{
  /* ST25R95 cannot be interrupted while an operation is ongoing */

  /* Check whether a Transceive operation is still running */
  if ((gRFAL.state == ST25R95_State_TXRX) && (gRFAL.TxRx.state > ST25R95_Transceive_State_TX_Idle)) {
    return (true);
  }

  return (false);
}

/*****************************************************************************
 *  NFCA Mode                                                              *
 *****************************************************************************/
/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ISO14443ATransceiveShortFrame(ST25R95_14443AShortFrameCmd txCmd, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *rxRcvdLen, uint32_t fwt)
{
  ST25R95_OpResult ret;
  ST25R95_TransceiveContext ctx;
  uint8_t st95hShortFrameBuffer;
  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < ST25R95_State_Mode_Set) || ((gRFAL.mode != ST25R95_Mode_Poll_NFCA) && (gRFAL.mode != ST25R95_Mode_Poll_NFCA_T1T)) || !gRFAL.field) {
    return ST25R95_WrongState;
  }

  /* Check for valid parameters */
  if ((rxBuf == NULL) || (rxRcvdLen == NULL) || (fwt == ST25R95_FWT_NONE)) {
    return ST25R95_InvalidParameter;
  }

  gRFAL.nfcaData.NfcaSplitFrame = false;
  /*******************************************************************************/
  /* Update the short frame buffer with the REQA or WUPA command                 */
  st95hShortFrameBuffer =  txCmd;


  ctx.flags     = (ST25R95_TXRX_FLAGS_CRC_TX_MANUAL | ST25R95_TXRX_FLAGS_CRC_RX_KEEP);
  ctx.txBuf     = &st95hShortFrameBuffer;
  ctx.txBufLen  = 7;
  ctx.rxBuf     = rxBuf;
  ctx.rxBufLen  = rxBufLen;
  ctx.rxRcvdLen = rxRcvdLen;
  ctx.fwt       = fwt;

  ST25R95_RFal_StartTransceive(&ctx);

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = ST25R95_RFal_TransceiveRunBlockingTx();
  if (ret == ST25R95_OK) {
    ret = ST25R95_RFal_TransceiveBlockingRx();
  }

  /* ST25R95 has no means to disable CRC check, discard CRC errors */
  if (ret == ST25R95_CRC_Error) {
    ret = ST25R95_OK;
  }

  return ret;
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ISO14443ATransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt)
{
  ST25R95_OpResult            ret;

  ret = ST25R95_RFal_ISO14443AStartTransceiveAnticollisionFrame(buf, bytesToSend, bitsToSend, rxLength, fwt);
  if(ret < ST25R95_OK)
  {
      return ret;
  }

  ST25R95_RunBlocking(ret, ST25R95_RFal_ISO14443AGetTransceiveAnticollisionFrameStatus());

  return ret;
}



/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ISO14443AStartTransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt)
{
  ST25R95_OpResult            ret;
  ST25R95_TransceiveContext ctx;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < ST25R95_State_Mode_Set) || (gRFAL.mode != ST25R95_Mode_Poll_NFCA)) {
    return ST25R95_WrongState;
  }

  /* Check for valid parameters */
  if ((buf == NULL) || (bytesToSend == NULL) || (bitsToSend == NULL) || (rxLength == NULL)) {
    return ST25R95_InvalidParameter;
  }

  gRFAL.nfcaData.NfcaSplitFrame = true;

  /*******************************************************************************/
  /* Prepare for Transceive                                                      */
  ctx.flags     = (ST25R95_TXRX_FLAGS_CRC_TX_MANUAL | ST25R95_TXRX_FLAGS_CRC_RX_KEEP);
  ctx.txBuf     = buf;
  ctx.txBufLen  = (ST25R95_ConvBytesToBits(*bytesToSend) + *bitsToSend);
  ctx.rxBuf     = (buf + (*bytesToSend));
  ctx.rxBufLen  = ST25R95_ConvBytesToBits(ST25R95_ISO14443A_SDD_RES_LEN);
  ctx.rxRcvdLen = rxLength;
  ctx.fwt       = fwt;

  ret = ST25R95_RFal_StartTransceive(&ctx);
  if(ret < ST25R95_OK)
  {
      return ret;
  }

  /*******************************************************************************/
  gRFAL.nfcaData.collByte = 0;

  /* save the collision byte */
  if ((*bitsToSend) > 0U) {
    buf[(*bytesToSend)] <<= (ST25R95_BITS_IN_BYTE - (*bitsToSend));
    buf[(*bytesToSend)] >>= (ST25R95_BITS_IN_BYTE - (*bitsToSend));
    gRFAL.nfcaData.collByte = buf[(*bytesToSend)];
  }

  gRFAL.nfcaData.buf         = buf;
  gRFAL.nfcaData.bytesToSend = bytesToSend;
  gRFAL.nfcaData.bitsToSend  = bitsToSend;
  gRFAL.nfcaData.rxLength    = rxLength;

  return ST25R95_OK;
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ISO14443AGetTransceiveAnticollisionFrameStatus(void)
{
  ST25R95_OpResult   ret;

  /*******************************************************************************/
  /* Run Transceive blocking Tx*/
  ret = ST25R95_RFal_TransceiveRunBlockingTx();
  if (ret == ST25R95_OK) {
    ret = ST25R95_RFal_TransceiveBlockingRx();
    /* ignore CRC error */
    if (ret == ST25R95_CRC_Error) {
      ret = ST25R95_OK;
    }

    /*******************************************************************************/
    if ((*gRFAL.nfcaData.bitsToSend) > 0U) {
      gRFAL.nfcaData.buf[(*gRFAL.nfcaData.bytesToSend)] >>= (*gRFAL.nfcaData.bitsToSend);
      gRFAL.nfcaData.buf[(*gRFAL.nfcaData.bytesToSend)] <<= (*gRFAL.nfcaData.bitsToSend);
      gRFAL.nfcaData.buf[(*gRFAL.nfcaData.bytesToSend)] |= gRFAL.nfcaData.collByte;
    }

    if ((ret == ST25R95_RF_Collision)) {
      (*gRFAL.nfcaData.rxLength) = ST25R95_ConvBytesToBits(gRFAL.RxInformationBytes[1]);
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
ST25R95_OpResult ST25R95_RFal_ISO15693TransceiveAnticollisionFrame(uint8_t *txBuf, uint8_t txBufLen, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
  ST25R95_OpResult            ret;
  ST25R95_TransceiveContext ctx;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < ST25R95_State_Mode_Set) || (gRFAL.mode != ST25R95_Mode_Poll_NFCV)) {
    return ST25R95_WrongState;
  }

  /*******************************************************************************/
  /* Prepare for Transceive  */
  ctx.flags     = (((txBufLen == 0) ? ST25R95_TXRX_FLAGS_CRC_TX_MANUAL : ST25R95_TXRX_FLAGS_CRC_TX_AUTO) | ST25R95_TXRX_FLAGS_CRC_RX_KEEP | ST25R95_TXRX_FLAGS_AGC_OFF | ((txBufLen == 0) ? ST25R95_TXRX_FLAGS_NFCV_FLAG_MANUAL : ST25R95_TXRX_FLAGS_NFCV_FLAG_AUTO)); /* Disable Automatic Gain Control (AGC) for better detection of collision */
  ctx.txBuf     = txBuf;
  ctx.txBufLen  = ST25R95_ConvBytesToBits(txBufLen);
  ctx.rxBuf     = rxBuf;
  ctx.rxBufLen  = ST25R95_ConvBytesToBits(rxBufLen);
  ctx.rxRcvdLen = actLen;
  ctx.fwt       = ST25R95_FWT_NONE;

  ret = ST25R95_RFal_StartTransceive(&ctx);
  if(ret < ST25R95_OK)
  {
      return ret;
  }

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = ST25R95_RFal_TransceiveRunBlockingTx();
  if (ret == ST25R95_OK) {
    ret = ST25R95_RFal_TransceiveBlockingRx();
  }

  return ret;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ISO15693TransceiveEOFAnticollision(uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
  uint8_t dummy;

  return ST25R95_RFal_ISO15693TransceiveAnticollisionFrame(&dummy, 0, rxBuf, rxBufLen, actLen);
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ISO15693TransceiveEOF(uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen)
{
  ST25R95_OpResult ret;
  uint8_t    dummy;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < ST25R95_State_Mode_Set) || (gRFAL.mode != ST25R95_Mode_Poll_NFCV)) {
    return ST25R95_WrongState;
  }

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = ST25R95_RFal_TransceiveBlockingTxRx(&dummy,
                                            0,
                                            rxBuf,
                                            rxBufLen,
                                            actLen,
                                            ((uint32_t)ST25R95_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)ST25R95_TXRX_FLAGS_CRC_RX_KEEP | (uint32_t)ST25R95_TXRX_FLAGS_AGC_ON),
                                            ST25R95_Conv64fcTo1fc(ST25R95_FWT_NONE));
  return ret;
}

/*****************************************************************************
 *  NFCF Mode                                                              *
 *****************************************************************************/
/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_FeliCaPoll(ST25R95_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, ST25R95_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected)
{
  ST25R95_OpResult ret;

  ret = ST25R95_RFal_StartFeliCaPoll(slots, sysCode, reqCode, pollResList, pollResListSize, devicesDetected, collisionsDetected);
  if(ret < ST25R95_OK)
  {
      return ret;
  }
  
  ST25R95_RunBlocking(ret, ST25R95_RFal_GetFeliCaPollStatus());

  return ret;
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_StartFeliCaPoll(ST25R95_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, ST25R95_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected)
{
  ST25R95_OpResult        ret;
  uint8_t           frame[ST25R95_FELICA_POLL_REQ_LEN - ST25R95_FELICA_LEN_LEN];  // LEN is added by ST25R95 automatically
  uint8_t           frameIdx;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < ST25R95_State_Mode_Set) || (gRFAL.mode != ST25R95_Mode_Poll_NFCF)) {
    return ST25R95_WrongState;
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
  ret = ST25R95_RFal_TransceiveBlockingTx(frame,
                                          (uint16_t)frameIdx,
                                          (uint8_t *)gRFAL.nfcfData.pollResponses,
                                          ST25R95_FELICA_POLL_RES_LEN,
                                          &gRFAL.nfcfData.actLen,
                                          ST25R95_TXRX_FLAGS_CRC_RX_REMV,
                                          ST25R95_Conv64fcTo1fc(ST25R95_FELICA_POLL_DELAY_TIME + (ST25R95_FELICA_POLL_SLOT_TIME * ((uint32_t)slots + 1U))));
  if(ret < ST25R95_OK)
  {
      return ret;
  }

  /* Store context */
  gRFAL.nfcfData.pollResList        = pollResList;
  gRFAL.nfcfData.pollResListSize    = pollResListSize;
  gRFAL.nfcfData.devicesDetected    = devicesDetected;
  gRFAL.nfcfData.collisionsDetected = collisionsDetected;
  gRFAL.nfcfData.slots              = slots;

  return ST25R95_OK;
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_GetFeliCaPollStatus(void)
{
  ST25R95_OpResult ret;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state != ST25R95_State_TXRX) || (gRFAL.mode != ST25R95_Mode_Poll_NFCF)) {
    return ST25R95_WrongState;
  }

  /* Wait until transceive has terminated */
  ret = ST25R95_RFal_GetTransceiveStatus();
  if(ret == ST25R95_Busy)
  {
      return ST25R95_Busy;
  }

  /*******************************************************************************/
  /* If Tx OK, Wait for first response                                           */
  if (ret == ST25R95_OK) {
    ret = ST25R95_RFal_TransceiveBlockingRx();
    if (ret != ST25R95_SlaveTimeout) {
      /* If the reception was OK, new device found */
      if (ret == ST25R95_OK) {
        gRFAL.nfcfData.devDetected++;
      }
      /* If the reception was not OK, mark as collision */
      else {
        gRFAL.nfcfData.colDetected++;
      }
    }
  }
  ST25R95_Set_SlotCounter((uint8_t)ST25R95_FeliCa_1_Slot);
  /*******************************************************************************/
  /* Restore NRT to normal mode - back to previous error handling */
  gRFAL.conf.eHandling = gRFAL.nfcfData.curHandling;

  /*******************************************************************************/
  /* Assign output parameters if requested                                       */

  if ((gRFAL.nfcfData.pollResList != NULL) && (gRFAL.nfcfData.pollResListSize > 0) && (gRFAL.nfcfData.devDetected > 0)) {
    memcpy(gRFAL.nfcfData.pollResList, gRFAL.nfcfData.pollResponses, (ST25R95_FELICA_POLL_RES_LEN * (uint32_t)MIN(gRFAL.nfcfData.pollResListSize, gRFAL.nfcfData.devDetected)));
  }

  if (gRFAL.nfcfData.devicesDetected != NULL) {
    *gRFAL.nfcfData.devicesDetected = gRFAL.nfcfData.devDetected;
  }

  if (gRFAL.nfcfData.collisionsDetected != NULL) {
    *gRFAL.nfcfData.collisionsDetected = gRFAL.nfcfData.colDetected;
  }

  return ((gRFAL.nfcfData.colDetected || gRFAL.nfcfData.devDetected) ? ST25R95_OK : ret);
}
/*****************************************************************************
 *  Listen Mode                                                              *
 *****************************************************************************/

/*******************************************************************************/
bool ST25R95_RFal_IsExtFieldOn(void)
{
  return (false);
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ListenStart(uint32_t lmMask, const ST25R95_LmConfPA *confA, const ST25R95_LmConfPB *confB, const ST25R95_LmConfPF *confF, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen)
{
  /* Check if RFAL is not initialized */
  if (gRFAL.state == ST25R95_State_Idle) {
    return ST25R95_WrongState;
  }

  gRFAL.cardEmulT4AT = false;

  /*******************************************************************************/
  /* Check whether a Transceive operation is still ongoing                       *
   * ST25R95 cannot be interrupted while a Transceive is ongoing, reject         */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }

  /*******************************************************************************/
  if ((lmMask & ST25R95_LM_MASK_ACTIVE_P2P) || (lmMask & ST25R95_LM_MASK_NFCB) || (lmMask & ST25R95_LM_MASK_NFCF)) {
    return ST25R95_Unsupport;
  }

  if ((lmMask & ST25R95_LM_MASK_NFCA)) {
    if (confA == NULL) {
      return (ST25R95_InvalidParameter);
    }

    ST25R95_RFal_SetMode(ST25R95_Mode_Listen_NFCA, ST25R95_BitRate_106, ST25R95_BitRate_106);

    if (st25r95SetACFilter(confA) != ST25R95_OK) {
      return (ST25R95_InvalidParameter);
    }

    gRFAL.Lm.rxBuf    = rxBuf;
    gRFAL.Lm.rxBufLen = ST25R95_ConvBytesToBits(rxBufLen);
    gRFAL.Lm.rxLen    = rxLen;
    *gRFAL.Lm.rxLen   = 0;
    gRFAL.Lm.dataFlag = false;
    gRFAL.state       = ST25R95_State_LM;

    return ST25R95_OK;
  }

  return ST25R95_Unsupport;
}

/*******************************************************************************/
static ST25R95_OpResult ST25R95_RFal_RunListenModeWorker(void)
{
  ST25R95_OpResult retCode = ST25R95_OK;

  if (!st25r95IsInListen()) {
    retCode = st25r95Listen();
  }

  if (retCode != ST25R95_OK) {
    return (retCode);
  }

  if (ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_NO_TIMEOUT) == ST25R95_SlaveTimeout) {
    return (ST25R95_OK);
  }

  retCode = ST25R95_IO_SPI_Complete_Rx(
    gRFAL.protocol,
    gRFAL.Lm.rxBuf,
    ST25R95_ConvBitsToBytes(gRFAL.Lm.rxBufLen),
    gRFAL.Lm.rxLen,
    (gRFAL.TxRx.ctx.flags & ST25R95_TXRX_FLAGS_CRC_RX_KEEP) != ST25R95_TXRX_FLAGS_CRC_RX_KEEP,
    gRFAL.RxInformationBytes
  );
  if (!((retCode == ST25R95_LinkLoss) || ((retCode == ST25R95_OK) && (gRFAL.Lm.rxLen == 0)))) {
    *gRFAL.Lm.rxLen   = ST25R95_ConvBytesToBits(*gRFAL.Lm.rxLen);
    gRFAL.Lm.dataFlag = true;
    gRFAL.state       = ST25R95_State_Mode_Set;
  }

  return (retCode);
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ListenStop(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }

  ST25R95_IO_SPI_Command_Echo(); /* kill listen command */
  ST25R95_FieldOff();
  gRFAL.state              = ST25R95_State_Init;
  gRFAL.mode               = ST25R95_Mode_None;
  gRFAL.protocol           = ST25R95_Protocol_FieldOff;
  gRFAL.field              = false;
  gRFAL.Lm.dataFlag        = false;

  return ST25R95_OK;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ListenSleepStart(ST25R95_LmState sleepSt, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen)
{
  ST25R95_OpResult retCode = ST25R95_InvalidParameter;

  if (sleepSt == ST25R95_LM_State_Sleep_A) {
    gRFAL.state       = ST25R95_State_LM;
    gRFAL.Lm.rxBuf    = rxBuf;
    gRFAL.Lm.rxBufLen = ST25R95_ConvBytesToBits(rxBufLen);
    gRFAL.Lm.rxLen    = rxLen;
    *gRFAL.Lm.rxLen   = 0;
    gRFAL.Lm.dataFlag = false;
    ST25R95_RFal_ListenSetState(sleepSt);
    retCode = ST25R95_OK;
  }

  return retCode;
}


/*******************************************************************************/
ST25R95_LmState ST25R95_RFal_ListenGetState(bool *dataFlag, ST25R95_BitRate *lastBR)
{
  ST25R95_LmState state;

  /* Allow state retrieval even if gRFAL.state != ST25R95_State_LM so  *
   * that this Lm state can be used by caller after activation      */

  if (lastBR != NULL) {
    *lastBR = gRFAL.txBR;
  }

  if (dataFlag != NULL) {
    *dataFlag = gRFAL.Lm.dataFlag;
  }
  state = st25r95GetLmState();

  if (((state == ST25R95_LM_State_Active_A) || (state == ST25R95_LM_State_Active_Ax)) && gRFAL.cardEmulT4AT) {
    state = ST25R95_LM_State_CardEmu_4A;
  }

  return (state);
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ListenSetState(ST25R95_LmState newSt)
{
  ST25R95_OpResult retCode = ST25R95_OK;
  uint8_t st25r95State;
  bool WasInListen;

  WasInListen = st25r95IsInListen();
  ST25R95_IO_SPI_Command_Echo(); /* kill listen command */
  gRFAL.cardEmulT4AT = false;

  switch (newSt) {
    default:
    case ST25R95_LM_State_Not_Init:
    case ST25R95_LM_State_Power_Off:
    case ST25R95_LM_State_Ready_B:
    case ST25R95_LM_State_Ready_F:
    case ST25R95_LM_State_CardEmu_4B:
    case ST25R95_LM_State_CardEmu_3:
    case ST25R95_LM_State_Target_A:
    case ST25R95_LM_State_Target_F:
    case ST25R95_LM_State_Sleep_B:
    case ST25R95_LM_State_Sleep_AF:
      retCode = ST25R95_InvalidParameter;
      break;

    case ST25R95_LM_State_Idle:
      st25r95State = ST25R95_ACSTATE_IDLE;
      break;

    case ST25R95_LM_State_Ready_A:
      st25r95State = ST25R95_ACSTATE_READYA;
      break;

    case ST25R95_LM_State_Active_A:
      st25r95State = ST25R95_ACSTATE_ACTIVE;
      break;

    case ST25R95_LM_State_Sleep_A:
      st25r95State = ST25R95_ACSTATE_HALT;
      break;

    case ST25R95_LM_State_Ready_Ax:
      st25r95State = ST25R95_ACSTATE_READYAX;
      break;

    case ST25R95_LM_State_Active_Ax:
      st25r95State = ST25R95_ACSTATE_ACTIVEX;
      break;

    case ST25R95_LM_State_CardEmu_4A:
      st25r95State = st25r95GetLmState();
      if ((st25r95State != ST25R95_ACSTATE_ACTIVE) || (st25r95State != ST25R95_ACSTATE_ACTIVEX)) {
        st25r95State = ST25R95_ACSTATE_ACTIVE;
      }
      gRFAL.cardEmulT4AT = true;
      break;
  }
  if (retCode == ST25R95_OK) {
    st25r95SetACState(st25r95State);
  }
  if (WasInListen) {
    st25r95Listen();
  }

  return (retCode);
}

/*******************************************************************************
 *  Wake-Up Mode                                                               *
 *******************************************************************************/

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_WakeUpModeStart(const ST25R95_WakeUpConfig *config)
{
  /* Check if RFAL is not initialized */
  if (gRFAL.state == ST25R95_State_Idle) {
    return ST25R95_WrongState;
  }

  if (config == NULL) {
    gRFAL.wum.cfg.period           = ST25R95_WUM_PERIOD_300MS;
    gRFAL.wum.cfg.irqTout          = false;
    gRFAL.wum.cfg.swTagDetect      = false;

    gRFAL.wum.cfg.indAmp.enabled   = true;
    gRFAL.wum.cfg.indPha.enabled   = false;
    gRFAL.wum.cfg.cap.enabled      = false;
    gRFAL.wum.cfg.indAmp.delta     = 8U;
    gRFAL.wum.cfg.indAmp.reference = ST25R95_WUM_REFERENCE_AUTO;
  } else {
    gRFAL.wum.cfg = *config;
  }

  /* Check for valid configuration */
  if (gRFAL.wum.cfg.cap.enabled || gRFAL.wum.cfg.indPha.enabled  || gRFAL.wum.cfg.swTagDetect || !gRFAL.wum.cfg.indAmp.enabled) {
    return ST25R95_InvalidParameter;
  }

  if (gRFAL.wum.cfg.indAmp.reference == ST25R95_WUM_REFERENCE_AUTO) {
    gRFAL.wum.cfg.indAmp.reference = gRFAL.wum.CalTagDet;
  }
  if ((gRFAL.wum.cfg.indAmp.delta > gRFAL.wum.cfg.indAmp.reference) || ((((uint32_t)gRFAL.wum.cfg.indAmp.delta) + ((uint32_t)gRFAL.wum.cfg.indAmp.reference)) > 0xFCUL)) {
    return ST25R95_InvalidParameter;
  }

  /* Use a fixed period of ~300 ms */
  ST25R95_IO_SPI_Idle(gRFAL.wum.cfg.indAmp.reference - gRFAL.wum.cfg.indAmp.delta, gRFAL.wum.cfg.indAmp.reference + gRFAL.wum.cfg.indAmp.delta, ST25R95_IDLE_DEFAULT_WUPERIOD);
  gRFAL.state     = ST25R95_State_WUM;
  gRFAL.wum.state = ST25R95_WUM_STATE_ENABLED;
  return ST25R95_OK;
}


/*******************************************************************************/
bool ST25R95_RFal_WakeUpModeIsEnabled(void)
{
  return ST25R95_Unsupport; /* ST25R95_Unsupport*/
}


/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_WakeUpModeGetInfo(bool force, ST25R95_WakeUpInfo *info)
{
  return ST25R95_Unsupport; /* ST25R95_Unsupport*/
}


/*******************************************************************************/
bool ST25R95_RFal_WakeUpModeHasWoke(void)
{
  return (gRFAL.wum.state >= ST25R95_WUM_STATE_ENABLED_WOKE);
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_WakeUpModeStop(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }


  if (gRFAL.wum.state == ST25R95_WUM_STATE_NOT_INIT) {
    return ST25R95_WrongState;
  }

  gRFAL.wum.state = ST25R95_WUM_STATE_NOT_INIT;
  ST25R95_IO_SPI_Kill_Idle();
  ST25R95_IO_SPI_Command_Echo();
  return ST25R95_OK;
}


/*******************************************************************************/
static ST25R95_OpResult ST25R95_RFal_RunWakeUpModeWorker(void)
{
  if (gRFAL.state != ST25R95_State_WUM) {
    return ST25R95_WrongState;
  }

  switch (gRFAL.wum.state) {
    case ST25R95_WUM_STATE_ENABLED:
    case ST25R95_WUM_STATE_ENABLED_WOKE:
      if (ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_NO_TIMEOUT) != ST25R95_SlaveTimeout) {
        ST25R95_IO_SPI_Get_Idle_Response();
        gRFAL.wum.state = ST25R95_WUM_STATE_ENABLED_WOKE;
      }

    default:
      return ST25R95_WrongState;
  }

  return ST25R95_OK;
}

/*******************************************************************************
 *  RF Chip                                                                    *
 *******************************************************************************/

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipWriteReg(uint16_t reg, const uint8_t *values, uint8_t len)
{
  ST25R95_OpResult retCode;

  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }


  if (len != 1) {
    retCode = ST25R95_InvalidParameter;
  } else {
    retCode = ST25R95_Write_Reg(gRFAL.protocol, reg, values[0]);
  }
  return (retCode);
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipReadReg(uint16_t reg, uint8_t *values, uint8_t len)
{
  ST25R95_OpResult retCode;

  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_RFal_ChipIsBusy()) {
    return ST25R95_RequestError;
  }


  if (len != 1) {
    retCode = ST25R95_InvalidParameter;
  } else {
    retCode = ST25R95_Read_Reg(reg, values);
  }
  return (retCode);
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipExecCmd(uint16_t cmd)
{
  return ST25R95_Unsupport;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipWriteTestReg(uint16_t reg, uint8_t value)
{
  return ST25R95_Unsupport;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipReadTestReg(uint16_t reg, uint8_t *value)
{
  return ST25R95_Unsupport;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipChangeRegBits(uint16_t reg, uint8_t valueMask, uint8_t value)
{
  ST25R95_OpResult retCode;
  uint8_t tmp;

  retCode = ST25R95_Read_Reg(reg, &tmp);

  if (retCode == ST25R95_OK) {
    /* mask out the bits we don't want to change */
    tmp &= (uint8_t)(~((uint32_t)valueMask));
    /* set the new value */
    tmp |= (value & valueMask);
    retCode = ST25R95_Write_Reg(gRFAL.protocol, reg, tmp);
  }

  return retCode;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipChangeTestRegBits(uint16_t reg, uint8_t valueMask, uint8_t value)
{
  return ST25R95_Unsupport;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipSetRFO(uint8_t rfo)
{
  return ST25R95_Unsupport;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipGetRFO(uint8_t *result)
{
  return ST25R95_Unsupport;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipMeasureAmplitude(uint8_t *result)
{
  return ST25R95_Unsupport;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipMeasurePhase(uint8_t *result)
{
  return ST25R95_Unsupport;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipMeasureCapacitance(uint8_t *result)
{
  return ST25R95_Unsupport;
}

/*******************************************************************************/
ST25R95_OpResult ST25R95_RFal_ChipMeasurePowerSupply(uint8_t param, uint8_t *result)
{
  return ST25R95_Unsupport;
}
