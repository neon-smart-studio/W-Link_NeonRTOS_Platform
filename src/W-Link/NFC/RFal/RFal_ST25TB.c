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

#include "RFal_NFC.h"
#include "RFal_ST25TB.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#include "NeonRTOS.h"

#define RFAL_ST25TB_CMD_LEN          1U                                 /*!< ST25TB length of a command                       */
#define RFAL_ST25TB_SLOTS            16U                                /*!< ST25TB number of slots                           */
#define RFAL_ST25TB_SLOTNUM_MASK     0x0FU                              /*!< ST25TB Slot Number bit mask on SlotMarker        */
#define RFAL_ST25TB_SLOTNUM_SHIFT    4U                                 /*!< ST25TB Slot Number shift on SlotMarker           */

#define RFAL_ST25TB_INITIATE_CMD1    0x06U                              /*!< ST25TB Initiate command byte1                    */
#define RFAL_ST25TB_INITIATE_CMD2    0x00U                              /*!< ST25TB Initiate command byte2                    */
#define RFAL_ST25TB_PCALL_CMD1       0x06U                              /*!< ST25TB Pcall16 command byte1                     */
#define RFAL_ST25TB_PCALL_CMD2       0x04U                              /*!< ST25TB Pcall16 command byte2                     */
#define RFAL_ST25TB_SELECT_CMD       0x0EU                              /*!< ST25TB Select command                            */
#define RFAL_ST25TB_GET_UID_CMD      0x0BU                              /*!< ST25TB Get UID command                           */
#define RFAL_ST25TB_COMPLETION_CMD   0x0FU                              /*!< ST25TB Completion command                        */
#define RFAL_ST25TB_RESET_INV_CMD    0x0CU                              /*!< ST25TB Reset to Inventory command                */
#define RFAL_ST25TB_READ_BLOCK_CMD   0x08U                              /*!< ST25TB Read Block command                        */
#define RFAL_ST25TB_WRITE_BLOCK_CMD  0x09U                              /*!< ST25TB Write Block command                       */


#define RFAL_ST25TB_T0               2157U                              /*!< ST25TB t0  159 us   ST25TB RF characteristics    */
#define RFAL_ST25TB_T1               2048U                              /*!< ST25TB t1  151 us   ST25TB RF characteristics    */

#define RFAL_ST25TB_FWT             (RFAL_ST25TB_T0 + RFAL_ST25TB_T1)   /*!< ST25TB FWT  = T0 + T1                            */
#define RFAL_ST25TB_TW              RFal_ConvMsTo1fc(7U)                 /*!< ST25TB TW : Programming time for write max 7ms   */


/*! Initiate Request */
typedef struct {
  uint8_t  cmd1;                       /*!< Initiate Request cmd1: 0x06 */
  uint8_t  cmd2;                       /*!< Initiate Request cmd2: 0x00 */
} RFal_ST25TB_InitiateReq;

/*! Pcall16 Request */
typedef struct {
  uint8_t  cmd1;                       /*!< Pcal16 Request cmd1: 0x06   */
  uint8_t  cmd2;                       /*!< Pcal16 Request cmd2: 0x04   */
} RFal_ST25TB_PcallReq;


/*! Select Request */
typedef struct {
  uint8_t  cmd;                       /*!< Select Request cmd: 0x0E     */
  uint8_t  chipId;                    /*!< Chip ID                      */
} RFal_ST25TB_SelectReq;

/*! Read Block Request */
typedef struct {
  uint8_t  cmd;                       /*!< Select Request cmd: 0x08     */
  uint8_t  address;                   /*!< Block address                */
} RFal_ST25TB_ReadBlockReq;

/*! Write Block Request */
typedef struct {
  uint8_t              cmd;           /*!< Select Request cmd: 0x09     */
  uint8_t              address;       /*!< Block address                */
  RFal_ST25TB_Block data;               /*!< Block Data                   */
} RFal_ST25TB_WriteBlockReq;

static bool RFal_ST25TB_PollerDoCollisionResolution(uint8_t devLimit, RFal_ST25TB_ListenDevice *st25tbDevList, uint8_t *devCnt)
{
  uint8_t    i;
  uint8_t    chipId;
  NFC_OpResult ret;
  bool col;

  col = false;

  for (i = 0; i < RFAL_ST25TB_SLOTS; i++) {
    NeonRTOS_Sleep(1);  /* Wait t2: Answer to new request NeonRTOS_Sleep  */

    if (i == 0U) {
      /* Step 2: Send Pcall16 */
      ret = RFal_ST25TB_PollerPcall(&chipId);
    } else {
      /* Step 3-17: Send Pcall16 */
      ret = RFal_ST25TB_PollerSlotMarker(i, &chipId);
    }

    if (ret == NFC_OK) {
      /* Found another device */
      st25tbDevList[*devCnt].chipID       = chipId;
      st25tbDevList[*devCnt].isDeselected = false;

      /* Select Device, retrieve its UID  */
      ret = RFal_ST25TB_PollerSelect(chipId);

      /* By Selecting this device, the previous gets Deselected */
      if ((*devCnt) > 0U) {
        st25tbDevList[(*devCnt) - 1U].isDeselected = true;
      }

      if (NFC_OK == ret) {
        ret = RFal_ST25TB_PollerGetUID(&st25tbDevList[*devCnt].UID);
      }

      if (NFC_OK == ret) {
        (*devCnt)++;
      }
    } else if ((ret == NFC_CRC_Error) || (ret == NFC_FramingError)) {
      col = true;
    } else {
      /* MISRA 15.7 - Empty else */
    }

    if (*devCnt >= devLimit) {
      break;
    }
  }
  return col;
}

/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerInit(void)
{
  return RFal_NFCB_PollerInit();
}

/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerCheckPresence(uint8_t *chipId)
{
  NFC_OpResult ret;
  uint8_t    chipIdRes;

  chipIdRes = 0x00;

  /* Send Initiate Request */
  ret = RFal_ST25TB_PollerInitiate(&chipIdRes);

  /*  Check if a transmission error was detected */
  if ((ret == NFC_CRC_Error) || (ret == NFC_FramingError)) {
    return NFC_OK;
  }

  /* Copy chip ID if requested */
  if (chipId != NULL) {
    *chipId = chipIdRes;
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerInitiate(uint8_t *chipId)
{
  NFC_OpResult            ret;
  uint16_t              rxLen;
  RFal_ST25TB_InitiateReq initiateReq;
  uint8_t               rxBuf[RFAL_ST25TB_CHIP_ID_LEN + RFAL_ST25TB_CRC_LEN]; /* In case we receive less data that CRC, RF layer will not remove the CRC from buffer */

  /* Compute Initiate Request */
  initiateReq.cmd1   = RFAL_ST25TB_INITIATE_CMD1;
  initiateReq.cmd2   = RFAL_ST25TB_INITIATE_CMD2;

  /* Send Initiate Request */
  ret = RFal_TransceiveBlockingTx((uint8_t *)&initiateReq, sizeof(RFal_ST25TB_InitiateReq), (uint8_t *)rxBuf, sizeof(rxBuf), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT);

  /* Check for valid Select Response   */
  if ((ret == NFC_OK) && (rxLen != RFAL_ST25TB_CHIP_ID_LEN)) {
    return NFC_ProtocolError;
  }

  /* Copy chip ID if requested */
  if (chipId != NULL) {
    *chipId = *rxBuf;
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerPcall(uint8_t *chipId)
{
  NFC_OpResult         ret;
  uint16_t           rxLen;
  RFal_ST25TB_PcallReq pcallReq;

  /* Compute Pcal16 Request */
  pcallReq.cmd1   = RFAL_ST25TB_PCALL_CMD1;
  pcallReq.cmd2   = RFAL_ST25TB_PCALL_CMD2;

  /* Send Pcal16 Request */
  ret = RFal_TransceiveBlockingTx((uint8_t *)&pcallReq, sizeof(RFal_ST25TB_PcallReq), (uint8_t *)chipId, RFAL_ST25TB_CHIP_ID_LEN, &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT);

  /* Check for valid Select Response   */
  if ((ret == NFC_OK) && (rxLen != RFAL_ST25TB_CHIP_ID_LEN)) {
    return NFC_ProtocolError;
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerSlotMarker(uint8_t slotNum, uint8_t *chipIdRes)
{
  NFC_OpResult ret;
  uint16_t   rxLen;
  uint8_t    slotMarker;

  if ((slotNum == 0U) || (slotNum > 15U)) {
    return NFC_InvalidParameter;
  }

  /* Compute SlotMarker */
  slotMarker = (((slotNum & RFAL_ST25TB_SLOTNUM_MASK) << RFAL_ST25TB_SLOTNUM_SHIFT) | RFAL_ST25TB_PCALL_CMD1);


  /* Send SlotMarker */
  ret = RFal_TransceiveBlockingTx((uint8_t *)&slotMarker, RFAL_ST25TB_CMD_LEN, (uint8_t *)chipIdRes, RFAL_ST25TB_CHIP_ID_LEN, &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT);

  /* Check for valid ChipID Response   */
  if ((ret == NFC_OK) && (rxLen != RFAL_ST25TB_CHIP_ID_LEN)) {
    return NFC_ProtocolError;
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerSelect(uint8_t chipId)
{
  NFC_OpResult          ret;
  uint16_t            rxLen;
  RFal_ST25TB_SelectReq selectReq;
  uint8_t             chipIdRes;

  /* Compute Select Request */
  selectReq.cmd    = RFAL_ST25TB_SELECT_CMD;
  selectReq.chipId = chipId;

  /* Send Select Request */
  ret = RFal_TransceiveBlockingTx((uint8_t *)&selectReq, sizeof(RFal_ST25TB_SelectReq), (uint8_t *)&chipIdRes, RFAL_ST25TB_CHIP_ID_LEN, &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT);

  /* Check for valid Select Response   */
  if ((ret == NFC_OK) && ((rxLen != RFAL_ST25TB_CHIP_ID_LEN) || (chipIdRes != chipId))) {
    return NFC_ProtocolError;
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerGetUID(RFal_ST25TB_UID *UID)
{
  NFC_OpResult ret;
  uint16_t   rxLen;
  uint8_t    getUidReq;


  /* Compute Get UID Request */
  getUidReq = RFAL_ST25TB_GET_UID_CMD;

  /* Send Select Request */
  ret = RFal_TransceiveBlockingTx((uint8_t *)&getUidReq, RFAL_ST25TB_CMD_LEN, (uint8_t *)UID, sizeof(RFal_ST25TB_UID), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT);

  /* Check for valid UID Response */
  if ((ret == NFC_OK) && (rxLen != RFAL_ST25TB_UID_LEN)) {
    return NFC_ProtocolError;
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerCollisionResolution(uint8_t devLimit, RFal_ST25TB_ListenDevice *st25tbDevList, uint8_t *devCnt)
{

  uint8_t    chipId;
  NFC_OpResult ret;
  bool       detected;  /* collision or device was detected */

  if ((st25tbDevList == NULL) || (devCnt == NULL) || (devLimit == 0U)) {
    return NFC_InvalidParameter;
  }

  *devCnt = 0;

  /* Step 1: Send Initiate */
  ret = RFal_ST25TB_PollerInitiate(&chipId);
  if (ret == NFC_OK) {
    /* If only 1 answer is detected */
    st25tbDevList[*devCnt].chipID       = chipId;
    st25tbDevList[*devCnt].isDeselected = false;

    /* Retrieve its UID and keep it Selected*/
    ret = RFal_ST25TB_PollerSelect(chipId);

    if (NFC_OK == ret) {
      ret = RFal_ST25TB_PollerGetUID(&st25tbDevList[*devCnt].UID);
    }

    if (NFC_OK == ret) {
      (*devCnt)++;
    }
  }
  /* Always proceed to Pcall16 anticollision as phase differences of tags can lead to no tag recognized, even if there is one */
  if (*devCnt < devLimit) {
    /* Multiple device responses */
    do {
      detected = RFal_ST25TB_PollerDoCollisionResolution(devLimit, st25tbDevList, devCnt);
    } while ((detected == true) && (*devCnt < devLimit));
  }

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerReadBlock(uint8_t blockAddress, RFal_ST25TB_Block *blockData)
{
  NFC_OpResult             ret;
  uint16_t               rxLen;
  RFal_ST25TB_ReadBlockReq readBlockReq;


  /* Compute Read Block Request */
  readBlockReq.cmd     = RFAL_ST25TB_READ_BLOCK_CMD;
  readBlockReq.address = blockAddress;

  /* Send Read Block Request */
  ret = RFal_TransceiveBlockingTx((uint8_t *)&readBlockReq, sizeof(RFal_ST25TB_ReadBlockReq), (uint8_t *)blockData, sizeof(RFal_ST25TB_Block), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT);

  /* Check for valid UID Response */
  if ((ret == NFC_OK) && (rxLen != RFAL_ST25TB_BLOCK_LEN)) {
    return NFC_ProtocolError;
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerWriteBlock(uint8_t blockAddress, const RFal_ST25TB_Block *blockData)
{
  NFC_OpResult              ret;
  uint16_t                rxLen;
  RFal_ST25TB_WriteBlockReq writeBlockReq;
  RFal_ST25TB_Block         tmpBlockData;


  /* Compute Write Block Request */
  writeBlockReq.cmd     = RFAL_ST25TB_WRITE_BLOCK_CMD;
  writeBlockReq.address = blockAddress;
  memcpy(&writeBlockReq.data, blockData, RFAL_ST25TB_BLOCK_LEN);

  /* Send Write Block Request */
  ret = RFal_TransceiveBlockingTx((uint8_t *)&writeBlockReq, sizeof(RFal_ST25TB_WriteBlockReq), tmpBlockData, RFAL_ST25TB_BLOCK_LEN, &rxLen, RFAL_TXRX_FLAGS_DEFAULT, (RFAL_ST25TB_FWT + RFAL_ST25TB_TW));

  /* Check if there was any error besides timeout */
  if (ret != NFC_SlaveTimeout) {
    /* Check if an unexpected answer was received */
    if (ret == NFC_OK) {
      return NFC_ProtocolError;
    }

    /* Check whether a transmission error occurred */
    if ((ret != NFC_CRC_Error) && (ret != NFC_FramingError) && (ret != NFC_MemoryError) && (ret != NFC_RF_Collision)) {
      return ret;
    }

    /* If a transmission error occurred (maybe noise while committing data) wait maximum programming time and verify data afterwards */
    RFal_SetGT((RFAL_ST25TB_FWT + RFAL_ST25TB_TW));
    RFal_FieldOnAndStartGT();
  }

  ret = RFal_ST25TB_PollerReadBlock(blockAddress, &tmpBlockData);
  if (ret == NFC_OK) {
    if (memcmp(&tmpBlockData, blockData, RFAL_ST25TB_BLOCK_LEN) == 0) {
      return NFC_OK;
    }
    return NFC_ProtocolError;
  }
  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerCompletion(void)
{
  uint8_t  completionReq;

  /* Compute Completion Request */
  completionReq = RFAL_ST25TB_COMPLETION_CMD;

  /* Send Completion Request, no response is expected */
  return RFal_TransceiveBlockingTx((uint8_t *)&completionReq, RFAL_ST25TB_CMD_LEN, NULL, 0, NULL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT);
}


/*******************************************************************************/
NFC_OpResult RFal_ST25TB_PollerResetToInventory(void)
{
  uint8_t resetInvReq;

  /* Compute Completion Request */
  resetInvReq = RFAL_ST25TB_RESET_INV_CMD;

  /* Send Completion Request, no response is expected */
  return RFal_TransceiveBlockingTx((uint8_t *)&resetInvReq, RFAL_ST25TB_CMD_LEN, NULL, 0, NULL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT);
}
