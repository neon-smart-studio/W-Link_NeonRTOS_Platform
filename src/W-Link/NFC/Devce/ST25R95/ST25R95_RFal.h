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

#ifndef RFST25R95_RFAL_H
#define RFST25R95_RFAL_H

#include "ST25R95_Def.h"

#define ST25R95_FELICA_POLL_DELAY_TIME     512                                           /*!<  FeliCa Poll Processing time is 2.417 ms ~512*64/fc Digital 1.1 A4              */
#define ST25R95_FELICA_POLL_SLOT_TIME      256                                           /*!<  FeliCa Poll Time Slot duration is 1.208 ms ~256*64/fc Digital 1.1 A4           */

#define ST25R95_ISO14443A_SDD_RES_LEN      5                                             /*!< SDD_RES | Anticollision (UID CLn) length  -  rfalNfcaSddRes                     */

#define ST25R95_ISO14443A_APPENDCRC                                             0x20U /*!< Transmission flags bit 5: Append CRC        */
#define ST25R95_ISO14443A_SPLITFRAME                                            0x40U /*!< Transmission flags bit 6: SplitFrame        */
#define ST25R95_ISO14443A_TOPAZFORMAT                                           0x80U /*!< Transmission flags bit 7: Topaz send format */

#define ST25R95_IDLE_DEFAULT_WUPERIOD                                           0x24U /*!< Fixed WU Period to reach ~300 ms timeout with Max Sleep = 0 */

#define ST25R95_TAGDETECT_DEF_CALIBRATION 0x7C             /*!< Tag Detection Calibration default value                    */

#define ST25R95_FWT_NONE                              0xFFFFFFFFU

#define ST25R95_ACSTATE_IDLE                             0x00U /*!< AC Filter state: Idle */
#define ST25R95_ACSTATE_READYA                           0x01U /*!< AC Filter state: ReadyA */
#define ST25R95_ACSTATE_ACTIVE                           0x04U /*!< AC Filter state: Active */
#define ST25R95_ACSTATE_HALT                             0x80U /*!< AC Filter state: Halt */
#define ST25R95_ACSTATE_READYAX                          0x81U /*!< AC Filter state: ReadyA* */
#define ST25R95_ACSTATE_ACTIVEX                          0x84U /*!< AC Filter state: Active* */

#define ST25R95_Conv1fcTo8fc( t )                (uint32_t)( (uint32_t)(t) / ST25R95_1FC_IN_8FC )                               /*!< Converts the given t from 1/fc to 8/fc     */
#define ST25R95_Conv8fcTo1fc( t )                (uint32_t)( (uint32_t)(t) * ST25R95_1FC_IN_8FC )                               /*!< Converts the given t from 8/fc to 1/fc     */

#define ST25R95_Conv1fcTo64fc( t )               (uint32_t)( (uint32_t)(t) / ST25R95_1FC_IN_64FC )                              /*!< Converts the given t from 1/fc  to 64/fc   */
#define ST25R95_Conv64fcTo1fc( t )               (uint32_t)( (uint32_t)(t) * ST25R95_1FC_IN_64FC )                              /*!< Converts the given t from 64/fc to 1/fc    */

#define ST25R95_Conv1fcTo512fc( t )              (uint32_t)( (uint32_t)(t) / ST25R95_1FC_IN_512FC )                             /*!< Converts the given t from 1/fc  to 512/fc  */
#define ST25R95_Conv512fcTo1fc( t )              (uint32_t)( (uint32_t)(t) * ST25R95_1FC_IN_512FC )                             /*!< Converts the given t from 512/fc to 1/fc   */

#define ST25R95_Conv1fcTo2018fc( t )             (uint32_t)( (uint32_t)(t) / ST25R95_1FC_IN_2048FC )                            /*!< Converts the given t from 1/fc to 2048/fc  */
#define ST25R95_Conv2048fcTo1fc( t )             (uint32_t)( (uint32_t)(t) * ST25R95_1FC_IN_2048FC )                            /*!< Converts the given t from 2048/fc to 1/fc  */

#define ST25R95_Conv1fcTo4096fc( t )             (uint32_t)( (uint32_t)(t) / ST25R95_1FC_IN_4096FC )                            /*!< Converts the given t from 1/fc to 4096/fc  */
#define ST25R95_Conv4096fcTo1fc( t )             (uint32_t)( (uint32_t)(t) * ST25R95_1FC_IN_4096FC )                            /*!< Converts the given t from 4096/fc to 1/fc  */

#define ST25R95_Conv1fcToMs( t )                 (uint32_t)( (uint32_t)(t) / ST25R95_1MS_IN_1FC )                               /*!< Converts the given t from 1/fc to ms       */
#define ST25R95_ConvMsTo1fc( t )                 (uint32_t)( (uint32_t)(t) * ST25R95_1MS_IN_1FC )                               /*!< Converts the given t from ms to 1/fc       */

#define ST25R95_Conv1fcToUs( t )                 (uint32_t)( ((uint32_t)(t) * ST25R95_US_IN_MS) / ST25R95_1MS_IN_1FC)              /*!< Converts the given t from 1/fc to us       */
#define ST25R95_ConvUsTo1fc( t )                 (uint32_t)( ((uint32_t)(t) * ST25R95_1MS_IN_1FC) / ST25R95_US_IN_MS)              /*!< Converts the given t from us to 1/fc       */

#define ST25R95_Conv64fcToMs( t )                (uint32_t)( (uint32_t)(t) / (ST25R95_1MS_IN_1FC / ST25R95_1FC_IN_64FC) )          /*!< Converts the given t from 64/fc to ms      */
#define ST25R95_ConvMsTo64fc( t )                (uint32_t)( (uint32_t)(t) * (ST25R95_1MS_IN_1FC / ST25R95_1FC_IN_64FC) )          /*!< Converts the given t from ms to 64/fc      */

#define ST25R95_ConvBitsToBytes( n )             (uint16_t)( ((uint16_t)(n)+(ST25R95_BITS_IN_BYTE-1U)) / (ST25R95_BITS_IN_BYTE) )  /*!< Converts the given n from bits to bytes    */
#define ST25R95_ConvBytesToBits( n )             (uint32_t)( (uint32_t)(n) * (ST25R95_BITS_IN_BYTE) )                           /*!< Converts the given n from bytes to bits    */

#define ST25R95_FELICA_LEN_LEN                        1U                                           /*!< FeliCa LEN byte length                                              */
#define ST25R95_FELICA_POLL_REQ_LEN                   (ST25R95_FELICA_LEN_LEN + 1U + 2U + 1U + 1U)    /*!< FeliCa Poll Request length (LEN + CMD + SC + RC + TSN)              */
#define ST25R95_FELICA_POLL_RES_LEN                   (ST25R95_FELICA_LEN_LEN + 1U + 8U + 8U + 2U)    /*!< Maximum FeliCa Poll Response length (LEN + CMD + NFCID2 + PAD + RD) */
#define ST25R95_FELICA_POLL_MAX_SLOTS                 16U                                          /*!< Maximum number of slots (TSN) on FeliCa Poll                        */

#define ST25R95_TIMING_NONE                           0x00U                                        /*!< Timing disabled | Don't apply                     */

#define ST25R95_1FC_IN_4096FC                         (uint32_t)4096U                              /*!< Number of 1/fc cycles in one 4096/fc              */
#define ST25R95_1FC_IN_2048FC                         (uint32_t)2048U                              /*!< Number of 1/fc cycles in one 2048/fc              */
#define ST25R95_1FC_IN_512FC                          (uint32_t)512U                               /*!< Number of 1/fc cycles in one 512/fc               */
#define ST25R95_1FC_IN_64FC                           (uint32_t)64U                                /*!< Number of 1/fc cycles in one 64/fc                */
#define ST25R95_1FC_IN_8FC                            (uint32_t)8U                                 /*!< Number of 1/fc cycles in one 8/fc                 */
#define ST25R95_US_IN_MS                              (uint32_t)1000U                              /*!< Number of us in one ms                            */
#define ST25R95_1MS_IN_1FC                            (uint32_t)13560U                             /*!< Number of 1/fc cycles in 1ms                      */
#define ST25R95_BITS_IN_BYTE                          (uint16_t)8U                                 /*!< Number of bits in one byte                        */

#define ST25R95_GPT_MAX_1FC         ST25R95_Conv8fcTo1fc(0xFFFF)                     /*!< Max GPT steps in 1fc (0xFFFF steps of 8/fc    => 0xFFFF * 590ns  = 38,7ms)      */
#define ST25R95_NRT_MAX_1FC         ST25R95_Conv4096fcTo1fc(0xFFFF)                  /*!< Max NRT steps in 1fc (0xFFFF steps of 4096/fc => 0xFFFF * 302us  = 19.8s)       */
#define ST25R95_NRT_DISABLED        0                                            /*!< NRT Disabled: All 0 No-response timer is not started, wait forever              */
#define ST25R95_MRT_MAX_1FC         ST25R95_Conv64fcTo1fc(0x00FF)                    /*!< Max MRT steps in 1fc (0x00FF steps of 64/fc   => 0x00FF * 4.72us = 1.2ms)       */
#define ST25R95_MRT_MIN_1FC         ST25R95_Conv64fcTo1fc(0x0004)                    /*!< Min MRT steps in 1fc (0<=mrt<=4 ; 4 (64/fc)  => 0x0004 * 4.72us = 18.88us)      */
#define ST25R95_GT_MAX_1FC          ST25R95_ConvMsTo1fc(5000)                        /*!< Max GT value allowed in 1/fc                                                    */
#define ST25R95_GT_MIN_1FC          ST25R95_ConvMsTo1fc(ST25R95_SW_TMR_MIN_1MS) /*!< Min GT value allowed in 1/fc                                                    */
#define ST25R95_SW_TMR_MIN_1MS      1

#define ST25R95_LM_MASK_NFCA                          ((uint32_t)1U<<(uint8_t)ST25R95_Mode_Listen_NFCA)        /*!< Bitmask for Listen Mode enabling NFCA    */
#define ST25R95_LM_MASK_NFCB                          ((uint32_t)1U<<(uint8_t)ST25R95_Mode_Listen_NFCB)        /*!< Bitmask for Listen Mode enabling NFCB    */
#define ST25R95_LM_MASK_NFCF                          ((uint32_t)1U<<(uint8_t)ST25R95_Mode_Listen_NFCF)        /*!< Bitmask for Listen Mode enabling NFCF    */
#define ST25R95_LM_MASK_ACTIVE_P2P                    ((uint32_t)1U<<(uint8_t)ST25R95_Mode_Listen_Active_P2P)  /*!< Bitmask for Listen Mode enabling AP2P    */

#define ST25R95_LM_SENS_RES_LEN                       2U                                           /*!< NFC-A SENS_RES (ATQA) length                      */
#define ST25R95_LM_SENSB_RES_LEN                      13U                                          /*!< NFC-B SENSB_RES (ATQB) length                     */
#define ST25R95_LM_SENSF_RES_LEN                      19U                                          /*!< NFC-F SENSF_RES  length                           */
#define ST25R95_LM_SENSF_SC_LEN                       2U                                           /*!< NFC-F System Code length                          */

#define ST25R95_NFCID3_LEN                            10U                                          /*!< NFCID3 length                                     */
#define ST25R95_NFCID2_LEN                            8U                                           /*!< NFCID2 length                                     */
#define ST25R95_NFCID1_TRIPLE_LEN                     10U                                          /*!< NFCID1 length                                     */
#define ST25R95_NFCID1_DOUBLE_LEN                     7U                                           /*!< NFCID1 length                                     */
#define ST25R95_NFCID1_SINGLE_LEN                     4U       

#define ST25R95_WUM_REFERENCE_AUTO           0xFFU      /*!< Indicates new reference is set by the driver */

/*! ST25R95 modes    */
typedef enum {
  ST25R95_Mode_None                   = 0,    /*!< No mode selected/defined                                         */
  ST25R95_Mode_Poll_NFCA              = 1,    /*!< Mode to perform as NFCA (ISO14443A) Poller (PCD)                 */
  ST25R95_Mode_Poll_NFCA_T1T          = 2,    /*!< Mode to perform as NFCA T1T (Topaz) Poller (PCD)                 */
  ST25R95_Mode_Poll_NFCB              = 3,    /*!< Mode to perform as NFCB (ISO14443B) Poller (PCD)                 */
  ST25R95_Mode_Poll_B_PRIME           = 4,    /*!< Mode to perform as B' Calypso (Innovatron) (PCD)                 */
  ST25R95_Mode_Poll_B_CTS             = 5,    /*!< Mode to perform as CTS Poller (PCD)                              */
  ST25R95_Mode_Poll_NFCF              = 6,    /*!< Mode to perform as NFCF (FeliCa) Poller (PCD)                    */
  ST25R95_Mode_Poll_NFCV              = 7,    /*!< Mode to perform as NFCV (ISO15963) Poller (PCD)                  */
  ST25R95_Mode_Poll_PICOPASS          = 8,    /*!< Mode to perform as PicoPass / iClass Poller (PCD)                */
  ST25R95_Mode_Poll_Active_P2P        = 9,    /*!< Mode to perform as Active P2P (ISO18092) Initiator               */
  ST25R95_Mode_Listen_NFCA            = 10,   /*!< Mode to perform as NFCA (ISO14443A) Listener (PICC)              */
  ST25R95_Mode_Listen_NFCB            = 11,   /*!< Mode to perform as NFCA (ISO14443B) Listener (PICC)              */
  ST25R95_Mode_Listen_NFCF            = 12,   /*!< Mode to perform as NFCA (ISO15963) Listener (PICC)               */
  ST25R95_Mode_Listen_Active_P2P      = 13    /*!< Mode to perform as Active P2P (ISO18092) Target                  */
} ST25R95_Mode;

/*! RFAL Listen Mode States */
typedef enum {
  ST25R95_LM_State_Not_Init              = 0x00,     /*!< Not Initialized state                       */
  ST25R95_LM_State_Power_Off             = 0x01,     /*!< Power Off state                             */
  ST25R95_LM_State_Idle                  = 0x02,     /*!< Idle state  Activity 1.1  5.2               */
  ST25R95_LM_State_Ready_A               = 0x03,     /*!< Ready A state  Activity 1.1  5.3 5.4 & 5.5  */
  ST25R95_LM_State_Ready_B               = 0x04,     /*!< Ready B state  Activity 1.1  5.11 5.12      */
  ST25R95_LM_State_Ready_F               = 0x05,     /*!< Ready F state  Activity 1.1  5.15           */
  ST25R95_LM_State_Active_A              = 0x06,     /*!< Active A state  Activity 1.1  5.6           */
  ST25R95_LM_State_CardEmu_4A            = 0x07,     /*!< Card Emulation 4A state  Activity 1.1  5.10 */
  ST25R95_LM_State_CardEmu_4B            = 0x08,     /*!< Card Emulation 4B state  Activity 1.1  5.14 */
  ST25R95_LM_State_CardEmu_3             = 0x09,     /*!< Card Emulation 3 state  Activity 1.1  5.18  */
  ST25R95_LM_State_Target_A              = 0x0A,     /*!< Target A state  Activity 1.1  5.9           */
  ST25R95_LM_State_Target_F              = 0x0B,     /*!< Target F state  Activity 1.1  5.17          */
  ST25R95_LM_State_Sleep_A               = 0x0C,     /*!< Sleep A state  Activity 1.1  5.7            */
  ST25R95_LM_State_Sleep_B               = 0x0D,     /*!< Sleep B state  Activity 1.1  5.13           */
  ST25R95_LM_State_Ready_Ax              = 0x0E,     /*!< Ready A* state  Activity 1.1  5.3 5.4 & 5.5 */
  ST25R95_LM_State_Active_Ax             = 0x0F,     /*!< Active A* state  Activity 1.1  5.6          */
  ST25R95_LM_State_Sleep_AF              = 0x10,     /*!< Sleep AF state  Activity 1.1  5.19          */

} ST25R95_LmState;

typedef enum {
  ST25R95_LM_NFCID_LEN_04  = ST25R95_NFCID1_SINGLE_LEN, /*!< Listen mode indicates  4 byte NFCID */
  ST25R95_LM_NFCID_LEN_07  = ST25R95_NFCID1_DOUBLE_LEN, /*!< Listen mode indicates  7 byte NFCID */
  ST25R95_LM_NFCID_LEN_10  = ST25R95_NFCID1_TRIPLE_LEN, /*!< Listen mode indicates 10 byte NFCID */
} ST25R95_Lm_Nfcid_Len;

typedef enum {
  ST25R95_14443A_SHORTFRAME_CMD_WUPA = 0x52,  /*!< ISO14443A WUPA / NFC-A ALL_REQ  */
  ST25R95_14443A_SHORTFRAME_CMD_REQA = 0x26   /*!< ISO14443A REQA / NFC-A SENS_REQ */
} ST25R95_14443AShortFrameCmd;

/*! ST25R95 main states flags    */
typedef enum {
  ST25R95_State_Idle                  = 0,
  ST25R95_State_Init                  = 1,
  ST25R95_State_Mode_Set              = 2,

  ST25R95_State_TXRX                  = 3,
  ST25R95_State_LM                    = 4,
  ST25R95_State_WUM                   = 5

} ST25R95_State;

typedef enum {
  ST25R95_Transceive_State_Idle             = 0,
  ST25R95_Transceive_State_Init             = 1,
  ST25R95_Transceive_State_Start            = 2,

  ST25R95_Transceive_State_TX_Idle          = 11,
  ST25R95_Transceive_State_TX_Wait_GT       = 12,
  ST25R95_Transceive_State_TX_Wait_FDT      = 13,
  ST25R95_Transceive_State_TX_Prepare_TX    = 14,
  ST25R95_Transceive_State_TX_Transmit      = 15,
  ST25R95_Transceive_State_TX_Wait_WL       = 16,
  ST25R95_Transceive_State_TX_Reload_FIFO   = 17,
  ST25R95_Transceive_State_TX_Wait_TXE      = 18,
  ST25R95_Transceive_State_TX_Done          = 19,
  ST25R95_Transceive_State_TX_Fail          = 20,

  ST25R95_Transceive_State_RX_Idle          = 81,
  ST25R95_Transceive_State_RX_Wait_EON      = 82,
  ST25R95_Transceive_State_RX_Wait_RXS      = 83,
  ST25R95_Transceive_State_RX_Wait_RXE      = 84,
  ST25R95_Transceive_State_RX_Read_FIFO     = 85,
  ST25R95_Transceive_State_RX_ERR_Check     = 86,
  ST25R95_Transceive_State_RX_Read_Data     = 87,
  ST25R95_Transceive_State_RX_Wait_EOF      = 88,
  ST25R95_Transceive_State_RX_Done          = 89,
  ST25R95_Transceive_State_RX_Fail          = 90,

} ST25R95_TransceiveState;
/*! RFAL Wake-Up Mode States */
typedef enum {
  ST25R95_WUM_STATE_NOT_INIT              = 0x00,     /*!< Not Initialized state                       */
  ST25R95_WUM_STATE_INITIALIZING          = 0x01,     /*!< Wake-Up mode is starting                    */
  ST25R95_WUM_STATE_ENABLED               = 0x02,     /*!< Wake-Up mode is enabled                     */
  ST25R95_WUM_STATE_ENABLED_WOKE          = 0x03,     /*!< Wake-Up mode enabled and has received IRQ(s)*/
} ST25R95_WumState;

/*! RFAL Wake-Up Period/Timer */
typedef enum {
  ST25R95_WUM_PERIOD_10MS      = 0x00,     /*!< Wake-Up timer ~9.7ms                                       */
  ST25R95_WUM_PERIOD_15MS      = 0x01,     /*!< Wake-Up timer ~13.3ms                                      */
  ST25R95_WUM_PERIOD_20MS      = 0x02,     /*!< Wake-Up timer ~19.3ms                                      */
  ST25R95_WUM_PERIOD_25MS      = 0x03,     /*!< Wake-Up timer ~26.6ms                                      */
  ST25R95_WUM_PERIOD_30MS      = 0x02,     /*!< Wake-Up timer 30ms                          */
  ST25R95_WUM_PERIOD_40MS      = 0x04,     /*!< Wake-Up timer ~38.7ms                                      */
  ST25R95_WUM_PERIOD_50MS      = 0x04,     /*!< Wake-Up timer 50ms                          */
  ST25R95_WUM_PERIOD_55MS      = 0x05,     /*!< Wake-Up timer ~53.2ms                                      */
  ST25R95_WUM_PERIOD_60MS      = 0x05,     /*!< Wake-Up timer 60ms                          */
  ST25R95_WUM_PERIOD_70MS      = 0x06,     /*!< Wake-Up timer 70ms                          */
  ST25R95_WUM_PERIOD_80MS      = 0x06,     /*!< Wake-Up timer ~77.3ms                                      */
  ST25R95_WUM_PERIOD_100MS     = 0x10,     /*!< Wake-Up timer 100ms                         */
  ST25R95_WUM_PERIOD_105MS     = 0x07,     /*!< Wake-Up timer ~106.3ms                                     */
  ST25R95_WUM_PERIOD_155MS     = 0x08,     /*!< Wake-Up timer ~154.7ms                                     */
  ST25R95_WUM_PERIOD_200MS     = 0x11,     /*!< Wake-Up timer 200ms                         */
  ST25R95_WUM_PERIOD_215MS     = 0x09,     /*!< Wake-Up timer ~212.7ms                                     */
  ST25R95_WUM_PERIOD_300MS     = 0x12,     /*!< Wake-Up timer 300ms                         */
  ST25R95_WUM_PERIOD_310MS     = 0x0A,     /*!< Wake-Up timer ~309.3ms                                     */
  ST25R95_WUM_PERIOD_400MS     = 0x13,     /*!< Wake-Up timer 400ms                         */
  ST25R95_WUM_PERIOD_425MS     = 0x0B,     /*!< Wake-Up timer ~425.3ms                                     */
  ST25R95_WUM_PERIOD_500MS     = 0x14,     /*!< Wake-Up timer 500ms                                        */
  ST25R95_WUM_PERIOD_600MS     = 0x15,     /*!< Wake-Up timer 600ms                         */
  ST25R95_WUM_PERIOD_620MS     = 0x0C,     /*!< Wake-Up timer ~618.6ms                                     */
  ST25R95_WUM_PERIOD_700MS     = 0x16,     /*!< Wake-Up timer 700ms                         */
  ST25R95_WUM_PERIOD_800MS     = 0x17,     /*!< Wake-Up timer 800ms                         */
  ST25R95_WUM_PERIOD_850MS     = 0x0D,     /*!< Wake-Up timer ~850.6ms                                     */
  ST25R95_WUM_PERIOD_1240MS    = 0x0E,     /*!< Wake-Up timer ~1237.3ms                                    */
  ST25R95_WUM_PERIOD_1700MS    = 0x0F,     /*!< Wake-Up timer ~1701.2ms                                    */
} ST25R95_WumPeriod;


/*! RFAL Wake-Up Period/Timer */
typedef enum {
  ST25R95_WUM_AA_WEIGHT_4       = 0x00,    /*!< Wake-Up Auto Average Weight 4                              */
  ST25R95_WUM_AA_WEIGHT_8       = 0x01,    /*!< Wake-Up Auto Average Weight 8                              */
  ST25R95_WUM_AA_WEIGHT_16      = 0x02,    /*!< Wake-Up Auto Average Weight 16                             */
  ST25R95_WUM_AA_WEIGHT_32      = 0x03,    /*!< Wake-Up Auto Average Weight 32                             */
} ST25R95_WumAAWeight;


/*! RFAL Wake-Up measurement duration */
typedef enum {
  ST25R95_WUM_MEAS_DUR_26_10    = 0,       /*!< WU measurement duration: 26.0us (slow) / 10.6us (fast)     */
  ST25R95_WUM_MEAS_DUR_30_14    = 1,       /*!< WU measurement duration: 29.5us (slow) / 14.2us (fast)     */
  ST25R95_WUM_MEAS_DUR_34_19    = 2,       /*!< WU measurement duration: 34.2us (slow) / 18.9us (fast)     */
  ST25R95_WUM_MEAS_DUR_44_28    = 3,       /*!< WU measurement duration: 43.7us (slow) / 28.3us (fast)     */
} ST25R95_WumMeasDuration;


/*! RFAL Wake-Up measurement filter */
typedef enum {
  ST25R95_WUM_MEAS_FIL_SLOW    = false,    /*!< Wake-Up measurement slow filter                            */
  ST25R95_WUM_MEAS_FIL_FAST    = true,     /*!< Wake-up measurement fast filter                            */
} ST25R95_WumMeasFilter;

/*! System callback to indicate an event that requires a system reRun        */
typedef void (* ST25R95_UpperLayerCallback)(void);

/*! Callback to be executed before a Transceive                              */
typedef void (* ST25R95_PreTxRxCallback)(void);

/*! Callback to be executed after a Transceive                               */
typedef void (* ST25R95_PostTxRxCallback)(void);

/*! Callback to sync actual transmission start                               */
typedef bool (* ST25R95_SyncTxRxCallback)(void);

/*! Callback upon External Field detected while in Listen Mode              */
typedef void (* ST25R95_LmEonCallback)(void);

/*! SPI transceive context definition */
typedef struct {
  uint8_t              *txBuf;                  /*!< (In)  Buffer where outgoing message is located       */
  uint16_t              txBufLen;               /*!< (In)  Length of the outgoing message in bits         */

  uint8_t              *rxBuf;                  /*!< (Out) Buffer where incoming message will be placed   */
  uint16_t              rxBufLen;               /*!< (In)  Maximum length of the incoming message in bits */
  uint16_t             *rxRcvdLen;              /*!< (Out) Actual received length in bits                 */

  uint32_t              flags;                  /*!< (In)  TransceiveFlags indication special handling    */
  uint32_t              fwt;                    /*!< (In)  Frame Waiting Time in 1/fc                     */
} ST25R95_TransceiveContext;

/*! Struct that holds all involved on a Transceive including the context passed by the caller     */
typedef struct {
  ST25R95_TransceiveState     state;       /*!< Current transceive state                            */
  ST25R95_TransceiveState     lastState;   /*!< Last transceive state (debug purposes)              */
  ST25R95_OpResult            status;      /*!< Current status/error of the transceive              */

  ST25R95_TransceiveContext   ctx;         /*!< The transceive context given by the caller          */
} ST25R95_TxRx;

/*! Struct that holds all context for the Listen Mode                                             */
typedef struct {
  uint8_t                *rxBuf;       /*!< Location to store incoming data in Listen Mode      */
  uint16_t                rxBufLen;    /*!< Length of rxBuf                                     */
  uint16_t               *rxLen;       /*!< Pointer to write the data length placed into rxBuf  */
  bool                    dataFlag;    /*!< Listen Mode current Data Flag                       */
} ST25R95_Lm;

typedef struct {
  ST25R95_Lm_Nfcid_Len   nfcidLen;                        /*!< NFCID Len (4, 7 or 10 bytes)              */
  uint8_t                nfcid[ST25R95_NFCID1_TRIPLE_LEN];   /*!< NFCID                                     */
  uint8_t                SENS_RES[ST25R95_LM_SENS_RES_LEN];  /*!< NFC-106k; SENS_REQ Response               */
  uint8_t                SEL_RES;                         /*!< SEL_RES (SAK) with complete NFCID1 (UID)  */
} ST25R95_LmConfPA;

typedef struct {
  uint8_t          SENSB_RES[ST25R95_LM_SENSB_RES_LEN];  /*!< SENSF_RES                               */
} ST25R95_LmConfPB;

/*! RFAL Listen Mode Passive F configs */
typedef struct {
  uint8_t          SC[ST25R95_LM_SENSF_SC_LEN];          /*!< System Code to listen for               */
  uint8_t          SENSF_RES[ST25R95_LM_SENSF_RES_LEN];  /*!< SENSF_RES                               */
} ST25R95_LmConfPF;

/*! RFAL Wake-Up channel configuration */
typedef struct {
  bool                 enabled;         /*!< Inductive Amplitude measurement enabled                    */
  uint8_t              delta;           /*!< Delta between the reference and measurement to wake-up     */
  uint8_t              fracDelta;       /*!< Fractional part of the delta [0;3] 0.25 steps (SW TD only) */
  uint8_t              reference;       /*!< Reference to be used;ST25R95_WUM_REFERENCE_AUTO sets it auto  */
  uint8_t              threshold;       /*!< Wake-Up trigger threshold bitmask                           */
  bool                 autoAvg;         /*!< Use the HW Auto Averaging feature                         */
  bool                 aaInclMeas;      /*!< When AutoAvg is enabled, include IRQ measurement           */
  ST25R95_WumAAWeight      aaWeight;        /*!< When AutoAvg is enabled, last measure weight               */
} ST25R95_WumMeasChannel;

/*! RFAL Wake-Up Mode configuration */
typedef struct ST25R95_WakeUpConfig {
  ST25R95_WumPeriod        period;          /*!< Wake-Up Timer period;how often measurement(s) is performed */
  bool                 irqTout;         /*!< IRQ at every timeout will refresh the measurement(s)       */
  bool                 swTagDetect;/*!< Use SW Tag Detection instead of HW Wake-Up mode            */
  bool                 autoAvg;         /*!< Use the HW Auto Averaging feature on the enabled channel(s)*/
  bool                 skipCal;         /*!< Do not perform calibration starting WU mode                */
  bool                 skipReCal;       /*!< Do not perform recalibration during WU mode                */
  bool                 delCal;          /*!< Delay calibration step starting WU mode                    */
  bool                 delRef;          /*!< Delay reference step starting WU mode                      */
  ST25R95_WumMeasDuration  measDur;         /*!< Wake-up measurement duration config                        */
  ST25R95_WumMeasFilter    measFil;         /*!< Wake-up measurement filter config                          */

  ST25R95_WumMeasChannel   indAmp;                        /*!< Inductive Amplitude Configuration                         */
  ST25R95_WumMeasChannel   indPha;                        /*!< Inductive Phase Configuration                             */
  ST25R95_WumMeasChannel   cap;
} ST25R95_WakeUpConfig;

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
} ST25R95_WakeUpInfo;

/*! Struct that holds all context for the Wake-Up Mode                                             */
typedef struct {
  ST25R95_WumState            state;       /*!< Current Wake-Up Mode state                           */
  ST25R95_WakeUpConfig        cfg;         /*!< Current Wake-Up Mode context                         */
  uint8_t                 CalTagDet;   /*!< Tag Detection calibration value                      */
} ST25R95_Wum;

typedef struct {
  uint32_t                GT;          /*!< GT in 1/fc                  */
  uint32_t                FDTListen;   /*!< FDTListen in 1/fc           */
  uint32_t                FDTPoll;     /*!< FDTPoll in 1/fc             */
} ST25R95_Timings;

/*! Struct that holds the software timers                                 */
typedef struct {
  uint32_t                GT;          /*!< RFAL's GT timer             */
  uint32_t                FDTPoll;     /*!< RFAL's FST Poll timer       */
} ST25R95_Timers;

/*! Struct that holds the RFAL's callbacks                                */
typedef struct {
  ST25R95_PreTxRxCallback     preTxRx;     /*!< RFAL's Pre TxRx callback    */
  ST25R95_PostTxRxCallback    postTxRx;    /*!< RFAL's Post TxRx callback   */
} ST25R95_Callbacks;

/*! RFAL error handling                                                                                                                      */
typedef enum {
  ERRORHANDLING_NONE          = 0,         /*!< No special error handling will be performed                                           */
  ERRORHANDLING_EMD           = 1          /*!< EMD suppression enabled  Digital 2.1  4.1.1.1 ; EMVCo 3.0  4.9.2 ; ISO 14443-3  8.3   */
} ST25R95_EHandling;

/*! Struct that holds RFAL's configuration settings                                                      */
typedef struct {
  uint8_t                 obsvModeTx;  /*!< RFAL's config of the ST25R3911's observation mode while Tx */
  uint8_t                 obsvModeRx;  /*!< RFAL's config of the ST25R3911's observation mode while Rx */
  ST25R95_EHandling           eHandling;   /*!< RFAL's error handling config/mode                          */
} ST25R95_Configs;

/*! Struct that holds NFC-A data - Used only inside ST25R95_ISO14443ATransceiveAnticollisionFrame()          */
typedef struct {
  uint8_t                 collByte;    /*!< NFC-A Anticollision collision byte                         */
  uint8_t                 *buf;        /*!< NFC-A Anticollision frame buffer                           */
  uint8_t                 *bytesToSend;/*!< NFC-A Anticollision NFCID|UID byte context                 */
  uint8_t                 *bitsToSend; /*!< NFC-A Anticollision NFCID|UID bit context                  */
  uint16_t                *rxLength;   /*!< NFC-A Anticollision received length                        */
  bool                    NfcaSplitFrame;
} ST25R95_NfcaWorkingData;

/*! NFCF Poll Response  NFC Forum Digital 1.1 Table 44 */
typedef uint8_t ST25R95_FeliCaPollRes[ST25R95_FELICA_POLL_RES_LEN];

/*! Struct that holds NFC-F data - Used only inside ST25R95_FelicaPoll() (static to avoid adding it into stack) */
typedef struct {
  uint16_t           actLen;                                      /* Received length                         */
  ST25R95_FeliCaPollRes *pollResList;                                 /* Location of NFC-F device list           */
  uint8_t            pollResListSize;                             /* Size of NFC-F device list               */
  uint8_t            devDetected;                                 /* Number of devices detected              */
  uint8_t            colDetected;                                 /* Number of collisions detected           */
  uint8_t            *devicesDetected;                            /* Location to place number of devices     */
  uint8_t            *collisionsDetected;                         /* Location to place number of collisions  */
  ST25R95_EHandling      curHandling;                                 /* RFAL's error handling                   */
  ST25R95_FeliCaPollRes pollResponses[ST25R95_FELICA_POLL_MAX_SLOTS];   /* FeliCa Poll response container for 16 slots */
  ST25R95_FeliCaPollSlots slots;
} ST25R95_NfcfWorkingData;

typedef struct {
  ST25R95_State         state;     /*!< RFAL's current state                            */
  ST25R95_Mode          mode;      /*!< RFAL's current mode                             */
  ST25R95_BitRate       txBR;      /*!< RFAL's current Tx Bit Rate                      */
  ST25R95_BitRate       rxBR;      /*!< RFAL's current Rx Bit Rate                      */
  bool                  field;     /*!< Current field state (On / Off)                  */

  ST25R95_Configs           conf;      /*!< RFAL's configuration settings                 */
  ST25R95_Timings           timings;   /*!< RFAL's timing setting                           */
  ST25R95_TxRx              TxRx;      /*!< RFAL's transceive management                    */
  ST25R95_Lm                Lm;        /*!< RFAL's listen mode management                   */
  ST25R95_Wum               wum;       /*!< RFAL's Wake-Up mode management                  */

  ST25R95_Timers            tmr;       /*!< RFAL's Software timers                          */
  ST25R95_Callbacks         callbacks; /*!< RFAL's callbacks                                */

  uint8_t               protocol;  /*!< ProtocolSelect protocol                         */
  uint8_t               RxInformationBytes[3]; /*!< ST25R95 additional information bytes*/

  bool                  cardEmulT4AT;
  ST25R95_NfcaWorkingData     nfcaData;  /*!< RFAL's working data when supporting NFC-A     */
  ST25R95_NfcfWorkingData   nfcfData; /*!< RFAL's working data when supporting NFC-F        */
} ST25R95_RFal;

/*! Felica's command set */
typedef enum {
  FELICA_CMD_POLLING                  = 0x00, /*!< Felica Poll/REQC command (aka SENSF_REQ) to identify a card    */
  FELICA_CMD_POLLING_RES              = 0x01, /*!< Felica Poll/REQC command (aka SENSF_RES) response              */
  FELICA_CMD_REQUEST_SERVICE          = 0x02, /*!< verify the existence of Area and Service                       */
  FELICA_CMD_REQUEST_RESPONSE         = 0x04, /*!< verify the existence of a card                                 */
  FELICA_CMD_READ_WITHOUT_ENCRYPTION  = 0x06, /*!< read Block Data from a Service that requires no authentication */
  FELICA_CMD_WRITE_WITHOUT_ENCRYPTION = 0x08, /*!< write Block Data to a Service that requires no authentication  */
  FELICA_CMD_REQUEST_SYSTEM_CODE      = 0x0c, /*!< acquire the System Code registered to a card                   */
  FELICA_CMD_AUTHENTICATION1          = 0x10, /*!< authenticate a card                                            */
  FELICA_CMD_AUTHENTICATION2          = 0x12, /*!< allow a card to authenticate a Reader/Writer                   */
  FELICA_CMD_READ                     = 0x14, /*!< read Block Data from a Service that requires authentication    */
  FELICA_CMD_WRITE                    = 0x16, /*!< write Block Data to a Service that requires authentication     */
} t_ST25R95_FeliCaCmd;

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#define rfalTimerStart(timer, time_ms)         (timer) = timerCalculateTimer((uint16_t)(time_ms)) /*!< Configures and starts the RTOX timer          */
#define rfalTimerisExpired(timer)              timerIsExpired(timer)          /*!< Checks if timer has expired                   */

#define rfalRunBlocking( e, fn )                 do{ (e)=(fn); rfalWorker(); }while( (e) == ERR_BUSY )

#ifdef __cplusplus
extern "C" {
#endif
/* -------------------------------------------------------------------------- */
/* Core */
/* -------------------------------------------------------------------------- */
ST25R95_OpResult ST25R95_RFal_Init(void);
ST25R95_OpResult ST25R95_RFal_Calibrate(void);
ST25R95_OpResult ST25R95_RFal_AdjustRegulators(uint16_t *result);
ST25R95_OpResult ST25R95_RFal_Deinitialize(void);
void ST25R95_RFal_SetObsvMode(uint32_t txMode, uint32_t rxMode);
void ST25R95_RFal_GetObsvMode(uint8_t *txMode, uint8_t *rxMode);
void ST25R95_RFal_DisableObsvMode(void);

/* -------------------------------------------------------------------------- */
/* Callback */
/* -------------------------------------------------------------------------- */
void ST25R95_RFal_SetUpperLayerCallback(ST25R95_UpperLayerCallback pFunc);
void ST25R95_RFal_SetPreTxRxCallback(ST25R95_PreTxRxCallback pFunc);
void ST25R95_RFal_SetSyncTxRxCallback(ST25R95_SyncTxRxCallback pFunc);
void ST25R95_RFal_SetPostTxRxCallback(ST25R95_PostTxRxCallback pFunc);
void ST25R95_RFal_SetLmEonCallback(ST25R95_LmEonCallback pFunc);

/* -------------------------------------------------------------------------- */
/* Mode / Bitrate / Timing / Field */
/* -------------------------------------------------------------------------- */
ST25R95_OpResult ST25R95_RFal_SetMode(ST25R95_Mode mode, ST25R95_BitRate txBR, ST25R95_BitRate rxBR);
ST25R95_Mode ST25R95_RFal_GetMode(void);
ST25R95_OpResult ST25R95_RFal_SetBitRate(ST25R95_BitRate txBR, ST25R95_BitRate rxBR);
ST25R95_OpResult ST25R95_RFal_GetBitRate(ST25R95_BitRate *txBR, ST25R95_BitRate *rxBR);
void ST25R95_RFal_SetErrorHandling(ST25R95_EHandling eHandling);
ST25R95_EHandling ST25R95_RFal_GetErrorHandling(void);
void ST25R95_RFal_SetFDTPoll(uint32_t FDTPoll);
uint32_t ST25R95_RFal_GetFDTPoll(void);
void ST25R95_RFal_SetFDTListen(uint32_t FDTListen);
uint32_t ST25R95_RFal_GetFDTListen(void);
void ST25R95_RFal_SetGT(uint32_t GT);
uint32_t ST25R95_RFal_GetGT(void);
bool ST25R95_RFal_IsGTExpired(void);
ST25R95_OpResult ST25R95_RFal_FieldOnAndStartGT(void);
ST25R95_OpResult ST25R95_RFal_FieldOff(void);
bool ST25R95_RFal_IsExtFieldOn(void);

/* -------------------------------------------------------------------------- */
/* Transceive */
/* -------------------------------------------------------------------------- */
ST25R95_OpResult ST25R95_RFal_StartTransceive(const ST25R95_TransceiveContext *ctx);
bool ST25R95_RFal_IsTransceiveInTx(void);
bool ST25R95_RFal_IsTransceiveInRx(void);
ST25R95_OpResult ST25R95_RFal_TransceiveBlockingTx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt);
ST25R95_OpResult ST25R95_RFal_TransceiveBlockingRx(void);
ST25R95_OpResult ST25R95_RFal_TransceiveBlockingTxRx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt);
ST25R95_TransceiveState ST25R95_RFal_GetTransceiveState(void);
ST25R95_OpResult ST25R95_RFal_GetTransceiveStatus(void);
ST25R95_OpResult ST25R95_RFal_GetTransceiveRSSI(uint16_t *rssi);
bool ST25R95_RFal_IsTransceiveSubcDetected(void);
void ST25R95_RFal_Worker(void);
void ST25R95_RFal_TransceiveTx(void);
void ST25R95_RFal_TransceiveRx(void);
ST25R95_OpResult ST25R95_RFal_TransceiveRunBlockingTx(void);

/* -------------------------------------------------------------------------- */
/* NFCA */
/* -------------------------------------------------------------------------- */
ST25R95_OpResult ST25R95_RFal_ISO14443ATransceiveShortFrame(ST25R95_14443AShortFrameCmd txCmd, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *rxRcvdLen, uint32_t fwt);
ST25R95_OpResult ST25R95_RFal_ISO14443ATransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt);
ST25R95_OpResult ST25R95_RFal_ISO14443AStartTransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt);
ST25R95_OpResult ST25R95_RFal_ISO14443AGetTransceiveAnticollisionFrameStatus(void);

/* -------------------------------------------------------------------------- */
/* NFCV */
/* -------------------------------------------------------------------------- */
ST25R95_OpResult ST25R95_RFal_ISO15693TransceiveAnticollisionFrame(uint8_t *txBuf, uint8_t txBufLen, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen);
ST25R95_OpResult ST25R95_RFal_ISO15693TransceiveEOFAnticollision(uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen);
ST25R95_OpResult ST25R95_RFal_ISO15693TransceiveEOF(uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen);

/* -------------------------------------------------------------------------- */
/* NFCF / FeliCa */
/* -------------------------------------------------------------------------- */
ST25R95_OpResult ST25R95_RFal_FeliCaPoll(ST25R95_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, ST25R95_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected);
ST25R95_OpResult ST25R95_RFal_StartFeliCaPoll(ST25R95_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, ST25R95_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected);
ST25R95_OpResult ST25R95_RFal_GetFeliCaPollStatus(void);

/* -------------------------------------------------------------------------- */
/* Listen Mode */
/* -------------------------------------------------------------------------- */
ST25R95_OpResult ST25R95_RFal_ListenStart(uint32_t lmMask, const ST25R95_LmConfPA *confA, const ST25R95_LmConfPB *confB, const ST25R95_LmConfPF *confF, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen);
ST25R95_OpResult ST25R95_RFal_ListenStop(void);
ST25R95_OpResult ST25R95_RFal_ListenSleepStart(ST25R95_LmState sleepSt, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen);
ST25R95_LmState ST25R95_RFal_ListenGetState(bool *dataFlag, ST25R95_BitRate *lastBR);
ST25R95_OpResult ST25R95_RFal_ListenSetState(ST25R95_LmState newSt);

/* -------------------------------------------------------------------------- */
/* Wake-Up Mode */
/* -------------------------------------------------------------------------- */
ST25R95_OpResult ST25R95_RFal_WakeUpModeStart(const ST25R95_WakeUpConfig *config);
bool ST25R95_RFal_WakeUpModeIsEnabled(void);
ST25R95_OpResult ST25R95_RFal_WakeUpModeGetInfo(bool force, ST25R95_WakeUpInfo *info);
bool ST25R95_RFal_WakeUpModeHasWoke(void);
ST25R95_OpResult ST25R95_RFal_WakeUpModeStop(void);

/* -------------------------------------------------------------------------- */
/* RF Chip */
/* -------------------------------------------------------------------------- */
bool ST25R95_RFal_ChipIsBusy(void);
ST25R95_OpResult ST25R95_RFal_ChipWriteReg(uint16_t reg, const uint8_t *values, uint8_t len);
ST25R95_OpResult ST25R95_RFal_ChipReadReg(uint16_t reg, uint8_t *values, uint8_t len);
ST25R95_OpResult ST25R95_RFal_ChipExecCmd(uint16_t cmd);
ST25R95_OpResult ST25R95_RFal_ChipWriteTestReg(uint16_t reg, uint8_t value);
ST25R95_OpResult ST25R95_RFal_ChipReadTestReg(uint16_t reg, uint8_t *value);
ST25R95_OpResult ST25R95_RFal_ChipChangeRegBits(uint16_t reg, uint8_t valueMask, uint8_t value);
ST25R95_OpResult ST25R95_RFal_ChipChangeTestRegBits(uint16_t reg, uint8_t valueMask, uint8_t value);
ST25R95_OpResult ST25R95_RFal_ChipSetRFO(uint8_t rfo);
ST25R95_OpResult ST25R95_RFal_ChipGetRFO(uint8_t *result);
ST25R95_OpResult ST25R95_RFal_ChipMeasureAmplitude(uint8_t *result);
ST25R95_OpResult ST25R95_RFal_ChipMeasurePhase(uint8_t *result);
ST25R95_OpResult ST25R95_RFal_ChipMeasureCapacitance(uint8_t *result);
ST25R95_OpResult ST25R95_RFal_ChipMeasurePowerSupply(uint8_t param, uint8_t *result);

#ifdef __cplusplus
}
#endif

#endif /* RFST25R95_RFAL_H */
