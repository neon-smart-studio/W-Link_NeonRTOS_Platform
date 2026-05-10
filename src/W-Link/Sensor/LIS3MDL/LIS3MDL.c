/**
 ******************************************************************************
 * @file    LIS3MDLSensor.cpp
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Implementation an LIS3MDL magnetometer sensor.
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
 * Based on STMicroelectronics LIS3MDL driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "LIS3MDL.h"

#include "LIS3MDL_MAG_Register_Def.h"
#include "LIS3MDL_MAG_Register.h"

LIS3MDL_OpStatus LIS3MDL_Init()
{
  LIS3MDL_OpStatus op_status;

  /* Operating mode selection - power down */
  op_status = LIS3MDL_MAG_W_SystemOperatingMode( LIS3MDL_MAG_MD_POWER_DOWN );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  /* Enable BDU */
  op_status = LIS3MDL_MAG_W_BlockDataUpdate( LIS3MDL_MAG_BDU_ENABLE );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  op_status = LIS3MDL_SetODR( 80.0f );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  op_status = LIS3MDL_SetFS( 4.0f );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  /* X and Y axes Operating mode selection */
  op_status = LIS3MDL_MAG_W_OperatingModeXY( LIS3MDL_MAG_OM_HIGH );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  /* Temperature sensor disable - temp. sensor not used */
  op_status = LIS3MDL_MAG_W_TemperatureSensor( LIS3MDL_MAG_TEMP_EN_DISABLE );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_DeInit()
{
  LIS3MDL_OpStatus op_status;

  /* Disable mag */
  op_status = LIS3MDL_Disable();
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_Enable(void)
{
  LIS3MDL_OpStatus op_status;

  /* Operating mode selection - continuous */
  op_status = LIS3MDL_MAG_W_SystemOperatingMode( LIS3MDL_MAG_MD_CONTINUOUS );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_Disable(void)
{
  LIS3MDL_OpStatus op_status;

  /* Operating mode selection - power down */
  op_status = LIS3MDL_MAG_W_SystemOperatingMode( LIS3MDL_MAG_MD_POWER_DOWN );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_ReadID(uint8_t *p_id)
{
  LIS3MDL_OpStatus op_status;

  if(!p_id)
  {
    return LIS3MDL_InvalidParameter;
  }

  /* Read WHO AM I register */
  op_status = LIS3MDL_MAG_R_WHO_AM_I_( p_id );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_GetAxes(int32_t *pData)
{
  LIS3MDL_OpStatus op_status;

  int16_t pDataRaw[3];
  float sensitivity = 0;

  /* Read raw data from LIS3MDL output register. */
  op_status = LIS3MDL_GetAxesRaw( pDataRaw );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  /* Get LIS3MDL actual sensitivity. */
  op_status = LIS3MDL_GetSensitivity( &sensitivity );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  /* Calculate the data. */
  pData[0] = ( int32_t )( pDataRaw[0] * sensitivity );
  pData[1] = ( int32_t )( pDataRaw[1] * sensitivity );
  pData[2] = ( int32_t )( pDataRaw[2] * sensitivity );

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_GetSensitivity(float *pfData)
{
  LIS3MDL_OpStatus op_status;

  LIS3MDL_MAG_FS_t fullScale;

  /* Read actual full scale selection from sensor. */
  op_status = LIS3MDL_MAG_R_FullScale( &fullScale );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  /* Store the sensitivity based on actual full scale. */
  switch( fullScale )
  {
    case LIS3MDL_MAG_FS_4Ga:
      *pfData = ( float )LIS3MDL_MAG_SENSITIVITY_FOR_FS_4G;
      break;
    case LIS3MDL_MAG_FS_8Ga:
      *pfData = ( float )LIS3MDL_MAG_SENSITIVITY_FOR_FS_8G;
      break;
    case LIS3MDL_MAG_FS_12Ga:
      *pfData = ( float )LIS3MDL_MAG_SENSITIVITY_FOR_FS_12G;
      break;
    case LIS3MDL_MAG_FS_16Ga:
      *pfData = ( float )LIS3MDL_MAG_SENSITIVITY_FOR_FS_16G;
      break;
    default:
      *pfData = -1.0f;
      break;
  }

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_GetAxesRaw(int16_t *pData)
{
  LIS3MDL_OpStatus op_status;

  uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};

  /* Read output registers from LIS3MDL_MAG_OUTX_L to LIS3MDL_MAG_OUTZ_H. */
  op_status = LIS3MDL_MAG_Get_Magnetic( ( uint8_t* )regValue );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  /* Format the data. */
  pData[0] = ( ( ( ( int16_t )regValue[1] ) << 8 ) + ( int16_t )regValue[0] );
  pData[1] = ( ( ( ( int16_t )regValue[3] ) << 8 ) + ( int16_t )regValue[2] );
  pData[2] = ( ( ( ( int16_t )regValue[5] ) << 8 ) + ( int16_t )regValue[4] );

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_GetODR(float* odr)
{
  LIS3MDL_OpStatus op_status;

  LIS3MDL_MAG_DO_t odr_low_level;

  op_status = LIS3MDL_MAG_R_OutputDataRate( &odr_low_level );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  switch( odr_low_level )
  {
    case LIS3MDL_MAG_DO_0_625Hz:
      *odr =  0.625f;
      break;
    case LIS3MDL_MAG_DO_1_25Hz:
      *odr =  1.250f;
      break;
    case LIS3MDL_MAG_DO_2_5Hz:
      *odr =  2.500f;
      break;
    case LIS3MDL_MAG_DO_5Hz:
      *odr =  5.000f;
      break;
    case LIS3MDL_MAG_DO_10Hz:
      *odr = 10.000f;
      break;
    case LIS3MDL_MAG_DO_20Hz:
      *odr = 20.000f;
      break;
    case LIS3MDL_MAG_DO_40Hz:
      *odr = 40.000f;
      break;
    case LIS3MDL_MAG_DO_80Hz:
      *odr = 80.000f;
      break;
    default:
      *odr = -1.000f;
      return LIS3MDL_InvalidParameter;
  }

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_SetODR(float odr)
{
  LIS3MDL_OpStatus op_status;

  LIS3MDL_MAG_DO_t new_odr;

  new_odr = ( odr <=  0.625f ) ? LIS3MDL_MAG_DO_0_625Hz
          : ( odr <=  1.250f ) ? LIS3MDL_MAG_DO_1_25Hz
          : ( odr <=  2.500f ) ? LIS3MDL_MAG_DO_2_5Hz
          : ( odr <=  5.000f ) ? LIS3MDL_MAG_DO_5Hz
          : ( odr <= 10.000f ) ? LIS3MDL_MAG_DO_10Hz
          : ( odr <= 20.000f ) ? LIS3MDL_MAG_DO_20Hz
          : ( odr <= 40.000f ) ? LIS3MDL_MAG_DO_40Hz
          :                      LIS3MDL_MAG_DO_80Hz;

  op_status = LIS3MDL_MAG_W_OutputDataRate( new_odr );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_GetFS(float* fullScale)
{
  LIS3MDL_OpStatus op_status;

  LIS3MDL_MAG_FS_t fs_low_level;

  op_status = LIS3MDL_MAG_R_FullScale( &fs_low_level );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  switch( fs_low_level )
  {
    case LIS3MDL_MAG_FS_4Ga:
      *fullScale =  4.0f;
      break;
    case LIS3MDL_MAG_FS_8Ga:
      *fullScale =  8.0f;
      break;
    case LIS3MDL_MAG_FS_12Ga:
      *fullScale = 12.0f;
      break;
    case LIS3MDL_MAG_FS_16Ga:
      *fullScale = 16.0f;
      break;
    default:
      *fullScale = -1.0f;
      return LIS3MDL_InvalidParameter;
  }

  return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_SetFS(float fullScale)
{
  LIS3MDL_OpStatus op_status;

  LIS3MDL_MAG_FS_t new_fs;

  new_fs = ( fullScale <=  4.0f ) ? LIS3MDL_MAG_FS_4Ga
         : ( fullScale <=  8.0f ) ? LIS3MDL_MAG_FS_8Ga
         : ( fullScale <= 12.0f ) ? LIS3MDL_MAG_FS_12Ga
         :                          LIS3MDL_MAG_FS_16Ga;

  op_status = LIS3MDL_MAG_W_FullScale( new_fs );
  if(op_status < LIS3MDL_OK)
  {
    return op_status;
  }

  return LIS3MDL_OK;
}