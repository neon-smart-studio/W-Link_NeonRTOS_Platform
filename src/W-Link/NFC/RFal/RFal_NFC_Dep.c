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
#include "RFal_NFC_Dep.h"
#include "RFal_NFCF.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#define NFCIP_ATR_RETRY_MAX             2U                              /*!< Max consecutive retrys of an ATR REQ with transm error*/

#define NFCIP_PSLPAY_LEN                (2U)                            /*!< PSL Payload length (BRS + FSL)                        */
#define NFCIP_PSLREQ_LEN                (3U + RFAL_NFCDEP_LEN_LEN)      /*!< PSL REQ length (incl LEN)                             */
#define NFCIP_PSLRES_LEN                (3U + RFAL_NFCDEP_LEN_LEN)      /*!< PSL RES length (incl LEN)                             */

#define NFCIP_ATRREQ_BUF_LEN            (RFAL_NFCDEP_ATRREQ_MAX_LEN + RFAL_NFCDEP_LEN_LEN) /*!< ATR REQ max length (incl LEN)     */
#define NFCIP_ATRRES_BUF_LEN            (RFAL_NFCDEP_ATRRES_MAX_LEN + RFAL_NFCDEP_LEN_LEN) /*!< ATR RES max length (incl LEN)     */

#define NFCIP_RLSREQ_LEN                (3U + RFAL_NFCDEP_LEN_LEN)      /*!< RLS REQ length (incl LEN)                             */
#define NFCIP_RLSRES_LEN                (3U + RFAL_NFCDEP_LEN_LEN)      /*!< RSL RES length (incl LEN)                             */
#define NFCIP_RLSRES_MIN                (2U + RFAL_NFCDEP_LEN_LEN)      /*!< Minimum length for a RLS RES (incl LEN)               */

#define NFCIP_DSLREQ_LEN                (3U + RFAL_NFCDEP_LEN_LEN)      /*!< DSL REQ length (incl LEN)                             */
#define NFCIP_DSLRES_LEN                (3U + RFAL_NFCDEP_LEN_LEN)      /*!< DSL RES length (incl LEN)                             */
#define NFCIP_DSLRES_MIN                (2U + RFAL_NFCDEP_LEN_LEN)      /*!< Minimum length for a DSL RES (incl LEN)               */

#define NFCIP_DSLRES_MAX_LEN            (3U + RFAL_NFCDEP_LEN_LEN)      /*!< Maximum length for a DSL RES (incl LEN)               */
#define NFCIP_RLSRES_MAX_LEN            (3U + RFAL_NFCDEP_LEN_LEN)      /*!< Minimum length for a RLS RES (incl LEN)               */
#define NFCIP_TARGET_RES_MAX            ( ( NFCIP_RLSRES_MAX_LEN > NFCIP_DSLRES_MAX_LEN) ? NFCIP_RLSRES_MAX_LEN : NFCIP_DSLRES_MAX_LEN ) /*!< Max target control res length    */



#define NFCIP_NO_FWT                    RFAL_FWT_NONE                   /*!< No FWT value - Target Mode                            */
#define NFCIP_INIT_MIN_RTOX             1U                              /*!< Minimum RTOX value  Digital 1.0  14.8.4.1             */
#define NFCIP_INIT_MAX_RTOX             59U                             /*!< Maximum RTOX value  Digital 1.0  14.8.4.1             */

#define NFCIP_TARG_MIN_RTOX             1U                              /*!< Minimum target RTOX value  Digital 1.0  14.8.4.1      */
#define NFCIP_TARG_MAX_RTOX             59U                             /*!< Maximum target RTOX value  Digital 1.0  14.8.4.1      */

#define NFCIP_TRECOV                    1280U                           /*!< Digital 1.0  A.10  Trecov                             */

#define NFCIP_TIMEOUT_ADJUSTMENT        3072U                            /*!< Timeout Adjustment to compensate timing from end of Tx to end of frame  */
#define NFCIP_RWT_ACTIVATION            (0x1000001U + NFCIP_TIMEOUT_ADJUSTMENT) /*!< Digital 2.2  B.11  RWT ACTIVATION  2^24 + RWT Delta + Adjustment*/
#define NFCIP_RWT_ACM_ACTIVATION        (0x200001U + NFCIP_TIMEOUT_ADJUSTMENT)  /*!< Digital 2.2  B.11  RWT ACTIVATION  2^21 + RWT Delta + Adjustment*/

#define RFAL_NFCDEP_HEADER_PAD          (RFAL_NFCDEP_DEPREQ_HEADER_LEN - RFAL_NFCDEP_LEN_MIN) /*!< Difference between expected rcvd header len and max foreseen */

#ifndef RFAL_NFCDEP_MAX_TX_RETRYS
  #define RFAL_NFCDEP_MAX_TX_RETRYS   (uint8_t)3U          /*!< Number of retransmit retyrs                           */
#endif /* RFAL_NFCDEP_MAX_TX_RETRYS */

#ifndef RFAL_NFCDEP_TO_RETRYS
  #define RFAL_NFCDEP_TO_RETRYS       (uint8_t)3U          /*!< Number of retrys for Timeout                          */
#endif /* RFAL_NFCDEP_TO_RETRYS */

#ifndef RFAL_NFCDEP_MAX_RTOX_RETRYS
  #define RFAL_NFCDEP_MAX_RTOX_RETRYS (uint8_t)10U         /*!< Number of retrys for RTOX    Digital 2.0 17.12.4.3    */
#endif /* RFAL_NFCDEP_MAX_RTOX_RETRYS */

#ifndef RFAL_NFCDEP_MAX_NACK_RETRYS
  #define RFAL_NFCDEP_MAX_NACK_RETRYS (uint8_t)3U          /*!< Number of retrys for NACK                             */
#endif /* RFAL_NFCDEP_MAX_NACK_RETRYS */

#ifndef RFAL_NFCDEP_MAX_ATN_RETRYS
  #define RFAL_NFCDEP_MAX_ATN_RETRYS  (uint8_t)3U          /*!< Number of retrys for ATN                              */
#endif /* RFAL_NFCDEP_MAX_ATN_RETRYS */

#define NFCIP_MIN_TXERROR_LEN           4U               /*!< Minimum frame length with error to be ignored  Digital 1.0 14.12.5.4 */

#define NFCIP_REQ                       (uint8_t)0xD4U   /*!<NFCIP REQuest code                                     */
#define NFCIP_RES                       (uint8_t)0xD5U   /*!<NFCIP RESponse code                                    */

#define NFCIP_BS_MASK                   0x0FU            /*!< Bit mask for BS value on a ATR REQ/RES                */
#define NFCIP_BR_MASK                   NFCIP_BS_MASK    /*!< Bit mask for BR value on a ATR REQ/RES                */

#define NFCIP_PP_GB_MASK                0x02U            /*!< Bit mask for GB value in PP byte on a ATR REQ/RES     */
#define NFCIP_PP_NAD_MASK               0x01U            /*!< Bit mask for NAD value in PP byte on a ATR REQ/RES    */

#define NFCIP_PFB_xPDU_MASK             0xE0U            /*!< Bit mask for PDU type                                 */
#define NFCIP_PFB_IPDU                  0x00U            /*!< Bit mask indicating a Information PDU                 */
#define NFCIP_PFB_RPDU                  0x40U            /*!< Bit mask indicating a Response PDU                    */
#define NFCIP_PFB_SPDU                  0x80U            /*!< Bit mask indicating a Supervisory PDU                 */

#define NFCIP_PFB_MI_BIT                0x10U            /*!< Bit mask for the chaining bit (MI) of PFB             */
#define NFCIP_PFB_DID_BIT               0x04U            /*!< Bit mask for the DID presence bit of PFB              */
#define NFCIP_PFB_NAD_BIT               0x08U            /*!< Bit mask for the NAD presence bit of PFB              */
#define NFCIP_PFB_PNI_MASK              0x03U            /*!< Bit mask for the Packet Number Information            */

#define NFCIP_PFB_Rx_MASK               0x10U            /*!< Bit mask for the R-PDU type                           */
#define NFCIP_PFB_ACK                   0x00U            /*!< Bit mask for R-PDU indicating ACK                     */
#define NFCIP_PFB_NACK                  0x10U            /*!< Bit mask for R-PDU indicating NAK                     */

#define NFCIP_PFB_Sx_MASK               0x10U            /*!< Bit mask for the R-PDU type                           */
#define NFCIP_PFB_ATN                   0x00U            /*!< Bit mask for R-PDU indicating ACK                     */
#define NFCIP_PFB_TO                    0x10U            /*!< Bit mask for R-PDU indicating NAK                     */

#define NFCIP_PFB_INVALID               0xFFU            /*!< Invalid PFB value                                     */

/*
 ******************************************************************************
 * MACROS
 ******************************************************************************
 */

#define nfcipIsTransmissionError(e)    ( ((e) == NFC_CRC_Error) || ((e) == NFC_FramingError) || ((e) == NFC_ParityError) ) /*!< Checks if is a Transmission error */


#define nfcipConv1FcToMs( v )          (RFal_Conv1fcToMs((v)) + 1U)                                     /*!< Converts value v 1fc into milliseconds (fc=13.56)     */

#define nfcipCmdIsReq( cmd )           (((uint8_t)(cmd) % 2U) == 0U)                                    /*!< Checks if the nfcip cmd is a REQ                      */

#define nfcip_PFBhasDID( pfb )         ( ((pfb) & NFCIP_PFB_DID_BIT) == NFCIP_PFB_DID_BIT)              /*!< Checks if pfb is signalling DID                       */
#define nfcip_PFBhasNAD( pfb )         ( ((pfb) & NFCIP_PFB_NAD_BIT) == NFCIP_PFB_NAD_BIT)              /*!< Checks if pfb is signalling NAD                       */

#define nfcip_PFBisIPDU( pfb )         ( ((pfb) & NFCIP_PFB_xPDU_MASK) == NFCIP_PFB_IPDU)               /*!< Checks if pfb is a Information PDU                    */
#define nfcip_PFBisRPDU( pfb )         ( ((pfb) & NFCIP_PFB_xPDU_MASK) == NFCIP_PFB_RPDU)               /*!< Checks if pfb is Response PDU                         */
#define nfcip_PFBisSPDU( pfb )         ( ((pfb) & NFCIP_PFB_xPDU_MASK) == NFCIP_PFB_SPDU)               /*!< Checks if pfb is a Supervisory PDU                    */

#define nfcip_PFBisIMI( pfb )          ( nfcip_PFBisIPDU( pfb ) && (((pfb) & NFCIP_PFB_MI_BIT) == NFCIP_PFB_MI_BIT))  /*!< Checks if pfb is a Information PDU indicating MI chaining */

#define nfcip_PFBisRNACK( pfb )        ( nfcip_PFBisRPDU( pfb ) && (((pfb) & NFCIP_PFB_Rx_MASK) == NFCIP_PFB_NACK)) /*!< Checks if pfb is a R-PDU indicating NACK  */
#define nfcip_PFBisRACK( pfb )         ( nfcip_PFBisRPDU( pfb ) && (((pfb) & NFCIP_PFB_Rx_MASK) == NFCIP_PFB_ACK )) /*!< Checks if pfb is a R-PDU indicating ACK   */

#define nfcip_PFBisSATN( pfb )         ( nfcip_PFBisSPDU( pfb ) && (((pfb) & NFCIP_PFB_Sx_MASK) == NFCIP_PFB_ATN))  /*!< Checks if pfb is a R-PDU indicating ATN   */
#define nfcip_PFBisSTO( pfb )          ( nfcip_PFBisSPDU( pfb ) && (((pfb) & NFCIP_PFB_Sx_MASK) == NFCIP_PFB_TO) )  /*!< Checks if pfb is a R-PDU indicating TO    */


#define nfcip_PFBIPDU( pni )           ( (uint8_t)( 0x00U | NFCIP_PFB_IPDU | ((pni) & NFCIP_PFB_PNI_MASK) ))/*!< Returns a PFB I-PDU with the given packet number (pni)                   */
#define nfcip_PFBIPDU_MI( pni )        ( (uint8_t)(isoDep_PCBIBlock(pni) | NFCIP_PFB_MI_BIT))               /*!< Returns a PFB I-PDU with the given packet number (pni) indicating chaining */

#define nfcip_PFBRPDU( pni )           ( (uint8_t)( 0x00U | NFCIP_PFB_RPDU | ((pni) & NFCIP_PFB_PNI_MASK) ))/*!< Returns a PFB R-PDU with the given packet number (pni)                   */
#define nfcip_PFBRPDU_NACK( pni )      ( (uint8_t)(nfcip_PFBRPDU(pni) | NFCIP_PFB_NACK))                    /*!< Returns a PFB R-PDU with the given packet number (pni) indicating NACK   */
#define nfcip_PFBRPDU_ACK( pni )       ( (uint8_t)(nfcip_PFBRPDU(pni) | NFCIP_PFB_ACK))                     /*!< Returns a PFB R-PDU with the given packet number (pni) indicating ACK    */

#define nfcip_PFBSPDU()                ( (uint8_t)( 0x00U | NFCIP_PFB_SPDU ))                           /*!< Returns a PFB S-PDU                                   */
#define nfcip_PFBSPDU_ATN()            ( (uint8_t)(nfcip_PFBSPDU() | NFCIP_PFB_ATN))                    /*!< Returns a PFB S-PDU indicating ATN                    */
#define nfcip_PFBSPDU_TO()             ( (uint8_t)(nfcip_PFBSPDU() | NFCIP_PFB_TO))                     /*!< Returns a PFB S-PDU indicating TO                     */


#define nfcip_PNIInc( pni )            ( (uint8_t) (((pni)+1U) & NFCIP_PFB_PNI_MASK) )                  /*!< Returns a incremented PNI from the given (pni)        */
#define nfcip_PNIDec( pni )            ( (uint8_t) (((pni)-1U) & NFCIP_PFB_PNI_MASK) )                  /*!< Returns a decremented PNI from the given (pni)        */

#define nfcip_PBF_PNI( pfb )           ( (uint8_t) ((pfb) & NFCIP_PFB_PNI_MASK ))                       /*!< Returns the Packet Number Information (pni)           */

#define nfcip_PPwGB( lr )              ( RFal_NFC_Dep_LR2PP( lr ) | NFCIP_PP_GB_MASK)                      /*!< Returns a PP byte containing the given PP value indicating GB                  */

#define nfcip_DIDMax( did )            ( ( (did) < RFAL_NFCDEP_DID_MAX) ? (did) : RFAL_NFCDEP_DID_MAX )                             /*!< Ensures that the given did has proper value  Digital 14.6.2.3 DID [0 14]       */
#define nfcip_RTOXTargMax( wt )        (uint8_t)( ( (RFAL_NFCDEP_RWT_TRG_MAX / RFal_NFC_Dep_WT2RWT(wt)) < NFCIP_TARG_MAX_RTOX) ? (RFAL_NFCDEP_RWT_TRG_MAX / RFal_NFC_Dep_WT2RWT(wt)) : NFCIP_TARG_MAX_RTOX )/*!< Calculates the Maximum RTOX value for the given wt as a Target */

#define nfcipIsInitiator( st )         ( ((st) >= NFCIP_ST_INIT_IDLE) && ((st) <= NFCIP_ST_INIT_RLS) )  /*!< Checks if module is set as Initiator                                           */
#define nfcipIsTarget( st )            (!nfcipIsInitiator(st))                                          /*!< Checks if module is set as Target                                              */

#define nfcipIsBRAllowed( br, mBR )    (((1U<<(br)) & (mBR)) != 0U)                                     /*!< Checks bit rate is allowed by given mask                                       */

#define nfcipIsEmptyDEPEnabled( op )   (!nfcipIsEmptyDEPDisabled(op))                                   /*!< Checks if empty payload is allowed by operation config  NCI 1.0 Table 81       */
#define nfcipIsEmptyDEPDisabled( op )  (((op) & RFAL_NFCDEP_OPER_EMPTY_DEP_DIS) != 0U)                  /*!< Checks if empty payload is not allowed by operation config  NCI 1.0 Table 81   */

#define nfcipIsRTOXReqEnabled( op )    (!nfcipIsRTOXReqDisabled(op))                                    /*!< Checks if send a RTOX_REQ is allowed by operation config  NCI 1.0 Table 81     */
#define nfcipIsRTOXReqDisabled( op )   (((op) & RFAL_NFCDEP_OPER_RTOX_REQ_DIS) != 0U)                   /*!< Checks if send a RTOX_REQ is not allowed by operation config  NCI 1.0 Table 81 */


/*! Checks if isDeactivating callback is set and calls it, otherwise returns false */
#define nfcipIsDeactivationPending()   ( (gNfcip.isDeactivating == NULL) ? false : gNfcip.isDeactivating() )

/*! Returns the RWT Activation according to the current communication mode */
#define nfcipRWTActivation()            ((gNfcip.cfg.commMode == RFAL_NFCDEP_COMM_ACTIVE) ? NFCIP_RWT_ACM_ACTIVATION : NFCIP_RWT_ACTIVATION)


#define nfcipRTOXAdjust( v )           ((v) - ((v)>>3))                                                 /*!< Adjust RTOX timer value to a percentage of the total, current 88% */

/*******************************************************************************/

// timerPollTimeoutValue is necessary after timerCalculateTimeout so that system will wake up upon timer timeout.
#define nfcipTimerStart( timer, time_ms ) (timer) = timerCalculateTimer((uint16_t)(time_ms))            /*!< Configures and starts the RTOX timer            */
#define nfcipTimerisExpired( timer )      timerIsExpired( timer )                               /*!< Checks RTOX timer has expired                   */

#define nfcipLogE(...)                                                                                  /*!< Macro for the error log method                  */
#define nfcipLogW(...)                                                                                  /*!< Macro for the warning log method                */
#define nfcipLogI(...)                                                                                  /*!< Macro for the info log method                   */
#define nfcipLogD(...)                                                                                  /*!< Macro for the debug log method                  */


/*! Digital 1.1 - 16.12.5.2  The Target SHALL NOT attempt any error recovery and remains in Rx mode upon Transmission or a Protocol Error */
#define nfcDepReEnableRx( rxB, rxBL, rxL )       RFal_TransceiveBlockingTx( NULL, 0, (rxB), (rxBL), (rxL), ( RFAL_TXRX_FLAGS_DEFAULT | (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_ON ), RFAL_FWT_NONE )

static RFal_NFC_Dep gNfcip;                    /*!< NFCIP module instance                         */

/*******************************************************************************/
static bool RFal_NFC_Dep_DxIsSupported(uint8_t Dx, uint8_t BRx, uint8_t BSx)
{
  uint8_t Bx;

  /* Take the min of the possible bit rates, we'll use one for both directions */
  Bx = (BRx < BSx) ? BRx : BSx;

  /* Lower bit rates must be supported for P2P */
  if ((Dx <= (uint8_t)RFAL_NFCDEP_Dx_04_424)) {
    return true;
  }

  if ((Dx == (uint8_t)RFAL_NFCDEP_Dx_08_848) && (Bx >= (uint8_t)RFAL_NFCDEP_Bx_08_848)) {
    return true;
  }

  return false;
}


/*******************************************************************************/
static NFC_OpResult RFal_NFC_Dep_Tx(RFal_NFC_Dep_Cmd cmd, uint8_t *txBuf, uint8_t *paylBuf, uint16_t paylLen, uint8_t pfbData, uint32_t fwt)
{
  uint16_t txBufIt;
  uint8_t *txBlock;
  uint8_t *payloadBuf;
  uint8_t  pfb;


  if (txBuf == NULL) {
    return NFC_InvalidParameter;
  }


  payloadBuf = paylBuf;                                               /* MISRA 17.8: Use intermediate variable */

  if ((paylLen == 0U) || (payloadBuf == NULL)) {
    payloadBuf = (uint8_t *) &txBuf[RFAL_NFCDEP_DEPREQ_HEADER_LEN]; /* If not a DEP (no Data) ensure enough space for header */
  }


  txBufIt  = 0;
  pfb      = pfbData;                                                 /* MISRA 17.8: Use intermediate variable */

  txBlock  = payloadBuf;                                              /* Point to beginning of the Data, and go backwards     */


  gNfcip.lastCmd = (uint8_t)cmd;                                      /* Store last cmd sent    */
  gNfcip.lastPFB = NFCIP_PFB_INVALID;                                 /* Reset last pfb sent    */

  /*******************************************************************************/
  /* Compute outgoing NFCIP message                                              */
  /*******************************************************************************/
  switch (cmd) {
    /*******************************************************************************/
    case NFCIP_CMD_ATR_RES:
    case NFCIP_CMD_ATR_REQ:

      RFal_NFC_Dep_SetNFCID(payloadBuf, gNfcip.cfg.nfcid, gNfcip.cfg.nfcidLen);      /* NFCID */
      txBufIt += RFAL_NFCDEP_NFCID3_LEN;

      payloadBuf[txBufIt++] = gNfcip.cfg.did;                                     /* DID   */
      payloadBuf[txBufIt++] = gNfcip.cfg.bs;                                      /* BS    */
      payloadBuf[txBufIt++] = gNfcip.cfg.br;                                      /* BR    */

      if (cmd == NFCIP_CMD_ATR_RES) {
        payloadBuf[txBufIt++] = gNfcip.cfg.to;                                  /* ATR_RES[ TO ] */
      }

      if (gNfcip.cfg.gbLen > 0U) {
        payloadBuf[txBufIt++] = nfcip_PPwGB(gNfcip.cfg.lr);                     /* PP signalling GB  */
        memcpy(&payloadBuf[txBufIt], gNfcip.cfg.gb, gNfcip.cfg.gbLen);       /* set General Bytes */
        txBufIt += gNfcip.cfg.gbLen;
      } else {
        payloadBuf[txBufIt++] = RFal_NFC_Dep_LR2PP(gNfcip.cfg.lr);                 /* PP without GB     */
      }

      if ((txBufIt + RFAL_NFCDEP_CMDTYPE_LEN + RFAL_NFCDEP_CMD_LEN) > RFAL_NFCDEP_ATRREQ_MAX_LEN) {  /* Check max ATR length (ATR_REQ = ATR_RES)*/
        return NFC_InvalidParameter;
      }
      break;

    /*******************************************************************************/
    case NFCIP_CMD_WUP_REQ:                               /* ISO 18092 - 12.5.2.1 */

      RFal_NFC_Dep_SetNFCID((payloadBuf), gNfcip.cfg.nfcid, gNfcip.cfg.nfcidLen);     /* NFCID */
      txBufIt += RFAL_NFCDEP_NFCID3_LEN;

      *(--txBlock) = gNfcip.cfg.did;                                               /* DID   */
      break;

    /*******************************************************************************/
    case NFCIP_CMD_WUP_RES:                               /* ISO 18092 - 12.5.2.2 */
    case NFCIP_CMD_PSL_REQ:
    case NFCIP_CMD_PSL_RES:

      *(--txBlock) = gNfcip.cfg.did;                                               /* DID   */
      break;

    /*******************************************************************************/
    case NFCIP_CMD_RLS_REQ:
    case NFCIP_CMD_RLS_RES:
    case NFCIP_CMD_DSL_REQ:
    case NFCIP_CMD_DSL_RES:

      /* Digital 1.0 - 14.8.1.1 & 14.9.1.1 & 14.10.1.1 Only add DID if not 0 */
      if (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO) {
        *(--txBlock) = gNfcip.cfg.did;                                           /* DID   */
      }
      break;

    /*******************************************************************************/
    case NFCIP_CMD_DEP_REQ:
    case NFCIP_CMD_DEP_RES:

      /* Compute optional PFB bits */
      if (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO)             {
        pfb |= NFCIP_PFB_DID_BIT;
      }
      if (gNfcip.cfg.nad != RFAL_NFCDEP_NAD_NO)             {
        pfb |= NFCIP_PFB_NAD_BIT;
      }
      if ((gNfcip.isTxChaining) && (nfcip_PFBisIPDU(pfb)))  {
        pfb |= NFCIP_PFB_MI_BIT;
      }

      /* Store PFB for future handling */
      gNfcip.lastPFB       = pfb;                                                  /* store PFB sent */

      if (!nfcip_PFBisSATN(pfb)) {
        gNfcip.lastPFBnATN   = pfb;                                              /* store last PFB different then ATN */
      }


      /* Add NAD if it is to be supported */
      if (gNfcip.cfg.nad != RFAL_NFCDEP_NAD_NO) {
        *(--txBlock) = gNfcip.cfg.nad;                                           /* NAD   */
      }

      /* Digital 1.0 - 14.8.1.1 & 14.8.1.1 Only add DID if not 0 */
      if (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO) {
        *(--txBlock) = gNfcip.cfg.did;                                           /* DID   */
      }

      *(--txBlock) = pfb;                                                          /* PFB */


      /* NCI 1.0 - Check if Empty frames are allowed */
      if ((paylLen == 0U) && nfcipIsEmptyDEPDisabled(gNfcip.cfg.oper) && nfcip_PFBisIPDU(pfb)) {
        return NFC_InvalidParameter;
      }
      break;

    /*******************************************************************************/
    default:
      return NFC_InvalidParameter;
  }

  /*******************************************************************************/
  /* Prepend Header                                                              */
  /*******************************************************************************/
  *(--txBlock) = (uint8_t)cmd;                                                         /* CMD     */
  *(--txBlock) = (uint8_t)(nfcipCmdIsReq(cmd) ? NFCIP_REQ : NFCIP_RES);                /* CMDType */


  txBufIt += paylLen + (uint16_t)((uintptr_t)payloadBuf - (uintptr_t)txBlock);           /* Calculate overall buffer size */


  if (txBufIt > gNfcip.fsc) {                                                          /* Check if msg length violates the maximum payload size FSC */
    return NFC_Unsupport;
  }

  /*******************************************************************************/
  return RFal_NFC_Dep_DataTx(txBlock, txBufIt, fwt);
}

/*******************************************************************************/
static NFC_OpResult RFal_NFC_Dep_TxRx(RFal_NFC_Dep_Cmd cmd, uint8_t *txBuf, uint32_t fwt, uint8_t *paylBuf, uint8_t paylBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxActLen)
{
  NFC_OpResult ret;

  if ((cmd == NFCIP_CMD_DEP_REQ) || (cmd == NFCIP_CMD_DEP_RES)) { /* this method cannot be used for DEPs */
    return NFC_InvalidParameter;
  }

  /* Assign the global params for this TxRx */
  gNfcip.rxBuf       = rxBuf;
  gNfcip.rxBufLen    = rxBufLen;
  gNfcip.rxRcvdLen   = rxActLen;


  /*******************************************************************************/
  /* Transmission                                                                */
  /*******************************************************************************/
  if (txBuf != NULL) {                                           /* if nothing to Tx, just do Rx */
    ret = RFal_NFC_Dep_Tx(cmd, txBuf, paylBuf, paylBufLen, 0, fwt);
    if(ret < NFC_OK)
    {
      return ret;
    }
  }

  /*******************************************************************************/
  /* Reception                                                                   */
  /*******************************************************************************/
  ret = RFal_NFC_Dep_DataRx(true);
  if (ret != NFC_OK) {
    return ret;
  }

  /*******************************************************************************/
  *rxActLen = *rxBuf;                                         /* Use LEN byte instead due to with/without CRC modes */
  return NFC_OK;                                            /* Tx and Rx completed successfully                   */
}


/*******************************************************************************/
static NFC_OpResult RFal_NFC_Dep_ControlMsg(uint8_t pfb, uint8_t RTOX)
{
  uint8_t        ctrlMsg[20];
  uint32_t       fwt;


  /*******************************************************************************/
  /* Calculate Cmd and fwt to be used                                            */
  /*******************************************************************************/
  const RFal_NFC_Dep_Cmd depCmd = ((gNfcip.cfg.role == RFAL_NFCDEP_ROLE_TARGET) ? NFCIP_CMD_DEP_RES : NFCIP_CMD_DEP_REQ);
  fwt    = ((gNfcip.cfg.role == RFAL_NFCDEP_ROLE_TARGET) ? NFCIP_NO_FWT : (nfcip_PFBisSTO(pfb) ? ((RTOX * gNfcip.cfg.fwt) + gNfcip.cfg.dFwt) : (gNfcip.cfg.fwt + gNfcip.cfg.dFwt)));

  if (nfcip_PFBisSTO(pfb)) {
    ctrlMsg[RFAL_NFCDEP_DEPREQ_HEADER_LEN] = RTOX;
    return RFal_NFC_Dep_Tx(depCmd, ctrlMsg, &ctrlMsg[RFAL_NFCDEP_DEPREQ_HEADER_LEN], sizeof(uint8_t), pfb, fwt);
  } else {
    return RFal_NFC_Dep_Tx(depCmd, ctrlMsg, NULL, 0, pfb, fwt);
  }
}

/*******************************************************************************/
static void RFal_NFC_Dep_ClearCounters(void)
{
  gNfcip.cntATNRetrys  = 0;
  gNfcip.cntNACKRetrys = 0;
  gNfcip.cntTORetrys   = 0;
  gNfcip.cntTxRetrys   = 0;
  gNfcip.cntRTOXRetrys = 0;
}

/*******************************************************************************/
static NFC_OpResult RFal_NFC_Dep_InitiatorHandle(NFC_OpResult rxRes, uint16_t rxLen, uint16_t *outActRxLen, bool *outIsChaining)
{
  NFC_OpResult ret;
  uint8_t    nfcDepLen;
  uint8_t    rxMsgIt;
  uint8_t    rxPFB;
  uint8_t    rxRTOX;
  uint8_t    optHdrLen;

  ret        = NFC_InternalError;
  rxMsgIt    = 0;
  optHdrLen  = 0;

  *outActRxLen    = 0;
  *outIsChaining  = false;


  /*******************************************************************************/
  /* Handle reception errors                                                     */
  /*******************************************************************************/
  switch (rxRes) {
    /*******************************************************************************/
    /* Timeout ->  Digital 1.0 14.15.5.6 */
    case NFC_SlaveTimeout:

      nfcipLogI(" NFCIP(I) TIMEOUT  TORetrys:%d \r\n", gNfcip.cntTORetrys);

      /* Digital 1.0 14.15.5.6 - If nTO >= Max raise protocol error */
      if (gNfcip.cntTORetrys++ >= RFAL_NFCDEP_TO_RETRYS) {
        return NFC_ProtocolError;
      }

      /*******************************************************************************/
      /* Upon Timeout error, if Deactivation is pending, no more error recovery
       * will be done #54.
       * This is used to address the issue some devices that have a big TO.
       * Normally LLCP layer has timeout already, and NFCIP layer is still
       * running error handling, retrying ATN/NACKs                                  */
      /*******************************************************************************/
      if (nfcipIsDeactivationPending()) {
        nfcipLogI(" skipping error recovery due deactivation pending \r\n");
        return NFC_SlaveTimeout;
      }

      /* Digital 1.0 14.15.5.6 1)  If last PDU was NACK */
      if (nfcip_PFBisRNACK(gNfcip.lastPFB)) {
        /* Digital 1.0 14.15.5.6 2)  if NACKs failed raise protocol error  */
        if (gNfcip.cntNACKRetrys++ >= RFAL_NFCDEP_MAX_NACK_RETRYS) {
          return NFC_ProtocolError;
        }

        /* Send NACK */
        nfcipLogI(" NFCIP(I) Sending NACK retry: %d \r\n", gNfcip.cntNACKRetrys);
        ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBRPDU_NACK(gNfcip.pni), 0);
        if(ret < NFC_OK)
        {
          return ret;
        }
        return NFC_Busy;
      }

      nfcipLogI(" NFCIP(I) Checking if to send ATN  ATNRetrys: %d \r\n", gNfcip.cntATNRetrys);

      /* Digital 1.0 14.15.5.6 3)  Otherwise send ATN */
      if (gNfcip.cntATNRetrys++ >= RFAL_NFCDEP_MAX_ATN_RETRYS) {
        return NFC_ProtocolError;
      }

      /* Send ATN */
      nfcipLogI(" NFCIP(I) Sending ATN \r\n");
      ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBSPDU_ATN(), 0);
      if(ret < NFC_OK)
      {
        return ret;
      }

      return NFC_Busy;

    /*******************************************************************************/
    /* Data rcvd with error ->  Digital 1.0 14.12.5.4 */
    case NFC_CRC_Error:
    case NFC_ParityError:
    case NFC_FramingError:
    case NFC_RF_Collision:

      nfcipLogI(" NFCIP(I) rx Error: %d \r\n", rxRes);

      /* Digital 1.0 14.12.5.4 Tx Error with data, ignore */
      if (rxLen < NFCIP_MIN_TXERROR_LEN) {
        nfcipLogI(" NFCIP(I) Transmission error w data  \r\n");
#if 0
        if (gNfcip.cfg.commMode == RFAL_NFCDEP_COMM_PASSIVE) {
          nfcipLogI(" NFCIP(I) Transmission error w data -> reEnabling Rx \r\n");
          nfcipReEnableRxTout(NFCIP_TRECOV);
          return NFC_Busy;
        }
#endif /* 0 */
      }

      /* Digital 1.1 16.12.5.4  if NACKs failed raise Transmission error  */
      if (gNfcip.cntNACKRetrys++ >= RFAL_NFCDEP_MAX_NACK_RETRYS) {
        return NFC_FramingError;
      }

      /* Send NACK */
      nfcipLogI(" NFCIP(I) Sending NACK  \r\n");
      ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBRPDU_NACK(gNfcip.pni), 0);
      if(ret < NFC_OK)
      {
        return ret;
      }

      return NFC_Busy;

    case NFC_OK:
      break;

    case NFC_Busy:
      return NFC_Busy;  /* Debug purposes */

    default:
      nfcipLogW(" NFCIP(I) Error: %d \r\n", rxRes);
      return rxRes;
  }

  /*******************************************************************************/
  /* Rx OK check if valid DEP PDU                                                */
  /*******************************************************************************/

  if (gNfcip.rxBuf == NULL) {
    return NFC_IO_Error;
  }

  /* Due to different modes on ST25R391x (with/without CRC) use NFC-DEP LEN instead of bytes retrieved */
  nfcDepLen = gNfcip.rxBuf[rxMsgIt++];

  nfcipLogD(" NFCIP(I) rx OK: %d bytes \r\n", nfcDepLen);

  /* Digital 1.0 14.15.5.5 Protocol Error  */
  if (gNfcip.rxBuf[rxMsgIt++] != NFCIP_RES) {
    nfcipLogW(" NFCIP(I) error %02X instead of %02X \r\n", gNfcip.rxBuf[(rxMsgIt - 1U)], NFCIP_RES);
    return NFC_ProtocolError;
  }

  /* Digital 1.0 14.15.5.5 Protocol Error  */
  if (gNfcip.rxBuf[rxMsgIt++] != (uint8_t)NFCIP_CMD_DEP_RES) {
    nfcipLogW(" NFCIP(I) error %02X instead of %02X \r\n", gNfcip.rxBuf[(rxMsgIt - 1U)], NFCIP_CMD_DEP_RES);
    return NFC_ProtocolError;
  }

  rxPFB = gNfcip.rxBuf[rxMsgIt++];

  /*******************************************************************************/
  /* Check for valid PFB type                                                    */
  if (!(nfcip_PFBisSPDU(rxPFB) || nfcip_PFBisRPDU(rxPFB) || nfcip_PFBisIPDU(rxPFB))) {
    return NFC_ProtocolError;
  }

  /*******************************************************************************/
  /* Digital 1.0 14.8.2.1  check if DID is expected and match -> Protocol Error  */
  if (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO) {
    if ((gNfcip.rxBuf[rxMsgIt++] != gNfcip.cfg.did) || (!nfcip_PFBhasDID(rxPFB))) {
      return NFC_ProtocolError;
    }
    optHdrLen++;                                    /* Inc header optional field cnt*/
  } else if (nfcip_PFBhasDID(rxPFB)) {                /* DID not expected but rcv */
    return NFC_ProtocolError;
  } else {
    /* MISRA 15.7 - Empty else */
  }

  /*******************************************************************************/
  /* Digital 1.0 14.6.2.8 & 14.6.3.11 NAD must not be used  */
  if (gNfcip.cfg.nad != RFAL_NFCDEP_NAD_NO) {
    if ((gNfcip.rxBuf[rxMsgIt++] != gNfcip.cfg.nad) || (!nfcip_PFBhasNAD(rxPFB))) {
      return NFC_ProtocolError;
    }
    optHdrLen++;                                    /* Inc header optional field cnt*/
  } else if (nfcip_PFBhasNAD(rxPFB)) {                /* NAD not expected but rcv */
    return NFC_ProtocolError;
  } else {
    /* MISRA 15.7 - Empty else */
  }

  /*******************************************************************************/
  /* Process R-PDU                                                               */
  /*******************************************************************************/
  if (nfcip_PFBisRPDU(rxPFB)) {
    /*******************************************************************************/
    /* R ACK                                                                       */
    /*******************************************************************************/
    if (nfcip_PFBisRACK(rxPFB)) {
      nfcipLogI(" NFCIP(I) Rcvd ACK  \r\n");
      if (gNfcip.pni == nfcip_PBF_PNI(rxPFB)) {
        /* 14.12.3.3 R-ACK with correct PNI -> Increment */
        gNfcip.pni = nfcip_PNIInc(gNfcip.pni);

        /* R-ACK while not performing chaining -> Protocol error*/
        if (!gNfcip.isTxChaining) {
          return NFC_ProtocolError;
        }

        RFal_NFC_Dep_ClearCounters();
        gNfcip.state = NFCIP_ST_INIT_DEP_IDLE;
        return NFC_OK;                            /* This block has been transmitted */
      } else { /* Digital 1.0 14.12.4.5 ACK with wrong PNI Initiator may retransmit */
        if (gNfcip.cntTxRetrys++ >= RFAL_NFCDEP_MAX_TX_RETRYS) {
          return NFC_ProtocolError;
        }

        /* Extended the MAY in Digital 1.0 14.12.4.5 to only reTransmit if the ACK
         * is for the previous DEP, otherwise raise Protocol immediately
         * If the PNI difference is more than 1 it is worthless to reTransmit 3x
         * and after raise the error                                              */

        if (nfcip_PNIDec(gNfcip.pni) ==  nfcip_PBF_PNI(rxPFB)) {
          /* ReTransmit */
          nfcipLogI(" NFCIP(I) Rcvd ACK prev PNI -> reTx \r\n");
          gNfcip.state = NFCIP_ST_INIT_DEP_TX;
          return NFC_Busy;
        }

        nfcipLogI(" NFCIP(I) Rcvd ACK unexpected far PNI -> Error \r\n");
        return NFC_ProtocolError;
      }
    } else { /* Digital 1.0 - 14.12.5.2 Target must never send NACK  */
      return NFC_ProtocolError;
    }
  }

  /*******************************************************************************/
  /* Process S-PDU                                                               */
  /*******************************************************************************/
  if (nfcip_PFBisSPDU(rxPFB)) {
    nfcipLogI(" NFCIP(I) Rcvd S-PDU  \r\n");
    /*******************************************************************************/
    /* S ATN                                                                       */
    /*******************************************************************************/
    if (nfcip_PFBisSATN(rxPFB)) {                          /* If is a S-ATN        */
      nfcipLogI(" NFCIP(I) Rcvd ATN  \r\n");
      if (nfcip_PFBisSATN(gNfcip.lastPFB)) {             /* Check if is expected */
        gNfcip.cntATNRetrys = 0;                       /* Clear ATN counter    */

        /* Although spec is not clear NFC Forum Digital test is expecting to
         * retransmit upon receiving ATN_RES */
        if (nfcip_PFBisSTO(gNfcip.lastPFBnATN)) {
          nfcipLogI(" NFCIP(I) Rcvd ATN  -> reTx RTOX_RES \r\n");
          ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBSPDU_TO(), gNfcip.lastRTOX);
          if(ret < NFC_OK)
          {
            return ret;
          }
        } else {
          /* ReTransmit ? */
          if (gNfcip.cntTxRetrys++ >= RFAL_NFCDEP_MAX_TX_RETRYS) {
            return NFC_ProtocolError;
          }

          nfcipLogI(" NFCIP(I) Rcvd ATN  -> reTx  PNI: %d \r\n", gNfcip.pni);
          gNfcip.state = NFCIP_ST_INIT_DEP_TX;
        }

        return NFC_Busy;
      } else {                                           /* Digital 1.0  14.12.4.4 & 14.12.4.8 */
        return NFC_ProtocolError;
      }
    }
    /*******************************************************************************/
    /* S TO                                                                        */
    /*******************************************************************************/
    else if (nfcip_PFBisSTO(rxPFB)) {                      /* If is a S-TO (RTOX)  */
      nfcipLogI(" NFCIP(I) Rcvd TO  \r\n");

      rxRTOX = gNfcip.rxBuf[rxMsgIt++];

      /* Digital 1.1 16.12.4.3 - Initiator MAY stop accepting subsequent RTOX Req   *
       *                       - RTOX request to an ATN -> Protocol error           */
      if ((gNfcip.cntRTOXRetrys++ > RFAL_NFCDEP_MAX_RTOX_RETRYS) || nfcip_PFBisSATN(gNfcip.lastPFB)) {
        return NFC_ProtocolError;
      }

      /* Digital 1.1 16.8.4.1 RTOX must be between [1,59] */
      if ((rxRTOX < NFCIP_INIT_MIN_RTOX) || (rxRTOX > NFCIP_INIT_MAX_RTOX)) {
        return NFC_ProtocolError;
      }

      ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBSPDU_TO(), rxRTOX);
      if(ret < NFC_OK)
      {
        return ret;
      }

      gNfcip.lastRTOX = rxRTOX;

      return NFC_Busy;
    } else {
      /* Unexpected S-PDU */
      return NFC_ProtocolError;                       /*  PRQA S  2880 # MISRA 2.1 - Guard code to prevent unexpected behavior */
    }
  }

  /*******************************************************************************/
  /* Process I-PDU                                                               */
  /*******************************************************************************/
  if (nfcip_PFBisIPDU(rxPFB)) {
    if (gNfcip.pni != nfcip_PBF_PNI(rxPFB)) {
      nfcipLogI(" NFCIP(I) Rcvd IPDU wrong PNI     curPNI: %d rxPNI: %d \r\n", gNfcip.pni, nfcip_PBF_PNI(rxPFB));
      return NFC_ProtocolError;
    }

    nfcipLogD(" NFCIP(I) Rcvd IPDU OK    PNI: %d \r\n", gNfcip.pni);

    /* 14.12.3.3 I-PDU with correct PNI -> Increment */
    gNfcip.pni = nfcip_PNIInc(gNfcip.pni);


    /* Successful data Exchange */
    RFal_NFC_Dep_ClearCounters();
    *outActRxLen  = ((uint16_t)nfcDepLen - RFAL_NFCDEP_DEP_HEADER - (uint16_t)optHdrLen);

    if ((&gNfcip.rxBuf[gNfcip.rxBufPaylPos] != &gNfcip.rxBuf[RFAL_NFCDEP_DEP_HEADER + optHdrLen]) && (*outActRxLen > 0U)) {
      memmove(&gNfcip.rxBuf[gNfcip.rxBufPaylPos], &gNfcip.rxBuf[RFAL_NFCDEP_DEP_HEADER + optHdrLen], *outActRxLen);
    }

    /*******************************************************************************/
    /* Check if target is indicating chaining MI                                   */
    /*******************************************************************************/
    if (nfcip_PFBisIMI(rxPFB)) {
      gNfcip.isRxChaining = true;
      *outIsChaining      = true;

      nfcipLogD(" NFCIP(I) Rcvd IPDU OK w MI -> ACK \r\n");
      ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBRPDU_ACK(gNfcip.pni), gNfcip.rxBuf[rxMsgIt++]);
      if(ret < NFC_OK)
      {
        return ret;
      }

      return NFC_Again;  /* Send Again signalling to run again, but some chaining data has arrived*/
    } else {
      gNfcip.isRxChaining = false;
      gNfcip.state        = NFCIP_ST_INIT_DEP_IDLE;

      ret = NFC_OK;    /* Data exchange done */
    }
  }
  return ret;
}


/*******************************************************************************/
static NFC_OpResult RFal_NFC_Dep_TargetHandleRX(NFC_OpResult rxRes, uint16_t *outActRxLen, bool *outIsChaining)
{
  NFC_OpResult ret;
  uint8_t    nfcDepLen;
  uint8_t    rxMsgIt;
  uint8_t    rxPFB;
  uint8_t    optHdrLen;
  uint8_t    resBuf[RFAL_NFCDEP_HEADER_PAD + NFCIP_TARGET_RES_MAX];


  ret        = NFC_InternalError;
  rxMsgIt    = 0;
  optHdrLen  = 0;

  *outActRxLen    = 0;
  *outIsChaining  = false;


  /*******************************************************************************/
  /* Handle reception errors                                                     */
  /*******************************************************************************/
  switch (rxRes) {
    /*******************************************************************************/
    case NFC_OK:
      break;

    case NFC_LinkLoss:
      nfcipLogW(" NFCIP(T) Error: %d \r\n", rxRes);
      return rxRes;

    case NFC_Busy:
      return NFC_Busy;  /* Debug purposes */

    case NFC_SlaveTimeout:
    case NFC_CRC_Error:
    case NFC_ParityError:
    case NFC_FramingError:
    case NFC_ProtocolError:
    default:
      /* Digital 1.1  16.12.5.2 The Target MUST NOT attempt any error recovery.      *
       * The Target MUST always stay in receive mode when a                          *
       * Transmission Error or a Protocol Error occurs.                              *
       *                                                                             *
       * Do not push Transmission/Protocol Errors to upper layer in Listen Mode #766 */

      nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
      return NFC_Busy;
  }

  /*******************************************************************************/
  /* Rx OK check if valid DEP PDU                                                */
  /*******************************************************************************/
  if (gNfcip.rxBuf == NULL) {
    return NFC_IO_Error;
  }

  /* Due to different modes on ST25R391x (with/without CRC) use NFC-DEP LEN instead of bytes retrieved */
  nfcDepLen = gNfcip.rxBuf[rxMsgIt++];

  nfcipLogD(" NFCIP(T) rx OK: %d bytes \r\n", nfcDepLen);

  if (gNfcip.rxBuf[rxMsgIt++] != NFCIP_REQ) {
    nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
    return NFC_Busy; /* NFC_ProtocolError - Ignore bad request */
  }


  /*******************************************************************************/
  /* Check whether target rcvd a normal DEP or deactivation request              */
  /*******************************************************************************/
  switch (gNfcip.rxBuf[rxMsgIt++]) {
    /*******************************************************************************/
    case (uint8_t)NFCIP_CMD_DEP_REQ:
      break;                                /* Continue to normal DEP processing */

    /*******************************************************************************/
    case (uint8_t)NFCIP_CMD_DSL_REQ:

      nfcipLogI(" NFCIP(T) rx DSL \r\n");

      /* Digital 1.0  14.9.1.2 If DID is used and incorrect ignore it */
      /* [Digital 1.0, 16.9.1.2]: If DID == 0, Target SHALL ignore DSL_REQ with DID */
      if ((((gNfcip.rxBuf[rxMsgIt++] != gNfcip.cfg.did) || (nfcDepLen != RFAL_NFCDEP_DSL_RLS_LEN_DID)) && (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO))
          || ((gNfcip.cfg.did == RFAL_NFCDEP_DID_NO) && (nfcDepLen != RFAL_NFCDEP_DSL_RLS_LEN_NO_DID))
         ) {
        nfcipLogI(" NFCIP(T) DSL wrong DID, ignoring \r\n");
        return NFC_Busy;
      }

      RFal_NFC_Dep_Tx(NFCIP_CMD_DSL_RES, resBuf, NULL, 0, 0, NFCIP_NO_FWT);

      gNfcip.state = NFCIP_ST_TARG_DEP_SLEEP;
      return NFC_SleepRequest;

    /*******************************************************************************/
    case (uint8_t)NFCIP_CMD_RLS_REQ:

      nfcipLogI(" NFCIP(T) rx RLS \r\n");

      /* Digital 1.0  14.10.1.2 If DID is used and incorrect ignore it */
      /* [Digital 1.0, 16.10.2.2]: If DID == 0, Target SHALL ignore DSL_REQ with DID */
      if ((((gNfcip.rxBuf[rxMsgIt++] != gNfcip.cfg.did) || (nfcDepLen != RFAL_NFCDEP_DSL_RLS_LEN_DID)) && (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO))
          || ((gNfcip.cfg.did == RFAL_NFCDEP_DID_NO) && (nfcDepLen > RFAL_NFCDEP_DSL_RLS_LEN_NO_DID))
         ) {
        nfcipLogI(" NFCIP(T) RLS wrong DID, ignoring \r\n");
        return NFC_Busy;
      }

      RFal_NFC_Dep_Tx(NFCIP_CMD_RLS_RES, resBuf, NULL, 0, 0, NFCIP_NO_FWT);

      gNfcip.state = NFCIP_ST_TARG_DEP_IDLE;
      return NFC_ReleaseRequest;

    /*******************************************************************************/
    /*case NFCIP_CMD_PSL_REQ:              PSL must be handled in Activation only */
    /*case NFCIP_CMD_WUP_REQ:              WUP not in NFC Forum Digital 1.0       */
    default:

      /* Don't go to NFCIP_ST_TARG_DEP_IDLE state as it needs to ignore this    *
       * invalid frame, and keep waiting for more frames                        */

      nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
      return NFC_Busy; /* NFC_ProtocolError - Ignore bad frame */
  }

  /*******************************************************************************/

  rxPFB = gNfcip.rxBuf[rxMsgIt++];                    /* Store rcvd PFB  */

  /*******************************************************************************/
  /* Check for valid PFB type                                                    */
  if (!(nfcip_PFBisSPDU(rxPFB) || nfcip_PFBisRPDU(rxPFB) || nfcip_PFBisIPDU(rxPFB))) {
    nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
    return NFC_Busy; /* NFC_ProtocolError - Ignore invalid PFB  */
  }

  /*******************************************************************************/
  if (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO) {
    if (!nfcip_PFBhasDID(rxPFB)) {
      nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
      return NFC_Busy; /* NFC_ProtocolError - Ignore bad/missing DID  */
    }
    if (gNfcip.rxBuf[rxMsgIt++] != gNfcip.cfg.did) { /* MISRA 13.5 */
      nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
      return NFC_Busy; /* NFC_ProtocolError - Ignore bad/missing DID  */
    }
    optHdrLen++;                                    /* Inc header optional field cnt*/
  } else if (nfcip_PFBhasDID(rxPFB)) {                /* DID not expected but rcv     */
    nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
    return NFC_Busy; /* NFC_ProtocolError - Ignore unexpected DID  */
  } else {
    /* MISRA 15.7 - Empty else */
  }


  /*******************************************************************************/
  if (gNfcip.cfg.nad != RFAL_NFCDEP_NAD_NO) {
    if ((gNfcip.rxBuf[rxMsgIt++] != gNfcip.cfg.did) || !nfcip_PFBhasDID(rxPFB)) {
      nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
      return NFC_Busy;                            /* NFC_ProtocolError - Ignore bad/missing DID  */
    }
    optHdrLen++;                                    /* Inc header optional field cnt*/
  } else if (nfcip_PFBhasNAD(rxPFB)) {                /* NAD not expected but rcv */
    nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
    return NFC_Busy;                                /* NFC_ProtocolError - Ignore unexpected NAD  */
  } else {
    /* MISRA 15.7 - Empty else */
  }


  /*******************************************************************************/
  /* Process R-PDU                                                               */
  /*******************************************************************************/
  if (nfcip_PFBisRPDU(rxPFB)) {
    nfcipLogD(" NFCIP(T) Rcvd R-PDU  \r\n");
    /*******************************************************************************/
    /* R ACK                                                                       */
    /*******************************************************************************/
    if (nfcip_PFBisRACK(rxPFB)) {
      nfcipLogI(" NFCIP(T) Rcvd ACK  \r\n");
      if (gNfcip.pni == nfcip_PBF_PNI(rxPFB)) {
        /* R-ACK while not performing chaining -> Protocol error */
        if (!gNfcip.isTxChaining) {
          nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
          return NFC_Busy;                    /* NFC_ProtocolError - Ignore unexpected ACK  */
        }

        /* This block has been transmitted and acknowledged, perform RTOX until next data is provided  */

        /* Digital 1.1  16.12.4.7 - If ACK rcvd continue with chaining or an RTOX */
        nfcipTimerStart(gNfcip.RTOXTimer, nfcipRTOXAdjust(nfcipConv1FcToMs(RFal_NFC_Dep_WT2RWT(gNfcip.cfg.to))));
        gNfcip.state = NFCIP_ST_TARG_DEP_RTOX;

        return NFC_OK;                        /* This block has been transmitted */
      }

      /* Digital 1.0 14.12.3.4 - If last send was ATN and rx PNI is minus 1 */
      else if (nfcip_PFBisSATN(gNfcip.lastPFB) && (nfcip_PNIDec(gNfcip.pni) == nfcip_PBF_PNI(rxPFB))) {
        nfcipLogI(" NFCIP(T) wrong PNI, last was ATN reTx  \r\n");
        /* Spec says to leave current PNI as is, but will be Inc after Tx, remaining the same */
        gNfcip.pni = nfcip_PNIDec(gNfcip.pni);

        gNfcip.state = NFCIP_ST_TARG_DEP_TX;
        return NFC_Busy;
      } else {
        /* MISRA 15.7 - Empty else */
      }
    }
    /*******************************************************************************/
    /* R NACK                                                                      */
    /*******************************************************************************/
    /* ISO 18092 12.6.1.3.3 When rcv NACK if PNI = prev PNI sent ->  reTx          */
    else if (nfcip_PFBisRNACK(rxPFB) && (nfcip_PNIDec(gNfcip.pni) == nfcip_PBF_PNI(rxPFB))) {
      nfcipLogI(" NFCIP(T) Rcvd NACK  \r\n");

      gNfcip.pni = nfcip_PNIDec(gNfcip.pni);     /* Dec so that has the prev PNI */

      gNfcip.state = NFCIP_ST_TARG_DEP_TX;
      return NFC_Busy;
    } else {
      nfcipLogI(" NFCIP(T) Unexpected R-PDU \r\n");

      nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
      return NFC_Busy; /* NFC_ProtocolError - Ignore unexpected R-PDU  */
    }
  }

  /*******************************************************************************/
  /* Process S-PDU                                                               */
  /*******************************************************************************/
  if (nfcip_PFBisSPDU(rxPFB)) {
    nfcipLogD(" NFCIP(T) Rcvd S-PDU  \r\n");

    /*******************************************************************************/
    /* S ATN                                                                       */
    /*******************************************************************************/
    /* ISO 18092 12.6.3 Attention                                                  */
    if (nfcip_PFBisSATN(rxPFB)) {                          /*    If is a S-ATN     */
      nfcipLogI(" NFCIP(T) Rcvd ATN  curPNI: %d \r\n", gNfcip.pni);
      ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBSPDU_ATN(), 0);
      if(ret < NFC_OK)
      {
        return ret;
      }
      
      return NFC_Busy;
    }

    /*******************************************************************************/
    /* S TO                                                                        */
    /*******************************************************************************/
    else if (nfcip_PFBisSTO(rxPFB)) {                      /* If is a S-TO (RTOX)  */
      if (nfcip_PFBisSTO(gNfcip.lastPFBnATN)) {
        nfcipLogI(" NFCIP(T) Rcvd TO  \r\n");

        /* Digital 1.1  16.8.4.6  RTOX value in RES different that in REQ -> Protocol Error */
        if (gNfcip.lastRTOX != gNfcip.rxBuf[rxMsgIt++]) {
          nfcipLogI(" NFCIP(T) Mismatched RTOX value \r\n");

          nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
          return NFC_Busy; /* NFC_ProtocolError - Ignore unexpected RTOX value  */
        }

        /* Clear waiting for RTOX Ack Flag */
        gNfcip.isWait4RTOX = false;

        /* Check if a Tx is already pending */
        if (gNfcip.isTxPending) {
          nfcipLogW(" NFCIP(T) Tx pending, go immediately to TX \r\n");

          gNfcip.state = NFCIP_ST_TARG_DEP_TX;
          return NFC_Busy;
        }

        /* Start RTOX timer and change to check state  */
        nfcipTimerStart(gNfcip.RTOXTimer, nfcipRTOXAdjust(nfcipConv1FcToMs(gNfcip.lastRTOX * RFal_NFC_Dep_WT2RWT(gNfcip.cfg.to))));
        gNfcip.state = NFCIP_ST_TARG_DEP_RTOX;

        return NFC_Busy;
      }
    } else {
      /* Unexpected S-PDU */
      nfcipLogI(" NFCIP(T) Unexpected S-PDU \r\n");           /*  PRQA S  2880 # MISRA 2.1 - Guard code to prevent unexpected behavior */

      nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
      return NFC_Busy; /* NFC_ProtocolError - Ignore unexpected S-PDU  */
    }
  }

  /*******************************************************************************/
  /* Process I-PDU                                                               */
  /*******************************************************************************/
  if (nfcip_PFBisIPDU(rxPFB)) {
    if (gNfcip.pni != nfcip_PBF_PNI(rxPFB)) {
      nfcipLogI(" NFCIP(T) Rcvd IPDU wrong PNI     curPNI: %d rxPNI: %d \r\n", gNfcip.pni, nfcip_PBF_PNI(rxPFB));

      /* Digital 1.1 16.12.3.4 - If last send was ATN and rx PNI is minus 1 */
      if (nfcip_PFBisSATN(gNfcip.lastPFB) && (nfcip_PNIDec(gNfcip.pni) == nfcip_PBF_PNI(rxPFB))) {
        /* Spec says to leave current PNI as is, but will be Inc after Data Tx, remaining the same */
        gNfcip.pni = nfcip_PNIDec(gNfcip.pni);

        if (nfcip_PFBisIMI(rxPFB)) {
          nfcipLogI(" NFCIP(T) PNI = prevPNI && ATN before && chaining -> send ACK  \r\n");
          ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBRPDU_ACK(gNfcip.pni), gNfcip.rxBuf[rxMsgIt++]);
          if(ret < NFC_OK)
          {
            return ret;
          }

          /* Digital 1.1 16.12.3.4 (...) leave the current PNI unchanged afterwards */
          gNfcip.pni = nfcip_PNIInc(gNfcip.pni);
        } else {
          nfcipLogI(" NFCIP(T) PNI = prevPNI && ATN before -> reTx last I-PDU  \r\n");
          gNfcip.state = NFCIP_ST_TARG_DEP_TX;
        }

        return NFC_Busy;
      }

      nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
      return NFC_Busy;            /* NFC_ProtocolError - Ignore bad PNI value  */
    }

    nfcipLogD(" NFCIP(T) Rcvd IPDU OK PNI: %d  \r\n", gNfcip.pni);

    /*******************************************************************************/
    /* Successful data exchange                                                    */
    /*******************************************************************************/
    *outActRxLen  = ((uint16_t)nfcDepLen - RFAL_NFCDEP_DEP_HEADER - (uint16_t)optHdrLen);

    RFal_NFC_Dep_ClearCounters();

    if ((&gNfcip.rxBuf[gNfcip.rxBufPaylPos] != &gNfcip.rxBuf[RFAL_NFCDEP_DEP_HEADER + optHdrLen]) && (*outActRxLen > 0U)) {
      memmove(&gNfcip.rxBuf[gNfcip.rxBufPaylPos], &gNfcip.rxBuf[RFAL_NFCDEP_DEP_HEADER + optHdrLen], *outActRxLen);
    }


    /*******************************************************************************/
    /* Check if Initiator is indicating chaining MI                                */
    /*******************************************************************************/
    if (nfcip_PFBisIMI(rxPFB)) {
      gNfcip.isRxChaining = true;
      *outIsChaining      = true;

      nfcipLogD(" NFCIP(T) Rcvd IPDU OK w MI -> ACK \r\n");
      ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBRPDU_ACK(gNfcip.pni), gNfcip.rxBuf[rxMsgIt++]);
      if(ret < NFC_OK)
      {
        return ret;
      }

      gNfcip.pni = nfcip_PNIInc(gNfcip.pni);

      return NFC_Again;  /* Send Again signalling to run again, but some chaining data has arrived*/
    } else {
      if (gNfcip.isRxChaining) {
        nfcipLogI(" NFCIP(T) Rcvd last IPDU chaining finished \r\n");
      }

      /*******************************************************************************/
      /* Reception done, send to DH and start RTOX timer                             */
      /*******************************************************************************/
      nfcipTimerStart(gNfcip.RTOXTimer, nfcipRTOXAdjust(nfcipConv1FcToMs(RFal_NFC_Dep_WT2RWT(gNfcip.cfg.to))));
      gNfcip.state = NFCIP_ST_TARG_DEP_RTOX;

      gNfcip.isRxChaining = false;
      ret = NFC_OK;                            /* Data exchange done */
    }
  }
  return ret;
}

void RFal_NFC_Dep_Config(const RFal_NFC_Dep_Configs *cfg)
{
  if (cfg == NULL) {
    return;
  }

  memcpy(&gNfcip.cfg, cfg, sizeof(RFal_NFC_Dep_Configs));          /* Copy given config to local       */

  gNfcip.cfg.to   = (RFAL_NFCDEP_WT_TRG_MAX < gNfcip.cfg.to) ? RFAL_NFCDEP_WT_TRG_MAX : gNfcip.cfg.to;    /* Ensure proper WT value           */
  gNfcip.cfg.did  = nfcip_DIDMax(gNfcip.cfg.did);                  /* Ensure proper DID value          */
  gNfcip.fsc      = RFal_NFC_Dep_LR2FS(gNfcip.cfg.lr);                /* Calculate FSC based on given LR  */

  gNfcip.state = ((gNfcip.cfg.role ==  RFAL_NFCDEP_ROLE_TARGET) ? NFCIP_ST_TARG_WAIT_ATR : NFCIP_ST_INIT_IDLE);
}

NFC_OpResult RFal_NFC_Dep_Run(uint16_t *outActRxLen, bool *outIsChaining)
{
  NFC_OpResult ret;

  ret = NFC_Syntax;

  nfcipLogD(" NFCIP Run() state: %d \r\n", gNfcip.state);

  switch (gNfcip.state) {
    /*******************************************************************************/
    case NFCIP_ST_IDLE:
    case NFCIP_ST_INIT_DEP_IDLE:
    case NFCIP_ST_TARG_DEP_IDLE:
    case NFCIP_ST_TARG_DEP_SLEEP:
      return NFC_OK;

    /*******************************************************************************/
    case NFCIP_ST_INIT_DEP_TX:

      nfcipLogD(" NFCIP(I) Tx PNI: %d txLen: %d \r\n", gNfcip.pni, gNfcip.txBufLen);
      ret = RFal_NFC_Dep_Tx(NFCIP_CMD_DEP_REQ, gNfcip.txBuf, &gNfcip.txBuf[gNfcip.txBufPaylPos], gNfcip.txBufLen, nfcip_PFBIPDU(gNfcip.pni), (gNfcip.cfg.fwt + gNfcip.cfg.dFwt));

      switch (ret) {
        case NFC_OK:
          gNfcip.state = NFCIP_ST_INIT_DEP_RX;
          break;
        case NFC_InvalidParameter:
        default:
          gNfcip.state = NFCIP_ST_INIT_DEP_IDLE;
          return ret;


      }
    /* fall through */

    /*******************************************************************************/
    case NFCIP_ST_INIT_DEP_RX:          /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      ret = RFal_NFC_Dep_DataRx(false);

      if (ret != NFC_Busy) {
        ret = RFal_NFC_Dep_InitiatorHandle(ret, ((gNfcip.rxRcvdLen != NULL) ? *gNfcip.rxRcvdLen : 0U), outActRxLen, outIsChaining);
      }

      break;

    /*******************************************************************************/
    case NFCIP_ST_TARG_DEP_RTOX:

      if (!nfcipTimerisExpired(gNfcip.RTOXTimer)) {                     /* Do nothing until RTOX timer has expired */
        return NFC_Busy;
      }

      /* If we cannot send a RTOX raise a Timeout error so that we do not
       * hold the field On forever in AP2P                                  */
      if (nfcipIsRTOXReqDisabled(gNfcip.cfg.oper)) {
        /* We should re-Enable Rx, and measure time between our field Off to
         * either report link loss or recover               #287          */
        nfcipLogI(" NFCIP(T) RTOX not sent due to config, NOT reenabling Rx \r\n");
        return NFC_SlaveTimeout;
      }

      if (gNfcip.cntRTOXRetrys++ > RFAL_NFCDEP_MAX_RTOX_RETRYS) {             /* Check maximum consecutive RTOX requests */
        return NFC_ProtocolError;
      }

      nfcipLogI(" NFCIP(T) RTOX sent \r\n");

      gNfcip.lastRTOX = nfcip_RTOXTargMax(gNfcip.cfg.to);               /* Calculate requested RTOX value, and send it */
      ret = RFal_NFC_Dep_ControlMsg(nfcip_PFBSPDU_TO(), gNfcip.lastRTOX);
      if(ret < NFC_OK)
      {
        return ret;
      }

      /* Set waiting for RTOX Ack Flag */
      gNfcip.isWait4RTOX = true;

      gNfcip.state = NFCIP_ST_TARG_DEP_RX;                              /* Go back to Rx to process RTOX ack       */
      return NFC_Busy;

    /*******************************************************************************/
    case NFCIP_ST_TARG_DEP_TX:

      nfcipLogD(" NFCIP(T) Tx PNI: %d txLen: %d \r\n", gNfcip.pni, gNfcip.txBufLen);
      ret = RFal_NFC_Dep_Tx(NFCIP_CMD_DEP_RES, gNfcip.txBuf, &gNfcip.txBuf[gNfcip.txBufPaylPos], gNfcip.txBufLen, nfcip_PFBIPDU(gNfcip.pni), NFCIP_NO_FWT);

      /* Clear flags */
      gNfcip.isTxPending = false;
      gNfcip.isWait4RTOX = false;

      /* Digital 1.0 14.12.3.4 Increment the current PNI after Tx */
      gNfcip.pni = nfcip_PNIInc(gNfcip.pni);

      switch (ret) {
        case NFC_OK:
          gNfcip.state = NFCIP_ST_TARG_DEP_RX;                        /* All OK, goto Rx state          */
          break;
        case NFC_InvalidParameter:
        default:
          gNfcip.state = NFCIP_ST_TARG_DEP_IDLE;                      /* Upon Tx error, goto IDLE state */
          return ret;
      }
    /* fall through */

    /*******************************************************************************/
    case NFCIP_ST_TARG_DEP_RX:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      if (gNfcip.isReqPending) {   /* if already has Data should be from a DEP from nfcipTargetHandleActivation()  */
        nfcipLogD(" NFCIP(T) Skipping Rx Using DEP from Activation \r\n");

        gNfcip.isReqPending = false;
        ret = NFC_OK;
      } else {
        ret = RFal_NFC_Dep_DataRx(false);
      }

      if (ret != NFC_Busy) {
        ret = RFal_NFC_Dep_TargetHandleRX(ret, outActRxLen, outIsChaining);
      }

      break;

    /*******************************************************************************/
    default:
      /* MISRA 16.4: no empty default statement (a comment being enough) */
      break;
  }
  return ret;
}

/*******************************************************************************/
void RFal_NFC_Dep_SetDeactivatingCallback(RFal_NFC_Dep_DeactCallback pFunc)
{
  gNfcip.isDeactivating = pFunc;
}

/*******************************************************************************/
void RFal_NFC_Dep_Init(void)
{
  nfcipLogD(" NFCIP Ini() \r\n");

  gNfcip.state          = NFCIP_ST_IDLE;
  gNfcip.isDeactivating = NULL;

  gNfcip.isTxPending    = false;
  gNfcip.isWait4RTOX    = false;
  gNfcip.isReqPending   = false;


  gNfcip.cfg.oper  = (RFAL_NFCDEP_OPER_FULL_MI_DIS | RFAL_NFCDEP_OPER_EMPTY_DEP_EN | RFAL_NFCDEP_OPER_ATN_EN | RFAL_NFCDEP_OPER_RTOX_REQ_EN);

  gNfcip.cfg.did   = RFAL_NFCDEP_DID_NO;
  gNfcip.cfg.nad   = RFAL_NFCDEP_NAD_NO;

  gNfcip.cfg.br    = RFAL_NFCDEP_Bx_NO_HIGH_BR;
  gNfcip.cfg.bs    = RFAL_NFCDEP_Bx_NO_HIGH_BR;

  gNfcip.cfg.lr    = RFAL_NFCDEP_LR_254;
  gNfcip.fsc       = RFal_NFC_Dep_LR2FS(gNfcip.cfg.lr);

  gNfcip.cfg.gbLen = 0;

  gNfcip.cfg.fwt   = NFCIP_RWT_ACTIVATION;
  gNfcip.cfg.dFwt  = RFAL_NFCDEP_WT_DELTA;

  gNfcip.pni       = 0;
  gNfcip.RTOXTimer = 0;

  gNfcip.PDUTxPos = 0;
  gNfcip.PDURxPos = 0;
  gNfcip.PDUParam.rxLen = NULL;
  gNfcip.PDUParam.rxBuf = NULL;
  gNfcip.PDUParam.txBuf = NULL;

  RFal_NFC_Dep_ClearCounters();
}

void RFal_NFC_Dep_SetDEPParams(RFal_NFC_Dep_DEPParams *DEPParams)
{
  nfcipLogD(" NFCIP SetDEP() txLen: %d \r\n", DEPParams->txBufLen);

  gNfcip.isTxChaining = DEPParams->txChaining;
  gNfcip.txBuf        = DEPParams->txBuf;
  gNfcip.rxBuf        = DEPParams->rxBuf;
  gNfcip.txBufLen     = DEPParams->txBufLen;
  gNfcip.rxBufLen     = DEPParams->rxBufLen;
  gNfcip.txBufPaylPos = DEPParams->txBufPaylPos;
  gNfcip.rxBufPaylPos = DEPParams->rxBufPaylPos;

  if (DEPParams->did != RFAL_NFCDEP_DID_KEEP) {
    gNfcip.cfg.did  = nfcip_DIDMax(DEPParams->did);
  }

  gNfcip.cfg.fwt      = DEPParams->fwt;
  gNfcip.cfg.dFwt     = DEPParams->dFwt;
  gNfcip.fsc          = DEPParams->fsc;

  if (gNfcip.cfg.role == RFAL_NFCDEP_ROLE_TARGET) {
    /* If there's any data to be sent go for Tx */
    if (DEPParams->txBufLen > 0U) {
      /* Ensure that an RTOX Ack is not being expected at moment */
      if (!gNfcip.isWait4RTOX) {
        gNfcip.state = NFCIP_ST_TARG_DEP_TX;
        return;
      } else {
        /* If RTOX Ack is expected, signal a pending Tx to be transmitted right after */
        gNfcip.isTxPending = true;
        nfcipLogW(" NFCIP(T) Waiting RTOX, queueing outgoing DEP Block \r\n");
      }
    }

    /*Digital 1.0  14.12.4.1 In target mode the first PDU MUST be sent by the Initiator */
    gNfcip.state = NFCIP_ST_TARG_DEP_RX;
    return;
  }

  /* New data TxRx request clear previous error counters for consecutive TxRx without resetting communication/protocol layer*/
  RFal_NFC_Dep_ClearCounters();

  gNfcip.state = NFCIP_ST_INIT_DEP_TX;
}

/*******************************************************************************/
bool RFal_NFC_Dep_TargetRcvdATR(void)
{
  return ((gNfcip.cfg.role == RFAL_NFCDEP_ROLE_TARGET) && nfcipIsTarget(gNfcip.state) && (gNfcip.state > NFCIP_ST_TARG_WAIT_ATR));
}

/*******************************************************************************/
bool RFal_NFC_Dep_IsAtrReq(const uint8_t *buf, uint16_t bufLen, uint8_t *nfcid3)
{
  uint8_t msgIt;

  msgIt = 0;

  if ((bufLen < RFAL_NFCDEP_ATRREQ_MIN_LEN) || (bufLen > RFAL_NFCDEP_ATRREQ_MAX_LEN)) {
    return false;
  }

  if (buf[msgIt++] != NFCIP_REQ) {
    return false;
  }

  if (buf[msgIt++] != (uint8_t)NFCIP_CMD_ATR_REQ) {
    return false;
  }

  /* Output NFID3 if requested */
  if (nfcid3 != NULL) {
    memcpy(nfcid3, &buf[RFAL_NFCDEP_ATR_REQ_NFCID3_POS], RFAL_NFCDEP_NFCID3_LEN);
  }

  return true;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_TargetHandleActivation(RFal_NFC_Dep_Device *nfcDepDev, uint8_t *outBRS)
{
  NFC_OpResult ret;
  uint8_t    msgIt;
  uint8_t    txBuf[RFAL_NFCDEP_HEADER_PAD + NFCIP_PSLRES_LEN];

  /*******************************************************************************/
  /*  Check if we are in correct state                                           */
  /*******************************************************************************/
  if (gNfcip.state != NFCIP_ST_TARG_WAIT_ACTV) {
    return NFC_WrongState;
  }


  /*******************************************************************************/
  /*  Check required parameters                                                  */
  /*******************************************************************************/
  if (outBRS == NULL) {
    return NFC_InvalidParameter;
  }

  /*******************************************************************************/
  /*  Wait and process incoming cmd (PSL / DEP)                                  */
  /*******************************************************************************/
  ret = RFal_NFC_Dep_DataRx(false);

  if (ret != NFC_OK) {
    return ret;
  }

  if (gNfcip.rxBuf == NULL) {
    return NFC_IO_Error;
  }

  msgIt   = 0;
  *outBRS = RFAL_NFCDEP_BRS_MAINTAIN;                   /* set out BRS to be maintained */

  msgIt++;                                              /* Skip LEN byte                */

  if (gNfcip.rxBuf[msgIt++] != NFCIP_REQ) {
    return NFC_ProtocolError;
  }

  if (gNfcip.rxBuf[msgIt] == (uint8_t)NFCIP_CMD_PSL_REQ) {
    msgIt++;

    if (gNfcip.rxBuf[msgIt++] != gNfcip.cfg.did) {    /* Checking DID                 */
      return NFC_ProtocolError;
    }

    nfcipLogI(" NFCIP(T) PSL REQ rcvd \r\n");

    *outBRS = gNfcip.rxBuf[msgIt++];                  /* assign output BRS value      */

    /* Store FSL(LR) and update current config */
    gNfcip.cfg.lr = (gNfcip.rxBuf[msgIt++] & RFAL_NFCDEP_LR_VAL_MASK);
    gNfcip.fsc    = RFal_NFC_Dep_LR2FS(gNfcip.cfg.lr);

    /*******************************************************************************/
    /* Update NFC-DDE Device info */
    if (nfcDepDev != NULL) {
      /* Update Bitrate info */
      /* PRQA S 4342 2 # MISRA 10.5 - Layout of enum rfalBitRate and definition of RFal_NFC_Dep_BRS2DSI guarantee no invalid enum values to be created */
      nfcDepDev->info.DSI = (RFal_BitRate)RFal_NFC_Dep_BRS2DSI(*outBRS);     /* DSI codes the bit rate from Initiator to Target */
      nfcDepDev->info.DRI = (RFal_BitRate)RFal_NFC_Dep_BRS2DRI(*outBRS);     /* DRI codes the bit rate from Target to Initiator */

      /* Update Length Reduction and Frame Size */
      nfcDepDev->info.LR = gNfcip.cfg.lr;
      nfcDepDev->info.FS = gNfcip.fsc;

      /* Update PPi byte */
      nfcDepDev->activation.Initiator.ATR_REQ.PPi &= ~RFAL_NFCDEP_PP_LR_MASK;
      nfcDepDev->activation.Initiator.ATR_REQ.PPi |= RFal_NFC_Dep_LR2PP(gNfcip.cfg.lr);
    }
    RFal_SetBitRate(RFAL_BR_KEEP, gNfcip.nfcDepDev->info.DSI);

    ret = RFal_NFC_Dep_Tx(NFCIP_CMD_PSL_RES, txBuf, NULL, 0, 0, NFCIP_NO_FWT);
    if(ret < NFC_OK)
    {
      return ret;
    }
  } else {
    if (gNfcip.rxBuf[msgIt] == (uint8_t)NFCIP_CMD_DEP_REQ) {
      msgIt++;

      /*******************************************************************************/
      /* Digital 1.0 14.12.3.1 PNI must be initialized to 0 */
      if (nfcip_PBF_PNI(gNfcip.rxBuf[msgIt]) != 0U) {
        return NFC_ProtocolError;
      }

      /*******************************************************************************/
      /* Digital 1.0 14.8.2.1  check if DID is expected and match -> Protocol Error  */
      if (nfcip_PFBhasDID(gNfcip.rxBuf[ msgIt])) {
        if (gNfcip.rxBuf[++msgIt] != gNfcip.cfg.did) {
          return NFC_ProtocolError;
        }
      } else if (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO) {       /* DID expected but not rcv */
        return NFC_ProtocolError;
      } else {
        /* MISRA 15.7 - Empty else */
      }
    }

    /* Signal Request pending to be digested on normal Handling (DEP_REQ, DSL_REQ, RLS_REQ) */
    gNfcip.isReqPending = true;
  }

  gNfcip.state = NFCIP_ST_TARG_DEP_RX;
  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_ATR(const RFal_NFC_Dep_AtrParam *param, RFal_NFC_Dep_AtrRes *atrRes, uint8_t *atrResLen)
{
  NFC_OpResult        ret;
  RFal_NFC_Dep_Configs cfg;
  uint16_t          rxLen;
  uint8_t           msgIt;
  uint8_t           txBuf[RFAL_NFCDEP_ATRREQ_MAX_LEN];
  uint8_t           rxBuf[NFCIP_ATRRES_BUF_LEN];


  if ((param == NULL) || (atrRes == NULL) || (atrResLen == NULL)) {
    return NFC_InvalidParameter;
  }

  memset(&cfg, 0x00, sizeof(RFal_NFC_Dep_Configs));

  /*******************************************************************************/
  /* Configure NFC-DEP layer                                                     */
  /*******************************************************************************/

  cfg.did  = param->DID;
  cfg.nad  = param->NAD;
  cfg.fwt  = RFAL_NFCDEP_MAX_FWT;
  cfg.dFwt = RFAL_NFCDEP_WT_DELTA;
  cfg.br   = param->BR;
  cfg.bs   = param->BS;
  cfg.lr   = param->LR;
  cfg.to   = RFAL_NFCDEP_WT_TRG_MAX;            /* Not used in Initiator mode */


  cfg.gbLen = param->GBLen;
  if (cfg.gbLen > 0U) {                         /* MISRA 21.18 */
    memcpy(cfg.gb, param->GB, cfg.gbLen);
  }

  cfg.nfcidLen = param->nfcidLen;
  if (cfg.nfcidLen > 0U) {                      /* MISRA 21.18 */
    memcpy(cfg.nfcid, param->nfcid, cfg.nfcidLen);
  }

  cfg.role     = RFAL_NFCDEP_ROLE_INITIATOR;
  cfg.oper     = param->operParam;
  cfg.commMode = param->commMode;

  RFal_NFC_Dep_Init();
  RFal_NFC_Dep_Config(&cfg);

  /*******************************************************************************/
  /* Send ATR_REQ                                                                */
  /*******************************************************************************/

  ret = RFal_NFC_Dep_TxRx(NFCIP_CMD_ATR_REQ, txBuf, nfcipRWTActivation(), NULL, 0, rxBuf, NFCIP_ATRRES_BUF_LEN, &rxLen);
  if(ret < NFC_OK)
  {
    return ret;
  }


  /*******************************************************************************/
  /* ATR sent, check response                                                    */
  /*******************************************************************************/
  msgIt = 0;
  rxLen = ((uint16_t)rxBuf[msgIt++] - RFAL_NFCDEP_LEN_LEN);                           /* use LEN byte             */

  if ((rxLen < RFAL_NFCDEP_ATRRES_MIN_LEN) || (rxLen > RFAL_NFCDEP_ATRRES_MAX_LEN)) { /* Checking length: ATR_RES */
    return NFC_ProtocolError;
  }

  if (rxBuf[msgIt++] != NFCIP_RES) {                                                  /* Checking if is a response*/
    return NFC_ProtocolError;
  }

  if (rxBuf[msgIt++] != (uint8_t)NFCIP_CMD_ATR_RES) {                                 /* Checking if is a ATR RES */
    return NFC_ProtocolError;
  }

  memcpy((uint8_t *)atrRes, (rxBuf + RFAL_NFCDEP_LEN_LEN), rxLen);
  *atrResLen = (uint8_t)rxLen;

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_PSL(uint8_t BRS, uint8_t FSL)
{
  NFC_OpResult ret;
  uint16_t   rxLen;
  uint8_t    msgIt;
  uint8_t    txBuf[NFCIP_PSLREQ_LEN + NFCIP_PSLPAY_LEN];
  uint8_t    rxBuf[NFCIP_PSLRES_LEN];

  msgIt = NFCIP_PSLREQ_LEN;

  txBuf[msgIt++] = BRS;
  txBuf[msgIt++] = FSL;

  /*******************************************************************************/
  /* Send PSL REQ and wait for response                                          */
  /*******************************************************************************/
  ret = RFal_NFC_Dep_TxRx(NFCIP_CMD_PSL_REQ, txBuf, (gNfcip.cfg.fwt + gNfcip.cfg.dFwt), &txBuf[NFCIP_PSLREQ_LEN], (msgIt - NFCIP_PSLREQ_LEN), rxBuf, NFCIP_PSLRES_LEN, &rxLen);
  if(ret < NFC_OK)
  {
    return ret;
  }


  /*******************************************************************************/
  /* PSL sent, check response                                                    */
  /*******************************************************************************/
  msgIt = 0;
  rxLen = (uint16_t)(rxBuf[msgIt++]);                /* use LEN byte                   */

  if (rxLen < NFCIP_PSLRES_LEN) {                    /* Checking length: LEN + RLS_RES */
    return NFC_ProtocolError;
  }

  if (rxBuf[msgIt++] != NFCIP_RES) {                 /* Checking if is a response      */
    return NFC_ProtocolError;
  }

  if (rxBuf[msgIt++] != (uint8_t)NFCIP_CMD_PSL_RES) { /* Checking if is a PSL RES       */
    return NFC_ProtocolError;
  }

  if (rxBuf[msgIt++] != gNfcip.cfg.did) {            /* Checking DID                   */
    return NFC_ProtocolError;
  }

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_DSL(void)
{
  NFC_OpResult ret;
  uint8_t  txBuf[ RFAL_NFCDEP_HEADER_PAD + NFCIP_DSLREQ_LEN];
  uint8_t  rxBuf[NFCIP_DSLRES_LEN];
  uint8_t  rxMsgIt;
  uint16_t rxLen = 0;

  if (gNfcip.cfg.role == RFAL_NFCDEP_ROLE_TARGET) {
    return NFC_OK;                                  /* Target has no deselect procedure */
  }

  /* Repeating a DSL REQ is optional, not doing it */
  ret = RFal_NFC_Dep_TxRx(NFCIP_CMD_DSL_REQ, txBuf, (gNfcip.cfg.fwt + gNfcip.cfg.dFwt), NULL, 0, rxBuf, (uint16_t)sizeof(rxBuf), &rxLen);
  if(ret < NFC_OK)
  {
    return ret;
  }

  /*******************************************************************************/
  rxMsgIt = 0;

  if (rxBuf[rxMsgIt++] < NFCIP_DSLRES_MIN) {            /* Checking length: LEN + DSL_RES */
    return NFC_ProtocolError;
  }

  if (rxBuf[rxMsgIt++] != NFCIP_RES) {                  /* Checking if is a response      */
    return NFC_ProtocolError;
  }

  if (rxBuf[rxMsgIt++] != (uint8_t)NFCIP_CMD_DSL_RES) { /* Checking if is DSL RES          */
    return NFC_ProtocolError;
  }

  if (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO) {
    if (rxBuf[rxMsgIt++] != gNfcip.cfg.did) {
      return NFC_ProtocolError;
    }
  }

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_RLS(void)
{
  NFC_OpResult ret;
  uint8_t    txBuf[RFAL_NFCDEP_HEADER_PAD + NFCIP_RLSREQ_LEN];
  uint8_t    rxBuf[NFCIP_RLSRES_LEN];
  uint8_t    rxMsgIt;
  uint16_t   rxLen = 0;

  if (gNfcip.cfg.role == RFAL_NFCDEP_ROLE_TARGET) {  /* Target has no release procedure */
    return NFC_OK;
  }

  /* Repeating a RLS REQ is optional, not doing it */
  ret = RFal_NFC_Dep_TxRx(NFCIP_CMD_RLS_REQ, txBuf, (gNfcip.cfg.fwt + gNfcip.cfg.dFwt), NULL, 0, rxBuf, (uint16_t)sizeof(rxBuf), &rxLen);
  if(ret < NFC_OK)
  {
    return ret;
  }

  /*******************************************************************************/
  rxMsgIt = 0;

  if (rxBuf[rxMsgIt++] < NFCIP_RLSRES_MIN) {            /* Checking length: LEN + RLS_RES */
    return NFC_ProtocolError;
  }

  if (rxBuf[rxMsgIt++] != NFCIP_RES) {                  /* Checking if is a response      */
    return NFC_ProtocolError;
  }

  if (rxBuf[rxMsgIt++] != (uint8_t)NFCIP_CMD_RLS_RES) { /* Checking if is RLS RES         */
    return NFC_ProtocolError;
  }

  if (gNfcip.cfg.did != RFAL_NFCDEP_DID_NO) {
    if (rxBuf[rxMsgIt++] != gNfcip.cfg.did) {
      return NFC_ProtocolError;
    }
  }

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_InitiatorHandleActivation(RFal_NFC_Dep_AtrParam *param, RFal_BitRate desiredBR, RFal_NFC_Dep_Device *nfcDepDev)
{
  NFC_OpResult ret;
  uint8_t    maxRetyrs;
  uint8_t    PSL_BRS;
  uint8_t    PSL_FSL;
  bool       sendPSL;

  if ((param == NULL) || (nfcDepDev == NULL)) {
    return NFC_InvalidParameter;
  }

  param->NAD = RFAL_NFCDEP_NAD_NO;          /* Digital 1.1  16.6.2.9  Initiator SHALL NOT use NAD */
  maxRetyrs  = NFCIP_ATR_RETRY_MAX;

  /*******************************************************************************/
  /* Send ATR REQ and wait for response                                          */
  /*******************************************************************************/
  do { /* Upon transmission error ATR REQ should be retried */

    ret = RFal_NFC_Dep_ATR(param, &nfcDepDev->activation.Target.ATR_RES, &nfcDepDev->activation.Target.ATR_RESLen);

    if (nfcipIsTransmissionError(ret)) {
      continue;
    }
    break;
  } while ((maxRetyrs--) != 0U);

  if (ret != NFC_OK) {
    return ret;
  }

  /*******************************************************************************/
  /* Compute NFC-DEP device with ATR_RES                                         */
  /*******************************************************************************/
  nfcDepDev->info.GBLen = (nfcDepDev->activation.Target.ATR_RESLen - RFAL_NFCDEP_ATRRES_MIN_LEN);
  nfcDepDev->info.DID   = nfcDepDev->activation.Target.ATR_RES.DID;
  nfcDepDev->info.NAD   = RFAL_NFCDEP_NAD_NO;                                      /* Digital 1.1  16.6.3.11 Initiator SHALL ignore b1 of PPt */
  nfcDepDev->info.LR    = RFal_NFC_Dep_PP2LR(nfcDepDev->activation.Target.ATR_RES.PPt);
  nfcDepDev->info.FS    = RFal_NFC_Dep_LR2FS(nfcDepDev->info.LR);
  nfcDepDev->info.WT    = (nfcDepDev->activation.Target.ATR_RES.TO & RFAL_NFCDEP_WT_MASK);
  nfcDepDev->info.FWT   = RFal_NFC_Dep_CalculateRWT(nfcDepDev->info.WT);
  nfcDepDev->info.dFWT  = RFAL_NFCDEP_WT_DELTA;

  RFal_GetBitRate(&nfcDepDev->info.DSI, &nfcDepDev->info.DRI);



  /*******************************************************************************/
  /* Check if a PSL needs to be sent                                                */
  /*******************************************************************************/
  sendPSL = false;
  PSL_BRS = RFal_NFC_Dep_Dx2BRS(nfcDepDev->info.DSI);    /* Set current bit rate divisor on both directions  */
  PSL_FSL = nfcDepDev->info.LR;                       /* Set current Frame Size                           */



  /* Activity 1.0  9.4.4.15 & 9.4.6.3   NFC-DEP Activation PSL
  *  Activity 2.0  9.4.4.17 & 9.4.6.6   NFC-DEP Activation PSL
  *
  *  PSL_REQ shall only be sent if desired bit rate is different from current (Activity 1.0)
  *  PSL_REQ shall be sent to update LR or bit rate  (Activity 2.0)
  * */

#if 0 /* PSL due to LR is disabled, can be enabled if desired*/
  /*******************************************************************************/
  /* Check Frame Size                                                            */
  /*******************************************************************************/
  if (gNfcip.cfg.lr < nfcDepDev->info.LR) { /* If our Length reduction is smaller */
    sendPSL = true;

    nfcDepDev->info.LR   = (nfcDepDev->info.LR < gNfcip.cfg.lr) ? nfcDepDev->info.LR : gNfcip.cfg.lr;

    gNfcip.cfg.lr = nfcDepDev->info.LR;                /* Update nfcip LR  to be used */
    gNfcip.fsc    = RFal_NFC_Dep_LR2FS(gNfcip.cfg.lr);    /* Update nfcip FSC to be used */

    PSL_FSL       = gNfcip.cfg.lr;                     /* Set LR to be sent           */

    nfcipLogI(" NFCIP(I) Frame Size differ, PSL new fsc: %d \r\n", gNfcip.fsc);
  }
#endif


  /*******************************************************************************/
  /* Check Baud rates                                                            */
  /*******************************************************************************/
  if ((nfcDepDev->info.DSI != desiredBR) && (desiredBR != RFAL_BR_KEEP)) {     /* if desired BR is different    */
    if (RFal_NFC_Dep_DxIsSupported((uint8_t)desiredBR, nfcDepDev->activation.Target.ATR_RES.BRt, nfcDepDev->activation.Target.ATR_RES.BSt)) {
      /* if desired BR is supported     */
      /* MISRA 13.5 */
      sendPSL = true;
      PSL_BRS = RFal_NFC_Dep_Dx2BRS(desiredBR);

      nfcipLogI(" NFCIP(I) BR differ, PSL BR: 0x%02X \r\n", PSL_BRS);
    }
  }


  /*******************************************************************************/
  if (sendPSL) {
    /* Apply target's FWT for PSL_REQ        Digital 2.2  17.11.2.5 */
    gNfcip.cfg.fwt = nfcDepDev->info.FWT;

    /*******************************************************************************/
    /* Send PSL REQ and wait for response                                          */
    /*******************************************************************************/
    ret = RFal_NFC_Dep_PSL(PSL_BRS, PSL_FSL);
    if(ret < NFC_OK)
    {
      return ret;
    }

    /* Check if bit rate has been changed */
    if (nfcDepDev->info.DSI != desiredBR) {
      /* Check if device was in Passive NFC-A and went to higher bit rates, use NFC-F */
      if ((nfcDepDev->info.DSI == RFAL_BR_106) && (gNfcip.cfg.commMode == RFAL_NFCDEP_COMM_PASSIVE)) {
        /* If Passive initialize NFC-F module */
        RFal_NFCF_PollerInit(desiredBR);
      }

      nfcDepDev->info.DRI  = desiredBR;  /* DSI Bit Rate coding from Initiator  to Target  */
      nfcDepDev->info.DSI  = desiredBR;  /* DRI Bit Rate coding from Target to Initiator   */

      RFal_SetBitRate(nfcDepDev->info.DSI, nfcDepDev->info.DRI);
    }


    return NFC_OK;   /* PSL has been sent    */
  }

  return NFC_OK;       /* No PSL has been sent */
}


/*******************************************************************************/
uint32_t RFal_NFC_Dep_CalculateRWT(uint8_t wt)
{
  /* Digital 1.0  14.6.3.8  &  Digital 1.1  16.6.3.9     */
  /* Digital 1.1  16.6.3.9 treat all RFU values as WT=14 */
  const uint8_t responseWaitTime = (RFAL_NFCDEP_WT_INI_MAX < wt) ? RFAL_NFCDEP_WT_INI_MAX : wt;

  return (uint32_t)RFal_NFC_Dep_WT2RWT(responseWaitTime);
}

NFC_OpResult RFal_NFC_Dep_DataTx(uint8_t *txBuf, uint16_t txBufLen, uint32_t fwt)
{
  return RFal_TransceiveBlockingTx(txBuf, txBufLen, gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen, (RFAL_TXRX_FLAGS_DEFAULT | (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_ON), ((fwt == NFCIP_NO_FWT) ? RFAL_FWT_NONE : fwt));
}

NFC_OpResult RFal_NFC_Dep_DataRx(bool blocking)
{
  NFC_OpResult ret;

  /* Perform Rx either blocking or non-blocking */
  if (blocking) {
    ret = RFal_TransceiveBlockingRx();
  } else {
    ret = RFal_GetTransceiveStatus();
  }

  if (ret != NFC_Busy) {
    if (gNfcip.rxRcvdLen != NULL) {
      (*gNfcip.rxRcvdLen) = RFal_ConvBitsToBytes(*gNfcip.rxRcvdLen);

      if ((ret == NFC_OK) && (gNfcip.rxBuf != NULL)) {
        /* Digital 1.1  16.4.1.3 - Length byte LEN SHALL have a value between 3 and 255 -> otherwise treat as Transmission Error *
         *                       - Ensure that actual received and frame length do match, otherwise treat as Transmission error  */
        if ((*gNfcip.rxRcvdLen != (uint16_t)*gNfcip.rxBuf) || (*gNfcip.rxRcvdLen < RFAL_NFCDEP_LEN_MIN) || (*gNfcip.rxRcvdLen > RFAL_NFCDEP_LEN_MAX)) {
          return NFC_FramingError;
        }
      }
    }
  }

  return ret;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_ListenStartActivation(const RFal_NFC_Dep_TargetParam *param, const uint8_t *atrReq, uint16_t atrReqLength, RFal_NFC_Dep_ListenActvParam rxParam)
{
  NFC_OpResult        ret;
  RFal_NFC_Dep_Configs cfg;


  if ((param == NULL) || (atrReq == NULL) || (rxParam.rxLen == NULL)) {
    return NFC_InvalidParameter;
  }


  /*******************************************************************************/
  /*  Check whether is a valid ATR_REQ Compute NFC-DEP device                    */
  if (!RFal_NFC_Dep_IsAtrReq(atrReq, atrReqLength, NULL)) {
    return NFC_InvalidParameter;
  }

  rxParam.nfcDepDev->activation.Initiator.ATR_REQLen = (uint8_t)atrReqLength;                   /* nfcipIsAtrReq() is already checking Min and Max buffer lengths */
  if (atrReqLength > 0U) {                                                                      /* MISRA 21.18 */
    memcpy((uint8_t *)&rxParam.nfcDepDev->activation.Initiator.ATR_REQ, atrReq, atrReqLength);
  }

  rxParam.nfcDepDev->info.GBLen = (uint8_t)(atrReqLength - RFAL_NFCDEP_ATRREQ_MIN_LEN);
  rxParam.nfcDepDev->info.DID   = rxParam.nfcDepDev->activation.Initiator.ATR_REQ.DID;
  rxParam.nfcDepDev->info.NAD   = RFAL_NFCDEP_NAD_NO;                        /* Digital 1.1  16.6.2.9  Initiator SHALL NOT use NAD */
  rxParam.nfcDepDev->info.LR    = RFal_NFC_Dep_PP2LR(rxParam.nfcDepDev->activation.Initiator.ATR_REQ.PPi);
  rxParam.nfcDepDev->info.FS    = RFal_NFC_Dep_LR2FS(rxParam.nfcDepDev->info.LR);
  rxParam.nfcDepDev->info.WT    = 0;
  rxParam.nfcDepDev->info.FWT   = NFCIP_NO_FWT;
  rxParam.nfcDepDev->info.dFWT  = NFCIP_NO_FWT;

  RFal_GetBitRate(&rxParam.nfcDepDev->info.DSI, &rxParam.nfcDepDev->info.DRI);


  /* Store Device Info location, updated upon a PSL  */
  gNfcip.nfcDepDev = rxParam.nfcDepDev;


  /*******************************************************************************/
  cfg.did = rxParam.nfcDepDev->activation.Initiator.ATR_REQ.DID;
  cfg.nad = RFAL_NFCDEP_NAD_NO;

  cfg.fwt   = RFAL_NFCDEP_MAX_FWT;
  cfg.dFwt  = RFAL_NFCDEP_WT_DELTA;

  cfg.br = param->brt;
  cfg.bs = param->bst;

  cfg.lr = RFal_NFC_Dep_PP2LR(param->ppt);

  cfg.gbLen = param->GBtLen;
  if (cfg.gbLen > 0U) {         /* MISRA 21.18 */
    memcpy(cfg.gb, param->GBt, cfg.gbLen);
  }

  cfg.nfcidLen = RFAL_NFCDEP_NFCID3_LEN;
  memcpy(cfg.nfcid, param->nfcid3, RFAL_NFCDEP_NFCID3_LEN);

  cfg.to = param->to;

  cfg.role     = RFAL_NFCDEP_ROLE_TARGET;
  cfg.oper     = param->operParam;
  cfg.commMode = param->commMode;

  RFal_NFC_Dep_Init();
  RFal_NFC_Dep_Config(&cfg);


  /*******************************************************************************/
  /*  Reply with ATR RES to Initiator                                            */
  /*******************************************************************************/
  gNfcip.rxBuf        = (uint8_t *)rxParam.rxBuf;
  gNfcip.rxBufLen     = sizeof(RFal_NFC_Dep_BufFormat);
  gNfcip.rxRcvdLen    = rxParam.rxLen;
  gNfcip.rxBufPaylPos = RFAL_NFCDEP_DEPREQ_HEADER_LEN;
  gNfcip.isChaining   = rxParam.isRxChaining;
  gNfcip.txBufPaylPos = RFAL_NFCDEP_DEPREQ_HEADER_LEN;

  ret = RFal_NFC_Dep_Tx(NFCIP_CMD_ATR_RES, (uint8_t *) gNfcip.rxBuf, NULL, 0, 0, NFCIP_NO_FWT);
  if(ret < NFC_OK)
  {
    return ret;
  }

  gNfcip.state = NFCIP_ST_TARG_WAIT_ACTV;

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_ListenGetActivationStatus(void)
{
  NFC_OpResult err;
  uint8_t    BRS;

  BRS = RFAL_NFCDEP_BRS_MAINTAIN;

  err = RFal_NFC_Dep_TargetHandleActivation(gNfcip.nfcDepDev, &BRS);

  switch (err) {
    case NFC_OK:

      if (BRS != RFAL_NFCDEP_BRS_MAINTAIN) {
        /* DSI codes the bit rate from Initiator to Target */
        /* DRI codes the bit rate from Target to Initiator */

        if (gNfcip.cfg.commMode == RFAL_NFCDEP_COMM_ACTIVE) {
          err = RFal_SetMode(RFAL_MODE_LISTEN_ACTIVE_P2P, gNfcip.nfcDepDev->info.DRI, gNfcip.nfcDepDev->info.DSI);
        } else {
          err = RFal_SetMode(((RFAL_BR_106 == gNfcip.nfcDepDev->info.DRI) ? RFAL_MODE_LISTEN_NFCA : RFAL_MODE_LISTEN_NFCF), gNfcip.nfcDepDev->info.DRI, gNfcip.nfcDepDev->info.DSI);
        }
        if(err < NFC_OK)
        {
          return err;
        }
      }
      break;

    case NFC_Busy:
      // do nothing
      break;

    case NFC_ProtocolError:
    default:
      // re-enable receiving of data
      nfcDepReEnableRx(gNfcip.rxBuf, gNfcip.rxBufLen, gNfcip.rxRcvdLen);
      break;
  } // if (no err)

  return err;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_StartTransceive(const RFal_NFC_Dep_TxRxParam *param)
{
  RFal_NFC_Dep_DEPParams nfcDepParams;

  nfcDepParams.txBuf        = (uint8_t *)param->txBuf;
  nfcDepParams.txBufLen     = param->txBufLen;
  nfcDepParams.txChaining   = param->isTxChaining;
  nfcDepParams.txBufPaylPos = RFAL_NFCDEP_DEPREQ_HEADER_LEN;  /* position in txBuf where actual outgoing data is located */
  nfcDepParams.did          = RFAL_NFCDEP_DID_KEEP;
  nfcDepParams.rxBufPaylPos = RFAL_NFCDEP_DEPREQ_HEADER_LEN;
  nfcDepParams.rxBuf        = (uint8_t *)param->rxBuf;
  nfcDepParams.rxBufLen     = sizeof(RFal_NFC_Dep_BufFormat);
  nfcDepParams.fsc          = param->FSx;
  nfcDepParams.fwt          = param->FWT;
  nfcDepParams.dFwt         = param->dFWT;

  gNfcip.rxRcvdLen          = param->rxLen;
  gNfcip.isChaining         = param->isRxChaining;

  RFal_NFC_Dep_SetDEPParams(&nfcDepParams);

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_GetTransceiveStatus(void)
{
  return RFal_NFC_Dep_Run(gNfcip.rxRcvdLen, gNfcip.isChaining);
}

/*******************************************************************************/
void RFal_NFC_Dep_Pdu2BLockParam(RFal_NFC_Dep_PduTxRxParam pduParam, RFal_NFC_Dep_TxRxParam *blockParam, uint16_t txPos, uint16_t rxPos)
{
  uint16_t maxInfLen;

  blockParam->DID    = pduParam.DID;
  blockParam->FSx    = pduParam.FSx;
  blockParam->FWT    = pduParam.FWT;
  blockParam->dFWT   = pduParam.dFWT;

  /* Calculate max INF/Payload to be sent to other device */
  maxInfLen  = (blockParam->FSx - (RFAL_NFCDEP_HEADER + RFAL_NFCDEP_DEP_PFB_LEN));
  maxInfLen += ((blockParam->DID != RFAL_NFCDEP_DID_NO) ? RFAL_NFCDEP_DID_LEN : 0U);


  if ((pduParam.txBufLen - txPos) > maxInfLen) {
    blockParam->isTxChaining = true;
    blockParam->txBufLen     = maxInfLen;
  } else {
    blockParam->isTxChaining = false;
    blockParam->txBufLen     = (pduParam.txBufLen - txPos);
  }

  /* TxBuf is moved to the beginning for every Block */
  blockParam->txBuf        = (RFal_NFC_Dep_BufFormat *)pduParam.txBuf;  /*  PRQA S 0310 # MISRA 11.3 - Intentional safe cast to avoiding large buffer duplication */
  blockParam->rxBuf        = pduParam.tmpBuf;                        /* Simply using the pdu buffer is not possible because of current ACK handling */
  blockParam->isRxChaining = &gNfcip.isPDURxChaining;
  blockParam->rxLen        = pduParam.rxLen;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_StartPduTransceive(RFal_NFC_Dep_PduTxRxParam param)
{
  RFal_NFC_Dep_TxRxParam txRxParam;

  /* Initialize and store APDU context */
  gNfcip.PDUParam = param;
  gNfcip.PDUTxPos = 0;
  gNfcip.PDURxPos = 0;

  /* Convert PDU TxRxParams to Block TxRxParams */
  RFal_NFC_Dep_Pdu2BLockParam(gNfcip.PDUParam, &txRxParam, gNfcip.PDUTxPos, gNfcip.PDURxPos);

  return RFal_NFC_Dep_StartTransceive(&txRxParam);
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_Dep_GetPduTransceiveStatus(void)
{
  NFC_OpResult          ret;
  RFal_NFC_Dep_TxRxParam txRxParam;

  ret = RFal_NFC_Dep_GetTransceiveStatus();
  switch (ret) {
    /*******************************************************************************/
    case NFC_OK:

      /* Check if we are still doing chaining on Tx */
      if (gNfcip.isTxChaining) {
        /* Add already Tx bytes */
        gNfcip.PDUTxPos += gNfcip.txBufLen;

        /* Convert APDU TxRxParams to I-Block TxRxParams */
        RFal_NFC_Dep_Pdu2BLockParam(gNfcip.PDUParam, &txRxParam, gNfcip.PDUTxPos, gNfcip.PDURxPos);

        if (txRxParam.txBufLen > 0U) {     /* MISRA 21.18 */
          /* Move next Block to beginning of APDU Tx buffer */
          memcpy(gNfcip.PDUParam.txBuf->pdu, &gNfcip.PDUParam.txBuf->pdu[gNfcip.PDUTxPos], txRxParam.txBufLen);
        }

        ret = RFal_NFC_Dep_StartTransceive(&txRxParam);
        if(ret < NFC_OK)
        {
          return ret;
        }

        return NFC_Busy;
      }

    /* PDU TxRx is done */
    /* fall through */

    /*******************************************************************************/
    case NFC_Again:       /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */


      /* Check if no PDU transceive has been started before (data from RFal_NFC_Dep_ListenStartActivation) */
      if (gNfcip.PDUParam.rxLen == NULL) {
        /* In Listen mode first chained packet cannot be retrieved via APDU interface */
        if (ret == NFC_Again) {
          return NFC_Unsupport;
        }

        /* TxRx is complete and full data is already available */
        return NFC_OK;
      }


      if ((*gNfcip.PDUParam.rxLen) > 0U) {   /* MISRA 21.18 */
        /* Ensure that data in tmpBuf still fits into PDU buffer */
        if ((uint16_t)((uint16_t)gNfcip.PDURxPos + (*gNfcip.PDUParam.rxLen)) > RFAL_FEATURE_NFC_DEP_PDU_MAX_LEN) {
          return NFC_MemoryError;
        }

        /* Copy chained packet from tmp buffer to PDU buffer */
        memcpy(&gNfcip.PDUParam.rxBuf->pdu[gNfcip.PDURxPos], gNfcip.PDUParam.tmpBuf->inf, *gNfcip.PDUParam.rxLen);
        gNfcip.PDURxPos += *gNfcip.PDUParam.rxLen;
      }

      /* Update output param rxLen */
      *gNfcip.PDUParam.rxLen = gNfcip.PDURxPos;

      /* Wait for following Block or PDU TxRx is done */
      return ((ret == NFC_Again) ? NFC_Busy : NFC_OK);

    /*******************************************************************************/
    default:
      /* MISRA 16.4: no empty default statement (a comment being enough) */
      break;
  }

  return ret;
}
