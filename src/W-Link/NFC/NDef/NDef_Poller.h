
/**
  ******************************************************************************
  * @file           : NDef_poller.h
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

#ifndef NDEF_POLLER_H
#define NDEF_POLLER_H

#include "NDef_Message.h"

#include "NFC/RFal/RFal_NFC.h"
#include "NFC/RFal/RFal_NFCF.h"
#include "NFC/RFal/RFal_ISO_Dep.h"

#define NDEF_CC_BUF_LEN             17U                                                /*!< CC buffer len. Max length = 17 in case of T4T v3             */
#define NDEF_NFCV_SUPPORTED_CMD_LEN  4U                                                /*!< Ext sys info supported commands list length                  */
#define NDEF_NFCV_UID_LEN            8U                                                /*!< NFC-V UID length                                             */

#define NDEF_SHORT_VFIELD_MAX_LEN  254U                                                /*!< Max V-field length for 1-byte Length encoding                 */
#define NDEF_TERMINATOR_TLV_LEN      1U                                                /*!< Terminator TLV size                                          */
#define NDEF_TERMINATOR_TLV_T     0xFEU                                                /*!< Terminator TLV T=FEh                                         */

#define NDEF_T2T_READ_RESP_SIZE     16U                                                /*!< Size of the READ response i.e. four blocks                   */
#define NDEF_T2T_MAX_RSVD_AREAS      3U                                                /*!< Number of reserved areas including 1 Dyn Lock area           */

#define NDEF_T3T_BLOCK_SIZE         16U                                                /*!< size for a block in t3t                                      */
#define NDEF_T3T_MAX_NB_BLOCKS       4U                                                /*!< size for a block in t3t                                      */
#define NDEF_T3T_BLOCK_NUM_MAX_SIZE  3U                                                /*!< Maximum size for a block number                              */
#define NDEF_T3T_MAX_RX_SIZE      ((NDEF_T3T_BLOCK_SIZE*NDEF_T3T_MAX_NB_BLOCKS) + 13U) /*!< size for a CHECK Response 13 bytes (LEN+07h+NFCID2+Status+Nos) + (block size x Max Nob)                                                */
#define NDEF_T3T_MAX_TX_SIZE      (((NDEF_T3T_BLOCK_SIZE + NDEF_T3T_BLOCK_NUM_MAX_SIZE) * NDEF_T3T_MAX_NB_BLOCKS) + 14U) \
                                                                                       /*!< size for an UPDATE command, 11 bytes (LEN+08h+NFCID2+Nos) + 2 bytes for 1 SC + 1 byte for NoB + (block size + block num Len) x Max NoB */

#define NDEF_T5T_TxRx_BUFF_HEADER_SIZE        1U                                       /*!< Request Flags/Responses Flags size                           */
#define NDEF_T5T_TxRx_BUFF_FOOTER_SIZE        2U                                       /*!< CRC size                                                     */

#define NDEF_T5T_TxRx_BUFF_SIZE               \
          (32U +  NDEF_T5T_TxRx_BUFF_HEADER_SIZE + NDEF_T5T_TxRx_BUFF_FOOTER_SIZE)     /*!< T5T working buffer size                                      */

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */

#define NDef_Bytes2Uint16(hiB, loB)          ((uint16_t)((((uint32_t)(hiB)) << 8U) | ((uint32_t)(loB))))                                                  /*!< convert 2 bytes to a u16 */

#define NDef_MajorVersion(V)                 ((uint8_t)((V) >>  4U))    /*!< Get major version */
#define NDef_MinorVersion(V)                 ((uint8_t)((V) & 0xFU))    /*!< Get minor version */


/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */


/*! NDEF device type */
typedef enum {
  NDEF_DEV_NONE          = 0x00U,                            /*!< Device type uudef                                  */
  NDEF_DEV_T1T           = 0x01U,                            /*!< Device type T1T                                    */
  NDEF_DEV_T2T           = 0x02U,                            /*!< Device type T2T                                    */
  NDEF_DEV_T3T           = 0x03U,                            /*!< Device type T3T                                    */
  NDEF_DEV_T4T           = 0x04U,                            /*!< Device type T4AT/T4BT                              */
  NDEF_DEV_T5T           = 0x05U,                            /*!< Device type T5T                                    */
} NDef_Device_Type;

/*! NDEF states  */
typedef enum {
  NDEF_STATE_INVALID     = 0x00U,                            /*!< Invalid state (e.g. no CC)                         */
  NDEF_STATE_INITIALIZED = 0x01U,                            /*!< State Initialized (no NDEF message)                */
  NDEF_STATE_READWRITE   = 0x02U,                            /*!< Valid NDEF found. Read/Write capability            */
  NDEF_STATE_READONLY    = 0x03U,                            /*!< Valid NDEF found. Read only                        */
} NDef_State;

/*! NDEF Information */
typedef struct {
  uint8_t                  majorVersion;                     /*!< Major version                                      */
  uint8_t                  minorVersion;                     /*!< Minor version                                      */
  uint32_t                 areaLen;                          /*!< Area Length for NDEF storage                       */
  uint32_t                 areaAvalableSpaceLen;             /*!< Remaining Space in case a propTLV is present       */
  uint32_t                 messageLen;                       /*!< NDEF message Length                                */
  NDef_State               state;                            /*!< Tag state e.g. NDEF_STATE_INITIALIZED              */
} NDef_Info;

/*! T1T Capability Container  */
typedef struct {
  uint8_t                  magicNumber;                      /*!< Magic number e.g. E1h                              */
  uint8_t                  majorVersion;                     /*!< Major version i.e. 1                               */
  uint8_t                  minorVersion;                     /*!< Minor version i.e. 2                               */
  uint16_t                 tagMemorySize;                    /*!< Tag Memory Size (TMS)                              */
  uint8_t                  readAccess;                       /*!< NDEF READ access condition                         */
  uint8_t                  writeAccess;                      /*!< NDEF WRITE access condition                        */
} NDef_CapabilityContainer_T1T;

/*! T2T Capability Container  */
typedef struct {
  uint8_t                  magicNumber;                      /*!< Magic number e.g. E1h                              */
  uint8_t                  majorVersion;                     /*!< Major version i.e. 1                               */
  uint8_t                  minorVersion;                     /*!< Minor version i.e. 2                               */
  uint8_t                  size;                             /*!< Size. T2T_Area_Size = Size * 8                     */
  uint8_t                  readAccess;                       /*!< NDEF READ access condition                         */
  uint8_t                  writeAccess;                      /*!< NDEF WRITE access condition                        */
} NDef_CapabilityContainer_T2T;

/*! T3T Attribute info block  */
typedef struct {
  uint8_t                  majorVersion;                     /*!< Major version i.e. 1                               */
  uint8_t                  minorVersion;                     /*!< Minor version i.e. 2                               */
  uint8_t                  nbR;                              /*!< Nbr: number of blocks read in one CHECK cmd        */
  uint8_t                  nbW;                              /*!< Nbr: number of blocks written in one UPDATE cmd    */
  uint16_t                 nMaxB;                            /*!< NmaxB: max number of blocks for NDEF data          */
  uint8_t                  writeFlag;                        /*!< WriteFlag indicates completion of previous NDEF    */
  uint8_t                  rwFlag;                           /*!< RWFlag indicates whether the NDEF can be updated   */
  uint32_t                 Ln;                               /*!< Ln size of the actual stored NDEF data in bytes    */
} NDef_AttribInfoBlock_T3T;

/*! T4T Capability Container  */
typedef struct {
  uint16_t                 ccLen;                            /*!< CCFILE Length                                      */
  uint8_t                  vNo;                              /*!< Mapping version                                    */
  uint16_t                 mLe;                              /*!< Max data size that can be read using a ReadBinary  */
  uint16_t                 mLc;                              /*!< Max data size that can be sent using a single cmd  */
  uint8_t                  fileId[2];                        /*!< NDEF File Identifier                               */
  uint32_t                 fileSize;                         /*!< NDEF File Size                                     */
  uint8_t                  readAccess;                       /*!< NDEF File READ access condition                    */
  uint8_t                  writeAccess;                      /*!< NDEF File WRITE access condition                   */
} NDef_CapabilityContainer_T4T;

/*! T5T Capability Container  */
typedef struct {
  uint8_t                  ccLen;                            /*!< CC Length                                          */
  uint8_t                  magicNumber;                      /*!< Magic number i.e. E1h or E2h                       */
  uint8_t                  majorVersion;                     /*!< Major version i.e. 1                               */
  uint8_t                  minorVersion;                     /*!< Minor version i.e. 0                               */
  uint8_t                  readAccess;                       /*!< NDEF READ access condition                         */
  uint8_t                  writeAccess;                      /*!< NDEF WRITE access condition                        */
  uint16_t                 memoryLen;                        /*!< MLEN (Memory Len). T5T_Area size = 8 * MLEN (bytes)*/
  bool                     specialFrame;                     /*!< Use Special Frames for Write-alike commands        */
  bool                     lockBlock;                        /*!< (EXTENDED_)LOCK_SINGLE_BLOCK supported             */
  bool                     mlenOverflow;                     /*!< memory size exceeds 2040 bytes (Android)           */
  bool                     multipleBlockRead;                /*!< (EXTENDED_)READ_MULTIPLE_BLOCK supported           */
} NDef_CapabilityContainer_T5T;

/*! Generic Capability Container  */
typedef union {
  NDef_CapabilityContainer_T1T   t1t;                          /*!< T1T Capability Container                           */
  NDef_CapabilityContainer_T2T   t2t;                          /*!< T2T Capability Container                           */
  NDef_AttribInfoBlock_T3T       t3t;                          /*!< T3T Attribute Information Block                    */
  NDef_CapabilityContainer_T4T   t4t;                          /*!< T4T Capability Container                           */
  NDef_CapabilityContainer_T5T   t5t;                          /*!< T5T Capability Container                           */
} NDef_CapabilityContainer;

/*! NDEF T1T sub context structure */
typedef struct {
  void *rfu;                                                 /*!< RFU                                                */
} NDef_T1TContext;

/*! NDEF T2T sub context structure */
typedef struct {
  uint8_t                      currentSecNo;                                   /*!< Current sector number                          */
  uint8_t                      cacheBuf[NDEF_T2T_READ_RESP_SIZE];              /*!< Cache buffer                                   */
  uint8_t                      nbrRsvdAreas;                                   /*!< Number of reserved Areas                        */
  uint16_t                     dynLockNbrLockBits;                             /*!< Number of bits inside the DynLock_Area         */
  uint16_t                     dynLockBytesLockedPerBit;                       /*!< Number of bytes locked by one Dynamic Lock bit */
  uint16_t                     dynLockNbrBytes;                                /*!< Number of bytes inside the DynLock_Area        */
  uint16_t                     rsvdAreaSize[NDEF_T2T_MAX_RSVD_AREAS];          /*!< Sizes of reserved areas                        */
  uint32_t                     cacheAddr;                                      /*!< Address of cached data                         */
  uint32_t                     offsetNdefTLV;                                  /*!< NDEF TLV message offset                        */
  uint32_t                     dynLockFirstByteAddr;                           /*!< Address of the first byte of the DynLock_Area  */
  uint32_t                     rsvdAreaFirstByteAddr[NDEF_T2T_MAX_RSVD_AREAS]; /*!< Addresses of reserved areas                    */
} NDef_T2TContext;

/*! NDEF T3T sub context structure */
typedef struct {
  uint8_t                      NFCID2[RFAL_NFCF_NFCID2_LEN];        /*!< NFCID2                                                  */
  uint8_t                      txbuf[NDEF_T3T_MAX_TX_SIZE];         /*!< Tx buffer dedicated for T3T internal operations         */
  uint8_t                      rxbuf[NDEF_T3T_MAX_RX_SIZE];         /*!< Rx buffer dedicated for T3T internal operations         */
  RFal_NFCF_BlockListElem      listBlocks[NDEF_T3T_MAX_NB_BLOCKS];  /*!< block number list for T3T internal operations           */
} NDef_T3TContext;

/*! NDEF T4T sub context structure */
typedef struct {
  uint8_t                      curMLe;                       /*!< Current MLe. Default Fh until CC file is read      */
  uint8_t                      curMLc;                       /*!< Current MLc. Default Dh until CC file is read      */
  bool                         mv1Flag;                      /*!< Mapping version 1 flag                             */
  RFal_ISO_Dep_ApduBufFormat   cApduBuf;                     /*!< Command-APDU buffer                                */
  RFal_ISO_Dep_ApduBufFormat   rApduBuf;                     /*!< Response-APDU buffer                               */
  RFal_T4T_RApduParam          respAPDU;                     /*!< Response-APDU params                               */
  RFal_ISO_Dep_BufFormat       tmpBuf;                       /*!< I-Block temporary buffer                           */
  uint16_t                     rApduBodyLen;                 /*!< Response Body Length                               */
  uint32_t                     FWT;                          /*!< Frame Waiting Time (1/fc)                          */
  uint32_t                     dFWT;                         /*!< Delta Frame Waiting Time (1/fc)                    */
  uint16_t                     FSx;                          /*!< Frame Size Device/Card (FSD or FSC)                */
  uint8_t                      DID;                          /*!< Device ID                                          */
} NDef_T4TContext;

/*! NFCV (Extended) System Information  */
typedef struct {
  uint16_t    numberOfBlock;                    /*!< Number of block                                    */
  uint8_t     UID[NDEF_NFCV_UID_LEN];           /*!< UID Value                                          */
  uint8_t     supportedCmd[NDEF_NFCV_SUPPORTED_CMD_LEN];/*!< Supported Commands list                    */
  uint8_t     infoFlags;                        /*!< Information flags                                  */
  uint8_t     DFSID;                            /*!< DFSID Value                                        */
  uint8_t     AFI;                              /*!< AFI Value                                          */
  uint8_t     blockSize;                        /*!< Block Size Value                                   */
  uint8_t     ICRef;                            /*!< IC Reference                                       */
} NDef_SystemInformation;

/*! NDEF T5T sub context structure */
typedef struct {
  const uint8_t               *uid;                          /*!< UID in Addressed mode, NULL: Non-addr/Selected mode*/
  uint8_t                      flags;                        /*!< Command flags                                      */
  uint32_t                     TlvNDEFOffset;                /*!< NDEF TLV message offset                            */
  uint8_t                      blockLen;                     /*!< T5T BLEN parameter                                 */
  NDef_SystemInformation       sysInfo;                      /*!< System Information (when supported)                */
  bool                         sysInfoSupported;             /*!< System Information Supported flag                  */
  bool                         legacySTHighDensity;          /*!< Legacy ST High Density flag                        */
  uint8_t                      txrxBuf[NDEF_T5T_TxRx_BUFF_SIZE];/*!< Tx Rx Buffer                                    */
  uint8_t                      cacheBuf[NDEF_T5T_TxRx_BUFF_SIZE];/*!< Cache buffer                                   */
  uint32_t                     cacheBlock;                   /*!< Block number of cached buffer                      */
  bool                         useMultipleBlockRead;         /*!< Access multiple block read                         */
  bool                         stDevice;                     /*!< ST device                                          */
} NDef_T5TContext;

/*! NDEF context structure */
typedef struct {
  NDef_Device_Type               type;                         /*!< NDEF Device type                                   */
  RFal_NFC_Device                device;                       /*!< NDEF Device                                        */
  NDef_State                     state;                        /*!< Tag state e.g. NDEF_STATE_INITIALIZED              */
  NDef_CapabilityContainer       cc;                           /*!< Capability Container                               */
  uint32_t                       messageLen;                   /*!< NDEF message length                                */
  uint32_t                       messageOffset;                /*!< NDEF message offset                                */
  uint32_t                       areaLen;                      /*!< Area Length for NDEF storage                       */
  uint8_t                        ccBuf[NDEF_CC_BUF_LEN];       /*!< buffer for CC                                      */
  const struct NDef_Poller_WrapperStruct *
    NDefPollWrapper;              /*!< pointer to array of function for wrapper           */
  union {
    NDef_T1TContext t1t;                                    /*!< T1T context                                        */
    NDef_T2TContext t2t;                                    /*!< T2T context                                        */
    NDef_T3TContext t3t;                                    /*!< T3T context                                        */
    NDef_T4TContext t4t;                                    /*!< T4T context                                        */
    NDef_T5TContext t5t;                                    /*!< T5T context                                        */
  } subCtx;                                                  /*!< Sub-context union                                  */
} NDef_Context;

/*! Wrapper structure to hold the function pointers on each tag type */
typedef struct NDef_Poller_WrapperStruct {
  NFC_OpResult(* pollerContextInitialization)(NDef_Context *ctx, const RFal_NFC_Device *dev);                                                   /*!< ContextInitialization function pointer                 */
  NFC_OpResult(* pollerNdefDetect)(NDef_Context *ctx, NDef_Info *info);                                                                     /*!< NdefDetect function pointer                            */
  NFC_OpResult(* pollerReadBytes)(NDef_Context *ctx, uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);                      /*!< Read function pointer                                  */
  NFC_OpResult(* pollerReadRawMessage)(NDef_Context *ctx, uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen, bool single);                  /*!< ReadRawMessage function pointer                        */
  NFC_OpResult(* pollerWriteBytes)(NDef_Context *ctx, uint32_t offset, const uint8_t *buf, uint32_t len, bool pad, bool writeTerminator);  /*!< Write function pointer                                 */
  NFC_OpResult(* pollerWriteRawMessage)(NDef_Context *ctx, const uint8_t *buf, uint32_t bufLen);                                           /*!< WriteRawMessage function pointer                       */
  NFC_OpResult(* pollerTagFormat)(NDef_Context *ctx, const NDef_CapabilityContainer *cc, uint32_t options);                                 /*!< TagFormat function pointer                             */
  NFC_OpResult(* pollerWriteRawMessageLen)(NDef_Context *ctx, uint32_t rawMessageLen, bool writeTerminator);                               /*!< WriteRawMessageLen function pointer                    */
  NFC_OpResult(* pollerCheckPresence)(NDef_Context *ctx);                                                                                  /*!< CheckPresence function pointer                         */
  NFC_OpResult(* pollerCheckAvailableSpace)(const NDef_Context *ctx, uint32_t messageLen);                                                 /*!< CheckAvailableSpace function pointer                   */
  NFC_OpResult(* pollerBeginWriteMessage)(NDef_Context *ctx, uint32_t messageLen);                                                         /*!< BeginWriteMessage function pointer                     */
  NFC_OpResult(* pollerEndWriteMessage)(NDef_Context *ctx, uint32_t messageLen, bool writeTerminator);                                     /*!< EndWriteMessage function pointer                       */
  NFC_OpResult(* pollerSetReadOnly)(NDef_Context *ctx);                                                                                    /*!< SetReadOnly function pointer                           */
} NDef_Poller_Wrapper;

NDef_Device_Type NDef_GetDeviceType(const RFal_NFC_Device *dev);
NFC_OpResult NDef_Poller_ContextInitialization(NDef_Context *ctx, const RFal_NFC_Device *dev);
NFC_OpResult NDef_Poller_NdefDetect(NDef_Context *ctx, NDef_Info *info);
NFC_OpResult NDef_Poller_ReadBytes(NDef_Context *ctx, uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen);
NFC_OpResult NDef_Poller_WriteBytes(NDef_Context *ctx, uint32_t offset, const uint8_t *buf, uint32_t len);
NFC_OpResult NDef_Poller_ReadRawMessage(NDef_Context *ctx, uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen, bool single);
NFC_OpResult NDef_Poller_WriteRawMessage(NDef_Context *ctx, const uint8_t *buf, uint32_t bufLen);
NFC_OpResult NDef_Poller_TagFormat(NDef_Context *ctx, const NDef_CapabilityContainer *cc, uint32_t options);
NFC_OpResult NDef_Poller_WriteRawMessageLen(NDef_Context *ctx, uint32_t rawMessageLen);
NFC_OpResult NDef_Poller_WriteMessage(NDef_Context *ctx, const NDef_Message *message);
NFC_OpResult NDef_Poller_CheckPresence(NDef_Context *ctx);
NFC_OpResult NDef_Poller_CheckAvailableSpace(const NDef_Context *ctx, uint32_t messageLen);
NFC_OpResult NDef_Poller_BeginWriteMessage(NDef_Context *ctx, uint32_t messageLen);
NFC_OpResult NDef_Poller_EndWriteMessage(NDef_Context *ctx, uint32_t messageLen);
NFC_OpResult NDef_Poller_SetReadOnly(NDef_Context *ctx);

#endif /* NDEF_POLLER_H */
