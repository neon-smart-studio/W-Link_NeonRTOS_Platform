/**
 ******************************************************************************
 * @file    vl53l5cx_class.h
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    11 November 2021
 * @brief   Abstract Class for VL53L5CX sensor.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2021 STMicroelectronics</center></h2>
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
 * Based on STMicroelectronics VL53L5CX driver
 * Modified by Neon Smart Studio for W-Link
 */

#ifndef VL53L5CX_H
#define VL53L5CX_H

#include <stdint.h>
#include <stdbool.h>

#include "VL53L5CX_Def.h"

#include "Sensor_Config.h"

#ifdef __cplusplus
extern "C" {
#endif

VL53L5CX_OpResult VL53L5CX_Init();
VL53L5CX_OpResult VL53L5CX_DeInit();
VL53L5CX_OpResult VL53L5CX_Power_Off();
VL53L5CX_OpResult VL53L5CX_Power_On();
VL53L5CX_OpResult VL53L5CX_Set_I2C_Address(uint8_t new_address);
VL53L5CX_OpResult VL53L5CX_SensorInit();
VL53L5CX_OpResult VL53L5CX_Is_Alive(int8_t *p_is_alive);
VL53L5CX_OpResult VL53L5CX_Get_Power_Mode(uint8_t *p_power_mode);
VL53L5CX_OpResult VL53L5CX_Set_Power_Mode(uint8_t power_mode);
VL53L5CX_OpResult VL53L5CX_Start_Ranging();
VL53L5CX_OpResult VL53L5CX_Stop_Ranging();
VL53L5CX_OpResult VL53L5CX_Check_Data_Ready(uint8_t *p_isReady);
VL53L5CX_OpResult VL53L5CX_Get_Ranging_Data(VL53L5CX_ResultsData *p_results);
VL53L5CX_OpResult VL53L5CX_Get_Resolution(uint8_t *p_resolution);
VL53L5CX_OpResult VL53L5CX_Set_Resolution(uint8_t resolution);
VL53L5CX_OpResult VL53L5CX_Get_Ranging_Frequency_Hz(uint8_t *p_frequency_hz);
VL53L5CX_OpResult VL53L5CX_Set_Ranging_Frequency_Hz(uint8_t frequency_hz);
VL53L5CX_OpResult VL53L5CX_Get_Integration_Time_mS(uint32_t *p_time_ms);
VL53L5CX_OpResult VL53L5CX_Set_Integration_Time_mS(uint32_t integration_time_ms);
VL53L5CX_OpResult VL53L5CX_Get_Sharpener_Percent(uint8_t *p_sharpener_percent);
VL53L5CX_OpResult VL53L5CX_Set_Sharpener_Percent(uint8_t sharpener_percent);
VL53L5CX_OpResult VL53L5CX_Get_Target_Order(uint8_t *p_target_order);
VL53L5CX_OpResult VL53L5CX_Set_Target_Order(uint8_t target_order);
VL53L5CX_OpResult VL53L5CX_Get_Ranging_Mode(uint8_t *p_ranging_mode);
VL53L5CX_OpResult VL53L5CX_Set_Ranging_Mode(uint8_t ranging_mode);
VL53L5CX_OpResult VL53L5CX_DCI_Read_Data(uint8_t *data, uint32_t index, uint16_t data_size);
VL53L5CX_OpResult VL53L5CX_DCI_Write_Data(uint8_t *data, uint32_t index, uint16_t data_size);
VL53L5CX_OpResult VL53L5CX_DCI_Replace_Data(uint8_t *data, uint32_t index, uint16_t data_size, uint8_t *new_data, uint16_t new_data_size, uint16_t new_data_pos);

#ifdef __cplusplus
}
#endif

#endif // VL53L5CX_H
