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

#ifndef RFAL_RFST25R3916_H
#define RFAL_RFST25R3916_H

#include <stdbool.h>
#include <stdint.h>

#include "ST25R3916_Def.h"

#include "NFC/RFal/RFal.h"
#include "NFC/RFal/RFal_RF.h"

#include "NFC_Config.h"

#if defined(CONFIG_NFC_READER_DEVICE_ST25R3916) || defined(CONFIG_NFC_READER_DEVICE_ST25R3916B)

#define ST25R3916_FIFO_STATUS_LEN                           2        /*!< Number of FIFO Status Register                       */

#define RFAL_FIFO_IN_WL                 200U                                          /*!< Number of bytes in the FIFO when WL interrupt occurs while Tx                   */
#define RFAL_FIFO_OUT_WL                (ST25R3916_FIFO_DEPTH - RFAL_FIFO_IN_WL)      /*!< Number of bytes sent/out of the FIFO when WL interrupt occurs while Tx          */

#define RFAL_FIFO_STATUS_REG1           0U                                            /*!< Location of FIFO status register 1 in local copy                                */
#define RFAL_FIFO_STATUS_REG2           1U                                            /*!< Location of FIFO status register 2 in local copy                                */
#define RFAL_FIFO_STATUS_INVALID        0xFFU                                         /*!< Value indicating that the local FIFO status in invalid|cleared                  */

#define RFAL_ST25R3916_GPT_MAX_1FC      RFal_Conv8fcTo1fc(  0xFFFFU )                  /*!< Max GPT steps in 1fc (0xFFFF steps of 8/fc    => 0xFFFF * 590ns  = 38,7ms)      */
#define RFAL_ST25R3916_NRT_MAX_1FC      RFal_Conv4096fcTo1fc( 0xFFFFU )                /*!< Max NRT steps in 1fc (0xFFFF steps of 4096/fc => 0xFFFF * 302us  = 19.8s )      */
#define RFAL_ST25R3916_NRT_DISABLED     0U                                            /*!< NRT Disabled: All 0 No-response timer is not started, wait forever              */
#define RFAL_ST25R3916_MRT_MAX_1FC      RFal_Conv64fcTo1fc( 0x00FFU )                  /*!< Max MRT steps in 1fc (0x00FF steps of 64/fc   => 0x00FF * 4.72us = 1.2ms )      */
#define RFAL_ST25R3916_MRT_MIN_1FC      RFal_Conv64fcTo1fc( 0x0004U )                  /*!< Min MRT steps in 1fc ( 0<=mrt<=4 ; 4 (64/fc)  => 0x0004 * 4.72us = 18.88us )    */
#define RFAL_ST25R3916_GT_MAX_1FC       RFal_ConvMsTo1fc( 6000U )                      /*!< Max GT value allowed in 1/fc (SFGI=14 => SFGT + dSFGT = 5.4s)                   */
#define RFAL_ST25R3916_GT_MIN_1FC       RFal_ConvMsTo1fc(RFAL_ST25R3916_SW_TMR_MIN_1MS)/*!< Min GT value allowed in 1/fc                                                    */
#define RFAL_ST25R3916_SW_TMR_MIN_1MS   1U                                            /*!< Min value of a SW timer in ms                                                   */

#define RFAL_OBSMODE_DISABLE            0x00U                                         /*!< Observation Mode disabled                                                       */

#define RFAL_RX_INC_BYTE_LEN            (uint8_t)1U                                   /*!< Threshold where incoming rx shall be considered incomplete byte NFC - T2T       */
#define RFAL_EMVCO_RX_MAXLEN            (uint8_t)4U                                   /*!< Maximum value where EMVCo to apply special error handling                       */

#define RFAL_NORXE_TOUT                 50U                                           /*!< Timeout to be used on a potential missing RXE - Silicon ST25R3916 Errata #2.1.2 */

#define RFAL_ISO14443A_SDD_RES_LEN      5U                                            /*!< SDD_RES | Anticollision (UID CLn) length  -  RFal_NfcaSddRes                     */
#define RFAL_ISO14443A_CRC_INTVAL       0x6363                                        /*!< ISO14443 CRC Initial Value|Register                                             */


#define RFAL_FELICA_POLL_DELAY_TIME     512U                                          /*!<  FeliCa Poll Processing time is 2.417 ms ~512*64/fc Digital 1.1 A4              */
#define RFAL_FELICA_POLL_SLOT_TIME      256U                                          /*!<  FeliCa Poll Time Slot duration is 1.208 ms ~256*64/fc Digital 1.1 A4           */

#define RFAL_LM_SENSF_RD0_POS           17U                                           /*!<  FeliCa SENSF_RES Request Data RD0 position                                     */
#define RFAL_LM_SENSF_RD1_POS           18U                                           /*!<  FeliCa SENSF_RES Request Data RD1 position                                     */

#define RFAL_LM_NFCID_INCOMPLETE        0x04U                                         /*!<  NFCA NFCID not complete bit in SEL_RES (SAK)                                   */

#define RFAL_ISO15693_IGNORE_BITS       RFal_ConvBytesToBits(2U)                       /*!< Ignore collisions before the UID (RES_FLAG + DSFID)                             */
#define RFAL_ISO15693_INV_RES_LEN       12U                                           /*!< ISO15693 Inventory response length with CRC (bytes)                             */
#define RFAL_ISO15693_INV_RES_DUR       4U                                            /*!< ISO15693 Inventory response duration @ 26 kbps (ms)                             */

#define RFAL_WU_MIN_WEIGHT_VAL          4U                                            /*!< ST25R3916 minimum Wake-up weight value                                         */

/*******************************************************************************/

#define RFAL_LM_GT                      RFal_ConvUsTo1fc(100U)                         /*!< Listen Mode Guard Time enforced (GT - Passive; TIRFG - Active)                  */
#define RFAL_FDT_POLL_ADJUSTMENT        RFal_ConvUsTo1fc(80U)                          /*!< FDT Poll adjustment: Time between the expiration of GPT to the actual Tx        */
#define RFAL_FDT_LISTEN_MRT_ADJUSTMENT  64U                                           /*!< MRT jitter adjustment: timeout will be between [ tout ; tout + 64 cycles ]      */
#define RFAL_AP2P_FIELDOFF_TCMDOFF      1356U                                         /*!< Time after TXE and Field Off t,CMD,OFF     Activity 2.1  3.2.1.3 & C            */

#ifndef RFAL_ST25R3916_AAT_SETTLE
  #define RFAL_ST25R3916_AAT_SETTLE   5U                                            /*!< Time in ms required for AAT pins and Osc to settle after en bit set             */
#endif /* RFAL_ST25R3916_AAT_SETTLE */

#ifndef RFAL_ST25R3916B_AAT_SETTLE
  #define RFAL_ST25R3916B_AAT_SETTLE  ST25R3916_REG_MEAS_TX_DELAY_meas_tx_del_4_83ms/*!< Time between Oscillator stable and TX On in meas_tx_del steps                    */
#endif /* RFAL_ST25R3916B_AAT_SETTLE */

/*! FWT adjustment:
 *    64 : NRT jitter between TXE and NRT start      */
#define RFAL_FWT_ADJUSTMENT             64U

/*! FWT ISO14443A adjustment:
 *   512  : 4bit length
 *    64  : Half a bit duration due to ST25R3916 Coherent receiver (1/fc)         */
#define RFAL_FWT_A_ADJUSTMENT           (512U + 64U)

/*! FWT ISO14443B adjustment:
 *    SOF (14etu) + 1Byte (10etu) + 1etu (IRQ comes 1etu after first byte) - 3etu (ST25R3916 sends TXE 3etu after) */
#define RFAL_FWT_B_ADJUSTMENT           (((14U + 10U + 1U) - 3U) * 128U)


/*! FWT FeliCa 212 adjustment:
 *    1024 : Length of the two Sync bytes at 212kbps */
#define RFAL_FWT_F_212_ADJUSTMENT       1024U

/*! FWT FeliCa 424 adjustment:
 *    512 : Length of the two Sync bytes at 424kbps  */
#define RFAL_FWT_F_424_ADJUSTMENT       512U


/*! Time between our field Off and other peer field On : Tadt + (n x Trfw)
 * Ecma 340 11.1.2 - Tadt: [56.64 , 188.72] us ;  n: [0 , 3]  ; Trfw = 37.76 us
 * Should be: 189 + (3*38) = 303us ; we'll use a more relaxed setting: 605 us    */
#define RFAL_AP2P_FIELDON_TADTTRFW      RFal_ConvUsTo1fc(605U)


/*! FDT Listen adjustment for ISO14443A   EMVCo 2.6  4.8.1.3  ;  Digital 1.1  6.10
 *
 *  276: Time from the rising pulse of the pause of the logic '1' (i.e. the time point to measure the deaftime from),
 *       to the actual end of the EOF sequence (the point where the MRT starts). Please note that the ST25R391x uses the
 *       ISO14443-2 definition where the EOF consists of logic '0' followed by sequence Y.
 *  -64: Further adjustment for receiver to be ready just before first bit
 */
#define RFAL_FDT_LISTEN_A_ADJUSTMENT    (276U-64U)


/*! FDT Listen adjustment for ISO14443B   EMVCo 2.6  4.8.1.6  ;  Digital 1.1  7.9
 *
 *  340: Time from the rising edge of the EoS to the starting point of the MRT timer (sometime after the final high
 *       part of the EoS is completed)
 */
#define RFAL_FDT_LISTEN_B_ADJUSTMENT    340U


/*! FDT Listen adjustment for ISO15693
 * ISO15693 2000  8.4  t1 MIN = 4192/fc
 * ISO15693 2009  9.1  t1 MIN = 4320/fc
 * Digital 2.1 B.5 FDTV,LISTEN,MIN  = 4310/fc
 * Set FDT Listen one step earlier than on the more recent spec versions for greater interoperability
 */
#define RFAL_FDT_LISTEN_V_ADJUSTMENT    64U


/*! FDT Poll adjustment for ISO14443B Correlator - sst 5 etu */
#define RFAL_FDT_LISTEN_B_ADJT_CORR     128U


/*! FDT Poll adjustment for ISO14443B Correlator sst window - 5 etu */
#define RFAL_FDT_LISTEN_B_ADJT_CORR_SST 20U

/*! Calculates Transceive Sanity Timer. It accounts for the slowest bit rate and the longest data format
 *    1s for transmission and reception of a 4K message at 106kpbs (~425ms each direction)
 *       plus TxRx preparation and FIFO load over Serial Interface                                      */
#define RFal_CalcSanityTmr( fwt )                 (uint16_t)(1000U + RFal_Conv1fcToMs((fwt)))

#define RFal_GennTRFW( n )                        ((uint8_t)(((n)+1U)%7U))                                          /*!< Generate next n*TRFW used for RFCA: modulo a prime to avoid alias effects */

#define RFal_CalcNumBytes( nBits )                (((uint32_t)(nBits) + 7U) / 8U)                                   /*!< Returns the number of bytes required to fit given the number of bits */

#define RFal_TimerStart( timer, time_ms )         (timer) = timerCalculateTimer((uint16_t)(time_ms))                /*!< Configures and starts the RTOX timer                                 */
#define RFal_TimerisExpired( timer )              timerIsExpired( timer )                                   /*!< Checks if timer has expired                                          */

#define RFal_ST25R3916ObsModeDisable()            ST25R3916_IO_WriteTestRegister(0x01U, (0x40U))                        /*!< Disable ST25R3916 Observation mode                                   */
#define RFal_ST25R3916ObsModeTx()                 ST25R3916_IO_WriteTestRegister(0x01U, (0x40U|gRFAL.conf.obsvModeTx))  /*!< Enable Tx Observation mode                                           */
#define RFal_ST25R3916ObsModeRx()                 ST25R3916_IO_WriteTestRegister(0x01U, (0x40U|gRFAL.conf.obsvModeRx))  /*!< Enable Rx Observation mode                                           */

#define RFal_CheckDisableObsMode()                if(gRFAL.conf.obsvModeRx != 0U){ RFal_ST25R3916ObsModeDisable(); } /*!< Checks if the observation mode is enabled, and applies on ST25R3916  */
#define RFal_CheckEnableObsModeTx()               if(gRFAL.conf.obsvModeTx != 0U){ RFal_ST25R3916ObsModeTx(); }      /*!< Checks if the observation mode is enabled, and applies on ST25R3916  */
#define RFal_CheckEnableObsModeRx()               if(gRFAL.conf.obsvModeRx != 0U){ RFal_ST25R3916ObsModeRx(); }      /*!< Checks if the observation mode is enabled, and applies on ST25R3916  */

#define RFal_GetIncmplBits( FIFOStatus2 )         (( (FIFOStatus2) >> 1) & 0x07U)                                           /*!< Returns the number of bits from fifo status                  */
#define RFal_IsIncompleteByteError( error )       (((error) >= ERR_INCOMPLETE_BYTE) && ((error) <= ERR_INCOMPLETE_BYTE_07)) /*!< Checks if given error is a Incomplete error                  */

#define RFal_AdjACBR( b )                         (((uint16_t)(b) >= (uint16_t)RFAL_BR_52p97) ? (uint16_t)(b) : ((uint16_t)(b)+1U))          /*!< Adjusts ST25R391x Bit rate to Analog Configuration              */
#define RFal_ConvBR2ACBR( b )                     (((RFal_AdjACBR((b)))<<RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_SHIFT) & RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_MASK) /*!< Converts ST25R391x Bit rate to Analog Configuration bit rate id */

#define RFal_ConvTDFormat( v )                    ((uint16_t)(v) << 8U) /*!< Converts a uint8_t to the format used in SW Tag Detection */
#define RFal_AddFracTDFormat( fd )                ((((uint16_t)(fd)) & 0x03U) * 64U)

#define RFal_RunBlocking( e, fn )                 do{ (e)=(fn); RFal_Worker(); }while( (e) == ERR_BUSY )

/*! Struct that holds all involved on a Transceive including the context passed by the caller     */
typedef struct {
  RFal_TransceiveState     state;       /*!< Current transceive state                            */
  RFal_TransceiveState     lastState;   /*!< Last transceive state (debug purposes)              */
  NFC_OpResult             status;      /*!< Current status/error of the transceive              */

  RFal_TransceiveContext   ctx;         /*!< The transceive context given by the caller          */
} RFal_TxRx;

/*! Struct that holds certain WU mode information to be retrieved by RFal_WakeUpModeGetInfo        */
typedef struct {
  bool                 irqWut;     /*!< Wake-Up Timer IRQ received (cleared upon read)          */

  struct {
    uint8_t          lastMeas;   /*!< Value of the latest measurement                         */
    bool             irqWu;      /*!< Amplitude WU IRQ received (cleared upon read)           */
  } indAmp;                        /*!< Inductive Amplitude                                     */
  struct {
    uint8_t          lastMeas;   /*!< Value of the latest measurement                         */
    bool             irqWu;      /*!< Phase WU IRQ received (cleared upon read)               */
  } indPha;                        /*!< Inductive Phase                                         */
  struct {
    uint8_t          lastMeas;   /*!< Value of the latest measurement                         */
    bool             irqWu;      /*!< Capacitive WU IRQ received (cleared upon read)          */
  } cap;                           /*!< Capacitance                                             */
} RFal_WakeUpData;

/*! Local struct that holds context for the Listen Mode                                          */
typedef struct {
  RFal_LmState             state;       /*!< Current Listen Mode state                           */
  uint32_t                 mdMask;      /*!< Listen Mode mask used                               */
  uint32_t                 mdReg;       /*!< Listen Mode register value used                     */
  uint32_t                 mdIrqs;      /*!< Listen Mode IRQs used                               */
  RFal_BitRate             brDetected;  /*!< Last bit rate detected                              */

  uint8_t                *rxBuf;       /*!< Location to store incoming data in Listen Mode      */
  uint16_t                rxBufLen;    /*!< Length of rxBuf                                     */
  uint16_t               *rxLen;       /*!< Pointer to write the data length placed into rxBuf  */
  bool                    dataFlag;    /*!< Listen Mode current Data Flag                       */
  bool                    iniFlag;     /*!< Listen Mode initialized Flag  (FeliCa slots)        */
} RFal_Lm;

/*! Struct that holds all context for the Wake-Up Mode                                            */
typedef struct {
  RFal_WumState            state;       /*!< Current Wake-Up Mode state                          */
  RFal_WakeUpConfig        cfg;         /*!< Current Wake-Up Mode config                         */
  RFal_WakeUpData          info;        /*!< Current Wake-Up Mode info                           */
} RFal_Wum;

/*! Struct that holds all context for the Low Power Mode                                          */
typedef struct {
  bool                    isRunning;
} RFal_Lpm;

/*! Struct that holds the timings GT and FDTs                           */
typedef struct {
  uint32_t                GT;          /*!< GT in 1/fc                */
  uint32_t                FDTListen;   /*!< FDTListen in 1/fc         */
  uint32_t                FDTPoll;     /*!< FDTPoll in 1/fc           */
  uint8_t                 nTRFW;       /*!< n*TRFW (last two bits) used during RF CA  */
} RFal_Timings;

/*! Struct that holds the software timers                               */
typedef struct {
  uint32_t                GT;          /*!< RFAL's GT timer           */
  uint32_t                RXE;         /*!< Timer between RXS - RXE   */
  uint32_t                PPON2;       /*!< Timer between TXE - PPON2 */
  uint32_t                txRx;        /*!< Transceive sanity timer   */
} RFal_Timers;

/*! Struct that holds the RFAL's callbacks                              */
typedef struct {
  RFal_PreTxRxCallback     preTxRx;     /*!< RFAL's Pre TxRx callback  */
  RFal_PostTxRxCallback    postTxRx;    /*!< RFAL's Post TxRx callback */
  RFal_SyncTxRxCallback    syncTxRx;    /*!< RFAL's Sync TxRx callback */
} RFal_Callbacks;

/*! Struct that holds counters to control the FIFO on Tx and Rx                                                                          */
typedef struct {
  uint16_t                expWL;       /*!< The amount of bytes expected to be Tx when a WL interrupt occurs                          */
  uint16_t                bytesTotal;  /*!< Total bytes to be transmitted OR the total bytes received                                  */
  uint16_t                bytesWritten;/*!< Amount of bytes already written on FIFO (Tx) OR read (RX) from FIFO and written on rxBuffer*/
  uint8_t                 status[ST25R3916_FIFO_STATUS_LEN];   /*!< FIFO Status Registers                                              */
} RFal_FIFO;

/*! Struct that holds RFAL's configuration settings                                                      */
typedef struct {
  uint8_t                 obsvModeTx;  /*!< RFAL's config of the ST25R3916's observation mode while Tx */
  uint8_t                 obsvModeRx;  /*!< RFAL's config of the ST25R3916's observation mode while Rx */
  RFal_EHandling           eHandling;   /*!< RFAL's error handling config/mode                          */
} RFal_Configs;

/*! Struct that holds NFC-A data - Used only inside RFal_ISO14443ATransceiveAnticollisionFrame()          */
typedef struct {
  uint8_t                 collByte;    /*!< NFC-A Anticollision collision byte                         */
  uint8_t                 *buf;        /*!< NFC-A Anticollision frame buffer                           */
  uint8_t                 *bytesToSend;/*!< NFC-A Anticollision NFCID|UID byte context                 */
  uint8_t                 *bitsToSend; /*!< NFC-A Anticollision NFCID|UID bit context                  */
  uint16_t                *rxLength;   /*!< NFC-A Anticollision received length                        */
} RFal_NfcaWorkingData;

/*! Struct that holds NFC-F data - Used only inside RFal_FelicaPoll()                                           */
typedef struct {
  uint16_t           actLen;                                      /* Received length                         */
  RFal_FeliCaPollRes *pollResList;                                 /* Location of NFC-F device list           */
  uint8_t            pollResListSize;                             /* Size of NFC-F device list               */
  uint8_t            devDetected;                                 /* Number of devices detected              */
  uint8_t            colDetected;                                 /* Number of collisions detected           */
  uint8_t            *devicesDetected;                            /* Location to place number of devices     */
  uint8_t            *collisionsDetected;                         /* Location to place number of collisions  */
  RFal_EHandling      curHandling;                                 /* RFAL's error handling                   */
  RFal_FeliCaPollRes  pollResponses[RFAL_FELICA_POLL_MAX_SLOTS];   /* FeliCa Poll response buffer (16 slots)  */
} RFal_NfcfWorkingData;

/*! Struct that holds NFC-V current context
 *
 * This buffer has to be big enough for coping with maximum response size (hamming coded)
 *    - inventory requests responses: 14*2+2 bytes
 *    - read single block responses: (32+4)*2+2 bytes
 *    - read multiple block could be very long... -> not supported
 *    - current implementation expects it be written in one bulk into FIFO
 *    - needs to be above FIFO water level of ST25R3916 (200)
 *    - the coding function needs to be able to
 *      put more than FIFO water level bytes into it (n*64+1)>200                                                          */
typedef struct {
  uint8_t                 codingBuffer[((2 + 255 + 3) * 2)]; /*!< Coding buffer,   length MUST be above 257: [257; ...]    */
  uint16_t                nfcvOffset;        /*!< Offset needed for ISO15693 coding function                             */
  RFal_TransceiveContext   origCtx;           /*!< context provided by user                                               */
  uint16_t                ignoreBits;        /*!< Number of bits at the beginning of a frame to be ignored when decoding */
} RFal_NfcvWorkingData;

/*! RFAL instance  */
typedef struct {
  RFal_State               state;     /*!< RFAL's current state                          */
  RFal_Mode                mode;      /*!< RFAL's current mode                           */
  RFal_BitRate             txBR;      /*!< RFAL's current Tx Bit Rate                    */
  RFal_BitRate             rxBR;      /*!< RFAL's current Rx Bit Rate                    */
  bool                    field;     /*!< Current field state (On / Off)                */

  RFal_Configs             conf;      /*!< RFAL's configuration settings                 */
  RFal_Timings             timings;   /*!< RFAL's timing setting                         */
  RFal_TxRx                TxRx;      /*!< RFAL's transceive management                  */
  RFal_FIFO                fifo;      /*!< RFAL's FIFO management                        */
  RFal_Timers              tmr;       /*!< RFAL's Software timers                        */
  RFal_Callbacks           callbacks; /*!< RFAL's callbacks                              */

  RFal_Lm                  Lm;        /*!< RFAL's listen mode management                 */
  RFal_Wum                 wum;       /*!< RFAL's Wake-up mode management                */
  RFal_Lpm                 lpm;       /*!< RFAL's Low power mode management              */
  RFal_NfcaWorkingData     nfcaData;  /*!< RFAL's working data when supporting NFC-A     */
  RFal_NfcfWorkingData     nfcfData;  /*!< RFAL's working data when supporting NFC-F     */
  RFal_NfcvWorkingData     nfcvData;  /*!< RFAL's working data when performing NFC-V     */
} RFal;

/*! Felica's command set */
typedef enum {
  FELICA_CMD_POLLING                  = 0x00, /*!< Felica Poll/REQC command (aka SENSF_REQ) to identify a card    */
  FELICA_CMD_POLLING_RES              = 0x01, /*!< Felica Poll/REQC command (aka SENSF_RES) response              */
  FELICA_CMD_REQUEST_SERVICE          = 0x02, /*!< verify the existence of Area and Service                       */
  FELICA_CMD_REQUEST_RESPONSE         = 0x04, /*!< verify the existence of a card                                 */
  FELICA_CMD_READ_WITHOUT_ENCRYPTION  = 0x06, /*!< read Block Data from a Service that requires no authentication */
  FELICA_CMD_WRITE_WITHOUT_ENCRYPTION = 0x08, /*!< write Block Data to a Service that requires no authentication  */
  FELICA_CMD_REQUEST_SYSTEM_CODE      = 0x0C, /*!< acquire the System Code registered to a card                   */
  FELICA_CMD_AUTHENTICATION1          = 0x10, /*!< authenticate a card                                            */
  FELICA_CMD_AUTHENTICATION2          = 0x12, /*!< allow a card to authenticate a Reader/Writer                   */
  FELICA_CMD_READ                     = 0x14, /*!< read Block Data from a Service that requires authentication    */
  FELICA_CMD_WRITE                    = 0x16, /*!< write Block Data to a Service that requires authentication     */
} t_RFal_FeliCaCmd;

/*! Union representing all PTMem sections */
typedef union { /*  PRQA S 0750 # MISRA 19.2 - Both members are of the same type, just different names.  Thus no problem can occur. */
  uint8_t PTMem_A[ST25R3916_PTM_A_LEN];       /*!< PT_Memory area allocated for NFC-A configuration               */
  uint8_t PTMem_F[ST25R3916_PTM_F_LEN];       /*!< PT_Memory area allocated for NFC-F configuration               */
  uint8_t TSN[ST25R3916_PTM_TSN_LEN];         /*!< PT_Memory area allocated for TSN - Random numbers              */
} t_RFal_PTMem;

#ifdef __cplusplus
extern "C" {
#endif
//ReturnCode RFal_Iso15693PhyVCDCode1Of4(const uint8_t data, uint8_t *outbuffer, uint16_t maxOutBufLen, uint16_t *outBufLen);
//ReturnCode RFal_Iso15693PhyVCDCode1Of256(const uint8_t data, uint8_t *outbuffer, uint16_t maxOutBufLen, uint16_t *outBufLen);
#ifdef __cplusplus
}
#endif

#endif //CONFIG_NFC_READER_DEVICE_ST25R3916 || CONFIG_NFC_READER_DEVICE_ST25R3916B

#endif /* RFAL_RFST25R3916_H */
