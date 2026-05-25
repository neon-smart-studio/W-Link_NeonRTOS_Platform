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

#include <stdbool.h>
#include <stdint.h>

#include "ST25R95_Def.h"

#include "NFC/RFal/RFal_RF.h"

#include "NFC_Config.h"

#ifdef CONFIG_NFC_READER_DEVICE_ST25R95

#define RFAL_ST25R95_GPT_MAX_1FC         RFal_Conv8fcTo1fc(0xFFFF)                     /*!< Max GPT steps in 1fc (0xFFFF steps of 8/fc    => 0xFFFF * 590ns  = 38,7ms)      */
#define RFAL_ST25R95_NRT_MAX_1FC         RFal_Conv4096fcTo1fc(0xFFFF)                  /*!< Max NRT steps in 1fc (0xFFFF steps of 4096/fc => 0xFFFF * 302us  = 19.8s)       */
#define RFAL_ST25R95_NRT_DISABLED        0                                            /*!< NRT Disabled: All 0 No-response timer is not started, wait forever              */
#define RFAL_ST25R95_MRT_MAX_1FC         RFal_Conv64fcTo1fc(0x00FF)                    /*!< Max MRT steps in 1fc (0x00FF steps of 64/fc   => 0x00FF * 4.72us = 1.2ms)       */
#define RFAL_ST25R95_MRT_MIN_1FC         RFal_Conv64fcTo1fc(0x0004)                    /*!< Min MRT steps in 1fc (0<=mrt<=4 ; 4 (64/fc)  => 0x0004 * 4.72us = 18.88us)      */
#define RFAL_ST25R95_GT_MAX_1FC          RFal_ConvMsTo1fc(5000)                        /*!< Max GT value allowed in 1/fc                                                    */
#define RFAL_ST25R95_GT_MIN_1FC          RFal_ConvMsTo1fc(RFAL_ST25R95_SW_TMR_MIN_1MS) /*!< Min GT value allowed in 1/fc                                                    */
#define RFAL_ST25R95_SW_TMR_MIN_1MS      1

#define RFAL_FELICA_POLL_DELAY_TIME     512                                           /*!<  FeliCa Poll Processing time is 2.417 ms ~512*64/fc Digital 1.1 A4              */
#define RFAL_FELICA_POLL_SLOT_TIME      256                                           /*!<  FeliCa Poll Time Slot duration is 1.208 ms ~256*64/fc Digital 1.1 A4           */

#define RFAL_ISO14443A_SDD_RES_LEN      5                                             /*!< SDD_RES | Anticollision (UID CLn) length  -  RFal_NfcaSddRes                     */

#define RFAL_ST25R95_ISO14443A_APPENDCRC                                             0x20U /*!< Transmission flags bit 5: Append CRC        */
#define RFAL_ST25R95_ISO14443A_SPLITFRAME                                            0x40U /*!< Transmission flags bit 6: SplitFrame        */
#define RFAL_ST25R95_ISO14443A_TOPAZFORMAT                                           0x80U /*!< Transmission flags bit 7: Topaz send format */

#define RFAL_ST25R95_IDLE_DEFAULT_WUPERIOD                                           0x24U /*!< Fixed WU Period to reach ~300 ms timeout with Max Sleep = 0 */

#define ST25R95_TAGDETECT_DEF_CALIBRATION 0x7C             /*!< Tag Detection Calibration default value                    */

#define RFal_TimerStart(timer, time_ms)         (timer) = timerCalculateTimer((uint16_t)(time_ms)) /*!< Configures and starts the RTOX timer          */
#define RFal_TimerisExpired(timer)              timerIsExpired(timer)          /*!< Checks if timer has expired                   */

/*! Struct that holds all involved on a Transceive including the context passed by the caller     */
typedef struct {
  RFal_TransceiveState     state;       /*!< Current transceive state                            */
  RFal_TransceiveState     lastState;   /*!< Last transceive state (debug purposes)              */
  NFC_OpResult              status;      /*!< Current status/error of the transceive              */

  RFal_TransceiveContext   ctx;         /*!< The transceive context given by the caller          */
} RFal_TxRx;

/*! Struct that holds all context for the Listen Mode                                             */
typedef struct {
  uint8_t                *rxBuf;       /*!< Location to store incoming data in Listen Mode      */
  uint16_t                rxBufLen;    /*!< Length of rxBuf                                     */
  uint16_t               *rxLen;       /*!< Pointer to write the data length placed into rxBuf  */
  bool                    dataFlag;    /*!< Listen Mode current Data Flag                       */
} RFal_Lm;

/*! Struct that holds all context for the Wake-Up Mode                                             */
typedef struct {
  RFal_WumState            state;       /*!< Current Wake-Up Mode state                           */
  RFal_WakeUpConfig        cfg;         /*!< Current Wake-Up Mode context                         */
  uint8_t                 CalTagDet;   /*!< Tag Detection calibration value                      */
} RFal_Wum;

typedef struct {
  uint32_t                GT;          /*!< GT in 1/fc                  */
  uint32_t                FDTListen;   /*!< FDTListen in 1/fc           */
  uint32_t                FDTPoll;     /*!< FDTPoll in 1/fc             */
} RFal_Timings;

/*! Struct that holds the software timers                                 */
typedef struct {
  uint32_t                GT;          /*!< RFAL's GT timer             */
  uint32_t                FDTPoll;     /*!< RFAL's FST Poll timer       */
} RFal_Timers;

/*! Struct that holds the RFAL's callbacks                                */
typedef struct {
  RFal_PreTxRxCallback     preTxRx;     /*!< RFAL's Pre TxRx callback    */
  RFal_PostTxRxCallback    postTxRx;    /*!< RFAL's Post TxRx callback   */
} RFal_Callbacks;


/*! Struct that holds RFAL's configuration settings                                                      */
typedef struct {
  uint8_t                 obsvModeTx;  /*!< RFAL's config of the ST25R3911's observation mode while Tx */
  uint8_t                 obsvModeRx;  /*!< RFAL's config of the ST25R3911's observation mode while Rx */
  RFal_EHandling           eHandling;   /*!< RFAL's error handling config/mode                          */
} RFal_Configs;

/*! Struct that holds NFC-A data - Used only inside RFal_ISO14443ATransceiveAnticollisionFrame()          */
typedef struct {
  uint8_t                 collByte;    /*!< NFC-A Anticollision collision byte                         */
  uint8_t                 *buf;        /*!< NFC-A Anticollision frame buffer                           */
  uint8_t                 *bytesToSend;/*!< NFC-A Anticollision NFCID|UID byte context                 */
  uint8_t                 *bitsToSend; /*!< NFC-A Anticollision NFCID|UID bit context                  */
  uint16_t                *rxLength;   /*!< NFC-A Anticollision received length                        */
  bool                    NfcaSplitFrame;
} RFal_NFCA_WorkingData;

/*! Struct that holds NFC-F data - Used only inside RFal_FelicaPoll() (static to avoid adding it into stack) */
typedef struct {
  uint16_t           actLen;                                      /* Received length                         */
  RFal_FeliCaPollRes *pollResList;                                 /* Location of NFC-F device list           */
  uint8_t            pollResListSize;                             /* Size of NFC-F device list               */
  uint8_t            devDetected;                                 /* Number of devices detected              */
  uint8_t            colDetected;                                 /* Number of collisions detected           */
  uint8_t            *devicesDetected;                            /* Location to place number of devices     */
  uint8_t            *collisionsDetected;                         /* Location to place number of collisions  */
  RFal_EHandling      curHandling;                                 /* RFAL's error handling                   */
  RFal_FeliCaPollRes pollResponses[RFAL_FELICA_POLL_MAX_SLOTS];   /* FeliCa Poll response container for 16 slots */
  RFal_FeliCaPollSlots slots;
} RFal_NFCF_WorkingData;

typedef struct {
  RFal_State             state;     /*!< RFAL's current state                            */
  RFal_Mode              mode;      /*!< RFAL's current mode                             */
  RFal_BitRate           txBR;      /*!< RFAL's current Tx Bit Rate                      */
  RFal_BitRate           rxBR;      /*!< RFAL's current Rx Bit Rate                      */
  bool                  field;     /*!< Current field state (On / Off)                  */

  RFal_Configs           conf;      /*!< RFAL's configuration settings                 */
  RFal_Timings           timings;   /*!< RFAL's timing setting                           */
  RFal_TxRx              TxRx;      /*!< RFAL's transceive management                    */
  RFal_Lm                Lm;        /*!< RFAL's listen mode management                   */
  RFal_Wum               wum;       /*!< RFAL's Wake-Up mode management                  */

  RFal_Timers            tmr;       /*!< RFAL's Software timers                          */
  RFal_Callbacks         callbacks; /*!< RFAL's callbacks                                */

  uint8_t               protocol;  /*!< ProtocolSelect protocol                         */
  uint8_t               RxInformationBytes[3]; /*!< ST25R95 additional information bytes*/

  bool                  cardEmulT4AT;
  RFal_NFCA_WorkingData     nfcaData;  /*!< RFAL's working data when supporting NFC-A     */
  RFal_NFCF_WorkingData   nfcfData; /*!< RFAL's working data when supporting NFC-F        */
} RFal_ST25R95;

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
} t_RFal_FeliCaCmd;

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#endif //CONFIG_NFC_READER_DEVICE_ST25R95

#endif /* RFST25R95_RFAL_H */
