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

#ifndef RFAL_NFCV_H
#define RFAL_NFCV_H

#include "RFal_RF.h"

#include "NFC/NFC_Def.h"

#define RFAL_NFCV_UID_LEN                 8U              /*!< NFC-V UID length                                             */
#define RFAL_NFCV_MAX_BLOCK_LEN           32U             /*!< Max Block size: can be of up to 256 bits  ISO 15693 2000  5  */
#define RFAL_NFCV_BNO_LEN                 1U              /*!< NFC-V Block Number length                                    */
#define RFAL_NFCV_CRC_LEN                 2U              /*!< NFC-V CRC length                                             */
#define RFAL_NFCV_MAX_GEN_DATA_LEN        (RFAL_NFCV_MAX_BLOCK_LEN + RFAL_NFCV_BNO_LEN + RFAL_NFCV_UID_LEN) /*!<Max data    */
#define RFAL_NFCV_BLOCKNUM_LEN            1U              /*!< Block Number length on normal commands: 8 bits               */
#define RFAL_NFCV_BLOCKNUM_EXTENDED_LEN   2U              /*!< Block Number length on extended commands: 16 bits            */
#define RFAL_NFCV_PARAM_SKIP              0U              /*!< Skip proprietary Param Request                               */
#define RFAL_NFCV_ST_IC_MFG_CODE          0x02U           /*!< ST IC Mfg code (used for custom commands)                    */

/*! NFC-V RequestFlags   ISO15693 2000 7.3.1 */
enum {
  RFAL_NFCV_REQ_FLAG_DEFAULT           = 0x02U,         /*!< Default Request Flags                                        */
  RFAL_NFCV_REQ_FLAG_SUB_CARRIER       = 0x01U,         /*!< Sub Carrier flag                                             */
  RFAL_NFCV_REQ_FLAG_DATA_RATE         = 0x02U,         /*!< Data Rate flag                                               */
  RFAL_NFCV_REQ_FLAG_INVENTORY         = 0x04U,         /*!< Inventory flag                                               */
  RFAL_NFCV_REQ_FLAG_PROTOCOL_EXT      = 0x08U,         /*!< Protocol Extension flag                                      */
  RFAL_NFCV_REQ_FLAG_SELECT            = 0x10U,         /*!< Select flag                                                  */
  RFAL_NFCV_REQ_FLAG_ADDRESS           = 0x20U,         /*!< Address flag                                                 */
  RFAL_NFCV_REQ_FLAG_OPTION            = 0x40U,         /*!< Option flag                                                  */
  RFAL_NFCV_REQ_FLAG_RFU               = 0x80U,         /*!< RFU flag                                                     */
  RFAL_NFCV_REQ_FLAG_AFI               = 0x10U,         /*!< AFI flag                                                     */
  RFAL_NFCV_REQ_FLAG_NB_SLOTS          = 0x20U,         /*!< Number of Slots flag                                         */
};

/*! NFC-V Response Flags   ISO15693 2000 7.4.1 */
enum {
  RFAL_NFCV_RES_FLAG_ERROR             = 0x01U,         /*!< Error flag                                                   */
  RFAL_NFCV_RES_FLAG_RFU1              = 0x02U,         /*!< RFU flag                                                     */
  RFAL_NFCV_RES_FLAG_RFU2              = 0x04U,         /*!< RFU flag                                                     */
  RFAL_NFCV_RES_FLAG_EXTENSION         = 0x08U,         /*!< Extension flag                                               */
  RFAL_NFCV_RES_FLAG_RFU3              = 0x10U,         /*!< RFU flag                                                     */
  RFAL_NFCV_RES_FLAG_RFU4              = 0x20U,         /*!< RFU flag                                                     */
  RFAL_NFCV_RES_FLAG_RFU5              = 0x40U,         /*!< RFU flag                                                     */
  RFAL_NFCV_RES_FLAG_RFU6              = 0x80U          /*!< RFU flag                                                     */
};

/*! NFC-V Error code  ISO15693 2000 7.4.2 */
enum {
  RFAL_NFCV_ERROR_CMD_NOT_SUPPORTED    = 0x01U,         /*!< The command is not supported, code is not recognised         */
  RFAL_NFCV_ERROR_CMD_NOT_RECOGNIZED   = 0x02U,         /*!< The command is not recognised, format error occurred         */
  RFAL_NFCV_ERROR_OPTION_NOT_SUPPORTED = 0x03U,         /*!< The option is not supported                                  */
  RFAL_NFCV_ERROR_UNKNOWN              = 0x0FU,         /*!< Unknown error                                                */
  RFAL_NFCV_ERROR_BLOCK_NOT_AVALIABLE  = 0x10U,         /*!< The specified block is not available                         */
  RFAL_NFCV_ERROR_BLOCK_ALREDY_LOCKED  = 0x11U,         /*!< The specified block is already locked                        */
  RFAL_NFCV_ERROR_BLOCK_LOCKED         = 0x12U,         /*!< The specified block is locked                                */
  RFAL_NFCV_ERROR_WRITE_FAILED         = 0x13U,         /*!< The specified block was not successfully programmed          */
  RFAL_NFCV_ERROR_BLOCK_FAILED         = 0x14U          /*!< The specified block was not successfully locked              */
};


/*! NFC-V command set   ISO15693 2000 9.1 */
enum {
  RFAL_NFCV_CMD_INVENTORY                     = 0x01U,  /*!< INVENTORY_REQ (Inventory) command                            */
  RFAL_NFCV_CMD_SLPV                          = 0x02U,  /*!< SLPV_REQ (Stay quiet) command                                */
  RFAL_NFCV_CMD_READ_SINGLE_BLOCK             = 0x20U,  /*!< Read single block command                                    */
  RFAL_NFCV_CMD_WRITE_SINGLE_BLOCK            = 0x21U,  /*!< Write single block command                                   */
  RFAL_NFCV_CMD_LOCK_BLOCK                    = 0x22U,  /*!< Lock block command                                           */
  RFAL_NFCV_CMD_READ_MULTIPLE_BLOCKS          = 0x23U,  /*!< Read multiple blocks command                                 */
  RFAL_NFCV_CMD_WRITE_MULTIPLE_BLOCKS         = 0x24U,  /*!< Write multiple blocks command                                */
  RFAL_NFCV_CMD_SELECT                        = 0x25U,  /*!< Select command                                               */
  RFAL_NFCV_CMD_RESET_TO_READY                = 0x26U,  /*!< Reset To Ready command                                       */
  RFAL_NFCV_CMD_GET_SYS_INFO                  = 0x2BU,  /*!< Get System Information command                               */
  RFAL_NFCV_CMD_EXTENDED_READ_SINGLE_BLOCK    = 0x30U,  /*!< Extended read single block command                           */
  RFAL_NFCV_CMD_EXTENDED_WRITE_SINGLE_BLOCK   = 0x31U,  /*!< Extended write single block command                          */
  RFAL_NFCV_CMD_EXTENDED_LOCK_SINGLE_BLOCK    = 0x32U,  /*!< Extended lock single block command                           */
  RFAL_NFCV_CMD_EXTENDED_READ_MULTIPLE_BLOCK  = 0x33U,  /*!< Extended read multiple block command                         */
  RFAL_NFCV_CMD_EXTENDED_WRITE_MULTIPLE_BLOCK = 0x34U,  /*!< Extended read multiple block command                         */
  RFAL_NFCV_CMD_EXTENDED_GET_SYS_INFO         = 0x3BU   /*!< Extended Get System Information command                      */
};

/*! ST25TV/ST25DV command set  */
enum {
  RFAL_NFCV_CMD_READ_CONFIGURATION                 = 0xA0U,  /*!< Read configuration command                                 */
  RFAL_NFCV_CMD_WRITE_CONFIGURATION                = 0xA1U,  /*!< Write configuration command                                */
  RFAL_NFCV_CMD_SET_EAS                            = 0xA2U,  /*!< Set EAS command                                            */
  RFAL_NFCV_CMD_RESET_EAS                          = 0xA3U,  /*!< Reset EAS command                                          */
  RFAL_NFCV_CMD_LOCK_EAS                           = 0xA4U,  /*!< Lock EAS command                                           */
  RFAL_NFCV_CMD_ENABLE_EAS                         = 0xA5U,  /*!< Enable EAS command                                         */
  RFAL_NFCV_CMD_KILL                               = 0xA6U,  /*!< Kill command                                               */
  RFAL_NFCV_CMD_WRITE_EAS_ID                       = 0xA7U,  /*!< Write EAS ID command                                       */
  RFAL_NFCV_CMD_WRITE_EAS_CONFIG                   = 0xA8U,  /*!< Write EAS CONFIG command                                   */
  RFAL_NFCV_CMD_MANAGE_GPO                         = 0xA9U,  /*!< Manage GPO command                                         */
  RFAL_NFCV_CMD_WRITE_MESSAGE                      = 0xAAU,  /*!< Write Message command                                      */
  RFAL_NFCV_CMD_READ_MESSAGE_LENGTH                = 0xABU,  /*!< Read Message Length command                                */
  RFAL_NFCV_CMD_READ_MESSAGE                       = 0xACU,  /*!< Read Message command                                       */
  RFAL_NFCV_CMD_READ_DYN_CONFIGURATION             = 0xADU,  /*!< Read Dynamic Configuration command                         */
  RFAL_NFCV_CMD_WRITE_DYN_CONFIGURATION            = 0xAEU,  /*!< Write Dynamic Configuration command                        */
  RFAL_NFCV_CMD_WRITE_PASSWORD                     = 0xB1U,  /*!< Write Kill Password / Write Password command               */
  RFAL_NFCV_CMD_LOCK_KILL                          = 0xB2U,  /*!< Lock Kill command                                          */
  RFAL_NFCV_CMD_PRESENT_PASSWORD                   = 0xB3U,  /*!< Present Password command                                   */
  RFAL_NFCV_CMD_GET_RANDOM_NUMBER                  = 0xB4U,  /*!< Get Random Number command                                  */
  RFAL_NFCV_CMD_FAST_READ_SINGLE_BLOCK             = 0xC0U,  /*!< Fast Read single block command                             */
  RFAL_NFCV_CMD_FAST_READ_MULTIPLE_BLOCKS          = 0xC3U,  /*!< Fast Read multiple blocks command                          */
  RFAL_NFCV_CMD_FAST_EXTENDED_READ_SINGLE_BLOCK    = 0xC4U,  /*!< Fast Extended Read single block command                    */
  RFAL_NFCV_CMD_FAST_EXTENDED_READ_MULTIPLE_BLOCKS = 0xC5U,  /*!< Fast Extended Read multiple blocks command                 */
  RFAL_NFCV_CMD_FAST_WRITE_MESSAGE                 = 0xCAU,  /*!< Fast Write Message                                         */
  RFAL_NFCV_CMD_FAST_READ_MESSAGE_LENGTH           = 0xCBU,  /*!< Fast Read Message Length                                   */
  RFAL_NFCV_CMD_FAST_READ_MESSAGE                  = 0xCCU,  /*!< Fast Read Message                                          */
  RFAL_NFCV_CMD_FAST_READ_DYN_CONFIGURATION        = 0xCDU,  /*!< Fast Read Dynamic configuration                            */
  RFAL_NFCV_CMD_FAST_WRITE_DYN_CONFIGURATION       = 0xCEU   /*!< Fast Write Dynamic Configuration                           */
};

/*! ISO 15693 Get System info parameter request field ISO15693 2018 Table 94 */
enum {
  RFAL_NFCV_SYSINFO_DFSID      = 0x01U,                 /*!< Get System info DFSID flag                                   */
  RFAL_NFCV_SYSINFO_AFI        = 0x02U,                 /*!< Get System info AFI flag                                     */
  RFAL_NFCV_SYSINFO_MEMSIZE    = 0x04U,                 /*!< Get System info MEMSIZE flag                                 */
  RFAL_NFCV_SYSINFO_ICREF      = 0x08U,                 /*!< Get System info ICREF flag                                   */
  RFAL_NFCV_SYSINFO_MOI        = 0x10U,                 /*!< Get System info MOI flag                                     */
  RFAL_NFCV_SYSINFO_CMDLIST    = 0x20U,                 /*!< Get System info CMDLIST flag                                 */
  RFAL_NFCV_SYSINFO_CSI        = 0x40U,                 /*!< Get System info CSI flag                                     */
  RFAL_NFCV_SYSINFO_REQ_ALL    = 0x7FU                  /*!< Get System info request of all parameters                    */
};

/*! NFC-V Number of slots  Digital 2.0  9.6.1 */
typedef enum {
  RFAL_NFCV_NUM_SLOTS_1    =  0x20,   /*!< Number of slots: 1             */
  RFAL_NFCV_NUM_SLOTS_16   =  0x00,   /*!< Number of slots: 16            */
} RFal_NFCV_NumSlots;


/*! NFC-V INVENTORY_RES format   Digital 2.0  9.6.2 */
typedef struct {
  uint8_t RES_FLAG;                   /*!< Response Flags                 */
  uint8_t DSFID;                      /*!< Data Storage Format Identifier */
  uint8_t UID[RFAL_NFCV_UID_LEN];     /*!< NFC-V device UID               */
  uint8_t crc[RFAL_CRC_LEN];          /*!< CRC                            */
} RFal_NFCV_InventoryRes;


/*! NFC-V Generic Req format  */
typedef struct {
  uint8_t  REQ_FLAG;                              /*!< Request Flags      */
  uint8_t  CMD;                                   /*!< Command code       */
  union { /*  PRQA S 0750 # MISRA 19.2 - Both members are of the same type, just different names.  Thus no problem can occur. */
    uint8_t  UID[RFAL_NFCV_UID_LEN];            /*!< Mask Value         */
    uint8_t  data[RFAL_NFCV_MAX_GEN_DATA_LEN];  /*!< Data               */
  } payload;                                      /*!< Payload            */
} RFal_NFCV_GenericReq;


/*! NFC-V Generic Response format */
typedef struct {
  uint8_t  RES_FLAG;                              /*!< Response Flags     */
  uint8_t  data[RFAL_NFCV_MAX_GEN_DATA_LEN];      /*!< Data               */
} RFal_NFCV_GenericRes;


/*! NFC-V listener device (VICC) struct  */
typedef struct {
  RFal_NFCV_InventoryRes    InvRes;     /*!< INVENTORY_RES                  */
  bool                    isSleep;    /*!< Device sleeping flag           */
} RFal_NFCV_ListenDevice;

#ifdef __cplusplus
extern "C" {
#endif

NFC_OpResult RFal_NFCV_PollerInit(void);
NFC_OpResult RFal_NFCV_PollerCheckPresence(RFal_NFCV_InventoryRes *invRes);
NFC_OpResult RFal_NFCV_PollerInventory(RFal_NFCV_NumSlots nSlots, uint8_t maskLen, const uint8_t *maskVal, RFal_NFCV_InventoryRes *invRes, uint16_t *rcvdLen);
NFC_OpResult RFal_NFCV_PollerCollisionResolution(RFal_ComplianceMode compMode, uint8_t devLimit, RFal_NFCV_ListenDevice *nfcvDevList, uint8_t *devCnt);
NFC_OpResult RFal_NFCV_PollerSleepCollisionResolution(uint8_t devLimit, RFal_NFCV_ListenDevice *nfcvDevList, uint8_t *devCnt);
NFC_OpResult RFal_NFCV_PollerSleep(uint8_t flags, const uint8_t *uid);
NFC_OpResult RFal_NFCV_PollerSelect(uint8_t flags, const uint8_t *uid);
NFC_OpResult RFal_NFCV_PollerReadSingleBlock(uint8_t flags, const uint8_t *uid, uint8_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_NFCV_PollerWriteSingleBlock(uint8_t flags, const uint8_t *uid, uint8_t blockNum, const uint8_t *wrData, uint8_t blockLen);
NFC_OpResult RFal_NFCV_PollerReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_NFCV_PollerWriteMultipleBlocks(uint8_t flags, const uint8_t *uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t *txBuf, uint16_t txBufLen, uint8_t blockLen, const uint8_t *wrData, uint16_t wrDataLen);
NFC_OpResult RFal_NFCV_PollerLockBlock(uint8_t flags, const uint8_t *uid, uint8_t blockNum);
NFC_OpResult RFal_NFCV_PollerExtendedLockSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum);
NFC_OpResult RFal_NFCV_PollerExtendedReadSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_NFCV_PollerExtendedWriteSingleBlock(uint8_t flags, const uint8_t *uid, uint16_t blockNum, const uint8_t *wrData, uint8_t blockLen);
NFC_OpResult RFal_NFCV_PollerExtendedReadMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_NFCV_PollerExtendedWriteMultipleBlocks(uint8_t flags, const uint8_t *uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t *txBuf, uint16_t txBufLen, uint8_t blockLen, const uint8_t *wrData, uint16_t wrDataLen);
NFC_OpResult RFal_NFCV_PollerGetSystemInformation(uint8_t flags, const uint8_t *uid, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_NFCV_PollerExtendedGetSystemInformation(uint8_t flags, const uint8_t *uid, uint8_t requestField, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_NFCV_PollerTransceiveReq(uint8_t cmd, uint8_t flags, uint8_t param, const uint8_t *uid, const uint8_t *data, uint16_t dataLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_NFCV_H */