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

#ifndef RFAL_ST25TB_H
#define RFAL_ST25TB_H

#include "RFal_RF.h"
#include "RFal_NFCB.h"

#include "NFC/NFC_Def.h"

#define RFAL_ST25TB_CHIP_ID_LEN      1U       /*!< ST25TB chip ID length       */
#define RFAL_ST25TB_CRC_LEN          2U       /*!< ST25TB CRC length           */
#define RFAL_ST25TB_UID_LEN          8U       /*!< ST25TB Unique ID length     */
#define RFAL_ST25TB_BLOCK_LEN        4U       /*!< ST25TB Data Block length    */

typedef uint8_t RFal_ST25TB_UID[RFAL_ST25TB_UID_LEN];        /*!< ST25TB UID type          */
typedef uint8_t RFal_ST25TB_Block[RFAL_ST25TB_BLOCK_LEN];    /*!< ST25TB Block type        */


/*! ST25TB listener device (PICC) struct  */
typedef struct {
  uint8_t           chipID;                              /*!< Device's session Chip ID */
  RFal_ST25TB_UID     UID;                                 /*!< Device's UID             */
  bool              isDeselected;                        /*!< Device deselect flag     */
} RFal_ST25TB_ListenDevice;

#ifdef __cplusplus
extern "C" {
#endif

NFC_OpResult RFal_ST25TB_PollerInit(void);
NFC_OpResult RFal_ST25TB_PollerCheckPresence(uint8_t *chipId);
NFC_OpResult RFal_ST25TB_PollerCollisionResolution(uint8_t devLimit, RFal_ST25TB_ListenDevice *st25tbDevList, uint8_t *devCnt);
NFC_OpResult RFal_ST25TB_PollerInitiate(uint8_t *chipId);
NFC_OpResult RFal_ST25TB_PollerPcall(uint8_t *chipId);
NFC_OpResult RFal_ST25TB_PollerSlotMarker(uint8_t slotNum, uint8_t *chipIdRes);
NFC_OpResult RFal_ST25TB_PollerSelect(uint8_t chipId);
NFC_OpResult RFal_ST25TB_PollerGetUID(RFal_ST25TB_UID *UID);
NFC_OpResult RFal_ST25TB_PollerReadBlock(uint8_t blockAddress, RFal_ST25TB_Block *blockData);
NFC_OpResult RFal_ST25TB_PollerWriteBlock(uint8_t blockAddress, const RFal_ST25TB_Block *blockData);
NFC_OpResult RFal_ST25TB_PollerCompletion(void);
NFC_OpResult RFal_ST25TB_PollerResetToInventory(void);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_ST25TB_H */
