/**
 ******************************************************************************
 * @file    LSM303AGR_ACC_driver.c
 * @author  MEMS Application Team
 * @version V1.1
 * @date    24-February-2016
 * @brief   LSM303AGR Accelerometer driver file
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

#include "LSM303AGR_Def.h"

#include "LSM303AGR_IO.h"

#include "LSM303AGR_ACC_Register_Def.h"
#include "LSM303AGR_ACC_Register.h"

typedef union{
	int16_t i16bit[3];
	uint8_t u8bit[6];
} Type3Axis16bit_U;	

LSM303AGR_OpStatus LSM303AGR_ACC_ReadReg(uint8_t reg, uint8_t* p_data) 
{
  return LSM303AGR_ACC_IO_Read(reg, p_data, 1);
}

LSM303AGR_OpStatus LSM303AGR_ACC_WriteReg(uint8_t reg, uint8_t data) 
{
  return LSM303AGR_ACC_IO_Write(reg, &data, 1);
}

void LSM303AGR_ACC_SwapHighLowByte(uint8_t *bufferToSwap, uint8_t numberOfByte, uint8_t dimension)
{
  LSM303AGR_OpStatus op_status;

  uint8_t numberOfByteForDimension, i, j;
  uint8_t tempValue[10];
  
  numberOfByteForDimension = numberOfByte/dimension;
  
  for (i=0; i<dimension;i++ )
  {
    for (j=0; j<numberOfByteForDimension;j++ )
    {
      tempValue[j]=bufferToSwap[j+i*numberOfByteForDimension];
    }
    for (j=0; j<numberOfByteForDimension;j++ )
    {
      *(bufferToSwap+i*(numberOfByteForDimension)+j)=*(tempValue+(numberOfByteForDimension-1)-j);
    }
  } 
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_x_data_avail(LSM303AGR_ACC_1DA_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG_AUX, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_1DA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_y_data_avail(LSM303AGR_ACC_2DA__t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG_AUX, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_2DA__MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_z_data_avail(LSM303AGR_ACC_3DA__t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG_AUX, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_3DA__MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_xyz_data_avail(LSM303AGR_ACC_321DA__t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG_AUX, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_321DA__MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_DataXOverrun(LSM303AGR_ACC_1OR__t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG_AUX, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_1OR__MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_DataYOverrun(LSM303AGR_ACC_2OR__t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG_AUX, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_2OR__MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_DataZOverrun(LSM303AGR_ACC_3OR__t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG_AUX, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_3OR__MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_DataXYZOverrun(LSM303AGR_ACC_321OR__t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG_AUX, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_321OR__MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_int_counter(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT_COUNTER_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_IC_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_IC_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_WHO_AM_I(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_WHO_AM_I_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_WHO_AM_I_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_WHO_AM_I_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_TEMP_EN_bits(LSM303AGR_ACC_TEMP_EN_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TEMP_CFG_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_TEMP_EN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_TEMP_CFG_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_TEMP_EN_bits(LSM303AGR_ACC_TEMP_EN_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TEMP_CFG_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_TEMP_EN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ADC_PD(LSM303AGR_ACC_ADC_PD_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TEMP_CFG_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ADC_PD_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_TEMP_CFG_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ADC_PD(LSM303AGR_ACC_ADC_PD_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TEMP_CFG_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ADC_PD_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_XEN(LSM303AGR_ACC_XEN_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_XEN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG1, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_XEN(LSM303AGR_ACC_XEN_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XEN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_YEN(LSM303AGR_ACC_YEN_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_YEN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG1, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_YEN(LSM303AGR_ACC_YEN_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YEN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ZEN(LSM303AGR_ACC_ZEN_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ZEN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG1, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ZEN(LSM303AGR_ACC_ZEN_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZEN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_LOWPWR_EN(LSM303AGR_ACC_LPEN_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_LPEN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG1, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_LOWPWR_EN(LSM303AGR_ACC_LPEN_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_LPEN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ODR(LSM303AGR_ACC_ODR_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ODR_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG1, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ODR(LSM303AGR_ACC_ODR_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG1, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ODR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_hpf_aoi_en_int1(LSM303AGR_ACC_HPIS1_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_HPIS1_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG2, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_hpf_aoi_en_int1(LSM303AGR_ACC_HPIS1_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_HPIS1_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_hpf_aoi_en_int2(LSM303AGR_ACC_HPIS2_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_HPIS2_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG2, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_hpf_aoi_en_int2(LSM303AGR_ACC_HPIS2_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_HPIS2_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_hpf_click_en(LSM303AGR_ACC_HPCLICK_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_HPCLICK_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG2, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_hpf_click_en(LSM303AGR_ACC_HPCLICK_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_HPCLICK_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Data_Filter(LSM303AGR_ACC_FDS_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_FDS_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG2, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Data_Filter(LSM303AGR_ACC_FDS_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_FDS_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_hpf_cutoff_freq(LSM303AGR_ACC_HPCF_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_HPCF_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG2, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_hpf_cutoff_freq(LSM303AGR_ACC_HPCF_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_HPCF_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_hpf_mode(LSM303AGR_ACC_HPM_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_HPM_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG2, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_hpf_mode(LSM303AGR_ACC_HPM_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_HPM_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FIFO_Overrun_on_INT1(LSM303AGR_ACC_I1_OVERRUN_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I1_OVERRUN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG3, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FIFO_Overrun_on_INT1(LSM303AGR_ACC_I1_OVERRUN_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I1_OVERRUN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FIFO_Watermark_on_INT1(LSM303AGR_ACC_I1_WTM_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I1_WTM_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG3, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FIFO_Watermark_on_INT1(LSM303AGR_ACC_I1_WTM_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I1_WTM_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FIFO_DRDY2_on_INT1(LSM303AGR_ACC_I1_DRDY2_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I1_DRDY2_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG3, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FIFO_DRDY2_on_INT1(LSM303AGR_ACC_I1_DRDY2_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I1_DRDY2_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FIFO_DRDY1_on_INT1(LSM303AGR_ACC_I1_DRDY1_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I1_DRDY1_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG3, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FIFO_DRDY1_on_INT1(LSM303AGR_ACC_I1_DRDY1_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I1_DRDY1_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FIFO_AOL2_on_INT1(LSM303AGR_ACC_I1_AOI2_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I1_AOI2_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG3, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FIFO_AOL2_on_INT1(LSM303AGR_ACC_I1_AOI2_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I1_AOI2_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FIFO_AOL1_on_INT1(LSM303AGR_ACC_I1_AOI1_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I1_AOI1_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG3, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FIFO_AOL1_on_INT1(LSM303AGR_ACC_I1_AOI1_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I1_AOI1_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FIFO_Click_on_INT1(LSM303AGR_ACC_I1_CLICK_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I1_CLICK_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG3, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FIFO_Click_on_INT1(LSM303AGR_ACC_I1_CLICK_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG3, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I1_CLICK_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_SPI_mode(LSM303AGR_ACC_SIM_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_SIM_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG4, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_SPI_mode(LSM303AGR_ACC_SIM_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_SIM_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_SelfTest(LSM303AGR_ACC_ST_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ST_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG4, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_SelfTest(LSM303AGR_ACC_ST_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ST_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_HiRes(LSM303AGR_ACC_HR_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_HR_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG4, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_HiRes(LSM303AGR_ACC_HR_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_HR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FullScale(LSM303AGR_ACC_FS_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_FS_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG4, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FullScale(LSM303AGR_ACC_FS_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_FS_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_LittleBigEndian(LSM303AGR_ACC_BLE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_BLE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG4, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_LittleBigEndian(LSM303AGR_ACC_BLE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_BLE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_BlockDataUpdate(LSM303AGR_ACC_BDU_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_BDU_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG4, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_BlockDataUpdate(LSM303AGR_ACC_BDU_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG4, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_BDU_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_4D_on_INT2(LSM303AGR_ACC_D4D_INT2_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_D4D_INT2_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG5, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_4D_on_INT2(LSM303AGR_ACC_D4D_INT2_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_D4D_INT2_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_LatchInterrupt_on_INT2(LSM303AGR_ACC_LIR_INT2_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_LIR_INT2_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG5, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_LatchInterrupt_on_INT2(LSM303AGR_ACC_LIR_INT2_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_LIR_INT2_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_4D_on_INT1(LSM303AGR_ACC_D4D_INT1_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_D4D_INT1_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG5, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_4D_on_INT1(LSM303AGR_ACC_D4D_INT1_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_D4D_INT1_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_LatchInterrupt_on_INT1(LSM303AGR_ACC_LIR_INT1_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_LIR_INT1_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG5, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_LatchInterrupt_on_INT1(LSM303AGR_ACC_LIR_INT1_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_LIR_INT1_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FIFO_EN(LSM303AGR_ACC_FIFO_EN_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_FIFO_EN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG5, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FIFO_EN(LSM303AGR_ACC_FIFO_EN_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_FIFO_EN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_RebootMemory(LSM303AGR_ACC_BOOT_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_BOOT_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG5, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_RebootMemory(LSM303AGR_ACC_BOOT_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG5, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_BOOT_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_IntActive(LSM303AGR_ACC_H_LACTIVE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_H_LACTIVE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG6, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_IntActive(LSM303AGR_ACC_H_LACTIVE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_H_LACTIVE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_P2_ACT(LSM303AGR_ACC_P2_ACT_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_P2_ACT_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG6, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_P2_ACT(LSM303AGR_ACC_P2_ACT_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_P2_ACT_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Boot_on_INT2(LSM303AGR_ACC_BOOT_I1_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_BOOT_I1_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG6, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Boot_on_INT2(LSM303AGR_ACC_BOOT_I1_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_BOOT_I1_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_I2_on_INT2(LSM303AGR_ACC_I2_INT2_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I2_INT2_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG6, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_I2_on_INT2(LSM303AGR_ACC_I2_INT2_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I2_INT2_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_I2_on_INT1(LSM303AGR_ACC_I2_INT1_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I2_INT1_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG6, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_I2_on_INT1(LSM303AGR_ACC_I2_INT1_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I2_INT1_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Click_on_INT2(LSM303AGR_ACC_I2_CLICKEN_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_I2_CLICKEN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CTRL_REG6, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Click_on_INT2(LSM303AGR_ACC_I2_CLICKEN_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CTRL_REG6, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_I2_CLICKEN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ReferenceVal(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_REF_POSITION; //mask	
  newValue &= LSM303AGR_ACC_REF_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_REFERENCE, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= (uint8_t)~LSM303AGR_ACC_REF_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_REFERENCE, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ReferenceVal(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_REFERENCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_REF_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_REF_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_XDataAvail(LSM303AGR_ACC_XDA_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XDA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_YDataAvail(LSM303AGR_ACC_YDA_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YDA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ZDataAvail(LSM303AGR_ACC_ZDA_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZDA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_XYZDataAvail(LSM303AGR_ACC_ZYXDA_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZYXDA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_XDataOverrun(LSM303AGR_ACC_XOR_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XOR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_YDataOverrun(LSM303AGR_ACC_YOR_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YOR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ZDataOverrun(LSM303AGR_ACC_ZOR_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZOR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_XYZDataOverrun(LSM303AGR_ACC_ZYXOR_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_STATUS_REG2, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZYXOR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FifoThreshold(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_FTH_POSITION; //mask	
  newValue &= LSM303AGR_ACC_FTH_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_CTRL_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_FTH_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_FIFO_CTRL_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FifoThreshold(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_CTRL_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_FTH_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_FTH_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_TriggerSel(LSM303AGR_ACC_TR_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_CTRL_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_TR_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_FIFO_CTRL_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_TriggerSel(LSM303AGR_ACC_TR_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_CTRL_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_TR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_FifoMode(LSM303AGR_ACC_FM_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_CTRL_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_FM_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_FIFO_CTRL_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FifoMode(LSM303AGR_ACC_FM_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_CTRL_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_FM_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FifoSamplesAvail(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_SRC_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_FSS_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_FSS_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FifoEmpty(LSM303AGR_ACC_EMPTY_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_SRC_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_EMPTY_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_FifoOverrun(LSM303AGR_ACC_OVRN_FIFO_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_SRC_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_OVRN_FIFO_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_WatermarkLevel(LSM303AGR_ACC_WTM_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_FIFO_SRC_REG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_WTM_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1EnXLo(LSM303AGR_ACC_XLIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_XLIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1EnXLo(LSM303AGR_ACC_XLIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XLIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1EnXHi(LSM303AGR_ACC_XHIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_XHIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1EnXHi(LSM303AGR_ACC_XHIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XHIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1EnYLo(LSM303AGR_ACC_YLIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_YLIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1EnYLo(LSM303AGR_ACC_YLIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YLIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1EnYHi(LSM303AGR_ACC_YHIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_YHIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1EnYHi(LSM303AGR_ACC_YHIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YHIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1EnZLo(LSM303AGR_ACC_ZLIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ZLIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1EnZLo(LSM303AGR_ACC_ZLIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZLIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1EnZHi(LSM303AGR_ACC_ZHIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ZHIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1EnZHi(LSM303AGR_ACC_ZHIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZHIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1_6D(LSM303AGR_ACC_6D_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_6D_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_6D(LSM303AGR_ACC_6D_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_6D_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1_AOI(LSM303AGR_ACC_AOI_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_AOI_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_AOI(LSM303AGR_ACC_AOI_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_AOI_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2EnXLo(LSM303AGR_ACC_XLIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_XLIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2EnXLo(LSM303AGR_ACC_XLIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XLIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2EnXHi(LSM303AGR_ACC_XHIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_XHIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2EnXHi(LSM303AGR_ACC_XHIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XHIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2EnYLo(LSM303AGR_ACC_YLIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_YLIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2EnYLo(LSM303AGR_ACC_YLIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YLIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2EnYHi(LSM303AGR_ACC_YHIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_YHIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2EnYHi(LSM303AGR_ACC_YHIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YHIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2EnZLo(LSM303AGR_ACC_ZLIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ZLIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2EnZLo(LSM303AGR_ACC_ZLIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZLIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2EnZHi(LSM303AGR_ACC_ZHIE_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ZHIE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2EnZHi(LSM303AGR_ACC_ZHIE_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZHIE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2_6D(LSM303AGR_ACC_6D_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_6D_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_6D(LSM303AGR_ACC_6D_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_6D_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2_AOI(LSM303AGR_ACC_AOI_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_AOI_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_AOI(LSM303AGR_ACC_AOI_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_AOI_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_Xlo(LSM303AGR_ACC_XL_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XL_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_XHi(LSM303AGR_ACC_XH_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XH_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_YLo(LSM303AGR_ACC_YL_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YL_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_YHi(LSM303AGR_ACC_YH_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YH_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_Zlo(LSM303AGR_ACC_ZL_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZL_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_ZHi(LSM303AGR_ACC_ZH_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZH_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_IA(LSM303AGR_ACC_IA_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_IA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_Xlo(LSM303AGR_ACC_XL_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XL_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_XHi(LSM303AGR_ACC_XH_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XH_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_YLo(LSM303AGR_ACC_YL_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YL_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_YHi(LSM303AGR_ACC_YH_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YH_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_Zlo(LSM303AGR_ACC_ZL_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZL_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_ZHi(LSM303AGR_ACC_ZH_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZH_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_IA(LSM303AGR_ACC_IA_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_SOURCE, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_IA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1_Threshold(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_THS_POSITION; //mask	
  newValue &= LSM303AGR_ACC_THS_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_THS, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_THS_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_THS, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_Threshold(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_THS, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_THS_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_THS_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2_Threshold(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_THS_POSITION; //mask	
  newValue &= LSM303AGR_ACC_THS_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_THS, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_THS_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_THS, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_Threshold(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_THS, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_THS_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_THS_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int1_Duration(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_D_POSITION; //mask	
  newValue &= LSM303AGR_ACC_D_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_DURATION, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_D_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT1_DURATION, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int1_Duration(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT1_DURATION, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_D_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_D_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_Int2_Duration(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_D_POSITION; //mask	
  newValue &= LSM303AGR_ACC_D_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_DURATION, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_D_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_INT2_DURATION, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_Int2_Duration(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_INT2_DURATION, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_D_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_D_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_XSingle(LSM303AGR_ACC_XS_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_XS_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CLICK_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_XSingle(LSM303AGR_ACC_XS_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XS_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_XDouble(LSM303AGR_ACC_XD_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_XD_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CLICK_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_XDouble(LSM303AGR_ACC_XD_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_XD_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_YSingle(LSM303AGR_ACC_YS_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_YS_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CLICK_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_YSingle(LSM303AGR_ACC_YS_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YS_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_YDouble(LSM303AGR_ACC_YD_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_YD_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CLICK_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_YDouble(LSM303AGR_ACC_YD_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_YD_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ZSingle(LSM303AGR_ACC_ZS_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ZS_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CLICK_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ZSingle(LSM303AGR_ACC_ZS_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZS_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ZDouble(LSM303AGR_ACC_ZD_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_ZD_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CLICK_CFG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ZDouble(LSM303AGR_ACC_ZD_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_CFG, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_ZD_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ClickX(LSM303AGR_ACC_X_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_SRC, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_X_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ClickY(LSM303AGR_ACC_Y_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_SRC, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_Y_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ClickZ(LSM303AGR_ACC_Z_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_SRC, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_Z_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ClickSign(LSM303AGR_ACC_SIGN_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_SRC, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_SIGN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_SingleCLICK(LSM303AGR_ACC_SCLICK_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_SRC, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_SCLICK_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_DoubleCLICK(LSM303AGR_ACC_DCLICK_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_SRC, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_DCLICK_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_CLICK_IA(LSM303AGR_ACC_CLICK_IA_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_SRC, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_IA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ClickThreshold(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_THS_POSITION; //mask	
  newValue &= LSM303AGR_ACC_THS_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_THS, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_THS_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_CLICK_THS, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ClickThreshold(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_CLICK_THS, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_THS_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_THS_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ClickTimeLimit(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_TLI_POSITION; //mask	
  newValue &= LSM303AGR_ACC_TLI_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TIME_LIMIT, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_ACC_TLI_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_TIME_LIMIT, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ClickTimeLimit(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TIME_LIMIT, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_TLI_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_TLI_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ClickTimeLatency(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_TLA_POSITION; //mask	
  newValue &= LSM303AGR_ACC_TLA_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TIME_LATENCY, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= (uint8_t)~LSM303AGR_ACC_TLA_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_TIME_LATENCY, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ClickTimeLatency(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TIME_LATENCY, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_TLA_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_TLA_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_W_ClickTimeWindow(uint8_t newValue)
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_ACC_TW_POSITION; //mask	
  newValue &= LSM303AGR_ACC_TW_MASK; //coerce
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TIME_WINDOW, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= (uint8_t)~LSM303AGR_ACC_TW_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_ACC_WriteReg(LSM303AGR_ACC_TIME_WINDOW, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_R_ClickTimeWindow(uint8_t *value)
{
  LSM303AGR_OpStatus op_status;
  
  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_TIME_WINDOW, (uint8_t *)value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *value &= LSM303AGR_ACC_TW_MASK; //coerce	
  *value = *value >> LSM303AGR_ACC_TW_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_Get_Voltage_ADC(uint8_t *buff) 
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;
  
  numberOfByteForDimension=6/3;

  k=0;
  for (i=0; i<3;i++ ) 
  {
    for (j=0; j<numberOfByteForDimension;j++ )
    {	
      op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_OUT_ADC1_L+k, &buff[k]);
      if (op_status < LSM303AGR_OK) 
      {
        return op_status;
      }
      k++;	
    }
  }

  return LSM303AGR_OK; 
}

LSM303AGR_OpStatus LSM303AGR_ACC_Get_Raw_Acceleration(uint8_t *buff) 
{
  LSM303AGR_OpStatus op_status;
  
  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;
  
  numberOfByteForDimension=6/3;

  k=0;
  for (i=0; i<3;i++ ) 
  {
    for (j=0; j<numberOfByteForDimension;j++ )
    {	
		  op_status = LSM303AGR_ACC_ReadReg(LSM303AGR_ACC_OUT_X_L+k, &buff[k]);
      if (op_status < LSM303AGR_OK) 
      {
        return op_status;
      }
      k++;	
    }
  }

  return LSM303AGR_OK; 
}

/*
 * Following is the table of sensitivity values for each case.
 * Values are espressed in ug/digit.
 */
const long long LSM303AGR_ACC_Sensitivity_List[3][4] = {
    /* HR 12-bit */
    {
       980,	/* FS @2g */
       1950,	/* FS @4g */
       3900,	/* FS @8g */
      11720,	/* FS @16g */
    },

    /* Normal 10-bit */
    {
      3900,	/* FS @2g */
      7820,	/* FS @4g */
      15630,	/* FS @8g */
      46900,	/* FS @16g */
    },

    /* LP 8-bit */
    {
      15630,	/* FS @2g */
      31260,	/* FS @4g */
      62520,	/* FS @8g */
      187580,	/* FS @16g */
    },
};

/*
 * Values returned are espressed in mg.
 */
LSM303AGR_OpStatus LSM303AGR_ACC_Get_Acceleration(int *buff)
{
  LSM303AGR_OpStatus op_status;
  
  Type3Axis16bit_U raw_data_tmp;
  uint8_t op_mode = 0, fs_mode = 0, shift = 0;
  LSM303AGR_ACC_LPEN_t lp;
  LSM303AGR_ACC_HR_t hr;
  LSM303AGR_ACC_FS_t fs;

  /* Determine which operational mode the acc is set */
  op_status = LSM303AGR_ACC_R_HiRes(&hr);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  op_status = LSM303AGR_ACC_R_LOWPWR_EN(&lp);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  if (lp == LSM303AGR_ACC_LPEN_ENABLED && hr == LSM303AGR_ACC_HR_DISABLED) {
    /* op mode is LP 8-bit */
    op_mode = 2;
    shift = 8;
  } else if (lp == LSM303AGR_ACC_LPEN_DISABLED && hr == LSM303AGR_ACC_HR_DISABLED) {
    /* op mode is Normal 10-bit */
    op_mode = 1;
    shift = 6;
  } else if (lp == LSM303AGR_ACC_LPEN_DISABLED && hr == LSM303AGR_ACC_HR_ENABLED) {
    /* op mode is HR 12-bit */
    op_mode = 0;
    shift = 4;
  } else {
    return LSM303AGR_InvalidParameter;
  }
 
  /* Determine the Full Scale the acc is set */
  op_status = LSM303AGR_ACC_R_FullScale(&fs);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  switch (fs) {
  case LSM303AGR_ACC_FS_2G:
    fs_mode = 0;
    break;

  case LSM303AGR_ACC_FS_4G:
    fs_mode = 1;
    break;

  case LSM303AGR_ACC_FS_8G:
    fs_mode = 2;
    break;

  case LSM303AGR_ACC_FS_16G:
    fs_mode = 3;
    break;
  }

  /* Read out raw accelerometer samples */
  op_status = LSM303AGR_ACC_Get_Raw_Acceleration(raw_data_tmp.u8bit);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  /* Apply proper shift and sensitivity */
  buff[0] = ((raw_data_tmp.i16bit[0] >> shift) * LSM303AGR_ACC_Sensitivity_List[op_mode][fs_mode] + 500) / 1000;
  buff[1] = ((raw_data_tmp.i16bit[1] >> shift) * LSM303AGR_ACC_Sensitivity_List[op_mode][fs_mode] + 500) / 1000;
  buff[2] = ((raw_data_tmp.i16bit[2] >> shift) * LSM303AGR_ACC_Sensitivity_List[op_mode][fs_mode] + 500) / 1000;

  return LSM303AGR_OK;
}
