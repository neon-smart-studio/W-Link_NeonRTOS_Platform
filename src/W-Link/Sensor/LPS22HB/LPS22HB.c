/**
 ******************************************************************************
 * @file    LPS22HBSensor.cpp
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Implementation of a LPS22HB Pressure sensor.
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
 * Based on STMicroelectronics LPS22HB driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "LPS22HB.h"

#include "LPS22HB_Register_Def.h"
#include "LPS22HB_Register.h"

static uint8_t isEnabled;
static float Last_ODR;

LPS22HB_OpStatus LPS22HB_SetODR_When_Enabled(float odr);
LPS22HB_OpStatus LPS22HB_SetODR_When_Disabled(float odr);

LPS22HB_OpStatus LPS22HB_Init(void)
{
  LPS22HB_OpStatus op_status;

  op_status = LPS22HB_Set_PowerMode(LPS22HB_LowPower);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Power down the device */
  op_status = LPS22HB_Set_Odr(LPS22HB_ODR_ONE_SHOT);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Disable low-pass filter on LPS22HB pressure data */
  op_status = LPS22HB_Set_LowPassFilter(LPS22HB_DISABLE);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Set low-pass filter cutoff configuration*/
  op_status = LPS22HB_Set_LowPassFilterCutoff(LPS22HB_ODR_9);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Set block data update mode */
  op_status = LPS22HB_Set_Bdu(LPS22HB_BDU_NO_UPDATE);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Disable automatic increment for multi-byte read/write */
  op_status = LPS22HB_Set_AutomaticIncrementRegAddress(LPS22HB_DISABLE);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_DeInit(void)
{
  LPS22HB_OpStatus op_status;

  /* Disable pressure and temperature sensor */

  op_status = LPS22HB_Disable();
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_Enable(void)
{
  LPS22HB_OpStatus op_status;

  /* Check if the component is already enabled */
  if ( isEnabled == 1 )
  {
    return LPS22HB_OK;
  }

  op_status = LPS22HB_SetODR_When_Enabled(Last_ODR);
  {
    return op_status;
  }

  isEnabled = 1;

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_Disable(void)
{
  LPS22HB_OpStatus op_status;

  /* Check if the component is already disabled */
  if ( isEnabled == 0 )
  {
    return LPS22HB_OK;
  }

  /* Power down the device */
  op_status = LPS22HB_Set_Odr(LPS22HB_ODR_ONE_SHOT);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  isEnabled = 0;

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_ReadID(uint8_t *p_id)
{
  LPS22HB_OpStatus op_status;

  if(!p_id)
  {
    return LPS22HB_InvalidParameter;
  }

  /* Read WHO AM I register */
  op_status = LPS22HB_Get_DeviceID(p_id);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_Reset(void)
{
  LPS22HB_OpStatus op_status;

  /* Read WHO AM I register */
  op_status = LPS22HB_MemoryBoot();
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_GetPressure(float* pfData)
{
  LPS22HB_OpStatus op_status;

  int32_t int32data = 0;

  /* Read data from LPS22HB. */
  op_status = LPS22HB_Get_Pressure(&int32data);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *pfData = ( float )int32data / 100.0f;

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_GetTemperature(float *pfData)
{
  LPS22HB_OpStatus op_status;

  int16_t int16data = 0;

  /* Read data from LPS22HB. */
  op_status = LPS22HB_Get_Temperature(&int16data);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *pfData = ( float )int16data / 10.0f;

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_GetODR(float* odr)
{
  LPS22HB_OpStatus op_status;

  LPS22HB_Odr_et odr_low_level;

  op_status = LPS22HB_Get_Odr(&odr_low_level);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  switch( odr_low_level )
  {
    case LPS22HB_ODR_ONE_SHOT:
      *odr = 0.0f;
      break;
    case LPS22HB_ODR_1HZ:
      *odr = 1.0f;
      break;
    case LPS22HB_ODR_10HZ:
      *odr = 10.0f;
      break;
    case LPS22HB_ODR_25HZ:
      *odr = 25.0f;
      break;
    case LPS22HB_ODR_50HZ:
      *odr = 50.0f;
      break;
    case LPS22HB_ODR_75HZ:
      *odr = 75.0f;
      break;
    default:
      *odr = -1.0f;
      return LPS22HB_InvalidParameter;
  }

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_SetODR(float odr)
{
  LPS22HB_OpStatus op_status;

  if(isEnabled == 1)
  {
    op_status = LPS22HB_SetODR_When_Enabled(odr);
    if(op_status < LPS22HB_OK)
    {
      return op_status;
    }
  }
  else
  {
    op_status = LPS22HB_SetODR_When_Disabled(odr);
    if(op_status < LPS22HB_OK)
    {
      return op_status;
    }
  }

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_SetODR_When_Enabled(float odr)
{
  LPS22HB_OpStatus op_status;

  LPS22HB_Odr_et new_odr;

  new_odr = ( odr <=  1.0f ) ? LPS22HB_ODR_1HZ
          : ( odr <= 10.0f ) ? LPS22HB_ODR_10HZ
          : ( odr <= 25.0f ) ? LPS22HB_ODR_25HZ
          : ( odr <= 50.0f ) ? LPS22HB_ODR_50HZ
          :                    LPS22HB_ODR_75HZ;

  op_status = LPS22HB_Set_Odr(new_odr);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  op_status = LPS22HB_GetODR(&Last_ODR);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_SetODR_When_Disabled(float odr)
{
  Last_ODR = ( odr <=  1.0f ) ? 1.0f
           : ( odr <= 10.0f ) ? 10.0f
           : ( odr <= 25.0f ) ? 25.0f
           : ( odr <= 50.0f ) ? 50.0f
           :                    75.0f;

  return LPS22HB_OK;
}
