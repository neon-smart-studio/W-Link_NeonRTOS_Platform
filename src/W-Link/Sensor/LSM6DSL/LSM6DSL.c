/**
 ******************************************************************************
 * @file    LSM6DSLSensor.cpp
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Implementation of an LSM6DSL Inertial Measurement Unit (IMU) 6 axes
 *          sensor.
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
 * Based on STMicroelectronics LSM6DSL driver
 * Modified by Neon Smart Studio for W-Link
 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "LSM6DSL.h"

#include "LSM6DSL_Register_Def.h"
#include "LSM6DSL_Register.h"

static uint8_t X_isEnabled;
static float X_Last_ODR;
static uint8_t G_isEnabled;
static float G_Last_ODR;

LSM6DSL_OpStatus LSM6DSL_Set_X_ODR_When_Enabled(float odr);
LSM6DSL_OpStatus LSM6DSL_Set_X_ODR_When_Disabled(float odr);
LSM6DSL_OpStatus LSM6DSL_Set_G_ODR_When_Enabled(float odr);
LSM6DSL_OpStatus LSM6DSL_Set_G_ODR_When_Disabled(float odr);

LSM6DSL_OpStatus LSM6DSL_Init()
{
  LSM6DSL_OpStatus op_status;

  /* Enable register address automatically incremented during a multiple byte
     access with a serial interface. */
  op_status = LSM6DSL_ACC_GYRO_W_IF_Addr_Incr(LSM6DSL_ACC_GYRO_IF_INC_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable BDU */
  op_status = LSM6DSL_ACC_GYRO_W_BDU(LSM6DSL_ACC_GYRO_BDU_BLOCK_UPDATE);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* FIFO mode selection */
  op_status = LSM6DSL_ACC_GYRO_W_FIFO_MODE(LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Output data rate selection - power down. */
  op_status = LSM6DSL_ACC_GYRO_W_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSL_Set_X_FS(2.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Output data rate selection - power down */
  op_status = LSM6DSL_ACC_GYRO_W_ODR_G(LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSL_Set_G_FS(2000.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  X_Last_ODR = 104.0f;

  X_isEnabled = 0;

  G_Last_ODR = 104.0f;

  G_isEnabled = 0;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_DeInit()
{
  LSM6DSL_OpStatus op_status;

  /* Disable both acc and gyro */
  op_status = LSM6DSL_Disable_X();
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  op_status = LSM6DSL_Disable_G();
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Enable_X(void)
{
  LSM6DSL_OpStatus op_status;

  /* Check if the component is already enabled */
  if (X_isEnabled == 1)
  {
    return LSM6DSL_OK;
  }

  /* Output data rate selection. */
  op_status = LSM6DSL_Set_X_ODR_When_Enabled(X_Last_ODR);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  X_isEnabled = 1;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Enable_G(void)
{
  LSM6DSL_OpStatus op_status;

  /* Check if the component is already enabled */
  if (G_isEnabled == 1)
  {
    return LSM6DSL_OK;
  }

  /* Output data rate selection. */
  op_status = LSM6DSL_Set_G_ODR_When_Enabled(G_Last_ODR);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  G_isEnabled = 1;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Disable_X(void)
{
  LSM6DSL_OpStatus op_status;

  /* Check if the component is already disabled */
  if (X_isEnabled == 0)
  {
    return LSM6DSL_OK;
  }

  /* Store actual output data rate. */
  op_status = LSM6DSL_Get_X_ODR(&X_Last_ODR);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Output data rate selection - power down. */
  op_status = LSM6DSL_ACC_GYRO_W_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  X_isEnabled = 0;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Disable_G(void)
{
  LSM6DSL_OpStatus op_status;

  /* Check if the component is already disabled */
  if (G_isEnabled == 0)
  {
    return LSM6DSL_OK;
  }

  /* Store actual output data rate. */
  op_status = LSM6DSL_Get_G_ODR(&G_Last_ODR);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Output data rate selection - power down */
  op_status = LSM6DSL_ACC_GYRO_W_ODR_G(LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  G_isEnabled = 0;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ReadID(uint8_t *p_id)
{
  LSM6DSL_OpStatus op_status;

  if(!p_id)
  {
    return LSM6DSL_InvalidParameter;
  }

  /* Read WHO AM I register */
  op_status = LSM6DSL_ACC_GYRO_R_WHO_AM_I(p_id);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_X_Axes(int32_t *pData)
{
  LSM6DSL_OpStatus op_status;

  int16_t dataRaw[3];
  float sensitivity = 0;

  /* Read raw data from LSM6DSL output register. */
  op_status = LSM6DSL_Get_X_AxesRaw(dataRaw);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Get LSM6DSL actual sensitivity. */
  op_status = LSM6DSL_Get_X_Sensitivity(&sensitivity);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Calculate the data. */
  pData[0] = (int32_t)(dataRaw[0] * sensitivity);
  pData[1] = (int32_t)(dataRaw[1] * sensitivity);
  pData[2] = (int32_t)(dataRaw[2] * sensitivity);

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_G_Axes(int32_t *pData)
{
  LSM6DSL_OpStatus op_status;

  int16_t dataRaw[3];
  float sensitivity = 0;

  /* Read raw data from LSM6DSL output register. */
  op_status = LSM6DSL_Get_G_AxesRaw(dataRaw);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Get LSM6DSL actual sensitivity. */
  op_status = LSM6DSL_Get_G_Sensitivity(&sensitivity);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Calculate the data. */
  pData[0] = (int32_t)(dataRaw[0] * sensitivity);
  pData[1] = (int32_t)(dataRaw[1] * sensitivity);
  pData[2] = (int32_t)(dataRaw[2] * sensitivity);

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_X_Sensitivity(float *pfData)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_FS_XL_t fullScale;

  /* Read actual full scale selection from sensor. */
  op_status = LSM6DSL_ACC_GYRO_R_FS_XL(&fullScale);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Store the sensitivity based on actual full scale. */
  switch(fullScale)
  {
    case LSM6DSL_ACC_GYRO_FS_XL_2g:
      *pfData = (float)LSM6DSL_ACC_SENSITIVITY_FOR_FS_2G;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_4g:
      *pfData = (float)LSM6DSL_ACC_SENSITIVITY_FOR_FS_4G;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_8g:
      *pfData = (float)LSM6DSL_ACC_SENSITIVITY_FOR_FS_8G;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_16g:
      *pfData = (float)LSM6DSL_ACC_SENSITIVITY_FOR_FS_16G;
      break;
    default:
      *pfData = -1.0f;
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_G_Sensitivity(float *pfData)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_FS_125_t fullScale125;
  LSM6DSL_ACC_GYRO_FS_G_t   fullScale;

  /* Read full scale 125 selection from sensor. */
  op_status = LSM6DSL_ACC_GYRO_R_FS_125(&fullScale125);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  if (fullScale125 == LSM6DSL_ACC_GYRO_FS_125_ENABLED)
  {
    *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_125DPS;
  }

  else
  {
    /* Read actual full scale selection from sensor. */
    op_status = LSM6DSL_ACC_GYRO_R_FS_G(&fullScale);
    if(op_status < LSM6DSL_OK)
    {
      return op_status;
    }

    /* Store the sensitivity based on actual full scale. */
    switch(fullScale)
    {
      case LSM6DSL_ACC_GYRO_FS_G_245dps:
        *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_245DPS;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_500dps:
        *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_500DPS;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_1000dps:
        *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_1000DPS;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_2000dps:
        *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_2000DPS;
        break;
      default:
        *pfData = -1.0f;
        return LSM6DSL_InvalidParameter;
    }
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_X_AxesRaw(int16_t *pData)
{
  LSM6DSL_OpStatus op_status;

  uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};

  /* Read output registers from LSM6DSL_ACC_GYRO_OUTX_L_XL to LSM6DSL_ACC_GYRO_OUTZ_H_XL. */
  op_status = LSM6DSL_ACC_GYRO_GetRawAccData(regValue);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Format the data. */
  pData[0] = ((((int16_t)regValue[1]) << 8) + (int16_t)regValue[0]);
  pData[1] = ((((int16_t)regValue[3]) << 8) + (int16_t)regValue[2]);
  pData[2] = ((((int16_t)regValue[5]) << 8) + (int16_t)regValue[4]);

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_G_AxesRaw(int16_t *pData)
{
  LSM6DSL_OpStatus op_status;

  uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};

  /* Read output registers from LSM6DSL_ACC_GYRO_OUTX_L_G to LSM6DSL_ACC_GYRO_OUTZ_H_G. */
  op_status = LSM6DSL_ACC_GYRO_GetRawGyroData(regValue);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Format the data. */
  pData[0] = ((((int16_t)regValue[1]) << 8) + (int16_t)regValue[0]);
  pData[1] = ((((int16_t)regValue[3]) << 8) + (int16_t)regValue[2]);
  pData[2] = ((((int16_t)regValue[5]) << 8) + (int16_t)regValue[4]);

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_X_ODR(float* odr)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_ODR_XL_t odr_low_level;

  op_status = LSM6DSL_ACC_GYRO_R_ODR_XL(&odr_low_level);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  switch(odr_low_level)
  {
    case LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN:
      *odr = 0.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_13Hz:
      *odr = 13.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_26Hz:
      *odr = 26.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_52Hz:
      *odr = 52.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_104Hz:
      *odr = 104.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_208Hz:
      *odr = 208.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_416Hz:
      *odr = 416.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_833Hz:
      *odr = 833.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_1660Hz:
      *odr = 1660.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_3330Hz:
      *odr = 3330.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_6660Hz:
      *odr = 6660.0f;
      break;
    default:
      *odr = -1.0f;
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_G_ODR(float* odr)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_ODR_G_t odr_low_level;

  op_status = LSM6DSL_ACC_GYRO_R_ODR_G(&odr_low_level);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  switch(odr_low_level)
  {
    case LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN:
      *odr = 0.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_13Hz:
      *odr = 13.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_26Hz:
      *odr = 26.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_52Hz:
      *odr = 52.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_104Hz:
      *odr = 104.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_208Hz:
      *odr = 208.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_416Hz:
      *odr = 416.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_833Hz:
      *odr = 833.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_1660Hz:
      *odr = 1660.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_3330Hz:
      *odr = 3330.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_6660Hz:
      *odr = 6660.0f;
      break;
    default:
      *odr = -1.0f;
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_X_ODR(float odr)
{
  LSM6DSL_OpStatus op_status;

  if(X_isEnabled == 1)
  {
    op_status = LSM6DSL_Set_X_ODR_When_Enabled(odr);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
  }
  else
  {
    op_status = LSM6DSL_Set_X_ODR_When_Disabled(odr);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_X_ODR_When_Enabled(float odr)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_ODR_XL_t new_odr;

  new_odr = (odr <=   13.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_13Hz
          : (odr <=   26.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_26Hz
          : (odr <=   52.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_52Hz
          : (odr <=  104.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_104Hz
          : (odr <=  208.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_208Hz
          : (odr <=  416.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_416Hz
          : (odr <=  833.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_833Hz
          : (odr <= 1660.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_1660Hz
          : (odr <= 3330.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_3330Hz
          :                      LSM6DSL_ACC_GYRO_ODR_XL_6660Hz;

  op_status = LSM6DSL_ACC_GYRO_W_ODR_XL(new_odr);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_X_ODR_When_Disabled(float odr)
{
  LSM6DSL_OpStatus op_status;

  X_Last_ODR = (odr <=   13.0f) ? 13.0f
             : (odr <=   26.0f) ? 26.0f
             : (odr <=   52.0f) ? 52.0f
             : (odr <=  104.0f) ? 104.0f
             : (odr <=  208.0f) ? 208.0f
             : (odr <=  416.0f) ? 416.0f
             : (odr <=  833.0f) ? 833.0f
             : (odr <= 1660.0f) ? 1660.0f
             : (odr <= 3330.0f) ? 3330.0f
             :                      6660.0f;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_G_ODR(float odr)
{
  LSM6DSL_OpStatus op_status;

  if(G_isEnabled == 1)
  {
    op_status = LSM6DSL_Set_G_ODR_When_Enabled(odr);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
  }
  else
  {
    op_status = LSM6DSL_Set_G_ODR_When_Disabled(odr);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_G_ODR_When_Enabled(float odr)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_ODR_G_t new_odr;

  new_odr = (odr <=  13.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_13Hz
          : (odr <=  26.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_26Hz
          : (odr <=  52.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_52Hz
          : (odr <= 104.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_104Hz
          : (odr <= 208.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_208Hz
          : (odr <= 416.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_416Hz
          : (odr <= 833.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_833Hz
          : (odr <= 1660.0f) ? LSM6DSL_ACC_GYRO_ODR_G_1660Hz
          : (odr <= 3330.0f) ? LSM6DSL_ACC_GYRO_ODR_G_3330Hz
          :                      LSM6DSL_ACC_GYRO_ODR_G_6660Hz;

  op_status = LSM6DSL_ACC_GYRO_W_ODR_G(new_odr);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_G_ODR_When_Disabled(float odr)
{
  LSM6DSL_OpStatus op_status;

  G_Last_ODR = (odr <=  13.0f)  ? 13.0f
             : (odr <=  26.0f)  ? 26.0f
             : (odr <=  52.0f)  ? 52.0f
             : (odr <= 104.0f)  ? 104.0f
             : (odr <= 208.0f)  ? 208.0f
             : (odr <= 416.0f)  ? 416.0f
             : (odr <= 833.0f)  ? 833.0f
             : (odr <= 1660.0f) ? 1660.0f
             : (odr <= 3330.0f) ? 3330.0f
             :                      6660.0f;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_X_FS(float* fullScale)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_FS_XL_t fs_low_level;

  op_status = LSM6DSL_ACC_GYRO_R_FS_XL(&fs_low_level);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  switch(fs_low_level)
  {
    case LSM6DSL_ACC_GYRO_FS_XL_2g:
      *fullScale = 2.0f;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_4g:
      *fullScale = 4.0f;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_8g:
      *fullScale = 8.0f;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_16g:
      *fullScale = 16.0f;
      break;
    default:
      *fullScale = -1.0f;
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_G_FS(float* fullScale)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_FS_G_t fs_low_level;
  LSM6DSL_ACC_GYRO_FS_125_t fs_125;

  op_status = LSM6DSL_ACC_GYRO_R_FS_125(&fs_125);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }
  op_status = LSM6DSL_ACC_GYRO_R_FS_G(&fs_low_level);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  if (fs_125 == LSM6DSL_ACC_GYRO_FS_125_ENABLED)
  {
    *fullScale = 125.0f;
  }

  else
  {
    switch(fs_low_level)
    {
      case LSM6DSL_ACC_GYRO_FS_G_245dps:
        *fullScale = 245.0f;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_500dps:
        *fullScale = 500.0f;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_1000dps:
        *fullScale = 1000.0f;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_2000dps:
        *fullScale = 2000.0f;
        break;
      default:
        *fullScale = -1.0f;
        return LSM6DSL_InvalidParameter;
    }
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_X_FS(float fullScale)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_FS_XL_t new_fs;

  new_fs = (fullScale <= 2.0f) ? LSM6DSL_ACC_GYRO_FS_XL_2g
         : (fullScale <= 4.0f) ? LSM6DSL_ACC_GYRO_FS_XL_4g
         : (fullScale <= 8.0f) ? LSM6DSL_ACC_GYRO_FS_XL_8g
         :                         LSM6DSL_ACC_GYRO_FS_XL_16g;

  op_status = LSM6DSL_ACC_GYRO_W_FS_XL(new_fs);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_G_FS(float fullScale)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_FS_G_t new_fs;

  if (fullScale <= 125.0f)
  {
    op_status = LSM6DSL_ACC_GYRO_W_FS_125(LSM6DSL_ACC_GYRO_FS_125_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
  }
  else
  {
    new_fs = (fullScale <=  245.0f) ? LSM6DSL_ACC_GYRO_FS_G_245dps
           : (fullScale <=  500.0f) ? LSM6DSL_ACC_GYRO_FS_G_500dps
           : (fullScale <= 1000.0f) ? LSM6DSL_ACC_GYRO_FS_G_1000dps
           :                            LSM6DSL_ACC_GYRO_FS_G_2000dps;

    op_status = LSM6DSL_ACC_GYRO_W_FS_125(LSM6DSL_ACC_GYRO_FS_125_DISABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    op_status = LSM6DSL_ACC_GYRO_W_FS_G(new_fs);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Enable_Free_Fall_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
  LSM6DSL_OpStatus op_status;

  /* Output Data Rate selection */
  op_status = LSM6DSL_Set_X_ODR(416.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Full scale selection */
  op_status = LSM6DSL_ACC_GYRO_W_FS_XL(LSM6DSL_ACC_GYRO_FS_XL_2g);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* FF_DUR setting */
  op_status = LSM6DSL_ACC_GYRO_W_FF_Duration(0x06);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* WAKE_DUR setting */
  op_status = LSM6DSL_ACC_GYRO_W_WAKE_DUR(0x00);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* TIMER_HR setting */
  op_status = LSM6DSL_ACC_GYRO_W_TIMER_HR(LSM6DSL_ACC_GYRO_TIMER_HR_6_4ms);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* SLEEP_DUR setting */
  op_status = LSM6DSL_ACC_GYRO_W_SLEEP_DUR(0x00);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* FF_THS setting */
  op_status = LSM6DSL_ACC_GYRO_W_FF_THS(LSM6DSL_ACC_GYRO_FF_THS_312mg);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable free fall event on either INT1 or INT2 pin */
  switch (int_pin)
  {
  case LSM6DSL_INT1_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_FFEvOnInt1(LSM6DSL_ACC_GYRO_INT1_FF_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  case LSM6DSL_INT2_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_FFEvOnInt2(LSM6DSL_ACC_GYRO_INT2_FF_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Disable_Free_Fall_Detection(void)
{
  LSM6DSL_OpStatus op_status;

  /* Disable free fall event on INT1 pin */
  op_status = LSM6DSL_ACC_GYRO_W_FFEvOnInt1(LSM6DSL_ACC_GYRO_INT1_FF_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable free fall event on INT2 pin */
  op_status = LSM6DSL_ACC_GYRO_W_FFEvOnInt2(LSM6DSL_ACC_GYRO_INT2_FF_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* FF_DUR setting */
  op_status = LSM6DSL_ACC_GYRO_W_FF_Duration(0x00);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* FF_THS setting */
  op_status = LSM6DSL_ACC_GYRO_W_FF_THS(LSM6DSL_ACC_GYRO_FF_THS_156mg);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_Free_Fall_Threshold(uint8_t thr)
{
  LSM6DSL_OpStatus op_status;


  op_status = LSM6DSL_ACC_GYRO_W_FF_THS((LSM6DSL_ACC_GYRO_FF_THS_t)thr);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Enable_Pedometer(void)
{
  LSM6DSL_OpStatus op_status;

  /* Output Data Rate selection */
  op_status = LSM6DSL_Set_X_ODR(26.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSL_Set_X_FS(2.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set pedometer threshold. */
  op_status = LSM6DSL_Set_Pedometer_Threshold(LSM6DSL_PEDOMETER_THRESHOLD_MID_HIGH);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable embedded functionalities. */
  op_status = LSM6DSL_ACC_GYRO_W_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable pedometer algorithm. */
  op_status = LSM6DSL_ACC_GYRO_W_PEDO(LSM6DSL_ACC_GYRO_PEDO_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable pedometer on INT1. */
  op_status = LSM6DSL_ACC_GYRO_W_STEP_DET_on_INT1(LSM6DSL_ACC_GYRO_INT1_PEDO_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Disable_Pedometer(void)
{
  LSM6DSL_OpStatus op_status;

  /* Disable pedometer on INT1. */
  op_status = LSM6DSL_ACC_GYRO_W_STEP_DET_on_INT1(LSM6DSL_ACC_GYRO_INT1_PEDO_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable pedometer algorithm. */
  op_status = LSM6DSL_ACC_GYRO_W_PEDO(LSM6DSL_ACC_GYRO_PEDO_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable embedded functionalities. */
  op_status = LSM6DSL_ACC_GYRO_W_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Reset pedometer threshold. */
  op_status = LSM6DSL_Set_Pedometer_Threshold(0x0);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_Step_Counter(uint16_t *step_count)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_Get_GetStepCounter((uint8_t*)step_count);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Reset_Step_Counter(void)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_W_PedoStepReset(LSM6DSL_ACC_GYRO_PEDO_RST_STEP_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  NeonRTOS_Sleep(10);

  op_status = LSM6DSL_ACC_GYRO_W_PedoStepReset(LSM6DSL_ACC_GYRO_PEDO_RST_STEP_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_Pedometer_Threshold(uint8_t thr)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_W_PedoThreshold(thr);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Enable_Tilt_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
  LSM6DSL_OpStatus op_status;

  /* Output Data Rate selection */
  op_status = LSM6DSL_Set_X_ODR(26.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSL_Set_X_FS(2.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable embedded functionalities */
  op_status = LSM6DSL_ACC_GYRO_W_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable tilt calculation. */
  op_status = LSM6DSL_ACC_GYRO_W_TILT(LSM6DSL_ACC_GYRO_TILT_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable tilt detection on either INT1 or INT2 pin */
  switch (int_pin)
  {
  case LSM6DSL_INT1_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_TiltEvOnInt1(LSM6DSL_ACC_GYRO_INT1_TILT_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  case LSM6DSL_INT2_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_TiltEvOnInt2(LSM6DSL_ACC_GYRO_INT2_TILT_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Disable_Tilt_Detection(void)
{
  LSM6DSL_OpStatus op_status;

  /* Disable tilt event on INT1. */
  op_status = LSM6DSL_ACC_GYRO_W_TiltEvOnInt1(LSM6DSL_ACC_GYRO_INT1_TILT_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable tilt event on INT2. */
  op_status = LSM6DSL_ACC_GYRO_W_TiltEvOnInt2(LSM6DSL_ACC_GYRO_INT2_TILT_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable tilt calculation. */
  op_status = LSM6DSL_ACC_GYRO_W_TILT(LSM6DSL_ACC_GYRO_TILT_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable embedded functionalities */
  op_status = LSM6DSL_ACC_GYRO_W_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Enable_Wake_Up_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
  LSM6DSL_OpStatus op_status;

  /* Output Data Rate selection */
  op_status = LSM6DSL_Set_X_ODR(416.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSL_Set_X_FS(2.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* WAKE_DUR setting */
  op_status = LSM6DSL_ACC_GYRO_W_WAKE_DUR(0x00);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set wake up threshold. */
  op_status = LSM6DSL_ACC_GYRO_W_WK_THS(0x02);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable wake up detection on either INT1 or INT2 pin */
  switch (int_pin)
  {
  case LSM6DSL_INT1_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_WUEvOnInt1(LSM6DSL_ACC_GYRO_INT1_WU_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  case LSM6DSL_INT2_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_WUEvOnInt2(LSM6DSL_ACC_GYRO_INT2_WU_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Disable_Wake_Up_Detection(void)
{
  LSM6DSL_OpStatus op_status;

  /* Disable wake up event on INT1 */
  op_status = LSM6DSL_ACC_GYRO_W_WUEvOnInt1(LSM6DSL_ACC_GYRO_INT1_WU_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable wake up event on INT2 */
  op_status = LSM6DSL_ACC_GYRO_W_WUEvOnInt2(LSM6DSL_ACC_GYRO_INT2_WU_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* WU_DUR setting */
  op_status = LSM6DSL_ACC_GYRO_W_WAKE_DUR(0x00);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* WU_THS setting */
  op_status = LSM6DSL_ACC_GYRO_W_WK_THS(0x00);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_Wake_Up_Threshold(uint8_t thr)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_W_WK_THS(thr);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Enable_Single_Tap_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
  LSM6DSL_OpStatus op_status;

  /* Output Data Rate selection */
  op_status = LSM6DSL_Set_X_ODR(416.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSL_Set_X_FS(2.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable X direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_X_EN(LSM6DSL_ACC_GYRO_TAP_X_EN_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable Y direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_Y_EN(LSM6DSL_ACC_GYRO_TAP_Y_EN_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable Z direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_Z_EN(LSM6DSL_ACC_GYRO_TAP_Z_EN_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set tap threshold. */
  op_status = LSM6DSL_Set_Tap_Threshold(LSM6DSL_TAP_THRESHOLD_MID_LOW);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set tap shock time window. */
  op_status = LSM6DSL_Set_Tap_Shock_Time(LSM6DSL_TAP_SHOCK_TIME_MID_HIGH);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set tap quiet time window. */
  op_status = LSM6DSL_Set_Tap_Quiet_Time(LSM6DSL_TAP_QUIET_TIME_MID_LOW);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* _NOTE_: Tap duration time window - don't care for single tap. */

  /* _NOTE_: Single/Double Tap event - don't care of this flag for single tap. */

  /* Enable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable single tap on either INT1 or INT2 pin */
  switch (int_pin)
  {
  case LSM6DSL_INT1_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_SingleTapOnInt1(LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  case LSM6DSL_INT2_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_SingleTapOnInt2(LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Disable_Single_Tap_Detection(void)
{
  LSM6DSL_OpStatus op_status;

  /* Disable single tap interrupt on INT1 pin. */
  op_status = LSM6DSL_ACC_GYRO_W_SingleTapOnInt1(LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable single tap interrupt on INT2 pin. */
  op_status = LSM6DSL_ACC_GYRO_W_SingleTapOnInt2(LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Reset tap threshold. */
  op_status = LSM6DSL_Set_Tap_Threshold(0x0);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Reset tap shock time window. */
  op_status = LSM6DSL_Set_Tap_Shock_Time(0x0);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Reset tap quiet time window. */
  op_status = LSM6DSL_Set_Tap_Quiet_Time(0x0);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* _NOTE_: Tap duration time window - don't care for single tap. */

  /* _NOTE_: Single/Double Tap event - don't care of this flag for single tap. */

  /* Disable Z direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_Z_EN(LSM6DSL_ACC_GYRO_TAP_Z_EN_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable Y direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_Y_EN(LSM6DSL_ACC_GYRO_TAP_Y_EN_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable X direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_X_EN(LSM6DSL_ACC_GYRO_TAP_X_EN_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Enable_Double_Tap_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
  LSM6DSL_OpStatus op_status;

  /* Output Data Rate selection */
  op_status = LSM6DSL_Set_X_ODR(416.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSL_Set_X_FS(2.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable X direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_X_EN(LSM6DSL_ACC_GYRO_TAP_X_EN_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable Y direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_Y_EN(LSM6DSL_ACC_GYRO_TAP_Y_EN_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable Z direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_Z_EN(LSM6DSL_ACC_GYRO_TAP_Z_EN_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set tap threshold. */
  op_status = LSM6DSL_Set_Tap_Threshold(LSM6DSL_TAP_THRESHOLD_MID_LOW);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set tap shock time window. */
  op_status = LSM6DSL_Set_Tap_Shock_Time(LSM6DSL_TAP_SHOCK_TIME_HIGH);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set tap quiet time window. */
  op_status = LSM6DSL_Set_Tap_Quiet_Time(LSM6DSL_TAP_QUIET_TIME_HIGH);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set tap duration time window. */
  op_status = LSM6DSL_Set_Tap_Duration_Time(LSM6DSL_TAP_DURATION_TIME_MID);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Single and double tap enabled. */
  op_status = LSM6DSL_ACC_GYRO_W_SINGLE_DOUBLE_TAP_EV(LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_DOUBLE_TAP);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable double tap on either INT1 or INT2 pin */
  switch (int_pin)
  {
  case LSM6DSL_INT1_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_TapEvOnInt1(LSM6DSL_ACC_GYRO_INT1_TAP_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  case LSM6DSL_INT2_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_TapEvOnInt2(LSM6DSL_ACC_GYRO_INT2_TAP_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Disable_Double_Tap_Detection(void)
{
  LSM6DSL_OpStatus op_status;

  /* Disable double tap interrupt on INT1 pin. */
  op_status = LSM6DSL_ACC_GYRO_W_TapEvOnInt1(LSM6DSL_ACC_GYRO_INT1_TAP_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable double tap interrupt on INT2 pin. */
  op_status = LSM6DSL_ACC_GYRO_W_TapEvOnInt2(LSM6DSL_ACC_GYRO_INT2_TAP_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Reset tap threshold. */
  op_status = LSM6DSL_Set_Tap_Threshold(0x0);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Reset tap shock time window. */
  op_status = LSM6DSL_Set_Tap_Shock_Time(0x0);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Reset tap quiet time window. */
  op_status = LSM6DSL_Set_Tap_Quiet_Time(0x0);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Reset tap duration time window. */
  op_status = LSM6DSL_Set_Tap_Duration_Time(0x0);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Only single tap enabled. */
  op_status = LSM6DSL_ACC_GYRO_W_SINGLE_DOUBLE_TAP_EV(LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_SINGLE_TAP);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable Z direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_Z_EN(LSM6DSL_ACC_GYRO_TAP_Z_EN_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable Y direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_Y_EN(LSM6DSL_ACC_GYRO_TAP_Y_EN_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable X direction in tap recognition. */
  op_status = LSM6DSL_ACC_GYRO_W_TAP_X_EN(LSM6DSL_ACC_GYRO_TAP_X_EN_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_Tap_Threshold(uint8_t thr)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_W_TAP_THS(thr);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_Tap_Shock_Time(uint8_t time)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_W_SHOCK_Duration(time);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_Tap_Quiet_Time(uint8_t time)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_W_QUIET_Duration(time);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Set_Tap_Duration_Time(uint8_t time)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_W_DUR(time);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Enable_6D_Orientation(LSM6DSL_Interrupt_Pin_t int_pin)
{
  LSM6DSL_OpStatus op_status;

  /* Output Data Rate selection */
  op_status = LSM6DSL_Set_X_ODR(416.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Full scale selection. */
  op_status = LSM6DSL_Set_X_FS(2.0f);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Set 6D threshold. */
  op_status = LSM6DSL_ACC_GYRO_W_SIXD_THS(LSM6DSL_ACC_GYRO_SIXD_THS_60_degree);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable 6D orientation on either INT1 or INT2 pin */
  switch (int_pin)
  {
  case LSM6DSL_INT1_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_6DEvOnInt1(LSM6DSL_ACC_GYRO_INT1_6D_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  case LSM6DSL_INT2_PIN:
    op_status = LSM6DSL_ACC_GYRO_W_6DEvOnInt2(LSM6DSL_ACC_GYRO_INT2_6D_ENABLED);
    if(op_status < LSM6DSL_OK)
    {
      return op_status ;
    }
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Disable_6D_Orientation(void)
{
  LSM6DSL_OpStatus op_status;

  /* Disable 6D orientation interrupt on INT1 pin. */
  op_status = LSM6DSL_ACC_GYRO_W_6DEvOnInt1(LSM6DSL_ACC_GYRO_INT1_6D_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable 6D orientation interrupt on INT2 pin. */
  op_status = LSM6DSL_ACC_GYRO_W_6DEvOnInt2(LSM6DSL_ACC_GYRO_INT2_6D_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Disable basic Interrupts */
  op_status = LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Reset 6D threshold. */
  op_status = LSM6DSL_ACC_GYRO_W_SIXD_THS(LSM6DSL_ACC_GYRO_SIXD_THS_80_degree);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_6D_Orientation_XL(uint8_t *xl)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_DSD_XL_t xl_raw;

  op_status = LSM6DSL_ACC_GYRO_R_DSD_XL(&xl_raw);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  switch(xl_raw)
  {
    case LSM6DSL_ACC_GYRO_DSD_XL_DETECTED:
      *xl = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_XL_NOT_DETECTED:
      *xl = 0;
      break;
    default:
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_6D_Orientation_XH(uint8_t *xh)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_DSD_XH_t xh_raw;

  op_status = LSM6DSL_ACC_GYRO_R_DSD_XH(&xh_raw);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  switch(xh_raw)
  {
    case LSM6DSL_ACC_GYRO_DSD_XH_DETECTED:
      *xh = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_XH_NOT_DETECTED:
      *xh = 0;
      break;
    default:
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_6D_Orientation_YL(uint8_t *yl)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_DSD_YL_t yl_raw;

  op_status = LSM6DSL_ACC_GYRO_R_DSD_YL(&yl_raw);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  switch(yl_raw)
  {
    case LSM6DSL_ACC_GYRO_DSD_YL_DETECTED:
      *yl = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_YL_NOT_DETECTED:
      *yl = 0;
      break;
    default:
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_6D_Orientation_YH(uint8_t *yh)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_DSD_YH_t yh_raw;

  op_status = LSM6DSL_ACC_GYRO_R_DSD_YH(&yh_raw);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  switch(yh_raw)
  {
    case LSM6DSL_ACC_GYRO_DSD_YH_DETECTED:
      *yh = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_YH_NOT_DETECTED:
      *yh = 0;
      break;
    default:
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_6D_Orientation_ZL(uint8_t *zl)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_DSD_ZL_t zl_raw;

  op_status = LSM6DSL_ACC_GYRO_R_DSD_ZL(&zl_raw);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  switch(zl_raw)
  {
    case LSM6DSL_ACC_GYRO_DSD_ZL_DETECTED:
      *zl = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_ZL_NOT_DETECTED:
      *zl = 0;
      break;
    default:
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_6D_Orientation_ZH(uint8_t *zh)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_DSD_ZH_t zh_raw;

  op_status = LSM6DSL_ACC_GYRO_R_DSD_ZH(&zh_raw);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  switch(zh_raw)
  {
    case LSM6DSL_ACC_GYRO_DSD_ZH_DETECTED:
      *zh = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_ZH_NOT_DETECTED:
      *zh = 0;
      break;
    default:
      return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_Get_Event_Status(LSM6DSL_Event_Status_t *status)
{
  LSM6DSL_OpStatus op_status;

  uint8_t Wake_Up_Src = 0, Tap_Src = 0, D6D_Src = 0, Func_Src = 0, Md1_Cfg = 0, Md2_Cfg = 0, Int1_Ctrl = 0;

  memset((void *)status, 0x0, sizeof(LSM6DSL_Event_Status_t));

  op_status = LSM6DSL_IO_Read(LSM6DSL_ACC_GYRO_WAKE_UP_SRC, &Wake_Up_Src, 1);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  op_status = LSM6DSL_IO_Read(LSM6DSL_ACC_GYRO_TAP_SRC, &Tap_Src, 1);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  op_status = LSM6DSL_IO_Read(LSM6DSL_ACC_GYRO_D6D_SRC, &D6D_Src, 1);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  op_status = LSM6DSL_IO_Read(LSM6DSL_ACC_GYRO_FUNC_SRC, &Func_Src, 1);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  op_status = LSM6DSL_IO_Read(LSM6DSL_ACC_GYRO_MD1_CFG, &Md1_Cfg, 1);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  op_status = LSM6DSL_IO_Read(LSM6DSL_ACC_GYRO_MD2_CFG, &Md2_Cfg, 1);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  op_status = LSM6DSL_IO_Read(LSM6DSL_ACC_GYRO_INT1_CTRL, &Int1_Ctrl, 1);
  if(op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_FF_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_FF_MASK))
  {
    if((Wake_Up_Src & LSM6DSL_ACC_GYRO_FF_EV_STATUS_MASK))
    {
      status->FreeFallStatus = 1;
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_WU_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_WU_MASK))
  {
    if((Wake_Up_Src & LSM6DSL_ACC_GYRO_WU_EV_STATUS_MASK))
    {
      status->WakeUpStatus = 1;
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_MASK))
  {
    if((Tap_Src & LSM6DSL_ACC_GYRO_SINGLE_TAP_EV_STATUS_MASK))
    {
      status->TapStatus = 1;
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_TAP_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_TAP_MASK))
  {
    if((Tap_Src & LSM6DSL_ACC_GYRO_DOUBLE_TAP_EV_STATUS_MASK))
    {
      status->DoubleTapStatus = 1;
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_6D_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_6D_MASK))
  {
    if((D6D_Src & LSM6DSL_ACC_GYRO_D6D_EV_STATUS_MASK))
    {
      status->D6DOrientationStatus = 1;
    }
  }

  if((Int1_Ctrl & LSM6DSL_ACC_GYRO_INT1_PEDO_MASK))
  {
    if((Func_Src & LSM6DSL_ACC_GYRO_PEDO_EV_STATUS_MASK))
    {
      status->StepStatus = 1;
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_TILT_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_TILT_MASK))
  {
    if((Func_Src & LSM6DSL_ACC_GYRO_TILT_EV_STATUS_MASK))
    {
      status->TiltStatus = 1;
    }
  }

  return LSM6DSL_OK;
}
