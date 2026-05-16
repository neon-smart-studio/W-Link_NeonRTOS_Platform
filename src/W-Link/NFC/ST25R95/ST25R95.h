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

#ifndef RFAL_RFST25R95_H
#define RFAL_RFST25R95_H

#include <stdbool.h>
#include <stdint.h>


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
} ST25R95_FeliCaCmd;

#if 0
/*! SPI transceive context definition */
typedef struct {
  bool rmvCRC;                         /*!< Remove CRC flag                 */
  bool inListen;                       /*!< inListen flags                  */
  bool NFCIP1;                         /*!< NFCIP1 flags                    */
  ST25R95_LmState LmState;                 /*!< LmState                         */
  uint16_t rxBufLen;                   /*!< rxBufLen                        */
  uint16_t *rxRcvdLen;                 /*!< rxRcvdLen                       */
  uint8_t *rxBuf;                      /*!< rxBuf                           */
  uint8_t *additionalRespBytes;        /*!< additionalRespBytes             */
  ReturnCode retCode;                  /*!< retCode                         */
  uint8_t BufCRC[2];                   /*!< BufCRC                          */
  uint8_t NFCIP1_SoD[1];               /*!< NFCIP1_SoD                      */
  uint8_t protocol;                    /*!< protocol                        */

} st25r95SPIRxContext;
 #endif

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

#define RFAL_ST25R95_GPT_MAX_1FC         ST25R95_Conv8fcTo1fc(0xFFFF)                     /*!< Max GPT steps in 1fc (0xFFFF steps of 8/fc    => 0xFFFF * 590ns  = 38,7ms)      */
#define RFAL_ST25R95_NRT_MAX_1FC         ST25R95_Conv4096fcTo1fc(0xFFFF)                  /*!< Max NRT steps in 1fc (0xFFFF steps of 4096/fc => 0xFFFF * 302us  = 19.8s)       */
#define RFAL_ST25R95_NRT_DISABLED        0                                            /*!< NRT Disabled: All 0 No-response timer is not started, wait forever              */
#define RFAL_ST25R95_MRT_MAX_1FC         ST25R95_Conv64fcTo1fc(0x00FF)                    /*!< Max MRT steps in 1fc (0x00FF steps of 64/fc   => 0x00FF * 4.72us = 1.2ms)       */
#define RFAL_ST25R95_MRT_MIN_1FC         ST25R95_Conv64fcTo1fc(0x0004)                    /*!< Min MRT steps in 1fc (0<=mrt<=4 ; 4 (64/fc)  => 0x0004 * 4.72us = 18.88us)      */
#define RFAL_ST25R95_GT_MAX_1FC          ST25R95_ConvMsTo1fc(5000)                        /*!< Max GT value allowed in 1/fc                                                    */
#define RFAL_ST25R95_GT_MIN_1FC          ST25R95_ConvMsTo1fc(RFAL_ST25R95_SW_TMR_MIN_1MS) /*!< Min GT value allowed in 1/fc                                                    */
#define RFAL_ST25R95_SW_TMR_MIN_1MS      1

#define RFAL_FELICA_POLL_DELAY_TIME     512                                           /*!<  FeliCa Poll Processing time is 2.417 ms ~512*64/fc Digital 1.1 A4              */
#define RFAL_FELICA_POLL_SLOT_TIME      256                                           /*!<  FeliCa Poll Time Slot duration is 1.208 ms ~256*64/fc Digital 1.1 A4           */

#define RFAL_ISO14443A_SDD_RES_LEN      5                                             /*!< SDD_RES | Anticollision (UID CLn) length  -  ST25R95_NfcaSddRes                     */

#define RFAL_ST25R95_ISO14443A_APPENDCRC                                             0x20U /*!< Transmission flags bit 5: Append CRC        */
#define RFAL_ST25R95_ISO14443A_SPLITFRAME                                            0x40U /*!< Transmission flags bit 6: SplitFrame        */
#define RFAL_ST25R95_ISO14443A_TOPAZFORMAT                                           0x80U /*!< Transmission flags bit 7: Topaz send format */

#define RFAL_ST25R95_IDLE_DEFAULT_WUPERIOD                                           0x24U /*!< Fixed WU Period to reach ~300 ms timeout with Max Sleep = 0 */

#define ST25R95_TAGDETECT_DEF_CALIBRATION 0x7C             /*!< Tag Detection Calibration default value                    */

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#define ST25R95_TimerStart(timer, time_ms)         (timer) = timerCalculateTimer((uint16_t)(time_ms)) /*!< Configures and starts the RTOX timer          */
#define ST25R95_TimerisExpired(timer)              timerIsExpired(timer)          /*!< Checks if timer has expired                   */

#define ST25R95_RunBlocking( e, fn )                 do{ (e)=(fn); ST25R95_Worker(); }while( (e) == ERR_BUSY )

    //iso15693PhyConfig_t iso15693PhyConfig; /*!< current phy configuration */
    //st25r95SPIRxContext st25r95SPIRxCtx; /*!< Context for SPI transceive     */
#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif /* RFAL_RFST25R95_H */
