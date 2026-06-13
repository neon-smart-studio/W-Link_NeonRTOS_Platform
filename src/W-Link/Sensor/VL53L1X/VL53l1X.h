/*******************************************************************************
 Copyright Ã‚Â© 2018, STMicroelectronics International N.V.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of STMicroelectronics nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED.
 IN NO EVENT SHALL STMICROELECTRONICS INTERNATIONAL N.V. BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
/*
 * Based on STMicroelectronics VL53L1X driver
 * Modified by Neon Smart Studio for W-Link
 */

#ifndef VL53l1X_H
#define VL53l1X_H

#include <stdint.h>
#include <stdbool.h>

#include "VL53L1X_Def.h"

#include "Sensor_Config.h"

#ifdef __cplusplus
extern "C" {
#endif

VL53L1X_OpResult VL53L1X_Init();
VL53L1X_OpResult VL53L1X_DeInit();
VL53L1X_OpResult VL53L1X_Power_Off();
VL53L1X_OpResult VL53L1X_Power_On();
VL53L1X_OpResult VL53L1X_SetI2CAddress(uint8_t new_address);
VL53L1X_OpResult VL53L1X_SensorInit();
VL53L1X_OpResult VL53L1X_ClearInterrupt();
VL53L1X_OpResult VL53L1X_SetInterruptPolarity(uint8_t IntPol);
VL53L1X_OpResult VL53L1X_GetInterruptPolarity(uint8_t *pIntPol);
VL53L1X_OpResult VL53L1X_StartRanging();
VL53L1X_OpResult VL53L1X_StopRanging();
VL53L1X_OpResult VL53L1X_CheckForDataReady(uint8_t *isDataReady);
VL53L1X_OpResult VL53L1X_SetTimingBudgetInMs(uint16_t TimingBudgetInMs);
VL53L1X_OpResult VL53L1X_GetTimingBudgetInMs(uint16_t *pTimingBudgetInMs);
VL53L1X_OpResult VL53L1X_SetDistanceMode(uint16_t DistanceMode);
VL53L1X_OpResult VL53L1X_GetDistanceMode(uint16_t *pDistanceMode);
VL53L1X_OpResult VL53L1X_SetInterMeasurementInMs(uint16_t InterMeasurementInMs);
VL53L1X_OpResult VL53L1X_GetInterMeasurementInMs(uint16_t * pIM);
VL53L1X_OpResult VL53L1X_BootState(uint8_t *state);
VL53L1X_OpResult VL53L1X_GetSensorId(uint16_t *id);
VL53L1X_OpResult VL53L1X_GetDistance(uint16_t *distance);
VL53L1X_OpResult VL53L1X_GetSignalPerSpad(uint16_t *signalPerSp);
VL53L1X_OpResult VL53L1X_GetAmbientPerSpad(uint16_t *amb);
VL53L1X_OpResult VL53L1X_GetSignalRate(uint16_t *signalRate);
VL53L1X_OpResult VL53L1X_GetSpadNb(uint16_t *spNb);
VL53L1X_OpResult VL53L1X_GetAmbientRate(uint16_t *ambRate);
VL53L1X_OpResult VL53L1X_GetRangeStatus(uint8_t *rangeStatus);
VL53L1X_OpResult VL53L1X_SetOffset(int16_t OffsetValue);
VL53L1X_OpResult VL53L1X_GetOffset(int16_t *Offset);
VL53L1X_OpResult VL53L1X_SetXtalk(uint16_t XtalkValue);
VL53L1X_OpResult VL53L1X_GetXtalk(uint16_t *Xtalk);
VL53L1X_OpResult VL53L1X_SetDistanceThreshold(uint16_t ThreshLow, uint16_t ThreshHigh, uint8_t Window, uint8_t IntOnNoTarget);
VL53L1X_OpResult VL53L1X_GetDistanceThresholdWindow(uint16_t *window);
VL53L1X_OpResult VL53L1X_GetDistanceThresholdLow(uint16_t *low);
VL53L1X_OpResult VL53L1X_GetDistanceThresholdHigh(uint16_t *high);
VL53L1X_OpResult VL53L1X_SetROI(uint16_t X, uint16_t Y);
VL53L1X_OpResult VL53L1X_GetROI_XY(uint16_t *ROI_X, uint16_t *ROI_Y);
VL53L1X_OpResult VL53L1X_SetROICenter(uint8_t ROICenter);
VL53L1X_OpResult VL53L1X_GetROICenter(uint8_t *ROICenter);
VL53L1X_OpResult VL53L1X_SetSignalThreshold(uint16_t signal);
VL53L1X_OpResult VL53L1X_GetSignalThreshold(uint16_t *signal);
VL53L1X_OpResult VL53L1X_SetSigmaThreshold(uint16_t sigma);
VL53L1X_OpResult VL53L1X_GetSigmaThreshold(uint16_t *signal);
VL53L1X_OpResult VL53L1X_StartTemperatureUpdate();
VL53L1X_OpResult VL53L1X_CalibrateOffset(uint16_t TargetDistInMm, int16_t *offset);
VL53L1X_OpResult VL53L1X_CalibrateXtalk(uint16_t TargetDistInMm, uint16_t *xtalk);

#ifdef __cplusplus
}
#endif

#endif // VL53l1X_H
