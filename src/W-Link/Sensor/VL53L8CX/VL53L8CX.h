/**
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef VL53L8CX_API_H_
#define VL53L8CX_API_H_

#include <stdint.h>
#include <stdbool.h>

#include "VL53L8CX_Def.h"

#include "Sensor_Config.h"

#ifdef __cplusplus
extern "C" {
#endif

VL53L8CX_OpResult VL53L8CX_Init();
VL53L8CX_OpResult VL53L8CX_DeInit();
VL53L8CX_OpResult VL53L8CX_Power_Off();
VL53L8CX_OpResult VL53L8CX_Power_On();
VL53L8CX_OpResult VL53L8CX_Set_I2C_Address(uint8_t new_address);
VL53L8CX_OpResult VL53L8CX_SensorInit();
VL53L8CX_OpResult VL53L8CX_Is_Alive(uint8_t *p_is_alive);
VL53L8CX_OpResult VL53L8CX_Get_Power_Mode(uint8_t *p_power_mode);
VL53L8CX_OpResult VL53L8CX_Set_Power_Mode(uint8_t power_mode);
VL53L8CX_OpResult VL53L8CX_Start_Ranging();
VL53L8CX_OpResult VL53L8CX_Stop_Ranging();
VL53L8CX_OpResult VL53L8CX_Check_Data_Ready(uint8_t *p_isReady);
VL53L8CX_OpResult VL53L8CX_Get_Ranging_Data(VL53L8CX_ResultsData *p_results);
VL53L8CX_OpResult VL53L8CX_Get_Resolution(uint8_t *p_resolution);
VL53L8CX_OpResult VL53L8CX_Set_Resolution(uint8_t resolution);
VL53L8CX_OpResult VL53L8CX_Get_Ranging_Frequency_Hz(uint8_t *p_frequency_hz);
VL53L8CX_OpResult VL53L8CX_Set_Ranging_Frequency_Hz(uint8_t frequency_hz);
VL53L8CX_OpResult VL53L8CX_Get_Integration_Time_mS(uint32_t *p_time_ms);
VL53L8CX_OpResult VL53L8CX_Set_Integration_Time_mS(uint32_t integration_time_ms);
VL53L8CX_OpResult VL53L8CX_Get_Sharpener_Percent(uint8_t *p_sharpener_percent);
VL53L8CX_OpResult VL53L8CX_Set_Sharpener_Percent(uint8_t sharpener_percent);
VL53L8CX_OpResult VL53L8CX_Get_Target_Order(uint8_t *p_target_order);
VL53L8CX_OpResult VL53L8CX_Set_Target_Order(uint8_t target_order);
VL53L8CX_OpResult VL53L8CX_Get_Ranging_Mode(uint8_t *p_ranging_mode);
VL53L8CX_OpResult VL53L8CX_Set_Ranging_Mode(uint8_t ranging_mode);
VL53L8CX_OpResult VL53L8CX_Get_External_Sync_Pin_Enable(uint8_t *p_is_sync_pin_enabled);
VL53L8CX_OpResult VL53L8CX_Set_External_Sync_Pin_Enable(uint8_t enable_sync_pin);
VL53L8CX_OpResult VL53L8CX_Get_VHV_Repeat_Count(uint32_t *p_repeat_count);
VL53L8CX_OpResult VL53L8CX_Set_VHV_Repeat_Count(uint32_t repeat_count);
VL53L8CX_OpResult VL53L8CX_DCI_Read_Data(uint8_t *data, uint32_t index, uint16_t data_size);
VL53L8CX_OpResult VL53L8CX_DCI_Write_Data(uint8_t *data, uint32_t index, uint16_t data_size);
VL53L8CX_OpResult VL53L8CX_DCI_Replace_Data(uint8_t *data, uint32_t index, uint16_t data_size, uint8_t *new_data, uint16_t new_data_size, uint16_t new_data_pos);


#ifdef __cplusplus
}
#endif

#endif //VL53L8CX_API_H_
