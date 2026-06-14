/**
 ******************************************************************************
 * @file    vl53l4cd_api.cpp
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    29 November 2021
 * @brief   Implementation of the VL53L4CD APIs.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2021 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/*
 * Based on STMicroelectronics VL53L4CD driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "VL53L4CD_Def.h"
#include "VL53L4CD_IO.h"
#include "VL53L4CD.h"

#define VL53L4CD_SOFT_RESET     ((uint16_t)0x0000))
#define VL53L4CD_I2C_SLAVE_DEVICE_ADDRESS      ((uint16_t)0x0001)
#define VL53L4CD_VHV_CONFIG_TIMEOUT_MACROP_LOOP_BOUND  ((uint16_t)0x0008)
#define VL53L4CD_XTALK_PLANE_OFFSET_KCPS ((uint16_t)0x0016)
#define VL53L4CD_XTALK_X_PLANE_GRADIENT_KCPS     ((uint16_t)0x0018)
#define VL53L4CD_XTALK_Y_PLANE_GRADIENT_KCPS     ((uint16_t)0x001A)
#define VL53L4CD_RANGE_OFFSET_MM     ((uint16_t)0x001E)
#define VL53L4CD_INNER_OFFSET_MM     ((uint16_t)0x0020)
#define VL53L4CD_OUTER_OFFSET_MM     ((uint16_t)0x0022)
#define VL53L4CD_I2C_FAST_MODE_PLUS     ((uint16_t)0x002D)
#define VL53L4CD_GPIO_HV_MUX_CTRL      ((uint16_t)0x0030)
#define VL53L4CD_GPIO_TIO_HV_STATUS    ((uint16_t)0x0031)
#define VL53L4CD_SYSTEM_INTERRUPT  ((uint16_t)0x0046)
#define VL53L4CD_RANGE_CONFIG_A     ((uint16_t)0x005E)
#define VL53L4CD_RANGE_CONFIG_B      ((uint16_t)0x0061)
#define VL53L4CD_RANGE_CONFIG_SIGMA_THRESH     ((uint16_t)0x0064)
#define VL53L4CD_MIN_COUNT_RATE_RTN_LIMIT_MCPS    ((uint16_t)0x0066)
#define VL53L4CD_INTERMEASUREMENT_MS ((uint16_t)0x006C)
#define VL53L4CD_THRESH_HIGH    ((uint16_t)0x0072)
#define VL53L4CD_THRESH_LOW     ((uint16_t)0x0074)
#define VL53L4CD_SYSTEM_INTERRUPT_CLEAR        ((uint16_t)0x0086)
#define VL53L4CD_SYSTEM_START     ((uint16_t)0x0087)
#define VL53L4CD_RESULT_RANGE_STATUS   ((uint16_t)0x0089)
#define VL53L4CD_RESULT_SPAD_NB   ((uint16_t)0x008C)
#define VL53L4CD_RESULT_SIGNAL_RATE   ((uint16_t)0x008E)
#define VL53L4CD_RESULT_AMBIENT_RATE   ((uint16_t)0x0090)
#define VL53L4CD_RESULT_SIGMA   ((uint16_t)0x0092)
#define VL53L4CD_RESULT_DISTANCE   ((uint16_t)0x0096)


#define VL53L4CD_RESULT_OSC_CALIBRATE_VAL      ((uint16_t)0x00DE)
#define VL53L4CD_FIRMWARE_SYSTEM_STATUS        ((uint16_t)0x00E5)
#define VL53L4CD_IDENTIFICATION_MODEL_ID       ((uint16_t)0x010F)

static const uint8_t VL53L4CD_DEFAULT_CONFIGURATION[] = {
  0x12, /* 0x2d : set bit 2 and 5 to 1 for fast plus mode (1MHz I2C),
   else don't touch */
  0x00, /* 0x2e : bit 0 if I2C pulled up at 1.8V, else set bit 0 to 1
   (pull up at AVDD) */
  0x00, /* 0x2f : bit 0 if GPIO pulled up at 1.8V, else set bit 0 to 1
  (pull up at AVDD) */
  0x11, /* 0x30 : set bit 4 to 0 for active high interrupt and 1 for active low
  (bits 3:0 must be 0x1), use SetInterruptPolarity() */
  0x02, /* 0x31 : bit 1 = interrupt depending on the polarity,
  use CheckForDataReady() */
  0x00, /* 0x32 : not user-modifiable */
  0x02, /* 0x33 : not user-modifiable */
  0x08, /* 0x34 : not user-modifiable */
  0x00, /* 0x35 : not user-modifiable */
  0x08, /* 0x36 : not user-modifiable */
  0x10, /* 0x37 : not user-modifiable */
  0x01, /* 0x38 : not user-modifiable */
  0x01, /* 0x39 : not user-modifiable */
  0x00, /* 0x3a : not user-modifiable */
  0x00, /* 0x3b : not user-modifiable */
  0x00, /* 0x3c : not user-modifiable */
  0x00, /* 0x3d : not user-modifiable */
  0xff, /* 0x3e : not user-modifiable */
  0x00, /* 0x3f : not user-modifiable */
  0x0F, /* 0x40 : not user-modifiable */
  0x00, /* 0x41 : not user-modifiable */
  0x00, /* 0x42 : not user-modifiable */
  0x00, /* 0x43 : not user-modifiable */
  0x00, /* 0x44 : not user-modifiable */
  0x00, /* 0x45 : not user-modifiable */
  0x20, /* 0x46 : interrupt configuration 0->level low detection, 1-> level high,
  2-> Out of window, 3->In window, 0x20-> New sample ready , TBC */
  0x0b, /* 0x47 : not user-modifiable */
  0x00, /* 0x48 : not user-modifiable */
  0x00, /* 0x49 : not user-modifiable */
  0x02, /* 0x4a : not user-modifiable */
  0x14, /* 0x4b : not user-modifiable */
  0x21, /* 0x4c : not user-modifiable */
  0x00, /* 0x4d : not user-modifiable */
  0x00, /* 0x4e : not user-modifiable */
  0x05, /* 0x4f : not user-modifiable */
  0x00, /* 0x50 : not user-modifiable */
  0x00, /* 0x51 : not user-modifiable */
  0x00, /* 0x52 : not user-modifiable */
  0x00, /* 0x53 : not user-modifiable */
  0xc8, /* 0x54 : not user-modifiable */
  0x00, /* 0x55 : not user-modifiable */
  0x00, /* 0x56 : not user-modifiable */
  0x38, /* 0x57 : not user-modifiable */
  0xff, /* 0x58 : not user-modifiable */
  0x01, /* 0x59 : not user-modifiable */
  0x00, /* 0x5a : not user-modifiable */
  0x08, /* 0x5b : not user-modifiable */
  0x00, /* 0x5c : not user-modifiable */
  0x00, /* 0x5d : not user-modifiable */
  0x01, /* 0x5e : not user-modifiable */
  0xcc, /* 0x5f : not user-modifiable */
  0x07, /* 0x60 : not user-modifiable */
  0x01, /* 0x61 : not user-modifiable */
  0xf1, /* 0x62 : not user-modifiable */
  0x05, /* 0x63 : not user-modifiable */
  0x00, /* 0x64 : Sigma threshold MSB (mm in 14.2 format for MSB+LSB),
   use SetSigmaThreshold(), default value 90 mm  */
  0xa0, /* 0x65 : Sigma threshold LSB */
  0x00, /* 0x66 : Min count Rate MSB (MCPS in 9.7 format for MSB+LSB),
   use SetSignalThreshold() */
  0x80, /* 0x67 : Min count Rate LSB */
  0x08, /* 0x68 : not user-modifiable */
  0x38, /* 0x69 : not user-modifiable */
  0x00, /* 0x6a : not user-modifiable */
  0x00, /* 0x6b : not user-modifiable */
  0x00, /* 0x6c : Intermeasurement period MSB, 32 bits register,
   use SetIntermeasurementInMs() */
  0x00, /* 0x6d : Intermeasurement period */
  0x0f, /* 0x6e : Intermeasurement period */
  0x89, /* 0x6f : Intermeasurement period LSB */
  0x00, /* 0x70 : not user-modifiable */
  0x00, /* 0x71 : not user-modifiable */
  0x00, /* 0x72 : distance threshold high MSB (in mm, MSB+LSB),
   use SetD:tanceThreshold() */
  0x00, /* 0x73 : distance threshold high LSB */
  0x00, /* 0x74 : distance threshold low MSB ( in mm, MSB+LSB),
   use SetD:tanceThreshold() */
  0x00, /* 0x75 : distance threshold low LSB */
  0x00, /* 0x76 : not user-modifiable */
  0x01, /* 0x77 : not user-modifiable */
  0x07, /* 0x78 : not user-modifiable */
  0x05, /* 0x79 : not user-modifiable */
  0x06, /* 0x7a : not user-modifiable */
  0x06, /* 0x7b : not user-modifiable */
  0x00, /* 0x7c : not user-modifiable */
  0x00, /* 0x7d : not user-modifiable */
  0x02, /* 0x7e : not user-modifiable */
  0xc7, /* 0x7f : not user-modifiable */
  0xff, /* 0x80 : not user-modifiable */
  0x9B, /* 0x81 : not user-modifiable */
  0x00, /* 0x82 : not user-modifiable */
  0x00, /* 0x83 : not user-modifiable */
  0x00, /* 0x84 : not user-modifiable */
  0x01, /* 0x85 : not user-modifiable */
  0x00, /* 0x86 : clear interrupt, use ClearInterrupt() */
  0x00  /* 0x87 : start ranging, use StartRanging() or StopRanging(),
   If you want an automatic start after VL53L4CD_init() call,
    put 0x40 in location 0x87 */
};

VL53L4CD_OpResult VL53L4CD_Init()
{
   return VL53L4CD_IO_Init();
}

VL53L4CD_OpResult VL53L4CD_DeInit()
{
   return VL53L4CD_IO_DeInit();
}

VL53L4CD_OpResult VL53L4CD_Power_Off()
{
   return VL53L4CD_IO_Power_Off();
}

VL53L4CD_OpResult VL53L4CD_Power_On()
{
   return VL53L4CD_IO_Power_On();
}

VL53L4CD_OpResult VL53L4CD_SetI2CAddress(uint8_t new_address)
{
   return VL53L4CD_IO_SetI2CAddress(new_address);
}

VL53L4CD_OpResult VL53L4CD_SensorInit()
{
  VL53L4CD_OpResult status;
  uint8_t Addr, tmp;
  uint16_t i;

  i = 0;
  do {
    status = VL53L4CD_IO_Read_Byte(VL53L4CD_FIRMWARE_SYSTEM_STATUS, &tmp);
    if(status < VL53L4CD_OK)
    {
        return status;
    }

    if (tmp == (uint8_t)0x3)
    { /* Sensor booted */
      break;
    }
    else if (i < (uint16_t)1000)
    {  /* Wait for boot */
      i++;
    }
    else
    { /* Timeout 1000ms reached */
      return VL53L4CD_SlaveTimeout;
    }

    NeonRTOS_Sleep(1);
  } while (1);

  /* Load default configuration */
  for (Addr = (uint8_t)0x2D; Addr <= (uint8_t)0x87; Addr++) {
    status = VL53L4CD_IO_Write_Byte(Addr, VL53L4CD_DEFAULT_CONFIGURATION[Addr - (uint8_t)0x2D]);
    if(status < VL53L4CD_OK)
    {
        return status;
    }
  }

  /* Start VHV */
  status = VL53L4CD_IO_Write_Byte(VL53L4CD_SYSTEM_START, (uint8_t)0x40);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  i = 0;
  do {
    status = VL53L4CD_CheckForDataReady(&tmp);
    if(status < VL53L4CD_OK)
    {
        return status;
    }

    if (tmp == (uint8_t)1)
    { /* Data ready */
      break;
    }
    else if (i < (uint16_t)1000)
    {  /* Wait for answer */
      i++;
    }
    else
    { /* Timeout 1000ms reached */
      return VL53L4CD_SlaveTimeout;
    }
    NeonRTOS_Sleep(1);
  } while (1);

  status = VL53L4CD_ClearInterrupt();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_StopRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }
  
  status = VL53L4CD_IO_Write_Byte(VL53L4CD_VHV_CONFIG_TIMEOUT_MACROP_LOOP_BOUND, (uint8_t)0x09);
  if(status < VL53L4CD_OK)
  {
      return status;
  }
  
  status = VL53L4CD_IO_Write_Byte(0x0B, (uint8_t)0);
  if(status < VL53L4CD_OK)
  {
      return status;
  }
  
  status = VL53L4CD_IO_Write_Word(0x0024, 0x500);
  if(status < VL53L4CD_OK)
  {
      return status;
  }
  
  status = VL53L4CD_SetRangeTiming(50, 0);
  if(status < VL53L4CD_OK)
  {
      return status;
  }
  
  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_GetSensorId(uint16_t *p_id)
{
  return VL53L4CD_IO_Read_Word(VL53L4CD_IDENTIFICATION_MODEL_ID, p_id);
}

VL53L4CD_OpResult VL53L4CD_ClearInterrupt()
{
  return VL53L4CD_IO_Write_Byte(VL53L4CD_SYSTEM_INTERRUPT_CLEAR, 0x01);
}

VL53L4CD_OpResult VL53L4CD_StartRanging()
{
  VL53L4CD_OpResult status;
  uint8_t data_ready;
  uint16_t i = 0;
  uint32_t tmp;

  status = VL53L4CD_RdDWord(VL53L4CD_INTERMEASUREMENT_MS, &tmp);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  /* Sensor runs in continuous mode */
  if (tmp == (uint32_t)0) {
    status = VL53L4CD_IO_Write_Byte(VL53L4CD_SYSTEM_START, 0x21);
    if(status < VL53L4CD_OK)
    {
        return status;
    }
  }
  /* Sensor runs in autonomous mode */
  else {
    status = VL53L4CD_IO_Write_Byte(VL53L4CD_SYSTEM_START, 0x40);
    if(status < VL53L4CD_OK)
    {
        return status;
    }
  }

  do {
    status = VL53L4CD_CheckForDataReady(&data_ready);
    if(status < VL53L4CD_OK)
    {
        return status;
    }

    if (data_ready == (uint8_t)1)
    { /* Data ready */
      break;
    } 
    else if (i < (uint16_t)1000)
    {  /* Wait for answer */
      i++;
    }
    else
    { /* Timeout 1000ms reached */
      return VL53L4CD_SlaveTimeout;
    }
    NeonRTOS_Sleep(1);
  } while (1);

  status = VL53L4CD_ClearInterrupt();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_StopRanging()
{
  return VL53L4CD_IO_Write_Byte(VL53L4CD_SYSTEM_START, 0x00);
}

VL53L4CD_OpResult VL53L4CD_CheckForDataReady(uint8_t *p_is_data_ready)
{
  VL53L4CD_OpResult status;
  uint8_t temp;
  uint8_t int_pol;

  status = VL53L4CD_IO_Read_Byte(VL53L4CD_GPIO_HV_MUX_CTRL, &temp);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  temp = temp & (uint8_t)0x10;
  temp = temp >> 4;

  if (temp == (uint8_t)1) {
    int_pol = (uint8_t)0;
  } else {
    int_pol = (uint8_t)1;
  }

  status = VL53L4CD_IO_Read_Byte(VL53L4CD_GPIO_TIO_HV_STATUS, &temp);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  if ((temp & (uint8_t)1) == int_pol) {
    *p_is_data_ready = (uint8_t)1;
  } else {
    *p_is_data_ready = (uint8_t)0;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_SetRangeTiming(uint32_t timing_budget_ms,
  uint32_t inter_measurement_ms)
{
  VL53L4CD_OpResult status;
  uint16_t clock_pll, osc_frequency, ms_byte;
  uint32_t macro_period_us = 0, timing_budget_us = 0, ls_byte, tmp;
  float inter_measurement_factor = (float)1.055;

  status = VL53L4CD_IO_Read_Word(0x0006, &osc_frequency);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  if (osc_frequency == (uint16_t)0)
  {
    return VL53L4CD_InvalidParameter;
  }

  timing_budget_us = timing_budget_ms * (uint32_t)1000;
  macro_period_us = (uint32_t)((uint32_t)2304 * ((uint32_t)0x40000000 / (uint32_t)osc_frequency)) >> 6;

  /* Timing budget check validity */
  if ((timing_budget_ms < (uint32_t)10) || (timing_budget_ms > (uint32_t)200) || (status != (uint8_t)0))
  {
    return VL53L4CD_InvalidParameter;
  }

  /* Sensor runs in continuous mode */
  if (inter_measurement_ms == (uint32_t)0) {
    status = VL53L4CD_IO_Write_DWord(VL53L4CD_INTERMEASUREMENT_MS, 0);
    if(status < VL53L4CD_OK)
    {
        return status;
    }

    timing_budget_us -= (uint32_t)2500;
  }
  /* Sensor runs in autonomous low power mode */
  else if (inter_measurement_ms > timing_budget_ms) {
    status = VL53L4CD_IO_Read_Word(VL53L4CD_RESULT_OSC_CALIBRATE_VAL, &clock_pll);
    if(status < VL53L4CD_OK)
    {
        return status;
    }

    clock_pll = clock_pll & (uint16_t)0x3FF;
    inter_measurement_factor = inter_measurement_factor * (float)inter_measurement_ms * (float)clock_pll;

    status = VL53L4CD_IO_Write_DWord(VL53L4CD_INTERMEASUREMENT_MS, (uint32_t)inter_measurement_factor);
    if(status < VL53L4CD_OK)
    {
        return status;
    }

    timing_budget_us -= (uint32_t)4300;
    timing_budget_us /= (uint32_t)2;

  }
  /* Invalid case */
  else {
    return VL53L4CD_InvalidParameter;
  }

  ms_byte = 0;
  timing_budget_us = timing_budget_us << 12;
  tmp = macro_period_us * (uint32_t)16;
  ls_byte = ((timing_budget_us + ((tmp >> 6) >> 1)) / (tmp >> 6)) - (uint32_t)1;

  while ((ls_byte & 0xFFFFFF00U) > 0U) {
    ls_byte = ls_byte >> 1;
    ms_byte++;
  }

  ms_byte = (uint16_t)(ms_byte << 8) + (uint16_t)(ls_byte & (uint32_t)0xFF);

  status = VL53L4CD_IO_Write_Word(VL53L4CD_RANGE_CONFIG_A, ms_byte);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  ms_byte = 0;
  tmp = macro_period_us * (uint32_t)12;
  ls_byte = ((timing_budget_us + ((tmp >> 6) >> 1)) / (tmp >> 6)) - (uint32_t)1;

  while ((ls_byte & 0xFFFFFF00U) > 0U) {
    ls_byte = ls_byte >> 1;
    ms_byte++;
  }

  ms_byte = (uint16_t)(ms_byte << 8) + (uint16_t)(ls_byte & (uint32_t)0xFF);

  status = VL53L4CD_IO_Write_Word(VL53L4CD_RANGE_CONFIG_B, ms_byte);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_GetRangeTiming(uint32_t *p_timing_budget_ms,
  uint32_t *p_inter_measurement_ms)
{
  VL53L4CD_OpResult status;
  uint16_t osc_frequency = 1, range_config_macrop_high, clock_pll = 1;
  uint32_t tmp, ls_byte, ms_byte, macro_period_us;
  float clock_pll_factor = (float)1.065;

  /* Get InterMeasurement */
  status = VL53L4CD_RdDWord(VL53L4CD_INTERMEASUREMENT_MS, &tmp);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Read_Word(VL53L4CD_RESULT_OSC_CALIBRATE_VAL, &clock_pll);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  clock_pll = clock_pll & (uint16_t)0x3FF;
  clock_pll_factor = clock_pll_factor * (float)clock_pll;
  clock_pll = (uint16_t)clock_pll_factor;
  *p_inter_measurement_ms = (uint16_t)(tmp / (uint32_t)clock_pll);

  /* Get TimingBudget */
  status = VL53L4CD_IO_Read_Word(0x0006, &osc_frequency);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Read_Word(VL53L4CD_RANGE_CONFIG_A, &range_config_macrop_high);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  macro_period_us = (uint32_t)((uint32_t)2304 * ((uint32_t)0x40000000 / (uint32_t)osc_frequency)) >> 6;
  ls_byte = (range_config_macrop_high & (uint32_t)0x00FF) << 4;
  ms_byte = (range_config_macrop_high & (uint32_t)0xFF00) >> 8;
  ms_byte = (uint32_t)0x04 - (ms_byte - (uint32_t)1) - (uint32_t)1;

  macro_period_us = macro_period_us * (uint32_t)16;
  *p_timing_budget_ms = (((ls_byte + (uint32_t)1) * (macro_period_us >> 6)) - ((macro_period_us >> 6) >> 1)) >> 12;

  if (ms_byte < (uint8_t)12) {
    *p_timing_budget_ms = (uint32_t)(*p_timing_budget_ms
                                     >> (uint8_t)ms_byte);
  }

  /* Mode continuous */
  if (tmp == (uint32_t)0) {
    *p_timing_budget_ms += (uint32_t)2500;
  }
  /* Mode autonomous */
  else {
    *p_timing_budget_ms *= (uint32_t)2;
    *p_timing_budget_ms += (uint32_t)4300;
  }

  *p_timing_budget_ms = *p_timing_budget_ms / (uint32_t)1000;

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_GetResult(VL53L4CD_Result_t *p_result)
{
  VL53L4CD_OpResult status;
  uint16_t temp_16;
  uint8_t temp_8;
  uint8_t status_rtn[24] = { 255, 255, 255, 5, 2, 4, 1, 7, 3,
                             0, 255, 255, 9, 13, 255, 255, 255, 255, 10, 6,
                             255, 255, 11, 12
                           };

  status = VL53L4CD_IO_Read_Byte(VL53L4CD_RESULT_RANGE_STATUS, &temp_8);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  temp_8 = temp_8 & (uint8_t)0x1F;

  if (temp_8 < (uint8_t)24) {
    temp_8 = status_rtn[temp_8];
  }

  p_result->range_status = temp_8;

  status = VL53L4CD_IO_Read_Word(VL53L4CD_RESULT_SPAD_NB, &temp_16);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  p_result->number_of_spad = temp_16 / (uint16_t) 256;

  if (p_result->number_of_spad == 0) {
    p_result->range_status = 255;
    return status;
  }

  status = VL53L4CD_IO_Read_Word(VL53L4CD_RESULT_SIGNAL_RATE, &temp_16);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  p_result->signal_rate_kcps = temp_16 * (uint16_t) 8;

  status = VL53L4CD_IO_Read_Word(VL53L4CD_RESULT_AMBIENT_RATE, &temp_16);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  p_result->ambient_rate_kcps = temp_16 * (uint16_t) 8;

  status = VL53L4CD_IO_Read_Word(VL53L4CD_RESULT_SIGMA, &temp_16);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  p_result->sigma_mm = temp_16 / (uint16_t) 4;

  status = VL53L4CD_IO_Read_Word(VL53L4CD_RESULT_DISTANCE, &temp_16);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  p_result->distance_mm = temp_16;

  p_result->signal_per_spad_kcps = p_result->signal_rate_kcps / p_result->number_of_spad;
  p_result->ambient_per_spad_kcps = p_result->ambient_rate_kcps / p_result->number_of_spad;

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_SetOffset(int16_t OffsetValueInMm)
{
  VL53L4CD_OpResult status;
  uint16_t temp;

  temp = (uint16_t)((uint16_t)OffsetValueInMm * (uint16_t)4);

  status = VL53L4CD_IO_Write_Word(VL53L4CD_RANGE_OFFSET_MM, temp);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Word(VL53L4CD_INNER_OFFSET_MM, (uint8_t)0x0);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Word(VL53L4CD_OUTER_OFFSET_MM, (uint8_t)0x0);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_GetOffset(int16_t *p_offset)
{
  VL53L4CD_OpResult status;
  uint16_t temp;

  status = VL53L4CD_IO_Read_Word(VL53L4CD_RANGE_OFFSET_MM, &temp);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  temp = temp << 3;
  temp = temp >> 5;
  *p_offset = (int16_t)(temp);

  if (*p_offset > 1024)
  {
    *p_offset = *p_offset - 2048;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_SetXtalk(uint16_t XtalkValueKcps)
{
  VL53L4CD_OpResult status;

  status = VL53L4CD_IO_Write_Word(VL53L4CD_XTALK_X_PLANE_GRADIENT_KCPS, 0x0000);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Word(VL53L4CD_XTALK_Y_PLANE_GRADIENT_KCPS, 0x0000);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Word(VL53L4CD_XTALK_PLANE_OFFSET_KCPS, (XtalkValueKcps << 9) / (uint16_t)1000);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_GetXtalk(uint16_t *p_xtalk_kcps)
{
  VL53L4CD_OpResult status;

  status = VL53L4CD_IO_Read_Word(VL53L4CD_XTALK_PLANE_OFFSET_KCPS, p_xtalk_kcps);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  *p_xtalk_kcps = (uint16_t)(*p_xtalk_kcps * (uint16_t)1000) >> 9;

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_SetDetectionThresholds(uint16_t distance_low_mm, uint16_t distance_high_mm, uint8_t window)
{
  VL53L4CD_OpResult status;

  status = VL53L4CD_IO_Write_Byte(VL53L4CD_SYSTEM_INTERRUPT, window);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Word(VL53L4CD_THRESH_HIGH, distance_high_mm);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Word(VL53L4CD_THRESH_LOW, distance_low_mm);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_GetDetectionThresholds(uint16_t *p_distance_low_mm, uint16_t *p_distance_high_mm, uint8_t *p_window)
{
  VL53L4CD_OpResult status;

  status = VL53L4CD_IO_Read_Word(VL53L4CD_THRESH_HIGH, p_distance_high_mm);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Read_Word(VL53L4CD_THRESH_LOW, p_distance_low_mm);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Read_Byte(VL53L4CD_SYSTEM_INTERRUPT, p_window);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  *p_window = (*p_window & (uint8_t)0x7);

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_SetSignalThreshold(uint16_t signal_kcps)
{
  return VL53L4CD_IO_Write_Word(VL53L4CD_MIN_COUNT_RATE_RTN_LIMIT_MCPS, signal_kcps >> 3);
}

VL53L4CD_OpResult VL53L4CD_GetSignalThreshold(uint16_t *p_signal_kcps)
{
  VL53L4CD_OpResult status;
  uint16_t tmp = 0;

  status = VL53L4CD_IO_Read_Word(VL53L4CD_MIN_COUNT_RATE_RTN_LIMIT_MCPS, &tmp);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  *p_signal_kcps = tmp << 3;

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_SetSigmaThreshold(uint16_t sigma_mm)
{
  VL53L4CD_OpResult status;

  if (sigma_mm > (uint16_t)((uint16_t)0xFFFF >> 2))
  {
    return VL53L4CD_InvalidParameter;
  }

  status = VL53L4CD_IO_Write_Word(VL53L4CD_RANGE_CONFIG_SIGMA_THRESH, sigma_mm << 2);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_GetSigmaThreshold(uint16_t *p_sigma_mm)
{
  VL53L4CD_OpResult status;

  status = VL53L4CD_IO_Read_Word(VL53L4CD_RANGE_CONFIG_SIGMA_THRESH, p_sigma_mm);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  *p_sigma_mm = *p_sigma_mm >> 2;

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_StartTemperatureUpdate()
{
  VL53L4CD_OpResult status;
  uint8_t tmp = 0;
  uint16_t i = 0;

  status = VL53L4CD_IO_Write_Byte(VL53L4CD_VHV_CONFIG_TIMEOUT_MACROP_LOOP_BOUND, (uint8_t)0x81);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Byte(0x0B, (uint8_t)0x92);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Byte(VL53L4CD_SYSTEM_START, (uint8_t)0x40);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  do {
    status = VL53L4CD_CheckForDataReady(&tmp);
    if(status < VL53L4CD_OK)
    {
        return status;
    }

    if (tmp == (uint8_t)1)
    { /* Data ready */
      break;
    }
    else if (i < (uint16_t)1000)
    {  /* Wait for answer */
      i++;
    }
    else { /* Timeout 1000ms reached */
      return VL53L4CD_SlaveTimeout;
    }
    NeonRTOS_Sleep(1);
  } while (1);

  status = VL53L4CD_ClearInterrupt();
  if(status < VL53L4CD_OK)
  {
      return status;
  }
  status = VL53L4CD_StopRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Byte(VL53L4CD_VHV_CONFIG_TIMEOUT_MACROP_LOOP_BOUND, 0x09);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  status = VL53L4CD_IO_Write_Byte(0x0B, 0);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_CalibrateOffset(int16_t TargetDistInMm, int16_t *p_measured_offset_mm, int16_t nb_samples)
{
  VL53L4CD_OpResult status;
  uint8_t i, tmp;
  uint16_t j, tmpOff;
  int16_t AvgDistance = 0;
  VL53L4CD_Result_t results;

  if (((nb_samples < (int16_t)5) || (nb_samples > (int16_t)255)) || ((TargetDistInMm < (int16_t)50) || (TargetDistInMm > (int16_t)1000)))
  {
    return VL53L4CD_InvalidParameter;
  }

  status = VL53L4CD_IO_Write_Word(VL53L4CD_RANGE_OFFSET_MM, 0x0);
  status = VL53L4CD_IO_Write_Word(VL53L4CD_INNER_OFFSET_MM, 0x0);
  status = VL53L4CD_IO_Write_Word(VL53L4CD_OUTER_OFFSET_MM, 0x0);

  /* Device heat loop (10 samples) */
  status = VL53L4CD_StartRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  for (i = 0; i < (uint8_t)10; i++) {
    tmp = (uint8_t)0;
    j = (uint16_t)0;

    do {
      status = VL53L4CD_CheckForDataReady(&tmp);
      if(status < VL53L4CD_OK)
      {
          return status;
      }

      if (tmp == (uint8_t)1)
      { /* Data ready */
        break;
      }
      else if (j < (uint16_t)5000)
      { /* Wait for answer*/
        j++;
      }
      else
      { /* Timeout 5000ms reached */
        return VL53L4CD_SlaveTimeout;
      }
      NeonRTOS_Sleep(1);
    } while (1);

    status = VL53L4CD_GetResult(&results);
    if(status < VL53L4CD_OK)
    {
        return status;
    }
    status = VL53L4CD_ClearInterrupt();
    if(status < VL53L4CD_OK)
    {
        return status;
    }
  }

  status = VL53L4CD_StopRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  /* Device ranging */
  status = VL53L4CD_StartRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  for (i = 0; i < (uint8_t)nb_samples; i++) {
    tmp = (uint8_t)0;
    j = (uint16_t)0;
    
    do {
      status = VL53L4CD_CheckForDataReady(&tmp);
      if(status < VL53L4CD_OK)
      {
          return status;
      }

      if (tmp == (uint8_t)1)
      { /* Data ready */
        break;
      }
      else if (j < (uint16_t)5000)
      { /* Wait for answer*/
        j++;
      }
      else
      { /* Timeout 5000ms reached */
        return VL53L4CD_SlaveTimeout;
      }
      NeonRTOS_Sleep(1);
    } while (1);

    status = VL53L4CD_GetResult(&results);
    if(status < VL53L4CD_OK)
    {
        return status;
    }
    status = VL53L4CD_ClearInterrupt();
    if(status < VL53L4CD_OK)
    {
        return status;
    }

    AvgDistance += (int16_t)results.distance_mm;
  }

  status = VL53L4CD_StopRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  AvgDistance = AvgDistance / nb_samples;
  *p_measured_offset_mm = (int16_t)TargetDistInMm - AvgDistance;
  tmpOff = (uint16_t) * p_measured_offset_mm * (uint16_t)4;

  status = VL53L4CD_IO_Write_Word(VL53L4CD_RANGE_OFFSET_MM, tmpOff);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_CalibrateXtalk(int16_t TargetDistInMm, uint16_t *p_measured_xtalk_kcps, int16_t nb_samples)
{
  VL53L4CD_OpResult status;
  uint8_t i, tmp, continue_loop;
  float AverageSignal = (float)0.0;
  float AvgDistance = (float)0.0;
  float AverageSpadNb = (float)0.0;
  float tmp_xtalk;
  VL53L4CD_Result_t results;

  uint16_t calXtalk, j;

  if (((nb_samples < (int16_t)5) || (nb_samples > (int16_t)255)) || ((TargetDistInMm < (int16_t)50) || (TargetDistInMm > (int16_t)5000)))
  {
    return VL53L4CD_InvalidParameter;
  }

  /* Device heat loop (10 samples) */
  status = VL53L4CD_StartRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }
  for (i = 0; i < (uint8_t)10; i++) {
    tmp = (uint8_t)0;
    j = (uint16_t)0;
    continue_loop = (uint8_t)1;
    do {
      status = VL53L4CD_CheckForDataReady(&tmp);
      if(status < VL53L4CD_OK)
      {
          return status;
      }

      if (tmp == (uint8_t)1)
      { /* Data ready */
        break;
      }
      else if (j < (uint16_t)5000)
      { /* Wait for answer*/
        j++;
      }
      else
      { /* Timeout 5000ms reached */
        return VL53L4CD_SlaveTimeout;
      }
      NeonRTOS_Sleep(1);
    } while (1);

    status = VL53L4CD_GetResult(&results);
    if(status < VL53L4CD_OK)
    {
        return status;
    }
    status = VL53L4CD_ClearInterrupt();
    if(status < VL53L4CD_OK)
    {
        return status;
    }
  }
  status = VL53L4CD_StopRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  /* Device ranging loop */
  status = VL53L4CD_StartRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  for (i = 0; i < (uint8_t)nb_samples; i++) {
    tmp = (uint8_t)0;
    j = (uint16_t)0;
    continue_loop = (uint8_t)1;
    do {
      status = VL53L4CD_CheckForDataReady(&tmp);
      if(status < VL53L4CD_OK)
      {
          return status;
      }

      if (tmp == (uint8_t)1)
      { /* Data ready */
        break;
      }
      else if (j < (uint16_t)5000)
      { /* Wait for answer*/
        j++;
      }
      else
      { /* Timeout 5000ms reached */
        return VL53L4CD_SlaveTimeout;
      }
      NeonRTOS_Sleep(1);
    } while (1);

    status = VL53L4CD_GetResult(&results);
    if(status < VL53L4CD_OK)
    {
        return status;
    }
    status = VL53L4CD_ClearInterrupt();
    if(status < VL53L4CD_OK)
    {
        return status;
    }

    AvgDistance += (float)results.distance_mm;
    AverageSpadNb += (float)results.number_of_spad;
    AverageSignal += (float)results.signal_rate_kcps;
  }

  status = VL53L4CD_StopRanging();
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  AvgDistance = AvgDistance / (float)nb_samples;
  AverageSpadNb = AverageSpadNb / (float)nb_samples;
  AverageSignal = AverageSignal / (float)nb_samples;

  tmp_xtalk = (float)512.0 * (AverageSignal * ((float)1.0 - (AvgDistance / (float)TargetDistInMm))) / AverageSpadNb;
  calXtalk = (uint16_t)tmp_xtalk;
  *p_measured_xtalk_kcps = (uint16_t)(calXtalk * (uint16_t)1000) >> 9;

  status = VL53L4CD_IO_Write_Word(VL53L4CD_XTALK_PLANE_OFFSET_KCPS, calXtalk);
  if(status < VL53L4CD_OK)
  {
      return status;
  }

  return VL53L4CD_OK;
}
