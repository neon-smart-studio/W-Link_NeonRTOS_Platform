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

#ifndef RFAL_RF_H
#define RFAL_RF_H

#define RFAL_VERSION                               0x030000U                                    /*!< RFAL Current Version: v3.0.0                      */

#define RFAL_FWT_NONE                              0xFFFFFFFFU                                  /*!< Disabled FWT: Wait forever for a response         */
#define RFAL_GT_NONE                               RFAL_TIMING_NONE                             /*!< Disabled GT: No GT will be applied after Field On */

#define RFAL_TIMING_NONE                           0x00U                                        /*!< Timing disabled | Don't apply                     */

#define RFAL_1FC_IN_4096FC                         (uint32_t)4096U                              /*!< Number of 1/fc cycles in one 4096/fc              */
#define RFAL_1FC_IN_2048FC                         (uint32_t)2048U                              /*!< Number of 1/fc cycles in one 2048/fc              */
#define RFAL_1FC_IN_512FC                          (uint32_t)512U                               /*!< Number of 1/fc cycles in one 512/fc               */
#define RFAL_1FC_IN_64FC                           (uint32_t)64U                                /*!< Number of 1/fc cycles in one 64/fc                */
#define RFAL_1FC_IN_8FC                            (uint32_t)8U                                 /*!< Number of 1/fc cycles in one 8/fc                 */
#define RFAL_US_IN_MS                              (uint32_t)1000U                              /*!< Number of us in one ms                            */
#define RFAL_1MS_IN_1FC                            (uint32_t)13560U                             /*!< Number of 1/fc cycles in 1ms                      */
#define RFAL_BITS_IN_BYTE                          (uint16_t)8U                                 /*!< Number of bits in one byte                        */

#define RFAL_CRC_LEN                               2U                                           /*!< RF CRC LEN                                        */

/*! Default TxRx flags: Tx CRC automatic, Rx CRC removed, NFCIP1 mode off, AGC On, Tx Parity automatic, Rx Parity removed */
#define RFAL_TXRX_FLAGS_DEFAULT                    ( (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_AUTO | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_REMV | (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_OFF | (uint32_t)RFAL_TXRX_FLAGS_AGC_ON | (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_REMV | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_AUTO | (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO)

#define RFAL_LM_MASK_NFCA                          ((uint32_t)1U<<(uint8_t)RFAL_MODE_LISTEN_NFCA)        /*!< Bitmask for Listen Mode enabling NFCA    */
#define RFAL_LM_MASK_NFCB                          ((uint32_t)1U<<(uint8_t)RFAL_MODE_LISTEN_NFCB)        /*!< Bitmask for Listen Mode enabling NFCB    */
#define RFAL_LM_MASK_NFCF                          ((uint32_t)1U<<(uint8_t)RFAL_MODE_LISTEN_NFCF)        /*!< Bitmask for Listen Mode enabling NFCF    */
#define RFAL_LM_MASK_ACTIVE_P2P                    ((uint32_t)1U<<(uint8_t)RFAL_MODE_LISTEN_ACTIVE_P2P)  /*!< Bitmask for Listen Mode enabling AP2P    */

#define RFAL_LM_SENS_RES_LEN                       2U                                           /*!< NFC-A SENS_RES (ATQA) length                      */
#define RFAL_LM_SENSB_RES_LEN                      13U                                          /*!< NFC-B SENSB_RES (ATQB) length                     */
#define RFAL_LM_SENSF_RES_LEN                      19U                                          /*!< NFC-F SENSF_RES  length                           */
#define RFAL_LM_SENSF_SC_LEN                       2U                                           /*!< NFC-F System Code length                          */

#define RFAL_NFCID3_LEN                            10U                                          /*!< NFCID3 length                                     */
#define RFAL_NFCID2_LEN                            8U                                           /*!< NFCID2 length                                     */
#define RFAL_NFCID1_TRIPLE_LEN                     10U                                          /*!< NFCID1 length                                     */
#define RFAL_NFCID1_DOUBLE_LEN                     7U                                           /*!< NFCID1 length                                     */
#define RFAL_NFCID1_SINGLE_LEN                     4U                                           /*!< NFCID1 length                                     */


/*! Returns the maximum supported bit rate for RW mode. Caller must check if mode is supported before, as even if mode is not supported will return the min  */
#define RFAL_GetMaxBrRW()                     ( ((RFAL_SUPPORT_BR_RW_6780)  ? RFAL_BR_6780 : ((RFAL_SUPPORT_BR_RW_3390)  ? RFAL_BR_3390 : ((RFAL_SUPPORT_BR_RW_1695)  ? RFAL_BR_1695 : ((RFAL_SUPPORT_BR_RW_848)  ? RFAL_BR_848 : ((RFAL_SUPPORT_BR_RW_424)  ? RFAL_BR_424 : ((RFAL_SUPPORT_BR_RW_212)  ? RFAL_BR_212 : RFAL_BR_106 ) ) ) ) ) ) )

/*! Returns the maximum supported bit rate for AP2P mode. Caller must check if mode is supported before, as even if mode is not supported will return the min  */
#define RFAL_GetMaxBrAP2P()                   ( ((RFAL_SUPPORT_BR_AP2P_848) ? RFAL_BR_848  : ((RFAL_SUPPORT_BR_AP2P_424) ? RFAL_BR_424  : ((RFAL_SUPPORT_BR_AP2P_212) ? RFAL_BR_212  : RFAL_BR_106 ) ) ) )

/*! Returns the maximum supported bit rate for CE-A mode. Caller must check if mode is supported before, as even if mode is not supported will return the min  */
#define RFAL_GetMaxBrCEA()                    ( ((RFAL_SUPPORT_BR_CE_A_848) ? RFAL_BR_848  : ((RFAL_SUPPORT_BR_CE_A_424) ? RFAL_BR_424  : ((RFAL_SUPPORT_BR_CE_A_212) ? RFAL_BR_212  : RFAL_BR_106 ) ) ) )

/*! Returns the maximum supported bit rate for CE-B mode. Caller must check if mode is supported before, as even if mode is not supported will return the min  */
#define RFAL_GetMaxBrCEB()                    ( ((RFAL_SUPPORT_BR_CE_B_848) ? RFAL_BR_848  : ((RFAL_SUPPORT_BR_CE_B_424) ? RFAL_BR_424  : ((RFAL_SUPPORT_BR_CE_B_212) ? RFAL_BR_212  : RFAL_BR_106 ) ) ) )

/*! Returns the maximum supported bit rate for CE-F mode. Caller must check if mode is supported before, as even if mode is not supported will return the min  */
#define RFAL_GetMaxBrCEF()                    ( ((RFAL_SUPPORT_BR_CE_F_424) ? RFAL_BR_424  : RFAL_BR_212 ) )


#define RFAL_IsModeActiveComm( md )           ( ((md) == RFAL_MODE_POLL_ACTIVE_P2P) || ((md) == RFAL_MODE_LISTEN_ACTIVE_P2P) )                          /*!< Checks if mode md is Active Communication  */
#define RFAL_IsModePassiveComm( md )          ( !RFAL_IsModeActiveComm(md) )                                                                             /*!< Checks if mode md is Passive Communication */
#define RFAL_IsModePassiveListen( md )        ( ((md) == RFAL_MODE_LISTEN_NFCA) || ((md) == RFAL_MODE_LISTEN_NFCB) || ((md) == RFAL_MODE_LISTEN_NFCF) ) /*!< Checks if mode md is Passive Listen        */
#define RFAL_IsModePassivePoll( md )          ( RFAL_IsModePassiveComm(md) && (!RFAL_IsModePassiveListen(md)) )                                           /*!< Checks if mode md is Passive Poll          */


#define RFAL_Conv1fcTo8fc( t )                (uint32_t)( (uint32_t)(t) / RFAL_1FC_IN_8FC )                               /*!< Converts the given t from 1/fc to 8/fc     */
#define RFAL_Conv8fcTo1fc( t )                (uint32_t)( (uint32_t)(t) * RFAL_1FC_IN_8FC )                               /*!< Converts the given t from 8/fc to 1/fc     */

#define RFAL_Conv1fcTo64fc( t )               (uint32_t)( (uint32_t)(t) / RFAL_1FC_IN_64FC )                              /*!< Converts the given t from 1/fc  to 64/fc   */
#define RFAL_Conv64fcTo1fc( t )               (uint32_t)( (uint32_t)(t) * RFAL_1FC_IN_64FC )                              /*!< Converts the given t from 64/fc to 1/fc    */

#define RFAL_Conv1fcTo512fc( t )              (uint32_t)( (uint32_t)(t) / RFAL_1FC_IN_512FC )                             /*!< Converts the given t from 1/fc  to 512/fc  */
#define RFAL_Conv512fcTo1fc( t )              (uint32_t)( (uint32_t)(t) * RFAL_1FC_IN_512FC )                             /*!< Converts the given t from 512/fc to 1/fc   */

#define RFAL_Conv1fcTo2018fc( t )             (uint32_t)( (uint32_t)(t) / RFAL_1FC_IN_2048FC )                            /*!< Converts the given t from 1/fc to 2048/fc  */
#define RFAL_Conv2048fcTo1fc( t )             (uint32_t)( (uint32_t)(t) * RFAL_1FC_IN_2048FC )                            /*!< Converts the given t from 2048/fc to 1/fc  */

#define RFAL_Conv1fcTo4096fc( t )             (uint32_t)( (uint32_t)(t) / RFAL_1FC_IN_4096FC )                            /*!< Converts the given t from 1/fc to 4096/fc  */
#define RFAL_Conv4096fcTo1fc( t )             (uint32_t)( (uint32_t)(t) * RFAL_1FC_IN_4096FC )                            /*!< Converts the given t from 4096/fc to 1/fc  */

#define RFAL_Conv1fcToMs( t )                 (uint32_t)( (uint32_t)(t) / RFAL_1MS_IN_1FC )                               /*!< Converts the given t from 1/fc to ms       */
#define RFAL_ConvMsTo1fc( t )                 (uint32_t)( (uint32_t)(t) * RFAL_1MS_IN_1FC )                               /*!< Converts the given t from ms to 1/fc       */

#define RFAL_Conv1fcToUs( t )                 (uint32_t)( ((uint32_t)(t) * RFAL_US_IN_MS) / RFAL_1MS_IN_1FC)              /*!< Converts the given t from 1/fc to us       */
#define RFAL_ConvUsTo1fc( t )                 (uint32_t)( ((uint32_t)(t) * RFAL_1MS_IN_1FC) / RFAL_US_IN_MS)              /*!< Converts the given t from us to 1/fc       */

#define RFAL_Conv64fcToMs( t )                (uint32_t)( (uint32_t)(t) / (RFAL_1MS_IN_1FC / RFAL_1FC_IN_64FC) )          /*!< Converts the given t from 64/fc to ms      */
#define RFAL_ConvMsTo64fc( t )                (uint32_t)( (uint32_t)(t) * (RFAL_1MS_IN_1FC / RFAL_1FC_IN_64FC) )          /*!< Converts the given t from ms to 64/fc      */

#define RFAL_ConvBitsToBytes( n )             (uint16_t)( ((uint16_t)(n)+(RFAL_BITS_IN_BYTE-1U)) / (RFAL_BITS_IN_BYTE) )  /*!< Converts the given n from bits to bytes    */
#define RFAL_ConvBytesToBits( n )             (uint32_t)( (uint32_t)(n) * (RFAL_BITS_IN_BYTE) )                           /*!< Converts the given n from bytes to bits    */

#define RFAL_RunBlocking( e, fn )              do{ (e)=(fn);  RFAL_RfDev->RFAL_Worker(); }while( (e) == ERR_BUSY )                      /*!< Macro used for the blocking methods        */


/*! Computes a Transceive context \a ctx with default flags and the lengths
 * in bytes with the given arguments
 *    \a ctx   : Transceive context to be assigned
 *    \a tB    : txBuf the pointer to the buffer to be sent
 *    \a tBL   : txBuf length in bytes
 *    \a rB    : rxBuf the pointer to the buffer to place the received frame
 *    \a rBL   : rxBuf length in bytes
 *    \a rdL   : rxRcvdLen the pointer to place the rx length
 *    \a t     : FWT to be used on this transceive in 1/fc
 */
#define RFAL_CreateByteTxRxContext( ctx, tB, tBL, rB, rBL, rdL, t ) \
    (ctx).txBuf     = (uint8_t*)(tB);                                      \
    (ctx).txBufLen  = (uint16_t)RFAL_ConvBytesToBits(tBL);                  \
    (ctx).rxBuf     = (uint8_t*)(rB);                                      \
    (ctx).rxBufLen  = (uint16_t)RFAL_ConvBytesToBits(rBL);                  \
    (ctx).rxRcvdLen = (uint16_t*)(rdL);                                    \
    (ctx).flags     = (uint32_t)RFAL_TXRX_FLAGS_DEFAULT;                   \
    (ctx).fwt       = (uint32_t)(t);


/*! Computes a Transceive context \a ctx using lengths in bytes
 * with the given flags and arguments
 *    \a ctx   : Transceive context to be assigned
 *    \a tB    : txBuf the pointer to the buffer to be sent
 *    \a tBL   : txBuf length in bytes
 *    \a rB    : rxBuf the pointer to the buffer to place the received frame
 *    \a rBL   : rxBuf length in bytes
 *    \a rBL   : rxBuf length in bytes
 *    \a t     : FWT to be used on this transceive in 1/fc
 */
#define RFAL_CreateByteFlagsTxRxContext( ctx, tB, tBL, rB, rBL, rdL, fl, t ) \
    (ctx).txBuf     = (uint8_t*)(tB);                                       \
    (ctx).txBufLen  = (uint16_t)RFAL_ConvBytesToBits(tBL);                   \
    (ctx).rxBuf     = (uint8_t*)(rB);                                       \
    (ctx).rxBufLen  = (uint16_t)RFAL_ConvBytesToBits(rBL);                   \
    (ctx).rxRcvdLen = (uint16_t*)(rdL);                                     \
    (ctx).flags     = (uint32_t)(fl);                                       \
    (ctx).fwt       = (uint32_t)(t);


#define RFAL_LogE(...)
#define RFAL_LogW(...)
#define RFAL_LogI(...)
#define RFAL_LogD(...)

/* RFAL Guard Time (GT) default values                 */
#define    RFAL_GT_NFCA                      RFAL_ConvMsTo1fc(5U)     /*!< GTA  Digital 2.0  6.10.4.1 & B.2                                                                 */
#define    RFAL_GT_NFCB                      RFAL_ConvMsTo1fc(5U)     /*!< GTB  Digital 2.0  7.9.4.1  & B.3                                                                 */
#define    RFAL_GT_NFCF                      RFAL_ConvMsTo1fc(20U)    /*!< GTF  Digital 2.0  8.7.4.1  & B.4                                                                 */
#define    RFAL_GT_NFCV                      RFAL_ConvMsTo1fc(5U)     /*!< GTV  Digital 2.0  9.7.5.1  & B.5                                                                 */
#define    RFAL_GT_PICOPASS                  RFAL_ConvMsTo1fc(1U)     /*!< GT Picopass                                                                                      */
#define    RFAL_GT_AP2P                      RFAL_ConvMsTo1fc(5U)     /*!< TIRFG  Ecma 340  11.1.1                                                                          */
#define    RFAL_GT_AP2P_ADJUSTED             RFAL_ConvMsTo1fc(5U+25U) /*!< Adjusted GT for greater interoperability (Sony XPERIA P, Nokia N9, Huawei P2)                    */

/* RFAL Frame Delay Time (FDT) Listen default values   */
#define    RFAL_FDT_LISTEN_NFCA_POLLER       1172U    /*!< FDTA,LISTEN,MIN (n=9) Last bit: Logic "1" - tnn,min/2 Digital 1.1  6.10 ;  EMV CCP Spec Book D v2.01  4.8.1.3   */
#define    RFAL_FDT_LISTEN_NFCB_POLLER       1008U    /*!< TR0B,MIN         Digital 1.1  7.1.3 & A.3  ; EMV CCP Spec Book D v2.01  4.8.1.3 & Table A.5                     */
#define    RFAL_FDT_LISTEN_NFCF_POLLER       2672U    /*!< TR0F,LISTEN,MIN  Digital 1.1  8.7.1.1 & A.4                                                                     */
#define    RFAL_FDT_LISTEN_NFCV_POLLER       4310U    /*!< FDTV,LISTEN,MIN  t1 min       Digital 2.1  B.5  ;  ISO15693-3 2009  9.1                                          */
#define    RFAL_FDT_LISTEN_PICOPASS_POLLER   3400U    /*!< ISO15693 t1 min - observed adjustment                                                                           */
#define    RFAL_FDT_LISTEN_AP2P_POLLER       64U      /*!< FDT AP2P No actual FDTListen is required as fields switch and collision avoidance                               */
#define    RFAL_FDT_LISTEN_NFCA_LISTENER     1172U    /*!< FDTA,LISTEN,MIN  Digital 1.1  6.10                                                                              */
#define    RFAL_FDT_LISTEN_NFCB_LISTENER     1024U    /*!< TR0B,MIN         Digital 1.1  7.1.3 & A.3  ;  EMV CCP Spec Book D v2.01  4.8.1.3 & Table A.5                    */
#define    RFAL_FDT_LISTEN_NFCF_LISTENER     2688U    /*!< TR0F,LISTEN,MIN  Digital 2.1  8.7.1.1 & B.4                                                                     */
#define    RFAL_FDT_LISTEN_AP2P_LISTENER     64U      /*!< FDT AP2P No actual FDTListen exists as fields switch and collision avoidance                                    */

/*  RFAL Frame Delay Time (FDT) Poll default values    */
#define    RFAL_FDT_POLL_NFCA_POLLER         6780U    /*!< FDTA,POLL,MIN   Digital 1.1  6.10.3.1 & A.2                                                                     */
#define    RFAL_FDT_POLL_NFCA_T1T_POLLER     384U     /*!< RRDDT1T,MIN,B1  Digital 1.1  10.7.1 & A.5                                                                       */
#define    RFAL_FDT_POLL_NFCB_POLLER         6780U    /*!< FDTB,POLL,MIN = TR2B,MIN,DEFAULT Digital 1.1 7.9.3 & A.3  ;  EMVCo 3.0 FDTB,PCD,MIN  Table A.5                  */
#define    RFAL_FDT_POLL_NFCF_POLLER         6800U    /*!< FDTF,POLL,MIN   Digital 2.1  8.7.3 & B.4                                                                        */
#define    RFAL_FDT_POLL_NFCV_POLLER         4192U    /*!< FDTV,POLL  Digital 2.1  9.7.3.1  & B.5                                                                          */
#define    RFAL_FDT_POLL_PICOPASS_POLLER     1790U    /*!< FDT Max                                                                                                         */
#define    RFAL_FDT_POLL_AP2P_POLLER         6800U    /*!< AP2P inhere FDT from the Technology used (use longest: TR0F,POLL,MIN + TR1F)     Digital 2.2  17.11.1           */

/*! RFal modes    */
typedef enum {
  RFAL_MODE_NONE                   = 0,    /*!< No mode selected/defined                                         */
  RFAL_MODE_POLL_NFCA              = 1,    /*!< Mode to perform as NFCA (ISO14443A) Poller (PCD)                 */
  RFAL_MODE_POLL_NFCA_T1T          = 2,    /*!< Mode to perform as NFCA T1T (Topaz) Poller (PCD)                 */
  RFAL_MODE_POLL_NFCB              = 3,    /*!< Mode to perform as NFCB (ISO14443B) Poller (PCD)                 */
  RFAL_MODE_POLL_B_PRIME           = 4,    /*!< Mode to perform as B' Calypso (Innovatron) (PCD)                 */
  RFAL_MODE_POLL_B_CTS             = 5,    /*!< Mode to perform as CTS Poller (PCD)                              */
  RFAL_MODE_POLL_NFCF              = 6,    /*!< Mode to perform as NFCF (FeliCa) Poller (PCD)                    */
  RFAL_MODE_POLL_NFCV              = 7,    /*!< Mode to perform as NFCV (ISO15963) Poller (PCD)                  */
  RFAL_MODE_POLL_PICOPASS          = 8,    /*!< Mode to perform as PicoPass / iClass Poller (PCD)                */
  RFAL_MODE_POLL_ACTIVE_P2P        = 9,    /*!< Mode to perform as Active P2P (ISO18092) Initiator               */
  RFAL_MODE_LISTEN_NFCA            = 10,   /*!< Mode to perform as NFCA (ISO14443A) Listener (PICC)              */
  RFAL_MODE_LISTEN_NFCB            = 11,   /*!< Mode to perform as NFCA (ISO14443B) Listener (PICC)              */
  RFAL_MODE_LISTEN_NFCF            = 12,   /*!< Mode to perform as NFCA (ISO15963) Listener (PICC)               */
  RFAL_MODE_LISTEN_ACTIVE_P2P      = 13    /*!< Mode to perform as Active P2P (ISO18092) Target                  */
} RFal_Mode;


/*! RFal Bit rates    */
typedef enum {
  RFAL_BR_106                      = 0,    /*!< Bit Rate 106 kbit/s (fc/128)                                     */
  RFAL_BR_212                      = 1,    /*!< Bit Rate 212 kbit/s (fc/64)                                      */
  RFAL_BR_424                      = 2,    /*!< Bit Rate 424 kbit/s (fc/32)                                      */
  RFAL_BR_848                      = 3,    /*!< Bit Rate 848 kbit/s (fc/16)                                      */
  RFAL_BR_1695                     = 4,    /*!< Bit Rate 1695 kbit/s (fc/8)                                      */
  RFAL_BR_3390                     = 5,    /*!< Bit Rate 3390 kbit/s (fc/4)                                      */
  RFAL_BR_6780                     = 6,    /*!< Bit Rate 6780 kbit/s (fc/2)                                      */
  RFAL_BR_13560                    = 7,    /*!< Bit Rate 13560 kbit/s (fc)                                       */
  RFAL_BR_211p88                   = 0xE9, /*!< Bit Rate 211,88 kbit/s (fc/64) Fast Mode VICC->VCD               */
  RFAL_BR_105p94                   = 0xEA, /*!< Bit Rate 105,94 kbit/s (fc/128) Fast Mode VICC->VCD              */
  RFAL_BR_52p97                    = 0xEB, /*!< Bit Rate 52.97 kbit/s (fc/256) Fast Mode VICC->VCD               */
  RFAL_BR_26p48                    = 0xEC, /*!< Bit Rate 26,48 kbit/s (fc/512) NFCV VICC->VCD & VCD->VICC 1of4   */
  RFAL_BR_1p66                     = 0xED, /*!< Bit Rate 1,66 kbit/s (fc/8192) NFCV VCD->VICC 1of256             */
  RFAL_BR_KEEP                     = 0xFF  /*!< Value indicating to keep the same previous bit rate              */
} RFal_BitRate;


/*! RFal Compliance modes for upper modules  */
typedef enum {
  RFAL_COMPLIANCE_MODE_NFC,                /*!< Perform with NFC Forum 1.1 compliance                            */
  RFAL_COMPLIANCE_MODE_EMV,                /*!< Perform with EMVCo compliance                                    */
  RFAL_COMPLIANCE_MODE_ISO                 /*!< Perform with ISO10373 compliance                                 */
} RFal_ComplianceMode;


/*! RFal main states flags    */
typedef enum {
  RFAL_STATE_IDLE                  = 0,
  RFAL_STATE_INIT                  = 1,
  RFAL_STATE_MODE_SET              = 2,

  RFAL_STATE_TXRX                  = 3,
  RFAL_STATE_LM                    = 4,
  RFAL_STATE_WUM                   = 5

} RFal_State;

/*! RFal transceive states    */
typedef enum {
  RFAL_TXRX_STATE_IDLE             = 0,
  RFAL_TXRX_STATE_INIT             = 1,
  RFAL_TXRX_STATE_START            = 2,

  RFAL_TXRX_STATE_TX_IDLE          = 11,
  RFAL_TXRX_STATE_TX_WAIT_GT       = 12,
  RFAL_TXRX_STATE_TX_WAIT_FDT      = 13,
  RFAL_TXRX_STATE_TX_PREP_TX       = 14,
  RFAL_TXRX_STATE_TX_TRANSMIT      = 15,
  RFAL_TXRX_STATE_TX_WAIT_WL       = 16,
  RFAL_TXRX_STATE_TX_RELOAD_FIFO   = 17,
  RFAL_TXRX_STATE_TX_WAIT_TXE      = 18,
  RFAL_TXRX_STATE_TX_DONE          = 19,
  RFAL_TXRX_STATE_TX_FAIL          = 20,

  RFAL_TXRX_STATE_RX_IDLE          = 81,
  RFAL_TXRX_STATE_RX_WAIT_EON      = 82,
  RFAL_TXRX_STATE_RX_WAIT_RXS      = 83,
  RFAL_TXRX_STATE_RX_WAIT_RXE      = 84,
  RFAL_TXRX_STATE_RX_READ_FIFO     = 85,
  RFAL_TXRX_STATE_RX_ERR_CHECK     = 86,
  RFAL_TXRX_STATE_RX_READ_DATA     = 87,
  RFAL_TXRX_STATE_RX_WAIT_EOF      = 88,
  RFAL_TXRX_STATE_RX_DONE          = 89,
  RFAL_TXRX_STATE_RX_FAIL          = 90,

} RFal_TransceiveState;


/*! RFal transceive flags                                                                                                                    */
enum {
  RFAL_TXRX_FLAGS_CRC_TX_AUTO      = (0U << 0), /*!< CRC will be generated automatic upon transmission                                     */
  RFAL_TXRX_FLAGS_CRC_TX_MANUAL    = (1U << 0), /*!< CRC was calculated manually, included in txBuffer                                     */
  RFAL_TXRX_FLAGS_CRC_RX_KEEP      = (1U << 1), /*!< Upon Reception keep the CRC in rxBuffer (reflected on rcvd length)                    */
  RFAL_TXRX_FLAGS_CRC_RX_REMV      = (0U << 1), /*!< Remove the CRC from rxBuffer                                                          */
  RFAL_TXRX_FLAGS_NFCIP1_ON        = (1U << 2), /*!< Enable NFCIP1 mode: Add SB(F0) and LEN bytes during Tx and skip SB(F0) byte during Rx */
  RFAL_TXRX_FLAGS_NFCIP1_OFF       = (0U << 2), /*!< Disable NFCIP1 mode: do not append protocol bytes while Tx nor skip while Rx          */
  RFAL_TXRX_FLAGS_AGC_OFF          = (1U << 3),   /*!< Disable Automatic Gain Control, improving multiple devices collision detection. \b DEPRECATED: flag is deprecated, usage of Anticollision APIs based on Analog Config table with RFal_ANALOG_CONFIG_ANTICOL settings */
  RFAL_TXRX_FLAGS_AGC_ON           = (0U << 3),   /*!< Enable Automatic Gain Control, improving single device reception                \b DEPRECATED: flag is deprecated, usage of Anticollision APIs based on Analog Config table with RFal_ANALOG_CONFIG_ANTICOL settings */
  RFAL_TXRX_FLAGS_PAR_RX_KEEP      = (1U << 4), /*!< Disable Parity check and keep the Parity and CRC bits in the received buffer          */
  RFAL_TXRX_FLAGS_PAR_RX_REMV      = (0U << 4), /*!< Enable Parity check and remove the parity bits from the received buffer               */
  RFAL_TXRX_FLAGS_PAR_TX_NONE      = (1U << 5), /*!< Disable automatic Parity generation (ISO14443A) and use the one provided in the buffer*/
  RFAL_TXRX_FLAGS_PAR_TX_AUTO      = (0U << 5), /*!< Enable automatic Parity generation (ISO14443A)                                        */
  RFAL_TXRX_FLAGS_NFCV_FLAG_MANUAL = (1U << 6), /*!< Disable automatic adaption of flag byte (ISO15693) according to current comm params   */
  RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO   = (0U << 6), /*!< Enable automatic adaption of flag byte (ISO115693) according to current comm params   */
  RFAL_TXRX_FLAGS_CRC_RX_MANUAL    = (1U << 7), /*!< Disable automatic CRC check                                                           */
  RFAL_TXRX_FLAGS_CRC_RX_AUTO      = (0U << 7), /*!< Enable automatic CRC check                                                            */
};


/*! RFal error handling                                                                                                                      */
typedef enum {
  ERRORHANDLING_NONE          = 0,         /*!< No special error handling will be performed                                           */
  ERRORHANDLING_EMD           = 1          /*!< EMD suppression enabled  Digital 2.1  4.1.1.1 ; EMVCo 3.0  4.9.2 ; ISO 14443-3  8.3   */
} RFal_EHandling;


/*! Struct that holds all context to be used on a Transceive                                                */
typedef struct {
  uint8_t              *txBuf;                  /*!< (In)  Buffer where outgoing message is located       */
  uint16_t              txBufLen;               /*!< (In)  Length of the outgoing message in bits         */

  uint8_t              *rxBuf;                  /*!< (Out) Buffer where incoming message will be placed   */
  uint16_t              rxBufLen;               /*!< (In)  Maximum length of the incoming message in bits */
  uint16_t             *rxRcvdLen;              /*!< (Out) Actual received length in bits                 */

  uint32_t              flags;                  /*!< (In)  TransceiveFlags indication special handling    */
  uint32_t              fwt;                    /*!< (In)  Frame Waiting Time in 1/fc                     */
} RFal_TransceiveContext;


/*! System callback to indicate an event that requires a system reRun        */
typedef void (* RFal_UpperLayerCallback)(void);

/*! Callback to be executed before a Transceive                              */
typedef void (* RFal_PreTxRxCallback)(void);

/*! Callback to be executed after a Transceive                               */
typedef void (* RFal_PostTxRxCallback)(void);

/*! Callback to sync actual transmission start                               */
typedef bool (* RFal_SyncTxRxCallback)(void);

/*! Callback upon External Field detected while in Listen Mode              */
typedef void (* RFal_LmEonCallback)(void);

/*******************************************************************************/
/*  ISO14443A                                                                  */
/*******************************************************************************/

/*! RFal ISO 14443A Short Frame Command */
typedef enum {
  RFAL_14443A_SHORTFRAME_CMD_WUPA = 0x52,  /*!< ISO14443A WUPA / NFC-A ALL_REQ  */
  RFAL_14443A_SHORTFRAME_CMD_REQA = 0x26   /*!< ISO14443A REQA / NFC-A SENS_REQ */
} RFal_14443AShortFrameCmd;

/*******************************************************************************/


/*******************************************************************************/
/*  FeliCa                                                                     */
/*******************************************************************************/

#define RFAL_FELICA_LEN_LEN                        1U                                           /*!< FeliCa LEN byte length                                              */
#define RFAL_FELICA_POLL_REQ_LEN                   (RFAL_FELICA_LEN_LEN + 1U + 2U + 1U + 1U)    /*!< FeliCa Poll Request length (LEN + CMD + SC + RC + TSN)              */
#define RFAL_FELICA_POLL_RES_LEN                   (RFAL_FELICA_LEN_LEN + 1U + 8U + 8U + 2U)    /*!< Maximum FeliCa Poll Response length (LEN + CMD + NFCID2 + PAD + RD) */
#define RFAL_FELICA_POLL_MAX_SLOTS                 16U                                          /*!< Maximum number of slots (TSN) on FeliCa Poll                        */


/*! NFC-F RC (Request Code) codes  NFC Forum Digital 1.1 Table 42                                                                                                        */
enum {
  RFAL_FELICA_POLL_RC_NO_REQUEST        =     0x00U,                                           /*!< RC: No System Code information requested                            */
  RFAL_FELICA_POLL_RC_SYSTEM_CODE       =     0x01U,                                           /*!< RC: System Code information requested                               */
  RFAL_FELICA_POLL_RC_COM_PERFORMANCE   =     0x02U                                            /*!< RC: Advanced protocol features supported                            */
};


/*! NFC-F TSN (Time Slot Number) codes  NFC Forum Digital 1.1 Table 43   */
typedef enum {
  RFAL_FELICA_1_SLOT    =  0,   /*!< TSN with number of Time Slots: 1  */
  RFAL_FELICA_2_SLOTS   =  1,   /*!< TSN with number of Time Slots: 2  */
  RFAL_FELICA_4_SLOTS   =  3,   /*!< TSN with number of Time Slots: 4  */
  RFAL_FELICA_8_SLOTS   =  7,   /*!< TSN with number of Time Slots: 8  */
  RFAL_FELICA_16_SLOTS  =  15   /*!< TSN with number of Time Slots: 16 */
} RFal_FeliCaPollSlots;


/*! NFCF Poll Response  NFC Forum Digital 1.1 Table 44 */
typedef uint8_t RFal_FeliCaPollRes[RFAL_FELICA_POLL_RES_LEN];


/*******************************************************************************/


/*******************************************************************************/
/*  Listen Mode                                                                */
/*******************************************************************************/

/*! RFal Listen Mode NFCID Length */
typedef enum {
  RFAL_LM_NFCID_LEN_04  = RFAL_NFCID1_SINGLE_LEN, /*!< Listen mode indicates  4 byte NFCID */
  RFAL_LM_NFCID_LEN_07  = RFAL_NFCID1_DOUBLE_LEN, /*!< Listen mode indicates  7 byte NFCID */
  RFAL_LM_NFCID_LEN_10  = RFAL_NFCID1_TRIPLE_LEN, /*!< Listen mode indicates 10 byte NFCID */
} RFal_LmNfcidLen;


/*! RFAL Listen Mode States */
typedef enum {
  RFAL_LM_STATE_NOT_INIT              = 0x00,     /*!< Not Initialized state                       */
  RFAL_LM_STATE_POWER_OFF             = 0x01,     /*!< Power Off state                             */
  RFAL_LM_STATE_IDLE                  = 0x02,     /*!< Idle state  Activity 1.1  5.2               */
  RFAL_LM_STATE_READY_A               = 0x03,     /*!< Ready A state  Activity 1.1  5.3 5.4 & 5.5  */
  RFAL_LM_STATE_READY_B               = 0x04,     /*!< Ready B state  Activity 1.1  5.11 5.12      */
  RFAL_LM_STATE_READY_F               = 0x05,     /*!< Ready F state  Activity 1.1  5.15           */
  RFAL_LM_STATE_ACTIVE_A              = 0x06,     /*!< Active A state  Activity 1.1  5.6           */
  RFAL_LM_STATE_CARDEMU_4A            = 0x07,     /*!< Card Emulation 4A state  Activity 1.1  5.10 */
  RFAL_LM_STATE_CARDEMU_4B            = 0x08,     /*!< Card Emulation 4B state  Activity 1.1  5.14 */
  RFAL_LM_STATE_CARDEMU_3             = 0x09,     /*!< Card Emulation 3 state  Activity 1.1  5.18  */
  RFAL_LM_STATE_TARGET_A              = 0x0A,     /*!< Target A state  Activity 1.1  5.9           */
  RFAL_LM_STATE_TARGET_F              = 0x0B,     /*!< Target F state  Activity 1.1  5.17          */
  RFAL_LM_STATE_SLEEP_A               = 0x0C,     /*!< Sleep A state  Activity 1.1  5.7            */
  RFAL_LM_STATE_SLEEP_B               = 0x0D,     /*!< Sleep B state  Activity 1.1  5.13           */
  RFAL_LM_STATE_READY_Ax              = 0x0E,     /*!< Ready A* state  Activity 1.1  5.3 5.4 & 5.5 */
  RFAL_LM_STATE_ACTIVE_Ax             = 0x0F,     /*!< Active A* state  Activity 1.1  5.6          */
  RFAL_LM_STATE_SLEEP_AF              = 0x10,     /*!< Sleep AF state  Activity 1.1  5.19          */
} RFal_LmState;


/*! RFal Listen Mode Passive A configs */
typedef struct {
  RFal_LmNfcidLen   nfcidLen;                        /*!< NFCID Len (4, 7 or 10 bytes)              */
  uint8_t          nfcid[RFAL_NFCID1_TRIPLE_LEN];   /*!< NFCID                                     */
  uint8_t          SENS_RES[RFAL_LM_SENS_RES_LEN];  /*!< NFC-106k; SENS_REQ Response               */
  uint8_t          SEL_RES;                         /*!< SEL_RES (SAK) with complete NFCID1 (UID)  */
} RFal_LmConfPA;


/*! RFal Listen Mode Passive B configs */
typedef struct {
  uint8_t          SENSB_RES[RFAL_LM_SENSB_RES_LEN];  /*!< SENSF_RES                               */
} RFal_LmConfPB;


/*! RFal Listen Mode Passive F configs */
typedef struct {
  uint8_t          SC[RFAL_LM_SENSF_SC_LEN];          /*!< System Code to listen for               */
  uint8_t          SENSF_RES[RFAL_LM_SENSF_RES_LEN];  /*!< SENSF_RES                               */
} RFal_LmConfPF;

/*! RFal low power modes    */
typedef enum {
  RFal_LP_MODE_PD  = 0,    /*!< Set RF Chip in Power Down state                                      */
  RFal_LP_MODE_HR  = 1     /*!< Set RF Chip in Hold Reset state (available for specific devices)     */
} RFal_LpMode;

/*******************************************************************************/


/*******************************************************************************/
/*  Wake-Up Mode                                                               */
/*******************************************************************************/

#define RFAL_WUM_REFERENCE_AUTO           0xFFU      /*!< Indicates new reference is set by the driver */

/*! RFal Wake-Up Mode States */
typedef enum {
  RFAL_WUM_STATE_NOT_INIT              = 0x00,     /*!< Not Initialized state                       */
  RFAL_WUM_STATE_INITIALIZING          = 0x01,     /*!< Wake-Up mode is starting                    */
  RFAL_WUM_STATE_ENABLED               = 0x02,     /*!< Wake-Up mode is enabled                     */
  RFAL_WUM_STATE_ENABLED_WOKE          = 0x03,     /*!< Wake-Up mode enabled and has received IRQ(s)*/
} RFal_WumState;

/*! RFal Wake-Up Period/Timer */
typedef enum {
  RFAL_WUM_PERIOD_10MS      = 0x00,     /*!< Wake-Up timer ~9.7ms                                       */
  RFAL_WUM_PERIOD_15MS      = 0x01,     /*!< Wake-Up timer ~13.3ms                                      */
  RFAL_WUM_PERIOD_20MS      = 0x02,     /*!< Wake-Up timer ~19.3ms                                      */
  RFAL_WUM_PERIOD_25MS      = 0x03,     /*!< Wake-Up timer ~26.6ms                                      */
  RFAL_WUM_PERIOD_30MS      = 0x02,     /*!< Wake-Up timer 30ms                          */
  RFAL_WUM_PERIOD_40MS      = 0x04,     /*!< Wake-Up timer ~38.7ms                                      */
  RFAL_WUM_PERIOD_50MS      = 0x04,     /*!< Wake-Up timer 50ms                          */
  RFAL_WUM_PERIOD_55MS      = 0x05,     /*!< Wake-Up timer ~53.2ms                                      */
  RFAL_WUM_PERIOD_60MS      = 0x05,     /*!< Wake-Up timer 60ms                          */
  RFAL_WUM_PERIOD_70MS      = 0x06,     /*!< Wake-Up timer 70ms                          */
  RFAL_WUM_PERIOD_80MS      = 0x06,     /*!< Wake-Up timer ~77.3ms                                      */
  RFAL_WUM_PERIOD_100MS     = 0x10,     /*!< Wake-Up timer 100ms                         */
  RFAL_WUM_PERIOD_105MS     = 0x07,     /*!< Wake-Up timer ~106.3ms                                     */
  RFAL_WUM_PERIOD_155MS     = 0x08,     /*!< Wake-Up timer ~154.7ms                                     */
  RFAL_WUM_PERIOD_200MS     = 0x11,     /*!< Wake-Up timer 200ms                         */
  RFAL_WUM_PERIOD_215MS     = 0x09,     /*!< Wake-Up timer ~212.7ms                                     */
  RFAL_WUM_PERIOD_300MS     = 0x12,     /*!< Wake-Up timer 300ms                         */
  RFAL_WUM_PERIOD_310MS     = 0x0A,     /*!< Wake-Up timer ~309.3ms                                     */
  RFAL_WUM_PERIOD_400MS     = 0x13,     /*!< Wake-Up timer 400ms                         */
  RFAL_WUM_PERIOD_425MS     = 0x0B,     /*!< Wake-Up timer ~425.3ms                                     */
  RFAL_WUM_PERIOD_500MS     = 0x14,     /*!< Wake-Up timer 500ms                                        */
  RFAL_WUM_PERIOD_600MS     = 0x15,     /*!< Wake-Up timer 600ms                         */
  RFAL_WUM_PERIOD_620MS     = 0x0C,     /*!< Wake-Up timer ~618.6ms                                     */
  RFAL_WUM_PERIOD_700MS     = 0x16,     /*!< Wake-Up timer 700ms                         */
  RFAL_WUM_PERIOD_800MS     = 0x17,     /*!< Wake-Up timer 800ms                         */
  RFAL_WUM_PERIOD_850MS     = 0x0D,     /*!< Wake-Up timer ~850.6ms                                     */
  RFAL_WUM_PERIOD_1240MS    = 0x0E,     /*!< Wake-Up timer ~1237.3ms                                    */
  RFAL_WUM_PERIOD_1700MS    = 0x0F,     /*!< Wake-Up timer ~1701.2ms                                    */
} RFal_WumPeriod;


/*! RFal Wake-Up Period/Timer */
typedef enum {
  RFAL_WUM_AA_WEIGHT_4       = 0x00,    /*!< Wake-Up Auto Average Weight 4                              */
  RFAL_WUM_AA_WEIGHT_8       = 0x01,    /*!< Wake-Up Auto Average Weight 8                              */
  RFAL_WUM_AA_WEIGHT_16      = 0x02,    /*!< Wake-Up Auto Average Weight 16                             */
  RFAL_WUM_AA_WEIGHT_32      = 0x03,    /*!< Wake-Up Auto Average Weight 32                             */
} RFal_WumAAWeight;


/*! RFal Wake-Up measurement duration */
typedef enum {
  RFAL_WUM_MEAS_DUR_26_10    = 0,       /*!< WU measurement duration: 26.0us (slow) / 10.6us (fast)     */
  RFAL_WUM_MEAS_DUR_30_14    = 1,       /*!< WU measurement duration: 29.5us (slow) / 14.2us (fast)     */
  RFAL_WUM_MEAS_DUR_34_19    = 2,       /*!< WU measurement duration: 34.2us (slow) / 18.9us (fast)     */
  RFAL_WUM_MEAS_DUR_44_28    = 3,       /*!< WU measurement duration: 43.7us (slow) / 28.3us (fast)     */
} RFal_WumMeasDuration;


/*! RFal Wake-Up measurement filter */
typedef enum {
  RFAL_WUM_MEAS_FIL_SLOW    = false,    /*!< Wake-Up measurement slow filter                            */
  RFAL_WUM_MEAS_FIL_FAST    = true,     /*!< Wake-up measurement fast filter                            */
} RFal_WumMeasFilter;

/*! RFal Wake-Up trigger thresholds for threshold bitmask config */
enum {
  RFAL_WUM_TRE_ABOVE    = (1U << 2),    /*!< Wake-up trigger threshold: above upper limit               */
  RFAL_WUM_TRE_BETWEEN  = (1U << 1),    /*!< Wake-up trigger threshold: between upper and lower limit   */
  RFAL_WUM_TRE_BELOW    = (1U << 0),    /*!< Wake-up trigger threshold: below lower limit               */
};

/*! RFal Wake-Up channel configuration */
typedef struct {
  bool                 enabled;         /*!< Inductive Amplitude measurement enabled                    */
  uint8_t              delta;           /*!< Delta between the reference and measurement to wake-up     */
  uint8_t              fracDelta;       /*!< Fractional part of the delta [0;3] 0.25 steps (SW TD only) */
  uint8_t              reference;       /*!< Reference to be used;RFAL_WUM_REFERENCE_AUTO sets it auto  */
  uint8_t              threshold;       /*!< Wake-Up trigger threshold bitmask                           */
  bool                 autoAvg;         /*!< Use the HW Auto Averaging feature                         */
  bool                 aaInclMeas;      /*!< When AutoAvg is enabled, include IRQ measurement           */
  RFal_WumAAWeight      aaWeight;        /*!< When AutoAvg is enabled, last measure weight               */
} RFal_WumMeasChannel;

/*! RFal Wake-Up Mode configuration */
typedef struct RFal_WakeUpConfig {
  RFal_WumPeriod        period;          /*!< Wake-Up Timer period;how often measurement(s) is performed */
  bool                 irqTout;         /*!< IRQ at every timeout will refresh the measurement(s)       */
  bool                 swTagDetect;/*!< Use SW Tag Detection instead of HW Wake-Up mode            */
  bool                 autoAvg;         /*!< Use the HW Auto Averaging feature on the enabled channel(s)*/
  bool                 skipCal;         /*!< Do not perform calibration starting WU mode                */
  bool                 skipReCal;       /*!< Do not perform recalibration during WU mode                */
  bool                 delCal;          /*!< Delay calibration step starting WU mode                    */
  bool                 delRef;          /*!< Delay reference step starting WU mode                      */
  RFal_WumMeasDuration  measDur;         /*!< Wake-up measurement duration config                        */
  RFal_WumMeasFilter    measFil;         /*!< Wake-up measurement filter config                          */

  RFal_WumMeasChannel   indAmp;                        /*!< Inductive Amplitude Configuration                         */
  RFal_WumMeasChannel   indPha;                        /*!< Inductive Phase Configuration                             */
  RFal_WumMeasChannel   cap;
} RFal_WakeUpConfig;


/*! RFal Wake-Up Mode information */
typedef struct {
  bool                 irqWut;          /*!< Wake-Up Timer IRQ received (cleared upon read)             */
  uint8_t              status;          /*!< Wake-Up status                                             */

  struct {
    uint8_t          lastMeas;        /*!< Value of the latest measurement                            */
    uint8_t         reference;       /*!< Current reference value (TD format if SW TD enabled)       */
    uint8_t          calib;           /*!< Current calibration value                                  */
    bool             irqWu;           /*!< Amplitude WU IRQ received (cleared upon read)              */
  } indAmp;                             /*!< Inductive Amplitude                                        */
  struct {
    uint8_t          lastMeas;        /*!< Value of the latest measurement                            */
    uint8_t         reference;       /*!< Current reference value (TD format if SW TD enabled)       */
    bool             irqWu;           /*!< Phase WU IRQ received (cleared upon read)                  */
  } indPha;                             /*!< Inductive Phase                                            */
  struct {
    uint8_t          lastMeas;        /*!< Value of the latest measurement                            */
    uint8_t         reference;       /*!< Current reference value                                    */
    uint8_t          calib;           /*!< Current calibration value                                  */
    bool             irqWu;           /*!< Capacitive WU IRQ received (cleared upon read)             */
  } cap;                                /*!< Capacitive                                                 */
} RFal_WakeUpInfo;

/*******************************************************************************/


#endif /* RFAL_RF_H */

