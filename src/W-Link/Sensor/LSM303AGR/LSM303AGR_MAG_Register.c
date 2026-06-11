/**
 *******************************************************************************
 * @file    LSM303AGR_MAG_driver.c
 * @author  MEMS Application Team
 * @version V1.1
 * @date    25-February-2016
 * @brief   LSM303AGR Magnetometer driver file
 *******************************************************************************
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

#include "LSM303AGR_MAG_Register_Def.h"
#include "LSM303AGR_MAG_Register.h"

typedef union{
	int16_t i16bit[3];
	uint8_t u8bit[6];
} Type3Axis16bit_U;	

LSM303AGR_OpResult LSM303AGR_MAG_ReadReg(uint8_t reg, uint8_t* p_data ) 
{
  return LSM303AGR_MAG_IO_Read(reg, p_data, 1);
}

LSM303AGR_OpResult LSM303AGR_MAG_WriteReg(uint8_t reg, uint8_t data ) 
{
  return LSM303AGR_MAG_IO_Write(reg, &data, 1);
}

void LSM303AGR_MAG_SwapHighLowByte(uint8_t *bufferToSwap, uint8_t numberOfByte, uint8_t dimension)
{
  uint8_t numberOfByteForDimension, i, j;
  uint8_t tempValue[10];
  
  numberOfByteForDimension=numberOfByte/dimension;
  
  for (i=0; i<dimension;i++ )
  {
	for (j=0; j<numberOfByteForDimension;j++ )
		tempValue[j]=bufferToSwap[j+i*numberOfByteForDimension];
	for (j=0; j<numberOfByteForDimension;j++ )
		*(bufferToSwap+i*(numberOfByteForDimension)+j)=*(tempValue+(numberOfByteForDimension-1)-j);
  } 
}

LSM303AGR_OpResult LSM303AGR_MAG_W_OFF_X_L(uint8_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_MAG_OFF_X_L_POSITION; //mask	
  newValue &= LSM303AGR_MAG_OFF_X_L_MASK; //coerce
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_X_REG_L, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= (uint8_t)~LSM303AGR_MAG_OFF_X_L_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_OFFSET_X_REG_L, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_OFF_X_L(uint8_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_X_REG_L, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_OFF_X_L_MASK; //coerce	
  *p_value = *p_value >> LSM303AGR_MAG_OFF_X_L_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_OFF_X_H(uint8_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_MAG_OFF_X_H_POSITION; //mask	
  newValue &= LSM303AGR_MAG_OFF_X_H_MASK; //coerce
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_X_REG_H, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= (uint8_t)~LSM303AGR_MAG_OFF_X_H_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_OFFSET_X_REG_H, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_OFF_X_H(uint8_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_X_REG_H, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_OFF_X_H_MASK; //coerce	
  *p_value = *p_value >> LSM303AGR_MAG_OFF_X_H_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_OFF_Y_L(uint8_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_MAG_OFF_Y_L_POSITION; //mask	
  newValue &= LSM303AGR_MAG_OFF_Y_L_MASK; //coerce
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_Y_REG_L, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= (uint8_t)~LSM303AGR_MAG_OFF_Y_L_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_OFFSET_Y_REG_L, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_OFF_Y_L(uint8_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_Y_REG_L, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_OFF_Y_L_MASK; //coerce	
  *p_value = *p_value >> LSM303AGR_MAG_OFF_Y_L_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_OFF_Y_H(uint8_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_MAG_OFF_Y_H_POSITION; //mask	
  newValue &= LSM303AGR_MAG_OFF_Y_H_MASK; //coerce
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_Y_REG_H, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= (uint8_t)~LSM303AGR_MAG_OFF_Y_H_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_OFFSET_Y_REG_H, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_OFF_Y_H(uint8_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_Y_REG_H, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_OFF_Y_H_MASK; //coerce	
  *p_value = *p_value >> LSM303AGR_MAG_OFF_Y_H_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_OFF_Z_L(uint8_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_MAG_OFF_Z_L_POSITION; //mask	
  newValue &= LSM303AGR_MAG_OFF_Z_L_MASK; //coerce
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_Z_REG_L, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= (uint8_t)~LSM303AGR_MAG_OFF_Z_L_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_OFFSET_Z_REG_L, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_OFF_Z_L(uint8_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_Z_REG_L, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_OFF_Z_L_MASK; //coerce	
  *p_value = *p_value >> LSM303AGR_MAG_OFF_Z_L_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_OFF_Z_H(uint8_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  newValue = newValue << LSM303AGR_MAG_OFF_Z_H_POSITION; //mask	
  newValue &= LSM303AGR_MAG_OFF_Z_H_MASK; //coerce
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_Z_REG_H, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= (uint8_t)~LSM303AGR_MAG_OFF_Z_H_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_OFFSET_Z_REG_H, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_Get_MagOff(uint16_t *magx_off, uint16_t *magy_off, uint16_t *magz_off)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t reg_l, reg_h;

  /* read mag_x_off */
  //LSM303AGR_MAG_R_OFF_X_L(&reg_l);
  //LSM303AGR_MAG_R_OFF_X_H(&reg_h);
  op_status = LSM303AGR_MAG_R_OFF_X_L(&reg_l);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }
  op_status = LSM303AGR_MAG_R_OFF_X_H(&reg_h);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *magx_off = ((reg_h << 8) & 0xff00) | reg_l;

  /* read mag_y_off */
  //LSM303AGR_MAG_R_OFF_Y_L(&reg_l);
  //LSM303AGR_MAG_R_OFF_Y_H(&reg_h);
  op_status = LSM303AGR_MAG_R_OFF_Y_L(&reg_l);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }
  op_status = LSM303AGR_MAG_R_OFF_Y_H(&reg_h);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *magy_off = ((reg_h << 8) & 0xff00) | reg_l;

  /* read mag_z_off */
  //LSM303AGR_MAG_R_OFF_Z_L(&reg_l);
  //LSM303AGR_MAG_R_OFF_Z_H(&reg_h);
  op_status = LSM303AGR_MAG_R_OFF_Z_L(&reg_l);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }
  op_status = LSM303AGR_MAG_R_OFF_Z_H(&reg_h);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *magz_off = ((reg_h << 8) & 0xff00) | reg_l;

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_Set_MagOff(uint16_t magx_off, uint16_t magy_off, uint16_t magz_off)
{
  LSM303AGR_OpResult op_status;
  
  /* write mag_x_off */
  //LSM303AGR_MAG_W_OFF_X_L(magx_off & 0xff);
  //LSM303AGR_MAG_W_OFF_X_H((magx_off >> 8) & 0xff);
  op_status = LSM303AGR_MAG_W_OFF_X_L(magx_off & 0xff);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }
  op_status = LSM303AGR_MAG_W_OFF_X_H((magx_off >> 8) & 0xff);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  /* write mag_y_off */
  //LSM303AGR_MAG_W_OFF_Y_L(magy_off & 0xff);
  //LSM303AGR_MAG_W_OFF_Y_H((magy_off >> 8) & 0xff);
  op_status = LSM303AGR_MAG_W_OFF_Y_L(magy_off & 0xff);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }
  op_status = LSM303AGR_MAG_W_OFF_Y_H((magy_off >> 8) & 0xff);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  /* write mag_z_off */
  //LSM303AGR_MAG_W_OFF_Z_L(magz_off & 0xff);
  //LSM303AGR_MAG_W_OFF_Z_H((magz_off >> 8) & 0xff);
  op_status = LSM303AGR_MAG_W_OFF_Z_L(magz_off & 0xff);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }
  op_status = LSM303AGR_MAG_W_OFF_Z_H((magz_off >> 8) & 0xff);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_OFF_Z_H(uint8_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OFFSET_Z_REG_H, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_OFF_Z_H_MASK; //coerce	
  *p_value = *p_value >> LSM303AGR_MAG_OFF_Z_H_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_WHO_AM_I(uint8_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_WHO_AM_I_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_WHO_AM_I_MASK; //coerce	
  *p_value = *p_value >> LSM303AGR_MAG_WHO_AM_I_POSITION; //mask	

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_MD(LSM303AGR_MAG_MD_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_A, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_MD_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_A, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_MD(LSM303AGR_MAG_MD_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_A, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_MD_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_ODR(LSM303AGR_MAG_ODR_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_A, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_ODR_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_A, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_ODR(LSM303AGR_MAG_ODR_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_A, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_ODR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_LP(LSM303AGR_MAG_LP_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_A, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_LP_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_A, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_LP(LSM303AGR_MAG_LP_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_A, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_LP_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_SOFT_RST(LSM303AGR_MAG_SOFT_RST_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_A, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_SOFT_RST_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_A, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_SOFT_RST(LSM303AGR_MAG_SOFT_RST_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_A, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_SOFT_RST_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_LPF(LSM303AGR_MAG_LPF_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_B, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_LPF_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_B, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_LPF(LSM303AGR_MAG_LPF_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_B, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_LPF_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_OFF_CANC(LSM303AGR_MAG_OFF_CANC_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_B, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_OFF_CANC_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_B, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_OFF_CANC(LSM303AGR_MAG_OFF_CANC_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_B, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_OFF_CANC_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_SET_FREQ(LSM303AGR_MAG_SET_FREQ_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_B, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_SET_FREQ_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_B, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_SET_FREQ(LSM303AGR_MAG_SET_FREQ_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_B, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_SET_FREQ_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_INT_ON_DATAOFF(LSM303AGR_MAG_INT_ON_DATAOFF_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_B, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_INT_ON_DATAOFF_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_B, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_INT_ON_DATAOFF(LSM303AGR_MAG_INT_ON_DATAOFF_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_B, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_INT_ON_DATAOFF_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_INT_MAG(LSM303AGR_MAG_INT_MAG_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_INT_MAG_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_C, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_INT_MAG(LSM303AGR_MAG_INT_MAG_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_INT_MAG_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_ST(LSM303AGR_MAG_ST_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_ST_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_C, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_ST(LSM303AGR_MAG_ST_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_ST_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_BLE(LSM303AGR_MAG_BLE_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_BLE_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_C, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_BLE(LSM303AGR_MAG_BLE_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_BLE_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_BDU(LSM303AGR_MAG_BDU_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_BDU_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_C, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_BDU(LSM303AGR_MAG_BDU_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_BDU_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_I2C_DIS(LSM303AGR_MAG_I2C_DIS_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_I2C_DIS_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_C, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_I2C_DIS(LSM303AGR_MAG_I2C_DIS_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_I2C_DIS_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_INT_MAG_PIN(LSM303AGR_MAG_INT_MAG_PIN_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_INT_MAG_PIN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_CFG_REG_C, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_INT_MAG_PIN(LSM303AGR_MAG_INT_MAG_PIN_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_CFG_REG_C, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_INT_MAG_PIN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_IEN(LSM303AGR_MAG_IEN_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_IEN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_INT_CTRL_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_IEN(LSM303AGR_MAG_IEN_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_IEN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_IEL(LSM303AGR_MAG_IEL_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_IEL_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_INT_CTRL_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_IEL(LSM303AGR_MAG_IEL_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_IEL_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_IEA(LSM303AGR_MAG_IEA_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_IEA_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_INT_CTRL_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_IEA(LSM303AGR_MAG_IEA_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_IEA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_ZIEN(LSM303AGR_MAG_ZIEN_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_ZIEN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_INT_CTRL_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_ZIEN(LSM303AGR_MAG_ZIEN_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_ZIEN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_YIEN(LSM303AGR_MAG_YIEN_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_YIEN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_INT_CTRL_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_YIEN(LSM303AGR_MAG_YIEN_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_YIEN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_W_XIEN(LSM303AGR_MAG_XIEN_t newValue)
{
  LSM303AGR_OpResult op_status;
  
  uint8_t value;

  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, &value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  value &= ~LSM303AGR_MAG_XIEN_MASK; 
  value |= newValue;
  
  op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_INT_CTRL_REG, value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_XIEN(LSM303AGR_MAG_XIEN_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_CTRL_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_XIEN_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_INT(LSM303AGR_MAG_INT_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_SOURCE_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_INT_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_MROI(LSM303AGR_MAG_MROI_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_SOURCE_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_MROI_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_N_TH_S_Z(LSM303AGR_MAG_N_TH_S_Z_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_SOURCE_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_N_TH_S_Z_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_N_TH_S_Y(LSM303AGR_MAG_N_TH_S_Y_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_SOURCE_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_N_TH_S_Y_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_N_TH_S_X(LSM303AGR_MAG_N_TH_S_X_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_SOURCE_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_N_TH_S_X_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_P_TH_S_Z(LSM303AGR_MAG_P_TH_S_Z_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_SOURCE_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_P_TH_S_Z_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_P_TH_S_Y(LSM303AGR_MAG_P_TH_S_Y_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_SOURCE_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_P_TH_S_Y_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_P_TH_S_X(LSM303AGR_MAG_P_TH_S_X_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_SOURCE_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_P_TH_S_X_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_XDA(LSM303AGR_MAG_XDA_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_STATUS_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_XDA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_YDA(LSM303AGR_MAG_YDA_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_STATUS_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_YDA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_ZDA(LSM303AGR_MAG_ZDA_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_STATUS_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_ZDA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_ZYXDA(LSM303AGR_MAG_ZYXDA_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_STATUS_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_ZYXDA_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_XOR(LSM303AGR_MAG_XOR_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_STATUS_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_XOR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_YOR(LSM303AGR_MAG_YOR_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_STATUS_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_YOR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_ZOR(LSM303AGR_MAG_ZOR_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_STATUS_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_ZOR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_R_ZYXOR(LSM303AGR_MAG_ZYXOR_t *p_value)
{
  LSM303AGR_OpResult op_status;
  
  op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_STATUS_REG, (uint8_t *)p_value);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  *p_value &= LSM303AGR_MAG_ZYXOR_MASK; //mask

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_Get_Raw_Magnetic(uint8_t *buff) 
{
  LSM303AGR_OpResult op_status;
  
  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;
  
  numberOfByteForDimension=6/3;

  k=0;
  for (i=0; i<3;i++ ) 
  {
	for (j=0; j<numberOfByteForDimension;j++ )
	{	
		op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_OUTX_L_REG+k, &buff[k]);
        if (op_status < LSM303AGR_OK) 
        {
            return op_status;
        }
		k++;	
	}
  }

  return LSM303AGR_OK; 
}

#define LSM303AGR_MAG_SENSITIVITY	15/10

LSM303AGR_OpResult LSM303AGR_MAG_Get_Magnetic(int *buff)
{
  LSM303AGR_OpResult op_status;
  
  Type3Axis16bit_U raw_data_tmp;

  /* Read out raw magnetometer samples */
  op_status = LSM303AGR_MAG_Get_Raw_Magnetic(raw_data_tmp.u8bit);
  if (op_status < LSM303AGR_OK) 
  {
    return op_status;
  }

  /* Applysensitivity */
  buff[0] = raw_data_tmp.i16bit[0] * LSM303AGR_MAG_SENSITIVITY;
  buff[1] = raw_data_tmp.i16bit[1] * LSM303AGR_MAG_SENSITIVITY;
  buff[2] = raw_data_tmp.i16bit[2] * LSM303AGR_MAG_SENSITIVITY;

  return LSM303AGR_OK;
}

LSM303AGR_OpResult LSM303AGR_MAG_Get_IntThreshld(uint8_t *buff) 
{
  LSM303AGR_OpResult op_status;
  
  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;
  
  numberOfByteForDimension=2/1;

  k=0;
  for (i=0; i<1;i++ ) 
  {
	for (j=0; j<numberOfByteForDimension;j++ )
	{	
		op_status = LSM303AGR_MAG_ReadReg(LSM303AGR_MAG_INT_THS_L_REG+k, &buff[k]);
        if (op_status < LSM303AGR_OK) 
        {
            return op_status;
        }
		k++;	
	}
  }

  return LSM303AGR_OK; 
}

LSM303AGR_OpResult LSM303AGR_MAG_Set_IntThreshld(uint8_t *buff) 
{
  LSM303AGR_OpResult op_status;
  
  uint8_t i, j, k;
  uint8_t numberOfByteForDimension;
  
  numberOfByteForDimension=2/1;

  k=0;
  for (i=0; i<1;i++ ) 
  {
	for (j=0; j<numberOfByteForDimension;j++ )
	{	
		op_status = LSM303AGR_MAG_WriteReg(LSM303AGR_MAG_INT_THS_L_REG+k, buff[k]);
        if (op_status < LSM303AGR_OK) 
        {
            return op_status;
        }
		k++;	
	}
  }

  return LSM303AGR_OK; 
}