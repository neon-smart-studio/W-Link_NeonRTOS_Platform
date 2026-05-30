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

#ifndef ST25R3916_H
#define ST25R3916_H

#include <stdbool.h>
#include <stdint.h>

#include "ST25R3916_Def.h"

#include "ST25R3916_IO.h"

#if defined(CONFIG_NFC_READER_DEVICE_ST25R3916) || defined(CONFIG_NFC_READER_DEVICE_ST25R3916B)

#define ST25R3916_TOUT_MEASURE_VDD            100U    /*!< Max duration time of Measure Power Supply command  Datasheet: 25us  */
#define ST25R3916_TOUT_MEASURE_AMPLITUDE      10U     /*!< Max duration time of Measure Amplitude command     Datasheet: 25us  */
#define ST25R3916_TOUT_MEASURE_PHASE          10U     /*!< Max duration time of Measure Phase command         Datasheet: 25us  */
#define ST25R3916_TOUT_MEASURE_CAPACITANCE    10U     /*!< Max duration time of Measure Capacitance command   Datasheet: 25us  */
#define ST25R3916_TOUT_CALIBRATE_CAP_SENSOR   4U      /*!< Max duration Calibrate Capacitive Sensor command   Datasheet: 3ms   */
#define ST25R3916_TOUT_CALIBRATE_AWS_RC       10U     /*!< Max duration Calibrate RC command                  Datasheet: 5ms   */
#define ST25R3916_TOUT_ADJUST_REGULATORS      6U      /*!< Max duration time of Adjust Regulators command     Datasheet: 5ms   */
#define ST25R3916_TOUT_CA                     10U     /*!< Max duration time of Collision Avoidance command                    */

#define ST25R3916_TEST_REG_PATTERN                0x33U   /*!< Register Read Write test pattern used during selftest               */
#define ST25R3916_TEST_WU_TOUT                    12U     /*!< Timeout used on WU timer during self test                           */
#define ST25R3916_TEST_TMR_TOUT                   20U     /*!< Timeout used during self test                                       */
#define ST25R3916_TEST_TMR_TOUT_DELTA             2U      /*!< Timeout used during self test                                       */
#define ST25R3916_TEST_TMR_TOUT_8FC               (ST25R3916_TEST_TMR_TOUT * 1695U)  /*!< Timeout in 8/fc                          */

/*! Enables the Transmitter (Field On) and Receiver                                          */
#define ST25R3916_IO_TxRxOn()             ST25R3916_IO_SetRegisterBits( ST25R3916_REG_OP_CONTROL, (ST25R3916_REG_OP_CONTROL_rx_en | ST25R3916_REG_OP_CONTROL_tx_en ) )

/*! Disables the Transmitter (Field Off) and Receiver                                         */
#define ST25R3916_IO_TxRxOff()            ST25R3916_IO_ClearRegisterBits( ST25R3916_REG_OP_CONTROL, (ST25R3916_REG_OP_CONTROL_rx_en | ST25R3916_REG_OP_CONTROL_tx_en ) )

/*! Disables the Transmitter (Field Off)                                         */
#define ST25R3916_IO_TxOff()              ST25R3916_IO_ClearRegisterBits( ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_tx_en )

/*! Checks if General Purpose Timer is still running by reading gpt_on flag                  */
#define ST25R3916_IO_IsGPTRunning( )      ST25R3916_IO_CheckReg( ST25R3916_REG_NFCIP1_BIT_RATE, ST25R3916_REG_NFCIP1_BIT_RATE_gpt_on, ST25R3916_REG_NFCIP1_BIT_RATE_gpt_on )

/*! Checks if External Filed is detected by reading ST25R3916 External Field Detector output    */
#define ST25R3916_IO_IsExtFieldOn()       ST25R3916_IO_CheckReg( ST25R3916_REG_AUX_DISPLAY, ST25R3916_REG_AUX_DISPLAY_efd_o, ST25R3916_REG_AUX_DISPLAY_efd_o )

/*! Checks if Transmitter is enabled (Field On) */
#define ST25R3916_IO_IsTxEnabled()        ST25R3916_IO_CheckReg( ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_tx_en, ST25R3916_REG_OP_CONTROL_tx_en )

/*! Checks if NRT is in EMV mode */
#define ST25R3916_IO_IsNRTinEMV()         ST25R3916_IO_CheckReg( ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv_on )

/*! Checks if last FIFO byte is complete */
#define ST25R3916_IO_IsLastFIFOComplete() ST25R3916_IO_CheckReg( ST25R3916_REG_FIFO_STATUS2, ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask, 0 )

/*! Checks if the Oscillator is enabled  */
#define ST25R3916_IO_IsOscOn()            ST25R3916_IO_CheckReg( ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en, ST25R3916_REG_OP_CONTROL_en )

/*! Checks if the AAT is enabled  */
#define ST25R3916_IO_IsAATOn()            ST25R3916_IO_CheckReg( ST25R3916_REG_IO_CONF2, ST25R3916_REG_IO_CONF2_aat_en, ST25R3916_REG_IO_CONF2_aat_en )

#ifdef __cplusplus
extern "C" {
#endif

/*! Struct to represent all regs on ST25R3916                                                             */
typedef struct {
  uint8_t RsA[(ST25R3916_REG_IC_IDENTITY + 1U)]; /*!< Registers contained on ST25R3916 space A (Rs-A)     */
  uint8_t RsB[ST25R3916_SPACE_B_REG_LEN];      /*!< Registers contained on ST25R3916 space B (Rs-B)     */
} t_ST25R3916_IO_Regs;

/*! Parameters how the stream mode should work                                                            */
struct ST25R3916_IO_StreamConfig {
  uint8_t useBPSK;                            /*!< 0: subcarrier, 1:BPSK                                */
  uint8_t din;                                /*!< Divider for the in subcarrier frequency: fc/2^din    */
  uint8_t dout;                               /*!< Divider for the in subcarrier frequency fc/2^dout    */
  uint8_t report_period_length;               /*!< Length of the reporting period 2^report_period_length*/
};

NFC_OpResult ST25R3916_OscOn(void);
NFC_OpResult ST25R3916_ExecuteCommandAndGetResult(uint8_t cmd, uint8_t resReg, uint8_t tout, uint8_t *result);
uint8_t ST25R3916_MeasurePowerSupply(uint8_t mpsv);
uint16_t ST25R3916_MeasureVoltage(uint8_t mpsv);
NFC_OpResult ST25R3916_AdjustRegulators(uint16_t *result_mV);
NFC_OpResult ST25R3916_MeasureAmplitude(uint8_t *result);
NFC_OpResult ST25R3916_MeasurePhase(uint8_t *result);
NFC_OpResult ST25R3916_MeasureCapacitance(uint8_t *result);
NFC_OpResult ST25R3916_CalibrateCapacitiveSensor(uint8_t *result);
NFC_OpResult ST25R3916_SetBitrate(ST25R3916_BitRate txrate, ST25R3916_BitRate rxrate);
NFC_OpResult ST25R3916_PerformCollisionAvoidance(uint8_t FieldONCmd, uint8_t pdThreshold, uint8_t caThreshold, uint8_t nTRFW);
void ST25R3916_SetNumTxBits(uint16_t nBits);
uint16_t ST25R3916_GetNumFIFOBytes(void);
uint8_t ST25R3916_GetNumFIFOLastBits(void);
uint32_t ST25R3916_GetNoResponseTime(void);
NFC_OpResult ST25R3916_SetNoResponseTime(uint32_t nrt_64fcs);
NFC_OpResult ST25R3916_SetStartNoResponseTimer(uint32_t nrt_64fcs);
void ST25R3916_SetGPTime(uint16_t gpt_8fcs);
NFC_OpResult ST25R3916_SetStartGPTimer(uint16_t gpt_8fcs, uint8_t trigger_source);
bool ST25R3916_CheckChipID(uint8_t *rev);
NFC_OpResult ST25R3916_GetRegsDump(t_ST25R3916_IO_Regs *regDump);
bool ST25R3916_IsCmdValid(uint8_t cmd);
NFC_OpResult ST25R3916_StreamConfigure(const struct ST25R3916_IO_StreamConfig *config);
NFC_OpResult ST25R3916_GetRSSI(uint16_t *amRssi, uint16_t *pmRssi);
NFC_OpResult ST25R3916_SetAntennaMode(bool single, bool rfiox);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_NFC_READER_DEVICE_ST25R3916 || CONFIG_NFC_READER_DEVICE_ST25R3916B */

#endif // ST25R3916_H
