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

#ifndef ST25R3916_DEF_H
#define ST25R3916_DEF_H

#include "NFC/NFC_Def.h"

#include "NFC_Config.h"

#ifdef CONFIG_NFC_READER_DEVICE_ST25R3916
#define ST25R3916
#endif

#ifdef CONFIG_NFC_READER_DEVICE_ST25R3916B
#define ST25R3916B
#endif

/*! ST25R95 Bit rates    */
typedef enum {
  ST25R3916_BitRate_106                      = 0,    /*!< Bit Rate 106 kbit/s (fc/128)                                      */
  ST25R3916_BitRate_212                      = 1,    /*!< Bit Rate 212 kbit/s (fc/64)                                      */
  ST25R3916_BitRate_424                      = 2,    /*!< Bit Rate 424 kbit/s (fc/32)                                      */
  ST25R3916_BitRate_848                      = 3,    /*!< Bit Rate 848 kbit/s (fc/16)                                      */
  ST25R3916_BitRate_1695                     = 4,    /*!< Bit Rate 1695 kbit/s (fc/8)                                      */
  ST25R3916_BitRate_3390                     = 5,    /*!< Bit Rate 3390 kbit/s (fc/4)                                      */
  ST25R3916_BitRate_6780                     = 7,    /*!< Bit Rate 6780 kbit/s (fc/2)                                      */
  ST25R3916_BitRate_DO_NOT_SET               = 0xFF  /*!< Value indicating to keep the same previous bit rate              */
} ST25R3916_BitRate;

/* ST25R3916 direct commands */
#define ST25R3916_CMD_SET_DEFAULT              0xC1U    /*!< Puts the chip in default state (same as after power-up) */
#define ST25R3916_CMD_STOP                     0xC2U    /*!< Stops all activities and clears FIFO                    */
#define ST25R3916_CMD_TRANSMIT_WITH_CRC        0xC4U    /*!< Transmit with CRC                                       */
#define ST25R3916_CMD_TRANSMIT_WITHOUT_CRC     0xC5U    /*!< Transmit without CRC                                    */
#define ST25R3916_CMD_TRANSMIT_REQA            0xC6U    /*!< Transmit REQA                                           */
#define ST25R3916_CMD_TRANSMIT_WUPA            0xC7U    /*!< Transmit WUPA                                           */
#define ST25R3916_CMD_INITIAL_RF_COLLISION     0xC8U    /*!< NFC transmit with Initial RF Collision Avoidance        */
#define ST25R3916_CMD_RESPONSE_RF_COLLISION_N  0xC9U    /*!< NFC transmit with Response RF Collision Avoidance       */
#define ST25R3916_CMD_GOTO_SENSE               0xCDU    /*!< Passive target logic to Sense/Idle state                */
#define ST25R3916_CMD_GOTO_SLEEP               0xCEU    /*!< Passive target logic to Sleep/Halt state                */
#define ST25R3916_CMD_MASK_RECEIVE_DATA        0xD0U    /*!< Mask receive data                                       */
#define ST25R3916_CMD_UNMASK_RECEIVE_DATA      0xD1U    /*!< Unmask receive data                                     */
#define ST25R3916_CMD_AM_MOD_STATE_CHANGE      0xD2U    /*!< AM Modulation state change                              */
#define ST25R3916_CMD_MEASURE_AMPLITUDE        0xD3U    /*!< Measure signal amplitude on RFI inputs                  */
#define ST25R3916_CMD_RESET_RXGAIN             0xD5U    /*!< Reset RX Gain                                           */
#define ST25R3916_CMD_ADJUST_REGULATORS        0xD6U    /*!< Adjust regulators                                       */
#define ST25R3916_CMD_CALIBRATE_DRIVER_TIMING  0xD8U    /*!< Starts the sequence to adjust the driver timing         */
#define ST25R3916_CMD_MEASURE_PHASE            0xD9U    /*!< Measure phase between RFO and RFI signal                */
#define ST25R3916_CMD_CLEAR_RSSI               0xDAU    /*!< Clear RSSI bits and restart the measurement             */
#define ST25R3916_CMD_CLEAR_FIFO               0xDBU    /*!< Clears FIFO, Collision and IRQ status                   */
#define ST25R3916_CMD_TRANSPARENT_MODE         0xDCU    /*!< Transparent mode                                        */
#ifdef ST25R3916
  #define ST25R3916_CMD_CALIBRATE_C_SENSOR       0xDDU    /*!< Calibrate the capacitive sensor                         */
  #define ST25R3916_CMD_MEASURE_CAPACITANCE      0xDEU    /*!< Measure capacitance                                     */
#endif /* ST25R3916 */
#define ST25R3916_CMD_MEASURE_VDD              0xDFU    /*!< Measure power supply voltage                            */
#define ST25R3916_CMD_START_GP_TIMER           0xE0U    /*!< Start the general purpose timer                         */
#define ST25R3916_CMD_START_WUP_TIMER          0xE1U    /*!< Start the wake-up timer                                 */
#define ST25R3916_CMD_START_MASK_RECEIVE_TIMER 0xE2U    /*!< Start the mask-receive timer                            */
#define ST25R3916_CMD_START_NO_RESPONSE_TIMER  0xE3U    /*!< Start the no-response timer                             */
#define ST25R3916_CMD_START_PPON2_TIMER        0xE4U    /*!< Start PPon2 timer                                       */
#define ST25R3916_CMD_STOP_NRT                 0xE8U    /*!< Stop No Response Timer                                  */
#ifdef ST25R3916B
  #define ST25R3916_CMD_RC_CAL                   0xEAU    /*!< Trigger RC calibration                                  */
#endif /* ST25R3916B */
#define ST25R3916_CMD_SPACE_B_ACCESS           0xFBU    /*!< Enable R/W access to the test registers                 */
#define ST25R3916_CMD_TEST_ACCESS              0xFCU    /*!< Enable R/W access to the test registers                 */


#define ST25R3916_THRESHOLD_DO_NOT_SET         0xFFU    /*!< Indicates not to change this Threshold                  */

#define ST25R3916_FIFO_DEPTH                   512U     /*!< Depth of FIFO                                           */
#define ST25R3916_TOUT_OSC_STABLE              10U      /*!< Max timeout for Oscillator to get stable      DS: 700us */

#endif // ST25R3916_DEF_H