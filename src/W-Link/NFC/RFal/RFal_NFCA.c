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
#include "RFal_NFCA.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#define RFAL_NFCA_SLP_FWT           RFal_ConvMsTo1fc(1)    /*!< Check 1ms for any modulation  ISO14443-3 6.4.3   */
#define RFAL_NFCA_SLP_CMD           0x50U                 /*!< SLP cmd (byte1)    Digital 1.1  6.9.1 & Table 20 */
#define RFAL_NFCA_SLP_BYTE2         0x00U                 /*!< SLP byte2          Digital 1.1  6.9.1 & Table 20 */
#define RFAL_NFCA_SLP_CMD_POS       0U                    /*!< SLP cmd position   Digital 1.1  6.9.1 & Table 20 */
#define RFAL_NFCA_SLP_BYTE2_POS     1U                    /*!< SLP byte2 position Digital 1.1  6.9.1 & Table 20 */

#define RFAL_NFCA_SDD_CT            0x88U                 /*!< Cascade Tag value Digital 1.1 6.7.2              */
#define RFAL_NFCA_SDD_CT_LEN        1U                    /*!< Cascade Tag length                               */

#define RFAL_NFCA_SLP_REQ_LEN       2U                    /*!< SLP_REQ length                                   */

#define RFAL_NFCA_SEL_CMD_LEN       1U                    /*!< SEL_CMD length                                   */
#define RFAL_NFCA_SEL_PAR_LEN       1U                    /*!< SEL_PAR length                                   */
#define RFAL_NFCA_SEL_SELPAR        RFal_NFCA_SelPar(7U, 0U)/*!< SEL_PAR on Select is always with 4 data/nfcid    */
#define RFAL_NFCA_BCC_LEN           1U                    /*!< BCC length                                       */

#define RFAL_NFCA_SDD_REQ_LEN       (RFAL_NFCA_SEL_CMD_LEN + RFAL_NFCA_SEL_PAR_LEN)   /*!< SDD_REQ length       */
#define RFAL_NFCA_SDD_RES_LEN       (RFAL_NFCA_CASCADE_1_UID_LEN + RFAL_NFCA_BCC_LEN) /*!< SDD_RES length       */

#define RFAL_NFCA_T_RETRANS         5U                    /*!< t RETRANSMISSION [3, 33]ms   EMVCo 2.6  A.5      */
#define RFAL_NFCA_N_RETRANS         2U                    /*!< Number of retries            EMVCo 2.6  9.6.1.3  */


/*! SDD_REQ (Select) Cascade Levels  */
enum {
  RFAL_NFCA_SEL_CASCADE_L1 = 0,  /*!< SDD_REQ Cascade Level 1 */
  RFAL_NFCA_SEL_CASCADE_L2 = 1,  /*!< SDD_REQ Cascade Level 2 */
  RFAL_NFCA_SEL_CASCADE_L3 = 2   /*!< SDD_REQ Cascade Level 3 */
};

/*! SDD_REQ (Select) request Cascade Level command   Digital 1.1 Table 15 */
enum {
  RFAL_NFCA_CMD_SEL_CL1 = 0x93, /*!< SDD_REQ command Cascade Level 1 */
  RFAL_NFCA_CMD_SEL_CL2 = 0x95, /*!< SDD_REQ command Cascade Level 2 */
  RFAL_NFCA_CMD_SEL_CL3 = 0x97, /*!< SDD_REQ command Cascade Level 3 */
};

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#define RFal_NFCA_SelPar( nBy, nbi )         (uint8_t)((((nBy)<<4U) & 0xF0U) | ((nbi)&0x0FU) )         /*!< Calculates SEL_PAR with the bytes/bits to be sent */
#define RFal_NFCA_CLn2SELCMD( cl )           (uint8_t)((uint8_t)(RFAL_NFCA_CMD_SEL_CL1) + (2U*(cl)))   /*!< Calculates SEL_CMD with the given cascade level   */
#define RFal_NFCA_NfcidLen2CL( len )         ((len) / 5U)                                              /*!< Calculates cascade level by the NFCID length      */

/*! Executes the given Tx method (f) and if a Timeout error is detected it retries (rt) times performing a delay of (dl) in between  */
#define RFal_NFCA_TxRetry( r, f, rt, dl )                            \
      {                                                      \
        uint8_t rts = (uint8_t)(rt);                       \
        do {                                   \
          (r)=(f);                                       \
          if (((rt)!=0U) && ((dl)!=0U)) {                \
            delay(dl);                         \
          }                                              \
        } while( ((rts--) != 0U) && ((r)==NFC_SlaveTimeout) );  \
      }

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! Technology Detection context */
typedef struct {
  RFal_ComplianceMode    compMode;        /*!< Compliance mode to be used      */
  NFC_OpResult            ret;             /*!< Outcome of presence check       */
} RFal_NFCA_TechDetParams;


/*! Collision Resolution states */
typedef enum {
  RFAL_NFCA_CR_IDLE,                      /*!< IDLE state                      */
  RFAL_NFCA_CR_CL,                        /*!< New Cascading Level state       */
  RFAL_NFCA_CR_SDD_TX,                    /*!< Perform anticollsion Tx state   */
  RFAL_NFCA_CR_SDD,                       /*!< Perform anticollsion state      */
  RFAL_NFCA_CR_SEL_TX,                    /*!< Perform CL Selection Tx state   */
  RFAL_NFCA_CR_SEL,                       /*!< Perform CL Selection state      */
  RFAL_NFCA_CR_DONE                       /*!< Collision Resolution done state */
} RFal_NFCA_ColResState;


/*! Full Collision Resolution states */
typedef enum {
  RFAL_NFCA_CR_FULL_START,                /*!< Start Full Collision Resolution state                   */
  RFAL_NFCA_CR_FULL_SLPCHECK,             /*!< Sleep and Check for restart state                       */
  RFAL_NFCA_CR_FULL_RESTART               /*!< Restart Full Collision Resolution state                 */
} RFal_NFCA_FColResState;


/*! Collision Resolution context */
typedef struct {
  uint8_t               devLimit;         /*!< Device limit to be used                                 */
  RFal_ComplianceMode    compMode;         /*!< Compliance mode to be used                              */
  RFal_NFCA_ListenDevice *nfcaDevList;      /*!< Location of the device list                             */
  uint8_t              *devCnt;           /*!< Location of the device counter                          */
  bool                  collPending;      /*!< Collision pending flag                                  */

  bool                 *collPend;         /*!< Location of collision pending flag (Single CR)          */
  RFal_NFCA_SelReq        selReq;           /*!< SelReqused during anticollision (Single CR)             */
  RFal_NFCA_SelRes       *selRes;           /*!< Location to place of the SEL_RES(SAK) (Single CR)       */
  uint8_t              *nfcId1;           /*!< Location to place the NFCID1 (Single CR)                */
  uint8_t              *nfcId1Len;        /*!< Location to place the NFCID1 length (Single CR)         */
  uint8_t               cascadeLv;        /*!< Current Cascading Level (Single CR)                     */
  RFal_NFCA_ColResState   state;            /*!< Single Collision Resolution state (Single CR)           */
  RFal_NFCA_FColResState  fState;           /*!< Full Collision Resolution state (Full CR)               */
  uint8_t               bytesTxRx;        /*!< TxRx bytes used during anticollision loop (Single CR)   */
  uint8_t               bitsTxRx;         /*!< TxRx bits used during anticollision loop (Single CR)    */
  uint16_t              rxLen;            /*!< Local reception length                                  */
  uint32_t              tmrFDT;           /*!< FDT timer used between SED_REQs  (Single CR)            */
  uint8_t               retries;          /*!< Retries to be performed upon a timeout error (Single CR)*/
  uint8_t               backtrackCnt;     /*!< Backtrack retries (Single CR)                           */
  bool                  doBacktrack;      /*!< Backtrack flag (Single CR)                              */
} RFal_NFCA_ColResParams;


/*! Collision Resolution context */
typedef struct {
  uint8_t               cascadeLv;        /*!< Current Cascading Level                                 */
  uint8_t               fCascadeLv;       /*!< Final Cascading Level                                   */
  RFal_NFCA_SelRes       *selRes;           /*!< Location to place of the SEL_RES(SAK)                   */
  uint16_t              rxLen;            /*!< Local reception length                                  */
  const uint8_t        *nfcid1;           /*!< Location of the NFCID to be selected                    */
  uint8_t               nfcidOffset;      /*!< Selected NFCID offset                                   */
  bool                  isRx;             /*!< Selection is in reception state                         */
} RFal_NFCA_SelParams;

/*! SLP_REQ (HLTA) format   Digital 1.1  6.9.1 & Table 20 */
typedef struct {
  uint8_t      frame[RFAL_NFCA_SLP_REQ_LEN];  /*!< SLP:  0x50 0x00  */
} RFal_NFCA_SlpReq;

/*! RFAL NFC-A instance */
typedef struct {
  RFal_NFCA_TechDetParams DT;               /*!< Technology Detection context                            */
  RFal_NFCA_ColResParams  CR;               /*!< Collision Resolution context                            */
  RFal_NFCA_SelParams     SEL;              /*!< Selection|Activation context                            */

  RFal_NFCA_SlpReq        slpReq;           /*!< SLP_REx buffer                                          */
} RFal_NFCA_;

// timerPollTimeoutValue is necessary after timerCalculateTimeout so that system will wake up upon timer timeout.
#define nfcaTimerStart( timer, time_ms ) (timer) = timerCalculateTimer((uint16_t)(time_ms))            /*!< Configures and starts the RTOX timer            */
#define nfcaTimerisExpired( timer )      timerIsExpired( timer )                               /*!< Checks RTOX timer has expired                   */

RFal_NFCA_ gNfca;  /*!< RFAL NFC-A instance  */

static uint8_t RFal_NFCA_CalculateBcc(const uint8_t *buf, uint8_t bufLen)
{
  uint8_t i;
  uint8_t BCC;

  BCC = 0;

  /* BCC is XOR over first 4 bytes of the SDD_RES  Digital 1.1 6.7.2 */
  for (i = 0; i < bufLen; i++) {
    BCC ^= buf[i];
  }

  return BCC;
}

/*******************************************************************************/
static NFC_OpResult RFal_NFCA_PollerStartSingleCollisionResolution(uint8_t devLimit, bool *collPending, RFal_NFCA_SelRes *selRes, uint8_t *nfcId1, uint8_t *nfcId1Len)
{
  /* Check parameters */
  if ((collPending == NULL) || (selRes == NULL) || (nfcId1 == NULL) || (nfcId1Len == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Initialize output parameters */
  *collPending = false;  /* Activity 1.1  9.3.4.6 */
  *nfcId1Len   = 0;
  memset(nfcId1, 0x00, RFAL_NFCA_CASCADE_3_UID_LEN);


  /* Save parameters */
  gNfca.CR.devLimit    = devLimit;
  gNfca.CR.collPend    = collPending;
  gNfca.CR.selRes      = selRes;
  gNfca.CR.nfcId1      = nfcId1;
  gNfca.CR.nfcId1Len   = nfcId1Len;

  gNfca.CR.tmrFDT      = RFAL_TIMING_NONE;
  gNfca.CR.retries     = RFAL_NFCA_N_RETRANS;
  gNfca.CR.cascadeLv   = (uint8_t)RFAL_NFCA_SEL_CASCADE_L1;
  gNfca.CR.state       = RFAL_NFCA_CR_CL;

  gNfca.CR.doBacktrack  = false;
  gNfca.CR.backtrackCnt = 3U;

  return NFC_OK;
}

/*******************************************************************************/
static NFC_OpResult RFal_NFCA_PollerGetSingleCollisionResolutionStatus(void)
{
  NFC_OpResult ret;
  uint8_t    collBit = 1U;  /* standards mandate or recommend collision bit to be set to One. */


  /* Check if FDT timer is still running */
  if (gNfca.CR.tmrFDT != RFAL_TIMING_NONE) {
    if ((!nfcaTimerisExpired(gNfca.CR.tmrFDT))) {
      return NFC_Busy;
    }
  }

  /*******************************************************************************/
  /* Go through all Cascade Levels     Activity 1.1  9.3.4 */
  if (gNfca.CR.cascadeLv > (uint8_t)RFAL_NFCA_SEL_CASCADE_L3) {
    return NFC_InternalError;
  }

  switch (gNfca.CR.state) {
    /*******************************************************************************/
    case RFAL_NFCA_CR_CL:

      /* Initialize the SDD_REQ to send for the new cascade level */
      memset((uint8_t *)&gNfca.CR.selReq, 0x00, sizeof(RFal_NFCA_SelReq));

      gNfca.CR.bytesTxRx = RFAL_NFCA_SDD_REQ_LEN;
      gNfca.CR.bitsTxRx  = 0U;
      gNfca.CR.state     = RFAL_NFCA_CR_SDD_TX;

    /* fall through */

    /*******************************************************************************/
    case RFAL_NFCA_CR_SDD_TX:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      /* Calculate SEL_CMD and SEL_PAR with the bytes/bits to be sent */
      gNfca.CR.selReq.selCmd = RFal_NFCA_CLn2SELCMD(gNfca.CR.cascadeLv);
      gNfca.CR.selReq.selPar = RFal_NFCA_SelPar(gNfca.CR.bytesTxRx, gNfca.CR.bitsTxRx);

      /* Send SDD_REQ (Anticollision frame) */
      RFal_ISO14443AStartTransceiveAnticollisionFrame((uint8_t *)&gNfca.CR.selReq, &gNfca.CR.bytesTxRx, &gNfca.CR.bitsTxRx, &gNfca.CR.rxLen, RFAL_NFCA_FDTMIN);

      gNfca.CR.state = RFAL_NFCA_CR_SDD;
      break;


    /*******************************************************************************/
    case RFAL_NFCA_CR_SDD:

      ret = RFal_ISO14443AGetTransceiveAnticollisionFrameStatus();
      if(ret == NFC_Busy)
      {
          return ret;
      }

      /* Retry upon timeout  EMVCo 2.6  9.6.1.3 */
      if ((ret == NFC_SlaveTimeout) && (gNfca.CR.devLimit == 0U) && (gNfca.CR.retries != 0U)) {
        gNfca.CR.retries--;
        nfcaTimerStart(gNfca.CR.tmrFDT, RFAL_NFCA_T_RETRANS);

        gNfca.CR.state = RFAL_NFCA_CR_SDD_TX;
        break;
      }

      /* Convert rxLen into bytes */
      gNfca.CR.rxLen = RFal_ConvBitsToBytes(gNfca.CR.rxLen);


      if ((ret == NFC_SlaveTimeout)
          && (gNfca.CR.backtrackCnt != 0U) && (!gNfca.CR.doBacktrack)
          && (!((RFAL_NFCA_SDD_REQ_LEN == gNfca.CR.bytesTxRx) && (0U == gNfca.CR.bitsTxRx)))) {
        /* In multiple card scenarios it may always happen that some
          * collisions of a weaker tag go unnoticed. If then a later
          * collision is recognized and the strong tag has a 0 at the
          * collision position then no tag will respond. Catch this
          * corner case and then try with the bit being sent as zero. */
        RFal_NFCA_SensRes sensRes;
        ret = NFC_RF_Collision;
        RFal_NFCA_PollerCheckPresence(RFAL_14443A_SHORTFRAME_CMD_REQA, &sensRes);
        /* Algorithm below does a post-increment, decrement to go back to current position */
        if (0U == gNfca.CR.bitsTxRx) {
          gNfca.CR.bitsTxRx = 7;
          gNfca.CR.bytesTxRx--;
        } else {
          gNfca.CR.bitsTxRx--;
        }
        collBit = (uint8_t)(((uint8_t *)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] & (1U << gNfca.CR.bitsTxRx));
        collBit = (uint8_t)((0U == collBit) ? 1U : 0U);                          /* Invert the collision bit */
        gNfca.CR.doBacktrack = true;
        gNfca.CR.backtrackCnt--;
      } else {
        gNfca.CR.doBacktrack = false;
      }

      if (ret == NFC_RF_Collision) {
        /* Check received length */
        if ((gNfca.CR.bytesTxRx + ((gNfca.CR.bitsTxRx != 0U) ? 1U : 0U)) > (RFAL_NFCA_SDD_RES_LEN + RFAL_NFCA_SDD_REQ_LEN)) {
          return NFC_ProtocolError;
        }

        /* Collision in BCC: Anticollide only UID part */
        if (((gNfca.CR.bytesTxRx + ((gNfca.CR.bitsTxRx != 0U) ? 1U : 0U)) > (RFAL_NFCA_CASCADE_1_UID_LEN + RFAL_NFCA_SDD_REQ_LEN)) && (gNfca.CR.backtrackCnt != 0U)) {
          gNfca.CR.backtrackCnt--;
          gNfca.CR.bytesTxRx = (RFAL_NFCA_CASCADE_1_UID_LEN + RFAL_NFCA_SDD_REQ_LEN) - 1U;
          gNfca.CR.bitsTxRx = 7;
          collBit = (uint8_t)(((uint8_t *)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] & (1U << gNfca.CR.bitsTxRx));  /* Not a real collision, extract the actual bit for the subsequent code */
        }


        /* Activity 1.0 & 1.1  9.3.4.12: If CON_DEVICES_LIMIT has a value of 0, then
          * NFC Forum Device is configured to perform collision detection only       */
        if ((gNfca.CR.devLimit == 0U) && (!(*gNfca.CR.collPend))) {
          *gNfca.CR.collPend = true;
          return NFC_Ignore;
        }

        *gNfca.CR.collPend = true;

        /* Set and select the collision bit, with the number of bytes/bits successfully TxRx */
        if (collBit != 0U) {
          ((uint8_t *)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] = (uint8_t)(((uint8_t *)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] | (1U << gNfca.CR.bitsTxRx)); /* MISRA 10.3 */
        } else {
          ((uint8_t *)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] = (uint8_t)(((uint8_t *)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] & ~(1U << gNfca.CR.bitsTxRx)); /* MISRA 10.3 */
        }

        gNfca.CR.bitsTxRx++;

        /* Check if number of bits form a byte */
        if (gNfca.CR.bitsTxRx == RFAL_BITS_IN_BYTE) {
          gNfca.CR.bitsTxRx = 0;
          gNfca.CR.bytesTxRx++;
        }

        gNfca.CR.state = RFAL_NFCA_CR_SDD_TX;
        break;
      }

      /*******************************************************************************/
      /* Check if Collision loop has failed */
      if (ret != NFC_OK) {
        return ret;
      }


      /* If collisions are to be reported check whether the response is complete */
      if ((gNfca.CR.devLimit == 0U) && (gNfca.CR.rxLen != sizeof(RFal_NFCA_SddRes))) {
        return NFC_ProtocolError;
      }

      /* Check if the received BCC match */
      if (gNfca.CR.selReq.bcc != RFal_NFCA_CalculateBcc(gNfca.CR.selReq.nfcid1, RFAL_NFCA_CASCADE_1_UID_LEN)) {
        return NFC_ProtocolError;
      }

      /*******************************************************************************/
      /* Anticollision OK, Select this Cascade Level */
      gNfca.CR.selReq.selPar = RFAL_NFCA_SEL_SELPAR;

      gNfca.CR.retries = RFAL_NFCA_N_RETRANS;
      gNfca.CR.state   = RFAL_NFCA_CR_SEL_TX;
      break;

    /*******************************************************************************/
    case RFAL_NFCA_CR_SEL_TX:

      /* Send SEL_REQ (Select command) - Retry upon timeout  EMVCo 2.6  9.6.1.3 */
      RFal_TransceiveBlockingTx((uint8_t *)&gNfca.CR.selReq, sizeof(RFal_NFCA_SelReq), (uint8_t *)gNfca.CR.selRes, sizeof(RFal_NFCA_SelRes), &gNfca.CR.rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCA_FDTMIN);
      gNfca.CR.state   = RFAL_NFCA_CR_SEL;
      break;

    /*******************************************************************************/
    case RFAL_NFCA_CR_SEL:

      ret = RFal_GetTransceiveStatus();
      if(ret == NFC_Busy)
      {
          return ret;
      }

      /* Retry upon timeout  EMVCo 2.6  9.6.1.3 */
      if ((ret == NFC_SlaveTimeout) && (gNfca.CR.devLimit == 0U) && (gNfca.CR.retries != 0U)) {
        gNfca.CR.retries--;
        nfcaTimerStart(gNfca.CR.tmrFDT, RFAL_NFCA_T_RETRANS);

        gNfca.CR.state = RFAL_NFCA_CR_SEL_TX;
        break;
      }

      if (ret != NFC_OK) {
        return ret;
      }

      gNfca.CR.rxLen = RFal_ConvBitsToBytes(gNfca.CR.rxLen);

      /* Ensure proper response length */
      if (gNfca.CR.rxLen != sizeof(RFal_NFCA_SelRes)) {
        return NFC_ProtocolError;
      }

      /*******************************************************************************/
      /* Check cascade byte, if cascade tag then go next cascade level */
      if (*gNfca.CR.selReq.nfcid1 == RFAL_NFCA_SDD_CT) {
        /* Cascade Tag present, store nfcid1 bytes (excluding cascade tag) and continue for next CL */
        memcpy(&gNfca.CR.nfcId1[*gNfca.CR.nfcId1Len], &((uint8_t *)&gNfca.CR.selReq.nfcid1)[RFAL_NFCA_SDD_CT_LEN], (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN));
        *gNfca.CR.nfcId1Len += (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN);

        /* Go to next cascade level */
        gNfca.CR.state = RFAL_NFCA_CR_CL;
        gNfca.CR.cascadeLv++;
      } else {
        /* UID Selection complete, Stop Cascade Level loop */
        memcpy(&gNfca.CR.nfcId1[*gNfca.CR.nfcId1Len], (uint8_t *)&gNfca.CR.selReq.nfcid1, RFAL_NFCA_CASCADE_1_UID_LEN);
        *gNfca.CR.nfcId1Len += RFAL_NFCA_CASCADE_1_UID_LEN;

        gNfca.CR.state = RFAL_NFCA_CR_DONE;
        break;                             /* Only flag operation complete on the next execution */
      }
      break;

    /*******************************************************************************/
    case RFAL_NFCA_CR_DONE:
      return NFC_OK;

    /*******************************************************************************/
    default:
      return NFC_WrongState;
  }
  return NFC_Busy;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerInit(void)
{
  NFC_OpResult ret;

  ret = RFal_SetMode(RFAL_MODE_POLL_NFCA, RFAL_BR_106, RFAL_BR_106);
  if(ret < NFC_OK)
  {
      return ret;
  }

  RFal_SetErrorHandling(ERRORHANDLING_NONE);

  RFal_SetGT(RFAL_GT_NFCA);
  RFal_SetFDTListen(RFAL_FDT_LISTEN_NFCA_POLLER);
  RFal_SetFDTPoll(RFAL_FDT_POLL_NFCA_POLLER);

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerCheckPresence(RFal_14443AShortFrameCmd cmd, RFal_NFCA_SensRes *sensRes)
{
  NFC_OpResult ret;
  uint16_t   rcvLen;

  /* Digital 1.1 6.10.1.3  For Commands ALL_REQ, SENS_REQ, SDD_REQ, and SEL_REQ, the NFC Forum Device      *
   *              MUST treat receipt of a Listen Frame at a time after FDT(Listen, min) as a Timeour Error */

  ret = RFal_ISO14443ATransceiveShortFrame(cmd, (uint8_t *)sensRes, (uint8_t)RFal_ConvBytesToBits(sizeof(RFal_NFCA_SensRes)), &rcvLen, RFAL_NFCA_FDTMIN);
  if ((ret == NFC_RF_Collision) || (ret == NFC_CRC_Error)  || (ret == NFC_MemoryError) || (ret == NFC_FramingError) || (ret == NFC_ParityError) || (ret == NFC_ImcompleteByte)) {
    ret = NFC_OK;
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerTechnologyDetection(RFal_ComplianceMode compMode, RFal_NFCA_SensRes *sensRes)
{
  NFC_OpResult ret;

  ret = RFal_NFCA_PollerStartTechnologyDetection(compMode, sensRes);
  if(ret < NFC_OK)
  {
      return ret;
  }

  do{ 
    ret = RFal_NFCA_PollerGetTechnologyDetectionStatus();
    RFal_Worker();
  }while( ret == NFC_Busy );

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerStartTechnologyDetection(RFal_ComplianceMode compMode, RFal_NFCA_SensRes *sensRes)
{
  NFC_OpResult ret;

  gNfca.DT.compMode = compMode;
  gNfca.DT.ret      = RFal_NFCA_PollerCheckPresence(((compMode == RFAL_COMPLIANCE_MODE_EMV) ? RFAL_14443A_SHORTFRAME_CMD_WUPA : RFAL_14443A_SHORTFRAME_CMD_REQA), sensRes);

  /* Send SLP_REQ as  Activity 1.1  9.2.3.6 and EMVCo 2.6  9.2.1.3 */
  if ((gNfca.DT.compMode != RFAL_COMPLIANCE_MODE_ISO) && (gNfca.DT.ret == NFC_OK)) {
    ret = RFal_NFCA_PollerStartSleep();
    if(ret < NFC_OK)
    {
        return ret;
    }
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerGetTechnologyDetectionStatus(void)
{
  NFC_OpResult ret;

  /* If Sleep was sent, wait until its termination */
  if ((gNfca.DT.compMode != RFAL_COMPLIANCE_MODE_ISO) && (gNfca.DT.ret == NFC_OK)) {
    ret = RFal_NFCA_PollerGetSleepStatus();
    if(ret == NFC_Busy)
    {
        return ret;
    }
  }

  return gNfca.DT.ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerSingleCollisionResolution(uint8_t devLimit, bool *collPending, RFal_NFCA_SelRes *selRes, uint8_t *nfcId1, uint8_t *nfcId1Len)
{
  NFC_OpResult ret;

  ret = RFal_NFCA_PollerStartSingleCollisionResolution(devLimit, collPending, selRes, nfcId1, nfcId1Len);
  if(ret < NFC_OK)
  {
      return ret;
  }

  do{ 
    ret = RFal_NFCA_PollerGetSingleCollisionResolutionStatus();
    RFal_Worker();
  }while( ret == NFC_Busy );

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerStartFullCollisionResolution(RFal_ComplianceMode compMode, uint8_t devLimit, RFal_NFCA_ListenDevice *nfcaDevList, uint8_t *devCnt)
{
  NFC_OpResult      ret;
  RFal_NFCA_SensRes sensRes;
  uint16_t        rcvLen;

  if ((nfcaDevList == NULL) || (devCnt == NULL)) {
    return NFC_InvalidParameter;
  }

  *devCnt = 0;
  ret     = NFC_OK;

  /*******************************************************************************/
  /* Send ALL_REQ before Anticollision if a Sleep was sent before  Activity 1.1  9.3.4.1 and EMVco 2.6  9.3.2.1 */
  if (compMode != RFAL_COMPLIANCE_MODE_ISO) {
    ret = RFal_ISO14443ATransceiveShortFrame(RFAL_14443A_SHORTFRAME_CMD_WUPA, (uint8_t *)&nfcaDevList->sensRes, (uint8_t)RFal_ConvBytesToBits(sizeof(RFal_NFCA_SensRes)), &rcvLen, RFAL_NFCA_FDTMIN);
    if (ret != NFC_OK) {
      if ((compMode == RFAL_COMPLIANCE_MODE_EMV) || ((ret != NFC_RF_Collision) && (ret != NFC_CRC_Error) && (ret != NFC_FramingError) && (ret != NFC_ParityError) && (ret != NFC_ImcompleteByte))) {
        return ret;
      }
    }

    /* Check proper SENS_RES/ATQA size */
    if ((ret == NFC_OK) && (RFal_ConvBytesToBits(sizeof(RFal_NFCA_SensRes)) != rcvLen)) {
      return NFC_ProtocolError;
    }
  }

  /*******************************************************************************/
  /* Store the SENS_RES from Technology Detection or from WUPA */
  sensRes = nfcaDevList->sensRes;

  if (devLimit > 0U) { /* MISRA 21.18 */
    memset(nfcaDevList, 0x00, (sizeof(RFal_NFCA_ListenDevice) * devLimit));
  }

  /* Restore the prev SENS_RES, assuming that the SENS_RES received is from first device
    * When only one device is detected it's not woken up then we'll have no SENS_RES (ATQA) */
  nfcaDevList->sensRes = sensRes;

  /* Save parameters */
  gNfca.CR.devCnt      = devCnt;
  gNfca.CR.devLimit    = devLimit;
  gNfca.CR.nfcaDevList = nfcaDevList;
  gNfca.CR.compMode    = compMode;
  gNfca.CR.fState      = RFAL_NFCA_CR_FULL_START;


  /*******************************************************************************/
  /* Only check for T1T if previous SENS_RES was received without a transmission  *
    * error. When collisions occur bits in the SENS_RES may look like a T1T        */
  /* If T1T Anticollision is not supported  Activity 1.1  9.3.4.3 */
  if (RFal_NFCA_IsSensResT1T(&nfcaDevList->sensRes) && (devLimit != 0U) && (ret == NFC_OK) && (compMode != RFAL_COMPLIANCE_MODE_EMV)) {
    /* RID_REQ shall be performed              Activity 1.1  9.3.4.24 */
    RFal_T1T_PollerInit();
    ret = RFal_T1T_PollerRid(&nfcaDevList->ridRes);
    if(ret < NFC_OK)
    {
        return ret;
    }

    *devCnt = 1U;
    nfcaDevList->isSleep   = false;
    nfcaDevList->type      = RFAL_NFCA_T1T;
    nfcaDevList->nfcId1Len = RFAL_NFCA_CASCADE_1_UID_LEN;
    memcpy(&nfcaDevList->nfcId1, &nfcaDevList->ridRes.uid, RFAL_NFCA_CASCADE_1_UID_LEN);

    return NFC_OK;
  }


  ret = RFal_NFCA_PollerStartSingleCollisionResolution(devLimit, &gNfca.CR.collPending, &nfcaDevList->selRes, (uint8_t *)&nfcaDevList->nfcId1, &nfcaDevList->nfcId1Len);
  if(ret < NFC_OK)
  {
      return ret;
  }

  gNfca.CR.fState = RFAL_NFCA_CR_FULL_START;
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerGetFullCollisionResolutionStatus(void)
{
  NFC_OpResult ret;
  uint8_t    newDevType;

  if ((gNfca.CR.nfcaDevList == NULL) || (gNfca.CR.devCnt == NULL)) {
    return NFC_WrongState;
  }


  switch (gNfca.CR.fState) {
    /*******************************************************************************/
    case RFAL_NFCA_CR_FULL_START:

      /*******************************************************************************/
      /* Check whether a T1T has already been detected */
      if (RFal_NFCA_IsSensResT1T(&gNfca.CR.nfcaDevList->sensRes) && (gNfca.CR.nfcaDevList->type == RFAL_NFCA_T1T)) {
        /* T1T doesn't support Anticollision */
        return NFC_OK;
      }

    /* fall through */

    /*******************************************************************************/
    case RFAL_NFCA_CR_FULL_RESTART:  /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      /*******************************************************************************/
      ret = RFal_NFCA_PollerGetSingleCollisionResolutionStatus();
      if(ret < NFC_OK)
      {
          return ret;
      }

      /* Assign Listen Device */
      newDevType = ((uint8_t)gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].selRes.sak) & RFAL_NFCA_SEL_RES_CONF_MASK;  /* MISRA 10.8 */
      /* PRQA S 4342 1 # MISRA 10.5 - Guaranteed that no invalid enum values are created: see guard_eq_RFAL_NFCA_T2T, .... */
      gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].type    = (RFal_NFCA_ListenDeviceType) newDevType;
      gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].isSleep = false;
      (*gNfca.CR.devCnt)++;


      /* If a collision was detected and device counter is lower than limit  Activity 1.1  9.3.4.21 */
      if ((*gNfca.CR.devCnt < gNfca.CR.devLimit) && (gNfca.CR.collPending)) {
        /* Put this device to Sleep  Activity 1.1  9.3.4.22 */
        ret = RFal_NFCA_PollerStartSleep();
        if(ret < NFC_OK)
        {
            return ret;
        }

        gNfca.CR.nfcaDevList[(*gNfca.CR.devCnt - 1U)].isSleep = true;

        gNfca.CR.fState = RFAL_NFCA_CR_FULL_SLPCHECK;
        return NFC_Busy;
      } else {
        /* Exit loop */
        gNfca.CR.collPending = false;
      }
      break;


    /*******************************************************************************/
    case RFAL_NFCA_CR_FULL_SLPCHECK:

      ret = RFal_NFCA_PollerGetSleepStatus();
      if(ret == NFC_Busy)
      {
          return ret;
      }

      /* Send a new SENS_REQ to check for other cards  Activity 1.1  9.3.4.23 */
      ret = RFal_NFCA_PollerCheckPresence(RFAL_14443A_SHORTFRAME_CMD_REQA, &gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].sensRes);
      if (ret == NFC_SlaveTimeout) {
        /* No more devices found, exit */
        gNfca.CR.collPending = false;
      } else {
        /* Another device found, restart|continue loop */
        gNfca.CR.collPending = true;

        /*******************************************************************************/
        /* Check if collision resolution shall continue */
        if ((*gNfca.CR.devCnt < gNfca.CR.devLimit) && (gNfca.CR.collPending)) {
          ret = RFal_NFCA_PollerStartSingleCollisionResolution(gNfca.CR.devLimit,
                                                                        &gNfca.CR.collPending,
                                                                        &gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].selRes,
                                                                        (uint8_t *)&gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].nfcId1,
                                                                        &gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].nfcId1Len);
          if(ret < NFC_OK)
          {
              return ret;
          }

          gNfca.CR.fState = RFAL_NFCA_CR_FULL_RESTART;
          return NFC_Busy;
        }
      }
      break;

    /*******************************************************************************/
    default:
      return NFC_WrongState;
  }

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerFullCollisionResolution(RFal_ComplianceMode compMode, uint8_t devLimit, RFal_NFCA_ListenDevice *nfcaDevList, uint8_t *devCnt)
{
  NFC_OpResult ret;

  ret = RFal_NFCA_PollerStartFullCollisionResolution(compMode, devLimit, nfcaDevList, devCnt);
  if(ret < NFC_OK)
  {
      return ret;
  }

  do{ 
    ret = RFal_NFCA_PollerGetFullCollisionResolutionStatus();
    RFal_Worker();
  }while( ret == NFC_Busy );

  return ret;

}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerSleepFullCollisionResolution(uint8_t devLimit, RFal_NFCA_ListenDevice *nfcaDevList, uint8_t *devCnt)
{
  bool       firstRound;
  uint8_t    tmpDevCnt;
  NFC_OpResult ret;


  if ((nfcaDevList == NULL) || (devCnt == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Only use ALL_REQ (WUPA) on the first round */
  firstRound = true;
  *devCnt    = 0;


  /* Perform collision resolution until no new device is found */
  do {
    tmpDevCnt = 0;
    ret = RFal_NFCA_PollerFullCollisionResolution((firstRound ? RFAL_COMPLIANCE_MODE_NFC : RFAL_COMPLIANCE_MODE_ISO), (devLimit - *devCnt), &nfcaDevList[*devCnt], &tmpDevCnt);

    if ((ret == NFC_OK) && (tmpDevCnt > 0U)) {
      *devCnt += tmpDevCnt;

      /* Check whether to search for more devices */
      if (*devCnt < devLimit) {
        /* Set last found device to sleep (all others are slept already) */
        RFal_NFCA_PollerSleep();
        nfcaDevList[((*devCnt) - 1U)].isSleep = true;

        /* Check if any other device is present */
        ret = RFal_NFCA_PollerCheckPresence(RFAL_14443A_SHORTFRAME_CMD_REQA, &nfcaDevList[*devCnt].sensRes);
        if (ret == NFC_OK) {
          firstRound = false;
          continue;
        }
      }
    }
    break;
  } while (true);

  return ((*devCnt > 0U) ? NFC_OK : ret);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerSelect(const uint8_t *nfcid1, uint8_t nfcidLen, RFal_NFCA_SelRes *selRes)
{
  NFC_OpResult ret;

  ret = RFal_NFCA_PollerStartSelect(nfcid1, nfcidLen, selRes);
  if(ret < NFC_OK)
  {
      return ret;
  }

  do{ 
    ret = RFal_NFCA_PollerGetSelectStatus();
    RFal_Worker();
  }while( ret == NFC_Busy );

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerStartSelect(const uint8_t *nfcid1, uint8_t nfcidLen, RFal_NFCA_SelRes *selRes)
{
  if ((nfcid1 == NULL) || (nfcidLen > RFAL_NFCA_CASCADE_3_UID_LEN) || (selRes == NULL)) {
    return NFC_InvalidParameter;
  }


  /* Calculate Cascate Level */
  gNfca.SEL.fCascadeLv = RFal_NFCA_NfcidLen2CL(nfcidLen);
  gNfca.SEL.cascadeLv  = RFAL_NFCA_SEL_CASCADE_L1;

  gNfca.SEL.nfcidOffset  = 0;
  gNfca.SEL.isRx         = false;
  gNfca.SEL.selRes       = selRes;
  gNfca.SEL.nfcid1       = nfcid1;

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerGetSelectStatus(void)
{
  NFC_OpResult     ret;
  RFal_NFCA_SelReq selReq;

  if ((!gNfca.SEL.isRx)) {
    /*******************************************************************************/
    /* Go through all Cascade Levels     Activity 1.1  9.4.4 */
    if (gNfca.SEL.cascadeLv <= gNfca.SEL.fCascadeLv) {
      /* Assign SEL_CMD according to the CLn and SEL_PAR*/
      selReq.selCmd = RFal_NFCA_CLn2SELCMD(gNfca.SEL.cascadeLv);
      selReq.selPar = RFAL_NFCA_SEL_SELPAR;

      /* Compute NFCID/Data on the SEL_REQ command   Digital 1.1  Table 18 */
      if (gNfca.SEL.fCascadeLv != gNfca.SEL.cascadeLv) {
        *selReq.nfcid1 = RFAL_NFCA_SDD_CT;
        memcpy(&selReq.nfcid1[RFAL_NFCA_SDD_CT_LEN], &gNfca.SEL.nfcid1[gNfca.SEL.nfcidOffset], (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN));
        gNfca.SEL.nfcidOffset += (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN);
      } else {
        memcpy(selReq.nfcid1, &gNfca.SEL.nfcid1[gNfca.SEL.nfcidOffset], RFAL_NFCA_CASCADE_1_UID_LEN);
      }

      /* Calculate nfcid's BCC */
      selReq.bcc = RFal_NFCA_CalculateBcc((uint8_t *)&selReq.nfcid1, sizeof(selReq.nfcid1));

      /*******************************************************************************/
      /* Send SEL_REQ  */
      ret = RFal_TransceiveBlockingTx((uint8_t *)&selReq, sizeof(RFal_NFCA_SelReq), (uint8_t *)gNfca.SEL.selRes, sizeof(RFal_NFCA_SelRes), &gNfca.SEL.rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCA_FDTMIN);
      if(ret < NFC_OK)
      {
          return ret;
      }

      /* Wait for Rx to conclude */
      gNfca.SEL.isRx = true;

      return NFC_Busy;
    }
  } else {
    ret = RFal_GetTransceiveStatus();
    if(ret == NFC_Busy)
    {
        return ret;
    }

    /* Ensure proper response length */
    if (RFal_ConvBitsToBytes(gNfca.SEL.rxLen) != sizeof(RFal_NFCA_SelRes)) {
      return NFC_ProtocolError;
    }

    /* Check if there are more level(s) to be selected */
    if (gNfca.SEL.cascadeLv < gNfca.SEL.fCascadeLv) {
      /* Advance to the next cascade level */
      gNfca.SEL.cascadeLv++;
      gNfca.SEL.isRx = false;

      return NFC_Busy;
    }
  }

  /* REMARK: Could check if NFCID1 is complete */

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerSleep(void)
{
  NFC_OpResult ret;

  ret = RFal_NFCA_PollerStartSleep();
  if(ret < NFC_OK)
  {
      return ret;
  }
  
  do{ 
    ret = RFal_NFCA_PollerGetSleepStatus();
    RFal_Worker();
  }while( ret == NFC_Busy );

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerStartSleep(void)
{
  RFal_TransceiveContext ctx;

  gNfca.slpReq.frame[RFAL_NFCA_SLP_CMD_POS]   = RFAL_NFCA_SLP_CMD;
  gNfca.slpReq.frame[RFAL_NFCA_SLP_BYTE2_POS] = RFAL_NFCA_SLP_BYTE2;

  RFAL_CreateByteFlagsTxRxContext(ctx, (uint8_t *)&gNfca.slpReq, sizeof(RFal_NFCA_SlpReq), (uint8_t *)&gNfca.slpReq, sizeof(gNfca.slpReq), NULL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCA_SLP_FWT);
  
  return RFal_StartTransceive(&ctx);
}

/*******************************************************************************/
NFC_OpResult RFal_NFCA_PollerGetSleepStatus(void)
{
  NFC_OpResult ret;

  /* ISO14443-3 6.4.3  HLTA - If PICC responds with any modulation during 1 ms this response shall be interpreted as not acknowledge
     Digital 2.0  6.9.2.1 & EMVCo 3.0  5.6.2.1 - consider the HLTA command always acknowledged
     No check to be compliant with NFC and EMVCo, and to improve interoperability (Kovio RFID Tag)
  */
  ret = RFal_GetTransceiveStatus();
  if(ret == NFC_Busy)
  {
      return ret;
  }

  return NFC_OK;
}

/*******************************************************************************/
bool RFal_NFCA_ListenerIsSleepReq(const uint8_t *buf, uint16_t bufLen)
{
  /* Check if length and payload match */
  if ((bufLen != sizeof(RFal_NFCA_SlpReq)) || (buf[RFAL_NFCA_SLP_CMD_POS] != RFAL_NFCA_SLP_CMD) || (buf[RFAL_NFCA_SLP_BYTE2_POS] != RFAL_NFCA_SLP_BYTE2)) {
    return false;
  }

  return true;
}

/* If the guards here don't compile then the code above cannot work anymore. */
extern uint8_t guard_eq_RFAL_NFCA_T2T[((RFAL_NFCA_SEL_RES_CONF_MASK & (uint8_t)RFAL_NFCA_T2T) == (uint8_t)RFAL_NFCA_T2T) ? 1 : (-1)];
extern uint8_t guard_eq_RFAL_NFCA_T4T[((RFAL_NFCA_SEL_RES_CONF_MASK & (uint8_t)RFAL_NFCA_T4T) == (uint8_t)RFAL_NFCA_T4T) ? 1 : (-1)];
extern uint8_t guard_eq_RFAL_NFCA_NFCDEP[((RFAL_NFCA_SEL_RES_CONF_MASK & (uint8_t)RFAL_NFCA_NFCDEP) == (uint8_t)RFAL_NFCA_NFCDEP) ? 1 : (-1)];
extern uint8_t guard_eq_RFAL_NFCA_T4T_NFCDEP[((RFAL_NFCA_SEL_RES_CONF_MASK & (uint8_t)RFAL_NFCA_T4T_NFCDEP) == (uint8_t)RFAL_NFCA_T4T_NFCDEP) ? 1 : (-1)];
