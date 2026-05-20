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

#include "RFal_NFC.h"
#include "RFal_NFCF.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#define RFAL_NFCF_SENSF_REQ_LEN_MIN                5U     /*!< SENSF_RES minimum length                              */

#define RFAL_NFCF_READ_WO_ENCRYPTION_MIN_LEN       15U    /*!< Minimum length for a Check Command         T3T  5.4.1 */
#define RFAL_NFCF_WRITE_WO_ENCRYPTION_MIN_LEN      31U    /*!< Minimum length for an Update Command       T3T  5.5.1 */

#define RFAL_NFCF_CHECK_RES_MIN_LEN                11U    /*!< CHECK Response minimum length       T3T 1.0  Table 8  */
#define RFAL_NFCF_UPDATE_RES_MIN_LEN               11U    /*!< UPDATE Response minimum length      T3T 1.0  Table 8  */

#define RFAL_NFCF_CHECK_REQ_MAX_LEN                86U    /*!< Max length of a Check request        T3T 1.0  Table 7 */
#define RFAL_NFCF_CHECK_REQ_MAX_SERV               15U    /*!< Max Services number on Check request T3T 1.0  5.4.1.5 */
#define RFAL_NFCF_CHECK_REQ_MAX_BLOCK              15U    /*!< Max Blocks number on Check request  T3T 1.0  5.4.1.10 */
#define RFAL_NFCF_UPDATE_REQ_MAX_SERV              15U    /*!< Max Services number Update request  T3T 1.0  5.4.1.5  */
#define RFAL_NFCF_UPDATE_REQ_MAX_BLOCK             13U    /*!< Max Blocks number on Update request T3T 1.0  5.4.1.10 */


/*! MRT Check | Update = (Tt3t x ((A+1) + n (B+1)) x 4^E) + dRWTt3t    T3T  5.8
    Max values used: A = 7 ; B = 7 ; E = 3 ; n = 15 (NFC Forum n = 15, JIS n = 32)
*/
#define RFAL_NFCF_MRT_CHECK_UPDATE   ((4096U * (8U + (15U * 8U)) * 64U ) + 16U)

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */
#define RFal_NFCF_Slots2CardNum( s )                 ((uint8_t)(s)+1U) /*!< Converts Time Slot Number (TSN) into num of slots  */

/*! Collision Resolution states */
typedef enum {
  RFAL_NFCF_CR_POLL,                     /*!< Poll Request                    */
  RFAL_NFCF_CR_PARSE,                    /*!< Parse Poll Response             */
  RFAL_NFCF_CR_POLL_SC,                  /*!< Poll Request with RC=SC         */
} RFal_NfcFColResState;



/*! Collision Resolution context */
typedef struct {
  RFal_NFCF_GreedyF       greedyF;
  uint8_t               devLimit;        /*!< Device limit to be used                                 */
  RFal_ComplianceMode    compMode;        /*!< Compliance mode to be used                              */
  RFal_NFCF_ListenDevice *nfcfDevList;     /*!< Location of the device list                             */
  uint8_t              *devCnt;          /*!< Location of the device counter                          */
  bool                  collPending;     /*!< Collision pending flag                                  */
  bool                  nfcDepFound;
  RFal_NfcFColResState   state;            /*!< Single Collision Resolution state (Single CR)           */
} RFal_NFCF_ColResParams;


/*! RFAL NFC-F instance */
typedef struct {
  RFal_NFCF_ColResParams CR;                 /*!< Collision Resolution */
} RFal_NFCF_;

static RFal_NFCF_ gNfcf;  /*!< RFAL NFC-F instance  */

//static RFal_NFC_fGreedyF gRFalNfcfGreedyF;   /*!< Activity's NFCF Greedy collection */
/*******************************************************************************/
static void RFal_NFCF_ComputeValidSENF(RFal_NFCF_ListenDevice *outDevInfo, uint8_t *curDevIdx, uint8_t devLimit, bool overwrite, bool *nfcDepFound)
{
  uint8_t             tmpIdx;
  bool                duplicate;
  const RFal_NFCF_SensfResBuf *sensfBuf;
  RFal_NFCF_SensfResBuf sensfCopy;


  /*******************************************************************************/
  /* Go through all responses check if valid and duplicates                      */
  /*******************************************************************************/
  while ((gNfcf.CR.greedyF.pollFound > 0U) && ((*curDevIdx) < devLimit)) {
    duplicate = false;
    gNfcf.CR.greedyF.pollFound--;

    /* MISRA 11.3 - Cannot point directly into different object type, use local copy */
    memcpy((uint8_t *)&sensfCopy, (uint8_t *)&gNfcf.CR.greedyF.POLL_F[gNfcf.CR.greedyF.pollFound], sizeof(RFal_NFCF_SensfResBuf));


    /* Point to received SENSF_RES */
    sensfBuf = &sensfCopy;


    /* Check for devices that are already in device list */
    for (tmpIdx = 0; tmpIdx < (*curDevIdx); tmpIdx++) {
      if (memcmp(sensfBuf->SENSF_RES.NFCID2, outDevInfo[tmpIdx].sensfRes.NFCID2, RFAL_NFCF_NFCID2_LEN) == 0) {
        duplicate = true;
        break;
      }
    }

    /* If is a duplicate skip this (and not to overwrite)*/
    if (duplicate && !overwrite) {
      continue;
    }

    /* Check if response length is OK */
    if (((sensfBuf->LEN - RFAL_NFCF_HEADER_LEN) < RFAL_NFCF_SENSF_RES_LEN_MIN) || ((sensfBuf->LEN - RFAL_NFCF_HEADER_LEN) > RFAL_NFCF_SENSF_RES_LEN_MAX)) {
      continue;
    }

    /* Check if the response is a SENSF_RES / Polling response */
    if (sensfBuf->SENSF_RES.CMD != (uint8_t)RFAL_NFCF_CMD_POLLING_RES) {
      continue;
    }

    /* Check if is an overwrite request or new device*/
    if (duplicate && overwrite) {
      /* overwrite deviceInfo/GRE_SENSF_RES with SENSF_RES */
      outDevInfo[tmpIdx].sensfResLen = (sensfBuf->LEN - RFAL_NFCF_LENGTH_LEN);
      memcpy(&outDevInfo[tmpIdx].sensfRes, &sensfBuf->SENSF_RES, outDevInfo[tmpIdx].sensfResLen);
      continue;
    } else {
      /* fill deviceInfo/GRE_SENSF_RES with new SENSF_RES */
      outDevInfo[(*curDevIdx)].sensfResLen = (sensfBuf->LEN - RFAL_NFCF_LENGTH_LEN);
      memcpy(&outDevInfo[(*curDevIdx)].sensfRes, &sensfBuf->SENSF_RES, outDevInfo[(*curDevIdx)].sensfResLen);
    }

    /* Check if this device supports NFC-DEP and signal it (ACTIVITY 1.1   9.3.6.63) */
    *nfcDepFound = RFal_NFCF_IsNfcDepSupported(&outDevInfo[(*curDevIdx)]);

    (*curDevIdx)++;
  }
}

/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerInit(RFal_BitRate bitRate)
{
  NFC_OpResult ret;

  if ((bitRate != RFAL_BR_212) && (bitRate != RFAL_BR_424)) {
    return NFC_InvalidParameter;
  }

  ret = RFal_SetMode(RFAL_MODE_POLL_NFCF, bitRate, bitRate);
  if(ret < NFC_OK)
  {
      return ret;
  }

  RFal_SetErrorHandling(ERRORHANDLING_NONE);

  RFal_SetGT(RFAL_GT_NFCF);
  RFal_SetFDTListen(RFAL_FDT_LISTEN_NFCF_POLLER);
  RFal_SetFDTPoll(RFAL_FDT_POLL_NFCF_POLLER);

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerPoll(RFal_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, RFal_FeliCaPollRes *cardList, uint8_t *devCnt, uint8_t *collisions)
{
  return RFal_FeliCaPoll(slots, sysCode, reqCode, cardList, RFal_NFCF_Slots2CardNum(slots), devCnt, collisions);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerCheckPresence(void)
{
  NFC_OpResult ret;

  ret = RFal_NFCF_PollerStartCheckPresence();
  if(ret < NFC_OK)
  {
      return ret;
  }

  RFal_RunBlocking(ret, RFal_NFCF_PollerGetCheckPresenceStatus());

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerStartCheckPresence(void)
{
  gNfcf.CR.greedyF.pollFound     = 0;
  gNfcf.CR.greedyF.pollCollision = 0;

  /* ACTIVITY 1.0 & 1.1 - 9.2.3.17 SENSF_REQ  must be with number of slots equal to 4
     *                                SC must be 0xFFFF
     *                                RC must be 0x00 (No system code info required) */
  return RFal_StartFeliCaPoll(RFAL_FELICA_4_SLOTS, RFAL_NFCF_SYSTEMCODE, RFAL_FELICA_POLL_RC_NO_REQUEST, gNfcf.CR.greedyF.POLL_F, RFal_NFCF_Slots2CardNum(RFAL_FELICA_4_SLOTS), &gNfcf.CR.greedyF.pollFound, &gNfcf.CR.greedyF.pollCollision);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerGetCheckPresenceStatus(void)
{
  return RFal_GetFeliCaPollStatus();
}

/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerCollisionResolution(RFal_ComplianceMode compMode, uint8_t devLimit, RFal_NFCF_ListenDevice *nfcfDevList, uint8_t *devCnt)
{
  NFC_OpResult ret;

  ret = RFal_NFCF_PollerStartCollisionResolution(compMode, devLimit, nfcfDevList, devCnt);
  if(ret < NFC_OK)
  {
      return ret;
  }

  RFal_RunBlocking(ret, RFal_NFCF_PollerGetCollisionResolutionStatus());

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerStartCollisionResolution(RFal_ComplianceMode compMode, uint8_t devLimit, RFal_NFCF_ListenDevice *nfcfDevList, uint8_t *devCnt)
{
  if ((nfcfDevList == NULL) || (devCnt == NULL)) {
    return NFC_InvalidParameter;
  }

  *devCnt = 0;

  /*******************************************************************************************/
  /* ACTIVITY 1.0 - 9.3.6.3 Copy valid SENSF_RES in GRE_POLL_F into GRE_SENSF_RES            */
  /* ACTIVITY 1.0 - 9.3.6.6 The NFC Forum Device MUST remove all entries from GRE_SENSF_RES[]*/
  /* ACTIVITY 2.1 - 9.3.6.2 Populate GRE_SENSF_RES with data from GRE_POLL_F                 */
  /*                                                                                         */
  /* CON_DEVICES_LIMIT = 0 Just check if devices from Tech Detection exceeds -> always true  */
  /* Allow the number of slots open on Technology Detection                                  */
  /*******************************************************************************************/
  RFal_NFCF_ComputeValidSENF(nfcfDevList, devCnt, ((devLimit == 0U) ? RFal_NFCF_Slots2CardNum(RFAL_FELICA_4_SLOTS) : devLimit), false, &gNfcf.CR.nfcDepFound);

  /* Store context */
  gNfcf.CR.nfcfDevList = nfcfDevList;
  gNfcf.CR.compMode    = compMode;
  gNfcf.CR.devLimit    = devLimit;
  gNfcf.CR.devCnt      = devCnt;
  gNfcf.CR.state       = RFAL_NFCF_CR_POLL;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerGetCollisionResolutionStatus(void)
{
  NFC_OpResult  ret;

  switch (gNfcf.CR.state) {
    /*******************************************************************************/
    case RFAL_NFCF_CR_POLL:
    case RFAL_NFCF_CR_POLL_SC:

      if (gNfcf.CR.state == RFAL_NFCF_CR_POLL) {
        /*******************************************************************************/
        /* Activity 2.1  9.3.6.3  - Symbol 2 Check if devices found are lower than the limit */
        if (*gNfcf.CR.devCnt >= gNfcf.CR.devLimit) {
          break;
        }

        /*******************************************************************************/
        /* Activity 1.0 - 9.3.6.5  Copy valid SENSF_RES and then to remove it          */
        /* Activity 1.1 - 9.3.6.65 Copy and filter duplicates                          */
        /* For now, due to some devices keep generating different nfcid2, we use 1.0   */
        /* Phones detected: Samsung Galaxy Nexus,Samsung Galaxy S3,Samsung Nexus S     */
        /*******************************************************************************/
        *gNfcf.CR.devCnt = 0;
      }

      ret = RFal_StartFeliCaPoll(RFAL_FELICA_16_SLOTS,
                                RFAL_NFCF_SYSTEMCODE,
                                (uint8_t)((gNfcf.CR.state == RFAL_NFCF_CR_POLL_SC) ? RFAL_FELICA_POLL_RC_SYSTEM_CODE : RFAL_FELICA_POLL_RC_NO_REQUEST),
                                gNfcf.CR.greedyF.POLL_F,
                                RFal_NFCF_Slots2CardNum((uint8_t)RFAL_FELICA_16_SLOTS),
                                &gNfcf.CR.greedyF.pollFound,
                                &gNfcf.CR.greedyF.pollCollision);
      if(ret < NFC_OK)
      {
          return ret;
      }

      gNfcf.CR.state = RFAL_NFCF_CR_PARSE;
      return NFC_Busy;


    /*******************************************************************************/
    case RFAL_NFCF_CR_PARSE:

      ret = RFal_GetFeliCaPollStatus();
      if(ret == NFC_Busy)
      {
          return NFC_Busy;
      }

      if (ret == NFC_OK) {
        /* Activity 2.1  9.3.6.5 - Symbol 4 Update device list */
        RFal_NFCF_ComputeValidSENF(gNfcf.CR.nfcfDevList, gNfcf.CR.devCnt, gNfcf.CR.devLimit, false, &gNfcf.CR.nfcDepFound);
      }

      /*******************************************************************************/
      /* Activity 2.1  9.3.6.6 - Symbol 5  Check if any device supports NFC DEP       */
      if ((gNfcf.CR.nfcDepFound) && (gNfcf.CR.compMode == RFAL_COMPLIANCE_MODE_NFC)) {
        /* Send another poll request with RC = System Code */
        gNfcf.CR.state = RFAL_NFCF_CR_POLL_SC;

        /* Set compliance mode to invalid (non NFC) to poll for NFC-DEP devices only once */
        gNfcf.CR.compMode = RFAL_COMPLIANCE_MODE_EMV;
        return NFC_Busy;
      }

      break;

    /*******************************************************************************/
    default:
      /* MISRA 16.4: no empty default statement (a comment being enough) */
      break;
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerCheck(const uint8_t *nfcid2, const RFal_NFCF_ServBlockListParam *servBlock, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvdLen)
{
  uint8_t       txBuf[RFAL_NFCF_CHECK_REQ_MAX_LEN];
  uint8_t       msgIt;
  uint8_t       i;
  NFC_OpResult    ret;
  const uint8_t *checkRes;

  /* Check parameters */
  if ((nfcid2 == NULL) || (rxBuf == NULL) || (servBlock == NULL)                           ||
      (servBlock->numBlock == 0U) || (servBlock->numBlock > RFAL_NFCF_CHECK_REQ_MAX_BLOCK) ||
      (servBlock->numServ == 0U) || (servBlock->numServ > RFAL_NFCF_CHECK_REQ_MAX_SERV)    ||
      (rxBufLen < (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_CHECK_RES_MIN_LEN))) {
    return NFC_InvalidParameter;
  }

  msgIt = 0;

  /*******************************************************************************/
  /* Compose CHECK command/request                                               */

  txBuf[msgIt++] = RFAL_NFCF_CMD_READ_WITHOUT_ENCRYPTION;                               /* Command Code    */

  memcpy(&txBuf[msgIt], nfcid2, RFAL_NFCF_NFCID2_LEN);                               /* NFCID2          */
  msgIt += RFAL_NFCF_NFCID2_LEN;

  txBuf[msgIt++] = servBlock->numServ;                                                  /* NoS             */
  for (i = 0; i < servBlock->numServ; i++) {
    txBuf[msgIt++] = (uint8_t)((servBlock->servList[i] >> 0U) & 0xFFU);               /* Service Code    */
    txBuf[msgIt++] = (uint8_t)((servBlock->servList[i] >> 8U) & 0xFFU);
  }

  txBuf[msgIt++] = servBlock->numBlock;                                                 /* NoB             */
  for (i = 0; i < servBlock->numBlock; i++) {
    txBuf[msgIt++] = servBlock->blockList[i].conf;                                    /* Block list element conf (Flag|Access|Service) */
    if ((servBlock->blockList[i].conf & RFAL_NFCF_BLOCKLISTELEM_LEN_BIT) != 0U) {                               /* Check if 2 or 3 byte block list element       */
      txBuf[msgIt++] = (uint8_t)(servBlock->blockList[i].blockNum & 0xFFU);         /* 1byte Block Num */
    } else {
      txBuf[msgIt++] = (uint8_t)((servBlock->blockList[i].blockNum >> 0U) & 0xFFU); /* 2byte Block Num */
      txBuf[msgIt++] = (uint8_t)((servBlock->blockList[i].blockNum >> 8U) & 0xFFU);
    }
  }

  /*******************************************************************************/
  /* Transceive CHECK command/request                                            */
  ret = RFal_TransceiveBlockingTxRx(txBuf, msgIt, rxBuf, rxBufLen, rcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCF_MRT_CHECK_UPDATE);

  if (ret == NFC_OK) {
    /* Skip LEN byte */
    checkRes = (rxBuf + RFAL_NFCF_LENGTH_LEN);

    /* Check NFCID and response length    T3T v1.0   5.4.2.3 */
    if ((memcmp(nfcid2, &checkRes[RFAL_NFCF_CMD_LEN], RFAL_NFCF_NFCID2_LEN) != 0) ||
        (*rcvdLen < (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_CHECKUPDATE_RES_ST2_POS))) {
      ret = NFC_ProtocolError;
    }
    /* Check for a valid response */
    else if ((checkRes[RFAL_NFCF_CMD_POS] != (uint8_t)RFAL_NFCF_CMD_READ_WITHOUT_ENCRYPTION_RES) ||
             (checkRes[RFAL_NFCF_CHECKUPDATE_RES_ST1_POS] != RFAL_NFCF_STATUS_FLAG_SUCCESS)      ||
             (checkRes[RFAL_NFCF_CHECKUPDATE_RES_ST2_POS] != RFAL_NFCF_STATUS_FLAG_SUCCESS)) {
      ret = NFC_RequestError;
    }
    /* CHECK successful, remove header */
    else {
      (*rcvdLen) -= (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_CHECKUPDATE_RES_NOB_POS);

      if (*rcvdLen > 0U) {
        ST_MEMMOVE(rxBuf, &checkRes[RFAL_NFCF_CHECKUPDATE_RES_NOB_POS], (*rcvdLen));
      }
    }
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_NFCF_PollerUpdate(const uint8_t *nfcid2, const RFal_NFCF_ServBlockListParam *servBlock,  uint8_t *txBuf, uint16_t txBufLen, const uint8_t *blockData, uint8_t *rxBuf, uint16_t rxBufLen)
{
  uint8_t    i;
  uint16_t   msgIt;
  uint16_t   rcvdLen;
  uint16_t   auxLen;
  const uint8_t    *updateRes;
  NFC_OpResult ret;

  /* Check parameters */
  if ((nfcid2 == NULL) || (rxBuf == NULL) || (servBlock == NULL) || (txBuf == NULL)         ||
      (servBlock->numBlock == 0U) || (servBlock->numBlock > RFAL_NFCF_UPDATE_REQ_MAX_BLOCK) ||
      (servBlock->numServ == 0U)   || (servBlock->numServ > RFAL_NFCF_UPDATE_REQ_MAX_SERV)  ||
      (rxBufLen < (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_UPDATE_RES_MIN_LEN))) {
    return NFC_InvalidParameter;
  }

  /* Calculate required txBuffer length T3T 1.0  Table 9 */
  auxLen = (uint16_t)(RFAL_NFCF_CMD_LEN + RFAL_NFCF_NFCID2_LEN + RFAL_NFCF_NOS_LEN + (servBlock->numServ * sizeof(RFal_NFCF_Serv)) +
                      RFAL_NFCF_NOB_LEN + (uint16_t)((uint16_t)servBlock->numBlock * RFAL_NFCF_BLOCKLISTELEM_MAX_LEN) + (uint16_t)((uint16_t)servBlock->numBlock * RFAL_NFCF_BLOCK_LEN));

  /* Check whether the provided buffer is sufficient for this request */
  if (txBufLen < auxLen) {
    return NFC_InvalidParameter;
  }

  msgIt = 0;

  /*******************************************************************************/
  /* Compose UPDATE command/request                                              */

  txBuf[msgIt++] = RFAL_NFCF_CMD_WRITE_WITHOUT_ENCRYPTION;                              /* Command Code    */

  memcpy(&txBuf[msgIt], nfcid2, RFAL_NFCF_NFCID2_LEN);                               /* NFCID2          */
  msgIt += RFAL_NFCF_NFCID2_LEN;

  txBuf[msgIt++] = servBlock->numServ;                                                  /* NoS             */
  for (i = 0; i < servBlock->numServ; i++) {
    txBuf[msgIt++] = (uint8_t)((servBlock->servList[i] >> 0U) & 0xFFU);               /* Service Code    */
    txBuf[msgIt++] = (uint8_t)((servBlock->servList[i] >> 8U) & 0xFFU);
  }

  txBuf[msgIt++] = servBlock->numBlock;                                                 /* NoB             */
  for (i = 0; i < servBlock->numBlock; i++) {
    txBuf[msgIt++] = servBlock->blockList[i].conf;                                    /* Block list element conf (Flag|Access|Service) */
    if ((servBlock->blockList[i].conf & RFAL_NFCF_BLOCKLISTELEM_LEN_BIT) != 0U) {                               /* Check if 2 or 3 byte block list element       */
      txBuf[msgIt++] = (uint8_t)(servBlock->blockList[i].blockNum & 0xFFU);         /* 1byte Block Num */
    } else {
      txBuf[msgIt++] = (uint8_t)((servBlock->blockList[i].blockNum >> 0U) & 0xFFU); /* 2byte Block Num */
      txBuf[msgIt++] = (uint8_t)((servBlock->blockList[i].blockNum >> 8U) & 0xFFU);
    }
  }

  auxLen = ((uint16_t)servBlock->numBlock * RFAL_NFCF_BLOCK_LEN);
  memcpy(&txBuf[msgIt], blockData, auxLen);                                          /* Block Data      */
  msgIt += auxLen;


  /*******************************************************************************/
  /* Transceive UPDATE command/request                                           */
  ret = RFal_TransceiveBlockingTxRx(txBuf, msgIt, rxBuf, rxBufLen, &rcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCF_MRT_CHECK_UPDATE);

  if (ret == NFC_OK) {
    /* Skip LEN byte */
    updateRes = (rxBuf + RFAL_NFCF_LENGTH_LEN);

    /* Check NFCID and response length    T3T v1.0   5.5.2.3 */
    if ((memcmp(nfcid2, &updateRes[RFAL_NFCF_CMD_LEN], RFAL_NFCF_NFCID2_LEN) != 0) || (rcvdLen < (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_CHECKUPDATE_RES_ST2_POS))) {
      ret = NFC_ProtocolError;
    }
    /* Check for a valid response */
    else if ((updateRes[RFAL_NFCF_CMD_POS] != (uint8_t)RFAL_NFCF_CMD_WRITE_WITHOUT_ENCRYPTION_RES) ||
             (updateRes[RFAL_NFCF_CHECKUPDATE_RES_ST1_POS] != RFAL_NFCF_STATUS_FLAG_SUCCESS)       ||
             (updateRes[RFAL_NFCF_CHECKUPDATE_RES_ST2_POS] != RFAL_NFCF_STATUS_FLAG_SUCCESS)) {
      ret = NFC_RequestError;
    } else {
      /* MISRA 15.7 - Empty else */
    }
  }

  return ret;
}



/*******************************************************************************/
bool RFal_NFCF_ListenerIsT3TReq(const uint8_t *buf, uint16_t bufLen, uint8_t *nfcid2)
{
  /* Check cmd byte */
  switch (*buf) {
    case RFAL_NFCF_CMD_READ_WITHOUT_ENCRYPTION:
      if (bufLen < RFAL_NFCF_READ_WO_ENCRYPTION_MIN_LEN) {
        return false;
      }
      break;

    case RFAL_NFCF_CMD_WRITE_WITHOUT_ENCRYPTION:
      if (bufLen < RFAL_NFCF_WRITE_WO_ENCRYPTION_MIN_LEN) {
        return false;
      }
      break;

    default:
      return false;
  }

  /* Output NFID2 if requested */
  if (nfcid2 != NULL) {
    memcpy(nfcid2, &buf[RFAL_NFCF_CMD_LEN], RFAL_NFCF_NFCID2_LEN);
  }

  return true;
}