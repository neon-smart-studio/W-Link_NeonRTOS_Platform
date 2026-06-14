/**
 ******************************************************************************
 * @file    vl53l4cd_class.h
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    29 November 2021
 * @brief   Abstract Class for VL53L4CD sensor.
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
 * Based on STMicroelectronics VL53L4CD driver
 * Modified by Neon Smart Studio for W-Link
 */

#ifndef VL53L4CD_H
#define VL53L4CD_H

#include <stdint.h>
#include <stdbool.h>

#include "VL53L4CD_Def.h"

#include "Sensor_Config.h"

#ifdef __cplusplus
extern "C" {
#endif

VL53L4CD_OpResult VL53L4CD_Init();
VL53L4CD_OpResult VL53L4CD_DeInit();
VL53L4CD_OpResult VL53L4CD_Power_Off();
VL53L4CD_OpResult VL53L4CD_Power_On();
VL53L4CD_OpResult VL53L4CD_SetI2CAddress(uint8_t new_address);
VL53L4CD_OpResult VL53L4CD_SensorInit();
VL53L4CD_OpResult VL53L4CD_GetSensorId(uint16_t *p_id);
VL53L4CD_OpResult VL53L4CD_ClearInterrupt();
VL53L4CD_OpResult VL53L4CD_StartRanging();
VL53L4CD_OpResult VL53L4CD_StopRanging();
VL53L4CD_OpResult VL53L4CD_CheckForDataReady(uint8_t *p_is_data_ready);
VL53L4CD_OpResult VL53L4CD_SetRangeTiming(uint32_t timing_budget_ms, uint32_t inter_measurement_ms);
VL53L4CD_OpResult VL53L4CD_GetRangeTiming(uint32_t *p_timing_budget_ms, uint32_t *p_inter_measurement_ms);
VL53L4CD_OpResult VL53L4CD_GetResult(VL53L4CD_Result_t *pResult);
VL53L4CD_OpResult VL53L4CD_SetOffset(int16_t OffsetValueInMm);
VL53L4CD_OpResult VL53L4CD_GetOffset(int16_t *Offset);
VL53L4CD_OpResult VL53L4CD_SetXtalk(uint16_t XtalkValueKcps);
VL53L4CD_OpResult VL53L4CD_GetXtalk(uint16_t *p_xtalk_kcps);
VL53L4CD_OpResult VL53L4CD_SetDetectionThresholds(uint16_t distance_low_mm, uint16_t distance_high_mm, uint8_t window);
VL53L4CD_OpResult VL53L4CD_GetDetectionThresholds(uint16_t *p_distance_low_mm, uint16_t *p_distance_high_mm, uint8_t *p_window);
VL53L4CD_OpResult VL53L4CD_SetSignalThreshold(uint16_t signal_kcps);
VL53L4CD_OpResult VL53L4CD_GetSignalThreshold(uint16_t *p_signal_kcps);
VL53L4CD_OpResult VL53L4CD_SetSigmaThreshold(uint16_t sigma_mm);
VL53L4CD_OpResult VL53L4CD_GetSigmaThreshold(uint16_t *p_sigma_mm);
VL53L4CD_OpResult VL53L4CD_StartTemperatureUpdate();
VL53L4CD_OpResult VL53L4CD_CalibrateOffset(int16_t TargetDistInMm, int16_t *p_measured_offset_mm, int16_t nb_samples);
VL53L4CD_OpResult VL53L4CD_CalibrateXtalk(int16_t TargetDistInMm, uint16_t *p_measured_xtalk_kcps, int16_t nb_samples);

#ifdef __cplusplus
}
#endif

#endif // VL53L4CD_H
