/**
 ******************************************************************************
 * @file    HTS221Sensor.cpp
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Implementation of a HTS221 Humidity and Temperature sensor.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
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
 * Based on STMicroelectronics HTS221 driver
 * Modified by Neon Smart Studio for W-Link
 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "HTS221.h"

#include "HTS221_Register_Def.h"
#include "HTS221_Register.h"

HTS221_OpResult HTS221_Init()
{
  HTS221_OpResult op_status;
  
  /* Power down the device */
  op_status = HTS221_DeActivate();
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  /* Enable BDU */
  op_status = HTS221_Set_BduMode(HTS221_ENABLE);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  op_status = HTS221_SetODR(1.0f);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_DeInit()
{
  HTS221_OpResult op_status;
  
  /* Disable humidity and temperature sensor */
  op_status = HTS221_Disable();
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Enable(void)
{
  HTS221_OpResult op_status;
  
  /* Power up the device */
  op_status = HTS221_Activate();
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Disable(void)
{
  HTS221_OpResult op_status;
  
  /* Power off the device */
  op_status = HTS221_DeActivate();
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_ReadID(uint8_t *ht_id)
{
  HTS221_OpResult op_status;
  
  /* Read WHO AM I register */
  op_status = HTS221_Get_DeviceID(ht_id);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Reset(void)
{
    HTS221_OpResult op_status;
  
    uint8_t tmpreg;

    /* Read CTRL_REG2 register */
    op_status = HTS221_ReadReg(HTS221_CTRL_REG2, 1, &tmpreg);
    if(op_status < HTS221_OK)
    {
      return op_status;
    }

    /* Enable or Disable the reboot memory */
    tmpreg |= (0x01 << HTS221_BOOT_BIT);

    /* Write value to MEMS CTRL_REG2 regsister */
	  op_status = HTS221_WriteReg(HTS221_CTRL_REG2, 1, &tmpreg);
    if(op_status < HTS221_OK)
    {
      return op_status;
    }

    return HTS221_OK;
}

HTS221_OpResult HTS221_GetHumidity(float* pfData)
{
  HTS221_OpResult op_status;
  
  uint16_t uint16data = 0;

  /* Read data from HTS221. */
  op_status = HTS221_Get_Humidity(&uint16data);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *pfData = ( float )uint16data / 10.0f;

  return HTS221_OK;
}

HTS221_OpResult HTS221_GetTemperature(float* pfData)
{
  HTS221_OpResult op_status;
  
  int16_t int16data = 0;

  /* Read data from HTS221. */
  op_status = HTS221_Get_Temperature(&int16data);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *pfData = ( float )int16data / 10.0f;

  return HTS221_OK;
}

HTS221_OpResult HTS221_GetODR(float* odr)
{
  HTS221_OpResult op_status;
  
  HTS221_Odr_et odr_low_level;

  op_status = HTS221_Get_Odr(&odr_low_level);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  switch( odr_low_level )
  {
    case HTS221_ODR_ONE_SHOT:
      *odr =  0.0f;
      break;
    case HTS221_ODR_1HZ:
      *odr =  1.0f;
      break;
    case HTS221_ODR_7HZ:
      *odr =  7.0f;
      break;
    case HTS221_ODR_12_5HZ:
      *odr = 12.5f;
      break;
    default:
      *odr = -1.0f;
      return HTS221_InvalidParameter;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_SetODR(float odr)
{
  HTS221_OpResult op_status;
  
  HTS221_Odr_et new_odr;

  new_odr = ( odr <= 1.0f ) ? HTS221_ODR_1HZ
          : ( odr <= 7.0f ) ? HTS221_ODR_7HZ
          :                   HTS221_ODR_12_5HZ;

  op_status = HTS221_Set_Odr(new_odr);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}
