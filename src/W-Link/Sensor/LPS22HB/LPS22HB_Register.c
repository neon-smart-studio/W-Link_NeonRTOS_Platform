/**
 *******************************************************************************
 * @file    LPS22HB_Driver.c
 * @author  HESA Application Team
 * @version V1.1
 * @date    10-August-2016
 * @brief   LPS22HB driver file
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
 * Based on STMicroelectronics LPS22HB driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "LPS22HB_Def.h"

#include "LPS22HB_Register_Def.h"
#include "LPS22HB_Register.h"

LPS22HB_OpResult LPS22HB_ReadReg( uint8_t RegAddr, uint16_t NumByteToRead, uint8_t *p_data )
{
  LPS22HB_OpResult op_status;
  
  int i = 0;

  for (i = 0; i < NumByteToRead; i++)
  {
    op_status = LPS22HB_IO_Read(RegAddr + i, &p_data[i], 1 );
    if(op_status < LPS22HB_OK)
    {
      return op_status;
    }
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_WriteReg( uint8_t RegAddr, uint16_t NumByteToWrite, uint8_t *p_data )
{
  LPS22HB_OpResult op_status;
  
  int i = 0;

  for (i = 0; i < NumByteToWrite; i++)
  {
    op_status = LPS22HB_IO_Write(RegAddr + i, &p_data[i], 1 );
    if(op_status < LPS22HB_OK)
    {
      return op_status;
    }
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_DeviceID(uint8_t* deviceid)
{
  LPS22HB_OpResult op_status;
  
  op_status = LPS22HB_ReadReg(LPS22HB_WHO_AM_I_REG, 1, deviceid);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_DriverVersion(LPS22HB_DriverVersion_st *Version)
{
  Version->Major = LPS22HB_DriverVersion_Major;
  Version->Minor = LPS22HB_DriverVersion_Minor;
  Version->Point = LPS22HB_DriverVersion_Point;

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_PowerMode(LPS22HB_PowerMode_et mode)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_RES_CONF_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_LCEN_MASK;
  tmp |= (uint8_t)mode;

  op_status = LPS22HB_WriteReg(LPS22HB_RES_CONF_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_PowerMode(LPS22HB_PowerMode_et* mode)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_RES_CONF_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *mode = (LPS22HB_PowerMode_et)(tmp & LPS22HB_LCEN_MASK);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_Odr(LPS22HB_Odr_et odr)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_ODR_MASK;
  tmp |= (uint8_t)odr;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_Odr(LPS22HB_Odr_et* odr)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *odr = (LPS22HB_Odr_et)(tmp & LPS22HB_ODR_MASK);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_LowPassFilter(LPS22HB_State_et state)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_LPFP_MASK;
  tmp |= ((uint8_t)state)<<LPS22HB_LPFP_BIT;


  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }


  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_LowPassFilterCutoff(LPS22HB_LPF_Cutoff_et cutoff){

  LPS22HB_OpResult op_status;

  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_LPFP_CUTOFF_MASK;
  tmp |= (uint8_t)cutoff;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_Bdu(LPS22HB_Bdu_et bdu)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_BDU_MASK;
  tmp |= ((uint8_t)bdu);

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_Bdu(LPS22HB_Bdu_et* bdu)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *bdu = (LPS22HB_Bdu_et)(tmp & LPS22HB_BDU_MASK);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_SpiInterface(LPS22HB_SPIMode_et spimode)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_SIM_MASK;
  tmp |= (uint8_t)spimode;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_ClockTreeConfifuration(LPS22HB_CTE_et mode)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CLOCK_TREE_CONFIGURATION, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_CTE_MASK;
  tmp |= (uint8_t)mode;

  op_status = LPS22HB_WriteReg(LPS22HB_CLOCK_TREE_CONFIGURATION, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_SpiInterface(LPS22HB_SPIMode_et* spimode)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *spimode = (LPS22HB_SPIMode_et)(tmp & LPS22HB_SIM_MASK);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_SwReset()
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp |= (0x01<<LPS22HB_SW_RESET_BIT);

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_MemoryBoot()
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp |= (0x01<<LPS22HB_BOOT_BIT);

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_SwResetAndMemoryBoot()
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp |= ((0x01<<LPS22HB_SW_RESET_BIT) | (0x01<<LPS22HB_BOOT_BIT));

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_FifoModeUse(LPS22HB_State_et status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_FIFO_EN_MASK;
  tmp |= ((uint8_t)status)<<LPS22HB_FIFO_EN_BIT;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_FifoWatermarkLevelUse(LPS22HB_State_et status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_WTM_EN_MASK;
  tmp |= ((uint8_t)status)<<LPS22HB_WTM_EN_BIT;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_AutomaticIncrementRegAddress(LPS22HB_State_et status){

  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_ADD_INC_MASK;
  tmp |= (((uint8_t)status)<<LPS22HB_ADD_INC_BIT);

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_I2C(LPS22HB_State_et statei2c)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /*Reset Bit->I2C Enabled*/
  tmp &= ~LPS22HB_I2C_MASK;
  tmp|=((uint8_t)~statei2c)<<LPS22HB_I2C_BIT;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_StartOneShotMeasurement()
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Set the one shot bit */
  /* Once the measurement is done, one shot bit will self-clear*/
  tmp |= LPS22HB_ONE_SHOT_MASK;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;

}

LPS22HB_OpResult LPS22HB_Set_InterruptActiveLevel(LPS22HB_InterruptActiveLevel_et mode)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_INT_H_L_MASK;
  tmp |= ((uint8_t)mode);

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_InterruptOutputType(LPS22HB_OutputType_et output)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_PP_OD_MASK;
  tmp |= (uint8_t)output;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_InterruptControlConfig(LPS22HB_OutputSignalConfig_et config)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~(LPS22HB_INT_S12_MASK);
  tmp |= (uint8_t)config;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_DRDYInterrupt(LPS22HB_State_et status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_DRDY_MASK;
  tmp |= ((uint8_t)status)<<LPS22HB_DRDY_BIT;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_FIFO_OVR_Interrupt(LPS22HB_State_et status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_FIFO_OVR_MASK;
  tmp |= ((uint8_t)status)<<LPS22HB_FIFO_OVR_BIT;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_FIFO_FTH_Interrupt(LPS22HB_State_et status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_FIFO_FTH_MASK;
  tmp |= ((uint8_t)status)<<LPS22HB_FIFO_FTH_BIT;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_FIFO_FULL_Interrupt(LPS22HB_State_et status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_FIFO_FULL_MASK;
  tmp |= ((uint8_t)status)<<LPS22HB_FIFO_FULL_BIT;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_AutoRifP()
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp |= ((uint8_t)LPS22HB_AUTORIFP_MASK);

  op_status = LPS22HB_WriteReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_ResetAutoRifP()
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp |= ((uint8_t)LPS22HB_RESET_ARP_MASK);

  op_status = LPS22HB_WriteReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_AutoZeroFunction()
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp |= LPS22HB_AUTOZERO_MASK;

  op_status = LPS22HB_WriteReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_ResetAutoZeroFunction()
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Set the RESET_AZ bit*/
  /* RESET_AZ is self cleared*/
  tmp |= LPS22HB_RESET_AZ_MASK;

  op_status = LPS22HB_WriteReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }


  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_InterruptDifferentialGeneration(LPS22HB_State_et diff_en)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_DIFF_EN_MASK;
  tmp |= ((uint8_t)diff_en)<<LPS22HB_DIFF_EN_BIT;

  op_status = LPS22HB_WriteReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_InterruptDifferentialGeneration(LPS22HB_State_et* diff_en)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *diff_en= (LPS22HB_State_et)((tmp & LPS22HB_DIFF_EN_MASK)>>LPS22HB_DIFF_EN_BIT);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_LatchInterruptRequest(LPS22HB_State_et status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_LIR_MASK;
  tmp |= (((uint8_t)status)<<LPS22HB_LIR_BIT);

  op_status = LPS22HB_WriteReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_PLE(LPS22HB_State_et status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_PLE_MASK;
  tmp |= (((uint8_t)status)<<LPS22HB_PLE_BIT);

  op_status = LPS22HB_WriteReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_PHE(LPS22HB_State_et status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_PHE_MASK;
  tmp |= (((uint8_t)status)<<LPS22HB_PHE_BIT);

  op_status = LPS22HB_WriteReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_InterruptDifferentialEventStatus(LPS22HB_InterruptDiffStatus_st* interruptsource)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_SOURCE_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  interruptsource->PHD = (uint8_t)(tmp & LPS22HB_PH_MASK);
  interruptsource->PLD = (uint8_t)((tmp & LPS22HB_PL_MASK)>>LPS22HB_PL_BIT);
  interruptsource->IA = (uint8_t)((tmp & LPS22HB_IA_MASK)>>LPS22HB_IA_BIT);
  interruptsource->BOOT= (uint8_t)((tmp & LPS22HB_BOOT_STATUS_MASK)>>LPS22HB_BOOT_STATUS_BIT);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_DataStatus(LPS22HB_DataStatus_st* datastatus)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_STATUS_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  datastatus->PressDataAvailable = (uint8_t)(tmp & LPS22HB_PDA_MASK);
  datastatus->TempDataAvailable = (uint8_t)((tmp & LPS22HB_TDA_MASK)>>LPS22HB_PDA_BIT);
  datastatus->TempDataOverrun = (uint8_t)((tmp & LPS22HB_TOR_MASK)>>LPS22HB_TOR_BIT);
  datastatus->PressDataOverrun = (uint8_t)((tmp & LPS22HB_POR_MASK)>>LPS22HB_POR_BIT);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_RawPressure(int32_t *raw_press)
{
  LPS22HB_OpResult op_status;
  
  uint8_t buffer[3];
  uint32_t tmp = 0;
  uint8_t i;

  op_status = LPS22HB_ReadReg(LPS22HB_PRESS_OUT_XL_REG, 3, buffer);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Build the raw data */
  for(i=0; i<3; i++)
    tmp |= (((uint32_t)buffer[i]) << (8*i));

  /* convert the 2's complement 24 bit to 2's complement 32 bit */
  if(tmp & 0x00800000)
    tmp |= 0xFF000000;

  *raw_press = ((int32_t)tmp);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_Pressure(int32_t* Pout)
{
  LPS22HB_OpResult op_status;
  
  int32_t raw_press;

  op_status = LPS22HB_Get_RawPressure(&raw_press);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *Pout = (raw_press*100)/4096;

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_RawTemperature(int16_t* raw_data)
{
  LPS22HB_OpResult op_status;
  
  uint8_t buffer[2];
  uint16_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_TEMP_OUT_L_REG, 2, buffer);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Build the raw tmp */
  tmp = (((uint16_t)buffer[1]) << 8) + (uint16_t)buffer[0];

  *raw_data = ((int16_t)tmp);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_Temperature(int16_t* Tout)
{
  LPS22HB_OpResult op_status;
  
  int16_t raw_data;

  op_status = LPS22HB_Get_RawTemperature(&raw_data);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *Tout = (raw_data*10)/100;

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_PressureThreshold(int16_t* P_ths)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tempReg[2];

  op_status = LPS22HB_ReadReg(LPS22HB_THS_P_LOW_REG, 2, tempReg);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *P_ths= (((((uint16_t)tempReg[1])<<8) + tempReg[0])/16);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_PressureThreshold(int16_t P_ths)
{
  LPS22HB_OpResult op_status;
  
  uint8_t buffer[2];

  buffer[0] = (uint8_t)(16 * P_ths);
  buffer[1] = (uint8_t)(((uint16_t)(16 * P_ths))>>8);

  op_status = LPS22HB_WriteReg(LPS22HB_THS_P_LOW_REG, 2, buffer);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_FifoMode(LPS22HB_FifoMode_et fifomode)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_FIFO_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_FIFO_MODE_MASK;
  tmp |= (uint8_t)fifomode;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_FIFO_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_FifoMode(LPS22HB_FifoMode_et* fifomode)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_FIFO_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= LPS22HB_FIFO_MODE_MASK;
  *fifomode = (LPS22HB_FifoMode_et)tmp;

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_FifoWatermarkLevel(uint8_t wtmlevel)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_FIFO_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  tmp &= ~LPS22HB_WTM_POINT_MASK;
  tmp |= wtmlevel;

  op_status = LPS22HB_WriteReg(LPS22HB_CTRL_FIFO_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_FifoWatermarkLevel(uint8_t *wtmlevel)
{
  LPS22HB_OpResult op_status;
  
  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_FIFO_REG, 1, wtmlevel);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  *wtmlevel &= LPS22HB_WTM_POINT_MASK;

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_FifoStatus(LPS22HB_FifoStatus_st* status)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_STATUS_FIFO_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  status->FIFO_FTH = (uint8_t)((tmp & LPS22HB_FTH_FIFO_MASK)>>LPS22HB_FTH_FIFO_BIT);
  status->FIFO_OVR=(uint8_t)((tmp & LPS22HB_OVR_FIFO_MASK)>>LPS22HB_OVR_FIFO_BIT);
  status->FIFO_LEVEL = (uint8_t)(tmp & LPS22HB_LEVEL_FIFO_MASK);

  if(status->FIFO_LEVEL ==LPS22HB_FIFO_EMPTY)
    status->FIFO_EMPTY=0x01;
  else
    status->FIFO_EMPTY=0x00;

  if (status->FIFO_LEVEL ==LPS22HB_FIFO_FULL)
     status->FIFO_FULL=0x01;
  else
    status->FIFO_FULL=0x00;


  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_PressureOffsetValue(int16_t *pressoffset)
{
  LPS22HB_OpResult op_status;
  
  uint8_t buffer[2];
  int16_t raw_press;

  op_status = LPS22HB_ReadReg(LPS22HB_RPDS_L_REG, 2, buffer);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  raw_press = (int16_t)((((uint16_t)buffer[1]) << 8) + (uint16_t)buffer[0]);

  *pressoffset = (raw_press*100)/4096;

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_ReferencePressure(int32_t* RefP)
{
  LPS22HB_OpResult op_status;
  
  uint8_t buffer[3];
  uint32_t tempVal=0;
  int32_t raw_press;
  uint8_t i;

  op_status = LPS22HB_ReadReg(LPS22HB_REF_P_XL_REG, 3, buffer);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Build the raw data */
  for(i=0; i<3; i++)
    tempVal |= (((uint32_t)buffer[i]) << (8*i));

  /* convert the 2's complement 24 bit to 2's complement 32 bit */
  if(tempVal & 0x00800000)
    tempVal |= 0xFF000000;

  raw_press =((int32_t)tempVal);
  *RefP = (raw_press*100)/4096;


  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_IsMeasurementCompleted(uint8_t* Is_Measurement_Completed)
{
  LPS22HB_OpResult op_status;
  
  uint8_t tmp;
  LPS22HB_DataStatus_st datastatus;

  op_status = LPS22HB_ReadReg(LPS22HB_STATUS_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  datastatus.TempDataAvailable=(uint8_t)((tmp&LPS22HB_TDA_MASK)>>LPS22HB_TDA_BIT);
  datastatus.PressDataAvailable= (uint8_t)(tmp&LPS22HB_PDA_MASK);

  *Is_Measurement_Completed=(uint8_t)((datastatus.PressDataAvailable) & (datastatus.TempDataAvailable));

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_Measurement(LPS22HB_MeasureTypeDef_st *Measurement_Value)
{
  LPS22HB_OpResult op_status;
  
  int16_t Tout;
  int32_t Pout;

  op_status = LPS22HB_Get_Temperature(&Tout);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  Measurement_Value->Tout=Tout;

  op_status = LPS22HB_Get_Pressure(&Pout);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  Measurement_Value->Pout=Pout;

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_InitConfig()
{
  LPS22HB_OpResult op_status;
  
  LPS22HB_ConfigTypeDef_st pLPS22HBInit;

  /* Make LPS22HB Reset and Reboot */
  op_status = LPS22HB_SwResetAndMemoryBoot();
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  pLPS22HBInit.PowerMode=LPS22HB_LowPower;
  pLPS22HBInit.OutputDataRate=LPS22HB_ODR_25HZ;
  pLPS22HBInit.LowPassFilter=LPS22HB_DISABLE;
  pLPS22HBInit.LPF_Cutoff=LPS22HB_ODR_9;
  pLPS22HBInit.BDU=LPS22HB_BDU_NO_UPDATE;
  pLPS22HBInit.IfAddInc=LPS22HB_ENABLE; //default
  pLPS22HBInit.Sim= LPS22HB_SPI_4_WIRE;

  /* Set Generic Configuration*/
  op_status = LPS22HB_Set_GenericConfig(&pLPS22HBInit);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Clear_InitConfig()
{
  LPS22HB_OpResult op_status;
  
  LPS22HB_MeasureTypeDef_st Measurement_Value;

  /* Make LPS22HB Reset and Reboot */
  op_status = LPS22HB_SwResetAndMemoryBoot();
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Dump of data output */
  op_status = LPS22HB_Get_Measurement(&Measurement_Value);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_GenericConfig(LPS22HB_ConfigTypeDef_st* pxLPS22HBInit)
{
  LPS22HB_OpResult op_status;
  

  /* Enable Low Current Mode (low Power) or Low Noise Mode*/
   op_status = LPS22HB_Set_PowerMode(pxLPS22HBInit->PowerMode);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Init the Output Data Rate*/
  op_status = LPS22HB_Set_Odr(pxLPS22HBInit->OutputDataRate);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* BDU bit is used to inhibit the output registers update between the reading of upper and
  lower register parts. In default mode (BDU = �0�), the lower and upper register parts are
  updated continuously. If it is not sure to read faster than output data rate, it is recommended
  to set BDU bit to �1�. In this way, after the reading of the lower (upper) register part, the
  content of that output registers is not updated until the upper (lower) part is read too.
  This feature avoids reading LSB and MSB related to different samples.*/

  op_status = LPS22HB_Set_Bdu(pxLPS22HBInit->BDU);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /*Enable/Disale low-pass filter on LPS22HB pressure data*/
  op_status = LPS22HB_Set_LowPassFilter(pxLPS22HBInit->LowPassFilter);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

   /* Set low-pass filter cutoff configuration*/
  op_status = LPS22HB_Set_LowPassFilterCutoff(pxLPS22HBInit->LPF_Cutoff);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* SIM bit selects the SPI serial interface mode.*/
  /* This feature has effect only if SPI interface is used*/

    op_status = LPS22HB_Set_SpiInterface(pxLPS22HBInit->Sim);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /*Enable or Disable the Automatic increment register address during a multiple byte access with a serial interface (I2C or SPI)*/
  op_status = LPS22HB_Set_AutomaticIncrementRegAddress(pxLPS22HBInit->IfAddInc);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }


  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_GenericConfig(LPS22HB_ConfigTypeDef_st* pxLPS22HBInit)
{
  LPS22HB_OpResult op_status;
  

  uint8_t tmp;

  /*Read LPS22HB_RES_CONF_REG*/
  op_status = LPS22HB_Get_PowerMode(&pxLPS22HBInit->PowerMode);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /*Read LPS22HB_CTRL_REG1*/
  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG1, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  pxLPS22HBInit->OutputDataRate= (LPS22HB_Odr_et)(tmp & LPS22HB_ODR_MASK);
  pxLPS22HBInit->BDU=(LPS22HB_Bdu_et)(tmp & LPS22HB_BDU_MASK);
  pxLPS22HBInit->Sim=(LPS22HB_SPIMode_et)(tmp& LPS22HB_SIM_MASK);
  pxLPS22HBInit->LowPassFilter=(LPS22HB_State_et)((tmp& LPS22HB_LPFP_MASK)>>LPS22HB_LPFP_BIT);
  pxLPS22HBInit->LPF_Cutoff=(LPS22HB_LPF_Cutoff_et)(tmp& LPS22HB_LPFP_CUTOFF_MASK);

  /*Read LPS22HB_CTRL_REG2*/
  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  pxLPS22HBInit->IfAddInc=(LPS22HB_State_et)((tmp& LPS22HB_ADD_INC_MASK)>>LPS22HB_ADD_INC_BIT);

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_InterruptConfig(LPS22HB_InterruptTypeDef_st* pLPS22HBInt)
{
  LPS22HB_OpResult op_status;
  
  /* Enable Low Current Mode (low Power) or Low Noise Mode*/
  op_status = LPS22HB_Set_InterruptActiveLevel(pLPS22HBInt->INT_H_L);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Push-pull/open drain selection on interrupt pads.*/
  op_status = LPS22HB_Set_InterruptOutputType(pLPS22HBInt->PP_OD);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Set Data signal on INT pad control bits.*/
  op_status = LPS22HB_Set_InterruptControlConfig(pLPS22HBInt->OutputSignal_INT);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Enable/Disable Data-ready signal on INT_DRDY pin. */
  op_status = LPS22HB_Set_DRDYInterrupt(pLPS22HBInt->DRDY);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Enable/Disable FIFO overrun interrupt on INT_DRDY pin. */
  op_status = LPS22HB_Set_FIFO_OVR_Interrupt(pLPS22HBInt->FIFO_OVR);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Enable/Disable FIFO Treshold interrupt on INT_DRDY pin. */
  op_status = LPS22HB_Set_FIFO_FTH_Interrupt(pLPS22HBInt->FIFO_FTH);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Enable/Disable FIFO FULL interrupt on INT_DRDY pin. */
  op_status = LPS22HB_Set_FIFO_FULL_Interrupt(pLPS22HBInt->FIFO_FULL);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Latch Interrupt request to the INT_SOURCE register. */
  op_status = LPS22HB_LatchInterruptRequest(pLPS22HBInt->LatchIRQ);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /* Set the threshold value  used for pressure interrupt generation (hPA). */
  op_status = LPS22HB_Set_PressureThreshold(pLPS22HBInt->THS_threshold);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

   /*Enable/Disable  AutoRifP function */
  if(pLPS22HBInt->AutoRifP==LPS22HB_ENABLE){
    op_status = LPS22HB_Set_AutoRifP();
  }
  else{
    op_status = LPS22HB_ResetAutoRifP();
  }
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /*Enable/Disable AutoZero function*/
  if(pLPS22HBInt->AutoZero==LPS22HB_ENABLE){
    op_status = LPS22HB_Set_AutoZeroFunction();
  }
  else{
    op_status = LPS22HB_ResetAutoZeroFunction();
  };
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

   if(pLPS22HBInt->OutputSignal_INT==LPS22HB_P_HIGH)
   {
    /* Enable\Disable Interrupt Generation on differential pressure high event*/
      op_status = LPS22HB_Set_PHE(LPS22HB_ENABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
       op_status = LPS22HB_Set_InterruptDifferentialGeneration(LPS22HB_ENABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
   }
   else  if(pLPS22HBInt->OutputSignal_INT==LPS22HB_P_LOW)
      {
    /* Enable Interrupt Generation on differential pressure Loe event*/
      op_status = LPS22HB_Set_PLE(LPS22HB_ENABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
       op_status = LPS22HB_Set_InterruptDifferentialGeneration(LPS22HB_ENABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
   }
    else  if(pLPS22HBInt->OutputSignal_INT==LPS22HB_P_LOW_HIGH)
    {
      /* Enable Interrupt Generation on differential pressure high event*/
      op_status = LPS22HB_Set_PHE(LPS22HB_ENABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
    /* Enable\Disable Interrupt Generation on differential pressure Loe event*/
      op_status = LPS22HB_Set_PLE(LPS22HB_ENABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
       op_status = LPS22HB_Set_InterruptDifferentialGeneration(LPS22HB_ENABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
   }
   else
   {
      op_status = LPS22HB_Set_InterruptDifferentialGeneration(LPS22HB_DISABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
      /* Disable Interrupt Generation on differential pressure High event*/
      op_status = LPS22HB_Set_PHE(LPS22HB_DISABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
     /* Disable Interrupt Generation on differential pressure Low event*/
      op_status = LPS22HB_Set_PLE(LPS22HB_DISABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
   }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_InterruptConfig(LPS22HB_InterruptTypeDef_st* pLPS22HBInt)
{
  LPS22HB_OpResult op_status;
  
   uint8_t tmp;

  /*Read LPS22HB_CTRL_REG3*/
  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG3, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  pLPS22HBInt->INT_H_L= (LPS22HB_InterruptActiveLevel_et)(tmp & LPS22HB_INT_H_L_MASK);
  pLPS22HBInt->PP_OD=(LPS22HB_OutputType_et)(tmp & LPS22HB_PP_OD_MASK);
  pLPS22HBInt->OutputSignal_INT=(LPS22HB_OutputSignalConfig_et)(tmp& LPS22HB_INT_S12_MASK);
  pLPS22HBInt->DRDY=(LPS22HB_State_et)((tmp& LPS22HB_DRDY_MASK)>>LPS22HB_DRDY_BIT);
  pLPS22HBInt->FIFO_OVR=(LPS22HB_State_et)((tmp& LPS22HB_FIFO_OVR_MASK)>>LPS22HB_FIFO_OVR_BIT);
  pLPS22HBInt->FIFO_FTH=(LPS22HB_State_et)((tmp& LPS22HB_FIFO_FTH_MASK)>>LPS22HB_FIFO_FTH_BIT);
  pLPS22HBInt->FIFO_FULL=(LPS22HB_State_et)((tmp& LPS22HB_FIFO_FULL_MASK)>>LPS22HB_FIFO_FULL_BIT);

  /*Read LPS22HB_INTERRUPT_CFG_REG*/
  op_status = LPS22HB_ReadReg(LPS22HB_INTERRUPT_CFG_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  pLPS22HBInt->LatchIRQ= (LPS22HB_State_et)((tmp& LPS22HB_LIR_MASK)>>LPS22HB_LIR_BIT);

  op_status = LPS22HB_Get_PressureThreshold(&pLPS22HBInt->THS_threshold);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  //AutoRifP and Autozero are self clear //
  pLPS22HBInt->AutoRifP=LPS22HB_DISABLE;
  pLPS22HBInt->AutoZero=LPS22HB_DISABLE;

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Set_FifoConfig(LPS22HB_FIFOTypeDef_st* pLPS22HBFIFO)
{
  LPS22HB_OpResult op_status;

   if(pLPS22HBFIFO->FIFO_MODE == LPS22HB_FIFO_BYPASS_MODE) {
    /* FIFO Disable-> FIFO_EN bit=0 in CTRL_REG2*/
    op_status = LPS22HB_Set_FifoModeUse(LPS22HB_DISABLE);
    if(op_status < LPS22HB_OK)
    {
      return op_status;
    }
    /* Force->Disable FIFO Watermark Level Use*/
     op_status = LPS22HB_Set_FifoWatermarkLevelUse(LPS22HB_DISABLE);
    if(op_status < LPS22HB_OK)
    {
      return op_status;
    }

    /* Force->Disable FIFO Treshold interrupt on INT_DRDY pin. */
     op_status = LPS22HB_Set_FIFO_FTH_Interrupt(LPS22HB_DISABLE);
    if(op_status < LPS22HB_OK)
    {
      return op_status;
    }
  }
  else {
    /* FIFO Enable-> FIFO_EN bit=1 in CTRL_REG2*/
    op_status = LPS22HB_Set_FifoModeUse(LPS22HB_ENABLE);
    if(op_status < LPS22HB_OK)
    {
      return op_status;
    }

    if (pLPS22HBFIFO->WTM_INT){
      /* Enable FIFO Watermark Level Use*/
      op_status = LPS22HB_Set_FifoWatermarkLevelUse(LPS22HB_ENABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
      /*Set Fifo Watermark Level*/
      op_status = LPS22HB_Set_FifoWatermarkLevel(pLPS22HBFIFO->WTM_LEVEL);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
      /* Force->Enable FIFO Treshold interrupt on INT_DRDY pin. */
      op_status = LPS22HB_Set_FIFO_FTH_Interrupt(LPS22HB_ENABLE);
      if(op_status < LPS22HB_OK)
      {
        return op_status;
      }
    }
  }

  op_status = LPS22HB_Set_FifoMode(pLPS22HBFIFO->FIFO_MODE);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  return LPS22HB_OK;
}

LPS22HB_OpResult LPS22HB_Get_FifoConfig(LPS22HB_FIFOTypeDef_st* pLPS22HBFIFO)
{
  LPS22HB_OpResult op_status;
  
   uint8_t tmp;

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_FIFO_REG, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

  /*!< Fifo Mode Selection */
  pLPS22HBFIFO->FIFO_MODE= (LPS22HB_FifoMode_et)(tmp& LPS22HB_FIFO_MODE_MASK);

  /*!< FIFO threshold/Watermark level selection*/
  pLPS22HBFIFO->WTM_LEVEL= (uint8_t)(tmp& LPS22HB_WTM_POINT_MASK);

  op_status = LPS22HB_ReadReg(LPS22HB_CTRL_REG2, 1, &tmp);
  if(op_status < LPS22HB_OK)
  {
    return op_status;
  }

   /*!< Enable/Disable the watermark interrupt*/
  pLPS22HBFIFO->WTM_INT= (LPS22HB_State_et)((tmp& LPS22HB_WTM_EN_MASK)>>LPS22HB_WTM_EN_BIT);


  return LPS22HB_OK;
}

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
