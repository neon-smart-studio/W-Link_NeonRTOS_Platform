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

#include "ST25R95_def.h"

#include "ST25R95_IO.h"

#include "ST25R95_AnalogConfig.h"

#include "ST25R95.h"

/*! Struct that holds all context for the Listen Mode                                             */
typedef struct {
  uint8_t                *rxBuf;       /*!< Location to store incoming data in Listen Mode      */
  uint16_t                rxBufLen;    /*!< Length of rxBuf                                     */
  uint16_t               *rxLen;       /*!< Pointer to write the data length placed into rxBuf  */
  bool                    dataFlag;    /*!< Listen Mode current Data Flag                       */
} ST25R95_Lm;

/*! Struct that holds all context for the Wake-Up Mode                                             */
typedef struct {
  ST25R95_WumState            state;       /*!< Current Wake-Up Mode state                           */
  ST25R95_WakeUpConfig        cfg;         /*!< Current Wake-Up Mode context                         */
  uint8_t                 CalTagDet;   /*!< Tag Detection calibration value                      */
} ST25R95_Wum;

typedef struct {
  uint32_t                GT;          /*!< GT in 1/fc                  */
  uint32_t                FDTListen;   /*!< FDTListen in 1/fc           */
  uint32_t                FDTPoll;     /*!< FDTPoll in 1/fc             */
} ST25R95_Timings;

/*! Struct that holds the software timers                                 */
typedef struct {
  uint32_t                GT;          /*!< ST25R95's GT timer             */
  uint32_t                FDTPoll;     /*!< ST25R95's FST Poll timer       */
} ST25R95_Timers;

/*! Struct that holds ST25R95's configuration settings                                                      */
typedef struct {
  uint8_t                 obsvModeTx;  /*!< ST25R95's config of the ST25R3911's observation mode while Tx */
  uint8_t                 obsvModeRx;  /*!< ST25R95's config of the ST25R3911's observation mode while Rx */
  ST25R95_EHandling           eHandling;   /*!< ST25R95's error handling config/mode                          */
} ST25R95_Configs;


/*! Struct that holds NFC-A data - Used only inside ST25R95_ISO14443ATransceiveAnticollisionFrame()          */
typedef struct {
  uint8_t                 collByte;    /*!< NFC-A Anticollision collision byte                         */
  uint8_t                 *buf;        /*!< NFC-A Anticollision frame buffer                           */
  uint8_t                 *bytesToSend;/*!< NFC-A Anticollision NFCID|UID byte context                 */
  uint8_t                 *bitsToSend; /*!< NFC-A Anticollision NFCID|UID bit context                  */
  uint16_t                *rxLength;   /*!< NFC-A Anticollision received length                        */
  bool                    NfcaSplitFrame;
} ST25R95_NfcaWorkingData;

/*! Struct that holds NFC-F data - Used only inside ST25R95_FelicaPoll() (static to avoid adding it into stack) */
typedef struct {
  uint16_t           actLen;                                      /* Received length                         */
  ST25R95_FeliCaPollRes *pollResList;                                 /* Location of NFC-F device list           */
  uint8_t            pollResListSize;                             /* Size of NFC-F device list               */
  uint8_t            devDetected;                                 /* Number of devices detected              */
  uint8_t            colDetected;                                 /* Number of collisions detected           */
  uint8_t            *devicesDetected;                            /* Location to place number of devices     */
  uint8_t            *collisionsDetected;                         /* Location to place number of collisions  */
  ST25R95_EHandling      curHandling;                                 /* ST25R95's error handling                   */
  ST25R95_FeliCaPollRes pollResponses[ST25R95_FELICA_POLL_MAX_SLOTS];   /* FeliCa Poll response container for 16 slots */
  ST25R95_FeliCaPollSlots slots;
} ST25R95_NfcfWorkingData;


ST25R95_State state = ST25R95_State_Idle;          /*!< Current transceive state                            */
ST25R95_Mode currentMode = ST25R95_Mode_None;      /*!< ST25R95's current mode                             */
ST25R95_BitRate currenTxBitRate = ST25R95_BitRate_KEEP;       /*!< ST25R95's current Tx Bit Rate                      */
ST25R95_BitRate currenRxBitRate = ST25R95_BitRate_KEEP;       /*!< ST25R95's current Rx Bit Rate                      */
bool field = false;                                /*!< Current field state (On / Off)                  */

ST25R95_Configs conf;      /*!< ST25R95's configuration settings                 */
ST25R95_Timings           timings;   /*!< ST25R95's timing setting                           */

/*!< ST25R95's transceive management                    */
ST25R95_TransceiveState TxRx_state     = ST25R95_Transceive_State_Idle;    /*!< Last transceive state (debug purposes) */  
ST25R95_TransceiveState TxRx_lastState = ST25R95_Transceive_State_Idle;    /*!< Last transceive state (debug purposes) */ 

ST25R95_Lm                Lm;        /*!< ST25R95's listen mode management                   */
ST25R95_Wum               wum;       /*!< ST25R95's Wake-Up mode management                  */

ST25R95_Timers            tmr;       /*!< ST25R95's Software timers                          */

uint8_t               protocol;  /*!< ProtocolSelect protocol                         */
uint8_t               RxInformationBytes[3]; /*!< ST25R95 additional information bytes*/

bool                  cardEmulT4AT;
ST25R95_NfcaWorkingData     nfcaData;  /*!< ST25R95's working data when supporting NFC-A     */
ST25R95_NfcfWorkingData   nfcfData; /*!< ST25R95's working data when supporting NFC-F        */

/*******************************************************************************/
ReturnCode ST25R95_SetMode(ST25R95_Mode mode, ST25R95_BitRate txBR, ST25R95_BitRate rxBR)
{
  /* Check if ST25R95 is not initialized */
  if (state == ST25R95_State_Idle) {
    return ERR_WRONG_STATE;
  }

  /* Check allowed bit rate value */
  if ((txBR == ST25R95_BitRate_KEEP) || (rxBR == ST25R95_BitRate_KEEP)) {
    return ERR_PARAM;
  }

  /*******************************************************************************/
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_ChipIsBusy()) {
    return ERR_REQUEST;
  }


  switch (mode) {
    /*******************************************************************************/
    case ST25R95_Mode_Poll_NFCA:
    case ST25R95_Mode_Poll_NFCA_T1T:
      protocol = ST25R95_Protocol_ISO14443A;
      nfcaData.NfcaSplitFrame = false;
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCA | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCA | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
    case ST25R95_Mode_Poll_NFCB:
      protocol = ST25R95_Protocol_ISO14443B;
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCB | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCB | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
    case ST25R95_Mode_Poll_NFCF:
      protocol = ST25R95_Protocol_ISO18092;
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCF | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCF | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
    case ST25R95_Mode_Poll_NFCV:
      protocol = ST25R95_Protocol_ISO15693;
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCV | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_POLL | ST25R95_ANALOG_CONFIG_TECH_NFCV | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
    case ST25R95_Mode_Listen_NFCA:
#if ST25R95_FEATURE_LISTEN_MODE
      protocol = ST25R95_Protocol_CE_ISO14443A;
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_LISTEN | ST25R95_ANALOG_CONFIG_TECH_NFCA | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_TX));
      ST25R95_Set_AnalogConfig((ST25R95_ANALOG_CONFIG_LISTEN | ST25R95_ANALOG_CONFIG_TECH_NFCA | ST25R95_ANALOG_CONFIG_BITRATE_COMMON | ST25R95_ANALOG_CONFIG_RX));
      break;
#else
      return ERR_NOTSUPP;
      /*NOTREACHED*/
      break;
#endif /* ST25R95_FEATURE_LISTEN_MODE */
    /*******************************************************************************/
    case ST25R95_Mode_Poll_B_PRIME:
    case ST25R95_Mode_Poll_B_CTS:
    case ST25R95_Mode_Poll_PICOPASS:
    case ST25R95_Mode_Poll_Active_P2P:
    case ST25R95_Mode_Listen_Active_P2P:
    case ST25R95_Mode_Listen_NFCB:
    case ST25R95_Mode_Listen_NFCF:
      return ERR_NOTSUPP;
      /*NOTREACHED*/
      break;

    /*******************************************************************************/
    default:
      return ERR_NOT_IMPLEMENTED;
  }

  /* Set state as STATE_MODE_SET only if not initialized yet (PSL) */
  state = ((state < ST25R95_State_Mode_Set) ? ST25R95_State_Mode_Set : state);
  currentMode  = mode;

  /* Apply the given bit rate and mode */
  return (ST25R95_SetBitRate(txBR, rxBR));
}

/*******************************************************************************/
ST25R95_Mode ST25R95_GetMode(void)
{
  return currentMode;
}

/*******************************************************************************/
ReturnCode ST25R95_SetBitRate(ST25R95_BitRate txBR, ST25R95_BitRate rxBR)
{
  ReturnCode retCode = ERR_NONE;

  /* Check if ST25R95 is not initialized */
  if (state == ST25R95_State_Idle) {
    return ERR_WRONG_STATE;
  }

  /*******************************************************************************/
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_ChipIsBusy()) {
    return ERR_REQUEST;
  }


  /* Store the new Bit Rates */
  currenTxBitRate = ((txBR == ST25R95_BitRate_KEEP) ? txBR : txBR);
  currenRxBitRate = ((rxBR == ST25R95_BitRate_KEEP) ? rxBR : rxBR);

  retCode = ST25R95_IO_SetBitRate(protocol, txBR, rxBR);
  if ((retCode == ERR_NONE) && (protocol != ST25R95_Protocol_FieldOff)) {
    /* If field on, update bitrate value through ProtocolSelect */
    retCode = ST25R95_IO_ProtocolSelect(protocol);
  }

  return (retCode);
}

/*******************************************************************************/
ReturnCode ST25R95_GetBitRate(ST25R95_BitRate *txBR, ST25R95_BitRate *rxBR)
{
  if ((state == ST25R95_State_Idle) || (currentMode == ST25R95_Mode_None)) {
    return ERR_WRONG_STATE;
  }

  if (txBR != NULL) {
    *txBR = currenTxBitRate;
  }

  if (rxBR != NULL) {
    *rxBR = currenRxBitRate;
  }

  return ERR_NONE;
}

/*******************************************************************************/
void ST25R95_SetFDTPoll(uint32_t FDTPoll)
{
  timings.FDTPoll = MIN(FDTPoll, ST25R95_GPT_MAX_1FC);
}

/*******************************************************************************/
uint32_t ST25R95_GetFDTPoll(void)
{
  return timings.FDTPoll;
}

/*******************************************************************************/
void ST25R95_SetFDTListen(uint32_t FDTListen)
{
  timings.FDTListen = MIN(FDTListen, ST25R95_MRT_MAX_1FC);
}

/*******************************************************************************/
uint32_t ST25R95_GetFDTListen(void)
{
  return timings.FDTListen;
}

/*******************************************************************************/
void ST25R95_SetGT(uint32_t GT)
{
  timings.GT = MIN(GT, ST25R95_GT_MAX_1FC);
}

/*******************************************************************************/
uint32_t ST25R95_GetGT(void)
{
  return timings.GT;
}

/*******************************************************************************/
bool ST25R95_IsGTExpired(void)
{
  if (tmr.GT != ST25R95_TIMING_NONE) {
    if (!ST25R95_TimerisExpired(tmr.GT)) {
      return false;
    }
  }
  return true;
}

/*******************************************************************************/
ReturnCode ST25R95_FieldOnAndStartGT(void)
{
  ReturnCode ret;

  /* Check if ST25R95 has been initialized  */
  if ((state < ST25R95_State_Init)) {
    return ERR_WRONG_STATE;
  }

  /*******************************************************************************/
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_ChipIsBusy()) {
    return ERR_REQUEST;
  }


  ret = ERR_NONE;

  /*******************************************************************************/
  /* Turn field On if not already On */
  if (!field) {
    ret = ST25R95_FieldOn(protocol);
    field = true;
  }

  /*******************************************************************************/
  /* Start GT timer in case the GT value is set */
  if ((timings.GT != ST25R95_TIMING_NONE)) {
    /* Ensure that a SW timer doesn't have a lower value then the minimum  */
    ST25R95_TimerStart(tmr.GT, ST25R95_Conv1fcToMs(MAX((timings.GT), ST25R95_GT_MIN_1FC)));
  }

  return ret;
}

/*******************************************************************************/
ReturnCode ST25R95_FieldOff(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_ChipIsBusy()) {
    return ERR_REQUEST;
  }

  ST25R95_WakeUpModeStop();
  field = false;
  protocol = ST25R95_Protocol_FieldOff;
  return (ST25R95_FieldOff());
}



/*******************************************************************************/
ReturnCode ST25R95_StartTransceive(const ST25R95_TransceiveContext *ctx)
{
  /* Ensure that ST25R95 is already Initialized and the mode has been set */
  if ((state >= ST25R95_State_Mode_Set)) {
    /*******************************************************************************/
    /* Ensure that no previous operation is still ongoing */
    if (ST25R95_ChipIsBusy()) {
      return ERR_REQUEST;
    }

    TxRx.ctx = *ctx;

    /*******************************************************************************/
    if (ST25R95_IsModePassiveComm(currentMode)) { /* Passive Comms */
      if ((TxRx.ctx.fwt != ST25R95_FWT_NONE) && (TxRx.ctx.fwt != 0)) {
        ST25R95_SetFWT(protocol, TxRx.ctx.fwt);
      } else {
        /* Since ST25R95 does not support, use max FWT available */
        ST25R95_SetFWT(protocol, ST25R95_FWT_MAX);
      }
    }

    state       = ST25R95_State_TXRX;
    TxRx_state  = ST25R95_Transceive_State_TX_Idle;
    TxRx.status = ERR_BUSY;
    Lm.dataFlag = false;

    /*******************************************************************************/
    if ((ST25R95_Mode_Poll_NFCV == currentMode) || (ST25R95_Mode_Poll_PICOPASS == currentMode)) {
      /* In NFCV a TxRx with a valid txBuf and txBufSize==0 indicates to send an EOF */
      /* Skip logic below that would go directly into receive                        */
      if (TxRx.ctx.txBuf != NULL) {
        return  ERR_NONE;
      }
    }

    /*******************************************************************************/
    /* Check if the Transceive start performing Tx or goes directly to Rx          */
    if ((TxRx.ctx.txBuf == NULL) || (TxRx.ctx.txBufLen == 0)) {
      return ERR_NOT_IMPLEMENTED;
    }

    return ERR_NONE;
  }

  return ERR_WRONG_STATE;
}


/*******************************************************************************/
bool ST25R95_IsTransceiveInTx(void)
{
  return ((TxRx_state >= ST25R95_Transceive_State_TX_Idle) && (TxRx_state < ST25R95_Transceive_State_RX_Idle));
}


/*******************************************************************************/
bool ST25R95_IsTransceiveInRx(void)
{
  return (TxRx_state >= ST25R95_Transceive_State_RX_Idle);
}


/*******************************************************************************/
ReturnCode ST25R95_TransceiveBlockingTx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt)
{
  ReturnCode               ret;
  ST25R95_TransceiveContext    ctx;

  ST25R95_CreateByteFlagsTxRxContext(ctx, txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt);
  EXIT_ON_ERR(ret, ST25R95_StartTransceive(&ctx));


  return ST25R95_TransceiveRunBlockingTx();
}

/*******************************************************************************/
ReturnCode ST25R95_TransceiveBlockingRx(void)
{
  ReturnCode ret;

  do {
    ST25R95_Worker();
  } while (((ret = ST25R95_GetTransceiveStatus()) == ERR_BUSY) && ST25R95_IsTransceiveInRx());

  return ret;
}


/*******************************************************************************/
ReturnCode ST25R95_TransceiveBlockingTxRx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt)
{
  ReturnCode ret;

  EXIT_ON_ERR(ret, ST25R95_TransceiveBlockingTx(txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt));
  ret = ST25R95_TransceiveBlockingRx();

  /* Convert received bits to bytes */
  if (actLen != NULL) {
    *actLen = ST25R95_ConvBitsToBytes(*actLen);
  }

  return ret;
}


/*******************************************************************************/
ReturnCode ST25R95_RunTransceiveWorker(void)
{
  if (state == ST25R95_State_TXRX) {
    /* Run Tx or Rx state machines */
    if (ST25R95_IsTransceiveInTx()) {
      ST25R95_TransceiveTx();
      return ST25R95_GetTransceiveStatus();
    } else if (ST25R95_IsTransceiveInRx()) {
      ST25R95_TransceiveRx();
      return ST25R95_GetTransceiveStatus();
    }
  }
  return ERR_WRONG_STATE;
}

/*******************************************************************************/
ST25R95_TransceiveState ST25R95_GetTransceiveState(void)
{
  return TxRx_state;
}

ReturnCode ST25R95_GetTransceiveStatus(void)
{
  return ((TxRx_state == ST25R95_Transceive_State_Idle) ? TxRx.status : ERR_BUSY);
}


/*******************************************************************************/
ReturnCode ST25R95_GetTransceiveRSSI(uint16_t *rssi)
{
  NO_WARNING(rssi);

  return ERR_NOTSUPP;
}

/*******************************************************************************/
bool ST25R95_IsTransceiveSubcDetected(void)
{
  return false;
}


/*******************************************************************************/
void ST25R95_Worker(void)
{
  switch (state) {
    case ST25R95_State_TXRX:
      ST25R95_RunTransceiveWorker();
      break;

    case ST25R95_State_LM:
      ST25R95_RunListenModeWorker();
      break;

    case ST25R95_State_WUM:
      ST25R95_RunWakeUpModeWorker();
      break;

    /* Nothing to be done */
    default:
      break;
  }
}

/*******************************************************************************/
void ST25R95_TransceiveTx(void)
{
  uint8_t transmitFlag = 0;

  if (TxRx_state != TxRx_lastState) {
    /* ST25R95_LogD("%s: lastSt: %d curSt: %d \r\n", __FUNCTION__, TxRx_lastState, TxRx_state);*/
    TxRx_lastState = TxRx_state;
  }

  switch (TxRx_state) {
    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Idle:
      /* Nothing to do */
      TxRx_state = ST25R95_Transceive_State_TX_Wait_GT ;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Wait_GT:
      /* Wait for GT and FDT Poll */

      if (!ST25R95_IsGTExpired() || !ST25R95_TimerisExpired(tmr.FDTPoll)) {
        break;
      }
      tmr.GT = ST25R95_TIMING_NONE;
      TxRx_state = ST25R95_Transceive_State_TX_Transmit;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Transmit:
      /*******************************************************************************/
      /* Execute Pre Transceive Callback                                             */
      /*******************************************************************************/
      if (callbacks.preTxRx != NULL) {
        callbacks.preTxRx();
      }
      /*******************************************************************************/
      /* Prepare Rx                                                                  */
      /*******************************************************************************/
      ST25R95_IO_SPIPrepareRx(
        protocol,
        TxRx.ctx.rxBuf,
        ST25R95_ConvBitsToBytes(TxRx.ctx.rxBufLen),
        TxRx.ctx.rxRcvdLen,
        TxRx.ctx.flags,
        RxInformationBytes
      );
      /*******************************************************************************/
      /* Send the data                                                               */
      /*******************************************************************************/
      ST25R95_SPISendData(TxRx.ctx.txBuf, ST25R95_ConvBitsToBytes(TxRx.ctx.txBufLen), protocol, TxRx.ctx.flags);

      /* Start FDTPoll SW timer */
      ST25R95_TimerStart(tmr.FDTPoll, (ST25R95_SW_TMR_MIN_1MS + ST25R95_Conv1fcToMs(timings.FDTPoll)));

      TxRx_state = ST25R95_Transceive_State_TX_Wait_TXE;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Wait_TXE:
      if (!ST25R95_SPIIsTransmitCompleted()) {
        break;
      }
      transmitFlag = TxRx.ctx.txBufLen % 8;
      if (transmitFlag == 0) {
        transmitFlag = 0x8;
      }
      if (!(TxRx.ctx.flags & ST25R95_TXRX_FLAGS_CRC_TX_MANUAL)) {
        transmitFlag |= ST25R95_ISO14443A_APPENDCRC;
      }
      if (nfcaData.NfcaSplitFrame) {
        transmitFlag |= ST25R95_ISO14443A_SPLITFRAME;
      }
      if (currentMode == ST25R95_Mode_Poll_NFCA_T1T) {
        transmitFlag |= ST25R95_ISO14443A_TOPAZFORMAT;
      }
      ST25R95_SPISendTransmitFlag(protocol, transmitFlag);
      if (protocol == ST25R95_Protocol_CE_ISO14443A) {
        ST25R95_RearmListen();
      }
      TxRx_state = ST25R95_Transceive_State_TX_Done;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Done:
      /* If no rxBuf is provided do not wait/expect Rx */
      if (TxRx.ctx.rxBuf == NULL) {
        /* Clean up Transceive */
        //ST25R95_CleanupTransceive();
        TxRx.status = ERR_NONE;
        TxRx_state  = ST25R95_Transceive_State_Idle;
        break;
      }
      /* Goto Rx */
      TxRx_state  =  ST25R95_Transceive_State_RX_Idle;
      break;

    /*******************************************************************************/
    case ST25R95_Transceive_State_TX_Fail:
      /* Error should be assigned by previous state */
      if (TxRx.status == ERR_BUSY) {
        TxRx.status = ERR_SYSTEM;
      }
      TxRx_state = ST25R95_Transceive_State_Idle;
      break;

    /*******************************************************************************/
    default:
      TxRx.status = ERR_SYSTEM;
      TxRx_state  = ST25R95_Transceive_State_TX_Fail;
      break;
  }
}

/*******************************************************************************/
void ST25R95_TransceiveRx(void)
{
  ReturnCode retCode;

  if (TxRx_state != TxRx_lastState) {
    /*ST25R95_LogD("%s: lastSt: %d curSt: %d \r\n", __FUNCTION__, TxRx_lastState, TxRx_state);*/
    TxRx_lastState = TxRx_state;
  }

  switch (TxRx_state) {
    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Idle:

      /* Clear rx counters */
      if (TxRx.ctx.rxRcvdLen)  {
        *TxRx.ctx.rxRcvdLen = 0;
      }

      TxRx_state = ST25R95_Transceive_State_RX_Wait_RXE;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Wait_RXE:
      if (ST25R95_SPIPollRead(ST25R95_CONTROL_POLL_NO_TIMEOUT) == ERR_TIMEOUT) {
        break;
      }
      TxRx_state = ST25R95_Transceive_State_RX_Read_Data;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Read_Data:
      retCode = ST25R95_IO_SPI_Complete_Rx();
      /* Re-Start FDTPoll SW timer */
      ST25R95_TimerStart(tmr.FDTPoll, (ST25R95_SW_TMR_MIN_1MS + ST25R95_Conv1fcToMs(timings.FDTPoll)));

      if (TxRx.ctx.rxRcvdLen != NULL) {
        (*TxRx.ctx.rxRcvdLen) = ST25R95_ConvBytesToBits(*TxRx.ctx.rxRcvdLen);

        /*******************************************************************************/
        /* In case of Incomplete byte append the residual bits                         */
        /*******************************************************************************/
        if ((retCode >= ERR_INCOMPLETE_BYTE_01) && (retCode <= ERR_INCOMPLETE_BYTE_07)) {
          (*TxRx.ctx.rxRcvdLen) += (retCode - ERR_INCOMPLETE_BYTE);

          if ((*TxRx.ctx.rxRcvdLen) > 0) {
            (*TxRx.ctx.rxRcvdLen) -= ST25R95_BITS_IN_BYTE;
          }

          retCode = ERR_INCOMPLETE_BYTE;
        }
      }


      /*******************************************************************************/
      /* Execute Post Transceive Callback                                            */
      /*******************************************************************************/
      if (callbacks.postTxRx != NULL) {
        callbacks.postTxRx();
      }

      if (retCode != ERR_NONE) {
        TxRx.status = retCode;
        TxRx_state  = ST25R95_Transceive_State_RX_Fail;
        break;
      }
      TxRx_state = ST25R95_Transceive_State_RX_Done;
    /* fall through */

    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Done:
      TxRx.status = ERR_NONE;
      TxRx_state  = ST25R95_Transceive_State_Idle;
      break;


    /*******************************************************************************/
    case ST25R95_Transceive_State_RX_Fail:
      TxRx_state = ST25R95_Transceive_State_Idle;
      break;

    /*******************************************************************************/
    default:
      TxRx.status = ERR_SYSTEM;
      TxRx_state  = ST25R95_Transceive_State_RX_Fail;
      break;
  }
}

/*******************************************************************************/
ReturnCode ST25R95_TransceiveRunBlockingTx(void)
{
  ReturnCode ret;

  do {
    ST25R95_Worker();
  } while (((ret = ST25R95_GetTransceiveStatus()) == ERR_BUSY) && ST25R95_IsTransceiveInTx());

  if (ST25R95_IsTransceiveInRx()) {
    return ERR_NONE;
  }

  return ret;
}

/*******************************************************************************/
ReturnCode ST25R95_ISO14443ATransceiveShortFrame(ST25R95_14443AShortFrameCmd txCmd, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *rxRcvdLen, uint32_t fwt)
{
  ReturnCode ret;
  ST25R95_TransceiveContext ctx;
  uint8_t st95hShortFrameBuffer;
  /* Check if ST25R95 is properly initialized */
  if ((state < ST25R95_State_Mode_Set) || ((currentMode != ST25R95_Mode_Poll_NFCA) && (currentMode != ST25R95_Mode_Poll_NFCA_T1T)) || !field) {
    return ERR_WRONG_STATE;
  }

  /* Check for valid parameters */
  if ((rxBuf == NULL) || (rxRcvdLen == NULL) || (fwt == ST25R95_FWT_NONE)) {
    return ERR_PARAM;
  }

  nfcaData.NfcaSplitFrame = false;
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

  ST25R95_StartTransceive(&ctx);

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = ST25R95_TransceiveRunBlockingTx();
  if (ret == ERR_NONE) {
    ret = ST25R95_TransceiveBlockingRx();
  }

  /* ST25R95 has no means to disable CRC check, discard CRC errors */
  if (ret == ERR_CRC) {
    ret = ERR_NONE;
  }

  return ret;
}


/*******************************************************************************/
ReturnCode ST25R95_ISO14443ATransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt)
{
  ReturnCode            ret;

  EXIT_ON_ERR(ret, ST25R95_ISO14443AStartTransceiveAnticollisionFrame(buf, bytesToSend, bitsToSend, rxLength, fwt));
  ST25R95_RunBlocking(ret, ST25R95_ISO14443AGetTransceiveAnticollisionFrameStatus());

  return ret;
}



/*******************************************************************************/
ReturnCode ST25R95_ISO14443AStartTransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt)
{
  ReturnCode            ret;
  ST25R95_TransceiveContext ctx;

  /* Check if ST25R95 is properly initialized */
  if ((state < ST25R95_State_Mode_Set) || (currentMode != ST25R95_Mode_Poll_NFCA)) {
    return ERR_WRONG_STATE;
  }

  /* Check for valid parameters */
  if ((buf == NULL) || (bytesToSend == NULL) || (bitsToSend == NULL) || (rxLength == NULL)) {
    return ERR_PARAM;
  }

  nfcaData.NfcaSplitFrame = true;

  /*******************************************************************************/
  /* Prepare for Transceive                                                      */
  ctx.flags     = (ST25R95_TXRX_FLAGS_CRC_TX_MANUAL | ST25R95_TXRX_FLAGS_CRC_RX_KEEP);
  ctx.txBuf     = buf;
  ctx.txBufLen  = (ST25R95_ConvBytesToBits(*bytesToSend) + *bitsToSend);
  ctx.rxBuf     = (buf + (*bytesToSend));
  ctx.rxBufLen  = ST25R95_ConvBytesToBits(ST25R95_ISO14443A_SDD_RES_LEN);
  ctx.rxRcvdLen = rxLength;
  ctx.fwt       = fwt;

  EXIT_ON_ERR(ret, ST25R95_StartTransceive(&ctx));

  /*******************************************************************************/
  nfcaData.collByte = 0;

  /* save the collision byte */
  if ((*bitsToSend) > 0U) {
    buf[(*bytesToSend)] <<= (ST25R95_BITS_IN_BYTE - (*bitsToSend));
    buf[(*bytesToSend)] >>= (ST25R95_BITS_IN_BYTE - (*bitsToSend));
    nfcaData.collByte = buf[(*bytesToSend)];
  }

  nfcaData.buf         = buf;
  nfcaData.bytesToSend = bytesToSend;
  nfcaData.bitsToSend  = bitsToSend;
  nfcaData.rxLength    = rxLength;

  return ERR_NONE;
}


/*******************************************************************************/
ReturnCode ST25R95_ISO14443AGetTransceiveAnticollisionFrameStatus(void)
{
  ReturnCode   ret;

  /*******************************************************************************/
  /* Run Transceive blocking Tx*/
  ret = ST25R95_TransceiveRunBlockingTx();
  if (ret == ERR_NONE) {
    ret = ST25R95_TransceiveBlockingRx();
    /* ignore CRC error */
    if (ret == ERR_CRC) {
      ret = ERR_NONE;
    }

    /*******************************************************************************/
    if ((*nfcaData.bitsToSend) > 0U) {
      nfcaData.buf[(*nfcaData.bytesToSend)] >>= (*nfcaData.bitsToSend);
      nfcaData.buf[(*nfcaData.bytesToSend)] <<= (*nfcaData.bitsToSend);
      nfcaData.buf[(*nfcaData.bytesToSend)] |= nfcaData.collByte;
    }

    if ((ret == ERR_RF_COLLISION)) {
      (*nfcaData.rxLength) = ST25R95_ConvBytesToBits(RxInformationBytes[1]);
      (*nfcaData.bytesToSend) = (RxInformationBytes[1] + (*nfcaData.bytesToSend)) & 0xF;
      (*nfcaData.bitsToSend)  = RxInformationBytes[2] & 0x7;
    }
  }
  nfcaData.NfcaSplitFrame = false;
  return ret;
}

/*******************************************************************************/
ReturnCode ST25R95_ISO15693TransceiveAnticollisionFrame(uint8_t *txBuf, uint8_t txBufLen, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
  ReturnCode            ret;
  ST25R95_TransceiveContext ctx;

  /* Check if ST25R95 is properly initialized */
  if ((state < ST25R95_State_Mode_Set) || (currentMode != ST25R95_Mode_Poll_NFCV)) {
    return ERR_WRONG_STATE;
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

  EXIT_ON_ERR(ret, ST25R95_StartTransceive(&ctx));

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = ST25R95_TransceiveRunBlockingTx();
  if (ret == ERR_NONE) {
    ret = ST25R95_TransceiveBlockingRx();
  }

  return ret;
}

/*******************************************************************************/
ReturnCode ST25R95_ISO15693TransceiveEOFAnticollision(uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
  uint8_t dummy;

  return ST25R95_ISO15693TransceiveAnticollisionFrame(&dummy, 0, rxBuf, rxBufLen, actLen);
}

/*******************************************************************************/
ReturnCode ST25R95_ISO15693TransceiveEOF(uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen)
{
  ReturnCode ret;
  uint8_t    dummy;

  /* Check if ST25R95 is properly initialized */
  if ((state < ST25R95_State_Mode_Set) || (currentMode != ST25R95_Mode_Poll_NFCV)) {
    return ERR_WRONG_STATE;
  }

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = ST25R95_TransceiveBlockingTxRx(&dummy,
                                   0,
                                   rxBuf,
                                   rxBufLen,
                                   actLen,
                                   ((uint32_t)ST25R95_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)ST25R95_TXRX_FLAGS_CRC_RX_KEEP | (uint32_t)ST25R95_TXRX_FLAGS_AGC_ON),
                                   ST25R95_Conv64fcTo1fc(ST25R95_FWT_NONE));
  return ret;
}

/*******************************************************************************/
ReturnCode ST25R95_FeliCaPoll(ST25R95_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, ST25R95_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected)
{
  ReturnCode ret;

  EXIT_ON_ERR(ret, ST25R95_StartFeliCaPoll(slots, sysCode, reqCode, pollResList, pollResListSize, devicesDetected, collisionsDetected));
  ST25R95_RunBlocking(ret, ST25R95_GetFeliCaPollStatus());

  return ret;
}


/*******************************************************************************/
ReturnCode ST25R95_StartFeliCaPoll(ST25R95_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, ST25R95_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected)
{
  ReturnCode        ret;
  uint8_t           frame[ST25R95_FELICA_POLL_REQ_LEN - ST25R95_FELICA_LEN_LEN];  // LEN is added by ST25R95 automatically
  uint8_t           frameIdx;

  /* Check if ST25R95 is properly initialized */
  if ((state < ST25R95_State_Mode_Set) || (currentMode != ST25R95_Mode_Poll_NFCF)) {
    return ERR_WRONG_STATE;
  }

  frameIdx    = 0;
  nfcfData.colDetected = 0;
  nfcfData.devDetected = 0;

  /*******************************************************************************/
  /* Compute SENSF_REQ frame */
  frame[frameIdx++] = (uint8_t)FELICA_CMD_POLLING;       /* CMD: SENF_REQ                       */
  frame[frameIdx++] = (uint8_t)(sysCode >> 8);   /* System Code (SC)                    */
  frame[frameIdx++] = (uint8_t)(sysCode & 0xFFU); /* System Code (SC)                    */
  frame[frameIdx++] = reqCode;                   /* Communication Parameter Request (RC)*/
  frame[frameIdx++] = (uint8_t)slots;            /* TimeSlot (TSN)                      */

  ST25R95_SetSlotCounter((uint8_t)slots);
  /*******************************************************************************/
  /* NRT should not stop on reception - Use EMVCo mode to run NRT in nrt_emv     *
   * ERRORHANDLING_EMVCO has no special handling for NFC-F mode                  */
  conf.eHandling       = ERRORHANDLING_EMD;

  /*******************************************************************************/
  /* Run transceive blocking,
   * Calculate Total Response Time in(64/fc):
   *                       512 PICC process time + (n * 256 Time Slot duration)  */
  EXIT_ON_ERR(ret, ST25R95_TransceiveBlockingTx(frame,
                                            (uint16_t)frameIdx,
                                            (uint8_t *)nfcfData.pollResponses,
                                            ST25R95_FELICA_POLL_RES_LEN,
                                            &nfcfData.actLen,
                                            ST25R95_TXRX_FLAGS_CRC_RX_REMV,
                                            ST25R95_Conv64fcTo1fc(ST25R95_FELICA_POLL_DELAY_TIME + (ST25R95_FELICA_POLL_SLOT_TIME * ((uint32_t)slots + 1U)))));

  /* Store context */
  nfcfData.pollResList        = pollResList;
  nfcfData.pollResListSize    = pollResListSize;
  nfcfData.devicesDetected    = devicesDetected;
  nfcfData.collisionsDetected = collisionsDetected;
  nfcfData.slots              = slots;

  return ERR_NONE;
}


/*******************************************************************************/
ReturnCode ST25R95_GetFeliCaPollStatus(void)
{
  ReturnCode ret;

  /* Check if ST25R95 is properly initialized */
  if ((state != ST25R95_State_TXRX) || (currentMode != ST25R95_Mode_Poll_NFCF)) {
    return ERR_WRONG_STATE;
  }

  /* Wait until transceive has terminated */
  EXIT_ON_BUSY(ret, ST25R95_GetTransceiveStatus());

  /*******************************************************************************/
  /* If Tx OK, Wait for first response                                           */
  if (ret == ERR_NONE) {
    ret = ST25R95_TransceiveBlockingRx();
    if (ret != ERR_TIMEOUT) {
      /* If the reception was OK, new device found */
      if (ret == ERR_NONE) {
        nfcfData.devDetected++;
      }
      /* If the reception was not OK, mark as collision */
      else {
        nfcfData.colDetected++;
      }
    }
  }
  ST25R95_SetSlotCounter((uint8_t)ST25R95_FELICA_1_SLOT);
  /*******************************************************************************/
  /* Restore NRT to normal mode - back to previous error handling */
  conf.eHandling = nfcfData.curHandling;

  /*******************************************************************************/
  /* Assign output parameters if requested                                       */

  if ((nfcfData.pollResList != NULL) && (nfcfData.pollResListSize > 0) && (nfcfData.devDetected > 0)) {
    ST_MEMCPY(nfcfData.pollResList, nfcfData.pollResponses, (ST25R95_FELICA_POLL_RES_LEN * (uint32_t)MIN(nfcfData.pollResListSize, nfcfData.devDetected)));
  }

  if (nfcfData.devicesDetected != NULL) {
    *nfcfData.devicesDetected = nfcfData.devDetected;
  }

  if (nfcfData.collisionsDetected != NULL) {
    *nfcfData.collisionsDetected = nfcfData.colDetected;
  }

  return ((nfcfData.colDetected || nfcfData.devDetected) ? ERR_NONE : ret);
}

/*****************************************************************************
 *  Listen Mode                                                              *
 *****************************************************************************/

/*******************************************************************************/
bool ST25R95_IsExtFieldOn(void)
{
  return (false);
}

/*******************************************************************************/
ReturnCode ST25R95_ListenStart(uint32_t lmMask, const ST25R95_LmConfPA *confA, const ST25R95_LmConfPB *confB, const ST25R95_LmConfPF *confF, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen)
{
  NO_WARNING(confB);
  NO_WARNING(confF);

  /* Check if ST25R95 is not initialized */
  if (state == ST25R95_State_Idle) {
    return ERR_WRONG_STATE;
  }

  cardEmulT4AT = false;

  /*******************************************************************************/
  /* Check whether a Transceive operation is still ongoing                       *
   * ST25R95 cannot be interrupted while a Transceive is ongoing, reject         */
  if (ST25R95_ChipIsBusy()) {
    return ERR_REQUEST;
  }

  /*******************************************************************************/
  if ((lmMask & ST25R95_LM_MASK_Active_P2P) || (lmMask & ST25R95_LM_MASK_NFCB) || (lmMask & ST25R95_LM_MASK_NFCF)) {
    return ERR_NOTSUPP;
  }

  if ((lmMask & ST25R95_LM_MASK_NFCA)) {
    if (confA == NULL) {
      return (ERR_PARAM);
    }

    ST25R95_SetMode(ST25R95_Mode_Listen_NFCA, ST25R95_BitRate_106, ST25R95_BitRate_106);

    if (ST25R95_SetACFilter(confA) != ERR_NONE) {
      return (ERR_PARAM);
    }

    Lm.rxBuf    = rxBuf;
    Lm.rxBufLen = ST25R95_ConvBytesToBits(rxBufLen);
    Lm.rxLen    = rxLen;
    *Lm.rxLen   = 0;
    Lm.dataFlag = false;
    state       = ST25R95_State_LM;

    return ERR_NONE;
  }

  return ERR_NOTSUPP;
}

/*******************************************************************************/
static ReturnCode ST25R95_RunListenModeWorker(void)
{
  ReturnCode retCode = ERR_NONE;

  if (!ST25R95_IsInListen()) {
    retCode = ST25R95_Listen();
  }

  if (retCode != ERR_NONE) {
    return (retCode);
  }

  if (ST25R95_PollRead(ST25R95_CONTROL_POLL_NO_TIMEOUT) == ERR_TIMEOUT) {
    return (ERR_NONE);
  }

  ST25R95_PrepareRx(
    protocol,
    Lm.rxBuf,
    ST25R95_ConvBitsToBytes(Lm.rxBufLen),
    Lm.rxLen,
    (TxRx.ctx.flags & ST25R95_TXRX_FLAGS_CRC_RX_KEEP) != ST25R95_TXRX_FLAGS_CRC_RX_KEEP,
    RxInformationBytes
  );
  retCode = ST25R95_CompleteRx();
  if (!((retCode == ERR_LINK_LOSS) || ((retCode == ERR_NONE) && (Lm.rxLen == 0)))) {
    *Lm.rxLen   = ST25R95_ConvBytesToBits(*Lm.rxLen);
    Lm.dataFlag = true;
    state       = ST25R95_State_Mode_Set;
  }

  return (retCode);
}


/*******************************************************************************/
ReturnCode ST25R95_ListenStop(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_ChipIsBusy()) {
    return ERR_REQUEST;
  }

  ST25R95_CommandEcho(); /* kill listen command */
  ST25R95_FieldOff();
  state              = ST25R95_State_Init;
  currentMode        = ST25R95_Mode_None;
  protocol           = ST25R95_Protocol_FieldOff;
  field              = false;
  Lm.dataFlag        = false;

  return ERR_NONE;
}

/*******************************************************************************/
ReturnCode ST25R95_ListenSleepStart(ST25R95_LmState sleepSt, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen)
{
  ReturnCode retCode = ERR_PARAM;

  if (sleepSt == ST25R95_LM_STATE_SLEEP_A) {
    state       = ST25R95_State_LM;
    Lm.rxBuf    = rxBuf;
    Lm.rxBufLen = ST25R95_ConvBytesToBits(rxBufLen);
    Lm.rxLen    = rxLen;
    *Lm.rxLen   = 0;
    Lm.dataFlag = false;
    ST25R95_ListenSetState(sleepSt);
    retCode = ERR_NONE;
  }

  return retCode;
}


/*******************************************************************************/
ST25R95_LmState ST25R95_ListenGetState(bool *dataFlag, ST25R95_BitRate *lastBR)
{
  ST25R95_LmState state;

  /* Allow state retrieval even if state != ST25R95_State_LM so  *
   * that this Lm state can be used by caller after activation      */

  if (lastBR != NULL) {
    *lastBR = currenTxBitRate;
  }

  if (dataFlag != NULL) {
    *dataFlag = Lm.dataFlag;
  }
  state = ST25R95_GetLmState();

  if (((state == ST25R95_LM_STATE_Active_A) || (state == ST25R95_LM_STATE_Active_Ax)) && cardEmulT4AT) {
    state = ST25R95_LM_STATE_CARDEMU_4A;
  }

  return (state);
}


/*******************************************************************************/
ReturnCode ST25R95_ListenSetState(ST25R95_LmState newSt)
{
  ReturnCode retCode = ERR_NONE;
  uint8_t ST25R95_State;
  bool WasInListen;

  WasInListen = ST25R95_IsInListen();
  ST25R95_CommandEcho(); /* kill listen command */
  cardEmulT4AT = false;

  switch (newSt) {
    default:
    case ST25R95_LM_STATE_NOT_INIT:
    case ST25R95_LM_STATE_POWER_OFF:
    case ST25R95_LM_STATE_READY_B:
    case ST25R95_LM_STATE_READY_F:
    case ST25R95_LM_STATE_CARDEMU_4B:
    case ST25R95_LM_STATE_CARDEMU_3:
    case ST25R95_LM_STATE_TARGET_A:
    case ST25R95_LM_STATE_TARGET_F:
    case ST25R95_LM_STATE_SLEEP_B:
    case ST25R95_LM_STATE_SLEEP_AF:
      retCode = ERR_PARAM;
      break;

    case ST25R95_LM_STATE_IDLE:
      ST25R95_State = ST25R95_ACSTATE_IDLE;
      break;

    case ST25R95_LM_STATE_READY_A:
      ST25R95_State = ST25R95_ACSTATE_READYA;
      break;

    case ST25R95_LM_STATE_Active_A:
      ST25R95_State = ST25R95_ACSTATE_Active;
      break;

    case ST25R95_LM_STATE_SLEEP_A:
      ST25R95_State = ST25R95_ACSTATE_HALT;
      break;

    case ST25R95_LM_STATE_READY_Ax:
      ST25R95_State = ST25R95_ACSTATE_READYAX;
      break;

    case ST25R95_LM_STATE_Active_Ax:
      ST25R95_State = ST25R95_ACSTATE_ActiveX;
      break;

    case ST25R95_LM_STATE_CARDEMU_4A:
      ST25R95_State = ST25R95_GetLmState();
      if ((ST25R95_State != ST25R95_ACSTATE_Active) || (ST25R95_State != ST25R95_ACSTATE_ActiveX)) {
        ST25R95_State = ST25R95_ACSTATE_Active;
      }
      cardEmulT4AT = true;
      break;
  }
  if (retCode == ERR_NONE) {
    ST25R95_SetACState(ST25R95_State);
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
ReturnCode ST25R95_WakeUpModeStart(const ST25R95_WakeUpConfig *config)
{
  /* Check if ST25R95 is not initialized */
  if (state == ST25R95_State_Idle) {
    return ERR_WRONG_STATE;
  }

  if (config == NULL) {
    wum.cfg.period           = ST25R95_WUM_PERIOD_300MS;
    wum.cfg.irqTout          = false;
    wum.cfg.swTagDetect      = false;

    wum.cfg.indAmp.enabled   = true;
    wum.cfg.indPha.enabled   = false;
    wum.cfg.cap.enabled      = false;
    wum.cfg.indAmp.delta     = 8U;
    wum.cfg.indAmp.reference = ST25R95_WUM_REFERENCE_AUTO;
  } else {
    wum.cfg = *config;
  }

  /* Check for valid configuration */
  if (wum.cfg.cap.enabled || wum.cfg.indPha.enabled  || wum.cfg.swTagDetect || !wum.cfg.indAmp.enabled) {
    return ERR_PARAM;
  }

  if (wum.cfg.indAmp.reference == ST25R95_WUM_REFERENCE_AUTO) {
    wum.cfg.indAmp.reference = wum.CalTagDet;
  }
  if ((wum.cfg.indAmp.delta > wum.cfg.indAmp.reference) || ((((uint32_t)wum.cfg.indAmp.delta) + ((uint32_t)wum.cfg.indAmp.reference)) > 0xFCUL)) {
    return ERR_PARAM;
  }

  /* Use a fixed period of ~300 ms */
  ST25R95_IO_SPI_Idle(wum.cfg.indAmp.reference - wum.cfg.indAmp.delta, wum.cfg.indAmp.reference + wum.cfg.indAmp.delta, ST25R95_IDLE_DEFAULT_WUPERIOD);
  state     = ST25R95_State_WUM;
  wum.state = ST25R95_WUM_State_Enabled;
  return ERR_NONE;
}


/*******************************************************************************/
bool ST25R95_WakeUpModeIsEnabled(void)
{
  return ERR_NOTSUPP; /* ERR_NOTSUPP*/
}


/*******************************************************************************/
ReturnCode ST25R95_WakeUpModeGetInfo(bool force, ST25R95_WakeUpInfo *info)
{
  NO_WARNING(force);
  NO_WARNING(info);
  return ERR_NOTSUPP; /* ERR_NOTSUPP*/
}


/*******************************************************************************/
bool ST25R95_WakeUpModeHasWoke(void)
{
  return (wum.state >= ST25R95_WUM_State_Enabled_Woke);
}

/*******************************************************************************/
ReturnCode ST25R95_WakeUpModeStop(void)
{
  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_ChipIsBusy()) {
    return ERR_REQUEST;
  }


  if (wum.state == ST25R95_WUM_State_Not_Init) {
    return ERR_WRONG_STATE;
  }

  wum.state = ST25R95_WUM_State_Not_Init;
  ST25R95_SPIKillIdle();
  ST25R95_SPICommandEcho();
  return ERR_NONE;
}


/*******************************************************************************/
void ST25R95_RunWakeUpModeWorker(void)
{
  if (state != ST25R95_State_WUM) {
    return;
  }

  switch (wum.state) {
    case ST25R95_WUM_State_Enabled:
    case ST25R95_WUM_State_Enabled_Woke:
      if (ST25R95_SPIPollRead(ST25R95_CONTROL_POLL_NO_TIMEOUT) != ERR_TIMEOUT) {
        ST25R95_SPIGetIdleResponse();
        wum.state = ST25R95_WUM_State_Enabled_Woke;
      }

    default:
      break;
  }
}

/*******************************************************************************
 *  RF Chip                                                                    *
 *******************************************************************************/

/*******************************************************************************/
ReturnCode ST25R95_ChipWriteReg(uint16_t reg, const uint8_t *values, uint8_t len)
{
  ReturnCode retCode;

  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_ChipIsBusy()) {
    return ERR_REQUEST;
  }


  if (len != 1) {
    retCode = ERR_PARAM;
  } else {
    retCode = ST25R95_WriteReg(protocol, reg, values[0]);
  }
  return (retCode);
}

/*******************************************************************************/
ReturnCode ST25R95_ChipReadReg(uint16_t reg, uint8_t *values, uint8_t len)
{
  ReturnCode retCode;

  /* Ensure that no previous operation is still ongoing */
  if (ST25R95_ChipIsBusy()) {
    return ERR_REQUEST;
  }


  if (len != 1) {
    retCode = ERR_PARAM;
  } else {
    retCode = ST25R95_IO_ReadReg(reg, values);
  }
  return (retCode);
}

/*******************************************************************************/
ReturnCode ST25R95_ChipChangeRegBits(uint16_t reg, uint8_t valueMask, uint8_t value)
{
  ReturnCode retCode;
  uint8_t tmp;

  retCode = ST25R95_IO_ReadReg(reg, &tmp);

  if (retCode == ERR_NONE) {
    /* mask out the bits we don't want to change */
    tmp &= (uint8_t)(~((uint32_t)valueMask));
    /* set the new value */
    tmp |= (value & valueMask);
    retCode = ST25R95_WriteReg(protocol, reg, tmp);
  }

  return retCode;
}
