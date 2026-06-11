/**
 ******************************************************************************
 * @file    HTS221_Driver.c
 * @author  HESA Application Team
 * @version V1.1
 * @date    10-August-2016
 * @brief   HTS221 driver file
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

#include "HTS221_Def.h"

#include "HTS221_Register_Def.h"
#include "HTS221_Register.h"

HTS221_OpResult HTS221_ReadReg(uint8_t RegAddr, uint16_t NumByteToRead, uint8_t *p_data)
{
  HTS221_OpResult op_status;
  
  if ( NumByteToRead > 1 ) RegAddr |= 0x80;

  op_status = HTS221_IO_Read( RegAddr, p_data, NumByteToRead );
  if(op_status < HTS221_OK)
  {
    return op_status;
  }
  
  return HTS221_OK;
}

HTS221_OpResult HTS221_WriteReg(uint8_t RegAddr, uint16_t NumByteToWrite, uint8_t *p_data)
{
  HTS221_OpResult op_status;
  
  if ( NumByteToWrite > 1 ) RegAddr |= 0x80;

  op_status = HTS221_IO_Write( RegAddr, p_data, NumByteToWrite );
  if(op_status < HTS221_OK)
  {
    return op_status;
  }
  
  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_DriverVersion(HTS221_DriverVersion_st* version)
{
  version->Major = HTS221_DRIVER_VERSION_MAJOR;
  version->Minor = HTS221_DRIVER_VERSION_MINOR;
  version->Point = HTS221_DRIVER_VERSION_POINT;

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_DeviceID(uint8_t* deviceid)
{
  HTS221_OpResult op_status;
  
  op_status = HTS221_ReadReg(HTS221_WHO_AM_I_REG, 1, deviceid);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_InitConfig(HTS221_Init_st* pxInit)
{
  HTS221_OpResult op_status;
  
  uint8_t buffer[3];

  op_status = HTS221_ReadReg(HTS221_AV_CONF_REG, 1, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  buffer[0] &= ~(HTS221_AVGH_MASK | HTS221_AVGT_MASK);
  buffer[0] |= (uint8_t)pxInit->avg_h;
  buffer[0] |= (uint8_t)pxInit->avg_t;

  op_status = HTS221_WriteReg(HTS221_AV_CONF_REG, 1, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 3, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  buffer[0] &= ~(HTS221_BDU_MASK | HTS221_ODR_MASK);
  buffer[0] |= (uint8_t)pxInit->odr;
  buffer[0] |= ((uint8_t)pxInit->bdu_status) << HTS221_BDU_BIT;

  buffer[1] &= ~HTS221_HEATHER_BIT;
  buffer[1] |= ((uint8_t)pxInit->heater_status) << HTS221_HEATHER_BIT;

  buffer[2] &= ~(HTS221_DRDY_H_L_MASK | HTS221_PP_OD_MASK | HTS221_DRDY_MASK);
  buffer[2] |= ((uint8_t)pxInit->irq_level) << HTS221_DRDY_H_L_BIT;
  buffer[2] |= (uint8_t)pxInit->irq_output_type;
  buffer[2] |= ((uint8_t)pxInit->irq_enable) << HTS221_DRDY_BIT;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG1, 3, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_InitConfig(HTS221_Init_st* pxInit)
{
  HTS221_OpResult op_status;
  
  uint8_t buffer[3];

  op_status = HTS221_ReadReg(HTS221_AV_CONF_REG, 1, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  pxInit->avg_h = (HTS221_Avgh_et)(buffer[0] & HTS221_AVGH_MASK);
  pxInit->avg_t = (HTS221_Avgt_et)(buffer[0] & HTS221_AVGT_MASK);

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 3, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  pxInit->odr = (HTS221_Odr_et)(buffer[0] & HTS221_ODR_MASK);
  pxInit->bdu_status = (HTS221_State_et)((buffer[0] & HTS221_BDU_MASK) >> HTS221_BDU_BIT);
  pxInit->heater_status = (HTS221_State_et)((buffer[1] & HTS221_HEATHER_MASK) >> HTS221_HEATHER_BIT);

  pxInit->irq_level = (HTS221_DrdyLevel_et)(buffer[2] & HTS221_DRDY_H_L_MASK);
  pxInit->irq_output_type = (HTS221_OutputType_et)(buffer[2] & HTS221_PP_OD_MASK);
  pxInit->irq_enable = (HTS221_State_et)((buffer[2] & HTS221_DRDY_MASK) >> HTS221_DRDY_BIT);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Clear_InitConfig()
{
  HTS221_OpResult op_status;
  
  uint8_t buffer[4];

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  /* HTS221 in power down */
  buffer[0] |= 0x01 << HTS221_PD_BIT;

  /* Make HTS221 boot */
  buffer[1] |= 0x01 << HTS221_BOOT_BIT;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG1, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  /* Dump of data output */
  op_status = HTS221_ReadReg(HTS221_HR_OUT_L_REG, 4, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_Measurement(uint16_t* humidity, int16_t* temperature)
{
  HTS221_OpResult op_status;
  
  op_status = HTS221_Get_Temperature( temperature );
  if(op_status < HTS221_OK)
  {
    return op_status;
  }
  op_status = HTS221_Get_Humidity( humidity );
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_RawMeasurement(int16_t* humidity, int16_t* temperature)
{
  HTS221_OpResult op_status;
  
  uint8_t buffer[4];

  op_status = HTS221_ReadReg(HTS221_HR_OUT_L_REG, 4, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *humidity = (int16_t)((((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0]);
  *temperature = (int16_t)((((uint16_t)buffer[3]) << 8) | (uint16_t)buffer[2]);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_Humidity(uint16_t* value)
{
  HTS221_OpResult op_status;
  
  int16_t H0_T0_out, H1_T0_out, H_T_out;
  int16_t H0_rh, H1_rh;
  uint8_t buffer[2];
  float   tmp_f;

  op_status = HTS221_ReadReg(HTS221_H0_RH_X2, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  H0_rh = buffer[0] >> 1;
  H1_rh = buffer[1] >> 1;

  op_status = HTS221_ReadReg(HTS221_H0_T0_OUT_L, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  H0_T0_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];

  op_status = HTS221_ReadReg(HTS221_H1_T0_OUT_L, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  H1_T0_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];

  op_status = HTS221_ReadReg(HTS221_HR_OUT_L_REG, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  H_T_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];

  tmp_f = (float)(H_T_out - H0_T0_out) * (float)(H1_rh - H0_rh) / (float)(H1_T0_out - H0_T0_out)  +  H0_rh;
  tmp_f *= 10.0f;

  *value = ( tmp_f > 1000.0f ) ? 1000
           : ( tmp_f <    0.0f ) ?    0
           : ( uint16_t )tmp_f;

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_HumidityRaw(int16_t* value)
{
  HTS221_OpResult op_status;
  
  uint8_t buffer[2];

  op_status = HTS221_ReadReg(HTS221_HR_OUT_L_REG, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *value = (int16_t)((((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0]);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_Temperature(int16_t *value)
{
  HTS221_OpResult op_status;
  
  int16_t T0_out, T1_out, T_out, T0_degC_x8_u16, T1_degC_x8_u16;
  int16_t T0_degC, T1_degC;
  uint8_t buffer[4], tmp;
  float   tmp_f;

  op_status = HTS221_ReadReg(HTS221_T0_DEGC_X8, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }
  op_status = HTS221_ReadReg(HTS221_T0_T1_DEGC_H2, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  T0_degC_x8_u16 = (((uint16_t)(tmp & 0x03)) << 8) | ((uint16_t)buffer[0]);
  T1_degC_x8_u16 = (((uint16_t)(tmp & 0x0C)) << 6) | ((uint16_t)buffer[1]);
  T0_degC = T0_degC_x8_u16 >> 3;
  T1_degC = T1_degC_x8_u16 >> 3;

  op_status = HTS221_ReadReg(HTS221_T0_OUT_L, 4, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  T0_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];
  T1_out = (((uint16_t)buffer[3]) << 8) | (uint16_t)buffer[2];

  op_status = HTS221_ReadReg(HTS221_TEMP_OUT_L_REG, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  T_out = (((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0];

  tmp_f = (float)(T_out - T0_out) * (float)(T1_degC - T0_degC) / (float)(T1_out - T0_out)  +  T0_degC;
  tmp_f *= 10.0f;

  *value = ( int16_t )tmp_f;

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_TemperatureRaw(int16_t* value)
{
  HTS221_OpResult op_status;
  
  uint8_t buffer[2];

  op_status = HTS221_ReadReg(HTS221_TEMP_OUT_L_REG, 2, buffer);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *value = (int16_t)((((uint16_t)buffer[1]) << 8) | (uint16_t)buffer[0]);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_DataStatus(HTS221_BitStatus_et* humidity, HTS221_BitStatus_et* temperature)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_STATUS_REG, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *humidity = (HTS221_BitStatus_et)((tmp & HTS221_HDA_MASK) >> HTS221_H_DA_BIT);
  *temperature = (HTS221_BitStatus_et)(tmp & HTS221_TDA_MASK);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Activate()
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp |= HTS221_PD_MASK;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_DeActivate()
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_PD_MASK;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_IsMeasurementCompleted(HTS221_BitStatus_et* Is_Measurement_Completed)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_STATUS_REG, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  if((tmp & (uint8_t)(HTS221_HDA_MASK | HTS221_TDA_MASK)) == (uint8_t)(HTS221_HDA_MASK | HTS221_TDA_MASK))
    *Is_Measurement_Completed = HTS221_SET;
  else
    *Is_Measurement_Completed = HTS221_RESET;

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_AvgHT(HTS221_Avgh_et avgh, HTS221_Avgt_et avgt)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_AV_CONF_REG, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~(HTS221_AVGH_MASK | HTS221_AVGT_MASK);
  tmp |= (uint8_t)avgh;
  tmp |= (uint8_t)avgt;

  op_status = HTS221_WriteReg(HTS221_AV_CONF_REG, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_AvgH(HTS221_Avgh_et avgh)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_AV_CONF_REG, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_AVGH_MASK;
  tmp |= (uint8_t)avgh;

  op_status = HTS221_WriteReg(HTS221_AV_CONF_REG, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_AvgT(HTS221_Avgt_et avgt)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_AV_CONF_REG, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_AVGT_MASK;
  tmp |= (uint8_t)avgt;

  op_status = HTS221_WriteReg(HTS221_AV_CONF_REG, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_AvgHT(HTS221_Avgh_et* avgh, HTS221_Avgt_et* avgt)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_AV_CONF_REG, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *avgh = (HTS221_Avgh_et)(tmp & HTS221_AVGH_MASK);
  *avgt = (HTS221_Avgt_et)(tmp & HTS221_AVGT_MASK);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_BduMode(HTS221_State_et status)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_BDU_MASK;
  tmp |= ((uint8_t)status) << HTS221_BDU_BIT;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_BduMode(HTS221_State_et* status)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *status = (HTS221_State_et)((tmp & HTS221_BDU_MASK) >> HTS221_BDU_BIT);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_PowerDownMode(HTS221_BitStatus_et status)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_PD_MASK;
  tmp |= ((uint8_t)status) << HTS221_PD_BIT;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_PowerDownMode(HTS221_BitStatus_et* status)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *status = (HTS221_BitStatus_et)((tmp & HTS221_PD_MASK) >> HTS221_PD_BIT);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_Odr(HTS221_Odr_et odr)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_ODR_MASK;
  tmp |= (uint8_t)odr;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_Odr(HTS221_Odr_et* odr)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG1, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= HTS221_ODR_MASK;
  *odr = (HTS221_Odr_et)tmp;

  return HTS221_OK;
}

HTS221_OpResult HTS221_MemoryBoot()
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG2, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp |= HTS221_BOOT_MASK;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG2, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_HeaterState(HTS221_State_et status)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG2, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_HEATHER_MASK;
  tmp |= ((uint8_t)status) << HTS221_HEATHER_BIT;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG2, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_HeaterState(HTS221_State_et* status)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG2, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *status = (HTS221_State_et)((tmp & HTS221_HEATHER_MASK) >> HTS221_HEATHER_BIT);

  return HTS221_OK;
}

HTS221_OpResult HTS221_StartOneShotMeasurement()
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG2, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp |= HTS221_ONE_SHOT_MASK;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG2, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;

}

HTS221_OpResult HTS221_Set_IrqActiveLevel(HTS221_DrdyLevel_et value)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG3, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_DRDY_H_L_MASK;
  tmp |= (uint8_t)value;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG3, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_IrqActiveLevel(HTS221_DrdyLevel_et* value)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG3, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *value = (HTS221_DrdyLevel_et)(tmp & HTS221_DRDY_H_L_MASK);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_IrqOutputType(HTS221_OutputType_et value)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG3, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_PP_OD_MASK;
  tmp |= (uint8_t)value;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG3, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_IrqOutputType(HTS221_OutputType_et* value)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG3, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *value = (HTS221_OutputType_et)(tmp & HTS221_PP_OD_MASK);

  return HTS221_OK;
}

HTS221_OpResult HTS221_Set_IrqEnable(HTS221_State_et status)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG3, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  tmp &= ~HTS221_DRDY_MASK;
  tmp |= ((uint8_t)status) << HTS221_DRDY_BIT;

  op_status = HTS221_WriteReg(HTS221_CTRL_REG3, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  return HTS221_OK;
}

HTS221_OpResult HTS221_Get_IrqEnable(HTS221_State_et* status)
{
  HTS221_OpResult op_status;
  
  uint8_t tmp;

  op_status = HTS221_ReadReg(HTS221_CTRL_REG3, 1, &tmp);
  if(op_status < HTS221_OK)
  {
    return op_status;
  }

  *status = (HTS221_State_et)((tmp & HTS221_DRDY_MASK) >> HTS221_DRDY_BIT);

  return HTS221_OK;
}

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
