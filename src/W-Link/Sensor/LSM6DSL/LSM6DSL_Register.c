/**
 ******************************************************************************
 * @file    LSM6DSL_ACC_GYRO_driver.c
 * @author  MEMS Application Team
 * @version V1.5
 * @date    17-May-2016
 * @brief   LSM6DSL driver file
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
 * SERVICES; LOSS OF USE, p_data, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
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

#include "LSM6DSL_Def.h"

#include "LSM6DSL_Register_Def.h"
#include "LSM6DSL_Register.h"

typedef union{
	int16_t i16bit[3];
	uint8_t u8bit[6];
} Type3Axis16bit_U;

typedef union{
	int16_t i16bit;
	uint8_t u8bit[2];
} Type1Axis16bit_U;

typedef union{
	int32_t i32bit;
	uint8_t u8bit[4];
} Type1Axis32bit_U;

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_ReadReg(uint8_t reg, uint8_t* p_data, uint16_t len)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_IO_Read(reg, p_data, len);

  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }
  
  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_WriteReg(uint8_t reg, uint8_t *p_data, uint16_t len)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_IO_Write(reg, p_data, len);

  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }
  
  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_WHO_AM_I(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WHO_AM_I_REG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_WHO_AM_I_BIT_MASK; //coerce
  *p_value = *p_value >> LSM6DSL_ACC_GYRO_WHO_AM_I_BIT_POSITION; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_BDU(LSM6DSL_ACC_GYRO_BDU_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_BDU_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_BDU(LSM6DSL_ACC_GYRO_BDU_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_BDU_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FS_XL(LSM6DSL_ACC_GYRO_FS_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FS_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FS_XL(LSM6DSL_ACC_GYRO_FS_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL1_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FS_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_GetRawAccData(uint8_t *buff)
{
  LSM6DSL_OpStatus op_status;

  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;

  numberOfByteForDimension=6/3;

  k=0;
  for (i=0; i<3;i++ )
  {
    for (j=0; j<numberOfByteForDimension;j++ )
    {
      op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_OUTX_L_XL+k, &buff[k], 1);
      
      if (op_status < LSM6DSL_OK)
      {
        return op_status;
      }
      k++;
    }
  }

  return LSM6DSL_OK;
}

/*
 * Following is the table of sensitivity values for each case.
 * Values are expressed in ug/digit.
 */
static const long long LSM6DSL_ACC_Sensitivity_List[4] = {
      61,	/* FS @2g */
      122,	/* FS @4g */
      244,	/* FS @8g */
      488,	/* FS @16g */
};
LSM6DSL_OpStatus LSM6DSL_ACC_Get_Acceleration(int *buff, bool from_fifo)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_FS_XL_t fs;
  long long sensitivity = 0;
  Type3Axis16bit_U raw_DATA_tmp;

  /* Read out current odr, fs, hf setting */
  op_status = LSM6DSL_ACC_GYRO_R_FS_XL(&fs);

  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Determine the sensitivity according to fs */
  switch(fs) {
    case LSM6DSL_ACC_GYRO_FS_XL_2g:
      sensitivity = LSM6DSL_ACC_Sensitivity_List[0];
      break;

    case LSM6DSL_ACC_GYRO_FS_XL_4g:
      sensitivity = LSM6DSL_ACC_Sensitivity_List[1];
      break;

    case LSM6DSL_ACC_GYRO_FS_XL_8g:
      sensitivity = LSM6DSL_ACC_Sensitivity_List[2];
      break;

    case LSM6DSL_ACC_GYRO_FS_XL_16g:
      sensitivity = LSM6DSL_ACC_Sensitivity_List[3];
      break;
  }

  /* Read out raw accelerometer samples */
  if (from_fifo)
  {
    uint8_t i;

    /* read all 3 axis from FIFO */
    for(i = 0; i < 3; i++)
    {
      op_status = LSM6DSL_ACC_GYRO_Get_GetFIFOData(raw_DATA_tmp.u8bit + 2*i);
      if (op_status < LSM6DSL_OK)
      {
        return op_status;
      }
    }
  }
  else
  {
    op_status = LSM6DSL_ACC_GYRO_GetRawAccData(raw_DATA_tmp.u8bit);
    if (op_status < LSM6DSL_OK)
    {
      return op_status;
    }
  }

  /* Apply proper shift and sensitivity */
  buff[0] = (raw_DATA_tmp.i16bit[0] * sensitivity + 500)/1000;
  buff[1] = (raw_DATA_tmp.i16bit[1] * sensitivity + 500)/1000;
  buff[2] = (raw_DATA_tmp.i16bit[2] * sensitivity + 500)/1000;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_ODR_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL1_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_ODR_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_translate_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_t value, uint16_t *odr_hz_val)
{
  LSM6DSL_OpStatus op_status;

  switch(value) {
  case LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN:
    *odr_hz_val = 0;
    break;

  case LSM6DSL_ACC_GYRO_ODR_XL_13Hz:
    *odr_hz_val = 13;
    break;

  case LSM6DSL_ACC_GYRO_ODR_XL_26Hz:
    *odr_hz_val = 26;
    break;

  case LSM6DSL_ACC_GYRO_ODR_XL_52Hz:
    *odr_hz_val = 52;
    break;

  case LSM6DSL_ACC_GYRO_ODR_XL_104Hz:
    *odr_hz_val = 104;
    break;

  case LSM6DSL_ACC_GYRO_ODR_XL_208Hz:
    *odr_hz_val = 208;
    break;

  case LSM6DSL_ACC_GYRO_ODR_XL_416Hz:
    *odr_hz_val = 416;
    break;

  case LSM6DSL_ACC_GYRO_ODR_XL_833Hz:
    *odr_hz_val = 833;
    break;

  case LSM6DSL_ACC_GYRO_ODR_XL_1660Hz:
    *odr_hz_val = 1660;
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FS_G(LSM6DSL_ACC_GYRO_FS_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL2_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FS_G_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL2_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FS_G(LSM6DSL_ACC_GYRO_FS_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL2_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FS_G_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_GetRawGyroData(uint8_t *buff)
{
  LSM6DSL_OpStatus op_status;

  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;

  numberOfByteForDimension=6/3;

  k=0;
  for (i=0; i<3;i++ )
  {
	for (j=0; j<numberOfByteForDimension;j++ )
	{
		op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_OUTX_L_G+k, &buff[k], 1);
		
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }
		k++;
	}
  }

  return LSM6DSL_OK;
}

/*
 * Following is the table of sensitivity values for each case.
 * Values are espressed in udps/digit.
 */
static const long long LSM6DSL_GYRO_Sensitivity_List[5] = {
      4375,	/* FS @125 */
      8750,	/* FS @245 */
      17500,	/* FS @500 */
      35000,	/* FS @1000 */
      70000,	/* FS @2000 */
};
LSM6DSL_OpStatus LSM6DSL_ACC_Get_AngularRate(int *buff, uint8_t from_fifo)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_FS_125_t fs_125;
  LSM6DSL_ACC_GYRO_FS_G_t fs;
  long long sensitivity = 0;
  Type3Axis16bit_U raw_DATA_tmp;

  /* Read out current odr, fs, hf setting */
  LSM6DSL_ACC_GYRO_R_FS_125(&fs_125);
  if (fs_125 == LSM6DSL_ACC_GYRO_FS_125_ENABLED) {
    sensitivity = LSM6DSL_GYRO_Sensitivity_List[0];
  } else {
    LSM6DSL_ACC_GYRO_R_FS_G(&fs);

    /* Determine the sensitivity according to fs */
    switch(fs) {
    case LSM6DSL_ACC_GYRO_FS_G_245dps:
      sensitivity = LSM6DSL_GYRO_Sensitivity_List[1];
      break;

    case LSM6DSL_ACC_GYRO_FS_G_500dps:
      sensitivity = LSM6DSL_GYRO_Sensitivity_List[2];
      break;

    case LSM6DSL_ACC_GYRO_FS_G_1000dps:
      sensitivity = LSM6DSL_GYRO_Sensitivity_List[3];
      break;

    case LSM6DSL_ACC_GYRO_FS_G_2000dps:
      sensitivity = LSM6DSL_GYRO_Sensitivity_List[4];
      break;
    }
  }

  /* Read out raw accelerometer samples */
  if (from_fifo) {
    uint8_t i;

    /* read all 3 axis from FIFO */
    for(i = 0; i < 3; i++)
      LSM6DSL_ACC_GYRO_Get_GetFIFOData(raw_DATA_tmp.u8bit + 2*i);
  } else
    LSM6DSL_ACC_GYRO_GetRawGyroData(raw_DATA_tmp.u8bit);

  /* Apply proper shift and sensitivity */
  buff[0] = (raw_DATA_tmp.i16bit[0] * sensitivity + 500)/1000;
  buff[1] = (raw_DATA_tmp.i16bit[1] * sensitivity + 500)/1000;
  buff[2] = (raw_DATA_tmp.i16bit[2] * sensitivity + 500)/1000;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_ODR_G(LSM6DSL_ACC_GYRO_ODR_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL2_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_ODR_G_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL2_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_ODR_G(LSM6DSL_ACC_GYRO_ODR_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL2_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_ODR_G_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_translate_ODR_G(LSM6DSL_ACC_GYRO_ODR_G_t value, uint16_t *odr_hz_val)
{
  LSM6DSL_OpStatus op_status;

  switch(value) {
  case LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN:
    *odr_hz_val = 0;
    break;

  case LSM6DSL_ACC_GYRO_ODR_G_13Hz:
    *odr_hz_val = 13;
    break;

  case LSM6DSL_ACC_GYRO_ODR_G_26Hz:
    *odr_hz_val = 26;
    break;

  case LSM6DSL_ACC_GYRO_ODR_G_52Hz:
    *odr_hz_val = 52;
    break;

  case LSM6DSL_ACC_GYRO_ODR_G_104Hz:
    *odr_hz_val = 104;
    break;

  case LSM6DSL_ACC_GYRO_ODR_G_208Hz:
    *odr_hz_val = 208;
    break;

  case LSM6DSL_ACC_GYRO_ODR_G_416Hz:
    *odr_hz_val = 416;
    break;

  case LSM6DSL_ACC_GYRO_ODR_G_833Hz:
    *odr_hz_val = 833;
    break;

  case LSM6DSL_ACC_GYRO_ODR_G_1660Hz:
    *odr_hz_val = 1660;
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FS_125(LSM6DSL_ACC_GYRO_FS_125_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL2_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FS_125_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL2_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FS_125(LSM6DSL_ACC_GYRO_FS_125_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL2_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FS_125_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_BW_SEL(LSM6DSL_ACC_GYRO_BW_SEL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_BW_SEL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_BW_SEL(LSM6DSL_ACC_GYRO_BW_SEL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL1_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_BW_SEL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_BLE(LSM6DSL_ACC_GYRO_BLE_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_BLE_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_BLE(LSM6DSL_ACC_GYRO_BLE_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_BLE_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMB_ACC_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_CFG_ACCESS, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_EMB_ACC_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FUNC_CFG_ACCESS, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMB_ACC_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_CFG_ACCESS, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_EMB_ACC_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SYNC_RES_RATIO(LSM6DSL_ACC_GYRO_SYNC_RES_RATIO_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_SENSOR_RES_RATIO, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SYNC_RES_RATIO_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_SENSOR_RES_RATIO, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SYNC_RES_RATIO(LSM6DSL_ACC_GYRO_SYNC_RES_RATIO_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_SENSOR_RES_RATIO, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SYNC_RES_RATIO_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_Stamping_Time_Frame(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  newValue = newValue << LSM6DSL_ACC_GYRO_TPH_POSITION; //mask
  newValue &= LSM6DSL_ACC_GYRO_TPH_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_SENSOR_SYNC_TIME, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= (uint8_t)~LSM6DSL_ACC_GYRO_TPH_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_SENSOR_SYNC_TIME, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_Stamping_Time_Frame(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_SENSOR_SYNC_TIME, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TPH_MASK; //coerce
  *p_value = *p_value >> LSM6DSL_ACC_GYRO_TPH_POSITION; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FIFO_Watermark(uint16_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t valueH, valueL;
  uint8_t value;

  valueL = newValue & 0xFF;
  valueH = (newValue >> 8) & 0xFF;

  /* Low part goes in FIFO_CTRL1 */
  valueL = valueL << LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL1_POSITION; //mask
  valueL &= LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL1_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= (uint8_t)~LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL1_MASK;
  value |= valueL;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* High part goes in FIFO_CTRL2 */
  valueH = valueH << LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL2_POSITION; //mask
  valueH &= LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL2_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL2_MASK;
  value |= valueH;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FIFO_Watermark(uint16_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  uint8_t valueH, valueL;

  /* Low part from FIFO_CTRL1 */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL1, (uint8_t *)&valueL, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  valueL &= LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL1_MASK; //coerce
  valueL = valueL >> LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL1_POSITION; //mask

  /* High part from FIFO_CTRL2 */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, (uint8_t *)&valueH, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  valueH &= LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL2_MASK; //coerce
  valueH = valueH >> LSM6DSL_ACC_GYRO_WTM_FIFO_CTRL2_POSITION; //mask

  *p_value = ((valueH << 8) & 0xFF00) | valueL;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FIFO_TEMP(LSM6DSL_ACC_GYRO_FIFO_TEMP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FIFO_TEMP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FIFO_TEMP(LSM6DSL_ACC_GYRO_FIFO_TEMP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FIFO_TEMP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TIM_PEDO_FIFO_Write_En(LSM6DSL_ACC_GYRO_TIM_PEDO_FIFO_DRDY_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_TIM_PEDO_FIFO_DRDY_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TIM_PEDO_FIFO_Write_En(LSM6DSL_ACC_GYRO_TIM_PEDO_FIFO_DRDY_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TIM_PEDO_FIFO_DRDY_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TIM_PEDO_FIFO_En(LSM6DSL_ACC_GYRO_TIM_PEDO_FIFO_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_TIM_PEDO_FIFO_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TIM_PEDO_FIFO_En(LSM6DSL_ACC_GYRO_TIM_PEDO_FIFO_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TIM_PEDO_FIFO_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL3, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DEC_FIFO_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL3, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL_val(uint16_t newValue)
{
  LSM6DSL_OpStatus op_status;

  switch(newValue) {
  case 0:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DATA_NOT_IN_FIFO);
    break;

  case 1:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_NO_DECIMATION);
    break;

  case 2:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DECIMATION_BY_2);
    break;

  case 3:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DECIMATION_BY_3);
    break;

  case 4:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DECIMATION_BY_4);
    break;

  case 8:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DECIMATION_BY_8);
    break;

  case 16:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DECIMATION_BY_16);
    break;

  case 32:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DECIMATION_BY_32);
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DEC_FIFO_XL(LSM6DSL_ACC_GYRO_DEC_FIFO_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL3, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DEC_FIFO_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL3, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DEC_FIFO_G_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL3, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DEC_FIFO_G_val(uint16_t newValue)
{
  LSM6DSL_OpStatus op_status;

  switch(newValue) {
  case 0:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_DATA_NOT_IN_FIFO);
    break;

  case 1:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_NO_DECIMATION);
    break;

  case 2:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_DECIMATION_BY_2);
    break;

  case 3:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_DECIMATION_BY_3);
    break;

  case 4:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_DECIMATION_BY_4);
    break;

  case 8:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_DECIMATION_BY_8);
    break;

  case 16:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_DECIMATION_BY_16);
    break;

  case 32:
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_DECIMATION_BY_32);
    break;

  default:
    return LSM6DSL_InvalidParameter;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DEC_FIFO_G(LSM6DSL_ACC_GYRO_DEC_FIFO_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL3, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DEC_FIFO_G_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DEC_FIFO_DS3(LSM6DSL_ACC_GYRO_DEC_FIFO_DS3_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DEC_FIFO_DS3_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DEC_FIFO_DS3(LSM6DSL_ACC_GYRO_DEC_FIFO_DS3_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DEC_FIFO_DS3_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DEC_FIFO_DS4(LSM6DSL_ACC_GYRO_DEC_FIFO_DS4_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DEC_FIFO_DS4_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DEC_FIFO_DS4(LSM6DSL_ACC_GYRO_DEC_FIFO_DS4_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DEC_FIFO_DS4_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_HI_DATA_ONLY(LSM6DSL_ACC_GYRO_HI_DATA_ONLY_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_HI_DATA_ONLY_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_HI_DATA_ONLY(LSM6DSL_ACC_GYRO_HI_DATA_ONLY_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_HI_DATA_ONLY_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_STOP_ON_FTH(LSM6DSL_ACC_GYRO_STOP_ON_FTH_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_STOP_ON_FTH_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_STOP_ON_FTH(LSM6DSL_ACC_GYRO_STOP_ON_FTH_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL4, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_STOP_ON_FTH_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FIFO_MODE(LSM6DSL_ACC_GYRO_FIFO_MODE_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FIFO_MODE_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FIFO_MODE(LSM6DSL_ACC_GYRO_FIFO_MODE_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FIFO_MODE_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_ODR_FIFO(LSM6DSL_ACC_GYRO_ODR_FIFO_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_ODR_FIFO_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_ODR_FIFO(LSM6DSL_ACC_GYRO_ODR_FIFO_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_ODR_FIFO_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DRDY_PULSE(LSM6DSL_ACC_GYRO_DRDY_PULSE_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_DRDY_PULSE_CFG_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DRDY_PULSE_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_DRDY_PULSE_CFG_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DRDY_PULSE(LSM6DSL_ACC_GYRO_DRDY_PULSE_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_DRDY_PULSE_CFG_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DRDY_PULSE_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DRDY_XL_on_INT1(LSM6DSL_ACC_GYRO_INT1_DRDY_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_DRDY_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DRDY_XL_on_INT1(LSM6DSL_ACC_GYRO_INT1_DRDY_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_DRDY_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DRDY_G_on_INT1(LSM6DSL_ACC_GYRO_INT1_DRDY_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_DRDY_G_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DRDY_G_on_INT1(LSM6DSL_ACC_GYRO_INT1_DRDY_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_DRDY_G_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_BOOT_on_INT1(LSM6DSL_ACC_GYRO_INT1_BOOT_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_BOOT_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_BOOT_on_INT1(LSM6DSL_ACC_GYRO_INT1_BOOT_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_BOOT_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FIFO_TSHLD_on_INT1(LSM6DSL_ACC_GYRO_INT1_FTH_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_FTH_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FIFO_TSHLD_on_INT1(LSM6DSL_ACC_GYRO_INT1_FTH_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_FTH_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_OVERRUN_on_INT1(LSM6DSL_ACC_GYRO_INT1_OVR_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_OVR_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_OVERRUN_on_INT1(LSM6DSL_ACC_GYRO_INT1_OVR_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_OVR_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FULL_FLAG_on_INT1(LSM6DSL_ACC_GYRO_INT1_FULL_FLAG_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_FULL_FLAG_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FULL_FLAG_on_INT1(LSM6DSL_ACC_GYRO_INT1_FULL_FLAG_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_FULL_FLAG_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SIGN_MOT_on_INT1(LSM6DSL_ACC_GYRO_INT1_SIGN_MOT_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_SIGN_MOT_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SIGN_MOT_on_INT1(LSM6DSL_ACC_GYRO_INT1_SIGN_MOT_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_SIGN_MOT_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_STEP_DET_on_INT1(LSM6DSL_ACC_GYRO_INT1_PEDO_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_PEDO_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_STEP_DET_on_INT1(LSM6DSL_ACC_GYRO_INT1_PEDO_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_PEDO_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DRDY_XL_on_INT2(LSM6DSL_ACC_GYRO_INT2_DRDY_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_DRDY_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DRDY_XL_on_INT2(LSM6DSL_ACC_GYRO_INT2_DRDY_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_DRDY_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DRDY_G_on_INT2(LSM6DSL_ACC_GYRO_INT2_DRDY_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_DRDY_G_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DRDY_G_on_INT2(LSM6DSL_ACC_GYRO_INT2_DRDY_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_DRDY_G_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DRDY_TEMP_on_INT2(LSM6DSL_ACC_GYRO_INT2_DRDY_TEMP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_DRDY_TEMP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DRDY_TEMP_on_INT2(LSM6DSL_ACC_GYRO_INT2_DRDY_TEMP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_DRDY_TEMP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FIFO_TSHLD_on_INT2(LSM6DSL_ACC_GYRO_INT2_FTH_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_FTH_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FIFO_TSHLD_on_INT2(LSM6DSL_ACC_GYRO_INT2_FTH_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_FTH_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_OVERRUN_on_INT2(LSM6DSL_ACC_GYRO_INT2_OVR_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_OVR_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_OVERRUN_on_INT2(LSM6DSL_ACC_GYRO_INT2_OVR_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_OVR_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FULL_FLAG_on_INT2(LSM6DSL_ACC_GYRO_INT2_FULL_FLAG_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_FULL_FLAG_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FULL_FLAG_on_INT2(LSM6DSL_ACC_GYRO_INT2_FULL_FLAG_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_FULL_FLAG_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_STEP_COUNT_OV_on_INT2(LSM6DSL_ACC_GYRO_INT2_STEP_COUNT_OV_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_STEP_COUNT_OV_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_STEP_COUNT_OV_on_INT2(LSM6DSL_ACC_GYRO_INT2_STEP_COUNT_OV_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_STEP_COUNT_OV_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_STEP_DELTA_on_INT2(LSM6DSL_ACC_GYRO_INT2_STEP_DELTA_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_STEP_DELTA_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT2_CTRL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_STEP_DELTA_on_INT2(LSM6DSL_ACC_GYRO_INT2_STEP_DELTA_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT2_CTRL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_STEP_DELTA_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SW_RESET(LSM6DSL_ACC_GYRO_SW_RESET_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SW_RESET_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SW_RESET(LSM6DSL_ACC_GYRO_SW_RESET_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SW_RESET_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_IF_Addr_Incr(LSM6DSL_ACC_GYRO_IF_INC_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_IF_INC_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_IF_Addr_Incr(LSM6DSL_ACC_GYRO_IF_INC_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_IF_INC_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SPI_Mode(LSM6DSL_ACC_GYRO_SIM_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SIM_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SPI_Mode(LSM6DSL_ACC_GYRO_SIM_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SIM_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_PadSel(LSM6DSL_ACC_GYRO_PP_OD_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_PP_OD_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_PadSel(LSM6DSL_ACC_GYRO_PP_OD_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_PP_OD_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_INT_ACT_LEVEL(LSM6DSL_ACC_GYRO_INT_ACT_LEVEL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT_ACT_LEVEL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_INT_ACT_LEVEL(LSM6DSL_ACC_GYRO_INT_ACT_LEVEL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT_ACT_LEVEL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_BOOT(LSM6DSL_ACC_GYRO_BOOT_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_BOOT_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL3_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_BOOT(LSM6DSL_ACC_GYRO_BOOT_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL3_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_BOOT_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_LPF1_SEL_G(LSM6DSL_ACC_GYRO_LPF1_SEL_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_LPF1_SEL_G_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_LPF1_SEL_G(LSM6DSL_ACC_GYRO_LPF1_SEL_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_LPF1_SEL_G_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_I2C_DISABLE(LSM6DSL_ACC_GYRO_I2C_DISABLE_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_I2C_DISABLE_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_I2C_DISABLE(LSM6DSL_ACC_GYRO_I2C_DISABLE_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_I2C_DISABLE_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DRDY_MSK(LSM6DSL_ACC_GYRO_DRDY_MSK_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DRDY_MSK_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DRDY_MSK(LSM6DSL_ACC_GYRO_DRDY_MSK_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DRDY_MSK_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_INT2_ON_INT1(LSM6DSL_ACC_GYRO_INT2_ON_INT1_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_ON_INT1_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_INT2_ON_INT1(LSM6DSL_ACC_GYRO_INT2_ON_INT1_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_ON_INT1_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SleepMode_G(LSM6DSL_ACC_GYRO_SLEEP_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SLEEP_G_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL4_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SleepMode_G(LSM6DSL_ACC_GYRO_SLEEP_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL4_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SLEEP_G_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SelfTest_XL(LSM6DSL_ACC_GYRO_ST_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL5_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_ST_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL5_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SelfTest_XL(LSM6DSL_ACC_GYRO_ST_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL5_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_ST_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SelfTest_G(LSM6DSL_ACC_GYRO_ST_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL5_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_ST_G_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL5_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SelfTest_G(LSM6DSL_ACC_GYRO_ST_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL5_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_ST_G_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DEN_Polarity(LSM6DSL_ACC_GYRO_DEN_LH_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL5_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DEN_LH_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL5_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DEN_Polarity(LSM6DSL_ACC_GYRO_DEN_LH_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL5_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DEN_LH_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_CircularBurstMode(LSM6DSL_ACC_GYRO_ROUNDING_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL5_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_LSM6DSL_ACC_GYRO_ROUNDING_t_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL5_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_CircularBurstMode(LSM6DSL_ACC_GYRO_ROUNDING_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL5_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_LSM6DSL_ACC_GYRO_ROUNDING_t_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_LP_BW_G(LSM6DSL_ACC_GYRO_FTYPE_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FTYPE_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_LP_BW_G(LSM6DSL_ACC_GYRO_FTYPE_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FTYPE_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_UserOffsetWeight(LSM6DSL_ACC_GYRO_USR_OFF_W_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_USR_OFF_W_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_UserOffsetWeight(LSM6DSL_ACC_GYRO_USR_OFF_W_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_USR_OFF_W_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_LowPower_XL(LSM6DSL_ACC_GYRO_LP_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_LP_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_LowPower_XL(LSM6DSL_ACC_GYRO_LP_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_LP_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DEN_LVL2_EN(LSM6DSL_ACC_GYRO_DEN_LVL2_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DEN_LVL2_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DEN_LVL2_EN(LSM6DSL_ACC_GYRO_DEN_LVL2_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DEN_LVL2_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DEN_LVL_EN(LSM6DSL_ACC_GYRO_DEN_LVL_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DEN_LVL_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DEN_LVL_EN(LSM6DSL_ACC_GYRO_DEN_LVL_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DEN_LVL_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_ExternalTrigger(LSM6DSL_ACC_GYRO_DEN_EDGE_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DEN_EDGE_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_ExternalTrigger(LSM6DSL_ACC_GYRO_DEN_EDGE_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL6_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DEN_EDGE_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_HPM_G(LSM6DSL_ACC_GYRO_HPM_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_HPM_G_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_HPM_G(LSM6DSL_ACC_GYRO_HPM_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_HPM_G_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_RoundingOnStatusRegisters(LSM6DSL_ACC_GYRO_RND_STATUS_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_RND_STATUS_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_RoundingOnStatusRegisters(LSM6DSL_ACC_GYRO_RND_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_RND_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_HPFilter_En(LSM6DSL_ACC_GYRO_HP_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_HP_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_HPFilter_En(LSM6DSL_ACC_GYRO_HP_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_HP_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_LP_Mode(LSM6DSL_ACC_GYRO_LP_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_LP_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_LP_Mode(LSM6DSL_ACC_GYRO_LP_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_LP_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_ROUNDING_STATUS(LSM6DSL_ACC_GYRO_ROUNDING_STATUS_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_ROUNDING_STATUS_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_ROUNDING_STATUS(LSM6DSL_ACC_GYRO_ROUNDING_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_ROUNDING_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_HP_G_RST(LSM6DSL_ACC_GYRO_HP_G_RST_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_HP_G_RST_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL7_G, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_HP_G_RST(LSM6DSL_ACC_GYRO_HP_G_RST_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL7_G, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_HP_G_RST_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_InComposit(LSM6DSL_ACC_GYRO_IN_COMP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_IN_COMP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_InComposit(LSM6DSL_ACC_GYRO_IN_COMP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_IN_COMP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_HPfilterReference(LSM6DSL_ACC_GYRO_HP_REF_MODE_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_HP_REF_MODE_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_HPfilterReference(LSM6DSL_ACC_GYRO_HP_REF_MODE_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_HP_REF_MODE_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_HPCF_XL(LSM6DSL_ACC_GYRO_HPCF_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_HPCF_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_HPCF_XL(LSM6DSL_ACC_GYRO_HPCF_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_HPCF_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_LowPassFiltSel_XL(LSM6DSL_ACC_GYRO_LPF2_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_LPF2_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_LowPassFiltSel_XL(LSM6DSL_ACC_GYRO_LPF2_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_LPF2_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_LOW_PASS_ON_6D(LSM6DSL_ACC_GYRO_LOW_PASS_ON_6D_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_LOW_PASS_ON_6D_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_LOW_PASS_ON_6D(LSM6DSL_ACC_GYRO_LOW_PASS_ON_6D_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_LOW_PASS_ON_6D_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_HP_SLOPE_XL(LSM6DSL_ACC_GYRO_HP_SLOPE_XL_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_HP_SLOPE_XL_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL8_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_HP_SLOPE_XL(LSM6DSL_ACC_GYRO_HP_SLOPE_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL8_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_HP_SLOPE_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SOFT(LSM6DSL_ACC_GYRO_SOFT_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL9_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SOFT_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL9_XL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SOFT(LSM6DSL_ACC_GYRO_SOFT_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL9_XL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SOFT_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SignifcantMotion(LSM6DSL_ACC_GYRO_SIGN_MOTION_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SIGN_MOTION_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SignifcantMotion(LSM6DSL_ACC_GYRO_SIGN_MOTION_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SIGN_MOTION_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_PedoStepReset(LSM6DSL_ACC_GYRO_PEDO_RST_STEP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_PEDO_RST_STEP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_PedoStepReset(LSM6DSL_ACC_GYRO_PEDO_RST_STEP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_PEDO_RST_STEP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TILT(LSM6DSL_ACC_GYRO_TILT_G_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_TILT_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TILT(LSM6DSL_ACC_GYRO_TILT_G_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TILT_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_PEDO(LSM6DSL_ACC_GYRO_PEDO_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_PEDO_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_PEDO(LSM6DSL_ACC_GYRO_PEDO_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_PEDO_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TIMER(LSM6DSL_ACC_GYRO_TIMER_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_TIMER_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TIMER(LSM6DSL_ACC_GYRO_TIMER_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TIMER_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FUNC_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CTRL10_C, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CTRL10_C, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FUNC_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_I2C_MASTER_Enable(LSM6DSL_ACC_GYRO_MASTER_ON_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_MASTER_ON_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_I2C_MASTER_Enable(LSM6DSL_ACC_GYRO_MASTER_ON_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_MASTER_ON_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_IronCorrection_EN(LSM6DSL_ACC_GYRO_IRON_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_IRON_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_IronCorrection_EN(LSM6DSL_ACC_GYRO_IRON_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_IRON_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_PASS_THRU_MODE(LSM6DSL_ACC_GYRO_PASS_THRU_MODE_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_PASS_THRU_MODE_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_PASS_THRU_MODE(LSM6DSL_ACC_GYRO_PASS_THRU_MODE_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_PASS_THRU_MODE_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_PULL_UP_EN(LSM6DSL_ACC_GYRO_PULL_UP_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_PULL_UP_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_PULL_UP_EN(LSM6DSL_ACC_GYRO_PULL_UP_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_PULL_UP_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SensorHUB_Trigger_Sel(LSM6DSL_ACC_GYRO_START_CONFIG_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_START_CONFIG_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SensorHUB_Trigger_Sel(LSM6DSL_ACC_GYRO_START_CONFIG_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_START_CONFIG_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DATA_VAL_SEL_FIFO(LSM6DSL_ACC_GYRO_DATA_VAL_SEL_FIFO_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DATA_VAL_SEL_FIFO_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DATA_VAL_SEL_FIFO(LSM6DSL_ACC_GYRO_DATA_VAL_SEL_FIFO_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DATA_VAL_SEL_FIFO_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DRDY_ON_INT1(LSM6DSL_ACC_GYRO_DRDY_ON_INT1_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DRDY_ON_INT1_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DRDY_ON_INT1(LSM6DSL_ACC_GYRO_DRDY_ON_INT1_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MASTER_CONFIG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DRDY_ON_INT1_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_Z_WU(LSM6DSL_ACC_GYRO_Z_WU_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_Z_WU_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_Y_WU(LSM6DSL_ACC_GYRO_Y_WU_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_Y_WU_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_X_WU(LSM6DSL_ACC_GYRO_X_WU_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_X_WU_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_WU_EV_STATUS(LSM6DSL_ACC_GYRO_WU_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_WU_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SLEEP_EV_STATUS(LSM6DSL_ACC_GYRO_SLEEP_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SLEEP_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FF_EV_STATUS(LSM6DSL_ACC_GYRO_FF_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FF_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_Z_TAP(LSM6DSL_ACC_GYRO_Z_TAP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_Z_TAP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_Y_TAP(LSM6DSL_ACC_GYRO_Y_TAP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_Y_TAP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_X_TAP(LSM6DSL_ACC_GYRO_X_TAP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_X_TAP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TAP_SIGN(LSM6DSL_ACC_GYRO_TAP_SIGN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TAP_SIGN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DOUBLE_TAP_EV_STATUS(LSM6DSL_ACC_GYRO_DOUBLE_TAP_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DOUBLE_TAP_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SINGLE_TAP_EV_STATUS(LSM6DSL_ACC_GYRO_SINGLE_TAP_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SINGLE_TAP_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TAP_EV_STATUS(LSM6DSL_ACC_GYRO_TAP_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TAP_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DSD_XL(LSM6DSL_ACC_GYRO_DSD_XL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_D6D_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DSD_XL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DSD_XH(LSM6DSL_ACC_GYRO_DSD_XH_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_D6D_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DSD_XH_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DSD_YL(LSM6DSL_ACC_GYRO_DSD_YL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_D6D_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DSD_YL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DSD_YH(LSM6DSL_ACC_GYRO_DSD_YH_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_D6D_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DSD_YH_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DSD_ZL(LSM6DSL_ACC_GYRO_DSD_ZL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_D6D_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DSD_ZL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DSD_ZH(LSM6DSL_ACC_GYRO_DSD_ZH_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_D6D_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DSD_ZH_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_D6D_EV_STATUS(LSM6DSL_ACC_GYRO_D6D_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_D6D_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_D6D_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_XLDA(LSM6DSL_ACC_GYRO_XLDA_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_STATUS_REG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_XLDA_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_GDA(LSM6DSL_ACC_GYRO_GDA_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_STATUS_REG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_GDA_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TDA(LSM6DSL_ACC_GYRO_TDA_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_STATUS_REG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TDA_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FIFONumOfEntries(uint16_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  uint8_t valueH, valueL;

  /* Low part from FIFO_STATUS1 */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_STATUS1, (uint8_t *)&valueL, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  valueL &= LSM6DSL_ACC_GYRO_DIFF_FIFO_STATUS1_MASK; //coerce
  valueL = valueL >> LSM6DSL_ACC_GYRO_DIFF_FIFO_STATUS1_POSITION; //mask

  /* High part from FIFO_STATUS2 */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_STATUS2, (uint8_t *)&valueH, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  valueH &= LSM6DSL_ACC_GYRO_DIFF_FIFO_STATUS2_MASK; //coerce
  valueH = valueH >> LSM6DSL_ACC_GYRO_DIFF_FIFO_STATUS2_POSITION; //mask

  *p_value = ((valueH << 8) & 0xFF00) | valueL;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FIFOEmpty(LSM6DSL_ACC_GYRO_FIFO_EMPTY_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_STATUS2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FIFO_EMPTY_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FIFOFull(LSM6DSL_ACC_GYRO_FIFO_FULL_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_STATUS2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FIFO_FULL_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_OVERRUN(LSM6DSL_ACC_GYRO_OVERRUN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_STATUS2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_OVERRUN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_WaterMark(LSM6DSL_ACC_GYRO_WTM_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_STATUS2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_WTM_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FIFOPattern(uint16_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  uint8_t valueH, valueL;

  /* Low part from FIFO_STATUS3 */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_STATUS3, (uint8_t *)&valueL, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  valueL &= LSM6DSL_ACC_GYRO_FIFO_STATUS3_PATTERN_MASK; //coerce
  valueL = valueL >> LSM6DSL_ACC_GYRO_FIFO_STATUS3_PATTERN_POSITION; //mask

  /* High part from FIFO_STATUS4 */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_STATUS4, (uint8_t *)&valueH, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  valueH &= LSM6DSL_ACC_GYRO_FIFO_STATUS4_PATTERN_MASK; //coerce
  valueH = valueH >> LSM6DSL_ACC_GYRO_FIFO_STATUS4_PATTERN_POSITION; //mask

  *p_value = ((valueH << 8) & 0xFF00) | valueL;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SENS_HUB_END(LSM6DSL_ACC_GYRO_SENS_HUB_END_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SENS_HUB_END_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SOFT_IRON_END(LSM6DSL_ACC_GYRO_SOFT_IRON_END_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SOFT_IRON_END_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_HardIron(LSM6DSL_ACC_GYRO_SOFT_HARD_IRON_STAT_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_HARD_IRON_STAT_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_STEP_OVERFLOW(LSM6DSL_ACC_GYRO_STEP_OVERFLOW_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_STEP_OVERFLOW_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_STEP_COUNT_DELTA(LSM6DSL_ACC_GYRO_STEP_COUNT_DELTA_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_STEP_COUNT_DELTA_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_PEDO_EV_STATUS(LSM6DSL_ACC_GYRO_PEDO_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_PEDO_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TILT_EV_STATUS(LSM6DSL_ACC_GYRO_TILT_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TILT_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SIGN_MOT_EV_STATUS(LSM6DSL_ACC_GYRO_SIGN_MOT_EV_STATUS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FUNC_SRC, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SIGN_MOT_EV_STATUS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_LIR(LSM6DSL_ACC_GYRO_LIR_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_LIR_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_LIR(LSM6DSL_ACC_GYRO_LIR_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_LIR_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TAP_Z_EN(LSM6DSL_ACC_GYRO_TAP_Z_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_TAP_Z_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TAP_Z_EN(LSM6DSL_ACC_GYRO_TAP_Z_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TAP_Z_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TAP_Y_EN(LSM6DSL_ACC_GYRO_TAP_Y_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_TAP_Y_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TAP_Y_EN(LSM6DSL_ACC_GYRO_TAP_Y_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TAP_Y_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TAP_X_EN(LSM6DSL_ACC_GYRO_TAP_X_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_TAP_X_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TAP_X_EN(LSM6DSL_ACC_GYRO_TAP_X_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TAP_X_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SLOPE_FDS(LSM6DSL_ACC_GYRO_SLOPE_FDS_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SLOPE_FDS_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SLOPE_FDS(LSM6DSL_ACC_GYRO_SLOPE_FDS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SLOPE_FDS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_BASIC_INT(LSM6DSL_ACC_GYRO_INT_EN_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT_EN_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_BASIC_INT(LSM6DSL_ACC_GYRO_INT_EN_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_CFG1, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT_EN_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TAP_THS(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  newValue = newValue << LSM6DSL_ACC_GYRO_TAP_THS_POSITION; //mask
  newValue &= LSM6DSL_ACC_GYRO_TAP_THS_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_TAP_THS_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TAP_THS(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TAP_THS_MASK; //coerce
  *p_value = *p_value >> LSM6DSL_ACC_GYRO_TAP_THS_POSITION; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SIXD_THS(LSM6DSL_ACC_GYRO_SIXD_THS_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SIXD_THS_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SIXD_THS(LSM6DSL_ACC_GYRO_SIXD_THS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SIXD_THS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_D4D(LSM6DSL_ACC_GYRO_D4D_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_D4D_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_D4D(LSM6DSL_ACC_GYRO_D4D_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_D4D_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SHOCK_Duration(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  newValue = newValue << LSM6DSL_ACC_GYRO_SHOCK_POSITION; //mask
  newValue &= LSM6DSL_ACC_GYRO_SHOCK_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT_DUR2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SHOCK_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT_DUR2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SHOCK_Duration(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT_DUR2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SHOCK_MASK; //coerce
  *p_value = *p_value >> LSM6DSL_ACC_GYRO_SHOCK_POSITION; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_QUIET_Duration(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  newValue = newValue << LSM6DSL_ACC_GYRO_QUIET_POSITION; //mask
  newValue &= LSM6DSL_ACC_GYRO_QUIET_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT_DUR2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_QUIET_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT_DUR2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_QUIET_Duration(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT_DUR2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_QUIET_MASK; //coerce
  *p_value = *p_value >> LSM6DSL_ACC_GYRO_QUIET_POSITION; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_DUR(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  newValue = newValue << LSM6DSL_ACC_GYRO_DUR_POSITION; //mask
  newValue &= LSM6DSL_ACC_GYRO_DUR_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT_DUR2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_DUR_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_INT_DUR2, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_DUR(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_INT_DUR2, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_DUR_MASK; //coerce
  *p_value = *p_value >> LSM6DSL_ACC_GYRO_DUR_POSITION; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_WK_THS(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  newValue = newValue << LSM6DSL_ACC_GYRO_WK_THS_POSITION; //mask
  newValue &= LSM6DSL_ACC_GYRO_WK_THS_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_THS, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_WK_THS_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_WAKE_UP_THS, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_WK_THS(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_THS, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_WK_THS_MASK; //coerce
  *p_value = *p_value >> LSM6DSL_ACC_GYRO_WK_THS_POSITION; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SINGLE_DOUBLE_TAP_EV(LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_THS, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_WAKE_UP_THS, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SINGLE_DOUBLE_TAP_EV(LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_THS, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SLEEP_DUR(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  newValue = newValue << LSM6DSL_ACC_GYRO_SLEEP_DUR_POSITION; //mask
  newValue &= LSM6DSL_ACC_GYRO_SLEEP_DUR_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_SLEEP_DUR_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SLEEP_DUR(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_SLEEP_DUR_MASK; //coerce
  *p_value = *p_value >> LSM6DSL_ACC_GYRO_SLEEP_DUR_POSITION; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TIMER_HR(LSM6DSL_ACC_GYRO_TIMER_HR_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_TIMER_HR_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TIMER_HR(LSM6DSL_ACC_GYRO_TIMER_HR_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_TIMER_HR_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_WAKE_DUR(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  newValue = newValue << LSM6DSL_ACC_GYRO_WAKE_DUR_POSITION; //mask
  newValue &= LSM6DSL_ACC_GYRO_WAKE_DUR_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_WAKE_DUR_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_WAKE_DUR(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_WAKE_DUR_MASK; //coerce
  *p_value = *p_value >> LSM6DSL_ACC_GYRO_WAKE_DUR_POSITION; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FF_THS(LSM6DSL_ACC_GYRO_FF_THS_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FREE_FALL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FF_THS_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FREE_FALL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FF_THS(LSM6DSL_ACC_GYRO_FF_THS_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FREE_FALL, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_FF_THS_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FF_Duration(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t valueH, valueL;
  uint8_t value;

  valueL = newValue & 0x1F;
  valueH = (newValue >> 5) & 0x1;

  /* Low part in FREE_FALL reg */
  valueL = valueL << LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_POSITION; //mask
  valueL &= LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FREE_FALL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK;
  value |= valueL;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_FREE_FALL, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* High part in WAKE_UP_DUR reg */
  valueH = valueH << LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_POSITION; //mask
  valueH &= LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_MASK; //coerce

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_MASK;
  value |= valueH;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FF_Duration(uint8_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  uint8_t valueH, valueL;

  /* Low part from FREE_FALL reg */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FREE_FALL, (uint8_t *)&valueL, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  valueL &= LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK; //coerce
  valueL = valueL >> LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_POSITION; //mask

  /* High part from WAKE_UP_DUR reg */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, (uint8_t *)&valueH, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  valueH &= LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_MASK; //coerce
  valueH = valueH >> LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_POSITION; //mask

  *p_value = ((valueH << 5) & 0x20) | valueL;

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TimerEvRouteInt1(LSM6DSL_ACC_GYRO_INT1_TIMER_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_TIMER_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TimerEvRouteInt1(LSM6DSL_ACC_GYRO_INT1_TIMER_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_TIMER_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TiltEvOnInt1(LSM6DSL_ACC_GYRO_INT1_TILT_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_TILT_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TiltEvOnInt1(LSM6DSL_ACC_GYRO_INT1_TILT_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_TILT_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_6DEvOnInt1(LSM6DSL_ACC_GYRO_INT1_6D_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_6D_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_6DEvOnInt1(LSM6DSL_ACC_GYRO_INT1_6D_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_6D_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TapEvOnInt1(LSM6DSL_ACC_GYRO_INT1_TAP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_TAP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TapEvOnInt1(LSM6DSL_ACC_GYRO_INT1_TAP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_TAP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FFEvOnInt1(LSM6DSL_ACC_GYRO_INT1_FF_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_FF_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FFEvOnInt1(LSM6DSL_ACC_GYRO_INT1_FF_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_FF_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_WUEvOnInt1(LSM6DSL_ACC_GYRO_INT1_WU_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_WU_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_WUEvOnInt1(LSM6DSL_ACC_GYRO_INT1_WU_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_WU_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SingleTapOnInt1(LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SingleTapOnInt1(LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SleepEvOnInt1(LSM6DSL_ACC_GYRO_INT1_SLEEP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT1_SLEEP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SleepEvOnInt1(LSM6DSL_ACC_GYRO_INT1_SLEEP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT1_SLEEP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_MagCorrection_Int2(LSM6DSL_ACC_GYRO_INT2_IRON_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_IRON_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_MagCorrection_Int2(LSM6DSL_ACC_GYRO_INT2_IRON_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_IRON_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TiltEvOnInt2(LSM6DSL_ACC_GYRO_INT2_TILT_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_TILT_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TiltEvOnInt2(LSM6DSL_ACC_GYRO_INT2_TILT_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_TILT_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_6DEvOnInt2(LSM6DSL_ACC_GYRO_INT2_6D_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_6D_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_6DEvOnInt2(LSM6DSL_ACC_GYRO_INT2_6D_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_6D_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_TapEvOnInt2(LSM6DSL_ACC_GYRO_INT2_TAP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_TAP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_TapEvOnInt2(LSM6DSL_ACC_GYRO_INT2_TAP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_TAP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_FFEvOnInt2(LSM6DSL_ACC_GYRO_INT2_FF_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_FF_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_FFEvOnInt2(LSM6DSL_ACC_GYRO_INT2_FF_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_FF_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_WUEvOnInt2(LSM6DSL_ACC_GYRO_INT2_WU_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_WU_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_WUEvOnInt2(LSM6DSL_ACC_GYRO_INT2_WU_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_WU_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SingleTapOnInt2(LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SingleTapOnInt2(LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_SleepEvOnInt2(LSM6DSL_ACC_GYRO_INT2_SLEEP_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~LSM6DSL_ACC_GYRO_INT2_SLEEP_MASK;
  value |= newValue;

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MD2_CFG, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_R_SleepEvOnInt2(LSM6DSL_ACC_GYRO_INT2_SLEEP_t *p_value)
{
  LSM6DSL_OpStatus op_status;

  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, (uint8_t *)p_value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  *p_value &= LSM6DSL_ACC_GYRO_INT2_SLEEP_MASK; //mask

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_Get_GetFIFOData(uint8_t *buff)
{
  LSM6DSL_OpStatus op_status;

  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;

  numberOfByteForDimension=2/1;

  k=0;
  for (i=0; i<1;i++ )
  {
    for (j=0; j<numberOfByteForDimension;j++ )
    {
      op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_FIFO_DATA_OUT_L+k, &buff[k], 1);
      
      if (op_status < LSM6DSL_OK)
      {
        return op_status;
      }
      k++;
    }
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_Get_GetTimestamp(uint8_t *buff)
{
  LSM6DSL_OpStatus op_status;

  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;

  numberOfByteForDimension=3/1;

  k=0;
  for (i=0; i<1;i++ )
  {
    for (j=0; j<numberOfByteForDimension;j++ )
    {
      op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_TIMESTAMP0_REG+k, &buff[k], 1);
      
      if (op_status < LSM6DSL_OK)
      {
        return op_status;
      }
      k++;
    }
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_Get_GetStepCounter(uint8_t *buff)
{
  LSM6DSL_OpStatus op_status;

  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;

  numberOfByteForDimension=2/1;

  k=0;
  for (i=0; i<1;i++ )
  {
    for (j=0; j<numberOfByteForDimension;j++ )
    {
      op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_STEP_COUNTER_L+k, &buff[k], 1);
      
      if (op_status < LSM6DSL_OK)
      {
        return op_status;
      }
      k++;
    }
  }

  return LSM6DSL_OK;
}

LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_W_PedoThreshold(uint8_t newValue)
{
  LSM6DSL_OpStatus op_status;

  uint8_t value;

  /* Open Embedded Function Register page*/
  op_status = LSM6DSL_ACC_GYRO_W_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_ENABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* read current value */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_CONFIG_PEDO_THS_MIN, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  value &= ~0x1F;
  value |= (newValue & 0x1F);

  /* write new value */
  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_CONFIG_PEDO_THS_MIN, &value, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Close Embedded Function Register page*/
  op_status = LSM6DSL_ACC_GYRO_W_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_DISABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

/************** Use Sensor Hub  *******************/
/*
 * Program the nine Soft Iron Matrix coefficients.
 * The SI_Matrix buffer must provide coefficients
 * in xx, xy, xz, yx, yy, yz, zx, zy, zz order.
 */
LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_SH_init_SI_Matrix(uint8_t *SI_matrix)
{
  LSM6DSL_OpStatus op_status;

  /* Open Embedded Function Register page*/
  op_status = LSM6DSL_ACC_GYRO_W_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_ENABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Write the Soft Iron Matrix coefficients */
  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_MAG_SI_XX, SI_matrix, 9);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Close Embedded Function Register page*/
  op_status = LSM6DSL_ACC_GYRO_W_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_DISABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

/* Read a remote device through I2C Sensor Hub Slave 0 */
LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_SH0_Program(uint8_t SlvAddr, uint8_t reg, uint8_t len)
{
  LSM6DSL_OpStatus op_status;

  /* Open Embedded Function Register page*/
  op_status = LSM6DSL_ACC_GYRO_W_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_ENABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Write remote device I2C slave address */
  SlvAddr |= 0x1; /* Raise the read op bit */
  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_SLV0_ADD, &SlvAddr, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Write remote device I2C subaddress */
  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_SLV0_SUBADD, &reg, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Write number of bytes to read [SLAVE0_CONFIG - 04h ]*/
  uint8_t sl0_cfg = 0;
  sl0_cfg |= 0x00;       //00 bit [7-6] : no decimation
  sl0_cfg |= 0x00;       //00 bit [5-4] : one sensor
  sl0_cfg |= 0x00;       // 0 bit [3] : source mode read disabled
  sl0_cfg |= len & 0x07; // bit [2-0] : number of bytes

  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_SLAVE0_CONFIG, &sl0_cfg, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Close Embedded Function Register page*/
  op_status = LSM6DSL_ACC_GYRO_W_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_DISABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable FUNC */
  op_status = LSM6DSL_ACC_GYRO_W_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* MASTER_EN */
  op_status = LSM6DSL_ACC_GYRO_W_I2C_MASTER_Enable(LSM6DSL_ACC_GYRO_MASTER_ON_ENABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}

/* Read a remote device through I2C Sensor Hub Slave 0 */
LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_SH0_ReadMem(uint8_t SlvAddr, uint8_t reg, uint8_t *Bufp, uint8_t len, uint8_t stop)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_SENS_HUB_END_t op_cmpl = LSM6DSL_ACC_GYRO_SENS_HUB_END_STILL_ONGOING;
  LSM6DSL_ACC_GYRO_XLDA_t op_update = LSM6DSL_ACC_GYRO_XLDA_NO_DATA_AVAIL;
  uint8_t dummy[6];

  op_status = LSM6DSL_ACC_GYRO_W_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  op_status = LSM6DSL_ACC_GYRO_SH0_Program(SlvAddr, reg, len);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Syncronize the SH with internal trigger (xl) */
  op_status = LSM6DSL_ACC_GYRO_W_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_104Hz);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Wait until operation is not completed */
  op_status = LSM6DSL_ACC_GYRO_GetRawAccData(dummy);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }
  do {
    op_status = LSM6DSL_ACC_GYRO_R_XLDA(&op_update);
    if (op_status < LSM6DSL_OK)
    {
      return op_status;
    }
  } while(op_update != LSM6DSL_ACC_GYRO_XLDA_DATA_AVAIL);
  do {
    op_status = LSM6DSL_ACC_GYRO_R_SENS_HUB_END(&op_cmpl);
    if (op_status < LSM6DSL_OK)
    {
      return op_status;
    }
  } while(op_cmpl != LSM6DSL_ACC_GYRO_SENS_HUB_END_OP_COMPLETED);

  /* Read the result */
  op_status = LSM6DSL_ACC_GYRO_ReadReg(LSM6DSL_ACC_GYRO_SENSORHUB1_REG, Bufp, len);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  op_status = LSM6DSL_ACC_GYRO_W_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  if (stop) {
    /* Stop everything */
    op_status = LSM6DSL_ACC_GYRO_W_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED);
    if (op_status < LSM6DSL_OK)
    {
      return op_status;
    }

    op_status = LSM6DSL_ACC_GYRO_W_I2C_MASTER_Enable(LSM6DSL_ACC_GYRO_MASTER_ON_DISABLED);
    if (op_status < LSM6DSL_OK)
    {
      return op_status;
    }
  }

  return LSM6DSL_OK;
}

/* Write a remote device through I2C Sensor Hub Slave 0 */
LSM6DSL_OpStatus LSM6DSL_ACC_GYRO_SH0_WriteByte(uint8_t SlvAddr, uint8_t reg, uint8_t Bufp)
{
  LSM6DSL_OpStatus op_status;

  LSM6DSL_ACC_GYRO_SENS_HUB_END_t op_cmpl = LSM6DSL_ACC_GYRO_SENS_HUB_END_STILL_ONGOING;
  LSM6DSL_ACC_GYRO_XLDA_t op_update = LSM6DSL_ACC_GYRO_XLDA_NO_DATA_AVAIL;
  uint8_t dummy[6];

  /* Open Embedded Function Register page*/
  op_status = LSM6DSL_ACC_GYRO_W_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_ENABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Write remote device I2C slave address */
  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_SLV0_ADD, &SlvAddr, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Write remote device I2C subaddress */
  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_SLV0_SUBADD, &reg, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Write the p_data */
  op_status = LSM6DSL_ACC_GYRO_WriteReg(LSM6DSL_ACC_GYRO_DATAWRITE_SRC_MODE_SUB_SLV0, &Bufp, 1);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Close Embedded Function Register page*/
  op_status = LSM6DSL_ACC_GYRO_W_EmbeddedAccess(LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_DISABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Enable FUNC */
  op_status = LSM6DSL_ACC_GYRO_W_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

    /* Enable PULL_UP_EN and MASTER_EN */
  //LSM6DSL_ACC_GYRO_W_PULL_UP_EN(LSM6DSL_ACC_GYRO_PULL_UP_EN_ENABLED);
  op_status = LSM6DSL_ACC_GYRO_W_I2C_MASTER_Enable(LSM6DSL_ACC_GYRO_MASTER_ON_ENABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Syncronize the SH with internal trigger (xl) */
  op_status = LSM6DSL_ACC_GYRO_W_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_104Hz);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Wait until operation is not completed */
  op_status = LSM6DSL_ACC_GYRO_GetRawAccData(dummy);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }
  do {
    op_status = LSM6DSL_ACC_GYRO_R_XLDA(&op_update);
    if (op_status < LSM6DSL_OK)
    {
      return op_status;
    }
  } while(op_update != LSM6DSL_ACC_GYRO_XLDA_DATA_AVAIL);
  do {
    op_status = LSM6DSL_ACC_GYRO_R_SENS_HUB_END(&op_cmpl);
    if (op_status < LSM6DSL_OK)
    {
      return op_status;
    }
  } while(op_cmpl != LSM6DSL_ACC_GYRO_SENS_HUB_END_OP_COMPLETED);

  op_status = LSM6DSL_ACC_GYRO_W_ODR_XL(LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  /* Stop everything */
  op_status = LSM6DSL_ACC_GYRO_W_FUNC_EN(LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }
  
  op_status = LSM6DSL_ACC_GYRO_W_I2C_MASTER_Enable(LSM6DSL_ACC_GYRO_MASTER_ON_DISABLED);
  if (op_status < LSM6DSL_OK)
  {
    return op_status;
  }

  return LSM6DSL_OK;
}
