/**
 ******************************************************************************
 * @file    LSM303AGR_ACC_Sensor.cpp
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Implementation an LSM303AGR accelerometer sensor.
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

#include "LSM303AGR_ACC.h"

#include "LSM303AGR_ACC_Register_Def.h"
#include "LSM303AGR_ACC_Register.h"

LSM303AGR_OpStatus LSM303AGR_ACC_SetODR_When_Enabled(float odr);
LSM303AGR_OpStatus LSM303AGR_ACC_SetODR_When_Disabled(float odr);
LSM303AGR_OpStatus LSM303AGR_ACC_GetSensitivity_Normal_Mode( float *sensitivity );
LSM303AGR_OpStatus LSM303AGR_ACC_GetSensitivity_LP_Mode( float *sensitivity );
LSM303AGR_OpStatus LSM303AGR_ACC_GetSensitivity_HR_Mode( float *sensitivity );

static uint8_t isEnabled;
static float Last_ODR;

LSM303AGR_OpStatus LSM303AGR_ACC_Init(void)
{
  LSM303AGR_OpStatus op_status;
  
  /* Enable BDU */
  op_status = LSM303AGR_ACC_W_BlockDataUpdate( LSM303AGR_ACC_BDU_ENABLED );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* FIFO mode selection */
  op_status = LSM303AGR_ACC_W_FifoMode( LSM303AGR_ACC_FM_BYPASS );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Output data rate selection - power down. */
  op_status = LSM303AGR_ACC_W_ODR( LSM303AGR_ACC_ODR_DO_PWR_DOWN );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Full scale selection. */
  op_status = LSM303AGR_ACC_SetFS( 2.0f );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Enable axes. */
  op_status = LSM303AGR_ACC_W_XEN( LSM303AGR_ACC_XEN_ENABLED );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  op_status = LSM303AGR_ACC_W_YEN ( LSM303AGR_ACC_YEN_ENABLED );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  op_status = LSM303AGR_ACC_W_ZEN ( LSM303AGR_ACC_ZEN_ENABLED );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Select default output data rate. */
  Last_ODR = 100.0f;
  
  isEnabled = 0;

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_end(void)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_Disable();
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_Enable(void)
{
  LSM303AGR_OpStatus op_status;
   
  /* Check if the component is already enabled */
  if ( isEnabled == 1 )
  {
    return LSM303AGR_OK;
  }
  
  /* Output data rate selection. */
  op_status = LSM303AGR_ACC_SetODR_When_Enabled( Last_ODR );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  isEnabled = 1;
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_Disable(void)
{
  LSM303AGR_OpStatus op_status;
   
  /* Check if the component is already disabled */
  if ( isEnabled == 0 )
  {
    return LSM303AGR_OK;
  }
  
  /* Store actual output data rate. */
  op_status = LSM303AGR_ACC_GetODR( &Last_ODR );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Output data rate selection - power down. */
  op_status = LSM303AGR_ACC_W_ODR( LSM303AGR_ACC_ODR_DO_PWR_DOWN );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  isEnabled = 0;
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_ReadID(uint8_t *p_id)
{
  LSM303AGR_OpStatus op_status;
  
  if(!p_id)
  { 
    return LSM303AGR_InvalidParameter; 
  }
 
  /* Read WHO AM I register */
  op_status = LSM303AGR_ACC_R_WHO_AM_I( p_id );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_GetAxes(int32_t *pData)
{
  LSM303AGR_OpStatus op_status;
  
  int data[3];
  
  /* Read data from LSM303AGR. */
  op_status = LSM303AGR_ACC_Get_Acceleration(data);
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Calculate the data. */
  pData[0] = (int32_t)data[0];
  pData[1] = (int32_t)data[1];
  pData[2] = (int32_t)data[2];
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_GetSensitivity(float *pfData)
{
  LSM303AGR_OpStatus op_status;
  
  LSM303AGR_ACC_LPEN_t lp_value;
  LSM303AGR_ACC_HR_t hr_value;
  
  /* Read low power flag */
  op_status = LSM303AGR_ACC_R_LOWPWR_EN( &lp_value );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Read high performance flag */
  op_status = LSM303AGR_ACC_R_HiRes( &hr_value );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  if( lp_value == LSM303AGR_ACC_LPEN_DISABLED && hr_value == LSM303AGR_ACC_HR_DISABLED )
  {
    /* Normal Mode */
    return LSM303AGR_ACC_GetSensitivity_Normal_Mode( pfData );
  } else if ( lp_value == LSM303AGR_ACC_LPEN_ENABLED && hr_value == LSM303AGR_ACC_HR_DISABLED )
  {
    /* Low Power Mode */
    return LSM303AGR_ACC_GetSensitivity_LP_Mode( pfData );
  } else if ( lp_value == LSM303AGR_ACC_LPEN_DISABLED && hr_value == LSM303AGR_ACC_HR_ENABLED )
  {
    /* High Resolution Mode */
    return LSM303AGR_ACC_GetSensitivity_HR_Mode( pfData );
  } else
  {
    /* Not allowed */
    return LSM303AGR_InvalidParameter;
  }
}

LSM303AGR_OpStatus LSM303AGR_ACC_GetSensitivity_Normal_Mode( float *sensitivity )
{
  LSM303AGR_OpStatus op_status;
  
  LSM303AGR_ACC_FS_t fullScale;
  
  /* Read actual full scale selection from sensor. */
  op_status = LSM303AGR_ACC_R_FullScale( &fullScale );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Store the sensitivity based on actual full scale. */
  switch( fullScale )
  {
    case LSM303AGR_ACC_FS_2G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_2G_NORMAL_MODE;
      break;
    case LSM303AGR_ACC_FS_4G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_4G_NORMAL_MODE;
      break;
    case LSM303AGR_ACC_FS_8G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_8G_NORMAL_MODE;
      break;
    case LSM303AGR_ACC_FS_16G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_16G_NORMAL_MODE;
      break;
    default:
      *sensitivity = -1.0f;
      return LSM303AGR_InvalidParameter;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_GetSensitivity_LP_Mode( float *sensitivity )
{
  LSM303AGR_OpStatus op_status;
  
  LSM303AGR_ACC_FS_t fullScale;
  
  /* Read actual full scale selection from sensor. */
  op_status = LSM303AGR_ACC_R_FullScale( &fullScale );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Store the sensitivity based on actual full scale. */
  switch( fullScale )
  {
    case LSM303AGR_ACC_FS_2G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_2G_LOW_POWER_MODE;
      break;
    case LSM303AGR_ACC_FS_4G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_4G_LOW_POWER_MODE;
      break;
    case LSM303AGR_ACC_FS_8G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_8G_LOW_POWER_MODE;
      break;
    case LSM303AGR_ACC_FS_16G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_16G_LOW_POWER_MODE;
      break;
    default:
      *sensitivity = -1.0f;
      return LSM303AGR_InvalidParameter;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_GetSensitivity_HR_Mode( float *sensitivity )
{
  LSM303AGR_OpStatus op_status;
  
  LSM303AGR_ACC_FS_t fullScale;
  
  /* Read actual full scale selection from sensor. */
  op_status = LSM303AGR_ACC_R_FullScale( &fullScale );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Store the sensitivity based on actual full scale. */
  switch( fullScale )
  {
    case LSM303AGR_ACC_FS_2G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_2G_HIGH_RESOLUTION_MODE;
      break;
    case LSM303AGR_ACC_FS_4G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_4G_HIGH_RESOLUTION_MODE;
      break;
    case LSM303AGR_ACC_FS_8G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_8G_HIGH_RESOLUTION_MODE;
      break;
    case LSM303AGR_ACC_FS_16G:
      *sensitivity = ( float )LSM303AGR_ACC_SENSITIVITY_FOR_FS_16G_HIGH_RESOLUTION_MODE;
      break;
    default:
      *sensitivity = -1.0f;
      return LSM303AGR_InvalidParameter;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_GetAxesRaw(int16_t *pData)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};
  uint8_t shift = 0;
  LSM303AGR_ACC_LPEN_t lp;
  LSM303AGR_ACC_HR_t hr;
  
  /* Determine which operational mode the acc is set */
  op_status = LSM303AGR_ACC_R_HiRes( &hr );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }

  op_status = LSM303AGR_ACC_R_LOWPWR_EN( &lp );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  if (lp == LSM303AGR_ACC_LPEN_ENABLED && hr == LSM303AGR_ACC_HR_DISABLED) {
    /* op mode is LP 8-bit */
    shift = 8;
  } else if (lp == LSM303AGR_ACC_LPEN_DISABLED && hr == LSM303AGR_ACC_HR_DISABLED) {
    /* op mode is Normal 10-bit */
    shift = 6;
  } else if (lp == LSM303AGR_ACC_LPEN_DISABLED && hr == LSM303AGR_ACC_HR_ENABLED) {
    /* op mode is HR 12-bit */
    shift = 4;
  } else {
    return LSM303AGR_InvalidParameter;
  }
  
  /* Read output registers from LSM303AGR_ACC_GYRO_OUTX_L_XL to LSM303AGR_ACC_GYRO_OUTZ_H_XL. */
  op_status = LSM303AGR_ACC_Get_Raw_Acceleration( ( uint8_t* )regValue );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  /* Format the data. */
  pData[0] = ( (int16_t)( ( ( ( int16_t )regValue[1] ) << 8 ) + ( int16_t )regValue[0] ) >> shift );
  pData[1] = ( (int16_t)( ( ( ( int16_t )regValue[3] ) << 8 ) + ( int16_t )regValue[2] ) >> shift );
  pData[2] = ( (int16_t)( ( ( ( int16_t )regValue[5] ) << 8 ) + ( int16_t )regValue[4] ) >> shift );
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_GetODR(float* odr)
{
  LSM303AGR_OpStatus op_status;
  
  LSM303AGR_ACC_ODR_t odr_low_level;
  
  op_status = LSM303AGR_ACC_R_ODR( &odr_low_level );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  switch( odr_low_level )
  {
    case LSM303AGR_ACC_ODR_DO_PWR_DOWN:
      *odr = 0.0f;
      break;
    case LSM303AGR_ACC_ODR_DO_1Hz:
      *odr = 1.0f;
      break;
    case LSM303AGR_ACC_ODR_DO_10Hz:
      *odr = 10.0f;
      break;
    case LSM303AGR_ACC_ODR_DO_25Hz:
      *odr = 25.0f;
      break;
    case LSM303AGR_ACC_ODR_DO_50Hz:
      *odr = 50.0f;
      break;
    case LSM303AGR_ACC_ODR_DO_100Hz:
      *odr = 100.0f;
      break;
    case LSM303AGR_ACC_ODR_DO_200Hz:
      *odr = 200.0f;
      break;
    case LSM303AGR_ACC_ODR_DO_400Hz:
      *odr = 400.0f;
      break;
    default:
      *odr = -1.0f;
      return LSM303AGR_InvalidParameter;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_SetODR(float odr)
{
  LSM303AGR_OpStatus op_status;
  
  if(isEnabled == 1)
  {
    op_status = LSM303AGR_ACC_SetODR_When_Enabled(odr);
  }
  else
  {
    op_status = LSM303AGR_ACC_SetODR_When_Disabled(odr);
  }
  
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_SetODR_When_Enabled(float odr)
{
  LSM303AGR_OpStatus op_status;
  
  LSM303AGR_ACC_ODR_t new_odr;
  
  new_odr = ( odr <=    1.0f ) ? LSM303AGR_ACC_ODR_DO_1Hz
          : ( odr <=   10.0f ) ? LSM303AGR_ACC_ODR_DO_10Hz
          : ( odr <=   25.0f ) ? LSM303AGR_ACC_ODR_DO_25Hz
          : ( odr <=   50.0f ) ? LSM303AGR_ACC_ODR_DO_50Hz
          : ( odr <=  100.0f ) ? LSM303AGR_ACC_ODR_DO_100Hz
          : ( odr <=  200.0f ) ? LSM303AGR_ACC_ODR_DO_200Hz
          :                      LSM303AGR_ACC_ODR_DO_400Hz;
            
  op_status = LSM303AGR_ACC_W_ODR( new_odr );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_SetODR_When_Disabled(float odr)
{
  LSM303AGR_OpStatus op_status;
   
  Last_ODR = ( odr <=    1.0f ) ?  1.0f
           : ( odr <=   10.0f ) ? 10.0f
           : ( odr <=   25.0f ) ? 25.0f
           : ( odr <=   50.0f ) ? 50.0f
           : ( odr <=  100.0f ) ? 100.0f
           : ( odr <=  200.0f ) ? 200.0f
           :                      400.0f;
                                 
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_GetFS(float* fullScale)
{
  LSM303AGR_OpStatus op_status;
  
  LSM303AGR_ACC_FS_t fs_low_level;
  
  op_status = LSM303AGR_ACC_R_FullScale( &fs_low_level );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  switch( fs_low_level )
  {
    case LSM303AGR_ACC_FS_2G:
      *fullScale =  2.0f;
      break;
    case LSM303AGR_ACC_FS_4G:
      *fullScale =  4.0f;
      break;
    case LSM303AGR_ACC_FS_8G:
      *fullScale =  8.0f;
      break;
    case LSM303AGR_ACC_FS_16G:
      *fullScale = 16.0f;
      break;
    default:
      *fullScale = -1.0f;
      return LSM303AGR_InvalidParameter;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_SetFS(float fullScale)
{
  LSM303AGR_OpStatus op_status;
  
  LSM303AGR_ACC_FS_t new_fs;
  
  new_fs = ( fullScale <= 2.0f ) ? LSM303AGR_ACC_FS_2G
         : ( fullScale <= 4.0f ) ? LSM303AGR_ACC_FS_4G
         : ( fullScale <= 8.0f ) ? LSM303AGR_ACC_FS_8G
         :                         LSM303AGR_ACC_FS_16G;
           
  op_status = LSM303AGR_ACC_W_FullScale( new_fs );
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }
  
  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_EnableSelfTest(bool self_test)
{
  // 4.2.4 Accelerometer self - test
  // The self-test allows the user to check the sensor functionality without moving it.When the
  // self-test is enabled, an actuation force is applied to the sensor, simulating a definite input
  // acceleration.In this case the sensor outputs will exhibit a change in their DC levels which
  // are related to the selected full scale through the device sensitivity.When the self-test is
  // activated, the device output level is given by the algebraic sum of the signals produced by
  // the acceleration acting on the sensor and by the electrostatic test-force.

  return LSM303AGR_ACC_W_SelfTest(self_test == 0 ? LSM303AGR_ACC_ST_SELF_TEST_0 : LSM303AGR_ACC_ST_SELF_TEST_1);   
}

LSM303AGR_OpStatus LSM303AGR_ACC_DisableSelfTest(void)
{
  return LSM303AGR_ACC_W_SelfTest(LSM303AGR_ACC_ST_DISABLED);
}

LSM303AGR_OpStatus LSM303AGR_ACC_EnableTemperatureSensor(void)
{
  return LSM303AGR_ACC_W_TEMP_EN_bits(LSM303AGR_ACC_TEMP_EN_ENABLED);
}

LSM303AGR_OpStatus LSM303AGR_ACC_DisableTemperatureSensor(void)
{
  return LSM303AGR_ACC_W_TEMP_EN_bits(LSM303AGR_ACC_TEMP_EN_DISABLED);
}

LSM303AGR_OpStatus LSM303AGR_ACC_GetTemperature(float *temperature)
{
  LSM303AGR_OpStatus op_status;
  
  uint16_t temp;
  uint8_t temp_low;
  LSM303AGR_ACC_3DA__t value;

  do
  {
    op_status = LSM303AGR_ACC_R_z_data_avail(&value);
    if(op_status < LSM303AGR_OK)
    {
      return op_status;
    }
  } while(value != LSM303AGR_ACC_3DA__AVAILABLE);

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_OUT_ADC3_H, (uint8_t*)&temp);
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_OUT_ADC3_L, &temp_low);
  if(op_status < LSM303AGR_OK)
  {
    return op_status;
  }

  temp  = (temp << 8) + temp_low;

  *temperature = (((int16_t)temp / 256.0f) + 25.0f);

  return LSM303AGR_OK;
}
