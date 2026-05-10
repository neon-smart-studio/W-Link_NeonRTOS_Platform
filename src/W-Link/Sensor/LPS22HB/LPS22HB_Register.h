/**
 ******************************************************************************
 * @file    LPS22HB_Driver.h
 * @author  HESA Application Team
 * @version V1.1
 * @date    10-August-2016
 * @brief   LPS22HB driver header file
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
 * Based on STMicroelectronics LPS22HB driver
 * Modified by Neon Smart Studio for W-Link
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LPS22HB_REGISTER_H
#define LPS22HB_REGISTER_H

#include <stdint.h>

#include <stdint.h>

#include "LPS22HB_IO.h"

#include "LPS22HB_Def.h"

#include "LPS22HB_Register_Def.h"

#ifdef __cplusplus
extern "C" {
#endif

LPS22HB_OpStatus LPS22HB_ReadReg( uint8_t RegAddr, uint16_t NumByteToRead, uint8_t *p_data );
LPS22HB_OpStatus LPS22HB_WriteReg( uint8_t RegAddr, uint16_t NumByteToWrite, uint8_t *p_data );

LPS22HB_OpStatus LPS22HB_Get_DriverVersion(LPS22HB_DriverVersion_st *Version);

LPS22HB_OpStatus LPS22HB_Set_InitConfig();

LPS22HB_OpStatus LPS22HB_Clear_InitConfig();

LPS22HB_OpStatus LPS22HB_Get_DeviceID(uint8_t* deviceid);

LPS22HB_OpStatus LPS22HB_Set_PowerMode(LPS22HB_PowerMode_et mode);

LPS22HB_OpStatus LPS22HB_Get_PowerMode(LPS22HB_PowerMode_et* mode);

LPS22HB_OpStatus LPS22HB_Set_Odr(LPS22HB_Odr_et odr);

LPS22HB_OpStatus LPS22HB_Get_Odr(LPS22HB_Odr_et* odr);

LPS22HB_OpStatus LPS22HB_Set_LowPassFilter(LPS22HB_State_et state);

LPS22HB_OpStatus LPS22HB_Set_LowPassFilterCutoff(LPS22HB_LPF_Cutoff_et cutoff);

LPS22HB_OpStatus LPS22HB_Set_Bdu(LPS22HB_Bdu_et bdu);

LPS22HB_OpStatus LPS22HB_Get_Bdu(LPS22HB_Bdu_et* bdu);

LPS22HB_OpStatus LPS22HB_Set_SpiInterface(LPS22HB_SPIMode_et spimode);

LPS22HB_OpStatus LPS22HB_Get_SpiInterface(LPS22HB_SPIMode_et* spimode);

LPS22HB_OpStatus LPS22HB_SwReset();

LPS22HB_OpStatus LPS22HB_MemoryBoot();

LPS22HB_OpStatus LPS22HB_SwResetAndMemoryBoot();

LPS22HB_OpStatus LPS22HB_Set_FifoModeUse(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_Set_FifoWatermarkLevelUse(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_Set_AutomaticIncrementRegAddress(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_StartOneShotMeasurement();

LPS22HB_OpStatus LPS22HB_Set_I2C(LPS22HB_State_et i2cstate);

LPS22HB_OpStatus LPS22HB_Set_InterruptActiveLevel(LPS22HB_InterruptActiveLevel_et mode);

LPS22HB_OpStatus LPS22HB_Set_InterruptOutputType(LPS22HB_OutputType_et output);

LPS22HB_OpStatus LPS22HB_Set_InterruptControlConfig(LPS22HB_OutputSignalConfig_et config);

LPS22HB_OpStatus LPS22HB_Set_DRDYInterrupt(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_Set_FIFO_OVR_Interrupt(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_Set_FIFO_FTH_Interrupt(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_Set_FIFO_FULL_Interrupt(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_Set_AutoRifP();

LPS22HB_OpStatus LPS22HB_ResetAutoRifP();

LPS22HB_OpStatus LPS22HB_Set_AutoZeroFunction();

LPS22HB_OpStatus LPS22HB_ResetAutoZeroFunction();

LPS22HB_OpStatus LPS22HB_Set_InterruptDifferentialGeneration(LPS22HB_State_et diff_en) ;

LPS22HB_OpStatus LPS22HB_Get_InterruptDifferentialGeneration(LPS22HB_State_et* diff_en);

LPS22HB_OpStatus LPS22HB_LatchInterruptRequest(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_Set_PLE(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_Set_PHE(LPS22HB_State_et status);

LPS22HB_OpStatus LPS22HB_Get_InterruptDifferentialEventStatus(LPS22HB_InterruptDiffStatus_st* interruptsource);

LPS22HB_OpStatus LPS22HB_Get_DataStatus(LPS22HB_DataStatus_st* datastatus);

LPS22HB_OpStatus LPS22HB_Get_RawPressure(int32_t *raw_press);

LPS22HB_OpStatus LPS22HB_Get_Pressure(int32_t* Pout);

LPS22HB_OpStatus LPS22HB_Get_RawTemperature(int16_t *raw_data);

LPS22HB_OpStatus LPS22HB_Get_Temperature(int16_t* Tout);

LPS22HB_OpStatus LPS22HB_Get_PressureThreshold(int16_t *P_ths);

LPS22HB_OpStatus LPS22HB_Set_PressureThreshold(int16_t P_ths);

LPS22HB_OpStatus LPS22HB_Set_FifoMode(LPS22HB_FifoMode_et fifomode);

LPS22HB_OpStatus LPS22HB_Get_FifoMode(LPS22HB_FifoMode_et* fifomode);

LPS22HB_OpStatus LPS22HB_Set_FifoWatermarkLevel(uint8_t wtmlevel);

LPS22HB_OpStatus LPS22HB_Get_FifoWatermarkLevel(uint8_t *wtmlevel);

LPS22HB_OpStatus LPS22HB_Get_FifoStatus(LPS22HB_FifoStatus_st* status);

LPS22HB_OpStatus LPS22HB_Get_PressureOffsetValue(int16_t *pressoffset);

LPS22HB_OpStatus LPS22HB_Get_ReferencePressure(int32_t* RefP);

LPS22HB_OpStatus LPS22HB_IsMeasurementCompleted(uint8_t* Is_Measurement_Completed);

LPS22HB_OpStatus LPS22HB_Get_Measurement(LPS22HB_MeasureTypeDef_st *Measurement_Value);

LPS22HB_OpStatus LPS22HB_Set_GenericConfig(LPS22HB_ConfigTypeDef_st* pxLPS22HBInit);

LPS22HB_OpStatus LPS22HB_Get_GenericConfig(LPS22HB_ConfigTypeDef_st* pxLPS22HBInit);

LPS22HB_OpStatus LPS22HB_Set_InterruptConfig(LPS22HB_InterruptTypeDef_st* pLPS22HBInt);

LPS22HB_OpStatus LPS22HB_Get_InterruptConfig(LPS22HB_InterruptTypeDef_st* pLPS22HBInt);

LPS22HB_OpStatus LPS22HB_Set_FifoConfig(LPS22HB_FIFOTypeDef_st* pLPS22HBFIFO);

LPS22HB_OpStatus LPS22HB_Get_FifoConfig(LPS22HB_FIFOTypeDef_st* pLPS22HBFIFO);

LPS22HB_OpStatus LPS22HB_Set_ClockTreeConfifuration(LPS22HB_CTE_et mode);

#ifdef __cplusplus
}
#endif

#endif // LPS22HB_REGISTER_H

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
