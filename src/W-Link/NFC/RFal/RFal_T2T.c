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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "RFal_RF.h"
#include "RFal_NFC.h"
#include "RFal_T2T.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#define RFAL_FDT_POLL_READ_MAX                 rfalConvMsTo1fc(5U)  /*!< Maximum Wait time for Read command as defined in TS T2T 1.0 table 18   */
#define RFAL_FDT_POLL_WRITE_MAX                rfalConvMsTo1fc(10U) /*!< Maximum Wait time for Write command as defined in TS T2T 1.0 table 18  */
#define RFAL_FDT_POLL_SL_MAX                   rfalConvMsTo1fc(1U)  /*!< Maximum Wait time for Sector Select as defined in TS T2T 1.0 table 18  */
#define RFAL_T2T_ACK_NACK_LEN                  1U                   /*!< Len of NACK in bytes (4 bits)                                          */
#define RFAL_T2T_ACK                           0x0AU                /*!< ACK value                                                              */
#define RFAL_T2T_ACK_MASK                      0x0FU                /*!< ACK value                                                              */


#define RFAL_T2T_SECTOR_SELECT_P1_BYTE2        0xFFU                /*!< Sector Select Packet 1 byte 2                                          */
#define RFAL_T2T_SECTOR_SELECT_P2_RFU_LEN      3U                   /*!< Sector Select RFU length                                               */

/*! NFC-A T2T command set    T2T 1.0 5.1 */
typedef enum {
  RFAL_T2T_CMD_READ           = 0x30,     /*!< T2T Read                                */
  RFAL_T2T_CMD_WRITE          = 0xA2,     /*!< T2T Write                               */
  RFAL_T2T_CMD_SECTOR_SELECT  = 0xC2      /*!< T2T Sector Select                       */
} RFal_T2T_cmds;


/*! NFC-A T2T READ     T2T 1.0 5.2 and table 11 */
typedef struct {
  uint8_t code;                           /*!< Command code                            */
  uint8_t blNo;                           /*!< Block number                            */
} RFal_T2T_ReadReq;


/*! NFC-A T2T WRITE    T2T 1.0 5.3 and table 12 */
typedef struct {
  uint8_t code;                           /*!< Command code                            */
  uint8_t blNo;                           /*!< Block number                            */
  uint8_t data[RFAL_T2T_WRITE_DATA_LEN];  /*!< Data                                    */
} RFal_T2T_WriteReq;


/*! NFC-A T2T SECTOR SELECT Packet 1   T2T 1.0 5.4 and table 13 */
typedef struct {
  uint8_t code;                           /*!< Command code                            */
  uint8_t byte2;                          /*!< Sector Select Packet 1 byte 2           */
} RFal_T2T_SectorSelectP1Req;


/*! NFC-A T2T SECTOR SELECT Packet 2   T2T 1.0 5.4 and table 13 */
typedef struct {
  uint8_t secNo;                                   /*!< Block number                   */
  uint8_t rfu[RFAL_T2T_SECTOR_SELECT_P2_RFU_LEN];  /*!< Sector Select Packet RFU       */
} RFal_T2T_SectorSelectP2Req;


NFC_OpResult RFal_T2T_PollerRead(uint8_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  NFC_OpResult      ret;
  RFal_T2T_ReadReq  req;

  if ((rxBuf == NULL) || (rcvLen == NULL)) {
    return NFC_InvalidParameter;
  }

  req.code = (uint8_t)RFAL_T2T_CMD_READ;
  req.blNo = blockNum;

  /* Transceive Command */
  ret = RFal_TransceiveBlockingTxRx((uint8_t *)&req, sizeof(RFal_T2T_ReadReq), rxBuf, rxBufLen, rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FDT_POLL_READ_MAX);

  /* T2T 1.0 5.2.1.7 The Reader/Writer SHALL treat a NACK in response to a READ Command as a Protocol Error */
  if ((ret == NFC_ImcompleteByte) && (*rcvLen == RFAL_T2T_ACK_NACK_LEN) && ((*rxBuf & RFAL_T2T_ACK_MASK) != RFAL_T2T_ACK)) {
    return NFC_ProtocolError;
  }
  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_T2T_PollerWrite(uint8_t blockNum, const uint8_t *wrData)
{
  NFC_OpResult         ret;
  RFal_T2T_WriteReq    req;
  uint8_t            res;
  uint16_t           rxLen;

  req.code = (uint8_t)RFAL_T2T_CMD_WRITE;
  req.blNo = blockNum;
  memcpy(req.data, wrData, RFAL_T2T_WRITE_DATA_LEN);


  /* Transceive WRITE Command */
  ret = RFal_TransceiveBlockingTxRx((uint8_t *)&req, sizeof(RFal_T2T_WriteReq), &res, sizeof(uint8_t), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FDT_POLL_WRITE_MAX);

  /* Check for a valid ACK */
  if ((ret == NFC_ImcompleteByte) || (ret == NFC_OK)) {
    ret = NFC_ProtocolError;

    if ((rxLen == RFAL_T2T_ACK_NACK_LEN) && ((res & RFAL_T2T_ACK_MASK) == RFAL_T2T_ACK)) {
      ret = NFC_OK;
    }
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_T2T_PollerSectorSelect(uint8_t sectorNum)
{
  RFal_T2T_SectorSelectP1Req p1Req;
  RFal_T2T_SectorSelectP2Req p2Req;
  NFC_OpResult               ret;
  uint8_t                  res;
  uint16_t                 rxLen;


  /* Compute SECTOR SELECT Packet 1  */
  p1Req.code  = (uint8_t)RFAL_T2T_CMD_SECTOR_SELECT;
  p1Req.byte2 = RFAL_T2T_SECTOR_SELECT_P1_BYTE2;

  /* Transceive SECTOR SELECT Packet 1 */
  ret = RFal_TransceiveBlockingTxRx((uint8_t *)&p1Req, sizeof(RFal_T2T_SectorSelectP1Req), &res, sizeof(uint8_t), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FDT_POLL_SL_MAX);

  /* Check and report any transmission error */
  if ((ret != NFC_ImcompleteByte) && (ret != NFC_OK)) {
    return ret;
  }

  /* Ensure that an ACK was received */
  if ((ret != NFC_ImcompleteByte) || (rxLen != RFAL_T2T_ACK_NACK_LEN) || ((res & RFAL_T2T_ACK_MASK) != RFAL_T2T_ACK)) {
    return NFC_ProtocolError;
  }


  /* Compute SECTOR SELECT Packet 2  */
  p2Req.secNo  = sectorNum;
  memset(&p2Req.rfu, 0x00, RFAL_T2T_SECTOR_SELECT_P2_RFU_LEN);


  /* Transceive SECTOR SELECT Packet 2 */
  ret = RFal_TransceiveBlockingTxRx((uint8_t *)&p2Req, sizeof(RFal_T2T_SectorSelectP2Req), &res, sizeof(uint8_t), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FDT_POLL_SL_MAX);

  /* T2T 1.0 5.4.1.14 The Reader/Writer SHALL treat any response received before the end of PATT2T,SL,MAX as a Protocol Error */
  if ((ret == NFC_OK) || (ret == NFC_ImcompleteByte)) {
    return NFC_ProtocolError;
  }

  /* T2T 1.0 5.4.1.13 The Reader/Writer SHALL treat the transmission of the SECTOR SELECT Command Packet 2 as being successful when it receives no response until PATT2T,SL,MAX. */
  if (ret == NFC_SlaveTimeout) {
    return NFC_OK;
  }

  return ret;
}
