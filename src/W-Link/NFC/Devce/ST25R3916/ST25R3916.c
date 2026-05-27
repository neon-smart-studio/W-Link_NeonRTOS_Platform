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

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "NeonRTOS.h"

#include "ST25R3916_Def.h"

#include "ST25R3916_IO.h"

#include "ST25R3916.h"

#define ST25R3916_NRT_MAX                     0xFFFFU /*!< Max Register value of NRT                                           */

static uint32_t gST25R3916NRT_64fcs = 0;

/*******************************************************************************/
NFC_OpResult ST25R3916_OscOn(void)
{
  /* Check if oscillator is already turned on and stable                                                */
  /* Use ST25R3916_REG_OP_CONTROL_en instead of ST25R3916_REG_AUX_DISPLAY_osc_ok to be on the safe side */
  if (!ST25R3916_IO_CheckReg(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en, ST25R3916_REG_OP_CONTROL_en)) {
    /* Clear any eventual previous oscillator frequency stable IRQ and enable it */
    ST25R3916_IO_ClearAndEnableInterrupts(ST25R3916_IRQ_MASK_OSC);

    /* Clear any oscillator IRQ that was potentially pending on ST25R */
    ST25R3916_IO_GetInterrupt(ST25R3916_IRQ_MASK_OSC);

    /* Enable oscillator and regulator output */
    ST25R3916_IO_SetRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en);

    /* Wait for the oscillator interrupt */
    ST25R3916_IO_WaitForInterruptsTimed(ST25R3916_IRQ_MASK_OSC, ST25R3916_TOUT_OSC_STABLE);
    ST25R3916_IO_DisableInterrupts(ST25R3916_IRQ_MASK_OSC);
  }

  if (!ST25R3916_IO_CheckReg(ST25R3916_REG_AUX_DISPLAY, ST25R3916_REG_AUX_DISPLAY_osc_ok, ST25R3916_REG_AUX_DISPLAY_osc_ok)) {
    return NFC_System;
  }

  return NFC_OK;
}

NFC_OpResult ST25R3916_ExecuteCommandAndGetResult(uint8_t cmd, uint8_t resReg, uint8_t tout, uint8_t *result)
{
  /* Clear and enable Direct Command interrupt */
  ST25R3916_IO_GetInterrupt(ST25R3916_IRQ_MASK_DCT);
  ST25R3916_IO_EnableInterrupts(ST25R3916_IRQ_MASK_DCT);

  ST25R3916_IO_ExecuteCommand(cmd);

  ST25R3916_IO_WaitForInterruptsTimed(ST25R3916_IRQ_MASK_DCT, tout);
  ST25R3916_IO_DisableInterrupts(ST25R3916_IRQ_MASK_DCT);

  /* After execution read out the result if the pointer is not NULL */
  if (result != NULL) {
    ST25R3916_IO_ReadRegister(resReg, result);
  }

  return NFC_OK;

}
/*******************************************************************************/
uint8_t ST25R3916_MeasurePowerSupply(uint8_t mpsv)
{
  uint8_t result;

  /* Set the source of direct command: Measure Power Supply Voltage */
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_mpsv_mask, mpsv);

  /* Execute command: Measure Power Supply Voltage */
  ST25R3916_ExecuteCommandAndGetResult(ST25R3916_CMD_MEASURE_VDD, ST25R3916_REG_AD_RESULT, ST25R3916_TOUT_MEASURE_VDD, &result);

  return result;
}

/*******************************************************************************/
uint16_t ST25R3916_MeasureVoltage(uint8_t mpsv)
{
  uint8_t result;
  uint16_t mV;

  result = ST25R3916_MeasurePowerSupply(mpsv);

  /* Convert cmd output into mV (each step represents 23.4 mV )*/
  mV  = ((uint16_t)result) * 23U;
  mV += (((((uint16_t)result) * 4U) + 5U) / 10U);

  return mV;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_AdjustRegulators(uint16_t *result_mV)
{
  uint8_t result;

  /* Reset logic and set regulated voltages to be defined by result of Adjust Regulators command */
  ST25R3916_IO_SetRegisterBits(ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_reg_s);
  ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_reg_s);

  /* Execute Adjust regulators cmd and retrieve result */
  ST25R3916_ExecuteCommandAndGetResult(ST25R3916_CMD_ADJUST_REGULATORS, ST25R3916_REG_REGULATOR_RESULT, ST25R3916_TOUT_ADJUST_REGULATORS, &result);

  /* Calculate result in mV */
  result >>= ST25R3916_REG_REGULATOR_RESULT_reg_shift;

  if (result_mV != NULL) {
    if (ST25R3916_IO_CheckReg(ST25R3916_REG_IO_CONF2, ST25R3916_REG_IO_CONF2_sup3V, ST25R3916_REG_IO_CONF2_sup3V)) {
      result -= ((result > 4U) ? (5U) : 0U);        /* In 3.3V mode [0,4] are not used                       */
      *result_mV = 2400U;                          /* Minimum regulated voltage 2.4V in case of 3.3V supply */
    } else {
      *result_mV = 3600U;                          /* Minimum regulated voltage 3.6V in case of 5V supply   */
    }

    *result_mV += (uint16_t)result * 100U;           /* 100mV steps in both 3.3V and 5V supply                */
  }
  return NFC_OK;

}

/*******************************************************************************/
NFC_OpResult ST25R3916_MeasureAmplitude(uint8_t *result)
{
  return ST25R3916_ExecuteCommandAndGetResult(ST25R3916_CMD_MEASURE_AMPLITUDE, ST25R3916_REG_AD_RESULT, ST25R3916_TOUT_MEASURE_AMPLITUDE, result);
}

/*******************************************************************************/
NFC_OpResult ST25R3916_MeasurePhase(uint8_t *result)
{
  return ST25R3916_ExecuteCommandAndGetResult(ST25R3916_CMD_MEASURE_PHASE, ST25R3916_REG_AD_RESULT, ST25R3916_TOUT_MEASURE_PHASE, result);
}

/*******************************************************************************/
NFC_OpResult ST25R3916_MeasureCapacitance(uint8_t *result)
{
#ifdef ST25R3916B
  return NFC_Unsupport;
#else
  return ST25R3916_ExecuteCommandAndGetResult(ST25R3916_CMD_MEASURE_CAPACITANCE, ST25R3916_REG_AD_RESULT, ST25R3916_TOUT_MEASURE_CAPACITANCE, result);
#endif /* ST25R3916B */
}

/*******************************************************************************/
NFC_OpResult ST25R3916_CalibrateCapacitiveSensor(uint8_t *result)
{
#ifdef ST25R3916B
  return NFC_Unsupport;
#else
  NFC_OpResult ret;
  uint8_t    res;

  /* Clear Manual calibration values to enable automatic calibration mode */
  ST25R3916_IO_ClearRegisterBits(ST25R3916_REG_CAP_SENSOR_CONTROL, ST25R3916_REG_CAP_SENSOR_CONTROL_cs_mcal_mask);

  /* Execute automatic calibration */
  ret = ST25R3916_ExecuteCommandAndGetResult(ST25R3916_CMD_CALIBRATE_C_SENSOR, ST25R3916_REG_CAP_SENSOR_RESULT, ST25R3916_TOUT_CALIBRATE_CAP_SENSOR, &res);

  /* Check whether the calibration was successull */
  if (((res & ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_end) != ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_end) ||
      ((res & ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_err) == ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_err) || (ret != NFC_OK)) {
    return NFC_IO_Error;
  }

  if (result != NULL) {
    (*result) = (uint8_t)(res >> ST25R3916_REG_CAP_SENSOR_RESULT_cs_cal_shift);
  }

  return NFC_OK;
#endif /* ST25R3916B */
}

/*******************************************************************************/
NFC_OpResult ST25R3916_SetBitrate(ST25R3916_BitRate txrate, ST25R3916_BitRate rxrate)
{
  uint8_t reg;

  ST25R3916_IO_ReadRegister(ST25R3916_REG_BIT_RATE, &reg);
  if (rxrate != ST25R3916_BitRate_DO_NOT_SET) {
    if (rxrate > ST25R3916_BitRate_848) {
      return NFC_InvalidParameter;
    }

    reg = (uint8_t)(reg & ~ST25R3916_REG_BIT_RATE_rxrate_mask);     /* MISRA 10.3 */
    reg |= rxrate << ST25R3916_REG_BIT_RATE_rxrate_shift;
  }
  if (txrate != ST25R3916_BitRate_DO_NOT_SET) {
    if (txrate > ST25R3916_BitRate_6780) {
      return NFC_InvalidParameter;
    }

    reg = (uint8_t)(reg & ~ST25R3916_REG_BIT_RATE_txrate_mask);     /* MISRA 10.3 */
    reg |= txrate << ST25R3916_REG_BIT_RATE_txrate_shift;

  }
  return ST25R3916_IO_WriteRegister(ST25R3916_REG_BIT_RATE, reg);
}

/*******************************************************************************/
NFC_OpResult ST25R3916_PerformCollisionAvoidance(uint8_t FieldONCmd, uint8_t pdThreshold, uint8_t caThreshold, uint8_t nTRFW)
{
  uint8_t    treMask;
  uint32_t   irqs;
  NFC_OpResult err;

  if ((FieldONCmd != ST25R3916_CMD_INITIAL_RF_COLLISION) && (FieldONCmd != ST25R3916_CMD_RESPONSE_RF_COLLISION_N)) {
    return NFC_InvalidParameter;
  }

  err = NFC_InternalError;

  /* Check if new thresholds are to be applied */
  if ((pdThreshold != ST25R3916_THRESHOLD_DO_NOT_SET) || (caThreshold != ST25R3916_THRESHOLD_DO_NOT_SET)) {
    treMask = 0;

    if (pdThreshold != ST25R3916_THRESHOLD_DO_NOT_SET) {
      treMask |= ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_mask;
    }

    if (caThreshold != ST25R3916_THRESHOLD_DO_NOT_SET) {
      treMask |= ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_mask;
    }

    /* Set Detection Threshold and|or Collision Avoidance Threshold */
    ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_FIELD_THRESHOLD_ACTV, treMask, (pdThreshold & ST25R3916_REG_FIELD_THRESHOLD_ACTV_trg_mask) | (caThreshold & ST25R3916_REG_FIELD_THRESHOLD_ACTV_rfe_mask));
  }

  /* Set n x TRFW */
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_n_mask, nTRFW);

  /*******************************************************************************/
  /* Enable and clear CA specific interrupts and execute command */
  ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_CAC | ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_APON));
  ST25R3916_IO_EnableInterrupts((ST25R3916_IRQ_MASK_CAC | ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_APON));

  ST25R3916_IO_ExecuteCommand(FieldONCmd);

  /*******************************************************************************/
  /* Wait for initial APON interrupt, indicating anticollision avoidance done and ST25R3916's
   * field is now on, or a CAC indicating a collision */
  irqs = ST25R3916_IO_WaitForInterruptsTimed((ST25R3916_IRQ_MASK_CAC | ST25R3916_IRQ_MASK_APON), ST25R3916_TOUT_CA);

  if ((ST25R3916_IRQ_MASK_CAC & irqs) != 0U) {       /* Collision occurred */
    err = NFC_RF_Collision;
  } else if ((ST25R3916_IRQ_MASK_APON & irqs) != 0U) {
    /* After APON wait for CAT interrupt, indication field was switched on minimum guard time has been fulfilled */
    irqs = ST25R3916_IO_WaitForInterruptsTimed((ST25R3916_IRQ_MASK_CAT), ST25R3916_TOUT_CA);

    if ((ST25R3916_IRQ_MASK_CAT & irqs) != 0U) {                            /* No Collision detected, Field On */
      err = NFC_OK;
    }
  } else {
    /* MISRA 15.7 - Empty else */
  }

  /* Clear any previous External Field events and disable CA specific interrupts */
  ST25R3916_IO_GetInterrupt((ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_EON));
  ST25R3916_IO_DisableInterrupts((ST25R3916_IRQ_MASK_CAC | ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_APON));

  return err;
}

/*******************************************************************************/
void ST25R3916_SetNumTxBits(uint16_t nBits)
{
  ST25R3916_IO_WriteRegister(ST25R3916_REG_NUM_TX_BYTES2, (uint8_t)((nBits >> 0) & 0xFFU));
  ST25R3916_IO_WriteRegister(ST25R3916_REG_NUM_TX_BYTES1, (uint8_t)((nBits >> 8) & 0xFFU));
}

/*******************************************************************************/
uint16_t ST25R3916_GetNumFIFOBytes(void)
{
  uint8_t  reg;
  uint16_t result;


  ST25R3916_IO_ReadRegister(ST25R3916_REG_FIFO_STATUS2, &reg);
  reg    = ((reg & ST25R3916_REG_FIFO_STATUS2_fifo_b_mask) >> ST25R3916_REG_FIFO_STATUS2_fifo_b_shift);
  result = ((uint16_t)reg << 8);

  ST25R3916_IO_ReadRegister(ST25R3916_REG_FIFO_STATUS1, &reg);
  result |= (((uint16_t)reg) & 0x00FFU);

  return result;
}

/*******************************************************************************/
uint8_t ST25R3916_GetNumFIFOLastBits(void)
{
  uint8_t  reg;

  ST25R3916_IO_ReadRegister(ST25R3916_REG_FIFO_STATUS2, &reg);

  return ((reg & ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask) >> ST25R3916_REG_FIFO_STATUS2_fifo_lb_shift);
}

/*******************************************************************************/
uint32_t ST25R3916_GetNoResponseTime(void)
{
  return gST25R3916NRT_64fcs;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_SetNoResponseTime(uint32_t nrt_64fcs)
{
  NFC_OpResult err;
  uint8_t    nrt_step;
  uint32_t   tmpNRT;

  tmpNRT = nrt_64fcs;       /* MISRA 17.8 */
  err    = NFC_OK;

  gST25R3916NRT_64fcs = tmpNRT;                                      /* Store given NRT value in 64/fc into local var       */
  nrt_step = ST25R3916_REG_TIMER_EMV_CONTROL_nrt_step_64fc;          /* Set default NRT in steps of 64/fc                   */


  if (tmpNRT > ST25R3916_NRT_MAX) {                                  /* Check if the given NRT value fits using 64/fc steps */
    nrt_step  = ST25R3916_REG_TIMER_EMV_CONTROL_nrt_step_4096_fc;  /* If not, change NRT set to 4096/fc                   */
    tmpNRT = ((tmpNRT + 63U) / 64U);                               /* Calculate number of steps in 4096/fc                */

    if (tmpNRT > ST25R3916_NRT_MAX) {                              /* Check if the NRT value fits using 64/fc steps       */
      tmpNRT = ST25R3916_NRT_MAX;                                /* Assign the maximum possible                         */
      err = NFC_InvalidParameter;                                           /* Signal parameter error                              */
    }
    gST25R3916NRT_64fcs = (64U * tmpNRT);
  }

  /* Set the ST25R3916 NRT step units and the value */
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_step, nrt_step);
  ST25R3916_IO_WriteRegister(ST25R3916_REG_NO_RESPONSE_TIMER1, (uint8_t)(tmpNRT >> 8U));
  ST25R3916_IO_WriteRegister(ST25R3916_REG_NO_RESPONSE_TIMER2, (uint8_t)(tmpNRT & 0xFFU));

  return err;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_SetStartNoResponseTimer(uint32_t nrt_64fcs)
{
  NFC_OpResult err;

  err = ST25R3916_SetNoResponseTime(nrt_64fcs);
  if (err == NFC_OK) {
    ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_START_NO_RESPONSE_TIMER);
  }

  return err;
}

/*******************************************************************************/
void ST25R3916_SetGPTime(uint16_t gpt_8fcs)
{
  ST25R3916_IO_WriteRegister(ST25R3916_REG_GPT1, (uint8_t)(gpt_8fcs >> 8));
  ST25R3916_IO_WriteRegister(ST25R3916_REG_GPT2, (uint8_t)(gpt_8fcs & 0xFFU));
}

/*******************************************************************************/
NFC_OpResult ST25R3916_SetStartGPTimer(uint16_t gpt_8fcs, uint8_t trigger_source)
{
  ST25R3916_SetGPTime(gpt_8fcs);
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_gptc_mask, trigger_source);

  /* If there's no trigger source, start GPT immediately */
  if (trigger_source == ST25R3916_REG_TIMER_EMV_CONTROL_gptc_no_trigger) {
    ST25R3916_IO_ExecuteCommand(ST25R3916_CMD_START_GP_TIMER);
  }

  return NFC_OK;
}

/*******************************************************************************/
bool ST25R3916_CheckChipID(uint8_t *rev)
{
  uint8_t ID;

  ID = 0;
  ST25R3916_IO_ReadRegister(ST25R3916_REG_IC_IDENTITY, &ID);

  /* Check if IC Identity Register contains ST25R3916's IC type code */
#if defined(ST25R3916)
  if ((ID & ST25R3916_REG_IC_IDENTITY_ic_type_mask) != ST25R3916_REG_IC_IDENTITY_ic_type_st25r3916) {
    return false;
  }
#elif defined(ST25R3916B)
  if (((ID & ST25R3916_REG_IC_IDENTITY_ic_type_mask) != ST25R3916_REG_IC_IDENTITY_ic_type_st25r3916B) ||
      ((ID & ST25R3916_REG_IC_IDENTITY_ic_rev_mask) < 1U)) {
    return false;
  }
#endif /* ST25R3916 */


  if (rev != NULL) {
    *rev = (ID & ST25R3916_REG_IC_IDENTITY_ic_rev_mask);
  }

  return true;

}

/*******************************************************************************/
NFC_OpResult ST25R3916_GetRegsDump(t_ST25R3916_IO_Regs *regDump)
{
  uint8_t regIt;

  if (regDump == NULL) {
    return NFC_InvalidParameter;
  }

  /* Dump Registers on space A */
  for (regIt = ST25R3916_REG_IO_CONF1; regIt <= ST25R3916_REG_IC_IDENTITY; regIt++) {
    ST25R3916_IO_ReadRegister(regIt, &regDump->RsA[regIt]);
  }

  regIt = 0;

  /* Read non-consecutive Registers on space B */
  ST25R3916_IO_ReadRegister(ST25R3916_REG_EMD_SUP_CONF,      &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_SUBC_START_TIME,   &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_P2P_RX_CONF,       &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_CORR_CONF1,        &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_CORR_CONF2,        &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_SQUELCH_TIMER,     &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_FIELD_ON_GT,       &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AUX_MOD,           &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_TX_DRIVER_TIMING,  &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_RES_AM_MOD,        &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_TX_DRIVER_STATUS,  &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_REGULATOR_RESULT,  &regDump->RsB[regIt++]);

#ifdef ST25R3916B
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AWS_CONF1,  &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AWS_CONF2,  &regDump->RsB[regIt++]);
#endif /* ST25R3916B */

  ST25R3916_IO_ReadRegister(ST25R3916_REG_OVERSHOOT_CONF1,   &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_OVERSHOOT_CONF2,   &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_UNDERSHOOT_CONF1,  &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_UNDERSHOOT_CONF2,  &regDump->RsB[regIt++]);

#ifdef ST25R3916B
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AWS_TIME1,   &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AWS_TIME2,   &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AWS_TIME3,   &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AWS_TIME4,   &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AWS_TIME5,   &regDump->RsB[regIt++]);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_AWS_RC_CAL,  &regDump->RsB[regIt++]);
#endif /* ST25R3916B */

  return NFC_OK;
}

/*******************************************************************************/
bool ST25R3916_IsCmdValid(uint8_t cmd)
{
  if ((!((cmd >= ST25R3916_CMD_SET_DEFAULT)             && (cmd <= ST25R3916_CMD_RESPONSE_RF_COLLISION_N)))   &&
      (!((cmd >= ST25R3916_CMD_GOTO_SENSE)              && (cmd <= ST25R3916_CMD_GOTO_SLEEP)))                &&
      (!((cmd >= ST25R3916_CMD_MASK_RECEIVE_DATA)       && (cmd <= ST25R3916_CMD_MEASURE_AMPLITUDE)))         &&
      (!((cmd >= ST25R3916_CMD_RESET_RXGAIN)            && (cmd <= ST25R3916_CMD_ADJUST_REGULATORS)))         &&
      (!((cmd >= ST25R3916_CMD_CALIBRATE_DRIVER_TIMING) && (cmd <= ST25R3916_CMD_START_PPON2_TIMER)))         &&
#ifdef ST25R3916B
      (cmd != ST25R3916_CMD_RC_CAL)                                                                           &&
#endif /* ST25R3916B */
      (cmd != ST25R3916_CMD_SPACE_B_ACCESS)           && (cmd != ST25R3916_CMD_STOP_NRT)) {
    return false;
  }
  return true;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_StreamConfigure(const struct ST25R3916_IO_StreamConfig *config)
{
  uint8_t smd;
  uint8_t mode;

  smd = 0;

  if (config->useBPSK != 0U) {
    mode = ST25R3916_REG_MODE_om_bpsk_stream;
    if ((config->din < 2U) || (config->din > 4U)) { /* not in fc/4 .. fc/16 */
      return NFC_InvalidParameter;
    }
    smd |= ((4U - config->din) << ST25R3916_REG_STREAM_MODE_scf_shift);
  } else {
    mode = ST25R3916_REG_MODE_om_subcarrier_stream;
    if ((config->din < 3U) || (config->din > 6U)) { /* not in fc/8 .. fc/64 */
      return NFC_InvalidParameter;
    }
    smd |= ((6U - config->din) << ST25R3916_REG_STREAM_MODE_scf_shift);
    if (config->report_period_length == 0U) {
      return NFC_InvalidParameter;
    }
  }

  if ((config->dout < 1U) || (config->dout > 7U)) { /* not in fc/2 .. fc/128 */
    return NFC_InvalidParameter;
  }
  smd |= (7U - config->dout) << ST25R3916_REG_STREAM_MODE_stx_shift;

  if (config->report_period_length > 3U) {
    return NFC_InvalidParameter;
  }
  smd |= (config->report_period_length << ST25R3916_REG_STREAM_MODE_scp_shift);

  ST25R3916_IO_WriteRegister(ST25R3916_REG_STREAM_MODE, smd);
  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_mask, mode);

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_GetRSSI(uint16_t *amRssi, uint16_t *pmRssi)
{
  /*******************************************************************************/
  /* MISRA 8.9 An object should be defined at block scope if its identifier only appears in a single function */
  /*< ST25R3916  RSSI Display Reg values:      0   1   2   3   4   5   6    7    8   9    a     b    c    d  e  f */
  static const uint16_t ST25R3916_IO_Rssi2mV[16] = { 0, 20, 27, 37, 52, 72, 99, 136, 190, 262, 357, 500, 686, 950, 1150, 1150 };

  /* ST25R3916 2/3 stage gain reduction [dB]          0    0    0    0    0    3    6    9   12   15   18  na na na na na */
  static const uint16_t ST25R3916_IO_Gain2Percent[16] = { 100, 100, 100, 100, 100, 141, 200, 281, 398, 562, 794, 1, 1, 1, 1, 1 };
  /*******************************************************************************/

  uint8_t  rssi;
  uint8_t  gainRed;

  ST25R3916_IO_ReadRegister(ST25R3916_REG_RSSI_RESULT, &rssi);
  ST25R3916_IO_ReadRegister(ST25R3916_REG_GAIN_RED_STATE, &gainRed);

  if (amRssi != NULL) {
    *amRssi = (uint16_t)(((uint32_t)ST25R3916_IO_Rssi2mV[(rssi >> ST25R3916_REG_RSSI_RESULT_rssi_am_shift) ] * (uint32_t)ST25R3916_IO_Gain2Percent[(gainRed >> ST25R3916_REG_GAIN_RED_STATE_gs_am_shift) ]) / 100U);
  }

  if (pmRssi != NULL) {
    *pmRssi = (uint16_t)(((uint32_t)ST25R3916_IO_Rssi2mV[(rssi & ST25R3916_REG_RSSI_RESULT_rssi_pm_mask) ] * (uint32_t)ST25R3916_IO_Gain2Percent[(gainRed & ST25R3916_REG_GAIN_RED_STATE_gs_pm_mask) ]) / 100U);
  }

  return NFC_OK;
}
/*******************************************************************************/
NFC_OpResult ST25R3916_SetAntennaMode(bool single, bool rfiox)
{
  uint8_t val;

  val  = 0U;
  val |= ((single) ? ST25R3916_REG_IO_CONF1_single : 0U);
  val |= ((rfiox) ? ST25R3916_REG_IO_CONF1_rfo2   : 0U);

  ST25R3916_IO_ChangeRegisterBits(ST25R3916_REG_IO_CONF1, (ST25R3916_REG_IO_CONF1_single | ST25R3916_REG_IO_CONF1_rfo2), val);
  return NFC_OK;
}