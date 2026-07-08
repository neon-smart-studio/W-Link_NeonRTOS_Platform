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

#ifndef VL53l1X_IO_H
#define VL53l1X_IO_H

#include <stdint.h>
#include <stdbool.h>

#include "GPIO/GPIO.h"

#include "VL53L1X_Def.h"

#include "Sensor_Config.h"

#define VL53L1X_DEFAULT_I2C_ADDRESS        0x52

#define VL53L1X_I2C_OP_TIMEOUT             500

#ifndef CONFIG_VL53L1X_I2C_INDEX
#define VL53L1X_I2C_INDEX                  hwI2C_Index_0
#else
#define VL53L1X_I2C_INDEX                  CONFIG_VL53L1X_I2C_INDEX
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*VL53L1X_Interrupt_Handler)(uint8_t sensor_index);

VL53L1X_OpResult VL53L1X_IO_Init(uint8_t num_of_sensor, hwGPIO_Pin* p_pwr_pin_list, hwGPIO_Int_Pin* p_int_pin_list, VL53L1X_Interrupt_Handler callback);
VL53L1X_OpResult VL53L1X_IO_DeInit();
VL53L1X_OpResult VL53L1X_IO_Power_On(uint8_t sensor_index);
VL53L1X_OpResult VL53L1X_IO_Power_Off(uint8_t sensor_index);
VL53L1X_OpResult VL53L1X_IO_Set_I2C_Address(uint8_t sensor_index, uint8_t new_address);
VL53L1X_OpResult VL53L1X_IO_Write_Multi(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t *pdata, uint32_t count);
VL53L1X_OpResult VL53L1X_IO_Read_Multi(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t *pdata, uint32_t count);
VL53L1X_OpResult VL53L1X_IO_Write_Byte(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t data);
VL53L1X_OpResult VL53L1X_IO_Write_Word(uint8_t sensor_index, uint16_t RegisterAddr, uint16_t data);
VL53L1X_OpResult VL53L1X_IO_Write_DWord(uint8_t sensor_index, uint16_t RegisterAddr, uint32_t data);
VL53L1X_OpResult VL53L1X_IO_Read_Byte(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t *data);
VL53L1X_OpResult VL53L1X_IO_Read_Word(uint8_t sensor_index, uint16_t RegisterAddr, uint16_t *data);
VL53L1X_OpResult VL53L1X_IO_Read_DWord(uint8_t sensor_index, uint16_t RegisterAddr, uint32_t *data);
VL53L1X_OpResult VL53L1X_IO_Update_Byte(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t AndData, uint8_t OrData);

#ifdef __cplusplus
}
#endif

#endif // VL53l1X_IO_H
