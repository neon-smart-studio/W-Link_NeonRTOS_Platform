
/**
  ******************************************************************************
  * @file           : ndef_t5t.cpp
  * @brief          : Provides NDEF methods and definitions to access NFC-V Forum T5T
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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "NDef_Poller.h"
#include "NDef_T5T.h"

#include "NFC/RFal/RFal_NFC.h"
#include "NFC/RFal/RFal_NFCV.h"
#include "NFC/RFal/RFal_ST25XV.h"

#include "NFC/NFC_Def.h"

#include "NeonRTOS.h"

#define NDEF_T5T_SYSINFO_MAX_LEN              22U    /*!< Max length for (Extended) Get System Info response */

#define NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR       256U    /*!< Max number of blocks for 1 byte addressing        */

#ifndef NDEF_T5T_N_RETRY_ERROR
  #define NDEF_T5T_N_RETRY_ERROR                2U     /*!< nT5T,RETRY,ERROR DP 2.2  �B.12                    */
#endif /* NDEF_T5T_N_RETRY_ERROR */

#define NDEF_T5T_FLAG_LEN                     1U     /*!< Flag byte length                                  */

#define NDEF_T5T_UID_MANUFACTURER_ID_POS       6U    /*!< Manufacturer ID Offset in UID buffer (reverse)    */
#define NDEF_T5T_MANUFACTURER_ID_ST         0x02U    /*!< Manufacturer ID for ST                            */

#define NDEF_T5T_MLEN_DIVIDER                  8U    /*!<  T5T_area size is measured in bytes is equal to 8 * MLEN */

#define NDEF_T5T_TLV_T_LEN                     1U    /*!< TLV T Length: 1 bytes                             */
#define NDEF_T5T_TLV_L_1_BYTES_LEN             1U    /*!< TLV L Length: 1 bytes                             */
#define NDEF_T5T_TLV_L_3_BYTES_LEN             3U    /*!< TLV L Length: 3 bytes                             */

#define NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR       256U    /*!< Max number of blocks for 1 byte addressing        */
#define NDEF_T5T_MAX_MLEN_1_BYTE_ENCODING    256U    /*!< MLEN max value for 1 byte encoding                */

#define NDEF_T5T_TL_MIN_SIZE  (NDEF_T5T_TLV_T_LEN \
                       + NDEF_T5T_TLV_L_1_BYTES_LEN) /*!< Min TL size                                       */

#define NDEF_T5T_TL_MAX_SIZE  (NDEF_T5T_TLV_T_LEN \
                       + NDEF_T5T_TLV_L_3_BYTES_LEN) /*!< Max TL size                                       */

#define NDEF_T5T_TLV_NDEF                   0x03U    /*!< TLV flag NDEF value                               */
#define NDEF_T5T_TLV_PROPRIETARY            0xFDU    /*!< TLV flag PROPRIETARY value                        */
#define NDEF_T5T_TLV_TERMINATOR             0xFEU    /*!< TLV flag TERMINATOR value                         */
#define NDEF_T5T_TLV_RFU                    0x00U    /*!< TLV flag RFU value                                */

#define NDEF_T5T_ACCESS_ALWAYS               0x0U    /*!< Read/Write Access. 00b: Always                    */
#define NDEF_T5T_ACCESS_RFU                  0x1U    /*!< Read/Write Access. 01b: RFU                       */
#define NDEF_T5T_ACCESS_PROPRIETARY          0x2U    /*!< Read/Write Access. 00b: Proprietary               */
#define NDEF_T5T_ACCESS_NEVER                0x3U    /*!< Read/Write Access. 00b: Never                     */

#define NDef_T5T_IsTransmissionError(err)      ( ((err) == NFC_FramingError) || ((err) == NFC_CRC_Error) || ((err) == NFC_ParityError) || ((err) == NFC_SlaveTimeout) )

static NDef_T5T_AccessMode gAccessMode = NDEF_T5T_ACCESS_MODE_SELECTED;

static NFC_OpResult NDef_T5T_Poller_ReadSingleBlock(NDef_Context *ctx, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
static NFC_OpResult NDef_T5T_Poller_ReadMultipleBlocks(NDef_Context *ctx, uint16_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
static NFC_OpResult NDef_T5T_GetSystemInformation(NDef_Context *ctx, bool extended);
static NFC_OpResult NDef_T5T_Poller_WriteSingleBlock(NDef_Context *ctx, uint16_t blockNum, const uint8_t *wrData);
static NFC_OpResult NDef_T5T_Poller_LockSingleBlock(NDef_Context *ctx, uint16_t blockNum);
static NFC_OpResult NDef_T5T_WriteCC(NDef_Context *ctx);
bool NDef_T5T_isSTDevice(const RFal_NFC_Device *dev)
{
  if (dev == NULL) {
    return false;
  }

  return (dev->dev.nfcv.InvRes.UID[NDEF_T5T_UID_MANUFACTURER_ID_POS] == NDEF_T5T_MANUFACTURER_ID_ST);
}


/*******************************************************************************/
bool NDef_T5T_isT5TDevice(const RFal_NFC_Device *dev)
{
  if (dev == NULL) {
    return false;
  }

  return dev->type == RFAL_NFC_LISTEN_TYPE_NFCV;
}


/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_AccessMode(NDef_Context *ctx, const RFal_NFC_Device *dev, NDef_T5T_AccessMode mode)
{
  NDef_T5T_AccessMode accessMode = mode;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T) ||
      (dev == NULL)) {
    return NFC_InvalidParameter;
  }

  ctx->subCtx.t5t.flags = (uint8_t)RFAL_NFCV_REQ_FLAG_DEFAULT;

  if (accessMode == NDEF_T5T_ACCESS_MODE_SELECTED) {
    if (RFal_NFCV_PollerSelect(ctx->subCtx.t5t.flags, dev->dev.nfcv.InvRes.UID) == NFC_OK) {
      /* Selected mode (AMS = 0, SMS = 1) */
      ctx->subCtx.t5t.uid    = NULL;
      ctx->subCtx.t5t.flags |= (uint8_t)RFAL_NFCV_REQ_FLAG_SELECT;
    } else {
      /* Set Addressed mode if Selected mode failed */
      accessMode = NDEF_T5T_ACCESS_MODE_ADDRESSED;
    }
  }
  if (accessMode == NDEF_T5T_ACCESS_MODE_ADDRESSED) {
    /* Addressed mode (AMS = 1, SMS = 0) */
    ctx->subCtx.t5t.uid    = dev->dev.nfcv.InvRes.UID;
    ctx->subCtx.t5t.flags |= (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS;
  } else if (accessMode == NDEF_T5T_ACCESS_MODE_NON_ADDRESSED) {
    /* Non-addressed mode (AMS = 0, SMS = 0) */
    ctx->subCtx.t5t.uid = NULL;
  } else {
    /* MISRA 15.7 - Empty else */
  }

  return NFC_OK;
}


/*******************************************************************************/
uint8_t NDef_T5T_GetBlockLength(NDef_Context *ctx)
{
  NFC_OpResult    result;
  uint16_t      rcvLen;
  uint8_t       blockLen = 0;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return 0;
  }

  /* GetBlockLength shall be called only once during context initialization */
  if (ctx->subCtx.t5t.blockLen == 0U) {
    /* T5T v1.1 4.1.1.3 Retrieve the Block Length */
    ctx->subCtx.t5t.legacySTHighDensity = false;
    result = NDef_T5T_Poller_ReadSingleBlock(ctx, 0U, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &rcvLen);
    if ((result != NFC_OK) && ctx->subCtx.t5t.stDevice) {
      /* Try High Density Legacy mode */
      ctx->subCtx.t5t.legacySTHighDensity = true;
      result = NDef_T5T_Poller_ReadSingleBlock(ctx, 0U, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &rcvLen);
      if (result != NFC_OK) {
        /* High Density Legacy mode not supported */
        ctx->subCtx.t5t.legacySTHighDensity = false;
        return 0;
      }
    }

    if ((rcvLen > 1U) && (ctx->subCtx.t5t.txrxBuf[0U] == (uint8_t)0U)) {
      blockLen = (uint8_t)(rcvLen - 1U);
    }
  }

  return blockLen;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_GetMemoryConfig(NDef_Context *ctx)
{
  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  if (!ctx->subCtx.t5t.legacySTHighDensity) {
    /* Extended Get System Info */
    if (NDef_T5T_GetSystemInformation(ctx, true) == NFC_OK) {
      ctx->subCtx.t5t.sysInfoSupported = true;
    }
  }
  if (!ctx->subCtx.t5t.sysInfoSupported) {
    /* Get System Info */
    if (NDef_T5T_GetSystemInformation(ctx, false) == NFC_OK) {
      ctx->subCtx.t5t.sysInfoSupported = true;
    }
  }

  return NFC_OK;
}


/*******************************************************************************/
static NFC_OpResult NDef_T5T_GetSystemInformation(NDef_Context *ctx, bool extended)
{
  NFC_OpResult                ret;
  uint8_t                   rxBuf[NDEF_T5T_SYSINFO_MAX_LEN];
  uint16_t                  rcvLen;
  uint8_t                  *resp;
  uint8_t                   flags;
  const uint8_t            *uid;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  uid   = ctx->subCtx.t5t.uid;
  flags = ctx->subCtx.t5t.flags;

  if (extended) {
    ret = RFal_NFCV_PollerExtendedGetSystemInformation(flags, uid, (uint8_t)RFAL_NFCV_SYSINFO_REQ_ALL, rxBuf, (uint16_t)sizeof(rxBuf), &rcvLen);
  } else {
    if (ctx->subCtx.t5t.legacySTHighDensity) {
      flags |= (uint8_t)RFAL_NFCV_REQ_FLAG_PROTOCOL_EXT;
    }
    ret = RFal_NFCV_PollerGetSystemInformation(flags, uid, rxBuf, (uint16_t)sizeof(rxBuf), &rcvLen);
  }

  if (ret != NFC_OK) {
    return ret;
  }

  resp = &rxBuf[0U];
  /* skip Flags */
  resp++;
  /* get Info flags */
  ctx->subCtx.t5t.sysInfo.infoFlags = *resp;
  resp++;
  if (extended && (NDef_T5T_SysInfoLenValue(ctx->subCtx.t5t.sysInfo.infoFlags) != 0U)) {
    return NFC_ProtocolError;
  }
  /* get UID */
  (void)memcpy(ctx->subCtx.t5t.sysInfo.UID, resp, RFAL_NFCV_UID_LEN);
  resp = &resp[RFAL_NFCV_UID_LEN];
  if (NDef_T5T_SysInfoDFSIDPresent(ctx->subCtx.t5t.sysInfo.infoFlags) != 0U) {
    ctx->subCtx.t5t.sysInfo.DFSID = *resp;
    resp++;
  }
  if (NDef_T5T_SysInfoAFIPresent(ctx->subCtx.t5t.sysInfo.infoFlags) != 0U) {
    ctx->subCtx.t5t.sysInfo.AFI = *resp;
    resp++;
  }
  if (NDef_T5T_SysInfoMemSizePresent(ctx->subCtx.t5t.sysInfo.infoFlags) != 0U) {
    if (ctx->subCtx.t5t.legacySTHighDensity || extended) {
      /* LRIS64K/M24LR16/M24LR64 */
      ctx->subCtx.t5t.sysInfo.numberOfBlock = *resp;
      resp++;
      ctx->subCtx.t5t.sysInfo.numberOfBlock |= (((uint16_t) * resp) << 8U);
      resp++;
    } else {
      ctx->subCtx.t5t.sysInfo.numberOfBlock = *resp;
      resp++;
    }
    ctx->subCtx.t5t.sysInfo.blockSize = *resp;
    resp++;
    /* Add 1 to get real values*/
    ctx->subCtx.t5t.sysInfo.numberOfBlock++;
    ctx->subCtx.t5t.sysInfo.blockSize++;
  }
  if (NDef_T5T_SysInfoICRefPresent(ctx->subCtx.t5t.sysInfo.infoFlags) != 0U) {
    ctx->subCtx.t5t.sysInfo.ICRef = *resp;
    resp++;
  }
  if (extended && (NDef_T5T_SysInfoCmdListPresent(ctx->subCtx.t5t.sysInfo.infoFlags) != 0U)) {
    ctx->subCtx.t5t.sysInfo.supportedCmd[0U] = *resp;
    resp++;
    ctx->subCtx.t5t.sysInfo.supportedCmd[1U] = *resp;
    resp++;
    ctx->subCtx.t5t.sysInfo.supportedCmd[2U] = *resp;
    resp++;
    ctx->subCtx.t5t.sysInfo.supportedCmd[3U] = *resp;
    resp++;
  }
  return NFC_OK;
}

/*******************************************************************************/
bool NDef_T5T_IsMultipleBlockReadSupported(NDef_Context *ctx)
{
  NFC_OpResult result;
  uint16_t   rcvdLen;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return false;
  }

  /* Autodetect the Multiple Block Read feature (CC Byte 3 b0: MBREAD) */
  result = NDef_T5T_Poller_ReadMultipleBlocks(ctx, 0U, 0U, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &rcvdLen);
  return (result == NFC_OK);
}


/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_ReadBytes(NDef_Context *ctx, uint32_t offset, uint32_t len, uint8_t *buf, uint32_t *rcvdLen)
{
  uint8_t         lastVal;
  uint16_t        res;
  uint16_t        nbRead;
  uint16_t        blockLen;
  uint16_t        startBlock;
  uint16_t        startAddr;
  uint32_t        currentLen = len;
  uint32_t        lvRcvLen   = 0U;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T) || (buf == NULL)) {
    return NFC_InvalidParameter;
  }

  if ((ctx->subCtx.t5t.blockLen > 0U) && (len > 0U)) {
    blockLen = (uint16_t)ctx->subCtx.t5t.blockLen;
    if (blockLen == 0U) {
      return NFC_System;
    }
    startBlock = (uint16_t)(offset / blockLen);
    startAddr  = (uint16_t)(startBlock * blockLen);

    res = ((ctx->cc.t5t.multipleBlockRead == true) && (ctx->subCtx.t5t.useMultipleBlockRead == true)) ?
          /* Read a single block using the ReadMultipleBlock command... */
          NDef_T5T_Poller_ReadMultipleBlocks(ctx, startBlock, 0U, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &nbRead) :
          NDef_T5T_Poller_ReadSingleBlock(ctx, startBlock, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &nbRead);
    if (res < NFC_OK) {
      return res;
    }

    nbRead = (uint16_t)(nbRead  + startAddr - (uint16_t)offset - 1U);
    if ((uint32_t) nbRead > currentLen) {
      nbRead = (uint16_t) currentLen;
    }
    if (nbRead > 0U) {
      /* Remove the Flag byte */
      (void)memcpy(buf, &ctx->subCtx.t5t.txrxBuf[1U - startAddr + (uint16_t)offset], nbRead);
    }
    lvRcvLen   += (uint32_t)nbRead;
    currentLen -= (uint32_t)nbRead;
    /* Process all blocks but not the last one */
    /* Rationale: NDef_T5T_Poller_ReadSingleBlock() reads 2 extra CRC bytes and could write after buffer end */
    while (currentLen > (uint32_t)blockLen) {
      startBlock++;
      lastVal = buf[lvRcvLen - 1U]; /* Read previous value that is going to be overwritten by status byte (1st byte in response) */

      res = ((ctx->cc.t5t.multipleBlockRead == true) && (ctx->subCtx.t5t.useMultipleBlockRead == true)) ?
            /* Read a single block using the ReadMultipleBlock command... */
            NDef_T5T_Poller_ReadMultipleBlocks(ctx, startBlock, 0U, &buf[lvRcvLen - 1U], blockLen + NDEF_T5T_FLAG_LEN + RFAL_CRC_LEN, &nbRead) :
            NDef_T5T_Poller_ReadSingleBlock(ctx, startBlock, &buf[lvRcvLen - 1U], blockLen + NDEF_T5T_FLAG_LEN + RFAL_CRC_LEN, &nbRead);
      if (res < NFC_OK) {
        return res;
      }

      buf[lvRcvLen - 1U] = lastVal; /* Restore previous value */

      lvRcvLen   += blockLen;
      currentLen -= blockLen;
    }
    if (currentLen > 0U) {
      /* Process the last block. Take care of removing status byte and 2 extra CRC bytes that could write after buffer end */
      startBlock++;

      res = ((ctx->cc.t5t.multipleBlockRead == true) && (ctx->subCtx.t5t.useMultipleBlockRead == true)) ?
            /* Read a single block using the ReadMultipleBlock command... */
            NDef_T5T_Poller_ReadMultipleBlocks(ctx, startBlock, 0U, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &nbRead) :
            NDef_T5T_Poller_ReadSingleBlock(ctx, startBlock, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &nbRead);
      if (res < NFC_OK) {
        return res;
      }

      nbRead--; /* Remove Flag byte */
      if (nbRead > currentLen) {
        nbRead = (uint16_t)currentLen;
      }
      if (nbRead > 0U) {
        (void)memcpy(&buf[lvRcvLen], & ctx->subCtx.t5t.txrxBuf[1U], nbRead);
      }
      lvRcvLen   += nbRead;
      currentLen -= nbRead;
    }
  }
  if (currentLen != 0U) {
    return NFC_System;
  }
  if (rcvdLen != NULL) {
    *rcvdLen = lvRcvLen;
  }
  return NFC_OK;
}


/*******************************************************************************/
static NFC_OpResult NDef_T5T_Poller_ReadSingleBlock(NDef_Context *ctx, uint16_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  NFC_OpResult                ret;
  uint8_t                   flags;
  const uint8_t            *uid;
  uint32_t                  retry;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T) || (rxBuf == NULL) || (rcvLen == NULL)) {
    return NFC_InvalidParameter;
  }

  if (NDef_T5T_IsValidCache(ctx, blockNum)) {
    /* Retrieve data from cache */
    (void)memcpy(rxBuf, ctx->subCtx.t5t.cacheBuf, NDEF_T5T_TxRx_BUFF_HEADER_SIZE + (uint32_t)ctx->subCtx.t5t.blockLen);
    *rcvLen = (uint16_t)NDEF_T5T_TxRx_BUFF_HEADER_SIZE + ctx->subCtx.t5t.blockLen;

    return NFC_OK;
  }

  uid   = ctx->subCtx.t5t.uid;
  flags = ctx->subCtx.t5t.flags;

  retry = NDEF_T5T_N_RETRY_ERROR;
  do {
    if (ctx->subCtx.t5t.legacySTHighDensity) {
      ret = RFal_ST25XV_PollerM24LRReadSingleBlock(flags, uid, blockNum, rxBuf, rxBufLen, rcvLen);
    } else {
      if (blockNum < NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR) {
        ret = RFal_NFCV_PollerReadSingleBlock(flags, uid, (uint8_t)blockNum, rxBuf, rxBufLen, rcvLen);
      } else {
        ret = RFal_NFCV_PollerExtendedReadSingleBlock(flags, uid, blockNum, rxBuf, rxBufLen, rcvLen);
      }
    }
  } while ((retry-- != 0U) && NDef_T5T_IsTransmissionError(ret));

  if (ret == NFC_OK) {
    /* Update cache */
    if (*rcvLen > 0U) {
      (void)memcpy(ctx->subCtx.t5t.cacheBuf, rxBuf, *rcvLen);
    }
    ctx->subCtx.t5t.cacheBlock = blockNum;
  }

  return ret;
}

/*******************************************************************************/
static NFC_OpResult NDef_T5T_Poller_ReadMultipleBlocks(NDef_Context *ctx, uint16_t firstBlockNum, uint8_t numOfBlocks, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen)
{
  NFC_OpResult                ret;
  uint8_t                   flags;
  const uint8_t            *uid;
  uint32_t                  retry;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  uid   = ctx->subCtx.t5t.uid;
  flags = ctx->subCtx.t5t.flags;

  /* 5.5 The number of data blocks returned by the Type 5 Tag in its response is (NB +1)
     e.g. NumOfBlocks = 0 means reading 1 block */

  retry = NDEF_T5T_N_RETRY_ERROR;
  do {
    if (ctx->subCtx.t5t.legacySTHighDensity) {
      ret = RFal_ST25XV_PollerM24LRReadMultipleBlocks(flags, uid, firstBlockNum, numOfBlocks, rxBuf, rxBufLen, rcvLen);
    } else {
      if (firstBlockNum < NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR) {
        ret = RFal_NFCV_PollerReadMultipleBlocks(flags, uid, (uint8_t)firstBlockNum, numOfBlocks, rxBuf, rxBufLen, rcvLen);
      } else {
        ret = RFal_NFCV_PollerExtendedReadMultipleBlocks(flags, uid, firstBlockNum, numOfBlocks, rxBuf, rxBufLen, rcvLen);
      }
    }
  } while ((retry-- != 0U) && NDef_T5T_IsTransmissionError(ret));

  return ret;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_WriteBytes(NDef_Context *ctx, uint32_t offset, const uint8_t *buf, uint32_t len, bool pad, bool writeTerminator)
{
  NFC_OpResult      res;
  uint16_t        nbRead;
  uint16_t        blockLen;
  uint16_t        startBlock;
  uint16_t        startAddr;
  const uint8_t  *wrbuf      = buf;
  uint32_t        currentLen = len;
  bool            lvWriteTerminator = writeTerminator;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T) || (len == 0U) || (ctx->subCtx.t5t.blockLen == 0U)) {
    return NFC_InvalidParameter;
  }
  blockLen = (uint16_t)ctx->subCtx.t5t.blockLen;
  if (blockLen == 0U) {
    return NFC_System;
  }
  startBlock = (uint16_t)(offset     / blockLen);
  startAddr  = (uint16_t)(startBlock * blockLen);

  if (startAddr != offset) {
    /* Unaligned start offset must read the first block before */
    res = NDef_T5T_Poller_ReadSingleBlock(ctx, startBlock, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &nbRead);
    if (res < NFC_OK) {
      return res;
    }
    if (nbRead != (blockLen + 1U)) {
      return NFC_ProtocolError;
    }
    nbRead = (uint16_t)((uint32_t)nbRead - 1U  + startAddr - offset);
    if (nbRead > (uint32_t)currentLen) {
      nbRead = (uint16_t)currentLen;
    }
    if (nbRead > 0U) {
      (void)memcpy(&ctx->subCtx.t5t.txrxBuf[offset - startAddr + 1U], wrbuf, nbRead);
    }
    if ((offset - startAddr + nbRead) < blockLen) {
      if (pad) {
        (void)memset(&ctx->subCtx.t5t.txrxBuf[offset - startAddr + nbRead + 1U], 0x00, blockLen - (offset - startAddr + nbRead));
      }
      if (lvWriteTerminator) {
        ctx->subCtx.t5t.txrxBuf[offset - startAddr + nbRead + 1U] = NDEF_TERMINATOR_TLV_T;
        lvWriteTerminator = false;
      }
    }
    res = NDef_T5T_Poller_WriteSingleBlock(ctx, startBlock, &ctx->subCtx.t5t.txrxBuf[1U]);
    if (res < NFC_OK) {
      return res;
    }
    currentLen -= nbRead;
    wrbuf       = &wrbuf[nbRead];
    startBlock++;
  }
  while (currentLen >= blockLen) {
    res = NDef_T5T_Poller_WriteSingleBlock(ctx, startBlock, wrbuf);
    if (res < NFC_OK) {
      return res;
    }
    currentLen -= blockLen;
    wrbuf       = &wrbuf[blockLen];
    startBlock++;
  }
  if (currentLen != 0U) {
    if (pad) {
      (void)memset(ctx->subCtx.t5t.txrxBuf, 0, (uint32_t)blockLen + 1U);
    } else {
      /* Unaligned end, must read the existing block before, except if padding  */
      res = NDef_T5T_Poller_ReadSingleBlock(ctx, startBlock, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &nbRead);
      if (res < NFC_OK) {
        return res;
      }
      if (nbRead != (blockLen + 1U)) {
        return NFC_ProtocolError;
      }
    }
    /* MISRA: PRQA requires to check the length to copy, IAR doesn't */
    if (currentLen > 0U) {
      (void)memcpy(&ctx->subCtx.t5t.txrxBuf[1U], wrbuf, currentLen);
    }
    if (lvWriteTerminator) {
      ctx->subCtx.t5t.txrxBuf[1U + currentLen] = NDEF_TERMINATOR_TLV_T;
      lvWriteTerminator = false;
    }
    res = NDef_T5T_Poller_WriteSingleBlock(ctx, startBlock, &ctx->subCtx.t5t.txrxBuf[1U]);
    if (res < NFC_OK) {
      return res;
    }
  }
  if (lvWriteTerminator) {
    (void)memset(ctx->subCtx.t5t.txrxBuf, 0, (uint32_t)blockLen + 1U);
    ctx->subCtx.t5t.txrxBuf[1U] = NDEF_TERMINATOR_TLV_T;
    (void)NDef_T5T_Poller_WriteSingleBlock(ctx, startBlock, &ctx->subCtx.t5t.txrxBuf[1U]);
  }
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_IsDevicePresent(NDef_Context *ctx)
{
  NFC_OpResult          ret;
  uint16_t            blockAddr;
  uint16_t            rcvLen;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  NDef_T5T_InvalidateCache(ctx);

  blockAddr = 0U;

  ret = NDef_T5T_Poller_ReadSingleBlock(ctx, blockAddr, ctx->subCtx.t5t.txrxBuf, (uint16_t)sizeof(ctx->subCtx.t5t.txrxBuf), &rcvLen);

  return ret;
}

/*******************************************************************************/
static NFC_OpResult NDef_T5T_Poller_WriteSingleBlock(NDef_Context *ctx, uint16_t blockNum, const uint8_t *wrData)
{
  NFC_OpResult                ret;
  uint8_t                   flags;
  const uint8_t            *uid;
  uint32_t                  retry;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  uid   = ctx->subCtx.t5t.uid;
  flags = ctx->subCtx.t5t.flags;
  if (ctx->cc.t5t.specialFrame) {
    flags |= (uint8_t)RFAL_NFCV_REQ_FLAG_OPTION;
  }

  NDef_T5T_InvalidateCache(ctx);

  retry = NDEF_T5T_N_RETRY_ERROR;
  do {
    if (ctx->subCtx.t5t.legacySTHighDensity) {
      ret = RFal_ST25XV_PollerM24LRWriteSingleBlock(flags, uid, blockNum, wrData, ctx->subCtx.t5t.blockLen);
    } else {
      if (blockNum < NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR) {
        ret = RFal_NFCV_PollerWriteSingleBlock(flags, uid, (uint8_t)blockNum, wrData, ctx->subCtx.t5t.blockLen);
      } else {
        ret = RFal_NFCV_PollerExtendedWriteSingleBlock(flags, uid, blockNum, wrData, ctx->subCtx.t5t.blockLen);
      }
    }
  } while ((retry-- != 0U) && NDef_T5T_IsTransmissionError(ret));

  return ret;
}

/*******************************************************************************/
static NFC_OpResult NDef_T5T_Poller_LockSingleBlock(NDef_Context *ctx, uint16_t blockNum)
{
  NFC_OpResult                ret;
  uint8_t                   flags;
  const uint8_t            *uid;
  uint32_t                  retry;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  uid   = ctx->subCtx.t5t.uid;
  flags = ctx->subCtx.t5t.flags;
  if (ctx->cc.t5t.specialFrame) {
    flags |= (uint8_t)RFAL_NFCV_REQ_FLAG_OPTION;
  }

  retry = NDEF_T5T_N_RETRY_ERROR;
  do {
    if (blockNum < NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR) {
      ret = RFal_NFCV_PollerLockBlock(flags, uid, (uint8_t)blockNum);
    } else {
      ret = RFal_NFCV_PollerExtendedLockSingleBlock(flags, uid, blockNum);
    }
  } while ((retry-- != 0U) && NDef_T5T_IsTransmissionError(ret));

  return ret;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_LockDevice(NDef_Context *ctx)
{
  NFC_OpResult ret;
  uint32_t   numBlocks;
  uint16_t   i;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  ctx->state = NDEF_STATE_READONLY;
  numBlocks = (ctx->areaLen + (uint32_t)ctx->cc.t5t.ccLen) / (uint32_t)ctx->subCtx.t5t.blockLen;
  if (ctx->cc.t5t.lockBlock && !ctx->subCtx.t5t.legacySTHighDensity) {
    for (i = 0; i < numBlocks; i++) {
      ret = NDef_T5T_Poller_LockSingleBlock(ctx, i);
      if (ret != NFC_OK) {
        return ret;
      }
    }
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_SetAccessMode(NDef_T5T_AccessMode mode)
{
  gAccessMode = mode;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_MultipleBlockRead(NDef_Context *ctx, bool enable)
{
  if ((ctx == NULL) || (ctx->state != NDEF_STATE_INVALID)) {
    return NFC_InvalidParameter;
  }

  ctx->subCtx.t5t.useMultipleBlockRead = enable;

  return NFC_OK;
}

/*******************************************************************************/
static NFC_OpResult NDef_T5T_ReadLField(NDef_Context *ctx)
{
  NFC_OpResult           ret;
  uint32_t             offset;
  uint8_t              data[3];
  uint16_t             lenTLV;

  ctx->state = NDEF_STATE_INVALID;
  offset = ctx->subCtx.t5t.TlvNDEFOffset;
  offset++;
  ret = NDef_T5T_Poller_ReadBytes(ctx, offset, 1, data, NULL);
  if (ret != NFC_OK) {
    /* Conclude procedure */
    return ret;
  }
  offset++;
  lenTLV = data[0];
  if (lenTLV == (NDEF_SHORT_VFIELD_MAX_LEN + 1U)) {
    ret = NDef_T5T_Poller_ReadBytes(ctx, offset, 2, data, NULL);
    if (ret != NFC_OK) {
      /* Conclude procedure */
      return ret;
    }
    offset += 2U;
    lenTLV = (((uint16_t)(&data[0])[0] << 8) | (uint16_t)(&data[0])[1]);
  }
  ctx->messageLen    = lenTLV;
  ctx->messageOffset = offset;

  if (lenTLV == 0U) {
    if (!((ctx->cc.t5t.readAccess  == NDEF_T5T_ACCESS_ALWAYS) && (ctx->cc.t5t.writeAccess == NDEF_T5T_ACCESS_ALWAYS))) {
      /* Conclude procedure  */
      return NFC_RequestError;
    }
    ctx->state = NDEF_STATE_INITIALIZED;
  } else {
    if (!(ctx->cc.t5t.readAccess == NDEF_T5T_ACCESS_ALWAYS)) {
      /* Conclude procedure  */
      return NFC_RequestError;
    }
    ctx->state = (ctx->cc.t5t.writeAccess == NDEF_T5T_ACCESS_ALWAYS) ? NDEF_STATE_READWRITE : NDEF_STATE_READONLY;
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_ContextInitialization(NDef_Context *ctx, const RFal_NFC_Device *dev)
{
  NFC_OpResult    result;

  if ((ctx == NULL) || (dev == NULL)) {
    return NFC_InvalidParameter;
  }
  if (!NDef_T5T_isT5TDevice(dev)) {
    return NFC_InvalidParameter;
  }

  (void)memcpy(&ctx->device, dev, sizeof(ctx->device));

  NDef_T5T_InvalidateCache(ctx);

  /* Reset info about the card */
  ctx->type                     = NDEF_DEV_T5T;
  ctx->state                    = NDEF_STATE_INVALID;

  /* Initialize CC fields, used in NDEF detect */
  ctx->cc.t5t.ccLen             = 0U;
  ctx->cc.t5t.magicNumber       = 0U;
  ctx->cc.t5t.majorVersion      = 0U;
  ctx->cc.t5t.minorVersion      = 0U;
  ctx->cc.t5t.readAccess        = 0U;
  ctx->cc.t5t.writeAccess       = 0U;
  ctx->cc.t5t.memoryLen         = 0U;
  ctx->cc.t5t.specialFrame      = false;
  ctx->cc.t5t.lockBlock         = false;
  ctx->cc.t5t.mlenOverflow      = false;
  ctx->cc.t5t.multipleBlockRead = false;

  ctx->subCtx.t5t.blockLen      = 0U;
  ctx->subCtx.t5t.TlvNDEFOffset = 0U; /* Offset for TLV */
  ctx->subCtx.t5t.useMultipleBlockRead = false;

  NDef_T5T_Poller_AccessMode(ctx, dev, gAccessMode);

  ctx->subCtx.t5t.stDevice = NDef_T5T_isSTDevice(dev);

  /* Get block length, and set subCtx.t5t.legacySTHighDensity */
  ctx->subCtx.t5t.blockLen = NDef_T5T_GetBlockLength(ctx);
  if (ctx->subCtx.t5t.blockLen == 0U) {
    return NFC_ProtocolError;
  }

  ctx->subCtx.t5t.sysInfoSupported = false;

  result = NDef_T5T_GetMemoryConfig(ctx);
  if (result != NFC_OK) {
    return result;
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_NdefDetect(NDef_Context *ctx, NDef_Info *info)
{
  NFC_OpResult result;
  uint8_t    tmpBuf[NDEF_T5T_TL_MAX_SIZE];
  NFC_OpResult returnCode = NFC_RequestError; /* Default return code */
  uint16_t   offset;
  uint16_t   length;
  uint32_t   TlvOffset;
  bool       exit;
  uint32_t   rcvLen;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  ctx->state                           = NDEF_STATE_INVALID;
  ctx->cc.t5t.ccLen                    = 0U;
  ctx->cc.t5t.memoryLen                = 0U;
  ctx->cc.t5t.multipleBlockRead        = false;
  ctx->messageLen                      = 0U;
  ctx->messageOffset                   = 0U;
  ctx->areaLen                         = 0U;

  if (info != NULL) {
    info->state                = NDEF_STATE_INVALID;
    info->majorVersion         = 0U;
    info->minorVersion         = 0U;
    info->areaLen              = 0U;
    info->areaAvalableSpaceLen = 0U;
    info->messageLen           = 0U;
  }

  result = NDef_T5T_Poller_ReadBytes(ctx, 0U, 4U, ctx->ccBuf, &rcvLen);
  if ((result == NFC_OK) && (rcvLen == 4U) && ((ctx->ccBuf[0] == (uint8_t)0xE1U) || (ctx->ccBuf[0] == (uint8_t)0xE2U))) {
    ctx->cc.t5t.magicNumber           =  ctx->ccBuf[0U];
    ctx->cc.t5t.majorVersion          = (ctx->ccBuf[1U] >> 6U) & 0x03U;
    ctx->cc.t5t.minorVersion          = (ctx->ccBuf[1U] >> 4U) & 0x03U;
    ctx->cc.t5t.readAccess            = (ctx->ccBuf[1U] >> 2U) & 0x03U;
    ctx->cc.t5t.writeAccess           = (ctx->ccBuf[1U] >> 0U) & 0x03U;
    ctx->cc.t5t.memoryLen             =  ctx->ccBuf[2U];
    ctx->cc.t5t.specialFrame          = (((ctx->ccBuf[3U] >> 4U) & 0x01U) != 0U);
    ctx->cc.t5t.lockBlock             = (((ctx->ccBuf[3U] >> 3U) & 0x01U) != 0U);
    ctx->cc.t5t.mlenOverflow          = (((ctx->ccBuf[3U] >> 2U) & 0x01U) != 0U);
    /* Read the CC with Single Block Read command(s) and update multipleBlockRead flag after */
    ctx->state                        = NDEF_STATE_INITIALIZED;

    /* Check Magic Number TS T5T v1.0 - 7.5.1.2 */
    if ((ctx->cc.t5t.magicNumber != NDEF_T5T_CC_MAGIC_1_BYTE_ADDR_MODE) &&
        (ctx->cc.t5t.magicNumber != NDEF_T5T_CC_MAGIC_2_BYTE_ADDR_MODE)) {
      return NFC_RequestError;
    }

    /* Check version - 7.5.1.2 */
    if (ctx->cc.t5t.majorVersion > NDef_T5T_MajorVersion(NDEF_T5T_MAPPING_VERSION_1_0)) {
      return NFC_RequestError;
    }

    /* Check read access - 7.5.1.2 */
    if (ctx->cc.t5t.readAccess != NDEF_T5T_ACCESS_ALWAYS) {
      return NFC_RequestError;
    }

    if (ctx->cc.t5t.memoryLen != 0U) {
      /* 4-byte CC */
      ctx->cc.t5t.ccLen         = NDEF_T5T_CC_LEN_4_BYTES;
      if ((ctx->cc.t5t.memoryLen == 0xFFU) && ctx->cc.t5t.mlenOverflow) {
        if ((ctx->subCtx.t5t.sysInfoSupported == true) && (NDef_T5T_SysInfoMemSizePresent(ctx->subCtx.t5t.sysInfo.infoFlags) != 0U)) {
          ctx->cc.t5t.memoryLen = (uint16_t)((ctx->subCtx.t5t.sysInfo.numberOfBlock * ctx->subCtx.t5t.sysInfo.blockSize) / NDEF_T5T_MLEN_DIVIDER);
        }
      }
    } else {
      /* 8-byte CC */
      result = NDef_T5T_Poller_ReadBytes(ctx, 4U, 4U, &ctx->ccBuf[4U], &rcvLen);
      if ((result == NFC_OK) && (rcvLen == 4U)) {
        ctx->cc.t5t.ccLen     = NDEF_T5T_CC_LEN_8_BYTES;
        ctx->cc.t5t.memoryLen = ((uint16_t)ctx->ccBuf[6U] << 8U) + (uint16_t)ctx->ccBuf[7U];
      }
    }

    /* Update multipleBlockRead flag after having read the second half of 8-byte CC */
    ctx->cc.t5t.multipleBlockRead     = (((ctx->ccBuf[3U] >> 0U) & 0x01U) != 0U);

    if ((ctx->subCtx.t5t.sysInfoSupported == true) &&
        (NDef_T5T_SysInfoMemSizePresent(ctx->subCtx.t5t.sysInfo.infoFlags) != 0U) &&
        (ctx->cc.t5t.memoryLen == (uint16_t)((ctx->subCtx.t5t.sysInfo.numberOfBlock * ctx->subCtx.t5t.sysInfo.blockSize) / NDEF_T5T_MLEN_DIVIDER)) &&
        (ctx->cc.t5t.memoryLen > 0U)) {
      ctx->cc.t5t.memoryLen--; /* remove CC area from memory length */
    }

    ctx->messageLen     = 0U;
    ctx->messageOffset  = ctx->cc.t5t.ccLen;
    /* TS T5T v1.0 4.3.1.17 T5T_Area size is measured in bytes, is equal to MLEN * 8 */
    ctx->areaLen        = (uint32_t)ctx->cc.t5t.memoryLen * NDEF_T5T_MLEN_DIVIDER;

    TlvOffset = ctx->cc.t5t.ccLen;
    exit      = false;
    while ((exit == false) && (TlvOffset < (ctx->cc.t5t.ccLen + ctx->areaLen))) {
      result = NDef_T5T_Poller_ReadBytes(ctx, TlvOffset, NDEF_T5T_TL_MIN_SIZE, tmpBuf, &rcvLen);
      if ((result != NFC_OK) || (rcvLen != NDEF_T5T_TL_MIN_SIZE)) {
        return result;
      }
      offset = NDEF_T5T_TLV_T_LEN + NDEF_T5T_TLV_L_1_BYTES_LEN;
      length = tmpBuf[1U];
      if (length == (NDEF_SHORT_VFIELD_MAX_LEN + 1U)) {
        /* Size is encoded in 1 + 2 bytes */
        result = NDef_T5T_Poller_ReadBytes(ctx, TlvOffset, NDEF_T5T_TL_MAX_SIZE, tmpBuf, &rcvLen);
        if ((result != NFC_OK) || (rcvLen != NDEF_T5T_TL_MAX_SIZE)) {
          return result;
        }
        length = (((uint16_t)tmpBuf[2U]) << 8U) + (uint16_t)tmpBuf[3U];
        offset += 2U;
      }
      if (tmpBuf[0U] == (uint8_t)NDEF_T5T_TLV_NDEF) {
        /* NDEF record return it */
        returnCode                    = NFC_OK;  /* Default */
        ctx->subCtx.t5t.TlvNDEFOffset = TlvOffset; /* Offset for TLV */
        ctx->messageOffset            = TlvOffset + offset;
        ctx->messageLen               = length;
        if (length == 0U) {
          /* Req 40 7.5.1.6 */
          if ((ctx->cc.t5t.readAccess  == NDEF_T5T_ACCESS_ALWAYS) &&
              (ctx->cc.t5t.writeAccess == NDEF_T5T_ACCESS_ALWAYS)) {
            ctx->state = NDEF_STATE_INITIALIZED;
          } else {
            ctx->state = NDEF_STATE_INVALID;
            returnCode = NFC_RequestError; /* Default */
          }
          exit = true;
        } else {
          if (ctx->cc.t5t.readAccess == NDEF_T5T_ACCESS_ALWAYS) {
            if (ctx->cc.t5t.writeAccess == NDEF_T5T_ACCESS_ALWAYS) {
              ctx->state = NDEF_STATE_READWRITE;
            } else {
              ctx->state = NDEF_STATE_READONLY;
            }
          }
          exit = true;
        }
      } else if (tmpBuf[0U] == (uint8_t) NDEF_T5T_TLV_TERMINATOR) {
        /* NDEF end */
        exit = true;
      } else {
        /* Skip Proprietary and RFU too */
        TlvOffset += (uint32_t)offset + (uint32_t)length;
      }
    }
  } else {
    /* No CC File */
    returnCode = NFC_RequestError;
    if (result != NFC_OK) {
      returnCode = result;
    }
  }

  if (info != NULL) {
    info->state                = ctx->state;
    info->majorVersion         = ctx->cc.t5t.majorVersion;
    info->minorVersion         = ctx->cc.t5t.minorVersion;
    info->areaLen              = ctx->areaLen;
    info->areaAvalableSpaceLen = (uint32_t)ctx->cc.t5t.ccLen + ctx->areaLen - ctx->messageOffset;
    info->messageLen           = ctx->messageLen;
  }
  return returnCode;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_ReadRawMessage(NDef_Context *ctx, uint8_t *buf, uint32_t bufLen, uint32_t *rcvdLen, bool single)
{
  NFC_OpResult result;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T) || (buf == NULL)) {
    return NFC_InvalidParameter;
  }

  if (!single) {
    NDef_T5T_InvalidateCache(ctx);
    result = NDef_T5T_ReadLField(ctx);
    if (result != NFC_OK) {
      /* Conclude procedure */
      return result;
    }
  }

  if (ctx->state <= NDEF_STATE_INITIALIZED) {
    /* Conclude procedure  */
    return NFC_WrongState;
  }

  if (ctx->messageLen > bufLen) {
    return NFC_MemoryError;
  }

  result = NDef_T5T_Poller_ReadBytes(ctx, ctx->messageOffset, ctx->messageLen, buf, rcvdLen);
  if (result != NFC_OK) {
    ctx->state = NDEF_STATE_INVALID;
  }
  return result;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_WriteRawMessageLen(NDef_Context *ctx, uint32_t rawMessageLen, bool writeTerminator)
{
  uint8_t    TLV[8U];
  NFC_OpResult result;
  uint8_t    len;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  if ((ctx->state != NDEF_STATE_INITIALIZED) && (ctx->state != NDEF_STATE_READWRITE)) {
    return NFC_WrongState;
  }

  if (writeTerminator && (rawMessageLen != 0U) && ((ctx->messageOffset + rawMessageLen) < ctx->areaLen)) {
    /* Write T5T TLV terminator */
    len = 0U;
    TLV[len] = NDEF_TERMINATOR_TLV_T; /* TLV terminator */
    len++;
    result = NDef_T5T_Poller_WriteBytes(ctx, ctx->messageOffset + rawMessageLen, TLV, len, true, false);
    if (result != NFC_OK) {
      return result;
    }
  }

  len = 0U;
  TLV[len] = NDEF_T5T_TLV_NDEF;
  len++;
  if (rawMessageLen <= NDEF_SHORT_VFIELD_MAX_LEN) {
    TLV[len] = (uint8_t) rawMessageLen;
    len++;
  } else {
    TLV[len] = (uint8_t)(NDEF_SHORT_VFIELD_MAX_LEN + 1U);
    len++;
    TLV[len] = (uint8_t)(rawMessageLen >> 8U);
    len++;
    TLV[len] = (uint8_t) rawMessageLen;
    len++;
  }
  if (writeTerminator && (rawMessageLen == 0U)) {
    TLV[len] = NDEF_TERMINATOR_TLV_T; /* TLV terminator */
    len++;
  }

  result = NDef_T5T_Poller_WriteBytes(ctx, ctx->subCtx.t5t.TlvNDEFOffset, TLV, len, writeTerminator && (rawMessageLen == 0U), false);

  return result;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_WriteRawMessage(NDef_Context *ctx, const uint8_t *buf, uint32_t bufLen)
{
  uint32_t   len = bufLen;
  NFC_OpResult result;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T) || (buf == NULL)) {
    return NFC_InvalidParameter;
  }

  /* TS T5T v1.0 7.5.3.1/2: T5T NDEF Detect should have been called before NDEF write procedure */
  /* Warning: current tag content must not be changed between NDEF Detect procedure and NDEF Write procedure*/

  /* TS T5T v1.0 7.5.3.3: check write access condition */
  if ((ctx->state != NDEF_STATE_INITIALIZED) && (ctx->state != NDEF_STATE_READWRITE)) {
    /* Conclude procedure */
    return NFC_WrongState;
  }

  /* TS T5T v1.0 7.5.3.3: verify available space */
  result = NDef_T5T_Poller_CheckAvailableSpace(ctx, bufLen);
  if (result != NFC_OK) {
    /* Conclude procedure */
    return NFC_InvalidParameter;
  }
  /* TS T5T v1.0 7.5.3.4: reset L-Field to 0 */
  /* and update ctx->messageOffset according to L-field len */
  result = NDef_T5T_Poller_BeginWriteMessage(ctx, bufLen);
  if (result != NFC_OK) {
    ctx->state = NDEF_STATE_INVALID;
    /* Conclude procedure */
    return result;
  }
  if (bufLen != 0U) {
    /* TS T5T v1.0 7.5.3.5: write new NDEF message and write terminator TLV is enough space for it*/
    result = NDef_T5T_Poller_WriteBytes(ctx, ctx->messageOffset, buf, len, true, NDef_T5T_Poller_CheckAvailableSpace(ctx, bufLen + 1U) == NFC_OK);
    if (result != NFC_OK) {
      /* Conclude procedure */
      ctx->state = NDEF_STATE_INVALID;
      return result;
    }
    /* TS T5T v1.0 7.5.3.6 & 7.5.3.7: update L-Field and write Terminator TLV */
    result = NDef_T5T_Poller_EndWriteMessage(ctx, len, false);
    if (result != NFC_OK) {
      /* Conclude procedure */
      ctx->state = NDEF_STATE_INVALID;
      return result;
    }
  }
  return result;
}

/*******************************************************************************/
static NFC_OpResult NDef_T5T_WriteCC(NDef_Context *ctx)
{
  NFC_OpResult  ret;
  uint8_t    *buf;
  uint8_t     dataIt;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  buf    = ctx->ccBuf;
  dataIt = 0U;
  /* Encode CC */
  buf[dataIt] = ctx->cc.t5t.magicNumber;                                                                /* Byte 0 */
  dataIt++;
  buf[dataIt] = (uint8_t)(((ctx->cc.t5t.majorVersion  & 0x03U) << 6) |                                  /* Byte 1 */
                          ((ctx->cc.t5t.minorVersion  & 0x03U) << 4) |                                  /*        */
                          ((ctx->cc.t5t.readAccess    & 0x03U) << 2) |                                  /*        */
                          ((ctx->cc.t5t.writeAccess   & 0x03U) << 0));                                  /*        */
  dataIt++;
  buf[dataIt] = (ctx->cc.t5t.ccLen == NDEF_T5T_CC_LEN_8_BYTES) ? 0U : (uint8_t)ctx->cc.t5t.memoryLen;   /* Byte 2 */
  dataIt++;
  buf[dataIt] = 0U;                                                                                     /* Byte 3 */
  if (ctx->cc.t5t.multipleBlockRead) {
    buf[dataIt] |= 0x01U;  /* Byte 3  b0 MBREAD                */
  }
  if (ctx->cc.t5t.mlenOverflow)      {
    buf[dataIt] |= 0x04U;  /* Byte 3  b2 Android MLEN overflow */
  }
  if (ctx->cc.t5t.lockBlock)         {
    buf[dataIt] |= 0x08U;  /* Byte 3  b3 Lock Block            */
  }
  if (ctx->cc.t5t.specialFrame)      {
    buf[dataIt] |= 0x10U;  /* Byte 3  b4 Special Frame         */
  }
  dataIt++;
  if (ctx->cc.t5t.ccLen == NDEF_T5T_CC_LEN_8_BYTES) {
    buf[dataIt] = 0U;                                                                                 /* Byte 4 */
    dataIt++;
    buf[dataIt] = 0U;                                                                                 /* Byte 5 */
    dataIt++;
    buf[dataIt] = (uint8_t)(ctx->cc.t5t.memoryLen >> 8);                                              /* Byte 6 */
    dataIt++;
    buf[dataIt] = (uint8_t)(ctx->cc.t5t.memoryLen >> 0);                                              /* Byte 7 */
    dataIt++;
  }

  ret = NDef_T5T_Poller_WriteBytes(ctx, 0U, buf, ctx->cc.t5t.ccLen, false, false);
  return ret;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_TagFormat(NDef_Context *ctx, const NDef_CapabilityContainer *cc, uint32_t options)
{
  NFC_OpResult               result;
  static const uint8_t     emptyNDEF[] = { 0x03U, 0x00U };

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  /* Reset previous potential info about NDEF messages */
  ctx->messageLen               = 0U;
  ctx->messageOffset            = 0U;
  ctx->subCtx.t5t.TlvNDEFOffset = 0U;

  if (cc != NULL) {
    if ((cc->t5t.ccLen != NDEF_T5T_CC_LEN_8_BYTES) && (cc->t5t.ccLen != NDEF_T5T_CC_LEN_4_BYTES)) {
      return NFC_InvalidParameter;
    }
    (void)memcpy(&ctx->cc, cc, sizeof(NDef_CapabilityContainer));
  } else {
    /* Try to find the appropriate cc values */
    ctx->cc.t5t.magicNumber  = NDEF_T5T_CC_MAGIC_1_BYTE_ADDR_MODE; /* E1 */
    ctx->cc.t5t.majorVersion = NDef_T5T_MajorVersion(NDEF_T5T_MAPPING_VERSION_1_0);
    ctx->cc.t5t.minorVersion = NDef_T5T_MinorVersion(NDEF_T5T_MAPPING_VERSION_1_0);
    ctx->cc.t5t.readAccess   = NDEF_T5T_ACCESS_ALWAYS;
    ctx->cc.t5t.writeAccess  = NDEF_T5T_ACCESS_ALWAYS;

    ctx->cc.t5t.specialFrame = false;
    ctx->cc.t5t.lockBlock    = false;
    ctx->cc.t5t.memoryLen    = 0U;
    ctx->cc.t5t.mlenOverflow = false;

    ctx->cc.t5t.multipleBlockRead = NDef_T5T_IsMultipleBlockReadSupported(ctx);

    /* Try to retrieve the tag's size using getSystemInfo and GetExtSystemInfo */
    if ((ctx->subCtx.t5t.sysInfoSupported == false) || (NDef_T5T_SysInfoMemSizePresent(ctx->subCtx.t5t.sysInfo.infoFlags) == 0U)) {
      return NFC_RequestError;
    }
    ctx->cc.t5t.memoryLen = (uint16_t)((ctx->subCtx.t5t.sysInfo.numberOfBlock * ctx->subCtx.t5t.sysInfo.blockSize) / NDEF_T5T_MLEN_DIVIDER);

    if ((options & NDEF_T5T_FORMAT_OPTION_NFC_FORUM) == NDEF_T5T_FORMAT_OPTION_NFC_FORUM) { /* NFC Forum format */
      if (ctx->cc.t5t.memoryLen >= NDEF_T5T_MAX_MLEN_1_BYTE_ENCODING) {
        ctx->cc.t5t.ccLen = NDEF_T5T_CC_LEN_8_BYTES;
      }
      if (ctx->cc.t5t.memoryLen > 0U) {
        ctx->cc.t5t.memoryLen--; /* remove CC area from memory length */
      }
    } else { /* Android format */
      ctx->cc.t5t.ccLen = NDEF_T5T_CC_LEN_4_BYTES;
      if (ctx->cc.t5t.memoryLen >= NDEF_T5T_MAX_MLEN_1_BYTE_ENCODING) {
        ctx->cc.t5t.mlenOverflow = true;
        ctx->cc.t5t.memoryLen    = 0xFFU;
      }
    }

    if (!ctx->subCtx.t5t.legacySTHighDensity && (ctx->subCtx.t5t.sysInfo.numberOfBlock > NDEF_T5T_MAX_BLOCK_1_BYTE_ADDR)) {
      ctx->cc.t5t.magicNumber = NDEF_T5T_CC_MAGIC_2_BYTE_ADDR_MODE; /* E2 */
    }
  }

  result = NDef_T5T_WriteCC(ctx);
  if (result != NFC_OK) {
    /* If write fails, try to use special frame if not yet used */
    if (!ctx->cc.t5t.specialFrame) {
      NeonRTOS_Sleep(20U); /* Wait to be sure that previous command has ended */
      ctx->cc.t5t.specialFrame = true; /* Add option flag */
      result = NDef_T5T_WriteCC(ctx);
      if (result != NFC_OK) {
        ctx->cc.t5t.specialFrame = false; /* Add option flag */
        return result;
      }
    } else {
      return result;
    }
  }

  /* Update info about current NDEF */

  ctx->subCtx.t5t.TlvNDEFOffset = ctx->cc.t5t.ccLen;

  result = NDef_T5T_Poller_WriteBytes(ctx, ctx->subCtx.t5t.TlvNDEFOffset, emptyNDEF, sizeof(emptyNDEF), true, true);
  if (result == NFC_OK) {
    /* Update info about current NDEF */
    ctx->messageOffset = (uint32_t)ctx->cc.t5t.ccLen + NDEF_T5T_TLV_T_LEN + NDEF_T5T_TLV_L_1_BYTES_LEN;
    ctx->state         = NDEF_STATE_INITIALIZED;
  }
  return result;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_CheckPresence(NDef_Context *ctx)
{
  NFC_OpResult ret;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  ret = NDef_T5T_IsDevicePresent(ctx);

  return ret;
}


/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_CheckAvailableSpace(const NDef_Context *ctx, uint32_t messageLen)
{
  uint32_t lLen;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  if (ctx->state == NDEF_STATE_INVALID) {
    return NFC_WrongState;
  }

  lLen = (messageLen > NDEF_SHORT_VFIELD_MAX_LEN) ? NDEF_T5T_TLV_L_3_BYTES_LEN : NDEF_T5T_TLV_L_1_BYTES_LEN;

  if ((messageLen + ctx->subCtx.t5t.TlvNDEFOffset + NDEF_T5T_TLV_T_LEN + lLen) > (ctx->areaLen + ctx->cc.t5t.ccLen)) {
    return NFC_MemoryError;
  }
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_BeginWriteMessage(NDef_Context *ctx, uint32_t messageLen)
{
  NFC_OpResult ret;
  uint32_t   lLen;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  if ((ctx->state != NDEF_STATE_INITIALIZED) && (ctx->state != NDEF_STATE_READWRITE)) {
    return NFC_WrongState;
  }

  /* TS T5T v1.0 7.5.3.4: reset L-Field to 0 */
  ret = NDef_T5T_Poller_WriteRawMessageLen(ctx, 0U, true);
  if (ret != NFC_OK) {
    /* Conclude procedure */
    ctx->state = NDEF_STATE_INVALID;
    return ret;
  }

  lLen                = (messageLen > NDEF_SHORT_VFIELD_MAX_LEN) ? NDEF_T5T_TLV_L_3_BYTES_LEN : NDEF_T5T_TLV_L_1_BYTES_LEN;
  ctx->messageOffset  = ctx->subCtx.t5t.TlvNDEFOffset;
  ctx->messageOffset += NDEF_T5T_TLV_T_LEN; /* T Length */
  ctx->messageOffset += lLen;               /* L Length */
  ctx->state          = NDEF_STATE_INITIALIZED;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_EndWriteMessage(NDef_Context *ctx, uint32_t messageLen, bool writeTerminator)
{
  NFC_OpResult ret;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  if (ctx->state != NDEF_STATE_INITIALIZED) {
    return NFC_WrongState;
  }

  /* TS T5T v1.0 7.5.3.6 & 7.5.3.7: update L-Field and write Terminator TLV */
  ret = NDef_T5T_Poller_WriteRawMessageLen(ctx, messageLen, writeTerminator);
  if (ret != NFC_OK) {
    /* Conclude procedure */
    ctx->state = NDEF_STATE_INVALID;
    return ret;
  }
  ctx->messageLen = messageLen;
  ctx->state      = (ctx->messageLen == 0U) ? NDEF_STATE_INITIALIZED : NDEF_STATE_READWRITE;
  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult NDef_T5T_Poller_SetReadOnly(NDef_Context *ctx)
{
  NFC_OpResult ret;

  if ((ctx == NULL) || (ctx->type != NDEF_DEV_T5T)) {
    return NFC_InvalidParameter;
  }

  if (ctx->state != NDEF_STATE_READWRITE) {
    return NFC_WrongState;
  }

  /* Change write access */
  ctx->cc.t5t.writeAccess = NDEF_T5T_ACCESS_NEVER;

  ret = NDef_T5T_WriteCC(ctx);
  if (ret != NFC_OK) {
    return ret;
  }

  ret = NDef_T5T_LockDevice(ctx);
  if (ret != NFC_OK) {
    return ret;
  }
  return NFC_OK;
}
