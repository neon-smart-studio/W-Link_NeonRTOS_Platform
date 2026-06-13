/**
 ******************************************************************************
 * @file    platform.cpp
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    29 November 2021
 * @brief   Implementation of the platform dependent APIs.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "I2C/I2C_Master.h"

#include "VL53L4CD_Def.h"
#include "VL53L4CD_IO.h"

static sw_i2c_address = VL53L4CD_I2C_NEW_I2C_ADDRESS;

static VL53L4CD_OpResult VL53L4CD_IO_Map_GPIO_Error(hwGPIO_OpResult error_code)
{
    switch(error_code)
    {
        case hwGPIO_OK:
            return VL53L4CD_OK;

        case hwGPIO_InvalidParameter:
            return VL53L4CD_InvalidParameter;

        case hwGPIO_PinConflict:
            return VL53L4CD_IO_Error;

        case hwGPIO_HW_Error:
            return VL53L4CD_IO_Error;

        case hwGPIO_Unsupport:
        default:
            return VL53L4CD_Unsupport;
    }
}

static VL53L4CD_OpResult VL53L4CD_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return VL53L4CD_OK;

        case hwI2C_NotInit:
            return VL53L4CD_NotInit;

        case hwI2C_InvalidParameter:
            return VL53L4CD_InvalidParameter;

        case hwI2C_MemoryError:
            return VL53L4CD_MemoryError;

        case hwI2C_MutexTimeout:
            return VL53L4CD_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return VL53L4CD_SlaveTimeout;

        case hwI2C_BusError:
            return VL53L4CD_IO_Error;

        case hwI2C_Unsupport:
        default:
            return VL53L4CD_Unsupport;
    }
}

static VL53L4CD_OpResult VL53L4CD_IO_I2C_Write(uint16_t RegisterAddr, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
    VL53L4CD_OpResult status;

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return VL53L4CD_InvalidParameter;
    }

   uint8_t buffer[2];
   buffer[0]=(uint8_t) (RegisterAddr>>8);
   buffer[1]=(uint8_t) (RegisterAddr&0xFF);

   status = VL53L4CD_IO_Map_I2C_Error(
        I2C_Master_Write(
            VL53L4CD_I2C_INDEX,
            sw_i2c_address >> 1,
            buffer,
            2,
            false,
            VL53L4CD_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L4CD_OK)
    {
        return status;
    }

    status = VL53L4CD_IO_Map_I2C_Error(
        I2C_Master_Read(
            VL53L4CD_I2C_INDEX,
            sw_i2c_address >> 1,
            pBuffer,
            NumByteToWrite,
            true,
            VL53L4CD_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L4CD_OK)
    {
        return status;
    }

    return VL53L4CD_OK;
}

static VL53L4CD_OpResult VL53L4CD_IO_I2C_Read(uint16_t RegisterAddr, uint8_t* pBuffer, uint16_t NumByteToRead)
{
    VL53L4CD_OpResult status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return VL53L4CD_InvalidParameter;
    }

   uint8_t buffer[2];
   buffer[0]=(uint8_t) (RegisterAddr>>8);
   buffer[1]=(uint8_t) (RegisterAddr&0xFF);

   status = VL53L4CD_IO_Map_I2C_Error(
        I2C_Master_Write(
            VL53L4CD_I2C_INDEX,
            sw_i2c_address >> 1,
            buffer,
            2,
            false,
            VL53L4CD_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L4CD_OK)
    {
        return status;
    }

    status = VL53L4CD_IO_Map_I2C_Error(
        I2C_Master_Read(
            VL53L4CD_I2C_INDEX,
            sw_i2c_address >> 1,
            pBuffer,
            NumByteToRead,
            true,
            VL53L4CD_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L4CD_OK)
    {
        return status;
    }

    return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_IO_Init()
{
   hwGPIO_OpResult gpio_status;

   gpio_status = GPIO_Pin_Init(VL53L4CD_POWER_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L4CD_IO_Map_GPIO_Error(gpio_status);
   }

   return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_IO_DeInit()
{
   hwGPIO_OpResult gpio_status;

   gpio_status = GPIO_Pin_DeInit(VL53L4CD_POWER_PIN);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L4CD_IO_Map_GPIO_Error(gpio_status);
   }

   return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_IO_Power_On()
{
   hwGPIO_OpResult gpio_status;

   gpio_status = GPIO_Pin_Write(VL53L4CD_POWER_PIN, true);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L4CD_IO_Map_GPIO_Error(gpio_status);
   }

   NeonRTOS_Sleep(10);

   return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_IO_Power_Off()
{
   hwGPIO_OpResult gpio_status;

   gpio_status = GPIO_Pin_Write(VL53L4CD_POWER_PIN, false);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L4CD_IO_Map_GPIO_Error(gpio_status);
   }

   NeonRTOS_Sleep(10);

   return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_IO_SetI2CAddress(uint8_t new_address)
{
  sw_i2c_address = new_address;
  return VL53L4CD_IO_SetI2CAddress(new_address);
}

VL53L4CD_OpResult VL53L4CD_IO_Write_Byte(uint16_t RegisterAdress, uint8_t value)
{
  return VL53L4CD_IO_I2C_Write(RegisterAdress, &value, 1);
}

VL53L4CD_OpResult VL53L4CD_IO_Write_Word(uint16_t RegisterAdress, uint16_t value)
{
  uint8_t buffer[2];

  buffer[0] = value >> 8;
  buffer[1] = value & 0x00FF;

  return VL53L4CD_IO_I2C_Read(RegisterAdress, (uint8_t *)buffer, 2);
}

VL53L4CD_OpResult VL53L4CD_IO_Write_DWord(uint16_t RegisterAdress, uint32_t value)
{
  uint8_t buffer[4];

  buffer[0] = (value >> 24) & 0xFF;
  buffer[1] = (value >> 16) & 0xFF;
  buffer[2] = (value >>  8) & 0xFF;
  buffer[3] = (value >>  0) & 0xFF;

  return VL53L4CD_IO_I2C_Write(RegisterAdress, (uint8_t *)buffer, 4);
}

VL53L4CD_OpResult VL53L4CD_IO_Read_Byte(uint16_t RegisterAdress, uint8_t *value)
{
  return VL53L4CD_IO_I2C_Read(RegisterAdress, value, 1);
}

VL53L4CD_OpResult VL53L4CD_IO_Read_Word(uint16_t RegisterAdress, uint16_t *value)
{
  VL53L4CD_OpResult status;
  uint8_t buffer[2] = {0, 0};

  status = VL53L4CD_IO_I2C_Read(RegisterAdress, buffer, 2);
  if(status < VL53L4CD_OK)
  {
    return status;
  }
  
  *value = (buffer[0] << 8) + buffer[1];

  return VL53L4CD_OK;
}

VL53L4CD_OpResult VL53L4CD_IO_Read_DWord(uint16_t RegisterAdress, uint32_t *value)
{
  VL53L4CD_OpResult status;
  uint8_t buffer[4] = {0, 0, 0, 0};

  status = VL53L4CD_IO_I2C_Read(RegisterAdress, buffer, 4);
  if(status < VL53L4CD_OK)
  {
    return status;
  }

  *value = ((uint32_t)buffer[0] << 24) + ((uint32_t)buffer[1] << 16) + ((uint32_t)buffer[2] << 8) + (uint32_t)buffer[3];
  
  return VL53L4CD_OK;
}
