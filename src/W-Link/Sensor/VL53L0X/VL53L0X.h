/*******************************************************************************
 Copyright Ã‚Â© 2016, STMicroelectronics International N.V.
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
 * Based on STMicroelectronics VL53L0X driver
 * Modified by Neon Smart Studio for W-Link
 */

#ifndef __VL53L0X_CLASS_H
#define __VL53L0X_CLASS_H

#include <stdint.h>
#include <stdbool.h>

#include "VL53L0x_Def.h"

#include "Sensor_Config.h"

#ifdef __cplusplus
extern "C" {
#endif

VL53L0X_OpResult VL53L0X_Init();
VL53L0X_OpResult VL53L0X_DeInit();
VL53L0X_OpResult VL53L0X_Power_Off();
VL53L0X_OpResult VL53L0X_Power_On();
VL53L0X_OpResult VL53L0X_SetI2CAddress(uint8_t DeviceAddress);
VL53L0X_OpResult VL53L0X_GetSensorId(uint16_t* rl_id);
VL53L0X_OpResult VL53L0X_WaitMeasurementDataReady();
VL53L0X_OpResult VL53L0X_WaitStopCompleted();
VL53L0X_OpResult VL53L0X_SensorInit();
VL53L0X_OpResult VL53L0X_GetDistance(uint32_t *piData);

#ifdef __cplusplus
}
#endif

#endif /* _VL53L0X_CLASS_H_ */
