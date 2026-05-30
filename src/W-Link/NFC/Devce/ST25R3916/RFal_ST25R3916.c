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

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "NeonRTOS.h"

#include "ST25R3916_IO.h"

#include "ST25R3916.h"

#include "ST25R3916_ISO15693.h"

#include "RFal_ST25R3916_AnalogConfig.h"
#include "RFal_ST25R3916.h"

#include "NFC/RFal/RFal.h"

#include "NFC_Config.h"

#if defined(CONFIG_NFC_READER_DEVICE_ST25R3916) || defined(CONFIG_NFC_READER_DEVICE_ST25R3916B)

#define RFal_CreateByteFlagsTxRxContext( ctx, tB, tBL, rB, rBL, rdL, fl, t ) \
    (ctx).txBuf     = (uint8_t*)(tB);                                       \
    (ctx).txBufLen  = (uint16_t)RFal_ConvBytesToBits(tBL);                   \
    (ctx).rxBuf     = (uint8_t*)(rB);                                       \
    (ctx).rxBufLen  = (uint16_t)RFal_ConvBytesToBits(rBL);                   \
    (ctx).rxRcvdLen = (uint16_t*)(rdL);                                     \
    (ctx).flags     = (uint32_t)(fl);                                       \
    (ctx).fwt       = (uint32_t)(t);

#define ST25R3916_SUPPLY_THRESHOLD            3600U   /*!< Power supply measure threshold between 3.3V or 5V                   */

static RFal gRFAL;

static NFC_OpResult RFal_RunListenModeWorker(void);
static NFC_OpResult RFal_RunWakeUpModeWorker(void);
static NFC_OpResult RFal_RunTransceiveWorker(void);

/*******************************************************************************/
static void RFal_FIFOStatusUpdate(void)
{
  if (gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] == RFAL_FIFO_STATUS_INVALID) {
    ST25R3916_IO_ReadMultipleRegisters(ST25R3916_REG_FIFO_STATUS1, gRFAL.fifo.status, ST25R3916_FIFO_STATUS_LEN);
  }
}

/*******************************************************************************/
static void RFal_FIFOStatusClear(void)
{
  gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] = RFAL_FIFO_STATUS_INVALID;
}

/*******************************************************************************/
static uint16_t RFal_FIFOStatusGetNumBytes(void)
{
  uint16_t result;

  RFal_FIFOStatusUpdate();

  result  = ((((uint16_t)gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] & ST25R3916_REG_FIFO_STATUS2_fifo_b_mask) >> ST25R3916_REG_FIFO_STATUS2_fifo_b_shift) << RFAL_BITS_IN_BYTE);
  result |= (((uint16_t)gRFAL.fifo.status[RFAL_FIFO_STATUS_REG1]) & 0x00FFU);
  return result;
}

/*******************************************************************************/
static bool RFal_FIFOStatusIsIncompleteByte(void)
{
  RFal_FIFOStatusUpdate();
  return ((gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] & ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask) != 0U);
}

/*******************************************************************************/
static bool RFal_FIFOStatusIsMissingPar(void)
{
  RFal_FIFOStatusUpdate();
  return ((gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] & ST25R3916_REG_FIFO_STATUS2_np_lb) != 0U);
}

/*******************************************************************************/
static uint8_t RFal_FIFOGetNumIncompleteBits(void)
{
  RFal_FIFOStatusUpdate();
  return ((gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] & ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask) >> ST25R3916_REG_FIFO_STATUS2_fifo_lb_shift);
}

/*******************************************************************************/
NFC_OpResult RFal_Init(void)
{
  NFC_OpResult err;

  RFal_ST25R3916_AnalogConfigInit();              /* Initialize RFAL's Analog Configs */

  err = ST25R3916_IO_Init();
  if (err < NFC_OK) {
    return err;
  }

  uint16_t vdd_mV;

  /* Set default state on the ST25R3916 */
  ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_SET_DEFAULT);

#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
  /* Increase MISO driving level as SPI can go up to 10MHz */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_IO_CONF2, ST25R3916_REG_IO_CONF2_io_drv_lvl);
#endif

  if (!ST25R3916_CheckChipID(NULL)) {
    return NFC_Hw_Mismatch;
  }

#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
  /* Enable pull downs on MISO line */
  ST25R3916_IO_SetRegisterBits(ST25R3916_REG_IO_CONF2, (ST25R3916_REG_IO_CONF2_miso_pd1 | ST25R3916_REG_IO_CONF2_miso_pd2));
#endif

#ifdef ST25R3916
  /* Disable internal overheat protection */
  ST25R3916_IO_ChangeTestRegisterBits(0x04, 0x10, 0x10);
#endif /* ST25R3916 */

#ifdef ST25R_SELFTEST
  /******************************************************************************
   * Check communication interface:
   *  - write a pattern in a register
   *  - reads back the register value
   *  - return NFC_IO_Error in case the read value is different
   */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_BIT_RATE, ST25R3916_TEST_REG_PATTERN);
  if (!ST25R3916_IO_CheckReg(ST25R3916_REG_BIT_RATE, (ST25R3916_REG_BIT_RATE_rxrate_mask | ST25R3916_REG_BIT_RATE_txrate_mask), ST25R3916_TEST_REG_PATTERN)) {
    return NFC_IO_Error;
  }

  /* Restore default value */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_BIT_RATE, 0x00);

  /*
   * Check IRQ Handling:
   *  - use the Wake-up timer to trigger an IRQ
   *  - wait the Wake-up timer interrupt
   *  - return NFC_SlaveTimeout when the Wake-up timer interrupt is not received
   */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_WUP_TIMER_CONTROL, ST25R3916_REG_WUP_TIMER_CONTROL_wur | ST25R3916_REG_WUP_TIMER_CONTROL_wto);
  ST25R3916_IO_EnableInterrupts(ST25R3916_IRQ_MASK_WT);
  ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_START_WUP_TIMER);
  if (ST25R3916_IO_WaitForInterruptsTimed(ST25R3916_IRQ_MASK_WT, ST25R3916_TEST_WU_TOUT) == 0U) {
    return NFC_SlaveTimeout;
  }
  ST25R3916_IO_DisableInterrupts(ST25R3916_IRQ_MASK_WT);
  ST25R3916_IO_WriteRegister(ST25R3916_REG_WUP_TIMER_CONTROL, 0U);
  /*******************************************************************************/
#endif /* ST25R_SELFTEST */

  /* Enable Oscillator and wait until it gets stable */
  err = ST25R3916_OscOn();
  if (err < NFC_OK) {
    return err;
  }


#ifdef ST25R3916B
  /* Trigger RC calibration */
  ST25R3916_ExecuteCommandAndGetResult(ST25R3916_CMD_RC_CAL,  ST25R3916_REG_AWS_RC_CAL, ST25R3916_TOUT_CALIBRATE_AWS_RC, NULL);
#endif /* ST25R3916B */


  /* Measure VDD and set sup3V bit according to Power supplied  */
  vdd_mV = ST25R3916_MeasureVoltage(ST25R3916_REG_REGULATOR_CONTROL_mpsv_vdd);
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_IO_CONF2, ST25R3916_REG_IO_CONF2_sup3V, ((vdd_mV < ST25R3916_SUPPLY_THRESHOLD) ? ST25R3916_REG_IO_CONF2_sup3V_3V : ST25R3916_REG_IO_CONF2_sup3V_5V));

  /* Make sure Transmitter and Receiver are disabled */
  ST25R3916_IO_TxRxOff();


#ifdef ST25R_SELFTEST_TIMER
  /******************************************************************************
   * Check SW timer operation :
   *  - use the General Purpose timer to measure an amount of time
   *  - test whether an interrupt is seen when less time was given
   *  - test whether an interrupt is seen when sufficient time was given
   */

  ST25R3916_IO_EnableInterrupts(ST25R3916_IRQ_MASK_GPE);
  ST25R3916_SetStartGPTimer((uint16_t)ST25R3916_TEST_TMR_TOUT_8FC, ST25R3916_REG_TIMER_EMV_CONTROL_gptc_no_trigger);
  if (ST25R3916_IO_WaitForInterruptsTimed(ST25R3916_IRQ_MASK_GPE, (ST25R3916_TEST_TMR_TOUT - ST25R3916_TEST_TMR_TOUT_DELTA)) != 0U) {
    return NFC_System;
  }

  /* Stop all activities to stop the GP timer */
  ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_STOP);
  ST25R3916_IO_ClearAndEnableInterrupts(ST25R3916_IRQ_MASK_GPE);
  ST25R3916_SetStartGPTimer((uint16_t)ST25R3916_TEST_TMR_TOUT_8FC, ST25R3916_REG_TIMER_EMV_CONTROL_gptc_no_trigger);
  if (ST25R3916_IO_WaitForInterruptsTimed(ST25R3916_IRQ_MASK_GPE, (ST25R3916_TEST_TMR_TOUT + ST25R3916_TEST_TMR_TOUT_DELTA)) == 0U) {
    return NFC_System;
  }

  /* Stop all activities to stop the GP timer */
  ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_STOP);
  /*******************************************************************************/
#endif /* ST25R_SELFTEST_TIMER */


  /* After reset all interrupts are enabled, so disable them at first */
  ST25R3916_IO_DisableInterrupts(ST25R3916_IRQ_MASK_ALL);

  /* And clear them, just to be sure */
  ST25R3916_IO_ClearInterrupts();

  /* Disable any previous observation mode */
  RFal_ST25R3916ObsModeDisable();

  /*******************************************************************************/
  /* Apply RF Chip generic initialization */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_INIT));


  /*******************************************************************************/
  /* Enable External Field Detector as: Automatics */
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en_fd_mask, ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);

  /* Clear FIFO status local copy */
  RFal_FIFOStatusClear();


  /*******************************************************************************/
  gRFAL.state              = RFAL_STATE_INIT;
  gRFAL.mode               = RFAL_MODE_NONE;
  gRFAL.field              = false;

  /* Set RFAL default configs */
  gRFAL.conf.obsvModeRx    = RFAL_OBSMODE_DISABLE;
  gRFAL.conf.obsvModeTx    = RFAL_OBSMODE_DISABLE;
  gRFAL.conf.eHandling     = ERRORHANDLING_NONE;

  /* Transceive set to IDLE */
  gRFAL.TxRx.lastState     = RFAL_TXRX_STATE_IDLE;
  gRFAL.TxRx.state         = RFAL_TXRX_STATE_IDLE;

  /* Disable all timings */
  gRFAL.timings.FDTListen  = RFAL_TIMING_NONE;
  gRFAL.timings.FDTPoll    = RFAL_TIMING_NONE;
  gRFAL.timings.GT         = RFAL_TIMING_NONE;
  gRFAL.timings.nTRFW      = 0U;


  gRFAL.tmr.GT             = RFAL_TIMING_NONE;
  gRFAL.tmr.txRx           = RFAL_TIMING_NONE;
  gRFAL.tmr.RXE            = RFAL_TIMING_NONE;
  gRFAL.tmr.PPON2          = RFAL_TIMING_NONE;


  gRFAL.callbacks.preTxRx  = NULL;
  gRFAL.callbacks.postTxRx = NULL;
  gRFAL.callbacks.syncTxRx = NULL;

  /* Initialize NFC-V Data */
  gRFAL.nfcvData.ignoreBits = 0;

  /* Initialize Listen Mode */
  gRFAL.Lm.state           = RFAL_LM_STATE_NOT_INIT;
  gRFAL.Lm.brDetected      = RFAL_BR_KEEP;
  gRFAL.Lm.iniFlag         = false;
  
  /* Initialize Wake-Up Mode */
  gRFAL.wum.state = RFAL_WUM_STATE_NOT_INIT;
  
  /* Initialize Low Power Mode */
  gRFAL.lpm.isRunning     = false;

  /*******************************************************************************/
  /* Perform Automatic Calibration (if configured to do so).                     *
   * Registers set by RFal_ST25R3916_SetAnalogConfig will tell RFal_Calibrate what to perform*/
  RFal_Calibrate();

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_Deinit(void)
{
  /* Set Analog configurations for deinitialization */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_DEINIT));

  gRFAL.state = RFAL_STATE_IDLE;

  /* Deinitialize chip */
  ST25R3916_IO_DeInit();

  ST25R3916_IO_DisableInterrupts(ST25R3916_IRQ_MASK_ALL);

  /* Disable Tx and Rx, Keep OSC On */
  ST25R3916_IO_TxRxOff();

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_Calibrate(void)
{
  uint16_t resValue;

  /* Check if RFAL is not initialized */
  if (gRFAL.state == RFAL_STATE_IDLE) {
    return NFC_WrongState;
  }

  /*******************************************************************************/
  /* Perform ST25R3916 regulators and antenna calibration                        */
  /*******************************************************************************/

  /* Automatic regulator adjustment only performed if not set manually on Analog Configs */
  if (ST25R3916_IO_CheckReg(ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_reg_s, 0x00)) {
    /* Adjust the regulators so that Antenna Calibrate has better Regulator values */
    ST25R3916_AdjustRegulators(&resValue);
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_AdjustRegulators(uint16_t *result)
{
  return ST25R3916_AdjustRegulators(result);
}

/*******************************************************************************/
void RFal_SetUpperLayerCallback(RFal_UpperLayerCallback pFunc)
{
  ST25R3916_IO_IRQCallbackSet(pFunc);
}

/*******************************************************************************/
void RFal_SetPreTxRxCallback(RFal_PreTxRxCallback pFunc)
{
  gRFAL.callbacks.preTxRx = pFunc;
}

/*******************************************************************************/
void RFal_SetSyncTxRxCallback(RFal_SyncTxRxCallback pFunc)
{
  gRFAL.callbacks.syncTxRx = pFunc;
}

/*******************************************************************************/
void RFal_SetPostTxRxCallback(RFal_PostTxRxCallback pFunc)
{
  gRFAL.callbacks.postTxRx = pFunc;
}

/*******************************************************************************/
void RFal_SetLmEonCallback(RFal_LmEonCallback pFunc)
{
  return;   /* NFC_Unsupport */
}

/*******************************************************************************/
void RFal_SetObsvMode(uint32_t txMode, uint32_t rxMode)
{
  gRFAL.conf.obsvModeTx = (uint8_t)txMode;
  gRFAL.conf.obsvModeRx = (uint8_t)rxMode;
}

/*******************************************************************************/
void RFal_GetObsvMode(uint8_t *txMode, uint8_t *rxMode)
{
  if (txMode != NULL) {
    *txMode = gRFAL.conf.obsvModeTx;
  }

  if (rxMode != NULL) {
    *rxMode = gRFAL.conf.obsvModeRx;
  }
}

/*******************************************************************************/
void RFal_DisableObsvMode(void)
{
  gRFAL.conf.obsvModeTx = RFAL_OBSMODE_DISABLE;
  gRFAL.conf.obsvModeRx = RFAL_OBSMODE_DISABLE;
}

/*******************************************************************************/
NFC_OpResult RFal_SetMode(RFal_Mode mode, RFal_BitRate txBR, RFal_BitRate rxBR)
{
  /* Check if RFAL is not initialized */
  if (gRFAL.state == RFAL_STATE_IDLE) {
    return NFC_WrongState;
  }

  /* Check allowed bit rate value */
  if ((txBR == RFAL_BR_KEEP) || (rxBR == RFAL_BR_KEEP)) {
    return NFC_InvalidParameter;
  }

  switch (mode) {
    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCA:
      /* Disable wake up mode, if set */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

      /* Enable ISO14443A mode */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_iso14443a);

      /* Set Analog configurations for this mode and bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCA_T1T:
      /* Disable wake up mode, if set */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

      /* Enable Topaz mode */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_topaz);

      /* Set Analog configurations for this mode and bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCB:
      /* Disable wake up mode, if set */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

      /* Enable ISO14443B mode */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_iso14443b);

      /* Set the EGT, SOF, EOF and EOF */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_ISO14443B_1,
                                  (ST25R3916_REG_ISO14443B_1_egt_mask | ST25R3916_REG_ISO14443B_1_sof_mask | ST25R3916_REG_ISO14443B_1_eof),
                                  ((0U << ST25R3916_REG_ISO14443B_1_egt_shift) | ST25R3916_REG_ISO14443B_1_sof_0_10etu | ST25R3916_REG_ISO14443B_1_sof_1_2etu | ST25R3916_REG_ISO14443B_1_eof_10etu));

      /* Set the minimum TR1, SOF, EOF and EOF12 */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_ISO14443B_2,
                                  (ST25R3916_REG_ISO14443B_2_tr1_mask | ST25R3916_REG_ISO14443B_2_no_sof | ST25R3916_REG_ISO14443B_2_no_eof),
                                  (ST25R3916_REG_ISO14443B_2_tr1_80fs80fs));

      /* Set Analog configurations for this mode and bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCB | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCB | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_B_PRIME:
      /* Disable wake up mode, if set */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

      /* Enable ISO14443B mode */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_iso14443b);

      /* Set the EGT, SOF, EOF and EOF */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_ISO14443B_1,
                                  (ST25R3916_REG_ISO14443B_1_egt_mask | ST25R3916_REG_ISO14443B_1_sof_mask | ST25R3916_REG_ISO14443B_1_eof),
                                  ((0U << ST25R3916_REG_ISO14443B_1_egt_shift) | ST25R3916_REG_ISO14443B_1_sof_0_10etu | ST25R3916_REG_ISO14443B_1_sof_1_2etu | ST25R3916_REG_ISO14443B_1_eof_10etu));

      /* Set the minimum TR1, EOF and EOF12 */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_ISO14443B_2,
                                  (ST25R3916_REG_ISO14443B_2_tr1_mask | ST25R3916_REG_ISO14443B_2_no_sof | ST25R3916_REG_ISO14443B_2_no_eof),
                                  (ST25R3916_REG_ISO14443B_2_tr1_80fs80fs | ST25R3916_REG_ISO14443B_2_no_sof));

      /* Set Analog configurations for this mode and bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCB | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCB | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_B_CTS:
      /* Disable wake up mode, if set */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

      /* Enable ISO14443B mode */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_iso14443b);

      /* Set the EGT, SOF, EOF and EOF */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_ISO14443B_1,
                                  (ST25R3916_REG_ISO14443B_1_egt_mask | ST25R3916_REG_ISO14443B_1_sof_mask | ST25R3916_REG_ISO14443B_1_eof),
                                  ((0U << ST25R3916_REG_ISO14443B_1_egt_shift) | ST25R3916_REG_ISO14443B_1_sof_0_10etu | ST25R3916_REG_ISO14443B_1_sof_1_2etu | ST25R3916_REG_ISO14443B_1_eof_10etu));

      /* Set the minimum TR1, clear SOF, EOF and EOF12 */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_ISO14443B_2,
                                  (ST25R3916_REG_ISO14443B_2_tr1_mask | ST25R3916_REG_ISO14443B_2_no_sof | ST25R3916_REG_ISO14443B_2_no_eof),
                                  (ST25R3916_REG_ISO14443B_2_tr1_80fs80fs | ST25R3916_REG_ISO14443B_2_no_sof | ST25R3916_REG_ISO14443B_2_no_eof));

      /* Set Analog configurations for this mode and bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCB | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCB | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCF:
      /* Disable wake up mode, if set */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

      /* Enable FeliCa mode */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_felica);

      /* Set Analog configurations for this mode and bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCF | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCF | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCV:
    case RFAL_MODE_POLL_PICOPASS:
      /* Disable wake up mode, if set */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

      /* Set Analog configurations for this mode and bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCV | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCV | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_ACTIVE_P2P:
      /* Set NFCIP1 active communication Initiator mode and Automatic Response RF Collision Avoidance to always after EOF */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, (ST25R3916_REG_MODE_targ_init | ST25R3916_REG_MODE_om_nfc | ST25R3916_REG_MODE_nfc_ar_eof));

      /* External Field Detector enabled as Automatics on RFal_Initialize() */

      /* Set NRT to start at end of TX (own) field */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc_off);

      /* Set GPT to start after end of TX, as GPT is used in active communication mode to timeout the field switching off after TXE */
      ST25R3916_SetStartGPTimer((uint16_t)RFal_Conv1fcTo8fc(RFAL_AP2P_FIELDOFF_TCMDOFF), ST25R3916_REG_TIMER_EMV_CONTROL_gptc_etx_nfc);

      /* Set PPon2 timer with the max time between our field Off and other peer field On : Tadt + (n x Trfw)    */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_PPON2, (uint8_t)RFal_Conv1fcTo64fc(RFAL_AP2P_FIELDON_TADTTRFW));

      /* Set Analog configurations for this mode and bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_AP2P | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_AP2P | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_ACTIVE_P2P:
      /* Set NFCIP1 active communication Target mode and Automatic Response RF Collision Avoidance to always after EOF */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om_targ_nfcip | ST25R3916_REG_MODE_nfc_ar_eof));

      /* Set TARFG: 0 (75us+0ms=75us), as Target no Guard time needed */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_FIELD_ON_GT, 0U);
      /* External Field Detector enabled as Automatics on RFal_Initialize() */

      /* Set NRT to start at end of TX (own) field */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc_off);

      /* Set GPT to start after end of TX, as GPT is used in active communication mode to timeout the field switching off after TXE */

      ST25R3916_SetStartGPTimer((uint16_t)RFal_Conv1fcTo8fc(RFAL_AP2P_FIELDOFF_TCMDOFF), ST25R3916_REG_TIMER_EMV_CONTROL_gptc_etx_nfc);

      /* Set PPon2 timer with the max time between our field Off and other peer field On : Tadt + (n x Trfw)    */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_PPON2, (uint8_t)RFal_Conv1fcTo64fc(RFAL_AP2P_FIELDON_TADTTRFW));

      /* Set Analog configurations for this mode and bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_AP2P | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_AP2P | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCA:
      /* Disable wake up mode, if set */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

      /* Enable Passive Target NFC-A mode, disable any Collision Avoidance */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_targ_nfca | ST25R3916_REG_MODE_nfc_ar_off));

      /* Set Analog configurations for this mode */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCF:
      /* Disable wake up mode, if set */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

      /* Enable Passive Target NFC-F mode, disable any Collision Avoidance */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_targ_nfcf | ST25R3916_REG_MODE_nfc_ar_off));

      /* Set Analog configurations for this mode */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCF | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCF | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCB:
      return NFC_Unsupport;

    /*******************************************************************************/
    default:
      return NFC_Unsupport;
  }

  /* Set state as STATE_MODE_SET only if not initialized yet (PSL) */
  gRFAL.state = ((gRFAL.state < RFAL_STATE_MODE_SET) ? RFAL_STATE_MODE_SET : gRFAL.state);
  gRFAL.mode  = mode;

  /* Apply the given bit rate */
  return RFal_SetBitRate(txBR, rxBR);
}

/*******************************************************************************/
RFal_Mode RFal_GetMode(void)
{
  return gRFAL.mode;
}

/*******************************************************************************/
NFC_OpResult RFal_SetBitRate(RFal_BitRate txBR, RFal_BitRate rxBR)
{
  NFC_OpResult ret;

  /* Check if RFAL is not initialized */
  if (gRFAL.state == RFAL_STATE_IDLE) {
    return NFC_WrongState;
  }

  /* Store the new Bit Rates */
  gRFAL.txBR = ((txBR == RFAL_BR_KEEP) ? gRFAL.txBR : txBR);
  gRFAL.rxBR = ((rxBR == RFAL_BR_KEEP) ? gRFAL.rxBR : rxBR);

  /* Update the bitrate reg if not in NFCV mode (streaming) */
  if ((RFAL_MODE_POLL_NFCV != gRFAL.mode) && (RFAL_MODE_POLL_PICOPASS != gRFAL.mode)) {
    /* Set bit rate register */
    ret = ST25R3916_SetBitrate((uint8_t)gRFAL.txBR, (uint8_t)gRFAL.rxBR);
    if(ret < NFC_OK)
    {
      return ret;
    }
  }


  switch (gRFAL.mode) {
    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCA:
    case RFAL_MODE_POLL_NFCA_T1T:
      /* Set Analog configurations for this bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_POLL_COMMON));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCB:
    case RFAL_MODE_POLL_B_PRIME:
    case RFAL_MODE_POLL_B_CTS:
      /* Set Analog configurations for this bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_POLL_COMMON));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCB | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCB | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCF:
      /* Set Analog configurations for this bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_POLL_COMMON));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCF | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCF | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCV:
    case RFAL_MODE_POLL_PICOPASS:

      if (((gRFAL.rxBR != RFAL_BR_26p48) && (gRFAL.rxBR != RFAL_BR_52p97))
          || ((gRFAL.txBR != RFAL_BR_1p66) && (gRFAL.txBR != RFAL_BR_26p48))) {
        return NFC_InvalidParameter;
      }

      {
        const struct ST25R3916_ISO15693_StreamConfig *RFal_Iso15693StreamConfig;
        struct ST25R3916_ISO15693_StreamConfig      st25rStreamConf;
        ST25R3916_ISO15693_PhyConfig_t           config;

        config.coding = ((gRFAL.txBR == RFAL_BR_1p66) ? ISO15693_VCD_CODING_1_256 : ISO15693_VCD_CODING_1_4);
        switch (gRFAL.rxBR) {
          case RFAL_BR_52p97:                        /*  PRQA S 2880 # MISRA 2.1 - Inconsistently marked as unreachable code */
            config.speedMode = 1;
            break;
          default:
            config.speedMode = 0;
            break;
        }

        ST25R3916_ISO15693_PhyConfigure(&config, &RFal_Iso15693StreamConfig);

        /* MISRA 11.3 - Cannot point directly into different object type, copy to local var */
        st25rStreamConf.din                  = RFal_Iso15693StreamConfig->din;
        st25rStreamConf.dout                 = RFal_Iso15693StreamConfig->dout;
        st25rStreamConf.report_period_length = RFal_Iso15693StreamConfig->report_period_length;
        st25rStreamConf.useBPSK              = RFal_Iso15693StreamConfig->useBPSK;
        ST25R3916_StreamConfigure((const struct ST25R3916_IO_StreamConfig *)&st25rStreamConf);
      }

      /* Set Analog configurations for this bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_POLL_COMMON));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCV | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCV | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_ACTIVE_P2P:
      /* Set Analog configurations for this bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_POLL_COMMON));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_AP2P | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_AP2P | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_ACTIVE_P2P:
      /* Set Analog configurations for this bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_LISTEN_COMMON));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_AP2P | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_AP2P | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCA:
      /* Set Analog configurations for this bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_LISTEN_COMMON));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCF:
      /* Set Analog configurations for this bit rate */
      RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_LISTEN_COMMON));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCF | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
      RFal_ST25R3916_SetAnalogConfig((RFal_AnalogConfigId)(RFAL_ST25R3916_ANALOG_CONFIG_LISTEN | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCF | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));
      break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCB:
    case RFAL_MODE_NONE:
      return NFC_WrongState;

    /*******************************************************************************/
    default:
      return NFC_Unsupport;
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_GetBitRate(RFal_BitRate *txBR, RFal_BitRate *rxBR)
{
  if ((gRFAL.state == RFAL_STATE_IDLE) || (gRFAL.mode == RFAL_MODE_NONE)) {
    return NFC_WrongState;
  }

  if (txBR != NULL) {
    *txBR = gRFAL.txBR;
  }

  if (rxBR != NULL) {
    *rxBR = gRFAL.rxBR;
  }

  return NFC_OK;
}

/*******************************************************************************/
void RFal_SetErrorHandling(RFal_EHandling eHandling)
{
  switch (eHandling) {
    case ERRORHANDLING_NONE:
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_EMD_SUP_CONF, ST25R3916_REG_EMD_SUP_CONF_emd_emv);
      break;

    case ERRORHANDLING_EMD:
      /* MISRA 16.4: no empty default statement (in case RFAL_SW_EMD is defined) */
#ifndef RFAL_SW_EMD
      ST25R3916_IO_ModifyRegister(ST25R3916_REG_EMD_SUP_CONF,
                              (ST25R3916_REG_EMD_SUP_CONF_emd_emv | ST25R3916_REG_EMD_SUP_CONF_emd_thld_mask),
                              (ST25R3916_REG_EMD_SUP_CONF_emd_emv_on | RFAL_EMVCO_RX_MAXLEN));
#endif /* RFAL_SW_EMD */
      break;
    default:
      /* MISRA 16.4: no empty default statement (a comment being enough) */
      break;
  }

  gRFAL.conf.eHandling = eHandling;
}

/*******************************************************************************/
RFal_EHandling RFal_GetErrorHandling(void)
{
  return gRFAL.conf.eHandling;
}

/*******************************************************************************/
void RFal_SetFDTPoll(uint32_t FDTPoll)
{
  gRFAL.timings.FDTPoll = (FDTPoll < RFAL_ST25R3916_GPT_MAX_1FC) ? FDTPoll : RFAL_ST25R3916_GPT_MAX_1FC;
}

/*******************************************************************************/
uint32_t RFal_GetFDTPoll(void)
{
  return gRFAL.timings.FDTPoll;
}

/*******************************************************************************/
void RFal_SetFDTListen(uint32_t FDTListen)
{
  gRFAL.timings.FDTListen = (FDTListen < RFAL_ST25R3916_MRT_MAX_1FC) ? FDTListen : RFAL_ST25R3916_MRT_MAX_1FC;
}

/*******************************************************************************/
uint32_t RFal_GetFDTListen(void)
{
  return gRFAL.timings.FDTListen;
}

/*******************************************************************************/
void RFal_SetGT(uint32_t GT)
{
  gRFAL.timings.GT = (GT < RFAL_ST25R3916_GT_MAX_1FC) ? GT : RFAL_ST25R3916_GT_MAX_1FC;
}

/*******************************************************************************/
uint32_t RFal_GetGT(void)
{
  return gRFAL.timings.GT;
}

/*******************************************************************************/
bool RFal_IsGTExpired(void)
{
  if (gRFAL.tmr.GT != RFAL_TIMING_NONE) {
    if (!RFal_TimerisExpired(gRFAL.tmr.GT)) {
      return false;
    }
  }
  return true;
}

/*******************************************************************************/
NFC_OpResult RFal_FieldOnAndStartGT(void)
{
  NFC_OpResult ret;

  /* Check if RFAL has been initialized (Oscillator should be running) and also
   * if a direct register access has been performed and left the Oscillator Off */
  if ((!ST25R3916_IO_IsOscOn()) || (gRFAL.state < RFAL_STATE_INIT)) {
    return NFC_WrongState;
  }

  ret = NFC_OK;

  /* Set Analog configurations for Field On event */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_FIELD_ON));

  /*******************************************************************************/
  /* Perform collision avoidance and turn field On if not already On */
  if ((!ST25R3916_IO_IsTxEnabled()) || (!gRFAL.field)) {

    /* Set TARFG: 0 (75us+0ms=75us), GT is fulfilled using a SW timer */
    ST25R3916_IO_WriteRegister(ST25R3916_REG_FIELD_ON_GT, 0U);

    /* Set External Field Detector as: Collision Avoidance Detection */
    ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en_fd_mask, ST25R3916_REG_OP_CONTROL_en_fd_manual_efd_ca);

    /* Use Thresholds set by AnalogConfig */
    ret = ST25R3916_PerformCollisionAvoidance(ST25R3916_CMD_INITIAL_RF_COLLISION, ST25R3916_THRESHOLD_DO_NOT_SET, ST25R3916_THRESHOLD_DO_NOT_SET, (ST25R3916_REG_AUX_nfc_n_mask & gRFAL.timings.nTRFW));

    /* Restore External Field Detector as: Automatics */
    ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en_fd_mask, ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);

    /* n * TRFW timing shall vary  Activity 2.1  3.3.1.1 */
    gRFAL.timings.nTRFW = RFal_GennTRFW(gRFAL.timings.nTRFW);

    gRFAL.field = ST25R3916_IO_IsTxEnabled();

    /* Only turn on Receiver and Transmitter if field was successfully turned On */
    if (gRFAL.field) {
      ST25R3916_IO_TxRxOn(); /* Enable Tx and Rx (Tx is already On)*/
    }
  }


  /*******************************************************************************/
  /* Start GT timer in case the GT value is set */
  if ((gRFAL.timings.GT != RFAL_TIMING_NONE)) {
    /* Ensure that a SW timer doesn't have a lower value then the minimum  */
    RFal_TimerStart(gRFAL.tmr.GT, RFal_Conv1fcToMs(((gRFAL.timings.GT) > RFAL_ST25R3916_GT_MIN_1FC) ? gRFAL.timings.GT : RFAL_ST25R3916_GT_MIN_1FC));
  }

  return ret;
}

/*******************************************************************************/
void RFal_CleanupTransceive(void)
{
  /*******************************************************************************/
  /* Transceive flags                                                            */
  /*******************************************************************************/

  /* Restore default settings on NFCIP1 mode, Receiving parity + CRC bits and manual Tx Parity*/
  ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_ISO14443A_NFC, (ST25R3916_REG_ISO14443A_NFC_no_tx_par | ST25R3916_REG_ISO14443A_NFC_no_rx_par | ST25R3916_REG_ISO14443A_NFC_nfc_f0));

  /* Restore AGC enabled */
  ST25R3916_IO_SetRegisterBits(ST25R3916_REG_RX_CONF2, ST25R3916_REG_RX_CONF2_agc_en);

  /*******************************************************************************/

  /*******************************************************************************/
  /* Transceive timers                                                           */
  /*******************************************************************************/
  gRFAL.tmr.txRx   = RFAL_TIMING_NONE;
  gRFAL.tmr.RXE    = RFAL_TIMING_NONE;
  gRFAL.tmr.PPON2  = RFAL_TIMING_NONE;
  /*******************************************************************************/
  /*******************************************************************************/
  /* Execute Post Transceive Callback                                            */
  /*******************************************************************************/
  if (gRFAL.callbacks.postTxRx != NULL) {
    gRFAL.callbacks.postTxRx();
  }
  /*******************************************************************************/
}

/*******************************************************************************/
void RFal_PrepareTransceive(void)
{
  uint32_t maskInterrupts;
  uint8_t  reg;

  /* If we are in RW or AP2P mode */
  if (!RFal_IsModePassiveListen(gRFAL.mode)) {
    /* Reset receive logic with STOP command */
    ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_STOP);

    /* Reset Rx Gain */
    ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_RESET_RXGAIN);
  } else {
    /* In Passive Listen Mode do not use STOP as it stops FDT timer */
    ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
  }

  /*******************************************************************************/
  /* FDT Poll                                                                    */
  /*******************************************************************************/
  if (gRFAL.timings.FDTPoll != RFAL_TIMING_NONE) {
    /* In Passive communications General Purpose Timer is used to measure FDT Poll */
    if (RFal_IsModePassiveComm(gRFAL.mode)) {   /* Passive Comms */
      /* Configure GPT to start at RX end */
      ST25R3916_SetStartGPTimer((uint16_t)RFal_Conv1fcTo8fc(((gRFAL.timings.FDTPoll < RFAL_FDT_POLL_ADJUSTMENT) ? gRFAL.timings.FDTPoll : (gRFAL.timings.FDTPoll - RFAL_FDT_POLL_ADJUSTMENT))), ST25R3916_REG_TIMER_EMV_CONTROL_gptc_erx);
    }
    /* In Active Poller mode GT PPON1 is used to ensure FDT Poll */
    else if (gRFAL.mode == RFAL_MODE_POLL_ACTIVE_P2P) {
      ST25R3916_IO_WriteRegister(ST25R3916_REG_FIELD_ON_GT, (uint8_t)RFal_Conv1fcTo2018fc(gRFAL.timings.FDTPoll));
    } else {
      /* MISRA 15.7 - Empty else */
    }
  }


  /*******************************************************************************/
  /* Execute Pre Transceive Callback                                             */
  /*******************************************************************************/
  if (gRFAL.callbacks.preTxRx != NULL) {
    gRFAL.callbacks.preTxRx();
  }
  /*******************************************************************************/

  maskInterrupts = (ST25R3916_IRQ_MASK_FWL  | ST25R3916_IRQ_MASK_TXE  |
                    ST25R3916_IRQ_MASK_RXS  | ST25R3916_IRQ_MASK_RXE  |
                    ST25R3916_IRQ_MASK_PAR  | ST25R3916_IRQ_MASK_CRC  |
                    ST25R3916_IRQ_MASK_ERR1 | ST25R3916_IRQ_MASK_ERR2 |
                    ST25R3916_IRQ_MASK_NRE);

  /*******************************************************************************/
  /* Transceive flags                                                            */
  /*******************************************************************************/

  reg = (ST25R3916_REG_ISO14443A_NFC_no_tx_par_off | ST25R3916_REG_ISO14443A_NFC_no_rx_par_off | ST25R3916_REG_ISO14443A_NFC_nfc_f0_off);

  /* Check if NFCIP1 mode is to be enabled */
  if ((gRFAL.TxRx.ctx.flags & (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_ON) != 0U) {
    reg |= ST25R3916_REG_ISO14443A_NFC_nfc_f0;
  }

  /* Check if Parity check is to be skipped and to keep the parity + CRC bits in FIFO */
  if ((gRFAL.TxRx.ctx.flags & (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_KEEP) != 0U) {
    reg |= ST25R3916_REG_ISO14443A_NFC_no_rx_par;
  }

  /* Check if automatic Parity bits is to be disabled */
  if ((gRFAL.TxRx.ctx.flags & (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_NONE) != 0U) {
    reg |= ST25R3916_REG_ISO14443A_NFC_no_tx_par;
  }

  /* Apply current TxRx flags on ISO14443A and NFC 106kb/s Settings Register */
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_ISO14443A_NFC, (ST25R3916_REG_ISO14443A_NFC_no_tx_par | ST25R3916_REG_ISO14443A_NFC_no_rx_par | ST25R3916_REG_ISO14443A_NFC_nfc_f0), reg);

  /* Check if CRC is to be checked automatically upon reception */
  if ((gRFAL.TxRx.ctx.flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_MANUAL) != 0U) {
    ST25R3916_IO_SetRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_no_crc_rx);
  } else {
    ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_no_crc_rx);
  }
  /* Check if AGC is to be disabled */
  if ((gRFAL.TxRx.ctx.flags & (uint32_t)RFAL_TXRX_FLAGS_AGC_OFF) != 0U) {
    ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_RX_CONF2, ST25R3916_REG_RX_CONF2_agc_en);
  } else {
    ST25R3916_IO_SetRegisterBits(ST25R3916_REG_RX_CONF2, ST25R3916_REG_RX_CONF2_agc_en);
  }
  /*******************************************************************************/

  /*******************************************************************************/
  /* EMVCo NRT mode                                                              */
  /*******************************************************************************/
  if (gRFAL.conf.eHandling == ERRORHANDLING_EMD) {
    ST25R3916_IO_SetRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv);
    maskInterrupts |= ST25R3916_IRQ_MASK_RX_REST;
  } else {
    ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv);
  }
  /*******************************************************************************/

  /* In Passive Listen mode additionally enable External Field interrupts  */
  if (RFal_IsModePassiveListen(gRFAL.mode)) {
    maskInterrupts |= (ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_WU_F);        /* Enable external Field interrupts to detect Link Loss and SENF_REQ auto responses */
  }

  /* In Active comms enable also External Field interrupts and set RF Collision Avoidance */
  if (RFal_IsModeActiveComm(gRFAL.mode)) {
    maskInterrupts |= (ST25R3916_IRQ_MASK_EOF  | ST25R3916_IRQ_MASK_EON  | ST25R3916_IRQ_MASK_PPON2 | ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_CAC);
    /* Set n=0 for subsequent RF Collision Avoidance */
    ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_n_mask, 0);
  }

  /*******************************************************************************/
  /* Start transceive Sanity Timer if a FWT is used */
  if ((gRFAL.TxRx.ctx.fwt != RFAL_FWT_NONE) && (gRFAL.TxRx.ctx.fwt != 0U)) {
    RFal_TimerStart(gRFAL.tmr.txRx, RFal_CalcSanityTmr(gRFAL.TxRx.ctx.fwt));
  }
  /*******************************************************************************/

  /*******************************************************************************/
  /* Clear and enable these interrupts */
  ST25R3916_IO_GetInterrupt(maskInterrupts);
  ST25R3916_IO_EnableInterrupts(maskInterrupts);

  /* Clear FIFO status local copy */
  RFal_FIFOStatusClear();
}

/*******************************************************************************/
NFC_OpResult RFal_FieldOff(void)
{
  /* Check whether a TxRx is not yet finished */
  if (gRFAL.TxRx.state != RFAL_TXRX_STATE_IDLE) {
    RFal_CleanupTransceive();
  }

  /* Disable Tx and Rx */
  ST25R3916_IO_TxRxOff();

  /* Set Analog configurations for Field Off event */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_FIELD_OFF));
  gRFAL.field = false;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_StartTransceive(const RFal_TransceiveContext *ctx)
{
  uint32_t FxTAdj;  /* FWT or FDT adjustment calculation */

  /* Check for valid parameters */
  if (ctx == NULL) {
    return NFC_InvalidParameter;
  }

  /* If parity check is disabled CRC check must be disabled as well */
  if (((ctx->flags & (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_KEEP) != 0U) && ((ctx->flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_MANUAL) == 0U)) {
    return NFC_Unsupport;
  }

  /* Ensure that RFAL is already Initialized and the mode has been set */
  if (gRFAL.state >= RFAL_STATE_MODE_SET) {
    /*******************************************************************************/
    /* Check whether the field is already On, otherwise no TXE will be received  */
    if ((!ST25R3916_IO_IsTxEnabled()) && ((!RFal_IsModePassiveListen(gRFAL.mode)) && (ctx->txBuf != NULL))) {
      return NFC_WrongState;
    }

    gRFAL.TxRx.ctx = *ctx;

    /*******************************************************************************/
    if (gRFAL.timings.FDTListen != RFAL_TIMING_NONE) {
      /* Calculate MRT adjustment accordingly to the current mode */
      FxTAdj = RFAL_FDT_LISTEN_MRT_ADJUSTMENT;
      if (gRFAL.mode == RFAL_MODE_POLL_NFCA)      {
        FxTAdj += (uint32_t)RFAL_FDT_LISTEN_A_ADJUSTMENT;
      }
      if (gRFAL.mode == RFAL_MODE_POLL_NFCA_T1T)  {
        FxTAdj += (uint32_t)RFAL_FDT_LISTEN_A_ADJUSTMENT;
      }
      if (gRFAL.mode == RFAL_MODE_POLL_NFCB)      {
        FxTAdj += (uint32_t)RFAL_FDT_LISTEN_B_ADJUSTMENT;
      }
      if (gRFAL.mode == RFAL_MODE_POLL_NFCV)      {
        FxTAdj += (uint32_t)RFAL_FDT_LISTEN_V_ADJUSTMENT;
      }

      /* Ensure that MRT is using 64/fc steps */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_mrt_step);


      /* If Correlator is being used further adjustment is required for NFCB */
      if (gRFAL.mode == RFAL_MODE_POLL_NFCB) {
        if (ST25R3916_IO_CheckReg(ST25R3916_REG_AUX, ST25R3916_REG_AUX_dis_corr, 0x00U)) {
          FxTAdj += (uint32_t)RFAL_FDT_LISTEN_B_ADJT_CORR;                                                                                        /* Reduce FDT(Listen)                   */
          ST25R3916_IO_SetRegisterBits(ST25R3916_REG_CORR_CONF1, ST25R3916_REG_CORR_CONF1_corr_s3);                                                   /* Ensure BPSK start to 33 pilot pulses */
          ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_SUBC_START_TIME, ST25R3916_REG_SUBC_START_TIME_sst_mask, RFAL_FDT_LISTEN_B_ADJT_CORR_SST);    /* Set sst                              */
        }
      }


      /* Set Minimum FDT(Listen) in which PICC is not allowed to send a response */
      ST25R3916_IO_WriteRegister(ST25R3916_REG_MASK_RX_TIMER, (uint8_t)RFal_Conv1fcTo64fc((FxTAdj > gRFAL.timings.FDTListen) ? RFAL_ST25R3916_MRT_MIN_1FC : (gRFAL.timings.FDTListen - FxTAdj)));
    }

    /*******************************************************************************/
    /* FDT Poll will be loaded in RFal_PrepareTransceive() once the previous was expired */

    /*******************************************************************************/
    if ((gRFAL.TxRx.ctx.fwt != RFAL_FWT_NONE) && (gRFAL.TxRx.ctx.fwt != 0U)) {
      /* Ensure proper timing configuration */
      if (gRFAL.timings.FDTListen >= gRFAL.TxRx.ctx.fwt) {
        return NFC_InvalidParameter;
      }

      FxTAdj = RFAL_FWT_ADJUSTMENT;
      if (gRFAL.mode == RFAL_MODE_POLL_NFCA)      {
        FxTAdj += (uint32_t)RFAL_FWT_A_ADJUSTMENT;
      }
      if (gRFAL.mode == RFAL_MODE_POLL_NFCA_T1T)  {
        FxTAdj += (uint32_t)RFAL_FWT_A_ADJUSTMENT;
      }
      if (gRFAL.mode == RFAL_MODE_POLL_NFCB)      {
        FxTAdj += (uint32_t)RFAL_FWT_B_ADJUSTMENT;
      }
      if ((gRFAL.mode == RFAL_MODE_POLL_NFCF) || (gRFAL.mode == RFAL_MODE_POLL_ACTIVE_P2P)) {
        FxTAdj += (uint32_t)((gRFAL.txBR == RFAL_BR_212) ? RFAL_FWT_F_212_ADJUSTMENT : RFAL_FWT_F_424_ADJUSTMENT);
      }

      /* Ensure that the given FWT doesn't exceed NRT maximum */
      gRFAL.TxRx.ctx.fwt = ((gRFAL.TxRx.ctx.fwt + FxTAdj) < RFAL_ST25R3916_NRT_MAX_1FC) ? (gRFAL.TxRx.ctx.fwt + FxTAdj) : RFAL_ST25R3916_NRT_MAX_1FC;

      /* Set FWT in the NRT */
      ST25R3916_SetNoResponseTime(RFal_Conv1fcTo64fc(gRFAL.TxRx.ctx.fwt));
    } else {
      /* Disable NRT, no NRE will be triggered, therefore wait endlessly for Rx */
      ST25R3916_SetNoResponseTime(RFAL_ST25R3916_NRT_DISABLED);
    }

    gRFAL.state       = RFAL_STATE_TXRX;
    gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_IDLE;
    gRFAL.TxRx.status = NFC_Busy;

    /*******************************************************************************/
    if ((RFAL_MODE_POLL_NFCV == gRFAL.mode) || (RFAL_MODE_POLL_PICOPASS == gRFAL.mode)) {
      /* Exchange receive buffer with internal buffer */
      gRFAL.nfcvData.origCtx = gRFAL.TxRx.ctx;

      gRFAL.TxRx.ctx.rxBuf    = ((gRFAL.nfcvData.origCtx.rxBuf != NULL) ? gRFAL.nfcvData.codingBuffer : NULL);
      gRFAL.TxRx.ctx.rxBufLen = (uint16_t)RFal_ConvBytesToBits(sizeof(gRFAL.nfcvData.codingBuffer));
      gRFAL.TxRx.ctx.flags = (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL
                             | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP
                             | (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_OFF
                             | (uint32_t)(gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_AGC_OFF)
                             | (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_KEEP
                             | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_NONE;

      /* In NFCV a TxRx with a valid txBuf and txBufSize==0 indicates to send an EOF */
      /* Skip logic below that would go directly into receive                        */
      if (gRFAL.TxRx.ctx.txBuf != NULL) {
        return  NFC_OK;
      }
    }


#ifdef ST25R3916B
    /* Check if ST25R3916 AWS is enabled and AP2P */
    if (ST25R3916_IO_CheckReg(ST25R3916_REG_AUX_MOD, ST25R3916_REG_AUX_MOD_rgs_am, ST25R3916_REG_AUX_MOD_rgs_am) && RFal_IsModeActiveComm(gRFAL.mode)) {
      /* If ST25R3916 with AWS set again the current mode to reload AWS config */
      RFal_SetMode(gRFAL.mode, gRFAL.txBR, gRFAL.rxBR);
    }
#endif /* ST25R3916B */


    /*******************************************************************************/
    /* Check if the Transceive start performing Tx or goes directly to Rx          */
    if ((gRFAL.TxRx.ctx.txBuf == NULL) || (gRFAL.TxRx.ctx.txBufLen == 0U)) {
      /* Clear FIFO, Clear and Enable the Interrupts */
      RFal_PrepareTransceive();

      /* In AP2P check the field status */
      if (RFal_IsModeActiveComm(gRFAL.mode)) {
        /* Disable our field upon a Rx re-enable, and start PPON2 manually */
        ST25R3916_IO_TxOff();
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_START_PPON2_TIMER);
      }

      /* No Tx done, enable the Receiver */
      ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

      /* Start NRT manually, if FWT = 0 (wait endlessly for Rx) chip will ignore anyhow */
      ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_START_NO_RESPONSE_TIMER);

      gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_IDLE;
    }

    return NFC_OK;
  }

  return NFC_WrongState;
}

/*******************************************************************************/
bool RFal_IsTransceiveInTx(void)
{
  return ((gRFAL.TxRx.state >= RFAL_TXRX_STATE_TX_IDLE) && (gRFAL.TxRx.state < RFAL_TXRX_STATE_RX_IDLE));
}

/*******************************************************************************/
bool RFal_IsTransceiveInRx(void)
{
  return (gRFAL.TxRx.state >= RFAL_TXRX_STATE_RX_IDLE);
}

/*******************************************************************************/
NFC_OpResult RFal_TransceiveBlockingTx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt)
{
  NFC_OpResult               ret;
  RFal_TransceiveContext    ctx;

  RFal_CreateByteFlagsTxRxContext(ctx, txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt);
  ret = RFal_StartTransceive(&ctx);
  if(ret < NFC_OK)
  {
    return ret;
  }

  return RFal_TransceiveRunBlockingTx();
}

/*******************************************************************************/
NFC_OpResult RFal_TransceiveRunBlockingTx(void)
{
  NFC_OpResult ret;

  do {
    RFal_Worker();
    ret = RFal_GetTransceiveStatus();
  } while ((RFal_IsTransceiveInTx()) && (ret == NFC_Busy));

  if (RFal_IsTransceiveInRx()) {
    return NFC_OK;
  }

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_TransceiveBlockingRx(void)
{
  NFC_OpResult ret;

  do {
    RFal_Worker();
    ret = RFal_GetTransceiveStatus();
  } while ((RFal_IsTransceiveInRx()) || (ret == NFC_Busy));

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_TransceiveBlockingTxRx(uint8_t *txBuf, uint16_t txBufLen, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen, uint32_t flags, uint32_t fwt)
{
  NFC_OpResult ret;

  ret = RFal_TransceiveBlockingTx(txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt);
  if(ret < NFC_OK)
  {
    return ret;
  }

  ret = RFal_TransceiveBlockingRx();

  /* Convert received bits to bytes */
  if (actLen != NULL) {
    *actLen = RFal_ConvBitsToBytes(*actLen);
  }

  return ret;
}

/*******************************************************************************/
static NFC_OpResult RFal_RunTransceiveWorker(void)
{
  if (gRFAL.state == RFAL_STATE_TXRX) {
    /*******************************************************************************/
    /* Check Transceive Sanity Timer has expired */
    if (gRFAL.tmr.txRx != RFAL_TIMING_NONE) {
      if (RFal_TimerisExpired(gRFAL.tmr.txRx)) {
        /* If sanity timer has expired abort ongoing transceive and signal error */
        gRFAL.TxRx.status = NFC_IO_Error;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
      }
    }

    /*******************************************************************************/
    /* Run Tx or Rx state machines */
    if (RFal_IsTransceiveInTx()) {
      RFal_TransceiveTx();
      return RFal_GetTransceiveStatus();
    }
    if (RFal_IsTransceiveInRx()) {
      RFal_TransceiveRx();
      return RFal_GetTransceiveStatus();
    }
  }
  return NFC_WrongState;
}

/*******************************************************************************/
RFal_TransceiveState RFal_GetTransceiveState(void)
{
  return gRFAL.TxRx.state;
}

/*******************************************************************************/
NFC_OpResult RFal_GetTransceiveStatus(void)
{
  return ((gRFAL.TxRx.state == RFAL_TXRX_STATE_IDLE) ? gRFAL.TxRx.status : NFC_Busy);
}

/*******************************************************************************/
NFC_OpResult RFal_GetTransceiveRSSI(uint16_t *rssi)
{
  uint16_t amRSSI;
  uint16_t pmRSSI;
  bool     isSumMode;

  if (rssi == NULL) {
    return NFC_InvalidParameter;
  }

  ST25R3916_GetRSSI(&amRSSI, &pmRSSI);

  /* Check if Correlator Summation mode is being used */
  isSumMode = (ST25R3916_IO_CheckReg(ST25R3916_REG_CORR_CONF1, ST25R3916_REG_CORR_CONF1_corr_s4, ST25R3916_REG_CORR_CONF1_corr_s4) ? ST25R3916_IO_CheckReg(ST25R3916_REG_AUX, ST25R3916_REG_AUX_dis_corr, 0x00) : false);
  if (isSumMode) {
    /*******************************************************************************/
    /* Usage of SQRT from math.h and float. Due to compiler, resources or          *
     * performance issues sqrt is not enabled by default. Using a less accuracy    *
     * accurate approach such as: average, max value, etc                           */

#ifdef RFAL_ACCURATE_RSSI
    *rssi = (uint16_t) sqrt(((double)amRSSI * (double)amRSSI) + ((double)pmRSSI * (double)pmRSSI));             /*  PRQA S 5209 # MISRA 4.9 - External function (sqrt()) requires double */
#else
    *rssi = ((amRSSI + pmRSSI) / 2U);
#endif
  } else {
    /* Check which channel was used */
    *rssi = (ST25R3916_IO_CheckReg(ST25R3916_REG_AUX_DISPLAY, ST25R3916_REG_AUX_DISPLAY_a_cha, ST25R3916_REG_AUX_DISPLAY_a_cha) ? pmRSSI : amRSSI);
  }
  return NFC_OK;
}

/*******************************************************************************/
bool RFal_IsTransceiveSubcDetected(void)
{
  return false;
}
/*******************************************************************************/
void RFal_Worker(void)
{
  switch (gRFAL.state) {
    case RFAL_STATE_TXRX:
      RFal_RunTransceiveWorker();
      break;
    case RFAL_STATE_LM:
      RFal_RunListenModeWorker();
      break;
    case RFAL_STATE_WUM:
      RFal_RunWakeUpModeWorker();
      break;

    /* Nothing to be done */
    default:
      /* MISRA 16.4: no empty default statement (a comment being enough) */
      break;
  }

}

/*******************************************************************************/
void RFal_ErrorHandling(void)
{
  uint16_t fifoBytesToRead;

  fifoBytesToRead = RFal_FIFOStatusGetNumBytes();
#ifdef RFAL_SW_EMD
  /*******************************************************************************/
  /* EMVCo                                                                       */
  /*******************************************************************************/
  if (gRFAL.conf.eHandling == ERRORHANDLING_EMD) {
    bool    rxHasIncParError;

    /*******************************************************************************/
    /* EMD Handling - Digital 2.1  4.1.1.1 ; EMVCo 3.0  4.9.2 ; ISO 14443-3  8.3   */
    /* Re-enable the receiver on frames with a length < 4 bytes, upon:              */
    /*   - Collision or Framing error detected                                     */
    /*   - Residual bits are detected (hard framing error)                         */
    /*   - Parity error                                                            */
    /*   - CRC error                                                               */
    /*******************************************************************************/

    /* Check if reception has incomplete bytes or parity error */
    rxHasIncParError = (RFal_FIFOStatusIsIncompleteByte() ? true : RFal_FIFOStatusIsMissingPar());     /* MISRA 13.5 */


    /* In case there are residual bits decrement FIFO bytes */
    /* Ensure FIFO contains some byte as the FIFO might be empty upon Framing errors */
    if ((fifoBytesToRead > 0U) && rxHasIncParError) {
      fifoBytesToRead--;
    }

    if (((gRFAL.fifo.bytesTotal + fifoBytesToRead) < RFAL_EMVCO_RX_MAXLEN)            &&
        ((gRFAL.TxRx.status == NFC_RF_Collision) || (gRFAL.TxRx.status == NFC_FramingError) ||
         (gRFAL.TxRx.status == NFC_ParityError)          || (gRFAL.TxRx.status == NFC_CRC_Error)     ||
         rxHasIncParError)) {
      /* Ignore this reception, Re-enable receiver which also clears the FIFO */
      ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);


      /* Ensure that the NRT has not expired meanwhile */
      if (ST25R3916_IO_CheckReg(ST25R3916_REG_NFCIP1_BIT_RATE, ST25R3916_REG_NFCIP1_BIT_RATE_nrt_on, 0x00)) {
        if (ST25R3916_IO_CheckReg(ST25R3916_REG_AUX_DISPLAY, ST25R3916_REG_AUX_DISPLAY_rx_act, 0x00)) {
          /* Abort reception */
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_MASK_RECEIVE_DATA);
          gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
          return;
        }
      }


      RFal_FIFOStatusClear();
      gRFAL.fifo.bytesTotal = 0;
      gRFAL.TxRx.status     = NFC_Busy;
      gRFAL.TxRx.state      = RFAL_TXRX_STATE_RX_WAIT_RXS;
    }
    return;
  }
#endif


  /*******************************************************************************/
  /* ISO14443A Mode                                                              */
  /*******************************************************************************/
  if (gRFAL.mode == RFAL_MODE_POLL_NFCA) {

    /*******************************************************************************/
    /* If we received a frame with a incomplete byte we`ll raise a specific error  *
     * ( support for T2T 4 bit ACK / NAK, MIFARE and Kovio )                       */
    /*******************************************************************************/
    if ((gRFAL.TxRx.status == NFC_ParityError) || (gRFAL.TxRx.status == NFC_CRC_Error)) {
      if ((RFal_FIFOStatusIsIncompleteByte()) && (fifoBytesToRead == RFAL_RX_INC_BYTE_LEN)) {
        ST25R3916_IO_ReadFifo((uint8_t *)(gRFAL.TxRx.ctx.rxBuf), fifoBytesToRead);
        if ((gRFAL.TxRx.ctx.rxRcvdLen) != NULL) {
          *gRFAL.TxRx.ctx.rxRcvdLen = RFal_FIFOGetNumIncompleteBits();
        }

        gRFAL.TxRx.status = NFC_ImcompleteByte;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
      }
    }
  }

}

/*******************************************************************************/
void RFal_TransceiveTx(void)
{
  volatile uint32_t irqs;
  uint16_t          tmp;
  NFC_OpResult        ret;

  /* Suppress warning in case NFC-V feature is disabled */
  ret = NFC_OK;

  irqs = ST25R3916_IRQ_MASK_NONE;

  if (gRFAL.TxRx.state != gRFAL.TxRx.lastState) {
    /* RFal_LogD( "RFAL: lastSt: %d curSt: %d \r\n", gRFAL.TxRx.lastState, gRFAL.TxRx.state ); */
    gRFAL.TxRx.lastState = gRFAL.TxRx.state;
  }

  switch (gRFAL.TxRx.state) {
    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_IDLE:

      /* Nothing to do */

      gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_WAIT_GT ;
    /* fall through */


    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_GT:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      if (!RFal_IsGTExpired()) {
        break;
      }

      gRFAL.tmr.GT = RFAL_TIMING_NONE;

      gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_WAIT_FDT;
    /* fall through */


    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_FDT:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      /* Only in Passive communications GPT is used to measure FDT Poll */
      if (RFal_IsModePassiveComm(gRFAL.mode)) {
        if (ST25R3916_IO_IsGPTRunning()) {
          break;
        }
      }

      gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_PREP_TX;
    /* fall through */


    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_PREP_TX:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */
      /* Clear FIFO, Clear and Enable the Interrupts */
      RFal_PrepareTransceive();

      /* ST25R3916 has a fixed FIFO water level */
      gRFAL.fifo.expWL = RFAL_FIFO_OUT_WL;

      /*******************************************************************************/
      /* In NFC-V streaming mode, the FIFO needs to be loaded with the coded bits    */
      if ((RFAL_MODE_POLL_NFCV == gRFAL.mode) || (RFAL_MODE_POLL_PICOPASS == gRFAL.mode)) {
#if 0
        /* Debugging code: output the payload bits by writing into the FIFO and subsequent clearing */
        ST25R3916_IO_WriteFifo(gRFAL.TxRx.ctx.txBuf, RFal_ConvBitsToBytes(gRFAL.TxRx.ctx.txBufLen));
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
#endif
        /* Calculate the bytes needed to be Written into FIFO (a incomplete byte will be added as 1byte) */
        gRFAL.nfcvData.nfcvOffset = 0;
        ret = ST25R3916_ISO15693_VCDCode(gRFAL.TxRx.ctx.txBuf, 
                                          RFal_ConvBitsToBytes(gRFAL.TxRx.ctx.txBufLen), 
                                          (((gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL) != 0U) ? false : true), 
                                          (((gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_MANUAL) != 0U) ? false : true), 
                                          (RFAL_MODE_POLL_PICOPASS == gRFAL.mode),
                                          &gRFAL.fifo.bytesTotal, 
                                          &gRFAL.nfcvData.nfcvOffset, 
                                          gRFAL.nfcvData.codingBuffer, 
                                          ((uint16_t)ST25R3916_FIFO_DEPTH < (uint16_t)sizeof(gRFAL.nfcvData.codingBuffer)) ? ST25R3916_FIFO_DEPTH : (uint16_t)sizeof(gRFAL.nfcvData.codingBuffer), 
                                          &gRFAL.fifo.bytesWritten);

        if ((ret != NFC_OK) && (ret != NFC_Again)) {
          gRFAL.TxRx.status = ret;
          gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_FAIL;
          break;
        }
        /* Set the number of full bytes and bits to be transmitted */
        ST25R3916_SetNumTxBits((uint16_t)RFal_ConvBytesToBits(gRFAL.fifo.bytesTotal));

        /* Load FIFO with coded bytes */
        ST25R3916_IO_WriteFifo(gRFAL.nfcvData.codingBuffer, gRFAL.fifo.bytesWritten);

      }
      /*******************************************************************************/
      else
      {
        /* Calculate the bytes needed to be Written into FIFO (a incomplete byte will be added as 1byte) */
        gRFAL.fifo.bytesTotal = (uint16_t)RFal_CalcNumBytes(gRFAL.TxRx.ctx.txBufLen);

        /* Set the number of full bytes and bits to be transmitted */
        ST25R3916_SetNumTxBits(gRFAL.TxRx.ctx.txBufLen);

        /* Load FIFO with total length or FIFO's maximum */
        gRFAL.fifo.bytesWritten = (gRFAL.fifo.bytesTotal < ST25R3916_FIFO_DEPTH) ? gRFAL.fifo.bytesTotal : ST25R3916_FIFO_DEPTH;
        ST25R3916_IO_WriteFifo(gRFAL.TxRx.ctx.txBuf, gRFAL.fifo.bytesWritten);
      }

      /*Check if Observation Mode is enabled and set it on ST25R391x */
      RFal_CheckEnableObsModeTx();


      /*******************************************************************************/
      /* If we're in Passive Listen mode ensure that the external field is still On  */
      if (RFal_IsModePassiveListen(gRFAL.mode)) {
        if (!RFal_IsExtFieldOn()) {
          gRFAL.TxRx.status = NFC_LinkLoss;
          gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_FAIL;
          break;
        }
      }

      gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_TRANSMIT;
    /* fall through */


    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_TRANSMIT:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      /*******************************************************************************/
      /* Execute Sync Transceive Callback                                             */
      /*******************************************************************************/
      if (gRFAL.callbacks.syncTxRx != NULL) {
        /* If set, wait for sync callback to signal sync/trigger transmission */
        if (!gRFAL.callbacks.syncTxRx()) {
          break;
        }
      }

      /*******************************************************************************/
      /* Trigger/Start transmission                                                  */
      if ((gRFAL.TxRx.ctx.flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL) != 0U) {
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_TRANSMIT_WITHOUT_CRC);
      } else {
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_TRANSMIT_WITH_CRC);
      }

      /* Check if a WL level is expected or TXE should come */
      gRFAL.TxRx.state = ((gRFAL.fifo.bytesWritten < gRFAL.fifo.bytesTotal) ? RFAL_TXRX_STATE_TX_WAIT_WL : RFAL_TXRX_STATE_TX_WAIT_TXE);
      break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_WL:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_FWL | ST25R3916_IRQ_MASK_TXE));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      if (((irqs & ST25R3916_IRQ_MASK_FWL) != 0U) && ((irqs & ST25R3916_IRQ_MASK_TXE) == 0U)) {
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_RELOAD_FIFO;
      } else {
        gRFAL.TxRx.status = NFC_IO_Error;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_FAIL;
        break;
      }

    /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_RELOAD_FIFO:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      /*******************************************************************************/
      /* In NFC-V streaming mode, the FIFO needs to be loaded with the coded bits    */
      if ((RFAL_MODE_POLL_NFCV == gRFAL.mode) || (RFAL_MODE_POLL_PICOPASS == gRFAL.mode)) {
        uint16_t maxLen;

        /* Load FIFO with the remaining length or maximum available (which fit on the coding buffer) */
        maxLen = (uint16_t)((gRFAL.fifo.bytesTotal - gRFAL.fifo.bytesWritten) < gRFAL.fifo.expWL ? (gRFAL.fifo.bytesTotal - gRFAL.fifo.bytesWritten) : gRFAL.fifo.expWL);
        maxLen = (uint16_t)(maxLen < sizeof(gRFAL.nfcvData.codingBuffer) ? maxLen : sizeof(gRFAL.nfcvData.codingBuffer));
        tmp    = 0;

        /* Calculate the bytes needed to be Written into FIFO (a incomplete byte will be added as 1byte) */
        ret = ST25R3916_ISO15693_VCDCode(gRFAL.TxRx.ctx.txBuf, RFal_ConvBitsToBytes(gRFAL.TxRx.ctx.txBufLen), (((gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL) != 0U) ? false : true), (((gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_MANUAL) != 0U) ? false : true), (RFAL_MODE_POLL_PICOPASS == gRFAL.mode),
                                  &gRFAL.fifo.bytesTotal, &gRFAL.nfcvData.nfcvOffset, gRFAL.nfcvData.codingBuffer, maxLen, &tmp);

        if ((ret != NFC_OK) && (ret != NFC_Again)) {
          gRFAL.TxRx.status = ret;
          gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_FAIL;
          break;
        }

        /* Load FIFO with coded bytes */
        ST25R3916_IO_WriteFifo(gRFAL.nfcvData.codingBuffer, tmp);
      }
      /*******************************************************************************/
      else
      {
        /* Load FIFO with the remaining length or maximum available */
        tmp = (gRFAL.fifo.bytesTotal - gRFAL.fifo.bytesWritten) < gRFAL.fifo.expWL ? (gRFAL.fifo.bytesTotal - gRFAL.fifo.bytesWritten) : gRFAL.fifo.expWL;
        ST25R3916_IO_WriteFifo(&gRFAL.TxRx.ctx.txBuf[gRFAL.fifo.bytesWritten], tmp);
      }

      /* Update total written bytes to FIFO */
      gRFAL.fifo.bytesWritten += tmp;

      /* Check if a WL level is expected or TXE should come */
      gRFAL.TxRx.state = ((gRFAL.fifo.bytesWritten < gRFAL.fifo.bytesTotal) ? RFAL_TXRX_STATE_TX_WAIT_WL : RFAL_TXRX_STATE_TX_WAIT_TXE);
      break;


    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_TXE:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_FWL | ST25R3916_IRQ_MASK_TXE));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }


      if ((irqs & ST25R3916_IRQ_MASK_TXE) != 0U) {
        gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_DONE;
      } else if ((irqs & ST25R3916_IRQ_MASK_FWL) != 0U) {
        break;  /* Ignore ST25R3916 FIFO WL if total TxLen is already on the FIFO */
      } else {
        gRFAL.TxRx.status = NFC_IO_Error;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_FAIL;
        break;
      }

    /* fall through */


    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_DONE:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      /* If no rxBuf is provided do not wait/expect Rx */
      if (gRFAL.TxRx.ctx.rxBuf == NULL) {
        /*Check if Observation Mode was enabled and disable it on ST25R391x */
        RFal_CheckDisableObsMode();

        /* Clean up Transceive */
        RFal_CleanupTransceive();

        gRFAL.TxRx.status = NFC_OK;
        gRFAL.TxRx.state  =  RFAL_TXRX_STATE_IDLE;
        break;
      }

      RFal_CheckEnableObsModeRx();

      /* Goto Rx */
      gRFAL.TxRx.state  =  RFAL_TXRX_STATE_RX_IDLE;
      break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_FAIL:

      /* Error should be assigned by previous state */
      if (gRFAL.TxRx.status == NFC_Busy) {
        gRFAL.TxRx.status = NFC_System;
      }

      /*Check if Observation Mode was enabled and disable it on ST25R391x */
      RFal_CheckDisableObsMode();

      /* Clean up Transceive */
      RFal_CleanupTransceive();

      gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
      break;

    /*******************************************************************************/
    default:
      gRFAL.TxRx.status = NFC_System;
      gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_FAIL;
      break;
  }
}

/*******************************************************************************/
void RFal_TransceiveRx(void)
{
  volatile uint32_t irqs;
  uint16_t          tmp;
  uint16_t          aux;

  irqs = ST25R3916_IRQ_MASK_NONE;

  if (gRFAL.TxRx.state != gRFAL.TxRx.lastState) {
#if 0 /* Debug purposes */
    RFal_LogD("RFAL: lastSt: %d curSt: %d \r\n", gRFAL.TxRx.lastState, gRFAL.TxRx.state);
#endif

    gRFAL.TxRx.lastState = gRFAL.TxRx.state;
  }

  switch (gRFAL.TxRx.state) {
    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_IDLE:

      /* Clear rx counters */
      gRFAL.fifo.bytesWritten   = 0;            /* Total bytes written on RxBuffer         */
      gRFAL.fifo.bytesTotal     = 0;            /* Total bytes in FIFO will now be from Rx */
      if (gRFAL.TxRx.ctx.rxRcvdLen != NULL) {
        *gRFAL.TxRx.ctx.rxRcvdLen = 0;
      }

      /*******************************************************************************/
      /* REMARK: Silicon workaround ST25R3916 Errata #2.1.3                          */
      RFal_TimerStart(gRFAL.tmr.PPON2, 10U);
      /*******************************************************************************/

      gRFAL.TxRx.state = (RFal_IsModeActiveComm(gRFAL.mode) ? RFAL_TXRX_STATE_RX_WAIT_EON : RFAL_TXRX_STATE_RX_WAIT_RXS);
      break;


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_WAIT_RXS:

      /*******************************************************************************/
      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_RXS | ST25R3916_IRQ_MASK_NRE | ST25R3916_IRQ_MASK_EOF));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      /* Only raise Timeout if NRE is detected with no Rx Start (NRT EMV mode) */
      if (((irqs & ST25R3916_IRQ_MASK_NRE) != 0U) && ((irqs & ST25R3916_IRQ_MASK_RXS) == 0U)) {
        gRFAL.TxRx.status = NFC_SlaveTimeout;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
        break;
      }

      /* Only raise Link Loss if EOF is detected with no Rx Start */
      if (((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) && ((irqs & ST25R3916_IRQ_MASK_RXS) == 0U)) {
        /* In AP2P a Field On has already occurred - treat this as timeout | mute */
        gRFAL.TxRx.status = (RFal_IsModeActiveComm(gRFAL.mode) ? NFC_SlaveTimeout : NFC_LinkLoss);
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
        break;
      }

      if ((irqs & ST25R3916_IRQ_MASK_RXS) != 0U) {
        /*******************************************************************************/
        /* REMARK: Silicon workaround ST25R3916 Errata #2.1.2                          */
        /* Rarely on corrupted frames I_rxs gets signaled but I_rxe is not signaled    */
        /* Use a SW timer to handle an eventual missing RXE                            */
        RFal_TimerStart(gRFAL.tmr.RXE, RFAL_NORXE_TOUT);
        /*******************************************************************************/

        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_WAIT_RXE;
      } else {
        gRFAL.TxRx.status = NFC_IO_Error;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
        break;
      }

      /* remove NRE that might appear together (NRT EMV mode), and remove RXS, but keep EOF if present for next state */
      irqs &= ~(ST25R3916_IRQ_MASK_RXS | ST25R3916_IRQ_MASK_NRE);

    /* fall through */


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_WAIT_RXE:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */


      irqs |= ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_RXE  | ST25R3916_IRQ_MASK_FWL | ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_RX_REST | ST25R3916_IRQ_MASK_WU_F));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        /*******************************************************************************/
        /* REMARK: Silicon workaround ST25R3916 Errata #2.1.2                          */
        /* ST25R396 may indicate RXS without RXE afterwards, this happens rarely on    */
        /* corrupted frames.                                                           */
        /* SW timer is used to timeout upon a missing RXE                              */
        if (RFal_TimerisExpired(gRFAL.tmr.RXE)) {
          gRFAL.TxRx.status = NFC_FramingError;
          gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
        }
        /*******************************************************************************/

        break;  /* No interrupt to process */
      }

      if ((irqs & ST25R3916_IRQ_MASK_RX_REST) != 0U) {
        /* RX_REST indicates that Receiver has been reset due to EMD, therefore a RXS + RXE should *
         * follow if a good reception is followed within the valid initial timeout                   */

        /* Check whether NRT has expired already, if so signal a timeout */
        if (ST25R3916_IO_GetInterrupt(ST25R3916_IRQ_MASK_NRE) != 0U) {
          gRFAL.TxRx.status = NFC_SlaveTimeout;
          gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
          break;
        }
        if (ST25R3916_IO_CheckReg(ST25R3916_REG_NFCIP1_BIT_RATE, ST25R3916_REG_NFCIP1_BIT_RATE_nrt_on, 0)) {    /* MISRA 13.5 */
          gRFAL.TxRx.status = NFC_SlaveTimeout;
          gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
          break;
        }

        /* Discard any previous RXS */
        ST25R3916_IO_GetInterrupt(ST25R3916_IRQ_MASK_RXS);

        /* Check whether a following reception has already started */
        if (ST25R3916_IO_CheckReg(ST25R3916_REG_AUX_DISPLAY, ST25R3916_REG_AUX_DISPLAY_rx_act, ST25R3916_REG_AUX_DISPLAY_rx_act)) {
          gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_WAIT_RXE;
          break;
        }

        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_WAIT_RXS;
        break;
      }

      if (((irqs & ST25R3916_IRQ_MASK_FWL) != 0U) && ((irqs & ST25R3916_IRQ_MASK_RXE) == 0U)) {
        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_FIFO;
        break;
      }

      /* Automatic responses allowed during TxRx only for the SENSF_REQ */
      if ((irqs & ST25R3916_IRQ_MASK_WU_F) != 0U) {
        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXS;
        break;
      }

      /* After RXE retrieve and check for any error irqs */
      irqs |= ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_ERR1 | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_COL));

      gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_ERR_CHECK;
    /* fall through */


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_ERR_CHECK:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      if ((irqs & ST25R3916_IRQ_MASK_ERR1) != 0U) {
        gRFAL.TxRx.status = NFC_FramingError;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_READ_DATA;

        /* Check if there's a specific error handling for this */
        RFal_ErrorHandling();
        break;
      }
      /* Discard Soft Framing errors in AP2P and CE */
      /* Discard Soft Framing errors in CTS as Correlator does not support no_eof */
      else if ((RFal_IsModePassivePoll(gRFAL.mode)) && ((irqs & ST25R3916_IRQ_MASK_ERR2) != 0U) && (gRFAL.mode != RFAL_MODE_POLL_B_CTS)) {
        gRFAL.TxRx.status = NFC_FramingError;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_READ_DATA;

        /* Check if there's a specific error handling for this */
        RFal_ErrorHandling();
        break;
      } else if ((irqs & ST25R3916_IRQ_MASK_PAR) != 0U) {
        gRFAL.TxRx.status = NFC_ParityError;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_READ_DATA;

        /* Check if there's a specific error handling for this */
        RFal_ErrorHandling();
        break;
      } else if ((irqs & ST25R3916_IRQ_MASK_CRC) != 0U) {
        gRFAL.TxRx.status = NFC_CRC_Error;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_READ_DATA;

        /* Check if there's a specific error handling for this */
        RFal_ErrorHandling();
        break;
      } else if ((irqs & ST25R3916_IRQ_MASK_COL) != 0U) {
        gRFAL.TxRx.status = NFC_RF_Collision;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_READ_DATA;

        /* Check if there's a specific error handling for this */
        RFal_ErrorHandling();
        break;
      } else if (RFal_IsModePassiveListen(gRFAL.mode) && ((irqs & ST25R3916_IRQ_MASK_EOF) != 0U)) {
        gRFAL.TxRx.status = NFC_LinkLoss;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
        break;
      } else if ((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) {
        /* Reception ended without any error indication,                  *
         * check FIFO status for malformed or incomplete frames           */

        /* Check if the reception ends with an incomplete byte (residual bits) */
        if (RFal_FIFOStatusIsIncompleteByte()) {
          gRFAL.TxRx.status = NFC_ImcompleteByte;
        }
        /* Check if the reception ends missing parity bit */
        else if (RFal_FIFOStatusIsMissingPar()) {
          gRFAL.TxRx.status = NFC_FramingError;
        } else {
          /* MISRA 15.7 - Empty else */
        }

        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_DATA;
      } else {
        gRFAL.TxRx.status = NFC_IO_Error;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
        break;
      }

    /* fall through */


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_READ_DATA:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      tmp = RFal_FIFOStatusGetNumBytes();

      /*******************************************************************************/
      /* Check if CRC should not be placed in rxBuf                                  */
      if (((gRFAL.TxRx.ctx.flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP) == 0U)) {
        /* if received frame was bigger than CRC */
        if ((uint16_t)(gRFAL.fifo.bytesTotal + tmp) > 0U) {
          /* By default CRC will not be placed into the rxBuffer */
          if ((tmp > RFAL_CRC_LEN)) {
            tmp -= RFAL_CRC_LEN;
          }
          /* If the CRC was already placed into rxBuffer (due to WL interrupt where CRC was already in FIFO Read)
           * cannot remove it from rxBuf. Can only remove it from rxBufLen not indicate the presence of CRC    */
          else if (gRFAL.fifo.bytesTotal > RFAL_CRC_LEN) {
            gRFAL.fifo.bytesTotal -= RFAL_CRC_LEN;
          } else {
            /* MISRA 15.7 - Empty else */
          }
        }
      }

      gRFAL.fifo.bytesTotal += tmp;                    /* add to total bytes counter */

      /*******************************************************************************/
      /* Check if remaining bytes fit on the rxBuf available                         */
      if (gRFAL.fifo.bytesTotal > RFal_ConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen)) {
        tmp = (uint16_t)(RFal_ConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen) - gRFAL.fifo.bytesWritten);

        /* Transmission errors have precedence over buffer error */
        if (gRFAL.TxRx.status == NFC_Busy) {
          gRFAL.TxRx.status = NFC_MemoryError;
        }
      }

      /*******************************************************************************/
      /* Retrieve remaining bytes from FIFO to rxBuf, and assign total length rcvd   */
      ST25R3916_IO_ReadFifo(&gRFAL.TxRx.ctx.rxBuf[gRFAL.fifo.bytesWritten], tmp);
      if (gRFAL.TxRx.ctx.rxRcvdLen != NULL) {
        (*gRFAL.TxRx.ctx.rxRcvdLen) = (uint16_t)RFal_ConvBytesToBits(gRFAL.fifo.bytesTotal);
        if (RFal_FIFOStatusIsIncompleteByte()) {
          (*gRFAL.TxRx.ctx.rxRcvdLen) -= (RFAL_BITS_IN_BYTE - RFal_FIFOGetNumIncompleteBits());
        }
      }

      /*******************************************************************************/
      /* Decode sub bit stream into payload bits for NFCV, if no error found so far  */
      if (((RFAL_MODE_POLL_NFCV == gRFAL.mode) || (RFAL_MODE_POLL_PICOPASS == gRFAL.mode)) && (gRFAL.TxRx.status == NFC_Busy)) {
        NFC_OpResult ret;
        uint16_t offset = 0; /* REMARK offset not currently used */

        ret = ST25R3916_ISO15693_VICCDecode(gRFAL.TxRx.ctx.rxBuf, gRFAL.fifo.bytesTotal,
                                            gRFAL.nfcvData.origCtx.rxBuf, RFal_ConvBitsToBytes(gRFAL.nfcvData.origCtx.rxBufLen), &offset, gRFAL.nfcvData.origCtx.rxRcvdLen, gRFAL.nfcvData.ignoreBits, (RFAL_MODE_POLL_PICOPASS == gRFAL.mode));

        if (((NFC_OK == ret) || (NFC_CRC_Error == ret))
            && (((uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP & gRFAL.nfcvData.origCtx.flags) == 0U)
            && ((*gRFAL.nfcvData.origCtx.rxRcvdLen % RFAL_BITS_IN_BYTE) == 0U)
            && (*gRFAL.nfcvData.origCtx.rxRcvdLen >= RFal_ConvBytesToBits(RFAL_CRC_LEN))
           ) {
          *gRFAL.nfcvData.origCtx.rxRcvdLen -= (uint16_t)RFal_ConvBytesToBits(RFAL_CRC_LEN); /* Remove CRC */
        }
#if 0
        /* Debugging code: output the payload bits by writing into the FIFO and subsequent clearing */
        ST25R3916_IO_WriteFifo(gRFAL.nfcvData.origCtx.rxBuf, RFal_ConvBitsToBytes(*gRFAL.nfcvData.origCtx.rxRcvdLen));
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
#endif

        /* Restore original ctx */
        gRFAL.TxRx.ctx    = gRFAL.nfcvData.origCtx;
        gRFAL.TxRx.status = ((ret != NFC_OK) ? ret : NFC_Busy);
      }

      if (RFal_IsModeActiveComm(gRFAL.mode)) {
        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_EOF;
        break;
      }

      /*******************************************************************************/
      /* If an error as been marked/detected don't fall into to RX_DONE  */
      if (gRFAL.TxRx.status != NFC_Busy) {
        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
        break;
      }

      gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_DONE;
    /* fall through */


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_DONE:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      /*Check if Observation Mode was enabled and disable it on ST25R391x */
      RFal_CheckDisableObsMode();

      /* Clean up Transceive */
      RFal_CleanupTransceive();


      gRFAL.TxRx.status = NFC_OK;
      gRFAL.TxRx.state  = RFAL_TXRX_STATE_IDLE;
      break;


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_READ_FIFO:

      /*******************************************************************************/
      /* REMARK: Silicon workaround ST25R3916 Errata #2.1.2                          */
      /* Rarely on corrupted frames I_rxs gets signaled but I_rxe is not signaled    */
      /* Use a SW timer to handle an eventual missing RXE                            */
      RFal_TimerStart(gRFAL.tmr.RXE, RFAL_NORXE_TOUT);
      /*******************************************************************************/

      tmp = RFal_FIFOStatusGetNumBytes();
      gRFAL.fifo.bytesTotal += tmp;

      /*******************************************************************************/
      /* Calculate the amount of bytes that still fits in rxBuf                      */
      aux = ((gRFAL.fifo.bytesTotal > RFal_ConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen)) ? (RFal_ConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen) - gRFAL.fifo.bytesWritten) : tmp);

      /*******************************************************************************/
      /* Retrieve incoming bytes from FIFO to rxBuf, and store already read amount   */
      ST25R3916_IO_ReadFifo(&gRFAL.TxRx.ctx.rxBuf[gRFAL.fifo.bytesWritten], aux);
      gRFAL.fifo.bytesWritten += aux;

      /*******************************************************************************/
      /* If the bytes already read were not the full FIFO WL, dump the remaining     *
       * FIFO so that ST25R391x can continue with reception                          */
      if (aux < tmp) {
        ST25R3916_IO_ReadFifo(NULL, (tmp - aux));
      }

      RFal_FIFOStatusClear();
      gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_WAIT_RXE;
      break;


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_FAIL:

      /*Check if Observation Mode was enabled and disable it on ST25R391x */
      RFal_CheckDisableObsMode();

      /* Clean up Transceive */
      RFal_CleanupTransceive();

      /* Error should be assigned by previous state */
      if (gRFAL.TxRx.status == NFC_Busy) {
        gRFAL.TxRx.status = NFC_System;
      }

#if 0 /* Debug purposes */
      RFal_LogD("RFAL: curSt: %d  Error: %d \r\n", gRFAL.TxRx.state, gRFAL.TxRx.status);
#endif

      gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
      break;


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_WAIT_EON:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_EON | ST25R3916_IRQ_MASK_NRE | ST25R3916_IRQ_MASK_PPON2));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        /*******************************************************************************/
        /* REMARK: Silicon workaround ST25R3916 Errata #2.1.3                          */
        if (RFal_TimerisExpired(gRFAL.tmr.PPON2)) {
          gRFAL.TxRx.status = NFC_LinkLoss;
          gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
        }
        /*******************************************************************************/

        break;  /* No interrupt to process */
      }

      if ((irqs & ST25R3916_IRQ_MASK_EON) != 0U) {
        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXS;

#ifdef ST25R3916B
        /* Check if ST25R3916 AWS is enabled */
        if (ST25R3916_IO_CheckReg(ST25R3916_REG_AUX_MOD, ST25R3916_REG_AUX_MOD_rgs_am, ST25R3916_REG_AUX_MOD_rgs_am)) {
          /* Set Analog configurations for our own following Field On */
          RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_FIELD_ON));
        }
#endif /* ST25R3916B */
      }

      if ((irqs & ST25R3916_IRQ_MASK_NRE) != 0U) {
        gRFAL.TxRx.status = NFC_SlaveTimeout;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
      }
      if ((irqs & ST25R3916_IRQ_MASK_PPON2) != 0U) {
        gRFAL.TxRx.status = NFC_LinkLoss;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
      }
      break;


    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_WAIT_EOF:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_CAC));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      if ((irqs & ST25R3916_IRQ_MASK_CAT) != 0U) {
        /* Check if an error has been marked/detected before */
        gRFAL.TxRx.state = ((gRFAL.TxRx.status != NFC_Busy) ? RFAL_TXRX_STATE_RX_FAIL : RFAL_TXRX_STATE_RX_DONE);
      } else if ((irqs & ST25R3916_IRQ_MASK_CAC) != 0U) {
        gRFAL.TxRx.status = NFC_RF_Collision;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
      } else {
        gRFAL.TxRx.status = NFC_IO_Error;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
      }
      break;


    /*******************************************************************************/
    default:
      gRFAL.TxRx.status = NFC_System;
      gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
      break;
  }
}

/*******************************************************************************/
NFC_OpResult RFal_ISO14443ATransceiveShortFrame(RFal_14443AShortFrameCmd txCmd, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *rxRcvdLen, uint32_t fwt)
{
  NFC_OpResult ret;
  uint8_t    directCmd;

  /* Check if RFAL is properly initialized */
  if ((!ST25R3916_IO_IsTxEnabled()) || (gRFAL.state < RFAL_STATE_MODE_SET) || ((gRFAL.mode != RFAL_MODE_POLL_NFCA) && (gRFAL.mode != RFAL_MODE_POLL_NFCA_T1T))) {
    return NFC_WrongState;
  }

  /* Check for valid parameters */
  if ((rxBuf == NULL) || (rxRcvdLen == NULL) || (fwt == RFAL_FWT_NONE)) {
    return NFC_InvalidParameter;
  }

  /*******************************************************************************/
  /* Select the Direct Command to be performed                                   */
  switch (txCmd) {
    case RFAL_14443A_SHORTFRAME_CMD_WUPA:
      directCmd = ST25R3916_CMD_TRANSMIT_WUPA;
      break;

    case RFAL_14443A_SHORTFRAME_CMD_REQA:
      directCmd = ST25R3916_CMD_TRANSMIT_REQA;
      break;

    default:
      return NFC_InvalidParameter;
  }

  /*******************************************************************************/
  /* Wait for GT and FDT */
  while (!RFal_IsGTExpired())      { /* MISRA 15.6: mandatory brackets */ };
  while (ST25R3916_IO_IsGPTRunning()) { /* MISRA 15.6: mandatory brackets */ };

  gRFAL.tmr.GT = RFAL_TIMING_NONE;


  /*******************************************************************************/
  /* Prepare for Transceive, Receive only (bypass Tx states) */
  gRFAL.TxRx.ctx.flags     = ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_MANUAL);
  gRFAL.TxRx.ctx.rxBuf     = rxBuf;
  gRFAL.TxRx.ctx.rxBufLen  = rxBufLen;
  gRFAL.TxRx.ctx.rxRcvdLen = rxRcvdLen;
  gRFAL.TxRx.ctx.fwt       = fwt;


  /*******************************************************************************/
  /* Load NRT with FWT */
  ST25R3916_SetNoResponseTime(RFal_Conv1fcTo64fc(((fwt + RFAL_FWT_ADJUSTMENT + RFAL_FWT_A_ADJUSTMENT) < RFAL_ST25R3916_NRT_MAX_1FC) ? (fwt + RFAL_FWT_ADJUSTMENT + RFAL_FWT_A_ADJUSTMENT) : RFAL_ST25R3916_NRT_MAX_1FC));

  if (gRFAL.timings.FDTListen != RFAL_TIMING_NONE) {

    /* Ensure that MRT is using 64/fc steps */
    ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_mrt_step);

    /* Set Minimum FDT(Listen) in which PICC is not allowed to send a response */
    ST25R3916_IO_WriteRegister(ST25R3916_REG_MASK_RX_TIMER, (uint8_t)RFal_Conv1fcTo64fc(((RFAL_FDT_LISTEN_MRT_ADJUSTMENT + RFAL_FDT_LISTEN_A_ADJUSTMENT) > gRFAL.timings.FDTListen) ? RFAL_ST25R3916_MRT_MIN_1FC : (gRFAL.timings.FDTListen - (RFAL_FDT_LISTEN_MRT_ADJUSTMENT + RFAL_FDT_LISTEN_A_ADJUSTMENT))));
  }

  /* In Passive communications General Purpose Timer is used to measure FDT Poll */
  if (gRFAL.timings.FDTPoll != RFAL_TIMING_NONE) {
    /* Configure GPT to start at RX end */
    ST25R3916_SetStartGPTimer((uint16_t)RFal_Conv1fcTo8fc(((gRFAL.timings.FDTPoll < RFAL_FDT_POLL_ADJUSTMENT) ? gRFAL.timings.FDTPoll : (gRFAL.timings.FDTPoll - RFAL_FDT_POLL_ADJUSTMENT))), ST25R3916_REG_TIMER_EMV_CONTROL_gptc_erx);
  }


  /*******************************************************************************/
  RFal_PrepareTransceive();

  /* Also enable bit collision interrupt */
  ST25R3916_IO_GetInterrupt(ST25R3916_IRQ_MASK_COL);
  ST25R3916_IO_EnableInterrupts(ST25R3916_IRQ_MASK_COL);

  /*Check if Observation Mode is enabled and set it on ST25R391x */
  RFal_CheckEnableObsModeTx();

  /*******************************************************************************/
  /* Clear nbtx bits before sending WUPA/REQA - otherwise ST25R3916 will report parity error, Note2 of the register */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_NUM_TX_BYTES2, 0);

  /* Send either WUPA or REQA. All affected tags will backscatter ATQA and change to READY state */
  ST25R3916_IO_ExecuteCommand(directCmd);

  /* Wait for TXE */
  if (ST25R3916_IO_WaitForInterruptsTimed(ST25R3916_IRQ_MASK_TXE, (uint16_t)(RFal_Conv1fcToMs(fwt) > RFAL_ST25R3916_SW_TMR_MIN_1MS) ? RFal_Conv1fcToMs(fwt) : RFAL_ST25R3916_SW_TMR_MIN_1MS) == 0U) {
    ret = NFC_IO_Error;
  } else {
    /*Check if Observation Mode is enabled and set it on ST25R391x */
    RFal_CheckEnableObsModeRx();

    /* Jump into a transceive Rx state for reception (bypass Tx states) */
    gRFAL.state       = RFAL_STATE_TXRX;
    gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_IDLE;
    gRFAL.TxRx.status = NFC_Busy;

    /* Execute Transceive Rx blocking */
    ret = RFal_TransceiveBlockingRx();
  }

  /* Disable Collision interrupt */
  ST25R3916_IO_DisableInterrupts((ST25R3916_IRQ_MASK_COL));

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO14443ATransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt)
{
  NFC_OpResult            ret;

  ret = RFal_ISO14443AStartTransceiveAnticollisionFrame(buf, bytesToSend, bitsToSend, rxLength, fwt);
  if(ret < NFC_OK)
  {
    return ret;
  }
  
  do{
    ret=RFal_ISO14443AGetTransceiveAnticollisionFrameStatus();
    RFal_Worker();
  }
  while( ret == NFC_Busy );

  return ret;
}
/*******************************************************************************/
NFC_OpResult RFal_ISO14443AStartTransceiveAnticollisionFrame(uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt)
{
  NFC_OpResult            ret;
  RFal_TransceiveContext ctx;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCA)) {
    return NFC_WrongState;
  }

  /* Check for valid parameters */
  if ((buf == NULL) || (bytesToSend == NULL) || (bitsToSend == NULL) || (rxLength == NULL)) {
    return NFC_InvalidParameter;
  }

  /*******************************************************************************/
  /* Set specific Analog Config for Anticollision if needed */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_ANTICOL));


  /*******************************************************************************/
  /* Enable anti collision to recognise collision in first byte of SENS_REQ */
  ST25R3916_IO_SetRegisterBits(ST25R3916_REG_ISO14443A_NFC, ST25R3916_REG_ISO14443A_NFC_antcl);


  /*******************************************************************************/
  /* Prepare for Transceive                                                      */
  ctx.flags     = ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_MANUAL);
  ctx.txBuf     = buf;
  ctx.txBufLen  = (uint16_t)(RFal_ConvBytesToBits(*bytesToSend) + *bitsToSend);
  ctx.rxBuf     = &buf[*bytesToSend];
  ctx.rxBufLen  = (uint16_t)RFal_ConvBytesToBits(RFAL_ISO14443A_SDD_RES_LEN);
  ctx.rxRcvdLen = rxLength;
  ctx.fwt       = fwt;

  /* Disable Automatic Gain Control (AGC) for better detection of collisions if using Coherent Receiver */
  ctx.flags    |= (ST25R3916_IO_CheckReg(ST25R3916_REG_AUX, ST25R3916_REG_AUX_dis_corr, ST25R3916_REG_AUX_dis_corr) ? (uint32_t)RFAL_TXRX_FLAGS_AGC_OFF : 0x00U);


  ret = RFal_StartTransceive(&ctx);
  if(ret < NFC_OK)
  {
    return ret;
  }

  /* Additionally enable bit collision interrupt */
  ST25R3916_IO_GetInterrupt(ST25R3916_IRQ_MASK_COL);
  ST25R3916_IO_EnableInterrupts(ST25R3916_IRQ_MASK_COL);

  /*******************************************************************************/
  gRFAL.nfcaData.collByte = 0;

  /* Save the collision byte */
  if ((*bitsToSend) > 0U) {
    buf[(*bytesToSend)] <<= (RFAL_BITS_IN_BYTE - (*bitsToSend));
    buf[(*bytesToSend)] >>= (RFAL_BITS_IN_BYTE - (*bitsToSend));
    gRFAL.nfcaData.collByte = buf[(*bytesToSend)];
  }


  gRFAL.nfcaData.buf         = buf;
  gRFAL.nfcaData.bytesToSend = bytesToSend;
  gRFAL.nfcaData.bitsToSend  = bitsToSend;
  gRFAL.nfcaData.rxLength    = rxLength;


  /*******************************************************************************/
  /* Run Transceive Tx */
  return RFal_TransceiveRunBlockingTx();

}

/*******************************************************************************/
NFC_OpResult RFal_ISO14443AGetTransceiveAnticollisionFrameStatus(void)
{
  NFC_OpResult   ret;
  uint8_t      collData;

  ret = RFal_GetTransceiveStatus();
  if (ret == NFC_Busy)
  {
    return ret;
  }

  /*******************************************************************************/
  if ((*gRFAL.nfcaData.bitsToSend) > 0U) {
    gRFAL.nfcaData.buf[(*gRFAL.nfcaData.bytesToSend)] >>= (*gRFAL.nfcaData.bitsToSend);
    gRFAL.nfcaData.buf[(*gRFAL.nfcaData.bytesToSend)] <<= (*gRFAL.nfcaData.bitsToSend);
    gRFAL.nfcaData.buf[(*gRFAL.nfcaData.bytesToSend)] |= gRFAL.nfcaData.collByte;
  }

  if (ret == NFC_RF_Collision) {
    /* Read out collision register */
    ST25R3916_IO_ReadRegister(ST25R3916_REG_COLLISION_STATUS, &collData);

    (*gRFAL.nfcaData.bytesToSend) = ((collData >> ST25R3916_REG_COLLISION_STATUS_c_byte_shift) & 0x0FU); // 4-bits Byte information
    (*gRFAL.nfcaData.bitsToSend)  = ((collData >> ST25R3916_REG_COLLISION_STATUS_c_bit_shift)  & 0x07U); // 3-bits bit information
  }


  /*******************************************************************************/
  /* Disable Collision interrupt */
  ST25R3916_IO_DisableInterrupts((ST25R3916_IRQ_MASK_COL));

  /* Disable anti collision again */
  ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_ISO14443A_NFC, ST25R3916_REG_ISO14443A_NFC_antcl);


  /*******************************************************************************/
  /* Restore common Analog configurations for this mode */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCA | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO15693TransceiveAnticollisionFrame(uint8_t *txBuf, uint8_t txBufLen, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
  NFC_OpResult            ret;
  RFal_TransceiveContext ctx;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCV)) {
    return NFC_WrongState;
  }

  /*******************************************************************************/
  /* Set specific Analog Config for Anticolission if needed */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCV | RFAL_ST25R3916_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ST25R3916_ANALOG_CONFIG_ANTICOL));


  /* Ignoring collisions before the UID (RES_FLAG + DSFID) */
  gRFAL.nfcvData.ignoreBits = (uint16_t)RFAL_ISO15693_IGNORE_BITS;

  /*******************************************************************************/
  /* Prepare for Transceive  */
  ctx.flags     = ((txBufLen == 0U) ? (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL : (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_AUTO) | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_AGC_OFF | ((txBufLen == 0U) ? (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_MANUAL : (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO); /* Disable Automatic Gain Control (AGC) for better detection of collision */
  ctx.txBuf     = txBuf;
  ctx.txBufLen  = (uint16_t)RFal_ConvBytesToBits(txBufLen);
  ctx.rxBuf     = rxBuf;
  ctx.rxBufLen  = (uint16_t)RFal_ConvBytesToBits(rxBufLen);
  ctx.rxRcvdLen = actLen;
  ctx.fwt       = RFal_Conv64fcTo1fc(ISO15693_FWT);

  ret = RFal_StartTransceive(&ctx);
  if(ret < NFC_OK)
  {
    return ret;
  }

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = RFal_TransceiveRunBlockingTx();
  if (ret == NFC_OK) {
    ret = RFal_TransceiveBlockingRx();
  }

  /* Check if a Transmission error and received data is less then expected */
  if (((ret == NFC_RF_Collision) || (ret == NFC_CRC_Error) || (ret == NFC_FramingError)) && (RFal_ConvBitsToBytes(*ctx.rxRcvdLen) < RFAL_ISO15693_INV_RES_LEN)) {
    /* If INVENTORY_RES is shorter than expected, tag is still modulating *
     * Ensure that response is complete before next frame                 */
    NeonRTOS_Sleep((uint8_t)((RFAL_ISO15693_INV_RES_LEN - RFal_ConvBitsToBytes(*ctx.rxRcvdLen)) / ((RFAL_ISO15693_INV_RES_LEN / RFAL_ISO15693_INV_RES_DUR) + 1U)));
  }

  /* Restore common Analog configurations for this mode */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCV | RFal_ConvBR2ACBR(gRFAL.txBR) | RFAL_ST25R3916_ANALOG_CONFIG_TX));
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_POLL | RFAL_ST25R3916_ANALOG_CONFIG_TECH_NFCV | RFal_ConvBR2ACBR(gRFAL.rxBR) | RFAL_ST25R3916_ANALOG_CONFIG_RX));

  gRFAL.nfcvData.ignoreBits = 0;
  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ISO15693TransceiveEOFAnticollision(uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
  uint8_t dummy;

  return RFal_ISO15693TransceiveAnticollisionFrame(&dummy, 0, rxBuf, rxBufLen, actLen);
}

/*******************************************************************************/
NFC_OpResult RFal_ISO15693TransceiveEOF(uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *actLen)
{
  NFC_OpResult ret;
  uint8_t    dummy;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCV)) {
    return NFC_WrongState;
  }

  /*******************************************************************************/
  /* Run Transceive blocking */
  ret = RFal_TransceiveBlockingTxRx(&dummy,
                                   0,
                                   rxBuf,
                                   rxBufLen,
                                   actLen,
                                   ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_AGC_ON),
                                   RFal_Conv64fcTo1fc(ISO15693_FWT));
  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_FeliCaPoll(RFal_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, RFal_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected)
{
  NFC_OpResult ret;

  ret = RFal_StartFeliCaPoll(slots, sysCode, reqCode, pollResList, pollResListSize, devicesDetected, collisionsDetected);
  if(ret < NFC_OK)
  {
    return ret;
  }

  do{
    ret=RFal_GetFeliCaPollStatus();
    RFal_Worker();
  }
  while( ret == NFC_Busy );

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_StartFeliCaPoll(RFal_FeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, RFal_FeliCaPollRes *pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected)
{
  NFC_OpResult        ret;
  uint8_t           frame[RFAL_FELICA_POLL_REQ_LEN - RFAL_FELICA_LEN_LEN];  // LEN is added by ST25R391x automatically
  uint8_t           frameIdx;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCF)) {
    return NFC_WrongState;
  }

  frameIdx                   = 0;
  gRFAL.nfcfData.colDetected = 0;
  gRFAL.nfcfData.devDetected = 0;

  /*******************************************************************************/
  /* Compute SENSF_REQ frame */
  frame[frameIdx++] = (uint8_t)FELICA_CMD_POLLING; /* CMD: SENF_REQ                       */
  frame[frameIdx++] = (uint8_t)(sysCode >> 8);     /* System Code (SC)                    */
  frame[frameIdx++] = (uint8_t)(sysCode & 0xFFU);  /* System Code (SC)                    */
  frame[frameIdx++] = reqCode;                     /* Communication Parameter Request (RC)*/
  frame[frameIdx++] = (uint8_t)slots;              /* TimeSlot (TSN)                      */


  /*******************************************************************************/
  /* NRT should not stop on reception - Fake EMD which uses NRT in nrt_emv       *
   * ERRORHANDLING_EMD has no special handling for NFC-F mode               */
  gRFAL.nfcfData.curHandling = gRFAL.conf.eHandling;
  gRFAL.conf.eHandling       = ERRORHANDLING_EMD;

  /*******************************************************************************/
  /* Run transceive blocking,
   * Calculate Total Response Time in(64/fc):
   *                       512 PICC process time + (n * 256 Time Slot duration)  */
  ret = RFal_TransceiveBlockingTx(frame,
                                  (uint16_t)frameIdx,
                                  (uint8_t *)gRFAL.nfcfData.pollResponses,
                                  RFAL_FELICA_POLL_RES_LEN,
                                  &gRFAL.nfcfData.actLen,
                                  (RFAL_TXRX_FLAGS_DEFAULT),
                                  RFal_Conv64fcTo1fc(RFAL_FELICA_POLL_DELAY_TIME + (RFAL_FELICA_POLL_SLOT_TIME * ((uint32_t)slots + 1U))));
  if(ret < NFC_OK)
  {
    return ret;
  }

  /* Store context */
  gRFAL.nfcfData.pollResList        = pollResList;
  gRFAL.nfcfData.pollResListSize    = pollResListSize;
  gRFAL.nfcfData.devicesDetected    = devicesDetected;
  gRFAL.nfcfData.collisionsDetected = collisionsDetected;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_GetFeliCaPollStatus(void)
{
  NFC_OpResult ret;

  /* Check if RFAL is properly initialized */
  if ((gRFAL.state != RFAL_STATE_TXRX) || (gRFAL.mode != RFAL_MODE_POLL_NFCF)) {
    return NFC_WrongState;
  }

  /* Wait until transceive has terminated */
  ret = RFal_GetTransceiveStatus();
  if (ret == NFC_Busy)
  {
    return ret;
  }

  /* Upon timeout the full Poll Delay + (Slot time)*(nbSlots) has expired */
  if (ret != NFC_SlaveTimeout) {
    /* Reception done, re-enabled Rx for following Slot */
    ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);
    ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_RESET_RXGAIN);
    RFal_FIFOStatusClear();

    /* If the reception was OK, new device found */
    if (ret == NFC_OK) {
      gRFAL.nfcfData.devDetected++;

      /* Overwrite the Transceive context for the next reception */
      gRFAL.TxRx.ctx.rxBuf = (uint8_t *)gRFAL.nfcfData.pollResponses[gRFAL.nfcfData.devDetected];
    }
    /* If the reception was not OK, mark as collision */
    else {
      gRFAL.nfcfData.colDetected++;
    }

    /* Check whether that NRT has not expired meanwhile */
    if (ST25R3916_IO_CheckReg(ST25R3916_REG_NFCIP1_BIT_RATE, ST25R3916_REG_NFCIP1_BIT_RATE_nrt_on, ST25R3916_REG_NFCIP1_BIT_RATE_nrt_on)) {
      /* Jump again into transceive Rx state for the following reception */
      gRFAL.TxRx.status = NFC_Busy;
      gRFAL.state       = RFAL_STATE_TXRX;
      gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_IDLE;
      return NFC_Busy;
    }
  }


  /*******************************************************************************/
  /* Back to previous error handling (restore NRT to normal mode)                */
  gRFAL.conf.eHandling = gRFAL.nfcfData.curHandling;

  /*******************************************************************************/
  /* Assign output parameters if requested                                       */
  if ((gRFAL.nfcfData.pollResList != NULL) && (gRFAL.nfcfData.pollResListSize > 0U) && (gRFAL.nfcfData.devDetected > 0U)) {
    memcpy(gRFAL.nfcfData.pollResList, gRFAL.nfcfData.pollResponses, (RFAL_FELICA_POLL_RES_LEN * (uint32_t)((gRFAL.nfcfData.pollResListSize < gRFAL.nfcfData.devDetected) ? gRFAL.nfcfData.pollResListSize : gRFAL.nfcfData.devDetected)));
  }

  if (gRFAL.nfcfData.devicesDetected != NULL) {
    *gRFAL.nfcfData.devicesDetected = gRFAL.nfcfData.devDetected;
  }

  if (gRFAL.nfcfData.collisionsDetected != NULL) {
    *gRFAL.nfcfData.collisionsDetected = gRFAL.nfcfData.colDetected;
  }

  return (((gRFAL.nfcfData.colDetected != 0U) || (gRFAL.nfcfData.devDetected != 0U)) ? NFC_OK : ret);
}

/*******************************************************************************/
bool RFal_IsExtFieldOn(void)
{
  return ST25R3916_IO_IsExtFieldOn();
}

/*******************************************************************************/
NFC_OpResult RFal_ListenStart(uint32_t lmMask, const RFal_LmConfPA *confA, const RFal_LmConfPB *confB, const RFal_LmConfPF *confF, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen)
{
  t_RFal_PTMem PTMem;        /*  PRQA S 0759 # MISRA 19.2 - Allocating Union where members are of the same type, just different names.  Thus no problem can occur. */
  uint8_t    *pPTMem;
  uint8_t     autoResp;


  /* Check if RFAL is initialized */
  if (gRFAL.state < RFAL_STATE_INIT) {
    return NFC_WrongState;
  }

  gRFAL.Lm.state  = RFAL_LM_STATE_NOT_INIT;
  gRFAL.Lm.mdIrqs = ST25R3916_IRQ_MASK_NONE;
  gRFAL.Lm.mdReg  = (ST25R3916_REG_MODE_targ_init | ST25R3916_REG_MODE_om_nfc | ST25R3916_REG_MODE_nfc_ar_off);


  /* By default disable all automatic responses */
  autoResp = (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a | ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r | ST25R3916_REG_PASSIVE_TARGET_d_ac_ap2p);

  /*******************************************************************************/
  if ((lmMask & RFAL_LM_MASK_NFCA) != 0U) {
    /* Check if the conf has been provided */
    if (confA == NULL) {
      return NFC_InvalidParameter;
    }

    pPTMem = (uint8_t *)PTMem.PTMem_A;

    /*******************************************************************************/
    /* Check and set supported NFCID Length */
    switch (confA->nfcidLen) {
      case RFAL_LM_NFCID_LEN_04:
        ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_id_mask, ST25R3916_REG_AUX_nfc_id_4bytes);
        break;

      case RFAL_LM_NFCID_LEN_07:
        ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_id_mask, ST25R3916_REG_AUX_nfc_id_7bytes);
        break;

      default:
        return NFC_InvalidParameter;
    }

    /*******************************************************************************/
    /* Set NFCID */
    memcpy(pPTMem, confA->nfcid, RFAL_NFCID1_TRIPLE_LEN);
    pPTMem = &pPTMem[RFAL_NFCID1_TRIPLE_LEN];                  /* MISRA 18.4 */

    /* Set SENS_RES */
    memcpy(pPTMem, confA->SENS_RES, RFAL_LM_SENS_RES_LEN);
    pPTMem = &pPTMem[RFAL_LM_SENS_RES_LEN];             /* MISRA 18.4 */

    /* Set SEL_RES */
    *(pPTMem++) = ((confA->nfcidLen == RFAL_LM_NFCID_LEN_04) ? (confA->SEL_RES & ~RFAL_LM_NFCID_INCOMPLETE) : (confA->SEL_RES | RFAL_LM_NFCID_INCOMPLETE));
    *(pPTMem++) = (confA->SEL_RES & ~RFAL_LM_NFCID_INCOMPLETE);
    *(pPTMem++) = (confA->SEL_RES & ~RFAL_LM_NFCID_INCOMPLETE);

    /* Write into PTMem-A */
    ST25R3916_IO_WritePTMem(PTMem.PTMem_A, ST25R3916_PTM_A_LEN);


    /*******************************************************************************/
    /* Enable automatic responses for A */
    autoResp &= ~ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a;

    /* Set Target mode, Bit Rate detection and Listen Mode for NFC-A */
    gRFAL.Lm.mdReg  |= (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om0 | ST25R3916_REG_MODE_nfc_ar_off);

    gRFAL.Lm.mdIrqs |= (ST25R3916_IRQ_MASK_WU_A | ST25R3916_IRQ_MASK_WU_A_X | ST25R3916_IRQ_MASK_RXE_PTA);
  }

  /*******************************************************************************/
  if ((lmMask & RFAL_LM_MASK_NFCB) != 0U) {
    /* Check if the conf has been provided */
    if (confB == NULL) {
      return NFC_InvalidParameter;
    }

    return NFC_Unsupport;
  }

  /*******************************************************************************/
  if ((lmMask & RFAL_LM_MASK_NFCF) != 0U) {
    pPTMem = (uint8_t *)PTMem.PTMem_F;

    /* Check if the conf has been provided */
    if (confF == NULL) {
      return NFC_InvalidParameter;
    }

    /*******************************************************************************/
    /* Set System Code */
    memcpy(pPTMem, confF->SC, RFAL_LM_SENSF_SC_LEN);
    pPTMem = &pPTMem[RFAL_LM_SENSF_SC_LEN];             /* MISRA 18.4 */

    /* Set SENSF_RES */
    memcpy(pPTMem, confF->SENSF_RES, RFAL_LM_SENSF_RES_LEN);

    /* Set RD bytes to 0x00 as ST25R3916 cannot support advances features */
    pPTMem[RFAL_LM_SENSF_RD0_POS] = 0x00;   /* NFC Forum Digital 1.1 Table 46: 0x00                   */
    pPTMem[RFAL_LM_SENSF_RD1_POS] = 0x00;   /* NFC Forum Digital 1.1 Table 47: No automatic bit rates */

    pPTMem = &pPTMem[RFAL_LM_SENS_RES_LEN];             /* MISRA 18.4 */

    /* Write into PTMem-F */
    ST25R3916_IO_WritePTMemF(PTMem.PTMem_F, ST25R3916_PTM_F_LEN);


    /*******************************************************************************/
    /* Write 24 TSN "Random" Numbers at first initialization and let it rollover   */
    if (!gRFAL.Lm.iniFlag) {
      pPTMem = (uint8_t *)PTMem.TSN;

      *(pPTMem++) = 0x12;
      *(pPTMem++) = 0x34;
      *(pPTMem++) = 0x56;
      *(pPTMem++) = 0x78;
      *(pPTMem++) = 0x9A;
      *(pPTMem++) = 0xBC;
      *(pPTMem++) = 0xDF;
      *(pPTMem++) = 0x21;
      *(pPTMem++) = 0x43;
      *(pPTMem++) = 0x65;
      *(pPTMem++) = 0x87;
      *(pPTMem++) = 0xA9;

      /* Write into PTMem-TSN */
      ST25R3916_IO_WritePTMemTSN(PTMem.TSN, ST25R3916_PTM_TSN_LEN);
    }

    /*******************************************************************************/
    /* Enable automatic responses for F */
    autoResp &= ~(ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r);

    /* Set Target mode, Bit Rate detection and Listen Mode for NFC-F */
    gRFAL.Lm.mdReg  |= (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om2 | ST25R3916_REG_MODE_nfc_ar_off);

    /* In CE NFC-F any data without error will be passed to FIFO, to support CUP */
    gRFAL.Lm.mdIrqs |= (ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE_PTA | ST25R3916_IRQ_MASK_RXE);
  }


  /*******************************************************************************/
  if ((lmMask & RFAL_LM_MASK_ACTIVE_P2P) != 0U) {
    /* Enable Reception of P2P frames */
    autoResp &= ~(ST25R3916_REG_PASSIVE_TARGET_d_ac_ap2p);

    /* Set Target mode, Bit Rate detection and Automatic Response RF Collision Avoidance */
    gRFAL.Lm.mdReg  |= (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om2 | ST25R3916_REG_MODE_om0 | ST25R3916_REG_MODE_nfc_ar_auto_rx);

    /* Ensure CRC check is enabled */
    ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_no_crc_rx);

    /* n * TRFW timing shall vary  Activity 2.1  3.4.1.1 */
    ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_n_mask, gRFAL.timings.nTRFW);
    gRFAL.timings.nTRFW = RFal_GennTRFW(gRFAL.timings.nTRFW);

    gRFAL.Lm.mdIrqs |= (ST25R3916_IRQ_MASK_RXE);
  }


  /* Check if one of the modes were selected */
  if ((gRFAL.Lm.mdReg & ST25R3916_REG_MODE_targ) == ST25R3916_REG_MODE_targ_targ) {
    gRFAL.state     = RFAL_STATE_LM;
    gRFAL.Lm.mdMask = lmMask;

    gRFAL.Lm.rxBuf    = rxBuf;
    gRFAL.Lm.rxBufLen = rxBufLen;
    gRFAL.Lm.rxLen    = rxLen;
    *gRFAL.Lm.rxLen   = 0;
    gRFAL.Lm.dataFlag = false;
    gRFAL.Lm.iniFlag  = true;

    /* Apply the Automatic Responses configuration */
    ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a | ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r | ST25R3916_REG_PASSIVE_TARGET_d_ac_ap2p), autoResp);

    /* Disable GPT trigger source */
    ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_gptc_mask, ST25R3916_REG_TIMER_EMV_CONTROL_gptc_no_trigger);

    /* On Bit Rate Detection Mode ST25R391x will filter incoming frames during MRT time starting on External Field On event, use 512/fc steps */
    ST25R3916_IO_SetRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_mrt_step_512);
    ST25R3916_IO_WriteRegister(ST25R3916_REG_MASK_RX_TIMER, (uint8_t)RFal_Conv1fcTo512fc(RFAL_LM_GT));


    /* Restore default settings on NFCIP1 mode, Receiving parity + CRC bits and manual Tx Parity*/
    ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_ISO14443A_NFC, (ST25R3916_REG_ISO14443A_NFC_no_tx_par | ST25R3916_REG_ISO14443A_NFC_no_rx_par | ST25R3916_REG_ISO14443A_NFC_nfc_f0));

    /* External Field Detector enabled as Automatics on RFal_Initialize() */

    /* Set Analog configurations for generic Listen mode */
    /* Not on SetState(POWER OFF) as otherwise would be applied on every Field Event */
    RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_LISTEN_ON));

    /* Initialize as POWER_OFF and set proper mode in RF Chip */
    RFal_ListenSetState(RFAL_LM_STATE_POWER_OFF);
  } else {
    return NFC_RequestError;   /* Listen Start called but no mode was enabled */
  }

  return NFC_OK;
}

/*******************************************************************************/
static NFC_OpResult RFal_RunListenModeWorker(void)
{
  volatile uint32_t irqs;
  uint8_t           tmp;

  if (gRFAL.state != RFAL_STATE_LM) {
    return NFC_WrongState;
  }

  switch (gRFAL.Lm.state) {
    /*******************************************************************************/
    case RFAL_LM_STATE_POWER_OFF:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_EON));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      if ((irqs & ST25R3916_IRQ_MASK_EON) != 0U) {
        RFal_ListenSetState(RFAL_LM_STATE_IDLE);
      } else {
        break;
      }
    /* fall through */


    /*******************************************************************************/
    case RFAL_LM_STATE_IDLE:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_NFCT | ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_RXE_PTA));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      if ((irqs & ST25R3916_IRQ_MASK_NFCT) != 0U) {
        /* Retrieve detected bitrate */
        uint8_t    newBr;
        ST25R3916_IO_ReadRegister(ST25R3916_REG_NFCIP1_BIT_RATE, &newBr);
        newBr >>= ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rate_shift;

        if (newBr > ST25R3916_REG_BIT_RATE_rxrate_424) {
          newBr = ST25R3916_REG_BIT_RATE_rxrate_424;
        }

        gRFAL.Lm.brDetected = (RFal_BitRate)(newBr); /* PRQA S 4342 # MISRA 10.5 - Guaranteed that no invalid enum values may be created. See also equalityGuard_RFAL_BR_106 ff.*/
      }


      /* If EOF has already been received processing of other events is neglectable */
      if (((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) && (!gRFAL.Lm.dataFlag)) {
        RFal_ListenSetState(RFAL_LM_STATE_POWER_OFF);
      } else if (((irqs & ST25R3916_IRQ_MASK_WU_F) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
        RFal_ListenSetState(RFAL_LM_STATE_READY_F);
      } else if (((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
        irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_ERR1));

        if (((irqs & ST25R3916_IRQ_MASK_CRC) != 0U) || ((irqs & ST25R3916_IRQ_MASK_PAR) != 0U) || ((irqs & ST25R3916_IRQ_MASK_ERR1) != 0U)) {
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);
          ST25R3916_IO_TxOff();
          break; /* A bad reception occurred, remain in same state */
        }

        /*******************************************************************************/
        /* REMARK: Silicon workaround ST25R3916 Errata #TBD                            */
        /* In bitrate detection mode the automatic RF Collision Avoidance              */
        /* may not be able to emit RF carrier depending on the pt_res setting          */
        /* Preemptively enter AP2P before FIFO retrieval and protocol checking         */
        if ((gRFAL.Lm.mdMask & RFAL_LM_MASK_ACTIVE_P2P) != 0U) {
          ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om_targ_nfcip | ST25R3916_REG_MODE_nfc_ar_eof));
        }
        /*******************************************************************************/

        /* Retrieve received data */
        *gRFAL.Lm.rxLen = ST25R3916_GetNumFIFOBytes();
        ST25R3916_IO_ReadFifo(gRFAL.Lm.rxBuf, ((*gRFAL.Lm.rxLen < RFal_ConvBitsToBytes(gRFAL.Lm.rxBufLen)) ? *gRFAL.Lm.rxLen : RFal_ConvBitsToBytes(gRFAL.Lm.rxBufLen)));


#ifdef ST25R3916
        /*******************************************************************************/
        /* REMARK: Silicon workaround ST25R3916 Errata #TBD                            */
        /* In bitrate detection mode CRC is not checked for NFC-A frames               */
        if ((*gRFAL.Lm.rxLen > RFAL_CRC_LEN) && (gRFAL.Lm.brDetected == RFAL_BR_106)) {
          if (ST25R3916_CRC_CalculateCcitt(RFAL_ISO14443A_CRC_INTVAL, gRFAL.Lm.rxBuf, *gRFAL.Lm.rxLen) != 0U) {
            ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
            ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);
            ST25R3916_IO_TxOff();
            break; /* A bad reception occurred, remain in same state */
          }
        }
        /*******************************************************************************/
#endif /* ST25R3916 */

        /* Check if the data we got has at least the CRC and remove it, otherwise leave at 0 */
        *gRFAL.Lm.rxLen  -= ((*gRFAL.Lm.rxLen > RFAL_CRC_LEN) ? RFAL_CRC_LEN : *gRFAL.Lm.rxLen);
        *gRFAL.Lm.rxLen   = (uint16_t)RFal_ConvBytesToBits(*gRFAL.Lm.rxLen);
        gRFAL.Lm.dataFlag = true;

        /*Check if Observation Mode was enabled and disable it on ST25R391x */
        RFal_CheckDisableObsMode();
      } else if (((irqs & ST25R3916_IRQ_MASK_RXE_PTA) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
        if (((gRFAL.Lm.mdMask & RFAL_LM_MASK_NFCA) != 0U) && (gRFAL.Lm.brDetected == RFAL_BR_106)) {
          ST25R3916_IO_ReadRegister(ST25R3916_REG_PASSIVE_TARGET_STATUS, &tmp);
          if (tmp > ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_idle) {
            RFal_ListenSetState(RFAL_LM_STATE_READY_A);
          }
        }
      } else {
        /* MISRA 15.7 - Empty else */
      }
      break;

    /*******************************************************************************/
    case RFAL_LM_STATE_READY_F:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_EOF));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      /* If EOF has already been received processing of other events is neglectable */
      if ((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
        RFal_ListenSetState(RFAL_LM_STATE_POWER_OFF);
      } else if ((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) {
        /* Retrieve the error flags/irqs */
        irqs |= ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_ERR1));

        if (((irqs & ST25R3916_IRQ_MASK_CRC) != 0U) || ((irqs & ST25R3916_IRQ_MASK_ERR1) != 0U)) {
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);
          break; /* A bad reception occurred, remain in same state */
        }

        /* Retrieve received data */
        *gRFAL.Lm.rxLen = ST25R3916_GetNumFIFOBytes();
        ST25R3916_IO_ReadFifo(gRFAL.Lm.rxBuf, ((*gRFAL.Lm.rxLen < RFal_ConvBitsToBytes(gRFAL.Lm.rxBufLen)) ? *gRFAL.Lm.rxLen : RFal_ConvBitsToBytes(gRFAL.Lm.rxBufLen)));

        /* Check if the data we got has at least the CRC and remove it, otherwise leave at 0 */
        *gRFAL.Lm.rxLen  -= ((*gRFAL.Lm.rxLen > RFAL_CRC_LEN) ? RFAL_CRC_LEN : *gRFAL.Lm.rxLen);
        *gRFAL.Lm.rxLen  = (uint16_t)RFal_ConvBytesToBits(*gRFAL.Lm.rxLen);
        gRFAL.Lm.dataFlag = true;
      } else if ((irqs & ST25R3916_IRQ_MASK_WU_F) != 0U) {
        break;          /* Remain in same state */
      } else {
        /* MISRA 15.7 - Empty else */
      }
      break;

    /*******************************************************************************/
    case RFAL_LM_STATE_READY_A:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_WU_A));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      /* If EOF has already been received processing of other events is neglectable */
      if ((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
        RFal_ListenSetState(RFAL_LM_STATE_POWER_OFF);
      } else if ((irqs & ST25R3916_IRQ_MASK_WU_A) != 0U) {
        RFal_ListenSetState(RFAL_LM_STATE_ACTIVE_A);
      } else {
        /* MISRA 15.7 - Empty else */
      }
      break;

    /*******************************************************************************/
    case RFAL_LM_STATE_ACTIVE_A:
    case RFAL_LM_STATE_ACTIVE_Ax:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_EOF));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      /* If EOF has already been received processing of other events is neglectable */
      if ((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
        RFal_ListenSetState(RFAL_LM_STATE_POWER_OFF);
      } else if ((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) {
        /* Retrieve the error flags/irqs */
        irqs |= ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_ERR1));
        *gRFAL.Lm.rxLen = ST25R3916_GetNumFIFOBytes();

        if (((irqs & ST25R3916_IRQ_MASK_CRC) != 0U) || ((irqs & ST25R3916_IRQ_MASK_ERR1) != 0U)    ||
            ((irqs & ST25R3916_IRQ_MASK_PAR) != 0U) || (*gRFAL.Lm.rxLen <= RFAL_CRC_LEN)) {
          /* Clear rx context and FIFO */
          *gRFAL.Lm.rxLen = 0;
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

          /* Check if we should go to IDLE or Sleep */
          if (gRFAL.Lm.state == RFAL_LM_STATE_ACTIVE_Ax) {
            RFal_ListenSleepStart(RFAL_LM_STATE_SLEEP_A, gRFAL.Lm.rxBuf, gRFAL.Lm.rxBufLen, gRFAL.Lm.rxLen);
          } else {
            RFal_ListenSetState(RFAL_LM_STATE_IDLE);
          }

          ST25R3916_IO_DisableInterrupts(ST25R3916_IRQ_MASK_RXE);
          break;
        }

        /* Remove CRC from length */
        *gRFAL.Lm.rxLen -= RFAL_CRC_LEN;

        /* Retrieve received data */
        ST25R3916_IO_ReadFifo(gRFAL.Lm.rxBuf, ((*gRFAL.Lm.rxLen < RFal_ConvBitsToBytes(gRFAL.Lm.rxBufLen)) ? *gRFAL.Lm.rxLen : RFal_ConvBitsToBytes(gRFAL.Lm.rxBufLen)));
        *gRFAL.Lm.rxLen   = (uint16_t)RFal_ConvBytesToBits(*gRFAL.Lm.rxLen);
        gRFAL.Lm.dataFlag = true;
      } else {
        /* MISRA 15.7 - Empty else */
      }
      break;


    /*******************************************************************************/
    case RFAL_LM_STATE_SLEEP_A:
    case RFAL_LM_STATE_SLEEP_B:
    case RFAL_LM_STATE_SLEEP_AF:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_NFCT | ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_RXE_PTA));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      if ((irqs & ST25R3916_IRQ_MASK_NFCT) != 0U) {
        uint8_t    newBr;
        /* Retrieve detected bitrate */
        ST25R3916_IO_ReadRegister(ST25R3916_REG_NFCIP1_BIT_RATE, &newBr);
        newBr >>= ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rate_shift;

        if (newBr > ST25R3916_REG_BIT_RATE_rxrate_424) {
          newBr = ST25R3916_REG_BIT_RATE_rxrate_424;
        }

        gRFAL.Lm.brDetected = (RFal_BitRate)(newBr); /* PRQA S 4342 # MISRA 10.5 - Guaranteed that no invalid enum values may be created. See also equalityGuard_RFAL_BR_106 ff.*/
      }

      /* If EOF has already been received processing of other events is neglectable */
      if ((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
        RFal_ListenSetState(RFAL_LM_STATE_POWER_OFF);
      } else if (((irqs & ST25R3916_IRQ_MASK_WU_F) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
        RFal_ListenSetState(RFAL_LM_STATE_READY_F);
      } else if (((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
        /* Clear rx context and FIFO */
        *gRFAL.Lm.rxLen = 0;
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

        /* REMARK: In order to support CUP or proprietary frames, handling could be added here */
      } else if (((irqs & ST25R3916_IRQ_MASK_RXE_PTA) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
        if (((gRFAL.Lm.mdMask & RFAL_LM_MASK_NFCA) != 0U) && (gRFAL.Lm.brDetected == RFAL_BR_106)) {
          ST25R3916_IO_ReadRegister(ST25R3916_REG_PASSIVE_TARGET_STATUS, &tmp);
          if (tmp > ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_halt) {
            RFal_ListenSetState(RFAL_LM_STATE_READY_Ax);
          }
        }
      } else {
        /* MISRA 15.7 - Empty else */
      }
      break;

    /*******************************************************************************/
    case RFAL_LM_STATE_READY_Ax:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_WU_A_X));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        break;  /* No interrupt to process */
      }

      /* If EOF has already been received processing of other events is neglectable */
      if ((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
        RFal_ListenSetState(RFAL_LM_STATE_POWER_OFF);
      } else if ((irqs & ST25R3916_IRQ_MASK_WU_A_X) != 0U) {
        RFal_ListenSetState(RFAL_LM_STATE_ACTIVE_Ax);
      } else {
        /* MISRA 15.7 - Empty else */
      }
      break;

    /*******************************************************************************/
    case RFAL_LM_STATE_CARDEMU_4A:
    case RFAL_LM_STATE_CARDEMU_4B:
    case RFAL_LM_STATE_CARDEMU_3:
    case RFAL_LM_STATE_TARGET_F:
    case RFAL_LM_STATE_TARGET_A:
      break;

    /*******************************************************************************/
    default:
      return NFC_WrongState;
  }
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ListenStop(void)
{

  /* Check if RFAL is initialized */
  if (gRFAL.state < RFAL_STATE_INIT) {
    return NFC_WrongState;
  }

  gRFAL.Lm.state = RFAL_LM_STATE_NOT_INIT;

  /*Check if Observation Mode was enabled and disable it on ST25R391x */
  RFal_CheckDisableObsMode();

  /* Re-Enable the Oscillator if not running */
  ST25R3916_OscOn();

  /* Disable Receiver and Transmitter */
  RFal_FieldOff();

  /* Disable all automatic responses */
  ST25R3916_IO_SetRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r | ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a | ST25R3916_REG_PASSIVE_TARGET_d_ac_ap2p));

  /* As there's no Off mode, set default value: ISO14443A with automatic RF Collision Avoidance Off */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, (ST25R3916_REG_MODE_om_iso14443a | ST25R3916_REG_MODE_tr_am_ook | ST25R3916_REG_MODE_nfc_ar_off));

  ST25R3916_IO_DisableInterrupts((ST25R3916_IRQ_MASK_RXE_PTA | ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_WU_A | ST25R3916_IRQ_MASK_WU_A_X | ST25R3916_IRQ_MASK_RFU2 | ST25R3916_IRQ_MASK_OSC));
  ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_RXE_PTA | ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_WU_A | ST25R3916_IRQ_MASK_WU_A_X | ST25R3916_IRQ_MASK_RFU2 | ST25R3916_IRQ_MASK_TXE));

  /* Set Analog configurations for Listen Off event */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_LISTEN_OFF));

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ListenSleepStart(RFal_LmState sleepSt, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen)
{

  /* Check if RFAL is not initialized */
  if (gRFAL.state < RFAL_STATE_INIT) {
    return NFC_WrongState;
  }

  switch (sleepSt) {
    /*******************************************************************************/
    case RFAL_LM_STATE_SLEEP_A:

      /* Enable automatic responses for A */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));

      /* Reset NFCA target */
      ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_GOTO_SLEEP);


      /* Set Target mode, Bit Rate detection and Listen Mode for NFC-A */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_MODE,
                                  (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_mask | ST25R3916_REG_MODE_nfc_ar_mask),
                                  (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om0 | ST25R3916_REG_MODE_nfc_ar_off));
      break;

    /*******************************************************************************/
    case RFAL_LM_STATE_SLEEP_AF:

      /* Enable automatic responses for A + F */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r | ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));

      /* Reset NFCA target state */
      ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_GOTO_SLEEP);

      /* Set Target mode, Bit Rate detection, Listen Mode for NFC-A and NFC-F */
      ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_MODE,
                                  (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_mask | ST25R3916_REG_MODE_nfc_ar_mask),
                                  (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om2 | ST25R3916_REG_MODE_om0 | ST25R3916_REG_MODE_nfc_ar_off));
      break;

    /*******************************************************************************/
    case RFAL_LM_STATE_SLEEP_B:
      /* REMARK: Support for CE-B would be added here  */
      return NFC_Unsupport;

    /*******************************************************************************/
    default:
      return NFC_InvalidParameter;

  }


  /* Ensure that the  NFCIP1 mode is disabled */
  ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_ISO14443A_NFC, ST25R3916_REG_ISO14443A_NFC_nfc_f0);

  ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);


  /* Clear and enable required IRQs */
  ST25R3916_IO_ClearAndEnableInterrupts((ST25R3916_IRQ_MASK_NFCT | ST25R3916_IRQ_MASK_RXS | ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR1 |
                                     ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_EON | ST25R3916_IRQ_MASK_EOF  | gRFAL.Lm.mdIrqs));

  /* Check whether the field was turn off right after the Sleep request */
  if (!RFal_IsExtFieldOn()) {
#if 0 /* Debug purposes */
    RFal_LogD("RFAL: curState: %02X newState: %02X \r\n", gRFAL.Lm.state, RFAL_LM_STATE_NOT_INIT);
#endif

    RFal_ListenStop();
    return NFC_LinkLoss;
  }

#if 0 /* Debug purposes */
  RFal_LogD("RFAL: curState: %02X newState: %02X \r\n", gRFAL.Lm.state, sleepSt);
#endif

  /* Set the new Sleep State*/
  gRFAL.Lm.state    = sleepSt;
  gRFAL.state       = RFAL_STATE_LM;

  gRFAL.Lm.rxBuf    = rxBuf;
  gRFAL.Lm.rxBufLen = rxBufLen;
  gRFAL.Lm.rxLen    = rxLen;
  *gRFAL.Lm.rxLen   = 0;
  gRFAL.Lm.dataFlag = false;

  return NFC_OK;
}

/*******************************************************************************/
RFal_LmState RFal_ListenGetState(bool *dataFlag, RFal_BitRate *lastBR)
{
  /* Allow state retrieval even if gRFAL.state != RFAL_STATE_LM so  *
   * that this Lm state can be used by caller after activation      */

  if (lastBR != NULL) {
    *lastBR = gRFAL.Lm.brDetected;
  }

  if (dataFlag != NULL) {
    *dataFlag = gRFAL.Lm.dataFlag;
  }

  return gRFAL.Lm.state;
}

/*******************************************************************************/
NFC_OpResult RFal_ListenSetState(RFal_LmState newSt)
{
  NFC_OpResult ret;
  RFal_LmState newState;
  bool        reSetState;

  /* Check if RFAL is initialized */
  if (gRFAL.state < RFAL_STATE_INIT) {
    return NFC_WrongState;
  }

  /* SetState clears the Data flag */
  gRFAL.Lm.dataFlag = false;
  newState          = newSt;
  ret               = NFC_OK;

  do {
    reSetState = false;

    /*******************************************************************************/
    switch (newState) {
      /*******************************************************************************/
      case RFAL_LM_STATE_POWER_OFF:

        /* Enable the receiver and reset logic */
        ST25R3916_IO_SetRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_rx_en);
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_STOP);

        if ((gRFAL.Lm.mdMask & RFAL_LM_MASK_NFCA) != 0U) {
          /* Enable automatic responses for A */
          ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_PASSIVE_TARGET, ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a);

          /* Prepares the NFCIP-1 Passive target logic to wait in the Sense/Idle state */
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_GOTO_SENSE);
        }

        if ((gRFAL.Lm.mdMask & RFAL_LM_MASK_NFCF) != 0U) {
          /* Enable automatic responses for F */
          ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r));
        }

        if ((gRFAL.Lm.mdMask & RFAL_LM_MASK_ACTIVE_P2P) != 0U) {
          /* Ensure automatic response RF Collision Avoidance is back to only after Rx */
          ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_MODE, ST25R3916_REG_MODE_nfc_ar_mask, ST25R3916_REG_MODE_nfc_ar_auto_rx);

          /* Ensure that our field is Off, as automatic response RF Collision Avoidance may have been triggered */
          ST25R3916_IO_TxOff();
        }

        /*******************************************************************************/
        /* Ensure that the  NFCIP1 mode is disabled */
        ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_ISO14443A_NFC, ST25R3916_REG_ISO14443A_NFC_nfc_f0);


        /*******************************************************************************/
        /* Clear and enable required IRQs */
        ST25R3916_IO_DisableInterrupts(ST25R3916_IRQ_MASK_ALL);

        ST25R3916_IO_ClearAndEnableInterrupts((ST25R3916_IRQ_MASK_NFCT | ST25R3916_IRQ_MASK_RXS | ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR1 | ST25R3916_IRQ_MASK_OSC |
                                           ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_EON | ST25R3916_IRQ_MASK_EOF  | gRFAL.Lm.mdIrqs));

        /*******************************************************************************/
        /* Clear the bitRate previously detected */
        gRFAL.Lm.brDetected = RFAL_BR_KEEP;


        /*******************************************************************************/
        /* Apply the initial mode */
        ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_MODE, (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_mask | ST25R3916_REG_MODE_nfc_ar_mask), (uint8_t)gRFAL.Lm.mdReg);

        /*******************************************************************************/
        /* Check if external Field is already On */
        if (RFal_IsExtFieldOn()) {
          reSetState = true;
          newState   = RFAL_LM_STATE_IDLE;                         /* Set IDLE state */
        }
        else {
          ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, (ST25R3916_REG_OP_CONTROL_tx_en | ST25R3916_REG_OP_CONTROL_rx_en | ST25R3916_REG_OP_CONTROL_en));
        }
        break;

      /*******************************************************************************/
      case RFAL_LM_STATE_IDLE:

        /*******************************************************************************/
        /* Check if device is coming from Low Power bit rate detection */
        if (!ST25R3916_IO_CheckReg(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en, ST25R3916_REG_OP_CONTROL_en)) {
          /* Exit Low Power mode and confirm the temporarily enable */
          ST25R3916_IO_SetRegisterBits(ST25R3916_REG_OP_CONTROL, (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_rx_en));

          if (!ST25R3916_IO_CheckReg(ST25R3916_REG_AUX_DISPLAY, ST25R3916_REG_AUX_DISPLAY_osc_ok, ST25R3916_REG_AUX_DISPLAY_osc_ok)) {
            /* Wait for Oscillator ready */
            if (ST25R3916_IO_WaitForInterruptsTimed(ST25R3916_IRQ_MASK_OSC, ST25R3916_TOUT_OSC_STABLE) == 0U) {
              ret = NFC_IO_Error;
              break;
            }
          }
        } else {
          ST25R3916_IO_GetInterrupt(ST25R3916_IRQ_MASK_OSC);
        }


        /*******************************************************************************/
        /* In Active P2P the Initiator may:  Turn its field On;  LM goes into IDLE state;
         *      Initiator sends an unexpected frame raising a Protocol error; Initiator
         *      turns its field Off and ST25R3916 performs the automatic RF Collision
         *      Avoidance keeping our field On; upon a Protocol error upper layer sets
         *      again the state to IDLE to clear dataFlag and wait for next data.
         *
         * Ensure that when upper layer calls SetState(IDLE), it restores initial
         * configuration and that check whether an external Field is still present     */
        if ((gRFAL.Lm.mdMask & RFAL_LM_MASK_ACTIVE_P2P) != 0U) {
          /* Ensure nfc_ar is reset and back to only after Rx */
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_STOP);
          ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_MODE, ST25R3916_REG_MODE_nfc_ar_mask, ST25R3916_REG_MODE_nfc_ar_auto_rx);

          /* Ensure that our field is Off, as automatic response RF Collision Avoidance may have been triggered */
          ST25R3916_IO_TxOff();

          /* If external Field is no longer detected go back to POWER_OFF */
          if (!ST25R3916_IO_IsExtFieldOn()) {
            reSetState = true;
            newState   = RFAL_LM_STATE_POWER_OFF;                    /* Set POWER_OFF state */
          }
        }
        /*******************************************************************************/

        /* If we are in ACTIVE_A, re-enable Listen for A before going to IDLE, otherwise do nothing */
        if (gRFAL.Lm.state == RFAL_LM_STATE_ACTIVE_A) {
          /* Enable automatic responses for A and Reset NFCA target state */
          ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));
          ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_GOTO_SENSE);
        }

        /* Re-enable the receiver */
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

        /*******************************************************************************/
        /*Check if Observation Mode is enabled and set it on ST25R391x */
        RFal_CheckEnableObsModeRx();
        break;

      /*******************************************************************************/
      case RFAL_LM_STATE_READY_F:

        /*******************************************************************************/
        /* If we're coming from BitRate detection mode, the Bit Rate Definition reg
         * still has the last bit rate used.
         * If a frame is received between setting the mode to Listen NFCA and
         * setting Bit Rate Definition reg, it will raise a framing error.
         * Set the bitrate immediately, and then the normal SetMode procedure          */
        ST25R3916_SetBitrate((uint8_t)gRFAL.Lm.brDetected, (uint8_t)gRFAL.Lm.brDetected);
        /*******************************************************************************/

        /* Disable automatic responses for NFC-A */
        ST25R3916_IO_SetRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));

        /* Set Mode NFC-F only */
        ret = RFal_SetMode(RFAL_MODE_LISTEN_NFCF, gRFAL.Lm.brDetected, gRFAL.Lm.brDetected);
        gRFAL.state = RFAL_STATE_LM;                    /* Keep in Listen Mode */

        /* Re-enable the receiver */
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
        ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

        /* Clear any previous transmission errors (if Reader polled for other/unsupported technologies) */
        ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_ERR1));

        ST25R3916_IO_EnableInterrupts(ST25R3916_IRQ_MASK_RXE);       /* Start looking for any incoming data */
        break;

      /*******************************************************************************/
      case RFAL_LM_STATE_CARDEMU_3:

        /* Set Listen NFCF mode  */
        ret = RFal_SetMode(RFAL_MODE_LISTEN_NFCF, gRFAL.Lm.brDetected, gRFAL.Lm.brDetected);
        break;

      /*******************************************************************************/
      case RFAL_LM_STATE_READY_Ax:
      case RFAL_LM_STATE_READY_A:

        /*******************************************************************************/
        /* If we're coming from BitRate detection mode, the Bit Rate Definition reg
         * still has the last bit rate used.
         * If a frame is received between setting the mode to Listen NFCA and
         * setting Bit Rate Definition reg, it will raise a framing error.
         * Set the bitrate immediately, and then the normal SetMode procedure          */
        ST25R3916_SetBitrate((uint8_t)gRFAL.Lm.brDetected, (uint8_t)gRFAL.Lm.brDetected);
        /*******************************************************************************/

        /* Disable automatic responses for NFC-F */
        ST25R3916_IO_SetRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r));

        /* Set Mode NFC-A only */
        ret = RFal_SetMode(RFAL_MODE_LISTEN_NFCA, gRFAL.Lm.brDetected, gRFAL.Lm.brDetected);

        gRFAL.state = RFAL_STATE_LM;                    /* Keep in Listen Mode */
        break;

      /*******************************************************************************/
      case RFAL_LM_STATE_ACTIVE_Ax:
      case RFAL_LM_STATE_ACTIVE_A:

        /* Disable automatic responses for A */
        ST25R3916_IO_SetRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));

        /* Clear any previous transmission errors (if Reader polled for other/unsupported technologies) */
        ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_ERR1));

        ST25R3916_IO_EnableInterrupts(ST25R3916_IRQ_MASK_RXE);      /* Start looking for any incoming data */
        break;

      case RFAL_LM_STATE_TARGET_F:
        /* Disable Automatic response SENSF_REQ */
        ST25R3916_IO_SetRegisterBits(ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r));
        break;

      /*******************************************************************************/
      case RFAL_LM_STATE_SLEEP_A:
      case RFAL_LM_STATE_SLEEP_B:
      case RFAL_LM_STATE_SLEEP_AF:
        /* These sleep states have to be set by the RFal_ListenSleepStart() method */
        return NFC_RequestError;

      /*******************************************************************************/
      case RFAL_LM_STATE_CARDEMU_4A:
      case RFAL_LM_STATE_CARDEMU_4B:
      case RFAL_LM_STATE_TARGET_A:
        /* States not handled by the LM, just keep state context */
        break;

      /*******************************************************************************/
      default:
        return NFC_WrongState;
    }
  } while (reSetState);

  gRFAL.Lm.state = newState;

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_WakeUpModeStart(const RFal_WakeUpConfig *config)
{
  uint8_t aux;
  uint8_t reg;
  uint32_t irqs;

  /* Check if RFAL is not initialized */
  if (gRFAL.state < RFAL_STATE_INIT) {
    return NFC_WrongState;
  }

  /* The Wake-Up procedure is explained in detail in Application Note: AN5320 */

  if (config == NULL) {
    gRFAL.wum.cfg.period = RFAL_WUM_PERIOD_200MS;
    gRFAL.wum.cfg.irqTout = false;
    gRFAL.wum.cfg.swTagDetect = false;

    gRFAL.wum.cfg.indAmp.enabled = true;
    gRFAL.wum.cfg.indPha.enabled = false;
    gRFAL.wum.cfg.cap.enabled = false;
    gRFAL.wum.cfg.indAmp.delta = 2U;
    gRFAL.wum.cfg.indAmp.fracDelta = 0U;
    gRFAL.wum.cfg.indAmp.reference = RFAL_WUM_REFERENCE_AUTO;
    gRFAL.wum.cfg.indAmp.autoAvg = false;
#ifdef ST25R3916
    /*******************************************************************************/
    /* Check if AAT is enabled and if so make use of the SW Tag Detection          */
    if (ST25R3916_IO_IsAATOn()) {
      /* Enable SW TD with delta of 1.5 and enable auto average */
      gRFAL.wum.cfg.swTagDetect = true;
      gRFAL.wum.cfg.indAmp.delta = 1U;
      gRFAL.wum.cfg.indAmp.fracDelta = 2U;
      gRFAL.wum.cfg.indAmp.autoAvg = true;
      gRFAL.wum.cfg.indAmp.aaWeight = RFAL_WUM_AA_WEIGHT_16;
    }
#endif /* ST25R3916 */
  } else {
    gRFAL.wum.cfg = *config;
  }

#ifdef ST25R3916B
  /* Check for not supported features */
  if (gRFAL.wum.cfg.cap.enabled) {
    return NFC_Unsupport;
  }

  /* Set ST25R3916B Measure Tx NeonRTOS_Sleep */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_MEAS_TX_DELAY, (ST25R3916_IO_IsAATOn() ? RFAL_ST25R3916B_AAT_SETTLE : 0x00));
#endif /* ST25R3916B */
  /* Check for valid configuration */
  if (((!gRFAL.wum.cfg.cap.enabled) && (!gRFAL.wum.cfg.indAmp.enabled) && (!gRFAL.wum.cfg.indPha.enabled)) ||
      ((gRFAL.wum.cfg.cap.enabled) && ((gRFAL.wum.cfg.indAmp.enabled) || (gRFAL.wum.cfg.indPha.enabled))) ||
      ((gRFAL.wum.cfg.cap.enabled) && (gRFAL.wum.cfg.swTagDetect)) ||
      ((gRFAL.wum.cfg.indAmp.reference > RFAL_WUM_REFERENCE_AUTO) ||
       (gRFAL.wum.cfg.indPha.reference > RFAL_WUM_REFERENCE_AUTO) ||
       (gRFAL.wum.cfg.cap.reference > RFAL_WUM_REFERENCE_AUTO))) {

    return NFC_InvalidParameter;
  }

  irqs = ST25R3916_IRQ_MASK_NONE;

  /* Disable Tx, Rx, External Field Detector and set default ISO14443A mode */
  ST25R3916_IO_TxRxOff();
  ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en_fd_mask);
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_MODE, (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_mask), (ST25R3916_REG_MODE_targ_init | ST25R3916_REG_MODE_om_iso14443a));

  /* Set Analog configurations for Wake-up On event */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_WAKEUP_ON));

  /*******************************************************************************/
  /* Prepare Wake-Up Timer Control Register */
  reg = (uint8_t)(((uint8_t)gRFAL.wum.cfg.period & 0x0FU) << ST25R3916_REG_WUP_TIMER_CONTROL_wut_shift);
  reg |= (uint8_t)(((uint8_t)gRFAL.wum.cfg.period < (uint8_t)RFAL_WUM_PERIOD_100MS) ? ST25R3916_REG_WUP_TIMER_CONTROL_wur : 0x00U);

  if (gRFAL.wum.cfg.irqTout || gRFAL.wum.cfg.swTagDetect) {
    reg |= ST25R3916_REG_WUP_TIMER_CONTROL_wto;
    irqs |= ST25R3916_IRQ_MASK_WT;
  }
  /* Check if HW Wake-up is to be used or SW Tag detection */
  if (gRFAL.wum.cfg.swTagDetect) {
    gRFAL.wum.cfg.indAmp.reference = 0U;
    gRFAL.wum.cfg.indPha.reference = 0U;
    gRFAL.wum.cfg.cap.reference = 0U;
  } else {

    /*******************************************************************************/
    /* Check if Inductive Amplitude is to be performed */
    if (gRFAL.wum.cfg.indAmp.enabled) {
      aux = (uint8_t)((gRFAL.wum.cfg.indAmp.delta) << ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_d_shift);
      aux |= (uint8_t)(gRFAL.wum.cfg.indAmp.aaInclMeas ? ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aam : 0x00U);
      aux |= (uint8_t)(((uint8_t)gRFAL.wum.cfg.indAmp.aaWeight << ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aew_shift) & ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aew_mask);
      aux |= (uint8_t)(gRFAL.wum.cfg.indAmp.autoAvg ? ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_ae : 0x00U);

      ST25R3916_IO_WriteRegister(ST25R3916_REG_AMPLITUDE_MEASURE_CONF, aux);

      /* Only need to set the reference if not using Auto Average */
      if (!gRFAL.wum.cfg.indAmp.autoAvg) {
        if (gRFAL.wum.cfg.indAmp.reference == RFAL_WUM_REFERENCE_AUTO) {
          ST25R3916_MeasureAmplitude(&aux);
          gRFAL.wum.cfg.indAmp.reference = aux;
        }
        ST25R3916_IO_WriteRegister(ST25R3916_REG_AMPLITUDE_MEASURE_REF, (uint8_t)gRFAL.wum.cfg.indAmp.reference);
      }

      reg |= ST25R3916_REG_WUP_TIMER_CONTROL_wam;
      irqs |= ST25R3916_IRQ_MASK_WAM;
    }

    /*******************************************************************************/
    /* Check if Inductive Phase is to be performed */
    if (gRFAL.wum.cfg.indPha.enabled) {
      aux = (uint8_t)((gRFAL.wum.cfg.indPha.delta) << ST25R3916_REG_PHASE_MEASURE_CONF_pm_d_shift);
      aux |= (uint8_t)(gRFAL.wum.cfg.indPha.aaInclMeas ? ST25R3916_REG_PHASE_MEASURE_CONF_pm_aam : 0x00U);
      aux |= (uint8_t)(((uint8_t)gRFAL.wum.cfg.indPha.aaWeight << ST25R3916_REG_PHASE_MEASURE_CONF_pm_aew_shift) & ST25R3916_REG_PHASE_MEASURE_CONF_pm_aew_mask);
      aux |= (uint8_t)(gRFAL.wum.cfg.indPha.autoAvg ? ST25R3916_REG_PHASE_MEASURE_CONF_pm_ae : 0x00U);

      ST25R3916_IO_WriteRegister(ST25R3916_REG_PHASE_MEASURE_CONF, aux);

      /* Only need to set the reference if not using Auto Average */
      if (!gRFAL.wum.cfg.indPha.autoAvg) {
        if (gRFAL.wum.cfg.indPha.reference == RFAL_WUM_REFERENCE_AUTO) {
          ST25R3916_MeasurePhase(&aux);
          gRFAL.wum.cfg.indPha.reference = aux;
        }
        ST25R3916_IO_WriteRegister(ST25R3916_REG_PHASE_MEASURE_REF, (uint8_t)gRFAL.wum.cfg.indPha.reference);
      }

      reg |= ST25R3916_REG_WUP_TIMER_CONTROL_wph;
      irqs |= ST25R3916_IRQ_MASK_WPH;
    }

#ifdef ST25R3916

    /*******************************************************************************/
    /* Check if Capacitive is to be performed */
    if (gRFAL.wum.cfg.cap.enabled) {
      /*******************************************************************************/
      /* Perform Capacitive sensor calibration */

      /* Disable Oscillator and Field */
      ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_tx_en));

      /* Sensor gain should be configured on Analog Config: RFAL_ST25R3916_ANALOG_CONFIG_CHIP_WAKEUP_ON */

      /* Perform calibration procedure */
      ST25R3916_CalibrateCapacitiveSensor(NULL);

      /*******************************************************************************/
      aux = (uint8_t)((gRFAL.wum.cfg.cap.delta) << ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_d_shift);
      aux |= (uint8_t)(gRFAL.wum.cfg.cap.aaInclMeas ? ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aam : 0x00U);
      aux |= (uint8_t)(((uint8_t)gRFAL.wum.cfg.cap.aaWeight << ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aew_shift) & ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aew_mask);
      aux |= (uint8_t)(gRFAL.wum.cfg.cap.autoAvg ? ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_ae : 0x00U);

      ST25R3916_IO_WriteRegister(ST25R3916_REG_CAPACITANCE_MEASURE_CONF, aux);

      /* Only need to set the reference if not using Auto Average */
      if ((!gRFAL.wum.cfg.cap.autoAvg) || (gRFAL.wum.cfg.swTagDetect)) {
        if (gRFAL.wum.cfg.cap.reference == RFAL_WUM_REFERENCE_AUTO) {
          ST25R3916_MeasureCapacitance(&aux);
          gRFAL.wum.cfg.cap.reference = aux;
        }
        ST25R3916_IO_WriteRegister(ST25R3916_REG_CAPACITANCE_MEASURE_REF, (uint8_t)gRFAL.wum.cfg.cap.reference);
      }

      reg |= ST25R3916_REG_WUP_TIMER_CONTROL_wcap;
      irqs |= ST25R3916_IRQ_MASK_WCAP;
    }
#endif /* ST25R3916 */
  }

  /* Disable and clear all interrupts except Wake-Up IRQs */
  ST25R3916_IO_DisableInterrupts(ST25R3916_IRQ_MASK_ALL);
  ST25R3916_IO_GetInterrupt(irqs);
  ST25R3916_IO_EnableInterrupts(irqs);

  /* Enable Low Power Wake-Up Mode (Disable: Oscilattor, Tx, Rx and External Field Detector)*/
  ST25R3916_IO_WriteRegister(ST25R3916_REG_WUP_TIMER_CONTROL, reg);
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_OP_CONTROL,
                              (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_rx_en | ST25R3916_REG_OP_CONTROL_tx_en |
                               ST25R3916_REG_OP_CONTROL_en_fd_mask | ST25R3916_REG_OP_CONTROL_wu),
                              ST25R3916_REG_OP_CONTROL_wu);

  gRFAL.wum.state = RFAL_WUM_STATE_ENABLED;
  gRFAL.state = RFAL_STATE_WUM;

  return NFC_OK;

}

/*******************************************************************************/
bool RFal_WakeUpModeHasWoke(void)
{
  return (gRFAL.wum.state >= RFAL_WUM_STATE_ENABLED_WOKE);
}

/*******************************************************************************/
bool RFal_WakeUpModeIsEnabled(void)
{
  return NFC_Unsupport; /* NFC_Unsupport*/
}

/*******************************************************************************/
NFC_OpResult RFal_WakeUpModeGetInfo(bool force, RFal_WakeUpInfo *info)
{
  uint8_t aux;

  /* Check if WU mode is running */
  if ((gRFAL.state != RFAL_STATE_WUM) || (gRFAL.wum.state < RFAL_WUM_STATE_ENABLED)) {
    return NFC_WrongState;
  }

  /* Check for valid parameters */
  if (info == NULL) {
    return NFC_InvalidParameter;
  }

  /* Clear info structure */
  memset(info, 0x00, sizeof(RFal_WakeUpInfo));

  /* Update general information */
  info->irqWut          = gRFAL.wum.info.irqWut;
  gRFAL.wum.info.irqWut = false;

  /* WUT IRQ is signaled when WUT expires. Delay slightly for the actual measurement to be performed */
  if ((info->irqWut) && (!gRFAL.wum.cfg.swTagDetect)) {
    NeonRTOS_Sleep(1);
  }

  if (gRFAL.wum.cfg.indAmp.enabled) {
    /* Update measure and reference from current info */
    info->indAmp.reference = gRFAL.wum.cfg.indAmp.reference;
    info->indAmp.lastMeas  = gRFAL.wum.info.indAmp.lastMeas; /* For the case of swTagDetect==1 */

    /* Only retrieve the reference from the device if needed */
    if ((force || (info->irqWut) || (gRFAL.wum.info.indAmp.irqWu)) && (!gRFAL.wum.cfg.swTagDetect)) {
      if (gRFAL.wum.cfg.indAmp.autoAvg) {
        ST25R3916_IO_ReadRegister(ST25R3916_REG_AMPLITUDE_MEASURE_AA_RESULT, &aux);
        info->indAmp.reference = aux;
        gRFAL.wum.cfg.indAmp.reference = aux; /* Store last value for subsequenct calls */
      }
      ST25R3916_IO_ReadRegister(ST25R3916_REG_AMPLITUDE_MEASURE_RESULT, &info->indAmp.lastMeas);
      gRFAL.wum.info.indAmp.lastMeas = info->indAmp.lastMeas; /* Store last value for subsequenct calls */
    }

    /* Update IRQ information and clear flag upon retrieving */
    info->indAmp.irqWu          = gRFAL.wum.info.indAmp.irqWu;
    gRFAL.wum.info.indAmp.irqWu = false;
  }

  if (gRFAL.wum.cfg.indPha.enabled) {
    /* Update measure and reference from current info */
    info->indPha.reference = gRFAL.wum.cfg.indPha.reference;
    info->indPha.lastMeas  = gRFAL.wum.info.indPha.lastMeas; /* For the case of swTagDetect==1 */

    /* Only retrieve the reference from the device if needed */
    if ((force || (info->irqWut) || (gRFAL.wum.info.indPha.irqWu)) && (!gRFAL.wum.cfg.swTagDetect)) {
      if (gRFAL.wum.cfg.indPha.autoAvg) {
        ST25R3916_IO_ReadRegister(ST25R3916_REG_PHASE_MEASURE_AA_RESULT, &aux);
        info->indPha.reference = aux;
        gRFAL.wum.cfg.indPha.reference = aux; /* Store last value for subsequenct calls */
      }
      ST25R3916_IO_ReadRegister(ST25R3916_REG_PHASE_MEASURE_RESULT, &info->indPha.lastMeas);
      gRFAL.wum.info.indPha.lastMeas = info->indPha.lastMeas; /* Store last value for subsequenct calls */
    }

    /* Update IRQ information and clear flag upon retrieving */
    info->indPha.irqWu          = gRFAL.wum.info.indPha.irqWu;
    gRFAL.wum.info.indPha.irqWu = false;
  }

#ifdef ST25R3916
  if (gRFAL.wum.cfg.cap.enabled) {
    /* Update measure and reference from current info */
    info->cap.reference = gRFAL.wum.cfg.cap.reference;
    info->cap.lastMeas  = gRFAL.wum.info.cap.lastMeas;

    /* Retrieve the measurement from the device if needed */
    if (force || (info->irqWut) || (gRFAL.wum.info.cap.irqWu)) {
      /* Only retrieve the reference from the device if needed */
      if (gRFAL.wum.cfg.cap.autoAvg) {
        ST25R3916_IO_ReadRegister(ST25R3916_REG_CAPACITANCE_MEASURE_AA_RESULT, &aux);
        info->cap.reference = aux;
        gRFAL.wum.cfg.cap.reference = aux; /* Store last value for subsequenct calls */
      }
      ST25R3916_IO_ReadRegister(ST25R3916_REG_CAPACITANCE_MEASURE_RESULT, &info->cap.lastMeas);
      gRFAL.wum.info.cap.lastMeas = info->cap.lastMeas; /* Store last value for subsequenct calls */
    }

    /* Update IRQ information and clear flag upon retrieving */
    info->cap.irqWu          = gRFAL.wum.info.cap.irqWu;
    gRFAL.wum.info.cap.irqWu = false;
  }
#endif /* ST25R3916 */

  return NFC_OK;
}

/*******************************************************************************/
uint16_t RFal_WakeUpModeFilter(uint16_t curRef, uint16_t curVal, uint8_t weight)
{
  uint16_t newRef;

  /* Perform the averaging|filter as describded in ST25R3916 DS */

  /* Avoid signed arithmetic by splitting in two cases */
  if (curVal > curRef) {
    newRef = curRef + ((curVal - curRef) / weight);

    /* In order for the reference to converge to final value   *
     * increment once the diff is smaller that the weight      */
    if ((curVal != curRef) && (curRef == newRef)) {
      newRef &= 0xFF00U;
      newRef += 0x0100U;
    }
  } else {
    newRef = curRef - ((curRef - curVal) / weight);

    /* In order for the reference to converge to final value   *
     * decrement once the diff is smaller that the weight      */
    if ((curVal != curRef) && (curRef == newRef)) {
      newRef &= 0xFF00U;
    }
  }

  return newRef;
}
/*******************************************************************************/
static NFC_OpResult RFal_RunWakeUpModeWorker(void)
{
  uint32_t irqs;
  uint8_t  reg;
  uint16_t value;
  uint16_t delta;
  bool     woke;

  if (gRFAL.state != RFAL_STATE_WUM) {
    return NFC_WrongState;
  }

  switch (gRFAL.wum.state) {
    case RFAL_WUM_STATE_ENABLED:
    case RFAL_WUM_STATE_ENABLED_WOKE:

      irqs = ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_WT | ST25R3916_IRQ_MASK_WAM | ST25R3916_IRQ_MASK_WPH | ST25R3916_IRQ_MASK_WCAP));
      if (irqs == ST25R3916_IRQ_MASK_NONE) {
        return NFC_WrongState;  /* No interrupt to process */
      }

      /*******************************************************************************/
      /* Check and mark which measurement(s) cause interrupt */
      if ((irqs & ST25R3916_IRQ_MASK_WAM) != 0U) {
        gRFAL.wum.info.indAmp.irqWu = true;
        gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
      }

      if ((irqs & ST25R3916_IRQ_MASK_WPH) != 0U) {
        gRFAL.wum.info.indPha.irqWu = true;
        gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
      }

#ifdef ST25R3916
      if ((irqs & ST25R3916_IRQ_MASK_WCAP) != 0U) {
        gRFAL.wum.info.cap.irqWu = true;
        gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
      }
#endif /* ST25R3916 */

      if ((irqs & ST25R3916_IRQ_MASK_WT) != 0U) {
        gRFAL.wum.info.irqWut = true;

        /*******************************************************************************/
        if (gRFAL.wum.cfg.swTagDetect) {
          woke = false;

          /* Enable Ready mode and wait the settle time if AAT is used */
          if (ST25R3916_IO_IsAATOn()) {
            ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_OP_CONTROL, (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_wu), ST25R3916_REG_OP_CONTROL_en);
            NeonRTOS_Sleep(RFAL_ST25R3916_AAT_SETTLE);
          } else {
            /* Disable wu mode - symmetric to above */
            ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu, 0);
            ST25R3916_OscOn();
          }


          /*******************************************************************************/
          if (gRFAL.wum.cfg.indAmp.enabled) {
            /* Perform amplitude measurement */
            ST25R3916_MeasureAmplitude(&reg);

            /* Update last measurement info */
            gRFAL.wum.info.indAmp.lastMeas = reg;

            /* Convert inputs to TD format */
            value = RFal_ConvTDFormat(reg);
            delta = RFal_ConvTDFormat(gRFAL.wum.cfg.indAmp.delta);
            delta |= RFal_AddFracTDFormat(gRFAL.wum.cfg.indAmp.fracDelta);

            /* Set first measurement as reference */
            if (gRFAL.wum.cfg.indAmp.reference == 0U) {
              gRFAL.wum.cfg.indAmp.reference = value;
            }

            /* Check if device should be woken */
            if ((value >= (gRFAL.wum.cfg.indAmp.reference + delta)) ||
                (value <= (gRFAL.wum.cfg.indAmp.reference - delta))) {
              woke = true;
              gRFAL.wum.info.indAmp.irqWu = true;
              gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
              /* continue wake-up as for HW */
            }

            /* Update moving reference if enabled */
            if ((gRFAL.wum.cfg.indAmp.autoAvg) && ((gRFAL.wum.cfg.indAmp.aaInclMeas) || (!woke))) {
              gRFAL.wum.cfg.indAmp.reference = RFal_WakeUpModeFilter(gRFAL.wum.cfg.indAmp.reference, value, (RFAL_WU_MIN_WEIGHT_VAL << (uint8_t)gRFAL.wum.cfg.indAmp.aaWeight));
            }
          }

          /*******************************************************************************/
          if (gRFAL.wum.cfg.indPha.enabled) {
            /* Perform Phase measurement */
            ST25R3916_MeasurePhase(&reg);

            /* Update last measurement info */
            gRFAL.wum.info.indPha.lastMeas = reg;

            /* Convert inputs to TD format */
            value = RFal_ConvTDFormat(reg);
            delta = RFal_ConvTDFormat(gRFAL.wum.cfg.indPha.delta);
            delta |= RFal_AddFracTDFormat(gRFAL.wum.cfg.indPha.fracDelta);

            /* Set first measurement as reference */
            if (gRFAL.wum.cfg.indPha.reference == 0U) {
              gRFAL.wum.cfg.indPha.reference = value;
            }

            /* Check if device should be woken */
            if ((value >= (gRFAL.wum.cfg.indPha.reference + delta)) ||
                (value <= (gRFAL.wum.cfg.indPha.reference - delta))) {
              woke = true;
              gRFAL.wum.info.indPha.irqWu = true;
              gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
              /* continue wake-up as for HW */
            }

            /* Update moving reference if enabled */
            if ((gRFAL.wum.cfg.indPha.autoAvg) && ((gRFAL.wum.cfg.indPha.aaInclMeas) || (!woke))) {
              gRFAL.wum.cfg.indPha.reference = RFal_WakeUpModeFilter(gRFAL.wum.cfg.indPha.reference, value, (RFAL_WU_MIN_WEIGHT_VAL << (uint8_t)gRFAL.wum.cfg.indPha.aaWeight));
            }
          }

          /* Re-Enable low power Wake-Up mode for wto to trigger another measurement(s) */
          ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_OP_CONTROL, (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_wu), (ST25R3916_REG_OP_CONTROL_wu));
        }
      }
      break;

    default:
      /* MISRA 16.4: no empty default statement (a comment being enough) */
      break;
  }
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_WakeUpModeStop(void)
{
  if (gRFAL.state != RFAL_STATE_WUM) {
    return NFC_WrongState;
  }

  gRFAL.wum.state = RFAL_WUM_STATE_NOT_INIT;

  /* Disable Wake-Up Mode */
  ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);
  ST25R3916_IO_DisableInterrupts((ST25R3916_IRQ_MASK_WT | ST25R3916_IRQ_MASK_WAM | ST25R3916_IRQ_MASK_WPH | ST25R3916_IRQ_MASK_WCAP));

  /* Stop any ongoing activity */
  ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_STOP);
  /* Re-Enable External Field Detector as: Automatics */
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en_fd_mask, ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);

  /* Re-Enable the Oscillator */
  ST25R3916_OscOn();

  /* Set Analog configurations for Wake-up Off event */
  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_WAKEUP_OFF));

  return NFC_OK;
}

NFC_OpResult RFal_LowPowerModeStart(RFal_LpMode mode)
{
  /* Check if RFAL is not initialized */
  if (gRFAL.state < RFAL_STATE_INIT) {
    return NFC_WrongState;
  }

  /* Check if mode is supported */
  if (mode != RFal_LP_MODE_PD) {
    return NFC_Unsupport;
  }

  /* Stop any ongoing activity and set the device in low power by disabling oscillator, transmitter, receiver and external field detector */
  ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_STOP);
  ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_OP_CONTROL, (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_rx_en |
                                                      ST25R3916_REG_OP_CONTROL_wu | ST25R3916_REG_OP_CONTROL_tx_en |
                                                      ST25R3916_REG_OP_CONTROL_en_fd_mask));

  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_LOWPOWER_ON));

  gRFAL.state         = RFAL_STATE_IDLE;
  gRFAL.lpm.isRunning = true;

  return NFC_OK;
}
/*******************************************************************************/
NFC_OpResult RFal_LowPowerModeStop(void)
{
  NFC_OpResult ret;

  /* Check if RFAL is on right state */
  if (!gRFAL.lpm.isRunning) {
    return NFC_WrongState;
  }

  /* Re-enable device */
  ret = ST25R3916_OscOn();
  if(ret < NFC_OK)
  {
    return ret;
  }

  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en_fd_mask, ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);

  RFal_ST25R3916_SetAnalogConfig((RFAL_ST25R3916_ANALOG_CONFIG_TECH_CHIP | RFAL_ST25R3916_ANALOG_CONFIG_CHIP_LOWPOWER_OFF));

  gRFAL.state         = RFAL_STATE_INIT;
  gRFAL.lpm.isRunning = false;
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipWriteReg(uint16_t reg, const uint8_t *values, uint8_t len)
{
  if (!ST25R3916_IO_IsRegValid((uint8_t)reg)) {
    return NFC_InvalidParameter;
  }

  return ST25R3916_IO_WriteMultipleRegisters((uint8_t)reg, values, len);
}

/*******************************************************************************/
NFC_OpResult RFal_ChipReadReg(uint16_t reg, uint8_t *values, uint8_t len)
{
  if (!ST25R3916_IO_IsRegValid((uint8_t)reg)) {
    return NFC_InvalidParameter;
  }

  return ST25R3916_IO_ReadMultipleRegisters((uint8_t)reg, values, len);
}

/*******************************************************************************/
NFC_OpResult RFal_ChipExecCmd(uint16_t cmd)
{
  if (!ST25R3916_IsCmdValid((uint8_t)cmd)) {
    return NFC_InvalidParameter;
  }

  return ST25R3916_IO_ExecuteCommand((uint8_t) cmd);
}

/*******************************************************************************/
NFC_OpResult RFal_ChipWriteTestReg(uint16_t reg, uint8_t value)
{
  return ST25R3916_IO_WriteTestRegister((uint8_t)reg, value);
}

/*******************************************************************************/
NFC_OpResult RFal_ChipReadTestReg(uint16_t reg, uint8_t *value)
{
  return ST25R3916_IO_ReadTestRegister((uint8_t)reg, value);
}

/*******************************************************************************/
NFC_OpResult RFal_ChipChangeRegBits(uint16_t reg, uint8_t valueMask, uint8_t value)
{
  if (!ST25R3916_IO_IsRegValid((uint8_t)reg)) {
    return NFC_InvalidParameter;
  }

  return ST25R3916_IO_ChangeRegisterBits((uint8_t)reg, valueMask, value);
}

/*******************************************************************************/
NFC_OpResult RFal_ChipChangeTestRegBits(uint16_t reg, uint8_t valueMask, uint8_t value)
{
  ST25R3916_IO_ChangeTestRegisterBits((uint8_t)reg, valueMask, value);
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipSetRFO(uint8_t rfo)
{
  return ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_TX_DRIVER, ST25R3916_REG_TX_DRIVER_d_res_mask, rfo);
}

/*******************************************************************************/
NFC_OpResult RFal_ChipGetRFO(uint8_t *result)
{
  NFC_OpResult ret;

  ret = ST25R3916_IO_ReadRegister(ST25R3916_REG_TX_DRIVER, result);

  (*result) = ((*result) & ST25R3916_REG_TX_DRIVER_d_res_mask);

  return ret;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasureAmplitude(uint8_t *result)
{
  NFC_OpResult err;
  uint8_t reg_opc, reg_mode, reg_conf1, reg_conf2, reg_auxmod;

  /* Save registers which will be adjusted below */
  ST25R3916_IO_ReadRegister(ST25R3916_REG_OP_CONTROL, &reg_opc);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_MODE, &reg_mode);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_RX_CONF1, &reg_conf1);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_RX_CONF2, &reg_conf2);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AUX_MOD, &reg_auxmod);

  /* Set values as per defaults of DS. These regs/bits influence receiver chain and change amplitude */
  /* Doing so achieves an amplitude comparable over a complete polling cycle */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_OP_CONTROL, (reg_opc & ~ST25R3916_REG_OP_CONTROL_rx_chn));
  ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, (ST25R3916_REG_MODE_om_iso14443a | ST25R3916_REG_MODE_targ_init |
                                              ST25R3916_REG_MODE_tr_am_ook | ST25R3916_REG_MODE_nfc_ar_off));

  ST25R3916_IO_WriteRegister(ST25R3916_REG_RX_CONF1, (reg_conf1 & ~ST25R3916_REG_RX_CONF1_ch_sel_AM));
  ST25R3916_IO_WriteRegister(ST25R3916_REG_RX_CONF2, ((reg_conf2 & ~(ST25R3916_REG_RX_CONF2_demod_mode | ST25R3916_REG_RX_CONF2_amd_sel))
                                                  | ST25R3916_REG_RX_CONF2_amd_sel_peak));

#ifdef ST25R3916B
  /* Disable AWS for Amplitude Measurement */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_AUX_MOD, (reg_auxmod & ~ST25R3916_REG_AUX_MOD_rgs_am));
#endif /* ST25R3916B */
  /* Perform the actual measurement */
  err = ST25R3916_MeasureAmplitude(result);

  /* Restore values */
  ST25R3916_IO_WriteRegister(ST25R3916_REG_OP_CONTROL, reg_opc);
  ST25R3916_IO_WriteRegister(ST25R3916_REG_MODE, reg_mode);
  ST25R3916_IO_WriteRegister(ST25R3916_REG_RX_CONF1, reg_conf1);
  ST25R3916_IO_WriteRegister(ST25R3916_REG_RX_CONF2, reg_conf2);
  ST25R3916_IO_WriteRegister(ST25R3916_REG_AUX_MOD, reg_auxmod);

  return err;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasurePhase(uint8_t *result)
{
  ST25R3916_MeasurePhase(result);

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasureCapacitance(uint8_t *result)
{
  ST25R3916_MeasureCapacitance(result);

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasurePowerSupply(uint8_t param, uint8_t *result)
{
  *result = ST25R3916_MeasurePowerSupply(param);

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasureIQ(int8_t *resI, int8_t *resQ)
{
  if (resI != NULL) {
    (*resI) = 0;
  }

  if (resQ != NULL) {
    (*resQ) = 0;
  }

  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipMeasureCombinedIQ(uint8_t *result)
{
  if (result != NULL) {
    (*result) = 0U;
  }

  return NFC_Unsupport;
}

/*******************************************************************************/
NFC_OpResult RFal_ChipSetAntennaMode(bool single, bool rfiox)
{
  return ST25R3916_SetAntennaMode(single, rfiox);
}

/*******************************************************************************/

//extern uint8_t invalid_size_of_stream_configs[(sizeof(struct ST25R3916_IO_StreamConfig) == sizeof(struct RFal_Iso15693StreamConfig)) ? 1 : (-1)];

#endif //CONFIG_NFC_READER_DEVICE_ST25R3916 || CONFIG_NFC_READER_DEVICE_ST25R3916B
