/**
 ******************************************************************************
 * @file    HTS221_Driver.h
 * @author  HESA Application Team
 * @version V1.1
 * @date    10-August-2016
 * @brief   HTS221 driver header file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef HTS221_REGISTER_H
#define HTS221_REGISTER_H

#include <stdint.h>

#include "HTS221_IO.h"

#include "HTS221_Def.h"

#include "HTS221_Register_Def.h"

#ifdef __cplusplus
extern "C" {
#endif

HTS221_OpStatus HTS221_ReadReg( uint8_t RegAddr, uint16_t NumByteToRead, uint8_t *p_data );
HTS221_OpStatus HTS221_WriteReg( uint8_t RegAddr, uint16_t NumByteToWrite, uint8_t *p_data );

HTS221_OpStatus HTS221_Get_DriverVersion(HTS221_DriverVersion_st* version);
HTS221_OpStatus HTS221_Get_DeviceID(uint8_t* deviceid);

HTS221_OpStatus HTS221_Set_InitConfig(HTS221_Init_st* pxInit);
HTS221_OpStatus HTS221_Get_InitConfig(HTS221_Init_st* pxInit);
HTS221_OpStatus HTS221_Clear_InitConfig();
HTS221_OpStatus HTS221_IsMeasurementCompleted(HTS221_BitStatus_et* Is_Measurement_Completed);

HTS221_OpStatus HTS221_Get_Measurement(uint16_t* humidity, int16_t* temperature);
HTS221_OpStatus HTS221_Get_RawMeasurement(int16_t* humidity, int16_t* temperature);
HTS221_OpStatus HTS221_Get_Humidity(uint16_t* value);
HTS221_OpStatus HTS221_Get_HumidityRaw(int16_t* value);
HTS221_OpStatus HTS221_Get_TemperatureRaw(int16_t* value);
HTS221_OpStatus HTS221_Get_Temperature(int16_t* value);
HTS221_OpStatus HTS221_Get_DataStatus(HTS221_BitStatus_et* humidity, HTS221_BitStatus_et* temperature);
HTS221_OpStatus HTS221_Activate();
HTS221_OpStatus HTS221_DeActivate();

HTS221_OpStatus HTS221_Set_AvgHT(HTS221_Avgh_et avgh, HTS221_Avgt_et avgt);
HTS221_OpStatus HTS221_Set_AvgH(HTS221_Avgh_et avgh);
HTS221_OpStatus HTS221_Set_AvgT(HTS221_Avgt_et avgt);
HTS221_OpStatus HTS221_Get_AvgHT(HTS221_Avgh_et* avgh, HTS221_Avgt_et* avgt);
HTS221_OpStatus HTS221_Set_BduMode(HTS221_State_et status);
HTS221_OpStatus HTS221_Get_BduMode(HTS221_State_et* status);
HTS221_OpStatus HTS221_Set_PowerDownMode(HTS221_BitStatus_et status);
HTS221_OpStatus HTS221_Get_PowerDownMode(HTS221_BitStatus_et* status);
HTS221_OpStatus HTS221_Set_Odr(HTS221_Odr_et odr);
HTS221_OpStatus HTS221_Get_Odr(HTS221_Odr_et* odr);
HTS221_OpStatus HTS221_MemoryBoot();
HTS221_OpStatus HTS221_Set_HeaterState(HTS221_State_et status);
HTS221_OpStatus HTS221_Get_HeaterState(HTS221_State_et* status);
HTS221_OpStatus HTS221_StartOneShotMeasurement();
HTS221_OpStatus HTS221_Set_IrqActiveLevel(HTS221_DrdyLevel_et status);
HTS221_OpStatus HTS221_Get_IrqActiveLevel(HTS221_DrdyLevel_et* status);
HTS221_OpStatus HTS221_Set_IrqOutputType(HTS221_OutputType_et value);
HTS221_OpStatus HTS221_Get_IrqOutputType(HTS221_OutputType_et* value);
HTS221_OpStatus HTS221_Set_IrqEnable(HTS221_State_et status);
HTS221_OpStatus HTS221_Get_IrqEnable(HTS221_State_et* status);

#ifdef __cplusplus
}
#endif

#endif // HTS221_REGISTER_H

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
