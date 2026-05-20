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

#ifndef RFAL_ST25xV_H
#define RFAL_ST25xV_H

#include "RFal_NFC.h"
#include "RFal_RF.h"

#include "NFC/NFC_Def.h"

#define RFAL_NFCV_BLOCKNUM_M24LR_LEN       2U      /*!< Block Number length of MR24LR tags: 16 bits                */

#ifdef __cplusplus
extern "C" {
#endif

NFC_OpResult RFal_ST25XV_PollerM24LRReadSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerM24LRFastReadSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerM24LRWriteSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, const uint8_t *wrData, uint8_t blockLen);
NFC_OpResult RFal_ST25XV_PollerM24LRReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerM24LRFastReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerFastReadSingleBlock(uint8_t flags, const uint8_t *uid, uint8_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerFastReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerFastExtendedReadSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerFastExtReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerReadConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t *regValue);
NFC_OpResult RFal_ST25XV_PollerWriteConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t regValue);
NFC_OpResult RFal_ST25XV_PollerReadDynamicConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t *regValue);
NFC_OpResult RFal_ST25XV_PollerWriteDynamicConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t regValue);
NFC_OpResult RFal_ST25XV_PollerFastReadDynamicConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t *regValue);
NFC_OpResult RFal_ST25XV_PollerFastWriteDynamicConfiguration(uint8_t flags, const uint8_t *uid, uint8_t pointer, uint8_t regValue);
NFC_OpResult RFal_ST25XV_PollerPresentPassword(uint8_t flags, const uint8_t *uid, uint8_t pwdNum, const uint8_t *pwd, uint8_t pwdLen);
NFC_OpResult RFal_ST25XV_PollerGetRandomNumber(uint8_t flags, const uint8_t *uid, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerReadMessageLength(uint8_t flags, const uint8_t *uid, uint8_t *msgLen);
NFC_OpResult RFal_ST25XV_PollerFastReadMsgLength(uint8_t flags, const uint8_t *uid, uint8_t *msgLen);
NFC_OpResult RFal_ST25XV_PollerReadMessage(uint8_t flags, const uint8_t *uid, uint8_t mbPointer, uint8_t numBytes, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerFastReadMessage(uint8_t flags, const uint8_t *uid, uint8_t mbPointer, uint8_t numBytes, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_ST25XV_PollerWriteMessage(uint8_t flags, const uint8_t *uid, uint8_t msgLen, const uint8_t *msgData, uint8_t *txBuf, uint16_t txBufLen);
NFC_OpResult RFal_ST25XV_PollerFastWriteMessage(uint8_t flags, const uint8_t *uid, uint8_t msgLen, const uint8_t *msgData, uint8_t *txBuf, uint16_t txBufLen);

NFC_OpResult RFal_ST25XV_PollerWritePassword(uint8_t flags, const uint8_t *uid, uint8_t pwdNum, const uint8_t *pwd,  uint8_t pwdLen);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_ST25xV_H */
