/**
 ******************************************************************************
 * @file    VL53L5CX_api.h
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    29 November 2021
 * @brief   Header file for the VL53L5CX main structures.
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

#ifndef VL53L5CX_IO_H
#define VL53L5CX_IO_H

#include <stdint.h>
#include <stdbool.h>

#include "GPIO/GPIO.h"

#include "VL53L5CX_Def.h"

#include "Sensor_Config.h"

#define VL53L5CX_DEFAULT_I2C_ADDRESS        0x52

#define VL53L5CX_I2C_OP_TIMEOUT             500

#ifndef CONFIG_VL53L5CX_I2C_INDEX
#define VL53L5CX_I2C_INDEX                  hwI2C_Index_0
#else
#define VL53L5CX_I2C_INDEX                  CONFIG_VL53L5CX_I2C_INDEX
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*VL53L5CX_Interrupt_Handler)();

VL53L5CX_OpResult VL53L5CX_IO_Init(hwGPIO_Pin lpn_pin, hwGPIO_Int_Pin int_pin, VL53L5CX_Interrupt_Handler callback);
VL53L5CX_OpResult VL53L5CX_IO_DeInit();
VL53L5CX_OpResult VL53L5CX_IO_Power_On();
VL53L5CX_OpResult VL53L5CX_IO_Power_Off();
VL53L5CX_OpResult VL53L5CX_IO_I2C_Reset();
VL53L5CX_OpResult VL53L5CX_IO_Set_I2C_Address(uint8_t new_address);
VL53L5CX_OpResult VL53L5CX_IO_Write_Byte(uint16_t RegisterAdress, uint8_t value);
VL53L5CX_OpResult VL53L5CX_IO_Write_Bytes(uint16_t RegisterAdress, uint8_t* wr_dat, uint32_t size);
VL53L5CX_OpResult VL53L5CX_IO_Read_Byte(uint16_t RegisterAdress, uint8_t *value);
VL53L5CX_OpResult VL53L5CX_IO_Read_Bytes(uint16_t RegisterAdress, uint8_t* rd_dat, uint32_t size);
VL53L5CX_OpResult VL53L5CX_IO_Write_Fw_Block(uint8_t page, const uint8_t *fw, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif  //VL53L5CX_IO_H
