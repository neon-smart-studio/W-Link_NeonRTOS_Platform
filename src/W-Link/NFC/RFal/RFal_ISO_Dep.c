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
#include "RFal_ISO_Dep.h"
#include "RFal_RF.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#define ISODEP_CRC_LEN RFAL_CRC_LEN /*!< ISO1443 CRC Length */

#define ISODEP_PCB_POS (0U)      /*!< PCB position on message header*/
#define ISODEP_SWTX_INF_POS (1U) /*!< INF position in a S-WTX       */

#define ISODEP_DID_POS (1U)        /*!< DID position on message header*/
#define ISODEP_SWTX_PARAM_LEN           (1U)         /*!< SWTX parameter length         */

#define ISODEP_DSL_MAX_LEN (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN) /*!< Deselect Req/Res length */

#define ISODEP_PCB_xBLOCK_MASK (0xC0U) /*!< Bit mask for Block type       */
#define ISODEP_PCB_IBLOCK (0x00U)      /*!< Bit mask indicating a I-Block */
#define ISODEP_PCB_RBLOCK (0x80U)      /*!< Bit mask indicating a R-Block */
#define ISODEP_PCB_SBLOCK (0xC0U)      /*!< Bit mask indicating a S-Block */
#define ISODEP_PCB_INVALID (0x40U)     /*!< Bit mask of an Invalid PCB    */

#define ISODEP_HDR_MAX_LEN (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN + RFAL_ISODEP_NAD_LEN) /*!< Max header length (PCB + DID + NAD)      */

#define ISODEP_PCB_IB_VALID_MASK (ISODEP_PCB_B6_BIT | ISODEP_PCB_B2_BIT)                     /*!< Bit mask for the MUST bits on I-Block    */
#define ISODEP_PCB_IB_VALID_VAL (ISODEP_PCB_B2_BIT)                                          /*!< Value for the MUST bits on I-Block       */
#define ISODEP_PCB_RB_VALID_MASK (ISODEP_PCB_B6_BIT | ISODEP_PCB_B3_BIT | ISODEP_PCB_B2_BIT) /*!< Bit mask for the MUST bits on R-Block    */
#define ISODEP_PCB_RB_VALID_VAL (ISODEP_PCB_B6_BIT | ISODEP_PCB_B2_BIT)                      /*!< Value for the MUST bits on R-Block       */
#define ISODEP_PCB_SB_VALID_MASK (ISODEP_PCB_B3_BIT | ISODEP_PCB_B2_BIT | ISODEP_PCB_B1_BIT) /*!< Bit mask for the MUST bits on I-Block    */
#define ISODEP_PCB_SB_VALID_VAL (ISODEP_PCB_B2_BIT)                                          /*!< Value for the MUST bits on I-Block       */

#define ISODEP_PCB_B1_BIT (0x01U)       /*!< Bit mask for the RFU S Blocks                                        */
#define ISODEP_PCB_B2_BIT (0x02U)       /*!< Bit mask for the RFU bit2 in I,S,R Blocks                            */
#define ISODEP_PCB_B3_BIT (0x04U)       /*!< Bit mask for the RFU bit3 in R Blocks                                */
#define ISODEP_PCB_B6_BIT (0x20U)       /*!< Bit mask for the RFU bit2 in R Blocks                                */
#define ISODEP_PCB_CHAINING_BIT (0x10U) /*!< Bit mask for the chaining bit of an ISO DEP I-Block in PCB.          */
#define ISODEP_PCB_DID_BIT (0x08U)      /*!< Bit mask for the DID presence bit of an ISO DEP I,S,R Blocks PCB.    */
#define ISODEP_PCB_NAD_BIT (0x04U)      /*!< Bit mask for the NAD presence bit of an ISO DEP I,S,R Blocks in PCB  */
#define ISODEP_PCB_BN_MASK (0x01U)      /*!< Bit mask for the block number of an ISO DEP I,R Block in PCB         */

#define ISODEP_SWTX_PL_MASK (0xC0U)   /*!< Bit mask for the Power Level bits of the inf byte of an WTX request or response */
#define ISODEP_SWTX_WTXM_MASK (0x3FU) /*!< Bit mask for the WTXM bits of the inf byte of an WTX request or response        */

#define ISODEP_RBLOCK_INF_LEN (0U) /*!< INF length of R-Block               Digital 1.1 15.1.3 */
#define ISODEP_SDSL_INF_LEN (0U)   /*!< INF length of S(DSL)                Digital 1.1 15.1.3 */
#define ISODEP_SWTX_INF_LEN (1U)   /*!< INF length of S(WTX)                Digital 1.1 15.2.2 */

#define ISODEP_WTXM_MIN (1U)  /*!< Minimum allowed value for the WTXM, Digital 1.0 13.2.2 */
#define ISODEP_WTXM_MAX (59U) /*!< Maximum allowed value for the WTXM, Digital 1.0 13.2.2 */

#define ISODEP_PCB_Sxx_MASK (0x30U) /*!< Bit mask for the S-Block type                          */
#define ISODEP_PCB_DESELECT (0x00U) /*!< Bit mask for S-Block indicating Deselect               */
#define ISODEP_PCB_WTX (0x30U)      /*!< Bit mask for S-Block indicating Waiting Time eXtension */

#define ISODEP_PCB_Rx_MASK (0x10U) /*!< Bit mask for the R-Block type       */
#define ISODEP_PCB_ACK (0x00U)     /*!< Bit mask for R-Block indicating ACK */
#define ISODEP_PCB_NAK (0x10U)     /*!< Bit mask for R-Block indicating NAK */

/*! Maximum length of control message (no INF) */
#define ISODEP_CONTROLMSG_BUF_LEN       (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN + RFAL_ISODEP_NAD_LEN + ISODEP_SWTX_PARAM_LEN)

#define ISODEP_FWT_DEACTIVATION         (71680U)     /*!< FWT used for DESELECT  Digital 2.2 B10  ISO1444-4 7.2 & 8.1 */
#define ISODEP_MAX_RERUNS (0x0FFFFFFFU)  /*!< Maximum rerun retrys for a blocking protocol run*/

#define ISODEP_PCBSBLOCK (0x00U | ISODEP_PCB_SBLOCK | ISODEP_PCB_B2_BIT) /*!< PCB Value of a S-Block                 */
#define ISODEP_PCB_SDSL (ISODEP_PCBSBLOCK | ISODEP_PCB_DESELECT)         /*!< PCB Value of a S-Block with DESELECT   */
#define ISODEP_PCB_SWTX (ISODEP_PCBSBLOCK | ISODEP_PCB_WTX)              /*!< PCB Value of a S-Block with WTX        */
#define ISODEP_PCB_SPARAMETERS (ISODEP_PCB_SBLOCK | ISODEP_PCB_WTX)      /*!< PCB Value of a S-Block with PARAMETERS */

#define ISODEP_FWI_LIS_MAX_NFC 8U                                                                                                        /*!< FWT Listener Max FWIT4ATmax FWIBmax  Digital 1.1  A6 & A3  */
#define ISODEP_FWI_LIS_MAX_EMVCO 7U                                                                                                      /*!< FWT Listener Max FWIMAX       EMVCo 2.6 A.5                */
#define ISODEP_FWI_LIS_MAX (uint8_t)((gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) ? ISODEP_FWI_LIS_MAX_EMVCO : ISODEP_FWI_LIS_MAX_NFC) /*!< FWI Listener Max as NFC / EMVCo */
#define ISODEP_FWT_LIS_MAX RFal_ISO_Dep_FWI2FWT(ISODEP_FWI_LIS_MAX)                                                                         /*!< FWT Listener Max                       */

#define ISODEP_FWI_MIN_10 (1U) /*!< Minimum value for FWI Digital 1.0 11.6.2.17 */
#define ISODEP_FWI_MIN_11 (0U) /*!< Default value for FWI Digital 1.1 13.6.2    */
#define ISODEP_FWI_MAX (14U)   /*!< Maximum value for FWI Digital 1.0 11.6.2.17 */
#define ISODEP_SFGI_MIN (0U)   /*!< Default value for FWI Digital 1.1 13.6.2.22 */
#define ISODEP_SFGI_MAX (14U)  /*!< Maximum value for FWI Digital 1.1 13.6.2.22 */

#define RFAL_ISODEP_SPARAM_TVL_HDR_LEN (2U)                                               /*!< S(PARAMETERS) TVL header length: Tag + Len */
#define RFAL_ISODEP_SPARAM_HDR_LEN (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_SPARAM_TVL_HDR_LEN) /*!< S(PARAMETERS) header length: PCB + Tag + Len */

/**********************************************************************************************************************/
/**********************************************************************************************************************/
#define RFAL_ISODEP_NO_PARAM (0U) /*!< No parameter flag for isoDepHandleControlMsg()     */

#define RFAL_ISODEP_CMD_RATS (0xE0U) /*!< RATS command   Digital 1.1  13.6.1                 */

#define RFAL_ISODEP_ATS_MIN_LEN (1U)                                                   /*!< Minimum ATS length   Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_HDR_LEN (5U)                                                   /*!< ATS headerlength     Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_MAX_LEN (RFAL_ISODEP_ATS_HDR_LEN + RFAL_ISODEP_ATS_HB_MAX_LEN) /*!< Maximum ATS length   Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_T0_FSCI_MASK (0x0FU)                                           /*!< ATS T0's FSCI mask   Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_TB_FWI_SHIFT (4U)                                              /*!< ATS TB's FWI shift   Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_FWI_MASK (0x0FU)                                               /*!< ATS TB's FWI shift   Digital 1.1  13.6.2 */
#define RFAL_ISODEP_ATS_TL_POS (0x00U)                                                 /*!< ATS TL's position    Digital 1.1  13.6.2 */

#define RFAL_ISODEP_PPS_SB (0xD0U)                /*!< PPS REQ PPSS's SB value (no CID)   ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_MASK (0xF0U)              /*!< PPS REQ PPSS's SB mask             ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_SB_DID_MASK (0x0FU)       /*!< PPS REQ PPSS's DID|CID mask        ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_PPS0_PPS1_PRESENT (0x11U) /*!< PPS REQ PPS0 indicating that PPS1 is present       */
#define RFAL_ISODEP_PPS_PPS1 (0x00U)              /*!< PPS REQ PPS1 fixed value           ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_PPS1_DSI_SHIFT (2U)       /*!< PPS REQ PPS1 fixed value           ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_PPS1_DXI_MASK (0x0FU)     /*!< PPS REQ PPS1 fixed value           ISO14443-4  5.3 */
#define RFAL_ISODEP_PPS_RES_LEN (1U)              /*!< PPS Response length                ISO14443-4  5.4 */
#define RFAL_ISODEP_PPS_STARTBYTE_POS (0U)        /*!< PPS REQ PPSS's byte position       ISO14443-4  5.4 */
#define RFAL_ISODEP_PPS_PPS0_POS (1U)             /*!< PPS REQ PPS0's byte position       ISO14443-4  5.4 */
#define RFAL_ISODEP_PPS_PPS1_POS (2U)             /*!< PPS REQ PPS1's byte position       ISO14443-4  5.4 */
#define RFAL_ISODEP_PPS0_VALID_MASK (0xEFU)       /*!< PPS REQ PPS0 valid coding mask     ISO14443-4  5.4 */

#define RFAL_ISODEP_CMD_ATTRIB (0x1DU)              /*!< ATTRIB command                 Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM2_DSI_SHIFT (6U)    /*!< ATTRIB PARAM2 DSI shift        Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM2_DRI_SHIFT (4U)    /*!< ATTRIB PARAM2 DRI shift        Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM2_DXI_MASK (0xF0U)  /*!< ATTRIB PARAM2 DxI mask         Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM2_FSDI_MASK (0x0FU) /*!< ATTRIB PARAM2 FSDI mask        Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_PARAM4_DID_MASK (0x0FU)  /*!< ATTRIB PARAM4 DID mask         Digital 1.1  14.6.1 */
#define RFAL_ISODEP_ATTRIB_HDR_LEN (9U)             /*!< ATTRIB REQ header length       Digital 1.1  14.6.1 */

#define RFAL_ISODEP_ATTRIB_RES_HDR_LEN (1U)      /*!< ATTRIB RES header length       Digital 1.1  14.6.2 */
#define RFAL_ISODEP_ATTRIB_RES_MBLIDID_POS (0U)  /*!< ATTRIB RES MBLI|DID position   Digital 1.1  14.6.2 */
#define RFAL_ISODEP_ATTRIB_RES_DID_MASK (0x0FU)  /*!< ATTRIB RES DID mask            Digital 1.1  14.6.2 */
#define RFAL_ISODEP_ATTRIB_RES_MBLI_MASK (0x0FU) /*!< ATTRIB RES MBLI mask           Digital 1.1  14.6.2 */
#define RFAL_ISODEP_ATTRIB_RES_MBLI_SHIFT (4U)   /*!< ATTRIB RES MBLI shift          Digital 1.1  14.6.2 */

#define RFAL_ISODEP_DID_MASK (0x0FU) /*!< ISODEP's DID mask                                  */
#define RFAL_ISODEP_DID_00 (0U)      /*!< ISODEP's DID value 0                               */

#define RFAL_ISODEP_FSDI_MAX_NFC (8U)       /*!< Max FSDI value   Digital 2.0  14.6.1.9 & B7 & B8   */
#define RFAL_ISODEP_FSDI_MAX_NFC_21 (0x0CU) /*!< Max FSDI value   Digital 2.1  14.6.1.9 & Table 72  */
#define RFAL_ISODEP_FSDI_MAX_EMV (0x0CU)    /*!< Max FSDI value   EMVCo 3.0  5.7.2.5                */

#define RFAL_ISODEP_RATS_PARAM_FSDI_MASK (0xF0U) /*!< Mask bits for FSDI in RATS                         */
#define RFAL_ISODEP_RATS_PARAM_FSDI_SHIFT (4U)   /*!< Shift for FSDI in RATS                             */
#define RFAL_ISODEP_RATS_PARAM_DID_MASK (0x0FU)  /*!< Mask bits for DID in RATS                          */

#define RFAL_ISODEP_ATS_TL_OFFSET (0x00U)   /*!< Offset of TL on ATS                                */
#define RFAL_ISODEP_ATS_TA_OFFSET (0x02U)   /*!< Offset of TA if it is present on ATS               */
#define RFAL_ISODEP_ATS_TB_OFFSET (0x03U)   /*!< Offset of TB if both TA and TB is present on ATS   */
#define RFAL_ISODEP_ATS_TC_OFFSET (0x04U)   /*!< Offset of TC if both TA,TB & TC are present on ATS */
#define RFAL_ISODEP_ATS_HIST_OFFSET (0x05U) /*!< Offset of Historical Bytes if TA, TB & TC are present on ATS          */
#define RFAL_ISODEP_ATS_TC_ADV_FEAT (0x10U) /*!< Bit mask indicating support for Advanced protocol features: DID & NAD */
#define RFAL_ISODEP_ATS_TC_DID (0x02U)      /*!< Bit mask indicating support for DID                 */
#define RFAL_ISODEP_ATS_TC_NAD (0x01U)      /*!< Bit mask indicating support for NAD                 */

#define RFAL_ISODEP_PPS0_PPS1_PRESENT (0x11U)     /*!< PPS0 byte indicating that PPS1 is present            */
#define RFAL_ISODEP_PPS0_PPS1_NOT_PRESENT (0x01U) /*!< PPS0 byte indicating that PPS1 is NOT present        */
#define RFAL_ISODEP_PPS1_DRI_MASK (0x03U)         /*!< PPS1 byte DRI mask bits                              */
#define RFAL_ISODEP_PPS1_DSI_MASK (0x0CU)         /*!< PPS1 byte DSI mask bits                              */
#define RFAL_ISODEP_PPS1_DSI_SHIFT (2U)           /*!< PPS1 byte DSI shift                                  */
#define RFAL_ISODEP_PPS1_DxI_MASK (0x03U)         /*!< PPS1 byte DSI/DRS mask bits                          */

/*! Delta Time for polling during Activation (ATS) : 20ms    Digital 1.0 11.7.1.1 & A.7    */
#define RFAL_ISODEP_T4T_DTIME_POLL_10 rfalConvMsTo1fc(20)

/*! Delta Time for polling during Activation (ATS) : 16.4ms  Digital 1.1 13.8.1.1 & A.6
 *  Use 16 ms as testcase T4AT_BI_10_03 sends a frame exactly at the border */
#define RFAL_ISODEP_T4T_DTIME_POLL_11 216960U

/*! Activation frame waiting time FWT(act) = 71680/fc (~5286us) Digital 1.1 13.8.1.1 & A.6 */
#define RFAL_ISODEP_T4T_FWT_ACTIVATION (71680U + RFAL_ISODEP_T4T_DTIME_POLL_11)

/*! Delta frame waiting time = 16/fc  Digital 1.0  11.7.1.3 & A.7*/
#define RFAL_ISODEP_DFWT_10 16U

/*! Delta frame waiting time = 16/fc  Digital 2.0  14.8.1.3 & B.7*/
#define RFAL_ISODEP_DFWT_20 49152U

/*
 ******************************************************************************
 * MACROS
 ******************************************************************************
 */

#define isoDep_PCBisIBlock(pcb) (((pcb) & (ISODEP_PCB_xBLOCK_MASK | ISODEP_PCB_IB_VALID_MASK)) == (ISODEP_PCB_IBLOCK | ISODEP_PCB_IB_VALID_VAL)) /*!< Checks if pcb is a I-Block */
#define isoDep_PCBisRBlock(pcb) (((pcb) & (ISODEP_PCB_xBLOCK_MASK | ISODEP_PCB_RB_VALID_MASK)) == (ISODEP_PCB_RBLOCK | ISODEP_PCB_RB_VALID_VAL)) /*!< Checks if pcb is a R-Block */
#define isoDep_PCBisSBlock(pcb) (((pcb) & (ISODEP_PCB_xBLOCK_MASK | ISODEP_PCB_SB_VALID_MASK)) == (ISODEP_PCB_SBLOCK | ISODEP_PCB_SB_VALID_VAL)) /*!< Checks if pcb is a S-Block */

#define isoDep_PCBisChaining(pcb) (((pcb) & ISODEP_PCB_CHAINING_BIT) == ISODEP_PCB_CHAINING_BIT) /*!< Checks if pcb is indicating chaining */

#define isoDep_PCBisDeselect(pcb) (((pcb) & ISODEP_PCB_Sxx_MASK) == ISODEP_PCB_DESELECT) /*!< Checks if pcb is indicating DESELECT */
#define isoDep_PCBisWTX(pcb) (((pcb) & ISODEP_PCB_Sxx_MASK) == ISODEP_PCB_WTX)           /*!< Checks if pcb is indicating WTX      */

#define isoDep_PCBisACK(pcb) (((pcb) & ISODEP_PCB_Rx_MASK) == ISODEP_PCB_ACK) /*!< Checks if pcb is indicating ACK      */
#define isoDep_PCBisNAK(pcb) (((pcb) & ISODEP_PCB_Rx_MASK) == ISODEP_PCB_NAK) /*!< Checks if pcb is indicating ACK      */

#define isoDep_PCBhasDID(pcb) (((pcb) & ISODEP_PCB_DID_BIT) == ISODEP_PCB_DID_BIT) /*!< Checks if pcb is indicating DID      */
#define isoDep_PCBhasNAD(pcb) (((pcb) & ISODEP_PCB_NAD_BIT) == ISODEP_PCB_NAD_BIT) /*!< Checks if pcb is indicating NAD      */

#define isoDep_PCBisIChaining(pcb) (isoDep_PCBisIBlock(pcb) && isoDep_PCBisChaining(pcb)) /*!< Checks if pcb is I-Block indicating chaining*/

#define isoDep_PCBisSDeselect(pcb) (isoDep_PCBisSBlock(pcb) && isoDep_PCBisDeselect(pcb)) /*!< Checks if pcb is S-Block indicating DESELECT*/
#define isoDep_PCBisSWTX(pcb) (isoDep_PCBisSBlock(pcb) && isoDep_PCBisWTX(pcb))           /*!< Checks if pcb is S-Block indicating WTX     */

#define isoDep_PCBisRACK(pcb) (isoDep_PCBisRBlock(pcb) && isoDep_PCBisACK(pcb)) /*!< Checks if pcb is R-Block indicating ACK     */
#define isoDep_PCBisRNAK(pcb) (isoDep_PCBisRBlock(pcb) && isoDep_PCBisNAK(pcb)) /*!< Checks if pcb is R-Block indicating NAK     */

#define isoDep_PCBIBlock(bn) ((uint8_t)(0x00U | ISODEP_PCB_IBLOCK | ISODEP_PCB_B2_BIT | ((bn) & ISODEP_PCB_BN_MASK))) /*!< Returns an I-Block with the given block number (bn)                     */
#define isoDep_PCBIBlockChaining(bn) ((uint8_t)(isoDep_PCBIBlock(bn) | ISODEP_PCB_CHAINING_BIT))                      /*!< Returns an I-Block with the given block number (bn) indicating chaining */

#define isoDep_PCBRBlock(bn) ((uint8_t)(0x00U | ISODEP_PCB_RBLOCK | ISODEP_PCB_B6_BIT | ISODEP_PCB_B2_BIT | ((bn) & ISODEP_PCB_BN_MASK))) /*!< Returns an R-Block with the given block number (bn)                */
#define isoDep_PCBRACK(bn) ((uint8_t)(isoDep_PCBRBlock(bn) | ISODEP_PCB_ACK))                                                             /*!< Returns an R-Block with the given block number (bn) indicating ACK */
#define isoDep_PCBRNAK(bn) ((uint8_t)(isoDep_PCBRBlock(bn) | ISODEP_PCB_NAK))                                                             /*!< Returns an R-Block with the given block number (bn) indicating NAK */

#define isoDep_GetBN(pcb) ((uint8_t)((pcb) & ISODEP_PCB_BN_MASK))                             /*!< Returns the block number (bn) from the given pcb */
#define isoDep_GetWTXM(inf) ((uint8_t)((inf) & ISODEP_SWTX_WTXM_MASK))                        /*!< Returns the WTX value from the given inf byte    */
#define isoDep_isWTXMValid(wtxm) (((wtxm) >= ISODEP_WTXM_MIN) && ((wtxm) <= ISODEP_WTXM_MAX)) /*!< Checks if the given wtxm is valid                */

#define isoDep_WTXMListenerMax(fwt) (MIN((uint8_t)(ISODEP_FWT_LIS_MAX / (fwt)), ISODEP_WTXM_MAX)) /*!< Calculates the Max WTXM value for the given fwt as a Listener    */

#define isoDepCalcdSGFT(s) (384U * ((uint32_t)1U << (s))) /*!< Calculates the dSFGT with given SFGI  Digital 1.1  13.8.2.1 & A.6*/
#define isoDepCalcSGFT(s) (4096U * ((uint32_t)1U << (s))) /*!< Calculates the SFGT with given SFGI  Digital 1.1  13.8.2         */

#define isoDep_PCBNextBN(bn) (((uint8_t)(bn) ^ 0x01U) & ISODEP_PCB_BN_MASK) /*!< Returns the value of the next block number based on bn     */
#define isoDep_PCBPrevBN(bn) isoDep_PCBNextBN(bn)                           /*!< Returns the value of the previous block number based on bn */
#define isoDep_ToggleBN(bn) ((bn) = (((bn) ^ 0x01U) & ISODEP_PCB_BN_MASK))  /*!< Toggles the block number value of the given bn             */

#define isoDep_WTXAdjust(v) ((v) - ((v) >> 3)) /*!< Adjust WTX timer value to a percentage of the total, current 88% */

/*! ISO 14443-4 7.5.6.2 & Digital 1.1 - 15.2.6.2  The CE SHALL NOT attempt error recovery and remains in Rx mode upon Transmission or a Protocol Error */
#define isoDepReEnableRx(rxB, rxBL, rxL) RFal_TransceiveBlockingTx(NULL, 0, rxB, rxBL, rxL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FWT_NONE)

#define isoDepTimerStart(timer, time_ms) (timer) = timerCalculateTimer((uint16_t)(time_ms)) /*!< Configures and starts the WTX timer  */
#define isoDepTimerisExpired(timer) timerIsExpired(timer)                                   /*!< Checks WTX timer has expired         */

static RFal_ISO_Dep gIsoDep;    /*!< ISO-DEP Module instance               */

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_Tx(uint8_t pcb, const uint8_t *txBuf, uint8_t *infBuf, uint16_t infLen, uint32_t fwt)
{
  uint8_t *txBlock;
  uint16_t txBufLen;
  uint8_t computedPcb;
  RFal_TransceiveContext ctx;

  txBlock = infBuf;      /* Point to beginning of the INF, and go backwards     */
  gIsoDep.lastPCB = pcb; /* Store the last PCB sent                             */

  if (infLen > 0U) {
    if (((uint32_t)infBuf - (uintptr_t)txBuf) < gIsoDep.hdrLen) {
      /* Check that we can fit the header in the given space */
      return NFC_MemoryError;
    }
  }

  /*******************************************************************************/
  /* Compute optional PCB bits */
  computedPcb = pcb;
  if (((gIsoDep.did != RFAL_ISODEP_NO_DID) && (gIsoDep.did != RFAL_ISODEP_DID_00)) || ((gIsoDep.did == RFAL_ISODEP_DID_00) && (gIsoDep.lastDID00))) {
    computedPcb |= ISODEP_PCB_DID_BIT;
  }
  if (gIsoDep.nad != RFAL_ISODEP_NO_NAD) {
    computedPcb |= ISODEP_PCB_NAD_BIT;
  }
  if ((gIsoDep.isTxChaining) && (isoDep_PCBisIBlock(computedPcb))) {
    computedPcb |= ISODEP_PCB_CHAINING_BIT;
  }

  /*******************************************************************************/
  /* Compute Payload on the given txBuf, start by the PCB | DID | NAD | before INF */

  if ((ISODEP_PCB_NAD_BIT & computedPcb) != 0U) {
    *(--txBlock) = gIsoDep.nad; /* NAD is optional */
  }

  if ((ISODEP_PCB_DID_BIT & computedPcb) != 0U) {
    *(--txBlock) = gIsoDep.did; /* DID is optional */
  }

  *(--txBlock) = computedPcb; /* PCB always present */

  txBufLen = (infLen + (uint16_t)((uintptr_t)infBuf - (uintptr_t)txBlock)); /* Calculate overall buffer size */

  if (txBufLen > (gIsoDep.fsx - ISODEP_CRC_LEN)) {
    /* Check if msg length violates the maximum frame size FSC */
    return NFC_Unsupport;
  }

  rfalCreateByteFlagsTxRxContext(ctx, txBlock, txBufLen, gIsoDep.rxBuf, gIsoDep.rxBufLen, gIsoDep.rxLen, RFAL_TXRX_FLAGS_DEFAULT, ((gIsoDep.role == ISODEP_ROLE_PICC) ? RFAL_FWT_NONE : fwt));
  return RFal_StartTransceive(&ctx);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_HandleControlMsg(RFal_ISO_Dep_ControlMsg controlMsg, uint8_t param)
{
  uint8_t pcb;
  uint8_t infLen;
  uint32_t fwtTemp;

  infLen = 0;
  fwtTemp = (gIsoDep.fwt + gIsoDep.dFwt);
  memset(gIsoDep.ctrlBuf, 0x00, ISODEP_CONTROLMSG_BUF_LEN);

  switch (controlMsg) {
    /*******************************************************************************/
    case ISODEP_R_ACK:

      if (gIsoDep.cntRRetrys++ > gIsoDep.maxRetriesR) {
        return NFC_SlaveTimeout; /* NFC Forum mandates timeout or transmission error depending on previous errors */
      }

      pcb = isoDep_PCBRACK(gIsoDep.blockNumber);
      break;

    /*******************************************************************************/
    case ISODEP_R_NAK:
      if ((gIsoDep.cntRRetrys++ > gIsoDep.maxRetriesR) || /* Max R Block retries reached */
          (gIsoDep.cntSWtxNack >= gIsoDep.maxRetriesSnWTX)) {
        /* Max number PICC is allowed to respond with S(WTX) to R(NAK) */
        return NFC_SlaveTimeout;
      }

      pcb = isoDep_PCBRNAK(gIsoDep.blockNumber);
      break;

    /*******************************************************************************/
    case ISODEP_S_WTX:
      if ((gIsoDep.cntSWtxRetrys++ > gIsoDep.maxRetriesSWTX) && (gIsoDep.maxRetriesSWTX != RFAL_ISODEP_MAX_WTX_RETRYS_ULTD)) {
        return NFC_ProtocolError;
      }

      /* Check if WTXM is valid */
      if (!isoDep_isWTXMValid(param)) {
        return NFC_ProtocolError;
      }

      if (gIsoDep.role == ISODEP_ROLE_PCD) {
        /* Calculate temp Wait Time eXtension */
        fwtTemp = (gIsoDep.fwt * param);
        fwtTemp = MIN(RFAL_ISODEP_MAX_FWT, fwtTemp);
        fwtTemp += gIsoDep.dFwt;
      }

      pcb = ISODEP_PCB_SWTX;
      gIsoDep.ctrlBuf[RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN + infLen++] = param;
      break;

    /*******************************************************************************/
    case ISODEP_S_DSL:
      if (gIsoDep.cntSDslRetrys++ > gIsoDep.maxRetriesSDSL) {
        return NFC_SlaveTimeout; /* NFC Forum mandates timeout or transmission error depending on previous errors */
      }

      if (gIsoDep.role == ISODEP_ROLE_PCD) {
        /* Digital 1.0 - 13.2.7.3 Poller must wait fwtDEACTIVATION */
        fwtTemp = ISODEP_FWT_DEACTIVATION;
        gIsoDep.state = ISODEP_ST_PCD_WAIT_DSL;
      }
      pcb = ISODEP_PCB_SDSL;
      break;

    /*******************************************************************************/
    default:
      return NFC_InternalError;
  }

  return isoDepTx(pcb, gIsoDep.ctrlBuf, &gIsoDep.ctrlBuf[RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN], infLen, fwtTemp);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_ReSendControlMsg(void)
{
  if (isoDep_PCBisRACK(gIsoDep.lastPCB)) {
    return isoDepHandleControlMsg(ISODEP_R_ACK, RFAL_ISODEP_NO_PARAM);
  }

  if (isoDep_PCBisRNAK(gIsoDep.lastPCB)) {
    return isoDepHandleControlMsg(ISODEP_R_NAK, RFAL_ISODEP_NO_PARAM);
  }

  if (isoDep_PCBisSDeselect(gIsoDep.lastPCB)) {
    return isoDepHandleControlMsg(ISODEP_S_DSL, RFAL_ISODEP_NO_PARAM);
  }

  if (isoDep_PCBisSWTX(gIsoDep.lastPCB)) {
    return isoDepHandleControlMsg(ISODEP_S_WTX, gIsoDep.lastWTXM);
  }

  return NFC_WrongState;
}

/*******************************************************************************/
void RFal_ISO_Dep_Init(void)
{
  gIsoDep.state           = ISODEP_ST_IDLE;
  gIsoDep.role            = ISODEP_ROLE_PCD;
  gIsoDep.did             = RFAL_ISODEP_NO_DID;
  gIsoDep.nad             = RFAL_ISODEP_NO_NAD;
  gIsoDep.blockNumber     = 0;
  gIsoDep.isTxChaining    = false;
  gIsoDep.isRxChaining    = false;
  gIsoDep.lastDID00       = false;
  gIsoDep.lastPCB         = ISODEP_PCB_INVALID;
  gIsoDep.fsx             = (uint16_t)RFAL_ISODEP_FSX_16;
  gIsoDep.ourFsx          = (uint16_t)RFAL_ISODEP_FSX_16;
  gIsoDep.hdrLen          = RFAL_ISODEP_PCB_LEN;

  gIsoDep.rxLen           = NULL;
  gIsoDep.rxBuf           = NULL;
  gIsoDep.rxBufInfPos     = 0U;
  gIsoDep.txBufInfPos     = 0U;

  gIsoDep.isTxPending     = false;
  gIsoDep.isWait4WTX      = false;

  gIsoDep.compMode        = RFAL_COMPLIANCE_MODE_NFC;
  gIsoDep.maxRetriesR     = RFAL_ISODEP_MAX_R_RETRYS;
  gIsoDep.maxRetriesI     = RFAL_ISODEP_MAX_I_RETRYS;
  gIsoDep.maxRetriesSDSL  = RFAL_ISODEP_MAX_DSL_RETRYS;
  gIsoDep.maxRetriesSWTX  = RFAL_ISODEP_MAX_WTX_RETRYS;
  gIsoDep.maxRetriesSnWTX = RFAL_ISODEP_MAX_WTX_NACK_RETRYS;
  gIsoDep.maxRetriesRATS  = RFAL_ISODEP_RATS_RETRIES;

  gIsoDep.WTXTimer        = 0U;

  gIsoDep.APDURxPos       = 0;
  gIsoDep.APDUTxPos       = 0;
  gIsoDep.APDUParam.rxLen = NULL;
  gIsoDep.APDUParam.rxBuf = NULL;
  gIsoDep.APDUParam.txBuf = NULL;

  isoDepClearCounters();
}

/*******************************************************************************/
void RFal_ISO_Dep_InitWithParams(RFal_ComplianceMode compMode, uint8_t maxRetriesR, uint8_t maxRetriesSnWTX, uint8_t maxRetriesSWTX, uint8_t maxRetriesSDSL, uint8_t maxRetriesI, uint8_t maxRetriesRATS)
{
  RFal_ISO_Dep_Init();

  gIsoDep.compMode = compMode;
  gIsoDep.maxRetriesR = maxRetriesR;
  gIsoDep.maxRetriesSnWTX = maxRetriesSnWTX;
  gIsoDep.maxRetriesSWTX = maxRetriesSWTX;
  gIsoDep.maxRetriesSDSL = maxRetriesSDSL;
  gIsoDep.maxRetriesI = maxRetriesI;
  gIsoDep.maxRetriesRATS = maxRetriesRATS;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_DataExchangePCD(uint16_t *outActRxLen, bool *outIsChaining)
{
  NFC_OpResult ret;
  uint8_t rxPCB;

  /* Check out parameters */
  if ((outActRxLen == NULL) || (outIsChaining == NULL)) {
    return NFC_InvalidParameter;
  }

  *outIsChaining = false;

  /* Calculate header required and check if the buffers InfPositions are suitable */
  gIsoDep.hdrLen = RFAL_ISODEP_PCB_LEN;
  if ((gIsoDep.did != RFAL_ISODEP_NO_DID) && (gIsoDep.did != RFAL_ISODEP_DID_00)) {
    gIsoDep.hdrLen += RFAL_ISODEP_DID_LEN;
  }
  if (gIsoDep.nad != RFAL_ISODEP_NO_NAD) {
    gIsoDep.hdrLen += RFAL_ISODEP_NAD_LEN;
  }

  /* check if there is enough space before the infPos to append ISO-DEP headers on rx and tx */
  if ((gIsoDep.rxBufInfPos < gIsoDep.hdrLen) || (gIsoDep.txBufInfPos < gIsoDep.hdrLen)) {
    return NFC_InvalidParameter;
  }

  /*******************************************************************************/
  switch (gIsoDep.state) {
    /*******************************************************************************/
    case ISODEP_ST_IDLE:
      return NFC_OK;

    /*******************************************************************************/
    case ISODEP_ST_PCD_TX:
      ret = isoDepTx(isoDep_PCBIBlock(gIsoDep.blockNumber), gIsoDep.txBuf, &gIsoDep.txBuf[gIsoDep.txBufInfPos], gIsoDep.txBufLen, (gIsoDep.fwt + gIsoDep.dFwt));
      switch (ret) {
        case NFC_OK:
          gIsoDep.state = ISODEP_ST_PCD_RX;
          break;

        default:
          return ret;
      }
    /* fall through */

    /*******************************************************************************/
    case ISODEP_ST_PCD_WAIT_DSL: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */
    case ISODEP_ST_PCD_RX:

      ret = RFal_GetTransceiveStatus();
      switch (ret) {
        /* Data rcvd with error or timeout -> Send R-NAK */
        case NFC_SlaveTimeout:
        case NFC_CRC_Error:
        case NFC_ParityError:
        case NFC_FramingError:         /* added to handle test cases scenario TC_POL_NFCB_T4AT_BI_82_x_y & TC_POL_NFCB_T4BT_BI_82_x_y */
        case NFC_ImcompleteByte: /* added to handle test cases scenario TC_POL_NFCB_T4AT_BI_82_x_y & TC_POL_NFCB_T4BT_BI_82_x_y  */

          if (gIsoDep.isRxChaining) {
            /* Rule 5 - In PICC chaining when a invalid/timeout occurs -> R-ACK */
            ret = isoDepHandleControlMsg(ISODEP_R_ACK, RFAL_ISODEP_NO_PARAM);
            if(ret < NFC_OK)
            {
                return ret;
            }
          } else if (gIsoDep.state == ISODEP_ST_PCD_WAIT_DSL) {
            /* Rule 8 - If s-Deselect response fails MAY retransmit */
            ret = isoDepHandleControlMsg(ISODEP_S_DSL, RFAL_ISODEP_NO_PARAM);
            if(ret < NFC_OK)
            {
                return ret;
            }
          } else {
            /* Rule 4 - When a invalid block or timeout occurs -> R-NACK */
            ret = isoDepHandleControlMsg(ISODEP_R_NAK, RFAL_ISODEP_NO_PARAM);
            if(ret < NFC_OK)
            {
                return ret;
            }
          }
          return NFC_Busy;

        case NFC_OK:
          break;

        case NFC_Busy:
          return NFC_Busy; /* Debug purposes */

        default:
          return ret;
      }

      /*******************************************************************************/
      /* No error, process incoming msg                                              */
      /*******************************************************************************/

      (*outActRxLen) = rfalConvBitsToBytes(*outActRxLen);

      /* Check rcvd msg length, cannot be less then the expected header */
      if (((*outActRxLen) < gIsoDep.hdrLen) || ((*outActRxLen) >= gIsoDep.ourFsx)) {
        return NFC_ProtocolError;
      }

      /* Grab rcvd PCB */
      rxPCB = gIsoDep.rxBuf[ISODEP_PCB_POS];

      /* EMVCo doesn't allow usage of for CID or NAD   EMVCo 2.6 TAble 10.2 */
      if ((gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) && (isoDep_PCBhasDID(rxPCB) || isoDep_PCBhasNAD(rxPCB))) {
        return NFC_ProtocolError;
      }

      /* If we are expecting DID, check if PCB signals its presence and if device ID match*/
      if ((gIsoDep.did != RFAL_ISODEP_NO_DID) && (gIsoDep.did != RFAL_ISODEP_DID_00) && ((!isoDep_PCBhasDID(rxPCB)) || (gIsoDep.did != gIsoDep.rxBuf[ISODEP_DID_POS]))) {
        return NFC_ProtocolError;
      }

      /*******************************************************************************/
      /* Process S-Block                                                             */
      /*******************************************************************************/
      if (isoDep_PCBisSBlock(rxPCB)) {
        /* Check if is a Wait Time eXtension */
        if (isoDep_PCBisSWTX(rxPCB)) {
          /* Check if PICC has requested S(WTX) as response to R(NAK)  EMVCo 3.0 10.3.5.5 / Digital 2.0  16.2.6.5 */
          if (isoDep_PCBisRNAK(gIsoDep.lastPCB)) {
            gIsoDep.cntSWtxNack++;  /* Count S(WTX) upon R(NAK) */
            gIsoDep.cntRRetrys = 0; /* Reset R-Block counter has PICC has responded */
          } else {
            gIsoDep.cntSWtxNack = 0; /* Reset R(NACK)->S(WTX) counter */
          }
          /* Rule 3 - respond to S-block: get 1st INF byte S(STW): Power + WTXM */
          ret = isoDepHandleControlMsg(ISODEP_S_WTX, isoDep_GetWTXM(gIsoDep.rxBuf[gIsoDep.hdrLen]));
          if(ret < NFC_OK)
          {
              return ret;
          }
  
          return NFC_Busy;
        }

        /* Check if is a deselect response */
        if (isoDep_PCBisSDeselect(rxPCB)) {
          if (gIsoDep.state == ISODEP_ST_PCD_WAIT_DSL) {
            RFal_ISO_Dep_Initialize(); /* Session finished reInit vars */
            return NFC_OK;
          }

          /* Deselect response not expected  */
          /* fall through to PROTO error */
        }
        /* Unexpected S-Block */
        return NFC_ProtocolError;
      }

      /*******************************************************************************/
      /* Process R-Block                                                             */
      /*******************************************************************************/
      else if (isoDep_PCBisRBlock(rxPCB)) {
        if (isoDep_PCBisRACK(rxPCB)) {
          /* Check if is a R-ACK */
          if (isoDep_GetBN(rxPCB) == gIsoDep.blockNumber) {
            /* Expected block number  */
            /* Rule B - ACK with expected bn -> Increment block number */
            gIsoDep.blockNumber = isoDep_PCBNextBN(gIsoDep.blockNumber);

            /* R-ACK only allowed when PCD chaining */
            if (!gIsoDep.isTxChaining) {
              return NFC_ProtocolError;
            }

            /* Rule 7 - Chaining transaction done, continue chaining */
            isoDepClearCounters();
            return NFC_OK; /* This block has been transmitted */
          } else {
            /* Rule 6 - R-ACK with wrong block number retransmit */
            /* Digital 2.0  16.2.5.4 - Retransmit maximum two times                       */
            /* EMVCo 3.0 10.3.4.3 -  PCD may re-transmit the last I-Block or report error */
            if (gIsoDep.cntIRetrys++ < gIsoDep.maxRetriesI) {
              gIsoDep.cntRRetrys = 0; /* Clear R counter only */
              gIsoDep.state = ISODEP_ST_PCD_TX;
              return NFC_Busy;
            }
            return NFC_SlaveTimeout; /* NFC Forum mandates timeout or transmission error depending on previous errors */
          }
        } else {
          /* Unexpected R-Block */
          return NFC_ProtocolError;
        }
      }

      /*******************************************************************************/
      /* Process I-Block                                                             */
      /*******************************************************************************/
      else if (isoDep_PCBisIBlock(rxPCB)) {
        /*******************************************************************************/
        /* is PICC performing chaining                                                 */
        if (isoDep_PCBisChaining(rxPCB)) {
          gIsoDep.isRxChaining = true;
          *outIsChaining = true;

          if (isoDep_GetBN(rxPCB) == gIsoDep.blockNumber) {
            /* Rule B - ACK with correct block number -> Increase Block number */
            isoDep_ToggleBN(gIsoDep.blockNumber);

            isoDepClearCounters(); /* Clear counters in case R counter is already at max */

            /* Rule 2 - Send ACK */
            ret = isoDepHandleControlMsg(ISODEP_R_ACK, RFAL_ISODEP_NO_PARAM);
            if(ret < NFC_OK)
            {
                return ret;
            }

            /* Received I-Block with chaining, send current data to DH */

            /* remove ISO DEP header, check is necessary to move the INF data on the buffer */
            *outActRxLen -= gIsoDep.hdrLen;
            if ((gIsoDep.hdrLen != gIsoDep.rxBufInfPos) && (*outActRxLen > 0U)) {
              ST_MEMMOVE(&gIsoDep.rxBuf[gIsoDep.rxBufInfPos], &gIsoDep.rxBuf[gIsoDep.hdrLen], *outActRxLen);
            }

            isoDepClearCounters();
            return NFC_Again; /* Send Again signalling to run again, but some chaining data has arrived */
          } else {
            /* Rule 5 - PICC chaining invalid I-Block -> R-ACK */
            ret = isoDepHandleControlMsg(ISODEP_R_ACK, RFAL_ISODEP_NO_PARAM);
            if(ret < NFC_OK)
            {
                return ret;
            }
  
          }
          return NFC_Busy;
        }

        gIsoDep.isRxChaining = false; /* clear PICC chaining flag */

        if (isoDep_GetBN(rxPCB) == gIsoDep.blockNumber) {
          /* Rule B - I-Block with correct block number -> Increase Block number */
          isoDep_ToggleBN(gIsoDep.blockNumber);

          /* I-Block transaction done successfully */

          /* remove ISO DEP header, check is necessary to move the INF data on the buffer */
          *outActRxLen -= gIsoDep.hdrLen;
          if ((gIsoDep.hdrLen != gIsoDep.rxBufInfPos) && (*outActRxLen > 0U)) {
            ST_MEMMOVE(&gIsoDep.rxBuf[gIsoDep.rxBufInfPos], &gIsoDep.rxBuf[gIsoDep.hdrLen], *outActRxLen);
          }

          gIsoDep.state = ISODEP_ST_IDLE;
          isoDepClearCounters();
          return NFC_OK;
        } else {
          if ((gIsoDep.compMode != RFAL_COMPLIANCE_MODE_ISO)) {
            /* Invalid Block (not chaining) -> Raise error   Digital 1.1  15.2.6.4   EMVCo 2.6  10.3.5.4 */
            return NFC_ProtocolError;
          }

          /* Rule 4 - Invalid Block -> R-NAK */
          ret = isoDepHandleControlMsg(ISODEP_R_NAK, RFAL_ISODEP_NO_PARAM);
          if(ret < NFC_OK)
          {
              return ret;
          }
  
          return NFC_Busy;
        }
      } else {
        /* not S/R/I - Block */
        return NFC_ProtocolError;
      }
    /* fall through */

    /*******************************************************************************/
    default: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */
      /* MISRA 16.4: no empty default (comment will suffice) */
      break;
  }

  return NFC_InternalError;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_Deselect(void)
{
  NFC_OpResult ret;

  ret = RFal_ISO_Dep_StartDeselect();
  if(ret < NFC_OK)
  {
      return ret;
  }

  rfalRunBlocking(ret, RFal_ISO_Dep_GetDeselectStatus());

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_StartDeselect(void)
{
  /* Check if  rx parameters have been set before, otherwise use global variable *
   * To cope with a Deselect after RATS\ATTRIB without any I-Block exchanged     */
  if ((gIsoDep.rxLen == NULL) || (gIsoDep.rxBuf == NULL)) {
    /* Using local static vars and static config to cope with a Deselect after     *
     * RATS\ATTRIB without any I-Block exchanged                                   */
    gIsoDep.rxLen = &gIsoDep.ctrlRxLen;
    gIsoDep.rxBuf = gIsoDep.ctrlBuf;
    gIsoDep.rxBufLen = ISODEP_CONTROLMSG_BUF_LEN - (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN);
    gIsoDep.rxBufInfPos = (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN);
    gIsoDep.txBufInfPos = (RFAL_ISODEP_PCB_LEN + RFAL_ISODEP_DID_LEN);
  }

  /* Send DSL request */
  return isoDepHandleControlMsg(ISODEP_S_DSL, RFAL_ISODEP_NO_PARAM);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_GetDeselectStatus(void)
{
  NFC_OpResult ret;
  bool dummyB;

  ret = isoDepDataExchangePCD(gIsoDep.rxLen, &dummyB);
  if(ret == NFC_Busy)
  {
      return NFC_Busy;
  }

  RFal_ISO_Dep_Initialize();
  return ret;
}

/*******************************************************************************/
uint32_t RFal_ISO_Dep_FWI2FWT(uint8_t fwi)
{
  uint32_t result;
  uint8_t tmpFWI;

  tmpFWI = fwi;

  /* RFU values -> take the default value
   * Digital 1.0  11.6.2.17  FWI[1,14]
   * Digital 1.1  7.6.2.22   FWI[0,14]
   * EMVCo 2.6    Table A.5  FWI[0,14] */
  if (tmpFWI > ISODEP_FWI_MAX) {
    tmpFWI = RFAL_ISODEP_FWI_DEFAULT;
  }

  /* FWT = (256 x 16/fC) x 2^FWI => 2^(FWI+12)  Digital 1.1  13.8.1 & 7.9.1 */

  result = ((uint32_t)1U << (tmpFWI + 12U));
  result = MIN(RFAL_ISODEP_MAX_FWT, result); /* Maximum Frame Waiting Time must be fulfilled */

  return result;
}

/*******************************************************************************/
uint16_t RFal_ISO_Dep_FSxI2FSx(uint8_t FSxI)
{
  uint16_t fsx;
  uint8_t fsi;

  /* Enforce maximum FSxI/FSx allowed - NFC Forum and EMVCo differ */
  fsi = ((gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) ? MIN(FSxI, RFAL_ISODEP_FSDI_MAX_EMV) : MIN(FSxI, RFAL_ISODEP_FSDI_MAX_NFC));

  switch (fsi) {
    case (uint8_t)RFAL_ISODEP_FSXI_16:
      fsx = (uint16_t)RFAL_ISODEP_FSX_16;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_24:
      fsx = (uint16_t)RFAL_ISODEP_FSX_24;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_32:
      fsx = (uint16_t)RFAL_ISODEP_FSX_32;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_40:
      fsx = (uint16_t)RFAL_ISODEP_FSX_40;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_48:
      fsx = (uint16_t)RFAL_ISODEP_FSX_48;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_64:
      fsx = (uint16_t)RFAL_ISODEP_FSX_64;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_96:
      fsx = (uint16_t)RFAL_ISODEP_FSX_96;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_128:
      fsx = (uint16_t)RFAL_ISODEP_FSX_128;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_256:
      fsx = (uint16_t)RFAL_ISODEP_FSX_256;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_512:
      fsx = (uint16_t)RFAL_ISODEP_FSX_512;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_1024:
      fsx = (uint16_t)RFAL_ISODEP_FSX_1024;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_2048:
      fsx = (uint16_t)RFAL_ISODEP_FSX_2048;
      break;
    case (uint8_t)RFAL_ISODEP_FSXI_4096:
      fsx = (uint16_t)RFAL_ISODEP_FSX_4096;
      break;
    default:
      fsx = (uint16_t)RFAL_ISODEP_FSX_256;
      break;
  }
  return fsx;
}

/*******************************************************************************/
bool RFal_ISO_Dep_IsRats(const uint8_t *buf, uint8_t bufLen)
{
  if (buf != NULL) {
    if ((RFAL_ISODEP_CMD_RATS == (uint8_t)*buf) && (sizeof(RFal_ISO_Dep_Rats) == bufLen)) {
      return true;
    }
  }
  return false;
}

/*******************************************************************************/
bool RFal_ISO_Dep_IsAttrib(const uint8_t *buf, uint8_t bufLen)
{
  if (buf != NULL) {
    if ((RFAL_ISODEP_CMD_ATTRIB == (uint8_t)*buf) &&
        (RFAL_ISODEP_ATTRIB_REQ_MIN_LEN <= bufLen) &&
        ((RFAL_ISODEP_ATTRIB_REQ_MIN_LEN + RFAL_ISODEP_ATTRIB_HLINFO_LEN) >= bufLen)) {
      return true;
    }
  }
  return false;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_ListenStartActivation(RFal_ISO_Dep_AtsParam *atsParam, const RFal_ISO_Dep_AttribResParam *attribResParam, const uint8_t *buf, uint16_t bufLen, RFal_ISO_Dep_ListenActvParam actParam)
{
  uint8_t *txBuf;
  uint8_t bufIt;
  const uint8_t *buffer = buf;

  /*******************************************************************************/
  bufIt = 0;
  txBuf = (uint8_t *)actParam.rxBuf; /* Use the rxBuf as TxBuf as well, the struct enforces a size enough RFAL_MAX( NFCA_ATS_MAX_LEN, NFCB_ATTRIB_RES_MAX_LEN ) */
  gIsoDep.txBR = RFAL_BR_106;
  gIsoDep.rxBR = RFAL_BR_106;

  /* Check for a valid buffer pointer */
  if (buffer == NULL) {
    return NFC_InvalidParameter;
  }

  /*******************************************************************************/
  if (*buffer == RFAL_ISODEP_CMD_RATS) {
    /* Check ATS parameters */
    if (atsParam == NULL) {
      return NFC_InvalidParameter;
    }

    /* If requested copy RATS to device info */
    if (actParam.isoDepDev != NULL) {
      memcpy((uint8_t *)&actParam.isoDepDev->activation.A.Poller.RATS, buffer, sizeof(RFal_ISO_Dep_Rats)); /* Copy RATS' CMD + PARAM */
    }

    /*******************************************************************************/
    /* Process RATS                                                                */
    buffer++;
    gIsoDep.fsx = RFal_ISO_Dep_FSxI2FSx((((*buffer) & RFAL_ISODEP_RATS_PARAM_FSDI_MASK) >> RFAL_ISODEP_RATS_PARAM_FSDI_SHIFT));
    gIsoDep.did = (*buffer & RFAL_ISODEP_DID_MASK);

    /*******************************************************************************/
    /* Digital 1.1  13.6.1.8 - DID has to between 0 and 14 */
    if (gIsoDep.did > RFAL_ISODEP_DID_MAX) {
      return NFC_ProtocolError;
    }

    /* Check if we are configured to support DID */
    if (!atsParam->didSupport) {
      gIsoDep.did = RFAL_ISODEP_NO_DID; /* Even if requested by PCD to use a certain DID, the ATS setting prevails */
    }

    /*******************************************************************************/
    /* Check RFAL supported bit rates  */
    if ((((atsParam->ta & RFAL_ISODEP_ATS_TA_DPL_212) != 0U) || ((atsParam->ta & RFAL_ISODEP_ATS_TA_DLP_212) != 0U)) ||
        (((atsParam->ta & RFAL_ISODEP_ATS_TA_DPL_424) != 0U) || ((atsParam->ta & RFAL_ISODEP_ATS_TA_DLP_424) != 0U)) ||
        (((atsParam->ta & RFAL_ISODEP_ATS_TA_DPL_848) != 0U) || ((atsParam->ta & RFAL_ISODEP_ATS_TA_DLP_848) != 0U))) {
      return NFC_Unsupport;
    }

    /* Enforce proper FWI configuration */
    if (atsParam->fwi > ISODEP_FWI_LIS_MAX) {
      atsParam->fwi = ISODEP_FWI_LIS_MAX;
    }

    gIsoDep.atsTA = atsParam->ta;
    gIsoDep.fwt = RFal_ISO_Dep_FWI2FWT(atsParam->fwi);
    gIsoDep.ourFsx = RFal_ISO_Dep_FSxI2FSx(atsParam->fsci);

    /* Ensure proper/maximum Historical Bytes length  */
    atsParam->hbLen = MIN(RFAL_ISODEP_ATS_HB_MAX_LEN, atsParam->hbLen);

    /*******************************************************************************/
    /* Compute ATS                                                                 */

    txBuf[bufIt++] = (RFAL_ISODEP_ATS_HIST_OFFSET + atsParam->hbLen); /* TL */
    txBuf[bufIt++] = ((RFAL_ISODEP_ATS_T0_TA_PRESENCE_MASK | RFAL_ISODEP_ATS_T0_TB_PRESENCE_MASK |
                       RFAL_ISODEP_ATS_T0_TC_PRESENCE_MASK) |
                      atsParam->fsci); /* T0 */
    txBuf[bufIt++] = atsParam->ta;     /* TA */
    txBuf[bufIt++] = ((atsParam->fwi << RFAL_ISODEP_RATS_PARAM_FSDI_SHIFT) |
                      (atsParam->sfgi & RFAL_ISODEP_RATS_PARAM_FSDI_MASK));           /* TB */
    txBuf[bufIt++] = (uint8_t)((atsParam->didSupport) ? RFAL_ISODEP_ATS_TC_DID : 0U); /* TC */

    if (atsParam->hbLen > 0U) { /* MISRA 21.18 */
      memcpy(&txBuf[bufIt], atsParam->hb, atsParam->hbLen); /* T1-Tk */
      bufIt += atsParam->hbLen;
    }

    gIsoDep.state = ISODEP_ST_PICC_ACT_ATS;
  }
  /*******************************************************************************/
  else if (*buffer == RFAL_ISODEP_CMD_ATTRIB) {
    /* Check ATTRIB parameters */
    if (attribResParam == NULL) {
      return NFC_InvalidParameter;
    }

    /*  REMARK: ATTRIB handling */
    return NFC_Unsupport;
  } else {
    return NFC_InvalidParameter;
  }

  gIsoDep.actvParam = actParam;

  /*******************************************************************************/
  /* If requested copy to ISO-DEP device info */
  if (actParam.isoDepDev != NULL) {
    actParam.isoDepDev->info.DID = gIsoDep.did;
    actParam.isoDepDev->info.FSx = gIsoDep.fsx;
    actParam.isoDepDev->info.FWT = gIsoDep.fwt;
    actParam.isoDepDev->info.dFWT = 0;
    actParam.isoDepDev->info.DSI = gIsoDep.txBR;
    actParam.isoDepDev->info.DRI = gIsoDep.rxBR;
  }

  return RFal_TransceiveBlockingTx(txBuf, bufIt, (uint8_t *)actParam.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), actParam.rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FWT_NONE);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_ListenGetActivationStatus(void)
{
  NFC_OpResult err;
  uint8_t *txBuf;
  uint8_t bufIt;

  RFal_BitRate dsi;
  RFal_BitRate dri;

  /* Check if Activation is running */
  if (gIsoDep.state < ISODEP_ST_PICC_ACT_ATS) {
    return NFC_WrongState;
  }

  /* Check if Activation has finished already */
  if (gIsoDep.state >= ISODEP_ST_PICC_RX) {
    return NFC_OK;
  }

  /*******************************************************************************/
  /* Check for incoming msg */
  err = RFal_GetTransceiveStatus();
  switch (err) {
    /*******************************************************************************/
    case NFC_OK:
      break;

    /*******************************************************************************/
    case NFC_LinkLoss:
    case NFC_Busy:
    case NFC_SleepRequest: /* S(DSL) handled by lower layer should cause an error reported */
      return err;

    /*******************************************************************************/
    case NFC_CRC_Error:
    case NFC_ParityError:
    case NFC_FramingError:

      /* ISO14443 4  5.6.2.2 2  If ATS has been replied upon a invalid block, PICC disables the PPS responses */
      if (gIsoDep.state == ISODEP_ST_PICC_ACT_ATS) {
        gIsoDep.state = ISODEP_ST_PICC_RX;
        break;
      }
    /* fall through */

    /*******************************************************************************/
    default: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */
      /* Re-enable the receiver and wait for another frame */
      isoDepReEnableRx((uint8_t *)gIsoDep.actvParam.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.actvParam.rxLen);

      return NFC_Busy;
  }

  txBuf = (uint8_t *)gIsoDep.actvParam.rxBuf; /* Use the rxBuf as TxBuf as well, the struct enforces a size enough  RFAL_MAX(NFCA_PPS_RES_LEN, ISODEP_DSL_MAX_LEN) */
  dri = RFAL_BR_KEEP;                         /* The RFAL_BR_KEEP is used to check if PPS with BR change was requested */
  dsi = RFAL_BR_KEEP;                         /* MISRA 9.1 */
  bufIt = 0;

  /*******************************************************************************/
  gIsoDep.role = ISODEP_ROLE_PICC;

  /*******************************************************************************/
  if (gIsoDep.state == ISODEP_ST_PICC_ACT_ATS) {
    /* Check for a PPS    ISO 14443-4  5.3 */
    if ((((uint8_t *)gIsoDep.actvParam.rxBuf)[RFAL_ISODEP_PPS_STARTBYTE_POS] & RFAL_ISODEP_PPS_MASK) == RFAL_ISODEP_PPS_SB) {
      /* ISO 14443-4  5.3.1  Check if the we are the addressed DID/CID */
      /* ISO 14443-4  5.3.2  Check for a valid PPS0 */
      if (((((uint8_t *)gIsoDep.actvParam.rxBuf)[RFAL_ISODEP_PPS_STARTBYTE_POS] & RFAL_ISODEP_DID_MASK) != gIsoDep.did) ||
          ((((uint8_t *)gIsoDep.actvParam.rxBuf)[RFAL_ISODEP_PPS_PPS0_POS] & RFAL_ISODEP_PPS0_VALID_MASK) != RFAL_ISODEP_PPS0_PPS1_NOT_PRESENT)) {
        /* Invalid DID on PPS request or Invalid PPS0, re-enable the receiver and wait another frame */
        isoDepReEnableRx((uint8_t *)gIsoDep.actvParam.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.actvParam.rxLen);

        return NFC_Busy;
      }

      /*******************************************************************************/
      /* Check PPS1 presence */
      if (((uint8_t *)gIsoDep.actvParam.rxBuf)[RFAL_ISODEP_PPS_PPS0_POS] == RFAL_ISODEP_PPS0_PPS1_PRESENT) {
        uint8_t newdri = ((uint8_t *)gIsoDep.actvParam.rxBuf)[RFAL_ISODEP_PPS_PPS1_POS] & RFAL_ISODEP_PPS1_DxI_MASK;                                 /* MISRA 10.8 */
        uint8_t newdsi = (((uint8_t *)gIsoDep.actvParam.rxBuf)[RFAL_ISODEP_PPS_PPS1_POS] >> RFAL_ISODEP_PPS1_DSI_SHIFT) & RFAL_ISODEP_PPS1_DxI_MASK; /* MISRA 10.8 */
        /* PRQA S 4342 2 # MISRA 10.5 - Layout of enum RFal_BitRate and above masks guarantee no invalid enum values to be created */
        dri = (RFal_BitRate)(newdri);
        dsi = (RFal_BitRate)(newdsi);

        if (((dsi == RFAL_BR_106) || (dri == RFAL_BR_106)) ||
            ((dsi == RFAL_BR_212) || (dri == RFAL_BR_212)) ||
            ((dsi == RFAL_BR_424) || (dri == RFAL_BR_424)) ||
            ((dsi == RFAL_BR_848) || (dri == RFAL_BR_848))) {
          return NFC_ProtocolError;
        }
      }

      /*******************************************************************************/
      /* Compute and send PPS RES / Ack                                              */
      txBuf[bufIt++] = ((uint8_t *)gIsoDep.actvParam.rxBuf)[RFAL_ISODEP_PPS_STARTBYTE_POS];

      RFal_TransceiveBlockingTx(txBuf, bufIt, (uint8_t *)gIsoDep.actvParam.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.actvParam.rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FWT_NONE);

      /*******************************************************************************/
      /* Exchange the bit rates if requested */
      if (dri != RFAL_BR_KEEP) {
        RFal_SetBitRate(dsi, dri);

        gIsoDep.txBR = dsi; /* DSI codes the divisor from PICC to PCD */
        gIsoDep.rxBR = dri; /* DRI codes the divisor from PCD to PICC */

        if (gIsoDep.actvParam.isoDepDev != NULL) {
          gIsoDep.actvParam.isoDepDev->info.DSI = dsi;
          gIsoDep.actvParam.isoDepDev->info.DRI = dri;
        }
      }
    }
    /* Check for a S-Deselect is done on Data Exchange Activity                    */
  }

  /*******************************************************************************/
  gIsoDep.hdrLen = RFAL_ISODEP_PCB_LEN;
  gIsoDep.hdrLen += RFAL_ISODEP_DID_LEN; /* Always assume DID to be aligned with Digital 1.1  15.1.2 and ISO14443  4 5.6.3    #454 */
  gIsoDep.hdrLen += (uint8_t)((gIsoDep.nad != RFAL_ISODEP_NO_NAD) ? RFAL_ISODEP_NAD_LEN : 0U);

  /*******************************************************************************/
  /* Rule C - The PICC block number shall be initialized to 1 at activation */
  gIsoDep.blockNumber = 1;

  /* Activation done, keep the rcvd data in, reMap the activation buffer to the global to be retrieved by the DEP method */
  gIsoDep.rxBuf = (uint8_t *)gIsoDep.actvParam.rxBuf;
  gIsoDep.rxBufLen = sizeof(RFal_ISO_Dep_BufFormat);
  gIsoDep.rxBufInfPos = (uint8_t)((uint32_t)gIsoDep.actvParam.rxBuf->inf - (uint32_t)gIsoDep.actvParam.rxBuf->prologue);
  gIsoDep.rxLen = gIsoDep.actvParam.rxLen;
  gIsoDep.rxChaining = gIsoDep.actvParam.isRxChaining;

  gIsoDep.state = ISODEP_ST_PICC_RX;
  return NFC_OK;
}

/*******************************************************************************/
uint16_t RFal_ISO_Dep_GetMaxInfLen(void)
{
  /* Check whether all parameters are valid, otherwise return minimum default value */
  if ((gIsoDep.fsx < (uint16_t)RFAL_ISODEP_FSX_16) || (gIsoDep.fsx > (uint16_t)RFAL_ISODEP_FSX_4096) || (gIsoDep.hdrLen > ISODEP_HDR_MAX_LEN)) {
    const uint16_t isodepFsx16 = (uint16_t)RFAL_ISODEP_FSX_16; /* MISRA 10.1 */
    return (isodepFsx16 - RFAL_ISODEP_PCB_LEN - ISODEP_CRC_LEN);
  }

  return (gIsoDep.fsx - gIsoDep.hdrLen - ISODEP_CRC_LEN);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_StartTransceive(RFal_ISO_Dep_TxRxParam param)
{
  gIsoDep.txBuf = param.txBuf->prologue;
  gIsoDep.txBufInfPos = (uint8_t)((uintptr_t)param.txBuf->inf - (uintptr_t)param.txBuf->prologue);
  gIsoDep.txBufLen = param.txBufLen;
  gIsoDep.isTxChaining = param.isTxChaining;

  gIsoDep.rxBuf = param.rxBuf->prologue;
  gIsoDep.rxBufInfPos = (uint8_t)((uintptr_t)param.rxBuf->inf - (uintptr_t)param.rxBuf->prologue);
  gIsoDep.rxBufLen = sizeof(RFal_ISO_Dep_BufFormat);

  gIsoDep.rxLen = param.rxLen;
  gIsoDep.rxChaining = param.isRxChaining;

  gIsoDep.fwt = param.FWT;
  gIsoDep.dFwt = param.dFWT;
  gIsoDep.fsx = param.FSx;
  gIsoDep.did = param.DID;

  /* Only change the FSx from activation if no to Keep */
  gIsoDep.ourFsx = ((param.ourFSx != RFAL_ISODEP_FSX_KEEP) ? param.ourFSx : gIsoDep.ourFsx);

  /* Clear inner control params for next dataExchange */
  gIsoDep.isRxChaining = false;
  isoDepClearCounters();

  if (gIsoDep.role == ISODEP_ROLE_PICC) {
    if (gIsoDep.txBufLen > 0U) {
      /* Ensure that an RTOX Ack is not being expected at moment */
      if (!gIsoDep.isWait4WTX) {
        gIsoDep.state = ISODEP_ST_PICC_TX;
        return NFC_OK;
      } else {
        /* If RTOX Ack is expected, signal a pending Tx to be transmitted right after */
        gIsoDep.isTxPending = true;
      }
    }

    /* Digital 1.1  15.2.5.1 The first block SHALL be sent by the Reader/Writer */
    gIsoDep.state = ISODEP_ST_PICC_RX;
    return NFC_OK;
  }

  gIsoDep.state = ISODEP_ST_PCD_TX;
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_GetTransceiveStatus(void)
{
  if (gIsoDep.role == ISODEP_ROLE_PICC) {
#if RFAL_FEATURE_ISO_DEP_LISTEN
    return RFal_ISO_Dep_DataExchangePICC();
#else
    return NFC_Unsupport;
#endif /* RFAL_FEATURE_ISO_DEP_LISTEN */
  } else {
#if RFAL_FEATURE_ISO_DEP_POLL
    return isoDepDataExchangePCD(gIsoDep.rxLen, gIsoDep.rxChaining);
#else
    return NFC_Unsupport;
#endif /* RFAL_FEATURE_ISO_DEP_POLL */
  }
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_DataExchangePICC(void)
{
  uint8_t rxPCB;
  NFC_OpResult ret;

  switch (gIsoDep.state) {
    /*******************************************************************************/
    case ISODEP_ST_IDLE:
      return NFC_OK;

    /*******************************************************************************/
    case ISODEP_ST_PICC_TX:

      ret = isoDepTx(isoDep_PCBIBlock(gIsoDep.blockNumber), gIsoDep.txBuf, &gIsoDep.txBuf[gIsoDep.txBufInfPos], gIsoDep.txBufLen, RFAL_FWT_NONE);

      /* Clear pending Tx flag */
      gIsoDep.isTxPending = false;

      switch (ret) {
        case NFC_OK:
          gIsoDep.state = ISODEP_ST_PICC_RX;
          return NFC_Busy;

        default:
          /* MISRA 16.4: no empty default statement (a comment being enough) */
          break;
      }
      return ret;

    /*******************************************************************************/
    case ISODEP_ST_PICC_RX:

      ret = RFal_GetTransceiveStatus();
      switch (ret) {
        /*******************************************************************************/
        /* Data rcvd with error or timeout -> mute */
        case NFC_SlaveTimeout:
        case NFC_CRC_Error:
        case NFC_ParityError:
        case NFC_FramingError:

          /* Digital 1.1 - 15.2.6.2  The CE SHALL NOT attempt error recovery and remains in Rx mode upon Transmission or a Protocol Error */
          isoDepReEnableRx((uint8_t *)gIsoDep.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.rxLen);

          return NFC_Busy;

        /*******************************************************************************/
        case NFC_LinkLoss:
          return ret; /* Debug purposes */

        case NFC_Busy:
          return ret; /* Debug purposes */

        /*******************************************************************************/
        case NFC_OK:
          *gIsoDep.rxLen = rfalConvBitsToBytes(*gIsoDep.rxLen);
          break;

        /*******************************************************************************/
        default:
          return ret;
      }
      break;

    /*******************************************************************************/
    case ISODEP_ST_PICC_SWTX:

      if (!isoDepTimerisExpired(gIsoDep.WTXTimer)) { /* Do nothing until WTX timer has expired */
        return NFC_Busy;
      }

      /* Set waiting for WTX Ack Flag */
      gIsoDep.isWait4WTX = true;

      /* Digital 1.1  15.2.2.9 - Calculate the WTXM such that FWTtemp <= FWTmax */
      gIsoDep.lastWTXM = (uint8_t)isoDep_WTXMListenerMax(gIsoDep.fwt);
      ret = isoDepHandleControlMsg(ISODEP_S_WTX, gIsoDep.lastWTXM);
      if(ret < NFC_OK)
      {
          return ret;
      }

      gIsoDep.state = ISODEP_ST_PICC_RX; /* Go back to Rx to process WTX ack        */
      return NFC_Busy;

    /*******************************************************************************/
    case ISODEP_ST_PICC_SDSL:

      if (!(RFal_IsTransceiveInTx())) { /* Wait until DSL response has been sent */
        RFal_ISO_Dep_Initialize(); /* Session finished reInit vars, go back to ISODEP_ST_IDLE */
        return NFC_SleepRequest;   /* Notify Deselect request      */
      }
      return NFC_Busy;

    /*******************************************************************************/
    default:
      return NFC_InternalError;
  }

  /* ISO 14443-4 7.5.6.2 CE SHALL NOT attempt error recovery -> clear counters */
  isoDepClearCounters();

  /*******************************************************************************/
  /* No error, process incoming msg                                              */
  /*******************************************************************************/

  /* Grab rcvd PCB */
  rxPCB = gIsoDep.rxBuf[ISODEP_PCB_POS];

  /*******************************************************************************/
  /* When DID=0 PCD may or may not use DID, therefore check whether current PCD request
   * has DID present to be reflected on max INF length                         #454  */

  /* ReCalculate Header Length */
  gIsoDep.hdrLen = RFAL_ISODEP_PCB_LEN;
  gIsoDep.hdrLen += (uint8_t)((isoDep_PCBhasDID(rxPCB)) ? RFAL_ISODEP_DID_LEN : 0U);
  gIsoDep.hdrLen += (uint8_t)((isoDep_PCBhasNAD(rxPCB)) ? RFAL_ISODEP_NAD_LEN : 0U);

  /* Store whether last PCD block had DID. for PICC special handling of DID = 0 */
  if (gIsoDep.did == RFAL_ISODEP_DID_00) {
    gIsoDep.lastDID00 = ((isoDep_PCBhasDID(rxPCB)) ? true : false);
  }

  /*******************************************************************************/
  /* Check rcvd msg length, cannot be less then the expected header    OR        *
   * if the rcvd msg exceeds our announced frame size (FSD)                      */
  if (((*gIsoDep.rxLen) < gIsoDep.hdrLen) || ((*gIsoDep.rxLen) > (gIsoDep.ourFsx - ISODEP_CRC_LEN))) {
    isoDepReEnableRx((uint8_t *)gIsoDep.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.rxLen);
    return NFC_Busy; /* NFC_ProtocolError Ignore this protocol request */
  }
  /* If we are not supporting DID, but we receive one OR
   * If we are expecting DID, check if PCB signals its presence and if device ID match OR
   * If our DID=0 and DID is sent but with an incorrect value                              */
  if (((gIsoDep.did == RFAL_ISODEP_NO_DID) && (isoDep_PCBhasDID(rxPCB))) ||
      ((gIsoDep.did != RFAL_ISODEP_NO_DID) && (gIsoDep.did != RFAL_ISODEP_DID_00) && ((!isoDep_PCBhasDID(rxPCB)) || (gIsoDep.did != gIsoDep.rxBuf[ISODEP_DID_POS]))) ||
      ((gIsoDep.did == RFAL_ISODEP_DID_00) && isoDep_PCBhasDID(rxPCB) && (RFAL_ISODEP_DID_00 != gIsoDep.rxBuf[ISODEP_DID_POS]))) {
    isoDepReEnableRx((uint8_t *)gIsoDep.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.rxLen);
    return NFC_Busy; /* Ignore a wrong DID request */
  }

  /* If we aren't expecting NAD and it's received */
  if ((gIsoDep.nad == RFAL_ISODEP_NO_NAD) && isoDep_PCBhasNAD(rxPCB)) {
    isoDepReEnableRx((uint8_t *)gIsoDep.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.rxLen);
    return NFC_Busy; /* Ignore a unexpected NAD request */
  }

  /*******************************************************************************/
  /* Process S-Block                                                             */
  /*******************************************************************************/
  if (isoDep_PCBisSBlock(rxPCB)) {
    /* Check if is a Wait Time eXtension */
    if (isoDep_PCBisSWTX(rxPCB)) {
      /* Check if we're expecting a S-WTX */
      if (isoDep_PCBisWTX(gIsoDep.lastPCB)) {
        /* Digital 1.1  15.2.2.11 S(WTX) Ack with different WTXM -> Protocol Error  *
         *              Power level indication also should be set to 0              */
        if ((gIsoDep.rxBuf[gIsoDep.hdrLen] == gIsoDep.lastWTXM) && ((*gIsoDep.rxLen - gIsoDep.hdrLen) == ISODEP_SWTX_INF_LEN)) {
          /* Clear waiting for RTOX Ack Flag */
          gIsoDep.isWait4WTX = false;

          /* Check if a Tx is already pending */
          if (gIsoDep.isTxPending) {
            /* Has a pending Tx, go immediately to TX */
            gIsoDep.state = ISODEP_ST_PICC_TX;
            return NFC_Busy;
          }

          /* Set WTX timer */
          isoDepTimerStart(gIsoDep.WTXTimer, isoDep_WTXAdjust((gIsoDep.lastWTXM * rfalConv1fcToMs(gIsoDep.fwt))));

          gIsoDep.state = ISODEP_ST_PICC_SWTX;
          return NFC_Busy;
        }
      }
      /* Unexpected/Incorrect S-WTX, fall into reRenable */
    }

    /* Check if is a Deselect request */
    if (isoDep_PCBisSDeselect(rxPCB) && ((*gIsoDep.rxLen - gIsoDep.hdrLen) == ISODEP_SDSL_INF_LEN)) {
      ret = isoDepHandleControlMsg(ISODEP_S_DSL, RFAL_ISODEP_NO_PARAM);
      if(ret < NFC_OK)
      {
          return ret;
      }

      /* S-DSL transmission ongoing, wait until complete */
      gIsoDep.state = ISODEP_ST_PICC_SDSL;
      return NFC_Busy;
    }

    /* Unexpected S-Block, fall into reRenable */
  }

  /*******************************************************************************/
  /* Process R-Block                                                             */
  /*******************************************************************************/
  else if (isoDep_PCBisRBlock(rxPCB) && ((*gIsoDep.rxLen - gIsoDep.hdrLen) == ISODEP_RBLOCK_INF_LEN)) {
    if (isoDep_PCBisRACK(rxPCB)) { /* Check if is a R-ACK */
      if (isoDep_GetBN(rxPCB) == gIsoDep.blockNumber) { /* Check block number  */
        /* Rule 11 - R(ACK) with current bn -> re-transmit */
        if (!isoDep_PCBisIBlock(gIsoDep.lastPCB)) {
          RFal_ISO_Dep_ReSendControlMsg();
        } else {
          gIsoDep.state = ISODEP_ST_PICC_TX;
        }

        return NFC_Busy;
      } else {
        if (!gIsoDep.isTxChaining) {
          /* Rule 13 violation R(ACK) without performing chaining */
          isoDepReEnableRx((uint8_t *)gIsoDep.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.rxLen);
          return NFC_Busy;
        }

        /* Rule E -  R(ACK) with not current bn -> toggle bn */
        isoDep_ToggleBN(gIsoDep.blockNumber);

        /* This block has been transmitted and acknowledged, perform WTX until next data is provided  */

        /* Rule 9 - PICC is allowed to send an S(WTX) instead of an I-block or an R(ACK) */
        isoDepTimerStart(gIsoDep.WTXTimer, isoDep_WTXAdjust(rfalConv1fcToMs(gIsoDep.fwt)));
        gIsoDep.state = ISODEP_ST_PICC_SWTX;

        /* Rule 13 - R(ACK) with not current bn -> continue chaining */
        return NFC_OK; /* This block has been transmitted */
      }
    } else if (isoDep_PCBisRNAK(rxPCB)) { /* Check if is a R-NACK */
      if (isoDep_GetBN(rxPCB) == gIsoDep.blockNumber) { /* Check block number  */
        /* Rule 11 - R(NAK) with current bn -> re-transmit last x-Block */
        if (!isoDep_PCBisIBlock(gIsoDep.lastPCB)) {
          RFal_ISO_Dep_ReSendControlMsg();
        } else {
          gIsoDep.state = ISODEP_ST_PICC_TX;
        }

        return NFC_Busy;
      } else {
        /* Rule 12 - R(NAK) with not current bn -> R(ACK) */
        ret = isoDepHandleControlMsg(ISODEP_R_ACK, RFAL_ISODEP_NO_PARAM);
        if(ret < NFC_OK)
        {
            return ret;
        }

        return NFC_Busy;
      }
    } else {
      /* MISRA 15.7 - Empty else */
    }

    /* Unexpected R-Block, fall into reRenable */
  }

  /*******************************************************************************/
  /* Process I-Block                                                             */
  /*******************************************************************************/
  else if (isoDep_PCBisIBlock(rxPCB)) {
    /* Rule D - When an I-block is received, the PICC shall toggle its block number before sending a block */
    isoDep_ToggleBN(gIsoDep.blockNumber);

    /*******************************************************************************/
    /* Check if the block number is the one expected                               */
    /* Check if PCD sent an I-Block instead ACK/NACK when we are chaining          */
    if ((isoDep_GetBN(rxPCB) != gIsoDep.blockNumber) || (gIsoDep.isTxChaining)) {
      /* Remain in the same Block Number */
      isoDep_ToggleBN(gIsoDep.blockNumber);

      /* ISO 14443-4 7.5.6.2 & Digital 1.1 - 15.2.6.2  The CE SHALL NOT attempt error recovery and remains in Rx mode upon Transmission or a Protocol Error */
      isoDepReEnableRx((uint8_t *)gIsoDep.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.rxLen);
      return NFC_Busy;
    }

    /*******************************************************************************/
    /* is PCD performing chaining  ?                                               */
    if (isoDep_PCBisChaining(rxPCB)) {
      gIsoDep.isRxChaining = true;
      *gIsoDep.rxChaining = true; /* Output Parameter*/

      ret = isoDepHandleControlMsg(ISODEP_R_ACK, RFAL_ISODEP_NO_PARAM);
      if(ret < NFC_OK)
      {
          return ret;
      }

      /* Received I-Block with chaining, send current data to DH */

      /* remove ISO DEP header, check is necessary to move the INF data on the buffer */
      *gIsoDep.rxLen -= gIsoDep.hdrLen;
      if ((gIsoDep.hdrLen != gIsoDep.rxBufInfPos) && (*gIsoDep.rxLen > 0U)) {
        ST_MEMMOVE(&gIsoDep.rxBuf[gIsoDep.rxBufInfPos], &gIsoDep.rxBuf[gIsoDep.hdrLen], *gIsoDep.rxLen);
      }
      return NFC_Again; /* Send Again signalling to run again, but some chaining data has arrived*/
    }

    /*******************************************************************************/
    /* PCD is not performing chaining                                              */
    gIsoDep.isRxChaining = false; /* clear PCD chaining flag */
    *gIsoDep.rxChaining = false;  /* Output Parameter        */

    /* remove ISO DEP header, check is necessary to move the INF data on the buffer */
    *gIsoDep.rxLen -= gIsoDep.hdrLen;
    if ((gIsoDep.hdrLen != gIsoDep.rxBufInfPos) && (*gIsoDep.rxLen > 0U)) {
      ST_MEMMOVE(&gIsoDep.rxBuf[gIsoDep.rxBufInfPos], &gIsoDep.rxBuf[gIsoDep.hdrLen], *gIsoDep.rxLen);
    }

    /*******************************************************************************/
    /* Reception done, send data back and start WTX timer                          */
    isoDepTimerStart(gIsoDep.WTXTimer, isoDep_WTXAdjust(rfalConv1fcToMs(gIsoDep.fwt)));

    gIsoDep.state = ISODEP_ST_PICC_SWTX;
    return NFC_OK;
  } else {
    /* MISRA 15.7 - Empty else */
  }

  /* Unexpected/Unknown Block */
  /* ISO 14443-4 7.5.6.2 & Digital 1.1 - 15.2.6.2  The CE SHALL NOT attempt error recovery and remains in Rx mode upon Transmission or a Protocol Error */
  isoDepReEnableRx((uint8_t *)gIsoDep.rxBuf, sizeof(RFal_ISO_Dep_BufFormat), gIsoDep.rxLen);

  return NFC_Busy;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_StartRATS(RFal_ISO_Dep_FSxI FSDI, uint8_t DID, RFal_ISO_Dep_Ats *ats, uint8_t *atsLen)
{
  RFal_TransceiveContext ctx;

  if (ats == NULL) {
    return NFC_InvalidParameter;
  }

  gIsoDep.rxBuf = (uint8_t *)ats;
  gIsoDep.rxLen8 = atsLen;
  gIsoDep.did = DID;

  /*******************************************************************************/
  /* Compose RATS */
  gIsoDep.actv.ratsReq.CMD = RFAL_ISODEP_CMD_RATS;
  gIsoDep.actv.ratsReq.PARAM = (((uint8_t)FSDI << RFAL_ISODEP_RATS_PARAM_FSDI_SHIFT) & RFAL_ISODEP_RATS_PARAM_FSDI_MASK) | (DID & RFAL_ISODEP_RATS_PARAM_DID_MASK);

  rfalCreateByteFlagsTxRxContext(ctx, (uint8_t *)&gIsoDep.actv.ratsReq, sizeof(RFal_ISO_Dep_Rats), (uint8_t *)ats, sizeof(RFal_ISO_Dep_Ats), &gIsoDep.rxBufLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ISODEP_T4T_FWT_ACTIVATION);
  return RFal_StartTransceive(&ctx);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_GetRATSStatus(void)
{
  NFC_OpResult ret;

  ret = RFal_GetTransceiveStatus();
  if (ret == NFC_OK) {
    gIsoDep.rxBufLen = rfalConvBitsToBytes(gIsoDep.rxBufLen);

    /* Check for valid ATS length  Digital 1.1  13.6.2.1 & 13.6.2.3 */
    if ((gIsoDep.rxBufLen < RFAL_ISODEP_ATS_MIN_LEN) || (gIsoDep.rxBufLen > RFAL_ISODEP_ATS_MAX_LEN) || (gIsoDep.rxBuf[RFAL_ISODEP_ATS_TL_POS] != gIsoDep.rxBufLen)) {
      return NFC_ProtocolError;
    }

    /* Assign our FSx, in case the a Deselect is send without Transceive */
    gIsoDep.ourFsx = RFal_ISO_Dep_FSxI2FSx((uint8_t)(gIsoDep.actv.ratsReq.PARAM >> RFAL_ISODEP_RATS_PARAM_FSDI_SHIFT));

    /* Check and assign if ATS length was requested (length also available on TL) */
    if (gIsoDep.rxLen8 != NULL) {
      *gIsoDep.rxLen8 = (uint8_t)gIsoDep.rxBufLen;
    }
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_RATS(RFal_ISO_Dep_FSxI FSDI, uint8_t DID, RFal_ISO_Dep_Ats *ats, uint8_t *atsLen)
{
  NFC_OpResult ret;

  ret = RFal_ISO_Dep_StartRATS(FSDI, DID, ats, atsLen);
  if(ret < NFC_OK)
  {
      return ret;
  }

  rfalRunBlocking(ret, RFal_ISO_Dep_GetRATSStatus());

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_StartPPS(uint8_t DID, RFal_BitRate DSI, RFal_BitRate DRI, RFal_ISO_Dep_PpsRes *ppsRes)
{
  RFal_TransceiveContext ctx;

  if ((ppsRes == NULL) || (DSI > RFAL_BR_848) || (DRI > RFAL_BR_848) || ((DID > RFAL_ISODEP_DID_MAX) && (DID != RFAL_ISODEP_NO_DID))) {
    return NFC_InvalidParameter;
  }

  gIsoDep.rxBuf = (uint8_t *)ppsRes;

  /*******************************************************************************/
  /* Compose PPS Request */
  gIsoDep.actv.ppsReq.PPSS = (RFAL_ISODEP_PPS_SB | (DID & RFAL_ISODEP_PPS_SB_DID_MASK));
  gIsoDep.actv.ppsReq.PPS0 = RFAL_ISODEP_PPS_PPS0_PPS1_PRESENT;
  gIsoDep.actv.ppsReq.PPS1 = (RFAL_ISODEP_PPS_PPS1 | ((((uint8_t)DSI << RFAL_ISODEP_PPS_PPS1_DSI_SHIFT) | (uint8_t)DRI) & RFAL_ISODEP_PPS_PPS1_DXI_MASK));

  rfalCreateByteFlagsTxRxContext(ctx, (uint8_t *)&gIsoDep.actv.ppsReq, sizeof(RFal_ISO_Dep_PpsReq), (uint8_t *)ppsRes, sizeof(RFal_ISO_Dep_PpsRes), &gIsoDep.rxBufLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ISODEP_T4T_FWT_ACTIVATION);
  return RFal_StartTransceive(&ctx);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_GetPPSSTatus(void)
{
  NFC_OpResult ret;

  ret = RFal_GetTransceiveStatus();
  if (ret == NFC_OK) {
    gIsoDep.rxBufLen = rfalConvBitsToBytes(gIsoDep.rxBufLen);

    /* Check for valid PPS Response   */
    if ((gIsoDep.rxBufLen != RFAL_ISODEP_PPS_RES_LEN) || (*gIsoDep.rxBuf != gIsoDep.actv.ppsReq.PPSS)) {
      return NFC_ProtocolError;
    }
  }
  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_PPS(uint8_t DID, RFal_BitRate DSI, RFal_BitRate DRI, RFal_ISO_Dep_PpsRes *ppsRes)
{
  NFC_OpResult ret;

  ret = RFal_ISO_Dep_StartPPS(DID, DSI, DRI, ppsRes);
  if(ret < NFC_OK)
  {
      return ret;
  }

  rfalRunBlocking(ret, RFal_ISO_Dep_GetPPSSTatus());

  return ret;
}

NFC_OpResult RFal_ISO_Dep_StartATTRIB(const uint8_t *nfcid0, uint8_t PARAM1, RFal_BitRate DSI, RFal_BitRate DRI, RFal_ISO_Dep_FSxI FSDI, uint8_t PARAM3, uint8_t DID, const uint8_t *HLInfo, uint8_t HLInfoLen, uint32_t fwt, RFal_ISO_Dep_AttribRes *attribRes, uint8_t *attribResLen)
{
  RFal_TransceiveContext ctx;

  if ((attribRes == NULL) || (attribResLen == NULL) || (DSI > RFAL_BR_848) || (DRI > RFAL_BR_848) || ((DID > RFAL_ISODEP_DID_MAX) && (DID != RFAL_ISODEP_NO_DID))) {
    return NFC_OK;
  }

  gIsoDep.rxBuf = (uint8_t *)attribRes;
  gIsoDep.rxLen8 = attribResLen;
  gIsoDep.did = DID;

  /*******************************************************************************/
  /* Compose ATTRIB command */
  gIsoDep.actv.attribReq.cmd = RFAL_ISODEP_CMD_ATTRIB;
  gIsoDep.actv.attribReq.Param.PARAM1 = PARAM1;
  gIsoDep.actv.attribReq.Param.PARAM2 = (((((uint8_t)DSI << RFAL_ISODEP_ATTRIB_PARAM2_DSI_SHIFT) | ((uint8_t)DRI << RFAL_ISODEP_ATTRIB_PARAM2_DRI_SHIFT)) & RFAL_ISODEP_ATTRIB_PARAM2_DXI_MASK) | ((uint8_t)FSDI & RFAL_ISODEP_ATTRIB_PARAM2_FSDI_MASK));
  gIsoDep.actv.attribReq.Param.PARAM3 = PARAM3;
  gIsoDep.actv.attribReq.Param.PARAM4 = (DID & RFAL_ISODEP_ATTRIB_PARAM4_DID_MASK);
  memcpy(gIsoDep.actv.attribReq.nfcid0, nfcid0, RFAL_NFCB_NFCID0_LEN);

  /* Append the Higher layer Info if provided */
  if ((HLInfo != NULL) && (HLInfoLen > 0U)) {
    memcpy(gIsoDep.actv.attribReq.HLInfo, HLInfo, MIN(HLInfoLen, RFAL_ISODEP_ATTRIB_HLINFO_LEN));
  }

  rfalCreateByteFlagsTxRxContext(ctx, (uint8_t *)&gIsoDep.actv.attribReq, (uint16_t)(RFAL_ISODEP_ATTRIB_HDR_LEN + MIN((uint16_t)HLInfoLen, RFAL_ISODEP_ATTRIB_HLINFO_LEN)), (uint8_t *)gIsoDep.rxBuf, sizeof(RFal_ISO_Dep_AttribRes), &gIsoDep.rxBufLen, RFAL_TXRX_FLAGS_DEFAULT, fwt);
  return RFal_StartTransceive(&ctx);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_GetATTRIBStatus(void)
{
  NFC_OpResult ret;

  ret = RFal_GetTransceiveStatus();
  if (ret == NFC_OK) {
    gIsoDep.rxBufLen = rfalConvBitsToBytes(gIsoDep.rxBufLen);

    /* Check for valid ATTRIB Response Digital 2.3  15.6.2.1 */
    if ((gIsoDep.rxBufLen < RFAL_ISODEP_ATTRIB_RES_HDR_LEN)) {
      return NFC_ProtocolError;
    }

    if (gIsoDep.rxLen8 != NULL) {
      *gIsoDep.rxLen8 = (uint8_t)gIsoDep.rxBufLen;
    }

    gIsoDep.ourFsx = RFal_ISO_Dep_FSxI2FSx((uint8_t)(gIsoDep.actv.attribReq.Param.PARAM2 & RFAL_ISODEP_ATTRIB_PARAM2_FSDI_MASK));
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_ATTRIB(const uint8_t *nfcid0, uint8_t PARAM1, RFal_BitRate DSI, RFal_BitRate DRI, RFal_ISO_Dep_FSxI FSDI, uint8_t PARAM3, uint8_t DID, const uint8_t *HLInfo, uint8_t HLInfoLen, uint32_t fwt, RFal_ISO_Dep_AttribRes *attribRes, uint8_t *attribResLen)
{
  NFC_OpResult ret;

  ret = RFal_ISO_Dep_StartATTRIB(nfcid0, PARAM1, DSI, DRI, FSDI, PARAM3, DID, HLInfo, HLInfoLen, fwt, attribRes, attribResLen);
  if(ret < NFC_OK)
  {
      return ret;
  }

  rfalRunBlocking(ret, RFal_ISO_Dep_GetATTRIBStatus());

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_PollAHandleActivation(RFal_ISO_Dep_FSxI FSDI, uint8_t DID, RFal_BitRate maxBR, RFal_ISO_Dep_Device *isoDepDev)
{
  NFC_OpResult ret;

  ret = RFal_ISO_Dep_PollAStartActivation(FSDI, DID, maxBR, isoDepDev);
  if(ret < NFC_OK)
  {
      return ret;
  }

  rfalRunBlocking(ret, RFal_ISO_Dep_PollAGetActivationStatus());

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_PollAStartActivation(RFal_ISO_Dep_FSxI FSDI, uint8_t DID, RFal_BitRate maxBR, RFal_ISO_Dep_Device *isoDepDev)
{
  NFC_OpResult ret;

  if (isoDepDev == NULL) {
    return NFC_InvalidParameter;
  }

  /* Enable EMD suppresssion|handling according to  Digital 2.1  4.1.1.1 ; EMVCo 3.0  4.9.2 ; ISO 14443-3  8.3 */
  RFal_SetErrorHandling(ERRORHANDLING_EMD);

  /* Start RATS Transceive */
  ret = RFal_ISO_Dep_StartRATS(FSDI, DID, &isoDepDev->activation.A.Listener.ATS, &isoDepDev->activation.A.Listener.ATSLen);
  if(ret < NFC_OK)
  {
      return ret;
  }

  isoDepDev->info.DSI = maxBR;
  gIsoDep.actvDev = isoDepDev;
  gIsoDep.cntRRetrys = gIsoDep.maxRetriesRATS;
  gIsoDep.state = ISODEP_ST_PCD_ACT_RATS;

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_PollAGetActivationStatus(void)
{
  NFC_OpResult ret;
  uint8_t msgIt;
  RFal_BitRate maxBR;

  switch (gIsoDep.state) {
    /*******************************************************************************/
    case ISODEP_ST_PCD_ACT_RATS:

      ret = RFal_ISO_Dep_GetRATSStatus();
      if (ret != NFC_Busy) {
        if (ret != NFC_OK) {
          /* EMVCo 2.6  9.6.1.1 & 9.6.1.2  If a timeout error is detected retransmit, on transmission error abort */
          if ((gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) && (ret != NFC_SlaveTimeout)) {
            break;
          }

          if (gIsoDep.cntRRetrys != 0U) {
            /* Ensure FDT before retransmission (reuse RFAL GT timer) */
            RFal_SetGT(RFal_GetFDTPoll());
            RFal_FieldOnAndStartGT();

            /* Send RATS retransmission */ /* PRQA S 4342 1 # MISRA 10.5 - Layout of enum RFal_ISO_Dep_FSxI is guaranteed within 4bit range */
            ret = RFal_ISO_Dep_StartRATS((RFal_ISO_Dep_FSxI)(uint8_t)(gIsoDep.actv.ratsReq.PARAM >> RFAL_ISODEP_RATS_PARAM_FSDI_SHIFT),
                                                 gIsoDep.did,
                                                 &gIsoDep.actvDev->activation.A.Listener.ATS,
                                                 &gIsoDep.actvDev->activation.A.Listener.ATSLen);
            if(ret < NFC_OK)
            {
                return ret;
            }

            gIsoDep.cntRRetrys--;
            ret = NFC_Busy;
          }
          /* Switch between NFC Forum and ISO14443-4 behaviour #595
           *   ISO14443-4  5.6.1  If RATS fails, a Deactivation sequence should be performed as defined on clause 8  (ISO10373-6 Scenario H.2.5)
           *   Activity 1.1  9.6  Device Deactivation Activity is to be only performed when there's an active device */
          else if (gIsoDep.compMode == RFAL_COMPLIANCE_MODE_ISO) {
            ret = RFal_ISO_Dep_StartDeselect();
            if(ret < NFC_OK)
            {
                return ret;
            }

            /* State ISODEP_ST_PCD_WAIT_DSL already set by RFal_ISO_Dep_HandleControlMsg DSL */
            return NFC_Busy;
          } else {
            /* MISRA 15.7 - Empty else */
          }
        } else { /* ATS received */
          maxBR = gIsoDep.actvDev->info.DSI; /* Retrieve requested max bitrate */

          /*******************************************************************************/
          /* Process ATS Response                                                        */
          gIsoDep.actvDev->info.FWI = RFAL_ISODEP_FWI_DEFAULT; /* Default value   EMVCo 2.6  5.7.2.6  */
          gIsoDep.actvDev->info.SFGI = 0U;
          gIsoDep.actvDev->info.MBL = 0U;
          gIsoDep.actvDev->info.DSI = RFAL_BR_106;
          gIsoDep.actvDev->info.DRI = RFAL_BR_106;
          gIsoDep.actvDev->info.FSxI = (uint8_t)RFAL_ISODEP_FSXI_32; /* FSC default value is 32 bytes  ISO14443-A  5.2.3 */

          /*******************************************************************************/
          /* Check for ATS optional fields                                               */
          if (gIsoDep.actvDev->activation.A.Listener.ATS.TL > RFAL_ISODEP_ATS_MIN_LEN) {
            msgIt = RFAL_ISODEP_ATS_MIN_LEN;

            /* Format byte T0 is optional, if present assign FSDI */
            gIsoDep.actvDev->info.FSxI = (gIsoDep.actvDev->activation.A.Listener.ATS.T0 & RFAL_ISODEP_ATS_T0_FSCI_MASK);

            /* T0 has already been processed, always the same position */
            msgIt++;

            /* Check if TA is present */
            if ((gIsoDep.actvDev->activation.A.Listener.ATS.T0 & RFAL_ISODEP_ATS_T0_TA_PRESENCE_MASK) != 0U) {
              RFal_ISO_Dep_CalcBitRate(maxBR, ((uint8_t *)&gIsoDep.actvDev->activation.A.Listener.ATS)[msgIt++], &gIsoDep.actvDev->info.DSI, &gIsoDep.actvDev->info.DRI);
            }

            /* Check if TB is present */
            if ((gIsoDep.actvDev->activation.A.Listener.ATS.T0 & RFAL_ISODEP_ATS_T0_TB_PRESENCE_MASK) != 0U) {
              gIsoDep.actvDev->info.SFGI = ((uint8_t *)&gIsoDep.actvDev->activation.A.Listener.ATS)[msgIt++];
              gIsoDep.actvDev->info.FWI = (uint8_t)((gIsoDep.actvDev->info.SFGI >> RFAL_ISODEP_ATS_TB_FWI_SHIFT) & RFAL_ISODEP_ATS_FWI_MASK);
              gIsoDep.actvDev->info.SFGI &= RFAL_ISODEP_ATS_TB_SFGI_MASK;
            }

            /* Check if TC is present */
            if ((gIsoDep.actvDev->activation.A.Listener.ATS.T0 & RFAL_ISODEP_ATS_T0_TC_PRESENCE_MASK) != 0U) {
              /* Check for Protocol features support */
              /* Advanced protocol features defined on Digital 1.0 Table 69, removed after */
              gIsoDep.actvDev->info.supAdFt = (((((uint8_t *)&gIsoDep.actvDev->activation.A.Listener.ATS)[msgIt] & RFAL_ISODEP_ATS_TC_ADV_FEAT) != 0U) ? true : false);
              gIsoDep.actvDev->info.supDID = (((((uint8_t *)&gIsoDep.actvDev->activation.A.Listener.ATS)[msgIt] & RFAL_ISODEP_ATS_TC_DID) != 0U) ? true : false);
              gIsoDep.actvDev->info.supNAD = (((((uint8_t *)&gIsoDep.actvDev->activation.A.Listener.ATS)[msgIt++] & RFAL_ISODEP_ATS_TC_NAD) != 0U) ? true : false);
            }
          }

          gIsoDep.actvDev->info.FSx = RFal_ISO_Dep_FSxI2FSx(gIsoDep.actvDev->info.FSxI);
          gIsoDep.fsx = gIsoDep.actvDev->info.FSx;

          gIsoDep.actvDev->info.SFGT = RFal_ISO_Dep_SFGI2SFGT((uint8_t)gIsoDep.actvDev->info.SFGI);

          /* Ensure SFGT before following frame (reuse RFAL GT timer) */
          RFal_SetGT(rfalConvMsTo1fc(gIsoDep.actvDev->info.SFGT));
          RFal_FieldOnAndStartGT();

          gIsoDep.actvDev->info.FWT = RFal_ISO_Dep_FWI2FWT(gIsoDep.actvDev->info.FWI);
          gIsoDep.actvDev->info.dFWT = RFAL_ISODEP_DFWT_20;

          gIsoDep.actvDev->info.DID = ((gIsoDep.actvDev->info.supDID) ? gIsoDep.did : RFAL_ISODEP_NO_DID);
          gIsoDep.actvDev->info.NAD = RFAL_ISODEP_NO_NAD;

          /*******************************************************************************/
          /* If higher bit rates are supported by both devices, send PPS                 */
          if ((gIsoDep.actvDev->info.DSI != RFAL_BR_106) || (gIsoDep.actvDev->info.DRI != RFAL_BR_106)) {
            /* Send PPS */ /*  PRQA S 0310 1 # MISRA 11.3 - Intentional safe cast to avoiding buffer duplication */
            ret = RFal_ISO_Dep_StartPPS(gIsoDep.actvDev->info.DID, gIsoDep.actvDev->info.DSI, gIsoDep.actvDev->info.DRI, (RFal_ISO_Dep_PpsRes *)&gIsoDep.ctrlBuf);
            if(ret < NFC_OK)
            {
                return ret;
            }

            gIsoDep.state = ISODEP_ST_PCD_ACT_PPS;
            return NFC_Busy;
          }

          return NFC_OK;
        }
      }
      break;

    /*******************************************************************************/
    case ISODEP_ST_PCD_ACT_PPS:

      ret = RFal_ISO_Dep_GetPPSSTatus();
      if (ret != NFC_Busy) {
        /* Check whether PPS has been acknowledge */
        if (ret == NFC_OK) {
          /* DSI code the divisor from PICC to PCD */
          /* DRI code the divisor from PCD to PICC */
          RFal_SetBitRate(gIsoDep.actvDev->info.DRI, gIsoDep.actvDev->info.DSI);
        } else {
          /* If PPS has faled keep activation bit rate */
          gIsoDep.actvDev->info.DSI = RFAL_BR_106;
          gIsoDep.actvDev->info.DRI = RFAL_BR_106;

          /* Ignore PPS response fail, proceed to data exchange */
          ret = NFC_OK;
        }
      }
      break;

    /*******************************************************************************/
    case ISODEP_ST_PCD_WAIT_DSL:

      ret = RFal_ISO_Dep_GetDeselectStatus();
      if (ret != NFC_Busy) {
        /* Report activation failed with generic transmission error */
        ret = NFC_FramingError;
      }
      break;

    /*******************************************************************************/
    default:
      ret = NFC_WrongState;
      break;
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_PollBHandleActivation(RFal_ISO_Dep_FSxI FSDI, uint8_t DID, RFal_BitRate maxBR, uint8_t PARAM1, const RFal_NFCB_ListenDevice *nfcbDev, const uint8_t *HLInfo, uint8_t HLInfoLen, RFal_ISO_Dep_Device *isoDepDev)
{

  NFC_OpResult ret;

  ret = RFal_ISO_Dep_PollBStartActivation(FSDI, DID, maxBR, PARAM1, nfcbDev, HLInfo, HLInfoLen, isoDepDev);
  if(ret < NFC_OK)
  {
      return ret;
  }

  rfalRunBlocking(ret, RFal_ISO_Dep_PollBGetActivationStatus());

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_PollBStartActivation(RFal_ISO_Dep_FSxI FSDI, uint8_t DID, RFal_BitRate maxBR, uint8_t PARAM1, const RFal_NFCB_ListenDevice *nfcbDev, const uint8_t *HLInfo, uint8_t HLInfoLen, RFal_ISO_Dep_Device *isoDepDev)
{
  NFC_OpResult ret;
  uint32_t tr2;

  /***************************************************************************/
  /* Initialize ISO-DEP Device with info from SENSB_RES                      */
  isoDepDev->info.FWI = ((nfcbDev->sensbRes.protInfo.FwiAdcFo >> RFAL_NFCB_SENSB_RES_FWI_SHIFT) & RFAL_NFCB_SENSB_RES_FWI_MASK);
  isoDepDev->info.FWT = RFal_ISO_Dep_FWI2FWT(isoDepDev->info.FWI);
  isoDepDev->info.dFWT = RFAL_NFCB_DFWT;
  isoDepDev->info.SFGI = (((uint32_t)nfcbDev->sensbRes.protInfo.SFGI >> RFAL_NFCB_SENSB_RES_SFGI_SHIFT) & RFAL_NFCB_SENSB_RES_SFGI_MASK);
  isoDepDev->info.SFGT = RFal_ISO_Dep_SFGI2SFGT((uint8_t)isoDepDev->info.SFGI);
  isoDepDev->info.FSxI = ((nfcbDev->sensbRes.protInfo.FsciProType >> RFAL_NFCB_SENSB_RES_FSCI_SHIFT) & RFAL_NFCB_SENSB_RES_FSCI_MASK);
  isoDepDev->info.FSx = RFal_ISO_Dep_FSxI2FSx(isoDepDev->info.FSxI);
  isoDepDev->info.DID = DID;
  isoDepDev->info.supDID = (((nfcbDev->sensbRes.protInfo.FwiAdcFo & RFAL_NFCB_SENSB_RES_FO_DID_MASK) != 0U) ? true : false);
  isoDepDev->info.supNAD = (((nfcbDev->sensbRes.protInfo.FwiAdcFo & RFAL_NFCB_SENSB_RES_FO_NAD_MASK) != 0U) ? true : false);

  /* Check if DID requested is supported by PICC */
  if ((DID != RFAL_ISODEP_NO_DID) && (!isoDepDev->info.supDID)) {
    return NFC_InvalidParameter;
  }

  /* Enable EMD suppresssion|handling according to  Digital 2.1  4.1.1.1 ; EMVCo 3.0  4.9.2 ; ISO 14443-3  8.3 */
  RFal_SetErrorHandling(ERRORHANDLING_EMD);

  /***************************************************************************/
  /* Set FDT Poll to be used on upcoming communications                      */
  if (gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) {
    /* Disregard Minimum TR2 returned by PICC, always use FDTb MIN   EMVCo 3.0  6.3.2.10  */
    RFal_SetFDTPoll(RFAL_FDT_POLL_NFCB_POLLER);
  } else {
    tr2 = rfalNfcbTR2ToFDT(((nfcbDev->sensbRes.protInfo.FsciProType >> RFAL_NFCB_SENSB_RES_PROTO_TR2_SHIFT) & RFAL_NFCB_SENSB_RES_PROTO_TR2_MASK));
    if ((RFal_GetFDTPoll()) < tr2) {
      /* In case TR2 is longer than the one currently running, ensure it's fulfilled (max: 9472/fc => 700us)  */
      delay(1);
    }

    /* Apply minimum TR2 from SENSB_RES   Digital 2.1  7.6.2.23 */
    RFal_SetFDTPoll(tr2);
  }

  /* Calculate max Bit Rate */
  RFal_ISO_Dep_CalcBitRate(maxBR, nfcbDev->sensbRes.protInfo.BRC, &isoDepDev->info.DSI, &isoDepDev->info.DRI);

  /***************************************************************************/
  /* Send ATTRIB Command                                                     */
  ret = RFal_ISO_Dep_StartATTRIB((const uint8_t *)&nfcbDev->sensbRes.nfcid0,
                                         (((nfcbDev->sensbRes.protInfo.FwiAdcFo & RFAL_NFCB_SENSB_RES_ADC_ADV_FEATURE_MASK) != 0U) ? PARAM1 : RFAL_ISODEP_ATTRIB_REQ_PARAM1_DEFAULT),
                                         isoDepDev->info.DSI,
                                         isoDepDev->info.DRI,
                                         FSDI,
                                         (gIsoDep.compMode == RFAL_COMPLIANCE_MODE_EMV) ? RFAL_NFCB_SENSB_RES_PROTO_ISO_MASK : (nfcbDev->sensbRes.protInfo.FsciProType & ((RFAL_NFCB_SENSB_RES_PROTO_TR2_MASK << RFAL_NFCB_SENSB_RES_PROTO_TR2_SHIFT) | RFAL_NFCB_SENSB_RES_PROTO_ISO_MASK)), /* EMVCo 2.6 6.4.1.9 */
                                         DID,
                                         HLInfo,
                                         HLInfoLen,
                                         (isoDepDev->info.FWT + isoDepDev->info.dFWT),
                                         &isoDepDev->activation.B.Listener.ATTRIB_RES,
                                         &isoDepDev->activation.B.Listener.ATTRIB_RESLen);
  if(ret < NFC_OK)
  {
      return ret;
  }

  gIsoDep.actvDev = isoDepDev;
  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_PollBGetActivationStatus(void)
{
  NFC_OpResult ret;
  uint8_t mbli;

  /***************************************************************************/
  /* Process ATTRIB Response                                                 */
  ret = RFal_ISO_Dep_GetATTRIBStatus();
  if (ret != NFC_Busy) {
    if (ret == NFC_OK) {
      /* Digital 1.1 14.6.2.3 - Check if received DID match */
      uint8_t expected_did = ((RFAL_ISODEP_NO_DID == gIsoDep.did) ? RFAL_ISODEP_DID_00 : gIsoDep.did);
      if ((gIsoDep.actvDev->activation.B.Listener.ATTRIB_RES.mbliDid & RFAL_ISODEP_ATTRIB_RES_DID_MASK) != expected_did) {
        return NFC_ProtocolError;
      }

      /* Retrieve MBLI and calculate new FDS/MBL (Maximum Buffer Length) */
      mbli = ((gIsoDep.actvDev->activation.B.Listener.ATTRIB_RES.mbliDid >> RFAL_ISODEP_ATTRIB_RES_MBLI_SHIFT) & RFAL_ISODEP_ATTRIB_RES_MBLI_MASK);
      if (mbli > 0U) {
        /* Digital 1.1  14.6.2  Calculate Maximum Buffer Length MBL = FSC x 2^(MBLI-1) */
        gIsoDep.actvDev->info.MBL = (gIsoDep.actvDev->info.FSx * ((uint32_t)1U << (mbli - 1U)));
      }

      /* DSI code the divisor from PICC to PCD */
      /* DRI code the divisor from PCD to PICC */
      RFal_SetBitRate(gIsoDep.actvDev->info.DRI, gIsoDep.actvDev->info.DSI);

      /* REMARK: SoF EoF TR0 and TR1 are not passed on to RF layer */

      /* Start the SFGT timer (reuse RFAL GT timer) */
      RFal_SetGT(rfalConvMsTo1fc(gIsoDep.actvDev->info.SFGT));
      RFal_FieldOnAndStartGT();
    } else {
      gIsoDep.actvDev->info.DSI = RFAL_BR_106;
      gIsoDep.actvDev->info.DRI = RFAL_BR_106;
    }

    /*******************************************************************************/
    /* Store already FS info,  RFal_ISO_Dep_GetMaxInfLen() may be called before setting TxRx params */
    gIsoDep.fsx = gIsoDep.actvDev->info.FSx;
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_PollHandleSParameters(RFal_ISO_Dep_Device *isoDepDev, RFal_BitRate maxTxBR, RFal_BitRate maxRxBR)
{
  uint8_t it;
  uint8_t supPCD2PICC;
  uint8_t supPICC2PCD;
  uint8_t currenttxBR;
  uint8_t currentrxBR;
  RFal_BitRate txBR;
  RFal_BitRate rxBR;
  uint16_t rcvLen;
  NFC_OpResult ret;
  RFal_ISO_Dep_ControlMsgSParam sParam;

  if ((isoDepDev == NULL) || (maxTxBR > RFAL_BR_13560) || (maxRxBR > RFAL_BR_13560)) {
    return NFC_InvalidParameter;
  }

  it = 0;
  supPICC2PCD = 0x00;
  supPCD2PICC = 0x00;
  txBR = RFAL_BR_106;
  rxBR = RFAL_BR_106;
  sParam.pcb = ISODEP_PCB_SPARAMETERS;

  /*******************************************************************************/
  /* Send S(PARAMETERS) - Block Info */
  sParam.sParam.tag = RFAL_ISODEP_SPARAM_TAG_BLOCKINFO;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_BRREQ;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_BRREQ_LEN;
  sParam.sParam.length = it;

  /* Send S(PARAMETERS). Use a fixed FWI of 4   ISO14443-4 2016  7.2 */
  ret = RFal_TransceiveBlockingTxRx((uint8_t *)&sParam, (RFAL_ISODEP_SPARAM_HDR_LEN + (uint16_t)it), (uint8_t *)&sParam, sizeof(RFal_ISO_Dep_ControlMsgSParam), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, ISODEP_FWT_DEACTIVATION);
  if(ret < NFC_OK)
  {
      return ret;
  }

  it = 0;

  /*******************************************************************************/
  /* Check S(PARAMETERS) response */
  if ((sParam.pcb != ISODEP_PCB_SPARAMETERS) || (sParam.sParam.tag != RFAL_ISODEP_SPARAM_TAG_BLOCKINFO) ||
      (sParam.sParam.value[it] != RFAL_ISODEP_SPARAM_TAG_BRIND) || (rcvLen < RFAL_ISODEP_SPARAM_HDR_LEN) ||
      (rcvLen != ((uint16_t)sParam.sParam.length + RFAL_ISODEP_SPARAM_HDR_LEN))) {
    return NFC_ProtocolError;
  }

  /* Retrieve PICC's bit rate PICC capabilities */
  for (it = 0; it < (rcvLen - (uint16_t)RFAL_ISODEP_SPARAM_TAG_LEN); it++) {
    if ((sParam.sParam.value[it] == RFAL_ISODEP_SPARAM_TAG_SUP_PCD2PICC) && (sParam.sParam.value[it + (uint16_t)RFAL_ISODEP_SPARAM_TAG_LEN] == RFAL_ISODEP_SPARAM_TAG_PCD2PICC_LEN)) {
      supPCD2PICC = sParam.sParam.value[it + RFAL_ISODEP_SPARAM_TAG_PCD2PICC_LEN];
    }

    if ((sParam.sParam.value[it] == RFAL_ISODEP_SPARAM_TAG_SUP_PICC2PCD) && (sParam.sParam.value[it + (uint16_t)RFAL_ISODEP_SPARAM_TAG_LEN] == RFAL_ISODEP_SPARAM_TAG_PICC2PCD_LEN)) {
      supPICC2PCD = sParam.sParam.value[it + RFAL_ISODEP_SPARAM_TAG_PICC2PCD_LEN];
    }
  }

  /*******************************************************************************/
  /* Check if requested bit rates are supported by PICC */
  if ((supPICC2PCD == 0x00U) || (supPCD2PICC == 0x00U)) {
    return NFC_ProtocolError;
  }

  for (it = 0; it <= (uint8_t)maxTxBR; it++) {
    if ((supPCD2PICC & (0x01U << it)) != 0U) {
      txBR = (RFal_BitRate)it; /* PRQA S 4342 # MISRA 10.5 - Layout of enum RFal_BitRate and above clamping of maxTxBR guarantee no invalid enum values to be created */
    }
  }
  for (it = 0; it <= (uint8_t)maxRxBR; it++) {
    if ((supPICC2PCD & (0x01U << it)) != 0U) {
      rxBR = (RFal_BitRate)it; /* PRQA S 4342 # MISRA 10.5 - Layout of enum RFal_BitRate and above clamping of maxTxBR guarantee no invalid enum values to be created */
    }
  }

  it = 0;
  currenttxBR = (uint8_t)txBR;
  currentrxBR = (uint8_t)rxBR;

  /*******************************************************************************/
  /* Send S(PARAMETERS) - Bit rates Activation */
  sParam.sParam.tag = RFAL_ISODEP_SPARAM_TAG_BLOCKINFO;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_BRACT;
  sParam.sParam.value[it++] = (RFAL_ISODEP_SPARAM_TVL_HDR_LEN + RFAL_ISODEP_SPARAM_TAG_PCD2PICC_LEN + RFAL_ISODEP_SPARAM_TVL_HDR_LEN + RFAL_ISODEP_SPARAM_TAG_PICC2PCD_LEN);
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_SEL_PCD2PICC;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_PCD2PICC_LEN;
  sParam.sParam.value[it++] = ((uint8_t)0x01U << currenttxBR);
  sParam.sParam.value[it++] = 0x00U;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_SEL_PICC2PCD;
  sParam.sParam.value[it++] = RFAL_ISODEP_SPARAM_TAG_PICC2PCD_LEN;
  sParam.sParam.value[it++] = ((uint8_t)0x01U << currentrxBR);
  sParam.sParam.value[it++] = 0x00U;
  sParam.sParam.length = it;

  ret = RFal_TransceiveBlockingTxRx((uint8_t *)&sParam, (RFAL_ISODEP_SPARAM_HDR_LEN + (uint16_t)it), (uint8_t *)&sParam, sizeof(RFal_ISO_Dep_ControlMsgSParam), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, (isoDepDev->info.FWT + isoDepDev->info.dFWT));
  if(ret < NFC_OK)
  {
      return ret;
  }

  it = 0;

  /*******************************************************************************/
  /* Check S(PARAMETERS) Acknowledge  */
  if ((sParam.pcb != ISODEP_PCB_SPARAMETERS) || (sParam.sParam.tag != RFAL_ISODEP_SPARAM_TAG_BLOCKINFO) ||
      (sParam.sParam.value[it] != RFAL_ISODEP_SPARAM_TAG_BRACK) || (rcvLen < RFAL_ISODEP_SPARAM_HDR_LEN)) {
    return NFC_ProtocolError;
  }

  ret = RFal_SetBitRate(txBR, rxBR);
  if(ret < NFC_OK)
  {
      return ret;
  }

  isoDepDev->info.DRI = txBR;
  isoDepDev->info.DSI = rxBR;

  return NFC_OK;
}

/*******************************************************************************/
void RFal_ISO_Dep_CalcBitRate(RFal_BitRate maxAllowedBR, uint8_t piccBRCapability, RFal_BitRate *dsi, RFal_BitRate *dri)
{
  uint8_t driMask;
  uint8_t dsiMask;
  int8_t i;
  bool bitrateFound;
  RFal_BitRate curMaxBR;

  curMaxBR = maxAllowedBR;

  do {
    bitrateFound = true;

    (*dsi) = RFAL_BR_106;
    (*dri) = RFAL_BR_106;

    /* Digital 1.0  5.6.2.5 & 11.6.2.14: A received RFU value of b4 = 1b MUST be interpreted as if b7 to b1 ? 0000000b (only 106 kbits/s in both direction) */
    if (((RFAL_ISODEP_BITRATE_RFU_MASK & piccBRCapability) != 0U) || (curMaxBR > RFAL_BR_848)) {
      return;
    }

    /***************************************************************************/
    /* Determine Listen->Poll bit rate */
    dsiMask = (piccBRCapability & RFAL_ISODEP_BSI_MASK);
    for (i = 2; i >= 0; i--) {
      // Check supported bit rate from the highest
      if (((dsiMask & (0x10U << (uint8_t)i)) != 0U) && (((uint8_t)i + 1U) <= (uint8_t)curMaxBR)) {
        uint8_t newdsi = ((uint8_t)i) + 1U;
        (*dsi) = (RFal_BitRate)newdsi; /* PRQA S 4342 # MISRA 10.5 - Layout of enum RFal_BitRate and range of loop variable guarantee no invalid enum values to be created */
        break;
      }
    }

    /***************************************************************************/
    /* Determine Poll->Listen bit rate */
    driMask = (piccBRCapability & RFAL_ISODEP_BRI_MASK);
    for (i = 2; i >= 0; i--) {
      /* Check supported bit rate from the highest */
      if (((driMask & (0x01U << (uint8_t)i)) != 0U) && (((uint8_t)i + 1U) <= (uint8_t)curMaxBR)) {
        const uint8_t newdri = ((uint8_t)i) + 1U;
        (*dri) = (RFal_BitRate)newdri; /* PRQA S 4342 # MISRA 10.5 - Layout of enum RFal_BitRate and range of loop variable guarantee no invalid enum values to be created */
        break;
      }
    }

    /***************************************************************************/
    /* Check if different bit rate is supported */

    /* Digital 1.0 Table 67: if b8=1b, then only the same bit rate divisor for both directions is supported */
    if ((piccBRCapability & RFAL_ISODEP_SAME_BITRATE_MASK) != 0U) {
      (*dsi) = MIN((*dsi), (*dri));
      (*dri) = (*dsi);
      /* Check that the baudrate is supported */
      if ((RFAL_BR_106 != (*dsi)) && (!(((dsiMask & (0x10U << ((uint8_t)(*dsi) - 1U))) != 0U) && ((driMask & (0x01U << ((uint8_t)(*dri) - 1U))) != 0U)))) {
        bitrateFound = false;
        curMaxBR = (*dsi); /* set allowed bitrate to be lowest and determine bit rate again */
      }
    }
  } while (!(bitrateFound));
}

/*******************************************************************************/
uint32_t RFal_ISO_Dep_SFGI2SFGT(uint8_t sfgi)
{
  uint32_t sfgt;
  uint8_t tmpSFGI;

  tmpSFGI = sfgi;

  if (tmpSFGI > ISODEP_SFGI_MAX) {
    tmpSFGI = ISODEP_SFGI_MIN;
  }

  if (tmpSFGI != ISODEP_SFGI_MIN) {
    /* If sfgi != 0 wait SFGT + dSFGT   Digital 1.1  13.8.2.1 */
    sfgt = isoDepCalcSGFT(sfgi) + isoDepCalcdSGFT(sfgi);
  }
  /* Otherwise use FDTPoll min Digital  1.1  13.8.2.3*/
  else {
    sfgt = RFAL_FDT_POLL_NFCA_POLLER;
  }

  /* Convert carrier cycles to milli seconds */
  return (rfalConv1fcToMs(sfgt) + 1U);
}


/*******************************************************************************/
void RFal_ISO_Dep_Apdu2IBLockParam(RFal_ISO_Dep_ApduTxRxParam apduParam, RFal_ISO_Dep_TxRxParam *iBlockParam, uint16_t txPos, uint16_t rxPos)
{
  iBlockParam->DID = apduParam.DID;
  iBlockParam->FSx = apduParam.FSx;
  iBlockParam->ourFSx = apduParam.ourFSx;
  iBlockParam->FWT = apduParam.FWT;
  iBlockParam->dFWT = apduParam.dFWT;

  if ((apduParam.txBufLen - txPos) > RFal_ISO_Dep_GetMaxInfLen()) {
    iBlockParam->isTxChaining = true;
    iBlockParam->txBufLen = RFal_ISO_Dep_GetMaxInfLen();
  } else {
    iBlockParam->isTxChaining = false;
    iBlockParam->txBufLen = (apduParam.txBufLen - txPos);
  }

  /* TxBuf is moved to the beginning for every I-Block */
  iBlockParam->txBuf = (RFal_ISO_Dep_BufFormat *)apduParam.txBuf; /*  PRQA S 0310 # MISRA 11.3 - Intentional safe cast to avoiding large buffer duplication */
  iBlockParam->rxBuf = apduParam.tmpBuf;                       /* Simply using the apdu buffer is not possible because of current ACK handling */
  iBlockParam->isRxChaining = &gIsoDep.isAPDURxChaining;
  iBlockParam->rxLen = apduParam.rxLen;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_StartApduTransceive(RFal_ISO_Dep_ApduTxRxParam param)
{
  RFal_ISO_Dep_TxRxParam txRxParam;

  /* Initialize and store APDU context */
  gIsoDep.APDUParam = param;
  gIsoDep.APDUTxPos = 0;
  gIsoDep.APDURxPos = 0;

  /* Assign current FSx to calculate INF length (only change the FSx from activation if no to Keep) */
  gIsoDep.ourFsx = ((param.ourFSx != RFAL_ISODEP_FSX_KEEP) ? param.ourFSx : gIsoDep.ourFsx);
  gIsoDep.fsx = param.FSx;

  /* Convert APDU TxRxParams to I-Block TxRxParams */
  RFal_ISO_Dep_Apdu2IBLockParam(gIsoDep.APDUParam, &txRxParam, gIsoDep.APDUTxPos, gIsoDep.APDURxPos);

  return RFal_ISO_Dep_StartTransceive(txRxParam);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO_Dep_GetApduTransceiveStatus(void)
{
  NFC_OpResult ret;
  RFal_ISO_Dep_TxRxParam txRxParam;

  ret = RFal_ISO_Dep_GetTransceiveStatus();
  switch (ret) {
    /*******************************************************************************/
    case NFC_OK:

      /* Check if we are still doing chaining on Tx */
      if (gIsoDep.isTxChaining) {
        /* Add already Tx bytes */
        gIsoDep.APDUTxPos += gIsoDep.txBufLen;

        /* Convert APDU TxRxParams to I-Block TxRxParams */
        RFal_ISO_Dep_Apdu2IBLockParam(gIsoDep.APDUParam, &txRxParam, gIsoDep.APDUTxPos, gIsoDep.APDURxPos);

        if (txRxParam.txBufLen > 0U) {
          /* MISRA 21.18 */
          /* Move next I-Block to beginning of APDU Tx buffer */
          memcpy(gIsoDep.APDUParam.txBuf->apdu, &gIsoDep.APDUParam.txBuf->apdu[gIsoDep.APDUTxPos], txRxParam.txBufLen);
        }

        ret = RFal_ISO_Dep_StartTransceive(txRxParam);
        if(ret < NFC_OK)
        {
            return ret;
        }

        return NFC_Busy;
      }

    /* APDU TxRx is done */
    /* fall through */

    /*******************************************************************************/
    case NFC_Again: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      /* Check if no APDU transceive has been started before (data from RFal_ISO_Dep_ListenStartActivation) */
      if (gIsoDep.APDUParam.rxLen == NULL) {
        if (ret == NFC_Again) {
          /* In Listen mode first chained packet cannot be retrieved via APDU interface */
          return NFC_Unsupport;
        }

        /* TxRx is complete and full data is already available */
        return NFC_OK;
      }

      if (*gIsoDep.APDUParam.rxLen > 0U) { /* MISRA 21.18 */
        /* Ensure that data in tmpBuf still fits into APDU buffer */
        if ((gIsoDep.APDURxPos + (*gIsoDep.APDUParam.rxLen)) > (uint16_t)RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN) {
          return NFC_MemoryError;
        }

        /* Copy chained packet from tmp buffer to APDU buffer */
        memcpy(&gIsoDep.APDUParam.rxBuf->apdu[gIsoDep.APDURxPos], gIsoDep.APDUParam.tmpBuf->inf, *gIsoDep.APDUParam.rxLen);
        gIsoDep.APDURxPos += *gIsoDep.APDUParam.rxLen;
      }

      /* Update output param rxLen */
      *gIsoDep.APDUParam.rxLen = gIsoDep.APDURxPos;

      /* Wait for following I-Block or APDU TxRx has finished */
      return ((ret == NFC_Again) ? NFC_Busy : NFC_OK);

    /*******************************************************************************/
    default:
      return ret;
  }

  *gIsoDep.APDUParam.rxLen = gIsoDep.APDURxPos;

  return NFC_OK;
}
