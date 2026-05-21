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
#include <string.h>

#include "RFal_ISO_Dep.h"
#include "RFal_T4T.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#define RFAL_T4T_OFFSET_DO          0x54U        /*!< Tag value for offset BER-TLV data object          */
#define RFAL_T4T_LENGTH_DO          0x03U        /*!< Len value for offset BER-TLV data object          */
#define RFAL_T4T_DATA_DO            0x53U        /*!< Tag value for data BER-TLV data object            */

#define RFAL_T4T_MAX_LC             255U         /*!< Maximum Lc value for short Lc coding              */

/*******************************************************************************/
NFC_OpResult RFal_T4T_PollerComposeCAPDU(const RFal_T4T_CApduParam *apduParam)
{
  uint8_t                  hdrLen;
  uint16_t                 msgIt;

  if ((apduParam == NULL) || (apduParam->cApduBuf == NULL) || (apduParam->cApduLen == NULL)) {
    return NFC_InvalidParameter;
  }

  msgIt                  = 0;
  *(apduParam->cApduLen) = 0;

  /*******************************************************************************/
  /* Compute Command-APDU  according to the format   T4T 1.0 5.1.2 & ISO7816-4 2013 Table 1 */

  /* Check if Data is present */
  if (apduParam->LcFlag) {
    if (apduParam->Lc == 0U) {
      /* Extended field coding not supported */
      return NFC_InvalidParameter;
    }

    /* Check whether requested Lc fits */
    if ((uint16_t)apduParam->Lc > (uint16_t)(RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN - RFAL_T4T_LE_LEN)) {
      return NFC_InvalidParameter; /*  PRQA S  2880 # MISRA 2.1 - Unreachable code due to configuration option being set/unset  */
    }

    /* Calculate the header length a place the data/body where it should be */
    hdrLen = RFAL_T4T_MAX_CAPDU_PROLOGUE_LEN + RFAL_T4T_LC_LEN;

    /* make sure not to exceed buffer size */
    if (((uint16_t)hdrLen + (uint16_t)apduParam->Lc + (apduParam->LeFlag ? RFAL_T4T_LC_LEN : 0U)) > RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN) {
      return NFC_MemoryError; /*  PRQA S  2880 # MISRA 2.1 - Unreachable code due to configuration option being set/unset */
    }
    memmove(&apduParam->cApduBuf->apdu[hdrLen], apduParam->cApduBuf->apdu, apduParam->Lc);
  }

  /* Prepend the ADPDU's header */
  apduParam->cApduBuf->apdu[msgIt++] = apduParam->CLA;
  apduParam->cApduBuf->apdu[msgIt++] = apduParam->INS;
  apduParam->cApduBuf->apdu[msgIt++] = apduParam->P1;
  apduParam->cApduBuf->apdu[msgIt++] = apduParam->P2;


  /* Check if Data field length is to be added */
  if (apduParam->LcFlag) {
    apduParam->cApduBuf->apdu[msgIt++] = apduParam->Lc;
    msgIt += apduParam->Lc;
  }

  /* Check if Expected Response Length is to be added */
  if (apduParam->LeFlag) {
    apduParam->cApduBuf->apdu[msgIt++] = apduParam->Le;
  }

  *(apduParam->cApduLen) = msgIt;

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_T4T_PollerParseRAPDU(RFal_T4T_RApduParam *apduParam)
{
  if ((apduParam == NULL) || (apduParam->rApduBuf == NULL)) {
    return NFC_InvalidParameter;
  }

  if (apduParam->rcvdLen < RFAL_T4T_MAX_RAPDU_SW1SW2_LEN) {
    return NFC_ProtocolError;
  }

  apduParam->rApduBodyLen = (apduParam->rcvdLen - (uint16_t)RFAL_T4T_MAX_RAPDU_SW1SW2_LEN);
  apduParam->statusWord   = (((uint16_t)(&apduParam->rApduBuf->apdu[apduParam->rApduBodyLen])[0] << 8) | (uint16_t)(&apduParam->rApduBuf->apdu[apduParam->rApduBodyLen])[1]);

  /* Check SW1 SW2    T4T 1.0 5.1.3 NOTE */
  if (apduParam->statusWord == RFAL_T4T_ISO7816_STATUS_COMPLETE) {
    return NFC_OK;
  }

  return NFC_RequestError;
}


/*******************************************************************************/
NFC_OpResult RFal_T4T_PollerComposeSelectAppl(RFal_ISO_Dep_ApduBufFormat *cApduBuf, const uint8_t *aid, uint8_t aidLen, uint16_t *cApduLen)
{
  RFal_T4T_CApduParam cAPDU;

  if (cApduBuf == NULL) {
    return NFC_InvalidParameter;
  }
  /* CLA INS P1  P2   Lc  Data   Le  */
  /* 00h A4h 00h 00h  07h AID    00h */
  cAPDU.CLA      = RFAL_T4T_CLA;
  cAPDU.INS      = (uint8_t)RFAL_T4T_INS_SELECT;
  cAPDU.P1       = RFAL_T4T_ISO7816_P1_SELECT_BY_DF_NAME;
  cAPDU.P2       = RFAL_T4T_ISO7816_P2_SELECT_FIRST_OR_ONLY_OCCURENCE | RFAL_T4T_ISO7816_P2_SELECT_RETURN_FCI_TEMPLATE;
  cAPDU.Lc       = aidLen;
  cAPDU.Le       = 0x00;
  cAPDU.LcFlag   = true;
  cAPDU.LeFlag   = true;
  cAPDU.cApduBuf = cApduBuf;
  cAPDU.cApduLen = cApduLen;

  if ((aid != NULL) && (aidLen > 0U)) {
    memcpy(cAPDU.cApduBuf->apdu, aid, aidLen);
  }

  return RFal_T4T_PollerComposeCAPDU(&cAPDU);
}


/*******************************************************************************/
NFC_OpResult RFal_T4T_PollerComposeSelectFile(RFal_ISO_Dep_ApduBufFormat *cApduBuf, const uint8_t *fid, uint8_t fidLen, uint16_t *cApduLen)
{
  RFal_T4T_CApduParam cAPDU;

  if (cApduBuf == NULL) {
    return NFC_InvalidParameter;
  }

  /* CLA INS P1  P2   Lc  Data   Le  */
  /* 00h A4h 00h 0Ch  02h FID    -   */
  cAPDU.CLA      = RFAL_T4T_CLA;
  cAPDU.INS      = (uint8_t)RFAL_T4T_INS_SELECT;
  cAPDU.P1       = RFAL_T4T_ISO7816_P1_SELECT_BY_FILEID;
  cAPDU.P2       = RFAL_T4T_ISO7816_P2_SELECT_FIRST_OR_ONLY_OCCURENCE | RFAL_T4T_ISO7816_P2_SELECT_NO_RESPONSE_DATA;
  cAPDU.Lc       = fidLen;
  cAPDU.Le       = 0x00;
  cAPDU.LcFlag   = true;
  cAPDU.LeFlag   = false;
  cAPDU.cApduBuf = cApduBuf;
  cAPDU.cApduLen = cApduLen;

  if ((fid != NULL) && (fidLen > 0U)) {
    memcpy(cAPDU.cApduBuf->apdu, fid, fidLen);
  }

  return RFal_T4T_PollerComposeCAPDU(&cAPDU);
}


/*******************************************************************************/
NFC_OpResult RFal_T4T_PollerComposeSelectFileV1Mapping(RFal_ISO_Dep_ApduBufFormat *cApduBuf, const uint8_t *fid, uint8_t fidLen, uint16_t *cApduLen)
{
  RFal_T4T_CApduParam cAPDU;

  if (cApduBuf == NULL) {
    return NFC_InvalidParameter;
  }

  /* CLA INS P1  P2   Lc  Data   Le  */
  /* 00h A4h 00h 00h  02h FID    -   */
  cAPDU.CLA      = RFAL_T4T_CLA;
  cAPDU.INS      = (uint8_t)RFAL_T4T_INS_SELECT;
  cAPDU.P1       = RFAL_T4T_ISO7816_P1_SELECT_BY_FILEID;
  cAPDU.P2       = RFAL_T4T_ISO7816_P2_SELECT_FIRST_OR_ONLY_OCCURENCE | RFAL_T4T_ISO7816_P2_SELECT_RETURN_FCI_TEMPLATE;
  cAPDU.Lc       = fidLen;
  cAPDU.Le       = 0x00;
  cAPDU.LcFlag   = true;
  cAPDU.LeFlag   = false;
  cAPDU.cApduBuf = cApduBuf;
  cAPDU.cApduLen = cApduLen;

  if ((fid != NULL) && (fidLen > 0U)) {
    memcpy(cAPDU.cApduBuf->apdu, fid, fidLen);
  }

  return RFal_T4T_PollerComposeCAPDU(&cAPDU);
}


/*******************************************************************************/
NFC_OpResult RFal_T4T_PollerComposeReadData(RFal_ISO_Dep_ApduBufFormat *cApduBuf, uint16_t offset, uint8_t expLen, uint16_t *cApduLen)
{
  RFal_T4T_CApduParam cAPDU;

  memset(&cAPDU, 0x00, sizeof(RFal_T4T_CApduParam));

  /* CLA INS P1  P2   Lc  Data   Le  */
  /* 00h B0h [Offset] -   -      len */
  cAPDU.CLA      = RFAL_T4T_CLA;
  cAPDU.INS      = (uint8_t)RFAL_T4T_INS_READBINARY;
  cAPDU.P1       = (uint8_t)((offset >> 8U) & 0xFFU);
  cAPDU.P2       = (uint8_t)((offset >> 0U) & 0xFFU);
  cAPDU.Le       = expLen;
  cAPDU.LcFlag   = false;
  cAPDU.LeFlag   = true;
  cAPDU.cApduBuf = cApduBuf;
  cAPDU.cApduLen = cApduLen;

  return RFal_T4T_PollerComposeCAPDU(&cAPDU);
}


/*******************************************************************************/
NFC_OpResult RFal_T4T_PollerComposeReadDataODO(RFal_ISO_Dep_ApduBufFormat *cApduBuf, uint32_t offset, uint8_t expLen, uint16_t *cApduLen)
{
  RFal_T4T_CApduParam cAPDU;
  uint8_t           dataIt;

  /* CLA INS P1  P2  Lc  Data         Le */
  /* 00h B1h 00h 00h Lc  54 03 xxyyzz len */
  /*                          [Offset]    */
  cAPDU.CLA      = RFAL_T4T_CLA;
  cAPDU.INS      = (uint8_t)RFAL_T4T_INS_READBINARY_ODO;
  cAPDU.P1       = 0x00U;
  cAPDU.P2       = 0x00U;
  cAPDU.Le       = expLen;
  cAPDU.LcFlag   = true;
  cAPDU.LeFlag   = true;
  cAPDU.cApduBuf = cApduBuf;
  cAPDU.cApduLen = cApduLen;

  dataIt = 0U;
  cApduBuf->apdu[dataIt++] = RFAL_T4T_OFFSET_DO;
  cApduBuf->apdu[dataIt++] = RFAL_T4T_LENGTH_DO;
  cApduBuf->apdu[dataIt++] = (uint8_t)(offset >> 16U);
  cApduBuf->apdu[dataIt++] = (uint8_t)(offset >> 8U);
  cApduBuf->apdu[dataIt++] = (uint8_t)(offset);
  cAPDU.Lc                 = dataIt;

  return RFal_T4T_PollerComposeCAPDU(&cAPDU);
}


/*******************************************************************************/
NFC_OpResult RFal_T4T_PollerComposeWriteData(RFal_ISO_Dep_ApduBufFormat *cApduBuf, uint16_t offset, const uint8_t *data, uint8_t dataLen, uint16_t *cApduLen)
{
  RFal_T4T_CApduParam cAPDU;

  if (cApduBuf == NULL) {
    return NFC_InvalidParameter;
  }

  memset(&cAPDU, 0x00, sizeof(RFal_T4T_CApduParam));

  /* CLA INS P1  P2   Lc  Data   Le  */
  /* 00h D6h [Offset] len Data   -   */
  cAPDU.CLA      = RFAL_T4T_CLA;
  cAPDU.INS      = (uint8_t)RFAL_T4T_INS_UPDATEBINARY;
  cAPDU.P1       = (uint8_t)((offset >> 8U) & 0xFFU);
  cAPDU.P2       = (uint8_t)((offset >> 0U) & 0xFFU);
  cAPDU.Lc       = dataLen;
  cAPDU.LcFlag   = true;
  cAPDU.LeFlag   = false;
  cAPDU.cApduBuf = cApduBuf;
  cAPDU.cApduLen = cApduLen;

  if ((data != NULL) && (dataLen > 0U)) {
    memcpy(cAPDU.cApduBuf->apdu, data, dataLen);
  }

  return RFal_T4T_PollerComposeCAPDU(&cAPDU);
}

/*******************************************************************************/
NFC_OpResult RFal_T4T_PollerComposeWriteDataODO(RFal_ISO_Dep_ApduBufFormat *cApduBuf, uint32_t offset, const uint8_t *data, uint8_t dataLen, uint16_t *cApduLen)
{
  RFal_T4T_CApduParam cAPDU;
  uint8_t           dataIt;

  if (cApduBuf == NULL) {
    return NFC_InvalidParameter;
  }

  memset(&cAPDU, 0x00, sizeof(RFal_T4T_CApduParam));

  /* CLA INS P1  P2   Lc  Data                     Le  */
  /* 00h D7h 00h 00h  len 54 03 xxyyzz 53 Ld data  -   */
  /*                           [offset]     [data]     */
  cAPDU.CLA      = RFAL_T4T_CLA;
  cAPDU.INS      = (uint8_t)RFAL_T4T_INS_UPDATEBINARY_ODO;
  cAPDU.P1       = 0x00U;
  cAPDU.P2       = 0x00U;
  cAPDU.LcFlag   = true;
  cAPDU.LeFlag   = false;
  cAPDU.cApduBuf = cApduBuf;
  cAPDU.cApduLen = cApduLen;

  dataIt = 0U;
  cApduBuf->apdu[dataIt++] = RFAL_T4T_OFFSET_DO;
  cApduBuf->apdu[dataIt++] = RFAL_T4T_LENGTH_DO;
  cApduBuf->apdu[dataIt++] = (uint8_t)(offset >> 16U);
  cApduBuf->apdu[dataIt++] = (uint8_t)(offset >> 8U);
  cApduBuf->apdu[dataIt++] = (uint8_t)(offset);
  cApduBuf->apdu[dataIt++] = RFAL_T4T_DATA_DO;
  cApduBuf->apdu[dataIt++] = dataLen;

  if ((((uint32_t)dataLen + (uint32_t)dataIt) >= RFAL_T4T_MAX_LC) || (((uint32_t)dataLen + (uint32_t)dataIt) >= RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN)) {
    return (NFC_MemoryError);
  }

  if ((data != NULL) && (dataLen > 0U)) {
    memcpy(&cAPDU.cApduBuf->apdu[dataIt], data, dataLen);
  }
  dataIt += dataLen;
  cAPDU.Lc = dataIt;

  return RFal_T4T_PollerComposeCAPDU(&cAPDU);
}
