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
#include <string.h>

#include "RFal.h"
#include "RFal_NFC.h"
#include "RFal_NFCV.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#include "NeonRTOS.h"

#define RFAL_NFCV_INV_REQ_FLAG            0x06U  /*!< INVENTORY_REQ  INV_FLAG  Digital  2.1  9.6.1                      */
#define RFAL_NFCV_MASKVAL_MAX_LEN         8U     /*!< Mask value max length: 64 bits  (UID length)                      */
#define RFAL_NFCV_MASKVAL_MAX_1SLOT_LEN   64U    /*!< Mask value max length in 1 Slot mode in bits  Digital 2.1 9.6.1.6 */
#define RFAL_NFCV_MASKVAL_MAX_16SLOT_LEN  60U    /*!< Mask value max length in 16 Slot mode in bits Digital 2.1 9.6.1.6 */
#define RFAL_NFCV_MAX_SLOTS               16U    /*!< NFC-V max number of Slots                                         */
#define RFAL_NFCV_INV_REQ_HEADER_LEN      3U     /*!< INVENTORY_REQ header length (INV_FLAG, CMD, MASK_LEN)             */
#define RFAL_NFCV_INV_RES_LEN             10U    /*!< INVENTORY_RES length                                              */
#define RFAL_NFCV_WR_MUL_REQ_HEADER_LEN   4U     /*!< Write Multiple header length (INV_FLAG, CMD, [UID], BNo, Bno)     */


#define RFAL_NFCV_CMD_LEN                 1U     /*!< Commandbyte length                                                */
#define RFAL_NFCV_FLAG_POS                0U     /*!< Flag byte position                                                */
#define RFAL_NFCV_FLAG_LEN                1U     /*!< Flag byte length                                                  */
#define RFAL_NFCV_PARAM_LEN               1U     /*!< CMD specific parameter length  (e.g. Extended Get System Info)    */
#define RFAL_NFCV_DATASTART_POS           1U     /*!< Position of start of data                                         */
#define RFAL_NFCV_DSFI_LEN                1U     /*!< DSFID length                                                      */
#define RFAL_NFCV_SLPREQ_REQ_FLAG         0x22U  /*!< SLPV_REQ request flags Digital 2.0 (Candidate) 9.7.1.1            */
#define RFAL_NFCV_RES_FLAG_NOERROR        0x00U  /*!< RES_FLAG indicating no error (checked during activation)          */

#define RFAL_NFCV_MAX_COLL_SUPPORTED      16U    /*!< Maximum number of collisions supported by the Anticollision loop  */

#define RFAL_NFCV_FDT_MAX1                4394U  /*!< Read alike command FWT FDTV,LISTEN,MAX1  Digital 2.0 B.5          */

/*! Maximum Wait time FDTV,EOF and MAX2   FDTV,LISTEN,MAX2 + Tolerance = 270644 + 512 = 271156 (~20ms)  Digital 2.3 B.5*/
#define RFAL_NFCV_FDT_MAX                 271156U



/*! Time from special frame to EOF
 *                    ISO15693 2009 10.4.2                 : <20ms
 *                    NFC Forum defines Digital 2.3  9.7.4 : FDTV,EOF = [10 ; 20]ms
 */
#define RFAL_NFCV_FDT_EOF               rfalConvMsTo1fc(16)



/*! Time between slots - ISO 15693 defines t3min depending on modulation depth and data rate.
 *  With only high-bitrate supported, AM modulation and a length of 12 bytes (96bits) for INV_RES we get:
 *                    - ISO t3min = 96/26 ms + 300us = 4 ms
 *                    - NFC Forum defines FDTV,INVENT_NORES = (4394 + 2048)/fc. Digital 2.0  B.5*/
#define RFAL_NFCV_FDT_V_INVENT_NORES      4U


/*! Checks if a valid INVENTORY_RES is valid    Digital 2.2  9.6.2.1 & 9.6.2.3  */
#define RFal_NFCV_CheckInvRes( f, l )     (((l)==rfalConvBytesToBits(RFAL_NFCV_INV_RES_LEN + RFAL_NFCV_CRC_LEN)) && ((f)==RFAL_NFCV_RES_FLAG_NOERROR))

/*! NFC-V INVENTORY_REQ format   Digital 2.0 9.6.1 */
typedef struct {
  uint8_t  INV_FLAG;                              /*!< Inventory Flags    */
  uint8_t  CMD;                                   /*!< Command code: 01h  */
  uint8_t  MASK_LEN;                              /*!< Mask Value Length  */
  uint8_t  MASK_VALUE[RFAL_NFCV_MASKVAL_MAX_LEN]; /*!< Mask Value         */
} RFal_NFCV_InventoryReq;


/*! NFC-V SLP_REQ format   Digital 2.0 (Candidate) 9.7.1 */
typedef struct {
  uint8_t  REQ_FLAG;                              /*!< Request Flags      */
  uint8_t  CMD;                                   /*!< Command code: 02h  */
  uint8_t  UID[RFAL_NFCV_UID_LEN];                /*!< Mask Value         */
} RFal_NFCV_SlpvReq;


/*! Container for a collision found during Anticollision loop */
typedef struct {
  uint8_t  maskLen;
  uint8_t  maskVal[RFAL_NFCV_MASKVAL_MAX_LEN];
} RFal_NFCV_Collision;

/*******************************************************************************/
static NFC_OpResult RFal_NFCV_ParseError(uint8_t err)
{
  switch (err) {
    case RFAL_NFCV_ERROR_CMD_NOT_SUPPORTED:
    case RFAL_NFCV_ERROR_OPTION_NOT_SUPPORTED:
      return NFC_Unsupport;

    case RFAL_NFCV_ERROR_CMD_NOT_RECOGNIZED:
      return NFC_ProtocolError;

    case RFAL_NFCV_ERROR_WRITE_FAILED:
      return NFC_WriteFailed;

    default:
      return NFC_RequestError;
  }
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerInit(void)
{
  NFC_OpResult ret;

  EXIT_ON_ERR(ret, RFal_SetMode(RFAL_MODE_POLL_NFCV, RFAL_BR_26p48, RFAL_BR_26p48));
  RFal_SetErrorHandling(ERRORHANDLING_NONE);

  RFal_SetGT(RFAL_GT_NFCV);
  RFal_SetFDTListen(RFAL_FDT_LISTEN_NFCV_POLLER);
  RFal_SetFDTPoll(RFAL_FDT_POLL_NFCV_POLLER);

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerCheckPresence(RFal_NFCV_InventoryRes *invRes)
{
  NFC_OpResult ret;

  /* INVENTORY_REQ with 1 slot and no Mask   Activity 2.0 (Candidate) 9.2.3.32 */
  ret = RFal_NFCV_PollerInventory(RFAL_NFCV_NUM_SLOTS_1, 0, NULL, invRes, NULL);

  if ((ret == NFC_RF_Collision) || (ret == NFC_CRC_Error)  ||
      (ret == NFC_FramingError) || (ret == NFC_ProtocolError)) {
    ret = NFC_OK;
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerInventory(RFal_NFCV_NumSlots nSlots, uint8_t maskLen, const uint8_t *maskVal, RFal_NFCV_InventoryRes *invRes, uint16_t *rcvdLen)
{
  NFC_OpResult           ret;
  RFal_NFCV_InventoryReq invReq;
  uint16_t             rxLen;

  if (((maskVal == NULL) && (maskLen != 0U)) || (invRes == NULL)) {
    return NFC_InvalidParameter;
  }

  invReq.INV_FLAG = (RFAL_NFCV_INV_REQ_FLAG | (uint8_t)nSlots);
  invReq.CMD      = RFAL_NFCV_CMD_INVENTORY;
  invReq.MASK_LEN = (uint8_t)MIN(maskLen, ((nSlots == RFAL_NFCV_NUM_SLOTS_1) ? RFAL_NFCV_MASKVAL_MAX_1SLOT_LEN : RFAL_NFCV_MASKVAL_MAX_16SLOT_LEN));     /* Digital 2.0  9.6.1.6 */

  if ((rfalConvBitsToBytes(invReq.MASK_LEN) > 0U) && (maskVal != NULL)) {   /* MISRA 21.18 & 1.3 */
    memcpy(invReq.MASK_VALUE, maskVal, rfalConvBitsToBytes(invReq.MASK_LEN));
  }

  ret = RFal_ISO15693TransceiveAnticollisionFrame((uint8_t *)&invReq, (uint8_t)(RFAL_NFCV_INV_REQ_HEADER_LEN + rfalConvBitsToBytes(invReq.MASK_LEN)), (uint8_t *)invRes, sizeof(RFal_NFCV_InventoryRes), &rxLen);

  /* Check for optional output parameter */
  if (rcvdLen != NULL) {
    *rcvdLen = rxLen;
  }

  if (ret == NFC_OK) {
    /* Check for valid INVENTORY_RES   Digital 2.2  9.6.2.1 & 9.6.2.3 */
    if (!RFal_NFCV_CheckInvRes(invRes->RES_FLAG, rxLen)) {
      return NFC_ProtocolError;
    }
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerCollisionResolution(RFal_ComplianceMode compMode, uint8_t devLimit, RFal_NFCV_ListenDevice *nfcvDevList, uint8_t *devCnt)
{
  NFC_OpResult        ret;
  uint8_t           slotNum;
  uint16_t          rcvdLen;
  uint8_t           colIt;
  uint8_t           colCnt;
  uint8_t           colPos;
  bool              colPending;
  RFal_NFCV_Collision colFound[RFAL_NFCV_MAX_COLL_SUPPORTED];


  if ((nfcvDevList == NULL) || (devCnt == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Initialize parameters */
  *devCnt = 0;
  colIt         = 0;
  colCnt        = 0;
  colPending    = false;
  memset(colFound, 0x00, (sizeof(RFal_NFCV_Collision)*RFAL_NFCV_MAX_COLL_SUPPORTED));

  if (devLimit > 0U) {      /* MISRA 21.18 */
    memset(nfcvDevList, 0x00, (sizeof(RFal_NFCV_ListenDevice)*devLimit));
  }

  if (compMode == RFAL_COMPLIANCE_MODE_NFC) {
    /* Send INVENTORY_REQ with one slot   Activity 2.0  9.3.7.1  (Symbol 0)  */
    ret = RFal_NFCV_PollerInventory(RFAL_NFCV_NUM_SLOTS_1, 0, NULL, &nfcvDevList->InvRes, NULL);

    /* Exit if no device found                              Activity 2.1  9.3.7.2 (Symbol 1)  */
    /* Exit if no correct frame (no Transmission Error)     Activity 2.1  9.3.7.3 (Symbol 2)  */
    if ((ret == NFC_SlaveTimeout) || ((ret == NFC_ProtocolError))) {
      return NFC_OK;
    }
    /* Valid Response found without transmission error/collision    Activity 2.1  9.3.7.6 (Symbol 5)  */
    if (ret == NFC_OK) {
      (*devCnt)++;
      return NFC_OK;
    }

    /* A Collision has been identified  Activity 2.1  9.3.7.4  (Symbol 3) */
    colPending = true;
    colCnt        = 1;

    /* Check if the Collision Resolution is set to perform only Collision detection   Activity 2.1  9.3.7.5 (Symbol 4)*/
    if (devLimit == 0U) {
      return NFC_RF_Collision;
    }

    NeonRTOS_Sleep(RFAL_NFCV_FDT_V_INVENT_NORES);

    /*******************************************************************************/
    /* Collisions pending, Anticollision loop must be executed                     */
    /*******************************************************************************/
  } else {
    /* Advance to 16 slots below without mask. Will give a good chance to identify multiple cards */
    colPending = true;
    colCnt        = 1;
  }


  /* Execute until all collisions are resolved Activity 2.1 9.3.7.18  (Symbol 17) */
  do {
    /* Activity 2.1  9.3.7.7  (Symbol 6 / 7) */
    colPending = false;
    slotNum    = 0;

    do {
      if (slotNum == 0U) {
        /* Send INVENTORY_REQ with 16 slots   Activity 2.1  9.3.7.9  (Symbol 8) */
        ret = RFal_NFCV_PollerInventory(RFAL_NFCV_NUM_SLOTS_16, colFound[colIt].maskLen, colFound[colIt].maskVal, &nfcvDevList[(*devCnt)].InvRes, &rcvdLen);
      } else {
        ret = RFal_ISO15693TransceiveEOFAnticollision((uint8_t *)&nfcvDevList[(*devCnt)].InvRes, sizeof(RFal_NFCV_InventoryRes), &rcvdLen);
      }
      slotNum++;

      /*******************************************************************************/
      if (ret != NFC_SlaveTimeout) {
        if (rcvdLen < rfalConvBytesToBits(RFAL_NFCV_INV_RES_LEN + RFAL_NFCV_CRC_LEN)) {
          /* If only a partial frame was received make sure the FDT_V_INVENT_NORES is fulfilled */
          NeonRTOS_Sleep(RFAL_NFCV_FDT_V_INVENT_NORES);
        }

        /* Check if response is a correct frame (no TxRx error)  Activity 2.1  9.3.7.11  (Symbol 10)*/
        if ((ret == NFC_OK) || (ret == NFC_ProtocolError)) {
          /* Check if the device found is already on the list and its response is a valid INVENTORY_RES */
          if (RFal_NFCV_CheckInvRes(nfcvDevList[(*devCnt)].InvRes.RES_FLAG, rcvdLen)) {
            /* Activity 2.1  9.3.7.12  (Symbol 11) */
            (*devCnt)++;
          }
        } else { /* Treat everything else as collision */
          /* Activity 2.1  9.3.7.17  (Symbol 16) */
          colPending = true;


          /*******************************************************************************/
          /* Ensure that this collision still fits on the container */
          if (colCnt < RFAL_NFCV_MAX_COLL_SUPPORTED) {
            /* Store this collision on the container to be resolved later */
            /* Activity 2.1  9.3.7.17  (Symbol 16): add the collision information
             * (MASK_VAL + SN) to the list containing the collision information */
            memcpy(colFound[colCnt].maskVal, colFound[colIt].maskVal, RFAL_NFCV_UID_LEN);
            colPos = colFound[colIt].maskLen;
            colFound[colCnt].maskVal[(colPos / RFAL_BITS_IN_BYTE)]      &= (uint8_t)((1U << (colPos % RFAL_BITS_IN_BYTE)) - 1U);
            colFound[colCnt].maskVal[(colPos / RFAL_BITS_IN_BYTE)]      |= (uint8_t)((slotNum - 1U) << (colPos % RFAL_BITS_IN_BYTE));
            colFound[colCnt].maskVal[((colPos / RFAL_BITS_IN_BYTE) + 1U)]  = (uint8_t)((slotNum - 1U) >> (RFAL_BITS_IN_BYTE - (colPos % RFAL_BITS_IN_BYTE)));

            colFound[colCnt].maskLen = (colFound[colIt].maskLen + 4U);

            colCnt++;
          }
        }
      } else {
        /* Timeout */
        NeonRTOS_Sleep(RFAL_NFCV_FDT_V_INVENT_NORES);
      }

      /* Check if devices found have reached device limit   Activity 2.1  9.3.7.13  (Symbol 12) */
      if (*devCnt >= devLimit) {
        return NFC_OK;
      }
    } while (slotNum < RFAL_NFCV_MAX_SLOTS);   /* Slot loop             */
    colIt++;
  } while (colIt < colCnt);                      /* Collisions found loop */

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerSleepCollisionResolution(uint8_t devLimit, RFal_NFCV_ListenDevice *nfcvDevList, uint8_t *devCnt)
{
  uint8_t    tmpDevCnt;
  NFC_OpResult ret;
  uint8_t    i;

  if ((nfcvDevList == NULL) || (devCnt == NULL)) {
    return NFC_InvalidParameter;
  }

  *devCnt = 0;

  do {
    tmpDevCnt = 0;
    ret = RFal_NFCV_PollerCollisionResolution(RFAL_COMPLIANCE_MODE_ISO, (devLimit - *devCnt), &nfcvDevList[*devCnt], &tmpDevCnt);

    for (i = *devCnt; i < (*devCnt + tmpDevCnt); i++) {
      RFal_NFCV_PollerSleep(0x00, nfcvDevList[i].InvRes.UID);
      nfcvDevList[i].isSleep = true;
    }
    *devCnt += tmpDevCnt;
  } while ((ret == NFC_OK) && (tmpDevCnt > 0U) && (*devCnt < devLimit));

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerSleep(uint8_t flags, const uint8_t *uid)
{
  NFC_OpResult      ret;
  RFal_NFCV_SlpvReq slpReq;
  uint8_t         rxBuf;    /* dummy buffer, just to perform Rx */

  if (uid == NULL) {
    return NFC_InvalidParameter;
  }

  /* Compute SLPV_REQ */
  slpReq.REQ_FLAG = (flags | (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS);   /* Should be with UID according Digital 2.0 (Candidate) 9.7.1.1 */
  slpReq.CMD      = RFAL_NFCV_CMD_SLPV;
  memcpy(slpReq.UID, uid, RFAL_NFCV_UID_LEN);

  /* NFC Forum device SHALL wait at least FDTVpp to consider the SLPV acknowledged (FDTVpp = FDTVpoll)  Digital 2.0 (Candidate)  9.7  9.8.2  */
  ret = RFal_TransceiveBlockingTxRx((uint8_t *)&slpReq, sizeof(RFal_NFCV_SlpvReq), &rxBuf, sizeof(rxBuf), NULL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCV_FDT_MAX1);
  if (ret != NFC_SlaveTimeout) {
    return ret;
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerSelect(uint8_t flags, const uint8_t *uid)
{
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  if (uid == NULL) {
    return NFC_InvalidParameter;
  }

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_SELECT, flags, RFAL_NFCV_PARAM_SKIP, uid, NULL, 0U, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerReadSingleBlock(uint8_t flags, const uint8_t *uid, uint8_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t bn;

  bn = blockNum;

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_READ_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, &bn, sizeof(uint8_t), rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerWriteSingleBlock(uint8_t flags, const uint8_t *uid, uint8_t blockNum, const uint8_t *wrData, uint8_t blockLen)
{
  uint8_t            data[(RFAL_NFCV_BLOCKNUM_LEN + RFAL_NFCV_MAX_BLOCK_LEN)];
  uint8_t            dataLen;
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  /* Check for valid parameters */
  if ((blockLen == 0U) || (blockLen > (uint8_t)RFAL_NFCV_MAX_BLOCK_LEN) || (wrData == NULL)) {
    return NFC_InvalidParameter;
  }

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = blockNum;                    /* Set Block Number (8 bits)  */
  memcpy(&data[dataLen], wrData, blockLen);   /* Append Block data to write */
  dataLen += blockLen;

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_WRITE_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerLockBlock(uint8_t flags, const uint8_t *uid, uint8_t blockNum)
{
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;
  uint8_t            bn;

  bn = blockNum;

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_LOCK_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, &bn, sizeof(uint8_t), (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t            data[(RFAL_NFCV_BLOCKNUM_LEN + RFAL_NFCV_BLOCKNUM_LEN)];
  uint8_t            dataLen;

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = firstBlockNum;                    /* Set first Block Number       */
  data[dataLen++] = numOfBlocks;                      /* Set number of blocks to read */

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_READ_MULTIPLE_BLOCKS, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerWriteMultipleBlocks(uint8_t flags, const uint8_t *uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t *txBuf, uint16_t txBufLen, uint8_t blockLen, const uint8_t *wrData, uint16_t wrDataLen)
{
  NFC_OpResult         ret;
  uint16_t           rcvLen;
  uint16_t           reqLen;
  RFal_NFCV_GenericRes res;
  uint16_t           msgIt;

  /* Calculate required buffer length */
  reqLen = (uint16_t)((uid != NULL) ? (RFAL_NFCV_WR_MUL_REQ_HEADER_LEN + RFAL_NFCV_UID_LEN + wrDataLen) : (RFAL_NFCV_WR_MUL_REQ_HEADER_LEN + wrDataLen));

  if ((reqLen > txBufLen) || (blockLen > (uint8_t)RFAL_NFCV_MAX_BLOCK_LEN) || ((((uint16_t)numOfBlocks) * (uint16_t)blockLen) != wrDataLen) || (numOfBlocks == 0U) || (wrData == NULL)) {
    return NFC_InvalidParameter;
  }

  msgIt = 0;

  /* Compute Request Command */
  txBuf[msgIt++] = (uint8_t)(flags & (~((uint32_t)RFAL_NFCV_REQ_FLAG_ADDRESS)));
  txBuf[msgIt++] = RFAL_NFCV_CMD_WRITE_MULTIPLE_BLOCKS;

  /* Check if Request is to be sent in Addressed mode. Select mode flag shall be set by user */
  if (uid != NULL) {
    txBuf[RFAL_NFCV_FLAG_POS] |= (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS;
    memcpy(&txBuf[msgIt], uid, RFAL_NFCV_UID_LEN);
    msgIt += (uint8_t)RFAL_NFCV_UID_LEN;
  } else {
    txBuf[RFAL_NFCV_FLAG_POS] |= (uint8_t)RFAL_NFCV_REQ_FLAG_SELECT;
  }

  txBuf[msgIt++] = firstBlockNum;
  txBuf[msgIt++] = (numOfBlocks - 1U);

  if (wrDataLen > 0U) {        /* MISRA 21.18 */
    memcpy(&txBuf[msgIt], wrData, wrDataLen);
    msgIt += wrDataLen;
  }

  /* Transceive Command */
  ret = RFal_TransceiveBlockingTxRx(txBuf, msgIt, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCV_FDT_MAX);

  if (ret != NFC_OK) {
    return ret;
  }

  /* Check if the response minimum length has been received */
  if (rcvLen < (uint8_t)RFAL_NFCV_FLAG_LEN) {
    return NFC_ProtocolError;
  }

  /* Check if an error has been signalled */
  if ((res.RES_FLAG & (uint8_t)RFAL_NFCV_RES_FLAG_ERROR) != 0U) {
    return RFal_NFCV_ParseError(*res.data);
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerExtendedReadSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[RFAL_NFCV_BLOCKNUM_EXTENDED_LEN];
  uint8_t dataLen;

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)blockNum; /* TS T5T 1.0 BNo is considered as a multi-byte field. TS T5T 1.0 5.1.1.13 multi-byte field follows [DIGITAL]. [DIGITAL] 9.3.1 A multiple byte field is transmitted LSB first. */
  data[dataLen++] = (uint8_t)((blockNum >> 8U) & 0xFFU);

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_EXTENDED_READ_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}


/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerExtendedWriteSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, const uint8_t *wrData, uint8_t blockLen)
{
  uint8_t            data[(RFAL_NFCV_BLOCKNUM_EXTENDED_LEN + RFAL_NFCV_MAX_BLOCK_LEN)];
  uint8_t            dataLen;
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  /* Check for valid parameters */
  if ((blockLen == 0U) || (blockLen > (uint8_t)RFAL_NFCV_MAX_BLOCK_LEN)) {
    return NFC_InvalidParameter;
  }

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)blockNum;                    /* TS T5T 1.0 BNo is considered as a multi-byte field. TS T5T 1.0 5.1.1.13 multi-byte field follows [DIGITAL]. [DIGITAL] 9.3.1 A multiple byte field is transmitted LSB first. */
  data[dataLen++] = (uint8_t)((blockNum >> 8U) & 0xFFU);
  memcpy(&data[dataLen], wrData, blockLen);           /* Append Block data to write */
  dataLen += blockLen;

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_EXTENDED_WRITE_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerExtendedLockSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum)
{
  uint8_t            data[RFAL_NFCV_BLOCKNUM_EXTENDED_LEN];
  uint8_t            dataLen;
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)blockNum;                   /* TS T5T 1.0 BNo is considered as a multi-byte field. TS T5T 1.0 5.1.1.13 multi-byte field follows [DIGITAL]. [DIGITAL] 9.3.1 A multiple byte field is transmitted LSB first. */
  data[dataLen++] = (uint8_t)((blockNum >> 8U) & 0xFFU);

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_EXTENDED_LOCK_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerExtendedReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[(RFAL_NFCV_BLOCKNUM_EXTENDED_LEN + RFAL_NFCV_BLOCKNUM_EXTENDED_LEN)];
  uint8_t dataLen;

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)((firstBlockNum >> 0U) & 0xFFU);
  data[dataLen++] = (uint8_t)((firstBlockNum >> 8U) & 0xFFU);
  data[dataLen++] = (uint8_t)((numOfBlocks >> 0U) & 0xFFU);
  data[dataLen++] = (uint8_t)((numOfBlocks >> 8U) & 0xFFU);

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_EXTENDED_READ_MULTIPLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerExtendedWriteMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t *txBuf, uint16_t txBufLen, uint8_t blockLen, const uint8_t *wrData, uint16_t wrDataLen)
{
  NFC_OpResult         ret;
  uint16_t           rcvLen;
  uint16_t           reqLen;
  RFal_NFCV_GenericRes res;
  uint16_t           msgIt;
  uint16_t           nBlocks;

  /* Calculate required buffer length */
  reqLen = ((uid != NULL) ? (RFAL_NFCV_WR_MUL_REQ_HEADER_LEN + RFAL_NFCV_UID_LEN + wrDataLen) : (RFAL_NFCV_WR_MUL_REQ_HEADER_LEN + wrDataLen));

  if ((reqLen > txBufLen) || (blockLen > (uint8_t)RFAL_NFCV_MAX_BLOCK_LEN) || (((uint16_t)numOfBlocks * (uint16_t)blockLen) != wrDataLen) || (numOfBlocks == 0U)) {
    return NFC_InvalidParameter;
  }

  msgIt   = 0;
  nBlocks = (numOfBlocks - 1U);

  /* Compute Request Command */
  txBuf[msgIt++] = (uint8_t)(flags & (~((uint32_t)RFAL_NFCV_REQ_FLAG_ADDRESS)));
  txBuf[msgIt++] = RFAL_NFCV_CMD_EXTENDED_WRITE_MULTIPLE_BLOCK;

  /* Check if Request is to be sent in Addressed mode. Select mode flag shall be set by user */
  if (uid != NULL) {
    txBuf[RFAL_NFCV_FLAG_POS] |= (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS;
    memcpy(&txBuf[msgIt], uid, RFAL_NFCV_UID_LEN);
    msgIt += (uint8_t)RFAL_NFCV_UID_LEN;
  }

  txBuf[msgIt++] = (uint8_t)((firstBlockNum >> 0) & 0xFFU);
  txBuf[msgIt++] = (uint8_t)((firstBlockNum >> 8) & 0xFFU);
  txBuf[msgIt++] = (uint8_t)((nBlocks >> 0) & 0xFFU);
  txBuf[msgIt++] = (uint8_t)((nBlocks >> 8) & 0xFFU);

  if (wrDataLen > 0U) {        /* MISRA 21.18 */
    memcpy(&txBuf[msgIt], wrData, wrDataLen);
    msgIt += wrDataLen;
  }

  /* Transceive Command */
  ret = RFal_TransceiveBlockingTxRx(txBuf, msgIt, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCV_FDT_MAX);

  if (ret != NFC_OK) {
    return ret;
  }

  /* Check if the response minimum length has been received */
  if (rcvLen < (uint8_t)RFAL_NFCV_FLAG_LEN) {
    return NFC_ProtocolError;
  }

  /* Check if an error has been signalled */
  if ((res.RES_FLAG & (uint8_t)RFAL_NFCV_RES_FLAG_ERROR) != 0U) {
    return RFal_NFCV_ParseError(*res.data);
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerGetSystemInformation(uint8_t flags, const uint8_t *uid, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_GET_SYS_INFO, flags, RFAL_NFCV_PARAM_SKIP, uid, NULL, 0U, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerExtendedGetSystemInformation(uint8_t flags, const uint8_t *uid, uint8_t requestField, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_EXTENDED_GET_SYS_INFO, flags, requestField, uid, NULL, 0U, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCV_PollerTransceiveReq(uint8_t cmd, uint8_t flags, uint8_t param, const uint8_t *uid, const uint8_t *data, uint16_t dataLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  NFC_OpResult         ret;
  RFal_NFCV_GenericReq req;
  uint8_t            msgIt;
  RFal_BitRate        rxBR;
  bool               fastMode;
  bool               specialFrame;

  msgIt    = 0;
  fastMode = false;
  specialFrame = false;

  /* Check for valid parameters */
  if ((rxBuf == NULL) || (rcvLen == NULL) || ((dataLen > 0U) && (data == NULL))                                  ||
      (dataLen > ((uid != NULL) ? RFAL_NFCV_MAX_GEN_DATA_LEN : (RFAL_NFCV_MAX_GEN_DATA_LEN - RFAL_NFCV_UID_LEN)))) {
    return NFC_InvalidParameter;
  }


  /* Check if the command is an ST's Fast command */
  if ((param == RFAL_NFCV_ST_IC_MFG_CODE) &&
      ((cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_SINGLE_BLOCK)    || (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_EXTENDED_READ_SINGLE_BLOCK)    ||
       (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_MULTIPLE_BLOCKS) || (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_EXTENDED_READ_MULTIPLE_BLOCKS) ||
       (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_WRITE_MESSAGE)        || (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_MESSAGE_LENGTH)           ||
       (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_MESSAGE)         || (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_DYN_CONFIGURATION)        ||
       (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_WRITE_DYN_CONFIGURATION))) {
    /* Store current Rx bit rate and move to fast mode */
    RFal_GetBitRate(NULL, &rxBR);
    RFal_SetBitRate(RFAL_BR_KEEP, RFAL_BR_52p97);

    fastMode = true;
  }


  /* Compute Request Command */
  req.REQ_FLAG  = (uint8_t)(flags & (~((uint32_t)RFAL_NFCV_REQ_FLAG_ADDRESS)));
  req.CMD       = cmd;

  /* Prepend parameter on certain proprietary requests: IC Manuf, Parameters */
  if (param != RFAL_NFCV_PARAM_SKIP) {
    req.payload.data[msgIt++] = param;         /* RFAL_NFCV_PARAM_LEN */
  }

  /* Check if Request is to be sent in Addressed mode. Select mode flag shall be set by user */
  if (uid != NULL) {
    req.REQ_FLAG |= (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS;
    memcpy(&req.payload.data[msgIt], uid, RFAL_NFCV_UID_LEN);
    msgIt += RFAL_NFCV_UID_LEN;
  }

  if (dataLen > 0U) {
    memcpy(&req.payload.data[msgIt], data, dataLen);
    msgIt += (uint8_t)dataLen;
  }

  /* If the Option Flag | Special Frame is set in certain commands an EOF needs to be sent within  FDTV,EOF to retrieve the VICC response     Digital 2.3  9.7.4    ISO15693-3 2009  10.4.2 & 10.4.3 & 10.4.5 */
  if (((flags & (uint8_t)RFAL_NFCV_REQ_FLAG_OPTION) != 0U) && ((cmd == (uint8_t)RFAL_NFCV_CMD_WRITE_SINGLE_BLOCK) || (cmd == (uint8_t)RFAL_NFCV_CMD_WRITE_MULTIPLE_BLOCKS)        ||
                                                               (cmd == (uint8_t)RFAL_NFCV_CMD_LOCK_BLOCK) || (cmd == (uint8_t)RFAL_NFCV_CMD_EXTENDED_WRITE_SINGLE_BLOCK)                   ||
                                                               (cmd == (uint8_t)RFAL_NFCV_CMD_EXTENDED_LOCK_SINGLE_BLOCK) || (cmd == (uint8_t)RFAL_NFCV_CMD_EXTENDED_WRITE_MULTIPLE_BLOCK))) {
    specialFrame = true;
  }

  /* Transceive Command */
  ret = RFal_TransceiveBlockingTxRx((uint8_t *)&req, (RFAL_NFCV_CMD_LEN + RFAL_NFCV_FLAG_LEN + (uint16_t)msgIt), rxBuf, rxBufLen, rcvLen, RFAL_TXRX_FLAGS_DEFAULT, (specialFrame ? RFAL_NFCV_FDT_EOF : RFAL_NFCV_FDT_MAX));

  /* If the Option Flag | Special Frame is set in certain commands an EOF needs to be sent within  FDTV,EOF to retrieve the VICC response     Digital 2.3  9.7.4    ISO15693-3 2009  10.4.2 & 10.4.3 & 10.4.5 */
  if (specialFrame) {
    ret = RFal_ISO15693TransceiveEOF(rxBuf, rxBufLen, rcvLen);
  }

  /* Restore Rx BitRate */
  if (fastMode) {
    RFal_SetBitRate(RFAL_BR_KEEP, rxBR);
  }

  if (ret != NFC_OK) {
    return ret;
  }

  /* Check if the response minimum length has been received */
  if ((*rcvLen) < (uint8_t)RFAL_NFCV_FLAG_LEN) {
    return NFC_ProtocolError;
  }

  /* Check if an error has been signalled */
  if ((rxBuf[RFAL_NFCV_FLAG_POS] & (uint8_t)RFAL_NFCV_RES_FLAG_ERROR) != 0U) {
    return RFal_NFCV_ParseError(rxBuf[RFAL_NFCV_DATASTART_POS]);
  }

  return NFC_OK;
}
