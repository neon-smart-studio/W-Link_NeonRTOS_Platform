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

#include "RFal_NFC.h"
#include "RFal_ST25XV.h"
#include "RFal_NFCV.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#include "NeonRTOS.h"

#define RFAL_ST25xV_READ_CONFIG_LEN      2U     /*!< READ CONFIGURATION length                                         */
#define RFAL_ST25xV_READ_MSG_LEN_LEN     2U     /*!< READ MESSAGE LENGTH length                                        */
#define RFAL_ST25xV_CONF_POINTER_LEN     1U     /*!< READ/WRITE CONFIGURATION Pointer length                           */
#define RFAL_ST25xV_CONF_REGISTER_LEN    1U     /*!< READ/WRITE CONFIGURATION Register length                          */
#define RFAL_ST25xV_PWDNUM_LEN           1U     /*!< Password Number length                                            */
#define RFAL_ST25xV_PWD_LEN              8U     /*!< Password length                                                   */
#define RFAL_ST25xV_MBPOINTER_LEN        1U     /*!< Read Message MBPointer length                                     */
#define RFAL_ST25xV_NUMBYTES_LEN         1U     /*!< Read Message Number of Bytes length                               */

#define RFAL_ST25TV02K_TBOOT_RF          1U     /*!< RF Boot time (Minimum time from carrier generation to first data) */
#define RFAL_ST25TV02K_TRF_OFF           2U     /*!< RF OFF time                                                       */

#define RFAL_ST25xV_FDT_POLL_MAX                 rfalConvMsTo1fc(20) /*!< Maximum Wait time FDTV,EOF 20 ms    Digital 2.1  B.5 */
#define RFAL_NFCV_FLAG_POS                0U     /*!< Flag byte position                                                */
#define RFAL_NFCV_FLAG_LEN                1U     /*!< Flag byte length                                                  */

/*******************************************************************************/
static NFC_OpResult RFal_ST25XV_PollerGenericReadConfiguration(uint8_t cmd, uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t *regValue)
{
  NFC_OpResult         ret;
  uint8_t            p;
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  if (regValue == NULL) {
    return NFC_InvalidParameter;
  }

  p = pointer;

  ret = RFal_NFCV_PollerTransceiveReq(cmd, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, &p, sizeof(uint8_t), (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);
  if (ret == NFC_OK) {
    if (rcvLen < RFAL_ST25xV_READ_CONFIG_LEN) {
      ret = NFC_ProtocolError;
    } else {
      *regValue = res.data[0];
    }
  }
  return ret;
}

/*******************************************************************************/
static NFC_OpResult RFal_ST25XV_PollerGenericWriteConfiguration(uint8_t cmd, uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t regValue)
{
  uint8_t            data[RFAL_ST25xV_CONF_POINTER_LEN + RFAL_ST25xV_CONF_REGISTER_LEN];
  uint8_t            dataLen;
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  dataLen = 0U;

  data[dataLen++] = pointer;
  data[dataLen++] = regValue;

  return RFal_NFCV_PollerTransceiveReq(cmd, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, data, dataLen, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);

}

/*******************************************************************************/
static NFC_OpResult RFal_ST25XV_PollerGenericReadMessageLength(uint8_t cmd, uint8_t flags, const uint8_t *uid, uint8_t *msgLen)
{
  NFC_OpResult         ret;
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  if (msgLen == NULL) {
    return NFC_InvalidParameter;
  }

  ret = RFal_NFCV_PollerTransceiveReq(cmd, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, NULL, 0, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);
  if (ret == NFC_OK) {
    if (rcvLen < RFAL_ST25xV_READ_MSG_LEN_LEN) {
      ret = NFC_ProtocolError;
    } else {
      *msgLen = res.data[0];
    }
  }
  return ret;
}

/*******************************************************************************/
static NFC_OpResult RFal_ST25XV_PollerGenericReadMessage(uint8_t cmd, uint8_t flags, const uint8_t *uid, uint8_t mbPointer, uint8_t numBytes, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[RFAL_ST25xV_MBPOINTER_LEN + RFAL_ST25xV_NUMBYTES_LEN];
  uint8_t dataLen;

  dataLen = 0;

  /* Compute Request Data */
  data[dataLen++] = mbPointer;
  data[dataLen++] = numBytes;

  return RFal_NFCV_PollerTransceiveReq(cmd, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
static NFC_OpResult RFal_ST25XV_PollerGenericWriteMessage(uint8_t cmd, uint8_t flags, const uint8_t *uid, uint8_t msgLen, const uint8_t *msgData, uint8_t *txBuf, uint16_t txBufLen)
{
  NFC_OpResult         ret;
  uint8_t            reqFlag;
  uint16_t           msgIt;
  RFal_BitRate        rxBR;
  bool               fastMode;
  RFal_NFCV_GenericRes res;
  uint16_t           rcvLen;

  /* Calculate required Tx buf length:                    Mfg Code               UID                      MSGLen  MSGLen+1 */
  msgIt = (uint16_t)(msgLen + sizeof(flags) + sizeof(cmd) + 1U  + ((uid != NULL) ? RFAL_NFCV_UID_LEN : 0U) + 1U + 1U);
  /* Note:  MSGlength parameter of the command is the number of Data bytes minus - 1 (00 for 1 byte of data, FFh for 256 bytes of data) */

  /* Check for valid parameters */
  if ((txBuf == NULL) || (msgData == NULL) || (txBufLen < msgIt)) {
    return NFC_InvalidParameter;
  }

  msgIt    = 0;
  fastMode = false;

  /* Check if the command is an ST's Fast command */
  if (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_WRITE_MESSAGE) {
    /* Store current Rx bit rate and move to fast mode */
    RFal_GetBitRate(NULL, &rxBR);
    RFal_SetBitRate(RFAL_BR_KEEP, RFAL_BR_52p97);

    fastMode = true;
  }

  /* Compute Request Command */
  reqFlag = (uint8_t)(flags & (~((uint32_t)RFAL_NFCV_REQ_FLAG_ADDRESS) & ~((uint32_t)RFAL_NFCV_REQ_FLAG_SELECT)));
  reqFlag |= ((uid != NULL) ? (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS : (uint8_t)RFAL_NFCV_REQ_FLAG_SELECT);

  txBuf[msgIt++] = reqFlag;
  txBuf[msgIt++] = cmd;
  txBuf[msgIt++] = RFAL_NFCV_ST_IC_MFG_CODE;

  if (uid != NULL) {
    ST_MEMCPY(&txBuf[msgIt], uid, RFAL_NFCV_UID_LEN);
    msgIt += RFAL_NFCV_UID_LEN;
  }
  txBuf[msgIt++] = msgLen;
  ST_MEMCPY(&txBuf[msgIt], msgData, (uint16_t)(msgLen + (uint16_t)1U));   /* Message Data contains (MSGLength + 1) bytes */
  msgIt += (uint16_t)(msgLen + (uint16_t)1U);

  /* Transceive Command */
  ret = RFal_TransceiveBlockingTxRx(txBuf, msgIt, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25xV_FDT_POLL_MAX);


  /* Restore Rx BitRate */
  if (fastMode) {
    RFal_SetBitRate(RFAL_BR_KEEP, rxBR);
  }

  if (ret != NFC_OK) {
    return ret;
  }

  /* Check if the response minimum length has been received */
  if (rcvLen < (uint8_t)RFAL_NFCV_FLAG_LEN) {
    return NFC_ProtocolError;
  }

  /* Check if an error has been signalled */
  if ((res.RES_FLAG & (uint8_t)RFAL_NFCV_RES_FLAG_ERROR) != 0U) {
    return NFC_ProtocolError;
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerM24LRReadSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[RFAL_NFCV_BLOCKNUM_M24LR_LEN];
  uint8_t dataLen;

  dataLen = 0;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)blockNum;         /* Set M24LR Block Number (16 bits) LSB */
  data[dataLen++] = (uint8_t)(blockNum >> 8U); /* Set M24LR Block Number (16 bits) MSB */

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_READ_SINGLE_BLOCK, (flags | (uint8_t)RFAL_NFCV_REQ_FLAG_PROTOCOL_EXT), RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerM24LRWriteSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, const uint8_t *wrData, uint8_t blockLen)
{
  uint8_t            data[(RFAL_NFCV_BLOCKNUM_M24LR_LEN + RFAL_NFCV_MAX_BLOCK_LEN)];
  uint8_t            dataLen;
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  /* Check for valid parameters */
  if ((blockLen == 0U) || (blockLen > (uint8_t)RFAL_NFCV_MAX_BLOCK_LEN) || (wrData == NULL)) {
    return NFC_InvalidParameter;
  }

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)blockNum;         /* Set M24LR Block Number (16 bits) LSB */
  data[dataLen++] = (uint8_t)(blockNum >> 8U); /* Set M24LR Block Number (16 bits) MSB */
  ST_MEMCPY(&data[dataLen], wrData, blockLen);   /* Append Block data to write       */
  dataLen += blockLen;

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_WRITE_SINGLE_BLOCK, (flags | (uint8_t)RFAL_NFCV_REQ_FLAG_PROTOCOL_EXT), RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerM24LRReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[(RFAL_NFCV_BLOCKNUM_M24LR_LEN + RFAL_NFCV_BLOCKNUM_M24LR_LEN)];
  uint8_t dataLen;

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)firstBlockNum;         /* Set M24LR Block Number (16 bits) LSB */
  data[dataLen++] = (uint8_t)(firstBlockNum >> 8U); /* Set M24LR Block Number (16 bits) MSB */
  data[dataLen++] = numOfBlocks;                    /* Set number of blocks to read         */

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_READ_MULTIPLE_BLOCKS, (flags | (uint8_t)RFAL_NFCV_REQ_FLAG_PROTOCOL_EXT), RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerFastReadSingleBlock(uint8_t flags, const uint8_t *uid, uint8_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t bn;

  bn = blockNum;

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_FAST_READ_SINGLE_BLOCK, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, &bn, sizeof(uint8_t), rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerM24LRFastReadSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[RFAL_NFCV_BLOCKNUM_M24LR_LEN];
  uint8_t dataLen;

  dataLen = 0;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)blockNum;         /* Set M24LR Block Number (16 bits) LSB */
  data[dataLen++] = (uint8_t)(blockNum >> 8U); /* Set M24LR Block Number (16 bits) MSB */

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_FAST_READ_SINGLE_BLOCK, (flags | (uint8_t)RFAL_NFCV_REQ_FLAG_PROTOCOL_EXT), RFAL_NFCV_ST_IC_MFG_CODE, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerM24LRFastReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[(RFAL_NFCV_BLOCKNUM_M24LR_LEN + RFAL_NFCV_BLOCKNUM_M24LR_LEN)];
  uint8_t dataLen;

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)firstBlockNum;         /* Set M24LR Block Number (16 bits) LSB */
  data[dataLen++] = (uint8_t)(firstBlockNum >> 8U); /* Set M24LR Block Number (16 bits) MSB */
  data[dataLen++] = numOfBlocks;                    /* Set number of blocks to read         */

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_FAST_READ_MULTIPLE_BLOCKS, (flags | (uint8_t)RFAL_NFCV_REQ_FLAG_PROTOCOL_EXT), RFAL_NFCV_ST_IC_MFG_CODE, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerFastReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[(RFAL_NFCV_BLOCKNUM_LEN + RFAL_NFCV_BLOCKNUM_LEN)];
  uint8_t dataLen;

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = firstBlockNum;                    /* Set first Block Number       */
  data[dataLen++] = numOfBlocks;                      /* Set number of blocks to read */

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_FAST_READ_MULTIPLE_BLOCKS, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerFastExtendedReadSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[RFAL_NFCV_BLOCKNUM_EXTENDED_LEN];
  uint8_t dataLen;

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)blockNum; /* TS T5T 1.0 BNo is considered as a multi-byte field. TS T5T 1.0 5.1.1.13 multi-byte field follows [DIGITAL]. [DIGITAL] 9.3.1 A multiple byte field is transmitted LSB first. */
  data[dataLen++] = (uint8_t)((blockNum >> 8U) & 0xFFU);

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_FAST_EXTENDED_READ_SINGLE_BLOCK, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerFastExtReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  uint8_t data[(RFAL_NFCV_BLOCKNUM_EXTENDED_LEN + RFAL_NFCV_BLOCKNUM_EXTENDED_LEN)];
  uint8_t dataLen;

  dataLen = 0U;

  /* Compute Request Data */
  data[dataLen++] = (uint8_t)((firstBlockNum >> 0U) & 0xFFU);
  data[dataLen++] = (uint8_t)((firstBlockNum >> 8U) & 0xFFU);
  data[dataLen++] = (uint8_t)((numOfBlocks >> 0U) & 0xFFU);
  data[dataLen++] = (uint8_t)((numOfBlocks >> 8U) & 0xFFU);

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_FAST_EXTENDED_READ_MULTIPLE_BLOCKS, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, data, dataLen, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerReadConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t *regValue)
{
  return  rfalST25xVPollerGenericReadConfiguration(RFAL_NFCV_CMD_READ_CONFIGURATION, flags,  uid, pointer, regValue);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerWriteConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t regValue)
{
  return rfalST25xVPollerGenericWriteConfiguration(RFAL_NFCV_CMD_WRITE_CONFIGURATION, flags, uid, pointer, regValue);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerReadDynamicConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t *regValue)
{
  return  rfalST25xVPollerGenericReadConfiguration(RFAL_NFCV_CMD_READ_DYN_CONFIGURATION, flags,  uid, pointer, regValue);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerWriteDynamicConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t regValue)
{
  return rfalST25xVPollerGenericWriteConfiguration(RFAL_NFCV_CMD_WRITE_DYN_CONFIGURATION, flags, uid, pointer, regValue);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerFastReadDynamicConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t *regValue)
{
  return  rfalST25xVPollerGenericReadConfiguration(RFAL_NFCV_CMD_FAST_READ_DYN_CONFIGURATION, flags,  uid, pointer, regValue);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerFastWriteDynamicConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t regValue)
{
  return rfalST25xVPollerGenericWriteConfiguration(RFAL_NFCV_CMD_FAST_WRITE_DYN_CONFIGURATION, flags, uid, pointer, regValue);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerPresentPassword(uint8_t flags, const uint8_t *uid, uint8_t pwdNum, const uint8_t *pwd,  uint8_t pwdLen)
{
  uint8_t            data[RFAL_ST25xV_PWDNUM_LEN + RFAL_ST25xV_PWD_LEN];
  uint8_t            dataLen;
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  if ((pwdLen > RFAL_ST25xV_PWD_LEN) || (pwd == NULL)) {
    return NFC_InvalidParameter;
  }

  dataLen = 0U;
  data[dataLen++] = pwdNum;;
  if (pwdLen > 0U) {
    ST_MEMCPY(&data[dataLen], pwd, pwdLen);
  }
  dataLen += pwdLen;

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_PRESENT_PASSWORD, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, data, dataLen, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);

}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerWritePassword(uint8_t flags, const uint8_t *uid, uint8_t pwdNum, const uint8_t *pwd,  uint8_t pwdLen)
{
  uint8_t            data[RFAL_ST25xV_PWDNUM_LEN + RFAL_ST25xV_PWD_LEN];
  uint8_t            dataLen;
  uint16_t           rcvLen;
  RFal_NFCV_GenericRes res;

  if ((pwdLen > RFAL_ST25xV_PWD_LEN) || (pwd == NULL)) {
    return NFC_InvalidParameter;
  }

  dataLen = 0U;
  data[dataLen++] = pwdNum;
  if (pwdLen > 0U) {
    ST_MEMCPY(&data[dataLen], pwd, pwdLen);
  }
  dataLen += pwdLen;

  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_WRITE_PASSWORD, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, data, dataLen, (uint8_t *)&res, sizeof(RFal_NFCV_GenericRes), &rcvLen);

}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerGetRandomNumber(uint8_t flags, const uint8_t *uid, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  RFal_FieldOff();
  NeonRTOS_Sleep(RFAL_ST25TV02K_TRF_OFF);
  RFal_NFCV_PollerInitialize();
  RFal_FieldOnAndStartGT();
  NeonRTOS_Sleep(RFAL_ST25TV02K_TBOOT_RF);
  return RFal_NFCV_PollerTransceiveReq(RFAL_NFCV_CMD_GET_RANDOM_NUMBER, flags, RFAL_NFCV_ST_IC_MFG_CODE, uid, NULL, 0U, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerWriteMessage(uint8_t flags, const uint8_t *uid, uint8_t msgLen, const uint8_t *msgData, uint8_t *txBuf, uint16_t txBufLen)
{
  return rfalST25xVPollerGenericWriteMessage(RFAL_NFCV_CMD_WRITE_MESSAGE, flags,  uid, msgLen, msgData,  txBuf, txBufLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerFastWriteMessage(uint8_t flags, const uint8_t *uid, uint8_t msgLen, const uint8_t *msgData, uint8_t *txBuf, uint16_t txBufLen)
{
  return rfalST25xVPollerGenericWriteMessage(RFAL_NFCV_CMD_FAST_WRITE_MESSAGE, flags,  uid, msgLen, msgData,  txBuf, txBufLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerReadMessageLength(uint8_t flags, const uint8_t *uid, uint8_t *msgLen)
{
  return rfalST25xVPollerGenericReadMessageLength(RFAL_NFCV_CMD_READ_MESSAGE_LENGTH, flags, uid, msgLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerFastReadMsgLength(uint8_t flags, const uint8_t *uid, uint8_t *msgLen)
{
  return rfalST25xVPollerGenericReadMessageLength(RFAL_NFCV_CMD_FAST_READ_MESSAGE_LENGTH, flags, uid, msgLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerReadMessage(uint8_t flags, const uint8_t *uid, uint8_t mbPointer, uint8_t numBytes, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  return rfalST25xVPollerGenericReadMessage(RFAL_NFCV_CMD_READ_MESSAGE, flags, uid, mbPointer, numBytes, rxBuf, rxBufLen, rcvLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ST25XV_PollerFastReadMessage(uint8_t flags, const uint8_t *uid, uint8_t mbPointer, uint8_t numBytes, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  return rfalST25xVPollerGenericReadMessage(RFAL_NFCV_CMD_FAST_READ_MESSAGE, flags, uid, mbPointer, numBytes, rxBuf, rxBufLen, rcvLen);
}
