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

#ifndef ST25R95_DEF_H
#define ST25R95_DEF_H

#include "NFC/NFC_Def.h"

#define ST25R95_ERRCODE_NONE                             0x00 /*!< no error occurred */
#define ST25R95_ERRCODE_FRAMEOKADDITIONALINFO            0x80 /*!< Frame correctly received (additionally see CRC/Parity information) */
#define ST25R95_ERRCODE_INVALIDCMDLENGHT                 0x82 /*!< Invalid command length */
#define ST25R95_ERRCODE_INVALIDPROTOCOL                  0x83 /*!< Invalid protocol */
#define ST25R95_ERRCODE_COMERROR                         0x86 /*!< Hardware communication error */
#define ST25R95_ERRCODE_FRAMEWAITTIMEOUT                 0x87 /*!< Frame wait time out (no valid reception) */
#define ST25R95_ERRCODE_INVALIDSOF                       0x88 /*!< Invalid SOF */
#define ST25R95_ERRCODE_OVERFLOW                         0x89 /*!< Too many bytes received and data still arriving */
#define ST25R95_ERRCODE_FRAMING                          0x8A /*!< if start bit = 1 or stop bit = 0 */
#define ST25R95_ERRCODE_EGT                              0x8B /*!< EGT time out */
#define ST25R95_ERRCODE_FIELDLENGTH                      0x8C /*!< Valid for ISO/IEC 18092, if Length <3 */
#define ST25R95_ERRCODE_CRC                              0x8D /*!< CRC error, Valid only for ISO/IEC 18092 */
#define ST25R95_ERRCODE_RECEPTIONLOST                    0x8E /*!< When reception is lost without EOF received (or subcarrier was lost) */
#define ST25R95_ERRCODE_NOFIELD                          0x8F /*!< When Listen command detects the absence of external field */
#define ST25R95_ERRCODE_RESULTSRESIDUAL                  0x90 /*!< Residual bits in last byte. Useful for ACK/NAK reception of ISO/IEC 14443 Type A. */
#define ST25R95_ERRCODE_61_SOF                           0x61 /*!< SOF error during the EMD process */
#define ST25R95_ERRCODE_62_CRC                           0x62 /*!< CRC error during the EMD process */
#define ST25R95_ERRCODE_63_SOF_HIGH                      0x63 /*!< SOF error in ISO14443B occurs during high part (duration of 2 to 3 Elementary Unit Time, ETU) */
#define ST25R95_ERRCODE_65_SOF_LOW                       0x65 /*!< SOF error in ISO14443B occurs during low part (duration of 10 to 11 Elementary Unit Time, ETU) */
#define ST25R95_ERRCODE_66_EGT                           0x66 /*!< Extra Guard Time (EGT) error in ISO14443B */
#define ST25R95_ERRCODE_67_TR1TOOLONG                    0x67 /*!< TR1 set by card too long in case of protocol ISO14443B */
#define ST25R95_ERRCODE_68_TR1TOOSHORT                   0x68 /*!< TTR1 set by card too short in case of protocol ISO14443B */

#define ST25R95_ERR_INCOMPLETE_BYTE                (40U) /*!< Incomplete byte rcvd         */
#define ST25R95_ERR_INCOMPLETE_BYTE_01             (41U) /*!< Incomplete byte rcvd - 1 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_02             (42U) /*!< Incomplete byte rcvd - 2 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_03             (43U) /*!< Incomplete byte rcvd - 3 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_04             (44U) /*!< Incomplete byte rcvd - 4 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_05             (45U) /*!< Incomplete byte rcvd - 5 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_06             (46U) /*!< Incomplete byte rcvd - 6 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_07             (47U) /*!< Incomplete byte rcvd - 7 bit */

#define ST25R95_IDN_RESPONSE_BUFLEN                              (15 + 2) /*!< IDN response buffer len */
#define ST25R95_PROTOCOLSELECT_RESPONSE_BUFLEN                   (0  + 2) /*!< ProtocolSelect response buffer len */
#define ST25R95_POLLFIELD_RESPONSE_BUFLEN                        (1  + 2) /*!< PollField response buffer len */
#define ST25R95_LISTEN_RESPONSE_BUFLEN                           (0  + 2) /*!< Listen response buffer len */
#define ST25R95_RDREG_RESPONSE_BUFLEN                            (1  + 2) /*!< ReadReg response buffer len */
#define ST25R95_WRREG_RESPONSE_BUFLEN                            (0  + 2) /*!< WriteRead response buffer len */
#define ST25R95_ACFILTER_RESPONSE_BUFLEN                         (1  + 2) /*!< ACFilter response buffer len */
#define ST25R95_IDLE_RESPONSE_BUFLEN                             (1  + 2) /*!< Idle response buffer len */
#define ST25R95_ECHO_RESPONSE_BUFLEN                             (3)      /*!< Echo response buffer len */
#define ST25R95_SEND_RESPONSE_BUFLEN                             (0  + 2) /*!< Send response buffer len */

#define ST25R95_REG_ARC_B                                0x6801 /*!< ARC_B register address */
#define ST25R95_REG_ACC_A                                0x6804 /*!< ACC_A register address */
#define ST25R95_REG_TIMERW                               0x3A00 /*!< TIMER register address */

typedef enum{
    ST25R95_Protocol_FieldOff       = 0x00U, /*!< Field OFF */
    ST25R95_Protocol_ISO15693       = 0x01U, /*!< ISO15693 Reader */
    ST25R95_Protocol_ISO14443A      = 0x02U, /*!< ISO14443A Reader */
    ST25R95_Protocol_ISO14443B      = 0x03U, /*!< ISO14443B Reader */
    ST25R95_Protocol_ISO18092       = 0x04U, /*!< ISO18092 Reader */
    ST25R95_Protocol_CE_ISO14443A   = 0x05U,  /*!< ISO14443A Card Emulation */
    ST25R95_Protocol_MAX
} ST25R95_Protocol;

/*! ST25R95 Bit rates    */
typedef enum {
  ST25R95_BitRate_106                      = 0,    /*!< Bit Rate 106 kbit/s (fc/128)                                      */
  ST25R95_BitRate_212                      = 1,    /*!< Bit Rate 212 kbit/s (fc/64)                                      */
  ST25R95_BitRate_424                      = 2,    /*!< Bit Rate 424 kbit/s (fc/32)                                      */
  ST25R95_BitRate_848                      = 3,    /*!< Bit Rate 848 kbit/s (fc/16)                                      */
  ST25R95_BitRate_1695                     = 4,    /*!< Bit Rate 1695 kbit/s (fc/8)                                      */
  ST25R95_BitRate_3390                     = 5,    /*!< Bit Rate 3390 kbit/s (fc/4)                                      */
  ST25R95_BitRate_6780                     = 6,    /*!< Bit Rate 6780 kbit/s (fc/2)                                      */
  ST25R95_BitRate_13560                    = 7,    /*!< Bit Rate 13560 kbit/s (fc)                                       */
  ST25R95_BitRate_211p88                   = 0xE9, /*!< Bit Rate 211,88 kbit/s (fc/64) Fast Mode VICC->VCD               */
  ST25R95_BitRate_105p94                   = 0xEA, /*!< Bit Rate 105,94 kbit/s (fc/128) Fast Mode VICC->VCD              */
  ST25R95_BitRate_52p97                    = 0xEB, /*!< Bit Rate 52.97 kbit/s (fc/256) Fast Mode VICC->VCD               */
  ST25R95_BitRate_26p48                    = 0xEC, /*!< Bit Rate 26,48 kbit/s (fc/512) NFCV VICC->VCD & VCD->VICC 1of4   */
  ST25R95_BitRate_1p66                     = 0xED, /*!< Bit Rate 1,66 kbit/s (fc/8192) NFCV VCD->VICC 1of256             */
  ST25R95_BitRate_KEEP                     = 0xFF  /*!< Value indicating to keep the same previous bit rate              */
} ST25R95_BitRate;

typedef enum {
  ST25R95_FeliCa_1_Slot    =  0,   /*!< TSN with number of Time Slots: 1  */
  ST25R95_FeliCa_2_Slots   =  1,   /*!< TSN with number of Time Slots: 2  */
  ST25R95_FeliCa_4_Slots   =  3,   /*!< TSN with number of Time Slots: 4  */
  ST25R95_FeliCa_8_Slots   =  7,   /*!< TSN with number of Time Slots: 8  */
  ST25R95_FeliCa_16_Slots  =  15   /*!< TSN with number of Time Slots: 16 */
} ST25R95_FeliCaPollSlots;

#endif // ST25R95_DEF_H