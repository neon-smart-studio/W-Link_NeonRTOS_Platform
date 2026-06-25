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
 * Based on STMicroelectronics VL53L5CX driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "I2C/I2C_Master.h"

#include "VL53L5CX_Def.h"
#include "VL53L5CX_IO.h"

static uint8_t sw_i2c_address = VL53L5CX_I2C_NEW_I2C_ADDRESS;

static VL53L5CX_OpResult VL53L5CX_IO_Map_GPIO_Error(hwGPIO_OpResult error_code)
{
    switch(error_code)
    {
        case hwGPIO_OK:
            return VL53L5CX_OK;

        case hwGPIO_InvalidParameter:
            return VL53L5CX_InvalidParameter;

        case hwGPIO_PinConflict:
            return VL53L5CX_IO_Error;

        case hwGPIO_HW_Error:
            return VL53L5CX_IO_Error;

        case hwGPIO_Unsupport:
        default:
            return VL53L5CX_Unsupport;
    }
}

static VL53L5CX_OpResult VL53L5CX_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return VL53L5CX_OK;

        case hwI2C_NotInit:
            return VL53L5CX_NotInit;

        case hwI2C_InvalidParameter:
            return VL53L5CX_InvalidParameter;

        case hwI2C_MemoryError:
            return VL53L5CX_MemoryError;

        case hwI2C_MutexTimeout:
            return VL53L5CX_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return VL53L5CX_SlaveTimeout;

        case hwI2C_BusError:
            return VL53L5CX_IO_Error;

        case hwI2C_Unsupport:
        default:
            return VL53L5CX_Unsupport;
    }
}

static VL53L5CX_OpResult VL53L5CX_IO_I2C_Write(uint16_t RegisterAddr, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
    VL53L5CX_OpResult status;

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return VL53L5CX_InvalidParameter;
    }

   uint8_t* pBuf = mem_Malloc(NumByteToWrite+2);
   if(pBuf==NULL)
   {
        return VL53L5CX_MemoryError;
   }

   pBuf[0]=(uint8_t) (RegisterAddr>>8);
   pBuf[1]=(uint8_t) (RegisterAddr&0xFF);
   memcpy(&pBuf[2], pBuffer, NumByteToWrite);

   status = VL53L5CX_IO_Map_I2C_Error(
        I2C_Master_Write(
            VL53L5CX_I2C_INDEX,
            sw_i2c_address >> 1,
            pBuf,
            NumByteToWrite+2,
            true,
            VL53L5CX_I2C_OP_TIMEOUT
        )
    );

    mem_Free(pBuf);

    if(status < VL53L5CX_OK)
    {
        return status;
    }

    return VL53L5CX_OK;
}

static VL53L5CX_OpResult VL53L5CX_IO_I2C_Read(uint16_t RegisterAddr, uint8_t* pBuffer, uint16_t NumByteToRead)
{
    VL53L5CX_OpResult status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return VL53L5CX_InvalidParameter;
    }

   uint8_t buffer[2];
   buffer[0]=(uint8_t) (RegisterAddr>>8);
   buffer[1]=(uint8_t) (RegisterAddr&0xFF);

   status = VL53L5CX_IO_Map_I2C_Error(
        I2C_Master_Write(
            VL53L5CX_I2C_INDEX,
            sw_i2c_address >> 1,
            buffer,
            2,
            false,
            VL53L5CX_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L5CX_OK)
    {
        return status;
    }

    status = VL53L5CX_IO_Map_I2C_Error(
        I2C_Master_Read(
            VL53L5CX_I2C_INDEX,
            sw_i2c_address >> 1,
            pBuffer,
            NumByteToRead,
            true,
            VL53L5CX_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L5CX_OK)
    {
        return status;
    }

    return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_IO_Init()
{
   hwGPIO_OpResult gpio_status;

   gpio_status = GPIO_Pin_Init(VL53L5CX_LPN_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L5CX_IO_Map_GPIO_Error(gpio_status);
   }

   gpio_status = GPIO_Pin_Init(VL53L5CX_RST_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L5CX_IO_Map_GPIO_Error(gpio_status);
   }

   return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_IO_DeInit()
{
   hwGPIO_OpResult gpio_status;

   gpio_status = GPIO_Pin_DeInit(VL53L5CX_LPN_PIN);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L5CX_IO_Map_GPIO_Error(gpio_status);
   }

   gpio_status = GPIO_Pin_DeInit(VL53L5CX_RST_PIN);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L5CX_IO_Map_GPIO_Error(gpio_status);
   }

   return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_IO_Power_On()
{
   hwGPIO_OpResult gpio_status;

   gpio_status = GPIO_Pin_Write(VL53L5CX_LPN_PIN, true);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L5CX_IO_Map_GPIO_Error(gpio_status);
   }

   NeonRTOS_Sleep(10);

   return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_IO_I2C_Reset()
{
   hwGPIO_OpResult gpio_status;

   gpio_status = GPIO_Pin_Write(VL53L5CX_LPN_PIN, false);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L5CX_IO_Map_GPIO_Error(gpio_status);
   }

   NeonRTOS_Sleep(10);

   gpio_status = GPIO_Pin_Write(VL53L5CX_LPN_PIN, true);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L5CX_IO_Map_GPIO_Error(gpio_status);
   }

   NeonRTOS_Sleep(10);

   gpio_status = GPIO_Pin_Write(VL53L5CX_LPN_PIN, false);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L5CX_IO_Map_GPIO_Error(gpio_status);
   }

   NeonRTOS_Sleep(10);

   return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_IO_Power_Off()
{
   hwGPIO_OpResult gpio_status;

   gpio_status = GPIO_Pin_Write(VL53L5CX_LPN_PIN, false);
   if(gpio_status<hwGPIO_OK)
   {
      return VL53L5CX_IO_Map_GPIO_Error(gpio_status);
   }

   NeonRTOS_Sleep(10);

   return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_IO_Set_I2C_Address(uint8_t new_address)
{
  VL53L5CX_OpResult status;

  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x00);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  status = VL53L5CX_IO_Write_Byte(0x4, (uint8_t)(new_address >> 1));
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  sw_i2c_address = new_address;

  status = VL53L5CX_IO_Write_Byte(0x7fff, 0x02);
  if(status < VL53L5CX_OK)
  {
    return status;
  }

  return VL53L5CX_OK;
}

VL53L5CX_OpResult VL53L5CX_IO_Write_Byte(uint16_t RegisterAdress, uint8_t value)
{
  return VL53L5CX_IO_I2C_Write(RegisterAdress, &value, 1);
}

VL53L5CX_OpResult VL53L5CX_IO_Write_Bytes(uint16_t RegisterAdress, uint16_t* wr_dat, uint32_t size)
{
  return VL53L5CX_IO_I2C_Write(RegisterAdress, (uint8_t *)wr_dat, size);
}

VL53L5CX_OpResult VL53L5CX_IO_Read_Byte(uint16_t RegisterAdress, uint8_t *value)
{
  return VL53L5CX_IO_I2C_Read(RegisterAdress, value, 1);
}

VL53L5CX_OpResult VL53L5CX_IO_Read_Bytes(uint16_t RegisterAdress, uint16_t* rd_dat, uint32_t size)
{
  return VL53L5CX_IO_I2C_Read(RegisterAdress, (uint8_t *)rd_dat, size);
}
