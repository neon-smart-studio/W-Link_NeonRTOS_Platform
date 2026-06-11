/**
 ******************************************************************************
 * @file    LSM303AGR_MAG_Sensor.cpp
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Implementation an LSM303AGR magnetometer sensor.
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
 * Based on STMicroelectronics LSM303AGR driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "LSM303AGR_MAG.h"

#include "LSM303AGR_MAG_Register_Def.h"
#include "LSM303AGR_MAG_Register.h"

LSM303AGR_OpResult LSM303AGR_MAG_Init(void)
{
  LSM303AGR_OpResult op_status;
  
  /* Operating mode selection - power down */
  op_status = LSM303AGR_MAG_W_MD( LSM303AGR_MAG_MD_IDLE1_MODE );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Enable BDU */
  op_status = LSM303AGR_MAG_W_BDU( LSM303AGR_MAG_BDU_ENABLED );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  op_status = LSM303AGR_MAG_SetODR( 100.0f );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  op_status = LSM303AGR_MAG_SetFS( 50.0f );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }

  op_status = LSM303AGR_MAG_W_ST( LSM303AGR_MAG_ST_DISABLED );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_DeInit(void)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_Disable();
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_Enable(void)
{
  /* Operating mode selection */
  
  return LSM303AGR_MAG_W_MD( LSM303AGR_MAG_MD_CONTINUOS_MODE );
}

LSM303AGR_OpResult LSM303AGR_MAG_Disable(void)
{
  /* Operating mode selection - power down */
  
  return LSM303AGR_MAG_W_MD( LSM303AGR_MAG_MD_IDLE1_MODE );
}

LSM303AGR_OpResult LSM303AGR_MAG_ReadID(uint8_t *p_id)
{
  LSM303AGR_OpResult op_status;
  
  if(!p_id)
  { 
    return LSM303AGR_InvalidParameter; 
  }
 
  /* Read WHO AM I register */
  op_status = LSM303AGR_MAG_R_WHO_AM_I( p_id );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_GetAxes(int32_t *pData)
{
  LSM303AGR_OpResult op_status;
  
  int16_t pDataRaw[3];
  float sensitivity = 0;
  
  /* Read raw data from LSM303AGR output register. */
  op_status = LSM303AGR_MAG_GetAxesRaw( pDataRaw );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Get LSM303AGR actual sensitivity. */
  op_status = LSM303AGR_MAG_GetSensitivity( &sensitivity );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Calculate the data. */
  pData[0] = ( int32_t )( pDataRaw[0] * sensitivity );
  pData[1] = ( int32_t )( pDataRaw[1] * sensitivity );
  pData[2] = ( int32_t )( pDataRaw[2] * sensitivity );
  
  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_GetSensitivity(float *pfData)
{
  *pfData = 1.5f;
  
  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_GetAxesRaw(int16_t *pData)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};
  int16_t *regValueInt16;
  
  /* Read output registers from LSM303AGR_MAG_OUTX_L to LSM303AGR_MAG_OUTZ_H. */
  op_status = LSM303AGR_MAG_Get_Raw_Magnetic( regValue );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  regValueInt16 = (int16_t *)regValue;
  
  /* Format the data. */
  pData[0] = regValueInt16[0];
  pData[1] = regValueInt16[1];
  pData[2] = regValueInt16[2];
  
  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_GetODR(float* odr)
{
  LSM303AGR_OpResult op_status;
  
  LSM303AGR_MAG_ODR_t odr_low_level;
  
  op_status = LSM303AGR_MAG_R_ODR( &odr_low_level );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  switch( odr_low_level )
  {
    case LSM303AGR_MAG_ODR_10Hz:
      *odr = 10.000f;
      break;
    case LSM303AGR_MAG_ODR_20Hz:
      *odr = 20.000f;
      break;
    case LSM303AGR_MAG_ODR_50Hz:
      *odr = 50.000f;
      break;
    case LSM303AGR_MAG_ODR_100Hz:
      *odr = 100.000f;
      break;
    default:
      *odr = -1.000f;
      return LSM303AGR_InvalidParameter;
  }  
  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_SetODR(float odr)
{
  LSM303AGR_OpResult op_status;
  
  LSM303AGR_MAG_ODR_t new_odr;
  
  new_odr = ( odr <= 10.000f ) ? LSM303AGR_MAG_ODR_10Hz
          : ( odr <= 20.000f ) ? LSM303AGR_MAG_ODR_20Hz
          : ( odr <= 50.000f ) ? LSM303AGR_MAG_ODR_50Hz
          :                      LSM303AGR_MAG_ODR_100Hz;
            
  op_status = LSM303AGR_MAG_W_ODR( new_odr );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_GetFS(float* fullScale)
{
  *fullScale = 50.0f;
  
  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_SetFS(float fullScale)
{
  (void)(fullScale);

  return LSM303AGR_OK;
}