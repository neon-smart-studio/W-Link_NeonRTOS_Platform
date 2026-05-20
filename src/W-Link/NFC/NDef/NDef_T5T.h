
/**
  ******************************************************************************
  * @file           : ndef_t5t.h
  * @brief          : Provides NDEF methods and definitions to access NFC Forum Tags
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
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

#ifndef NDEF_T5T_H
#define NDEF_T5T_H

#include "NDef_Poller.h"

#include "NFC/RFal/RFal_NFC.h"

#include "NFC/NFC_Def.h"

#define NDEF_T5T_MAPPING_VERSION_1_0                     (1U << 6)    /*!< T5T Version 1.0                                    */

#define NDEF_SYSINFO_FLAG_DFSID_POS                           (0U)    /*!< Info flags DFSID flag position                     */
#define NDEF_SYSINFO_FLAG_AFI_POS                             (1U)    /*!< Info flags AFI flag position                       */
#define NDEF_SYSINFO_FLAG_MEMSIZE_POS                         (2U)    /*!< Info flags Memory Size flag position               */
#define NDEF_SYSINFO_FLAG_ICREF_POS                           (3U)    /*!< Info flags IC reference flag position              */
#define NDEF_SYSINFO_FLAG_MOI_POS                             (4U)    /*!< Info flags MOI flag position                       */
#define NDEF_SYSINFO_FLAG_CMDLIST_POS                         (5U)    /*!< Info flags Command List flag position              */
#define NDEF_SYSINFO_FLAG_CSI_POS                             (6U)    /*!< Info flags CSI flag position                       */
#define NDEF_SYSINFO_FLAG_LEN_POS                             (7U)    /*!< Info flags Length  position                        */

#define NDEF_CMDLIST_READSINGLEBLOCK_POS                      (0U)    /*!< Cmd List: ReadSingleBlock position                 */
#define NDEF_CMDLIST_WRITESINGLEBLOCK_POS                     (1U)    /*!< Cmd List: WriteSingleBlock position                */
#define NDEF_CMDLIST_LOCKSINGLEBLOCK_POS                      (2U)    /*!< Cmd List: LockSingleBlock position                 */
#define NDEF_CMDLIST_READMULTIPLEBLOCKS_POS                   (3U)    /*!< Cmd List: ReadMultipleBlocks position              */
#define NDEF_CMDLIST_WRITEMULTIPLEBLOCKS_POS                  (4U)    /*!< Cmd List: WriteMultipleBlocks position             */
#define NDEF_CMDLIST_SELECT_POS                               (5U)    /*!< Cmd List: SelectSupported position                 */
#define NDEF_CMDLIST_RESETTOREADY_POS                         (6U)    /*!< Cmd List: ResetToReady position                    */
#define NDEF_CMDLIST_GETMULTIPLEBLOCKSECSTATUS_POS            (7U)    /*!< Cmd List: GetMultipleBlockSecStatus position       */

#define NDEF_CMDLIST_WRITEAFI_POS                             (0U)    /*!< Cmd List: WriteAFI position                        */
#define NDEF_CMDLIST_LOCKAFI_POS                              (1U)    /*!< Cmd List: LockAFI position                         */
#define NDEF_CMDLIST_WRITEDSFID_POS                           (2U)    /*!< Cmd List: WriteDSFID position                      */
#define NDEF_CMDLIST_LOCKDSFID_POS                            (3U)    /*!< Cmd List: LockDSFID position                       */
#define NDEF_CMDLIST_GETSYSTEMINFORMATION_POS                 (4U)    /*!< Cmd List: GetSystemInformation position            */
#define NDEF_CMDLIST_CUSTOMCMDS_POS                           (5U)    /*!< Cmd List: CustomCmds position                      */
#define NDEF_CMDLIST_FASTREADMULTIPLEBLOCKS_POS               (6U)    /*!< Cmd List: FastReadMultipleBlocks position          */

#define NDEF_CMDLIST_EXTREADSINGLEBLOCK_POS                   (0U)    /*!< Cmd List: ExtReadSingleBlock position              */
#define NDEF_CMDLIST_EXTWRITESINGLEBLOCK_POS                  (1U)    /*!< Cmd List: ExtWriteSingleBlock position             */
#define NDEF_CMDLIST_EXTLOCKSINGLEBLOCK_POS                   (2U)    /*!< Cmd List: ExtLockSingleBlock position              */
#define NDEF_CMDLIST_EXTREADMULTIPLEBLOCKS_POS                (3U)    /*!< Cmd List: ExtReadMultipleBlocks position           */
#define NDEF_CMDLIST_EXTWRITEMULTIPLEBLOCKS_POS               (4U)    /*!< Cmd List: ExtWriteMultipleBlocks position          */
#define NDEF_CMDLIST_EXTGETMULTIPLEBLOCKSECSTATUS_POS         (5U)    /*!< Cmd List: ExtGetMultipleBlockSecStatus position    */
#define NDEF_CMDLIST_FASTEXTENDEDREADMULTIPLEBLOCKS_POS       (6U)    /*!< Cmd List: FastExtendedReadMultipleBlocks position  */

#define NDEF_T5T_CC_MAGIC_1_BYTE_ADDR_MODE                   0xE1U    /*!< T5T CC Magic Number (1-byte Address Mode)           */
#define NDEF_T5T_CC_MAGIC_2_BYTE_ADDR_MODE                   0xE2U    /*!< T5T CC Magic Number (2-byte Address Mode)           */
#define NDEF_T5T_CC_LEN_4_BYTES                                 4U    /*!< T5T CC Length (4 bytes)                            */
#define NDEF_T5T_CC_LEN_8_BYTES                                 8U    /*!< T5T CC Length (8 bytes)                            */
#define NDEF_T5T_FORMAT_OPTION_NFC_FORUM                        1U    /*!< Format tag according to NFC Forum MLEN computation */

#define NDef_T5T_MajorVersion(V)                                         ((uint8_t)( (V) >> 6U))                                            /*!< Get major version                                  */
#define NDef_T5T_MinorVersion(V)                                         ((uint8_t)(((V) >> 4U) & 3U))                                      /*!< Get minor version                                  */

#define NDef_T5T_SysInfoDFSIDPresent(infoFlags)                          (((infoFlags) >> NDEF_SYSINFO_FLAG_DFSID_POS)   & 0x01U)           /*!< Returns DFSID presence flag                        */
#define NDef_T5T_SysInfoAFIPresent(infoFlags)                            (((infoFlags) >> NDEF_SYSINFO_FLAG_AFI_POS)     & 0x01U)           /*!< Returns AFI presence flag                          */
#define NDef_T5T_SysInfoMemSizePresent(infoFlags)                        (((infoFlags) >> NDEF_SYSINFO_FLAG_MEMSIZE_POS) & 0x01U)           /*!< Returns Memory size presence flag                  */
#define NDef_T5T_SysInfoICRefPresent(infoFlags)                          (((infoFlags) >> NDEF_SYSINFO_FLAG_ICREF_POS)   & 0x01U)           /*!< Returns IC Reference presence flag                 */
#define NDef_T5T_SysInfoMOIValue(infoFlags)                              (((infoFlags) >> NDEF_SYSINFO_FLAG_MOI_POS)     & 0x01U)           /*!< Returns MOI value                                  */
#define NDef_T5T_SysInfoCmdListPresent(infoFlags)                        (((infoFlags) >> NDEF_SYSINFO_FLAG_CMDLIST_POS) & 0x01U)           /*!< Returns Command List presence flag                 */
#define NDef_T5T_SysInfoCSIPresent(infoFlags)                            (((infoFlags) >> NDEF_SYSINFO_FLAG_CSI_POS)     & 0x01U)           /*!< Returns CSI presence flag                          */
#define NDef_T5T_SysInfoLenValue(infoFlags)                              (((infoFlags) >> NDEF_SYSINFO_FLAG_LEN_POS)     & 0x01U)           /*!< Returns Info flag length value                     */

#define NDef_T5T_SysInfoReadSingleBlockSupported(cmdList)                (((cmdList)[0] >> NDEF_CMDLIST_READSINGLEBLOCK_POS)                & 0x01U) /*!< Returns ReadSingleBlock support flag                 */
#define NDef_T5T_SysInfoWriteSingleBlockSupported(cmdList)               (((cmdList)[0] >> NDEF_CMDLIST_WRITESINGLEBLOCK_POS)               & 0x01U) /*!< Returns WriteSingleBlock support flag                */
#define NDef_T5T_SysInfoLockSingleBlockSupported(cmdList)                (((cmdList)[0] >> NDEF_CMDLIST_LOCKSINGLEBLOCK_POS)                & 0x01U) /*!< Returns LockSingleBlock support flag                 */
#define NDef_T5T_SysInfoReadMultipleBlocksSupported(cmdList)             (((cmdList)[0] >> NDEF_CMDLIST_READMULTIPLEBLOCKS_POS)             & 0x01U) /*!< Returns ReadMultipleBlocks support flag              */
#define NDef_T5T_SysInfoWriteMultipleBlocksSupported(cmdList)            (((cmdList)[0] >> NDEF_CMDLIST_WRITEMULTIPLEBLOCKS_POS)            & 0x01U) /*!< Returns WriteMultipleBlocks support flag             */
#define NDef_T5T_SysInfoSelectSupported(cmdList)                         (((cmdList)[0] >> NDEF_CMDLIST_SELECT_POS)                         & 0x01U) /*!< Returns SelectSupported support flag                 */
#define NDef_T5T_SysInfoResetToReadySupported(cmdList)                   (((cmdList)[0] >> NDEF_CMDLIST_RESETTOREADY_POS)                   & 0x01U) /*!< Returns ResetToReady support flag                    */
#define NDef_T5T_SysInfoGetMultipleBlockSecStatusSupported(cmdList)      (((cmdList)[0] >> NDEF_CMDLIST_GETMULTIPLEBLOCKSECSTATUS_POS)      & 0x01U) /*!< Returns GetMultipleBlockSecStatus support flag       */

#define NDef_T5T_SysInfoWriteAFISupported(cmdList)                       (((cmdList)[1] >> NDEF_CMDLIST_WRITEAFI_POS)                       & 0x01U) /*!< Returns WriteAFI support flag                        */
#define NDef_T5T_SysInfoLockAFISupported(cmdList)                        (((cmdList)[1] >> NDEF_CMDLIST_LOCKAFI_POS)                        & 0x01U) /*!< Returns LockAFI support flag                         */
#define NDef_T5T_SysInfoWriteDSFIDSupported(cmdList)                     (((cmdList)[1] >> NDEF_CMDLIST_WRITEDSFID_POS)                     & 0x01U) /*!< Returns WriteDSFID support flag                      */
#define NDef_T5T_SysInfoLockDSFIDSupported(cmdList)                      (((cmdList)[1] >> NDEF_CMDLIST_LOCKDSFID_POS)                      & 0x01U) /*!< Returns LockDSFID support flag                       */
#define NDef_T5T_SysInfoGetSystemInformationSupported(cmdList)           (((cmdList)[1] >> NDEF_CMDLIST_GETSYSTEMINFORMATION_POS)           & 0x01U) /*!< Returns GetSystemInformation support flag            */
#define NDef_T5T_SysInfoCustomCmdsSupported(cmdList)                     (((cmdList)[1] >> NDEF_CMDLIST_CUSTOMCMDS_POS)                     & 0x01U) /*!< Returns CustomCmds support flag                      */
#define NDef_T5T_SysInfoFastReadMultipleBlocksSupported(cmdList)         (((cmdList)[1] >> NDEF_CMDLIST_FASTREADMULTIPLEBLOCKS_POS)         & 0x01U) /*!< Returns FastReadMultipleBlocks support flag          */

#define NDef_T5T_SysInfoExtReadSingleBlockSupported(cmdList)             (((cmdList)[2] >> NDEF_CMDLIST_EXTREADSINGLEBLOCK_POS)             & 0x01U) /*!< Returns ExtReadSingleBlock support flag              */
#define NDef_T5T_SysInfoExtWriteSingleBlockSupported(cmdList)            (((cmdList)[2] >> NDEF_CMDLIST_EXTWRITESINGLEBLOCK_POS)            & 0x01U) /*!< Returns ExtWriteSingleBlock support flag             */
#define NDef_T5T_SysInfoExtLockSingleBlockSupported(cmdList)             (((cmdList)[2] >> NDEF_CMDLIST_EXTLOCKSINGLEBLOCK_POS)             & 0x01U) /*!< Returns ExtLockSingleBlock support flag              */
#define NDef_T5T_SysInfoExtReadMultipleBlocksSupported(cmdList)          (((cmdList)[2] >> NDEF_CMDLIST_EXTREADMULTIPLEBLOCKS_POS)          & 0x01U) /*!< Returns ExtReadMultipleBlocks support flag           */
#define NDef_T5T_SysInfoExtWriteMultipleBlocksSupported(cmdList)         (((cmdList)[2] >> NDEF_CMDLIST_EXTWRITEMULTIPLEBLOCKS_POS)         & 0x01U) /*!< Returns ExtWriteMultipleBlocks support flag          */
#define NDef_T5T_SysInfoExtGetMultipleBlockSecStatusSupported(cmdList)   (((cmdList)[2] >> NDEF_CMDLIST_EXTGETMULTIPLEBLOCKSECSTATUS_POS)   & 0x01U) /*!< Returns ExtGetMultipleBlockSecStatus support flag    */
#define NDef_T5T_SysInfoFastExtendedReadMultipleBlocksSupported(cmdList) (((cmdList)[2] >> NDEF_CMDLIST_FASTEXTENDEDREADMULTIPLEBLOCKS_POS) & 0x01U) /*!< Returns FastExtendedReadMultipleBlocks support flag  */

#define NDef_T5T_InvalidateCache(ctx)     { (ctx)->subCtx.t5t.cacheBlock = 0xFFFFFFFFU; }    /*!< Invalidate the internal cache, before reading a buffer  */
#define NDef_T5T_IsValidCache(ctx, block) ( (ctx)->subCtx.t5t.cacheBlock == (block) )        /*!< Check the internal cache is valid to avoid useless read */

/*! T5T Access mode */
typedef enum {
  NDEF_T5T_ACCESS_MODE_SELECTED,
  NDEF_T5T_ACCESS_MODE_ADDRESSED,
  NDEF_T5T_ACCESS_MODE_NON_ADDRESSED,
} NDef_T5T_AccessMode;

bool NDef_T5T_isSTDevice(const RFal_NFC_Device *dev);
bool NDef_T5T_isT5TDevice(const RFal_NFC_Device *dev);
NFC_OpResult NDef_T5T_Poller_AccessMode(NDef_Context *ctx, const RFal_NFC_Device *dev, NDef_T5T_AccessMode mode);
uint8_t NDef_T5T_GetBlockLength(NDef_Context *ctx);
NFC_OpResult NDef_T5T_GetMemoryConfig(NDef_Context *ctx);
bool NDef_T5T_IsMultipleBlockReadSupported(NDef_Context *ctx);
NFC_OpResult NDef_T5T_IsDevicePresent(NDef_Context *ctx);
NFC_OpResult NDef_T5T_LockDevice(NDef_Context *ctx);

NFC_OpResult NDef_T5T_Poller_SetAccessMode(NDef_T5T_AccessMode mode);
NFC_OpResult NDef_T5T_Poller_MultipleBlockRead(NDef_Context *ctx, bool enable);
NFC_OpResult NDef_T5T_Poller_ContextInitialization(NDef_Context *ctx, const RFal_NFC_Device *dev);
NFC_OpResult NDef_T5T_Poller_NdefDetect(NDef_Context *ctx, NDef_Info *info);
NFC_OpResult NDef_T5T_Poller_ReadBytes(NDef_Context *ctx, uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);
NFC_OpResult NDef_T5T_Poller_WriteBytes(NDef_Context *ctx, uint32_t offset, const uint8_t *buf, uint32_t len, bool pad, bool writeTerminator);
NFC_OpResult NDef_T5T_Poller_ReadRawMessage(NDef_Context *ctx, uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen, bool single);
NFC_OpResult NDef_T5T_Poller_WriteRawMessage(NDef_Context *ctx, const uint8_t *buf, uint32_t bufLen);
NFC_OpResult NDef_T5T_Poller_WriteRawMessageLen(NDef_Context *ctx, uint32_t rawMessageLen, bool writeTerminator);
NFC_OpResult NDef_T5T_Poller_TagFormat(NDef_Context *ctx, const NDef_CapabilityContainer *cc, uint32_t options);
NFC_OpResult NDef_T5T_Poller_CheckPresence(NDef_Context *ctx);
NFC_OpResult NDef_T5T_Poller_CheckAvailableSpace(const NDef_Context *ctx, uint32_t messageLen);
NFC_OpResult NDef_T5T_Poller_BeginWriteMessage(NDef_Context *ctx, uint32_t messageLen);
NFC_OpResult NDef_T5T_Poller_EndWriteMessage(NDef_Context *ctx, uint32_t messageLen, bool writeTerminator);
NFC_OpResult NDef_T5T_Poller_SetReadOnly(NDef_Context *ctx);

#endif /* NDEF_T5T_H */
