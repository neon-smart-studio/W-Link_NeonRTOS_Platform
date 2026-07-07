/**
 ******************************************************************************
 * @file    vl53l0x_class.cpp
 * @author  IMG
 * @version V0.0.1
 * @date    14-December-2018
 * @brief   Implementation file for the VL53L0X driver class
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2018 STMicroelectronics</center></h2>
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
 * Based on STMicroelectronics VL53L0X driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "I2C/I2C_Master.h"

#include "VL53L0X_Def.h"
#include "VL53L0X_IO.h"

static uint8_t g_num_of_sensor = 0;
static uint8_t* p_power_pin_list = NULL;
static uint8_t* p_sw_i2c_address_list = NULL;

static VL53L0X_OpResult VL53L0X_IO_Map_GPIO_Error(hwGPIO_OpResult error_code)
{
    switch(error_code)
    {
        case hwGPIO_OK:
            return VL53L0X_OK;

        case hwGPIO_InvalidParameter:
            return VL53L0X_InvalidParameter;

        case hwGPIO_PinConflict:
            return VL53L0X_IO_Error;

        case hwGPIO_HW_Error:
            return VL53L0X_IO_Error;

        case hwGPIO_Unsupport:
        default:
            return VL53L0X_Unsupport;
    }
}

static VL53L0X_OpResult VL53L0X_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return VL53L0X_OK;

        case hwI2C_NotInit:
            return VL53L0X_NotInit;

        case hwI2C_InvalidParameter:
            return VL53L0X_InvalidParameter;

        case hwI2C_MemoryError:
            return VL53L0X_MemoryError;

        case hwI2C_MutexTimeout:
            return VL53L0X_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return VL53L0X_SlaveTimeout;

        case hwI2C_BusError:
            return VL53L0X_IO_Error;

        case hwI2C_Unsupport:
        default:
            return VL53L0X_Unsupport;
    }
}

static VL53L0X_OpResult VL53L0X_IO_I2C_Write(uint8_t sensor_index, uint8_t RegisterAddr, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
    VL53L0X_OpResult status;

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return VL53L0X_InvalidParameter;
    }

    if(p_sw_i2c_address_list == NULL)
    {
        return VL53L0X_NotInit;
    }

    if(sensor_index >= g_num_of_sensor)
    {
        return VL53L0X_InvalidParameter;
    }

    uint8_t buffer[NumByteToWrite+1];
    buffer[0]=(uint8_t) (RegisterAddr&0xFF);
    memcpy(&buffer[1], pBuffer, NumByteToWrite);

    status = VL53L0X_IO_Map_I2C_Error(
        I2C_Master_Write(
            VL53L0X_I2C_INDEX,
            p_sw_i2c_address_list[sensor_index] >> 1,
            buffer,
            NumByteToWrite+1,
            true,
            VL53L0X_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L0X_OK)
    {
        return status;
    }

    return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_IO_I2C_Read(uint8_t sensor_index, uint8_t RegisterAddr, uint8_t* pBuffer, uint16_t NumByteToRead)
{
    VL53L0X_OpResult status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return VL53L0X_InvalidParameter;
    }

    if(p_sw_i2c_address_list == NULL)
    {
        return VL53L0X_NotInit;
    }

    if(sensor_index >= g_num_of_sensor)
    {
        return VL53L0X_InvalidParameter;
    }

    uint8_t buffer[1];
    buffer[0]=(uint8_t) (RegisterAddr&0xFF);

    status = VL53L0X_IO_Map_I2C_Error(
        I2C_Master_Write(
            VL53L0X_I2C_INDEX,
            p_sw_i2c_address_list[sensor_index] >> 1,
            buffer,
            1,
            false,
            VL53L0X_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L0X_OK)
    {
        return status;
    }

    status = VL53L0X_IO_Map_I2C_Error(
        I2C_Master_Read(
            VL53L0X_I2C_INDEX,
            p_sw_i2c_address_list[sensor_index] >> 1,
            pBuffer,
            NumByteToRead,
            true,
            VL53L0X_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L0X_OK)
    {
        return status;
    }

    return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_IO_Power_On(uint8_t sensor_index)
{
    hwGPIO_OpResult gpio_status;

    if(p_power_pin_list == NULL)
    {
        return VL53L0X_NotInit;
    }

    gpio_status = GPIO_Pin_Write(p_power_pin_list[sensor_index], true);
    if(gpio_status<hwGPIO_OK)
    {
        return VL53L0X_IO_Map_GPIO_Error(gpio_status);
    }

    NeonRTOS_Sleep(10);

    return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_IO_Power_Off(uint8_t sensor_index)
{
    hwGPIO_OpResult gpio_status;

    if(p_power_pin_list == NULL)
    {
        return VL53L0X_NotInit;
    }

    gpio_status = GPIO_Pin_Write(p_power_pin_list[sensor_index], false);
    if(gpio_status<hwGPIO_OK)
    {
        return VL53L0X_IO_Map_GPIO_Error(gpio_status);
    }

    NeonRTOS_Sleep(10);

    return VL53L0X_OK;
}

static VL53L0X_OpResult VL53L0X_IO_SetI2CAddress(uint8_t sensor_index)
{
   VL53L0X_OpResult status;
   
   status = VL53L0X_IO_Write_Byte(sensor_index, VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS, p_sw_i2c_address_list[sensor_index] >> 1);
   if(status < VL53L0X_OK)
   {
      return status;
   }

   return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_IO_Init(uint8_t num_of_sensor, uint8_t* p_i2x_addr_list, hwGPIO_Pin* p_pwr_pin_list)
{
    hwGPIO_OpResult gpio_status;

    VL53L0X_OpResult status;
   
    g_num_of_sensor = num_of_sensor;

    p_power_pin_list = mem_Malloc(sizeof(hwGPIO_Pin)*g_num_of_sensor);
    if(p_power_pin_list==NULL)
    {
        g_num_of_sensor = 0;
        return VL53L0X_MemoryError;
    }

    p_sw_i2c_address_list = mem_Malloc(sizeof(uint8_t)*g_num_of_sensor);
    if(p_sw_i2c_address_list==NULL)
    {
        g_num_of_sensor = 0;
        mem_Free(p_power_pin_list);
        return VL53L0X_MemoryError;
    }
    
    for(uint8_t i = 0; i<g_num_of_sensor; i++)
    {
        p_sw_i2c_address_list[i] = VL53L0X_ACC_I2C_ADDRESS;
    }

    for(uint8_t i = 0; i<g_num_of_sensor; i++)
    {
        gpio_status = GPIO_Pin_Init(p_power_pin_list[i], hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
        if(gpio_status<hwGPIO_OK)
        {
            g_num_of_sensor = 0;
            mem_Free(p_power_pin_list);
            mem_Free(p_sw_i2c_address_list);
            return VL53L0X_IO_Map_GPIO_Error(gpio_status);
        }
    }

    for(uint8_t i = 0; i<g_num_of_sensor; i++)
    {
        status = VL53L0X_IO_Power_Off(i);
        if(status < VL53L0X_OK)
        {
            g_num_of_sensor = 0;
            mem_Free(p_power_pin_list);
            mem_Free(p_sw_i2c_address_list);
            return status;
        }
    }

    for(uint8_t i = 0; i<g_num_of_sensor; i++)
    {
        status = VL53L0X_IO_Power_On(i);
        if(status < VL53L0X_OK)
        {
            g_num_of_sensor = 0;
            mem_Free(p_power_pin_list);
            mem_Free(p_sw_i2c_address_list);
            return status;
        }

        status = VL53L0X_IO_SetI2CAddress(i);
        if(status < VL53L0X_OK)
        {
            g_num_of_sensor = 0;
            mem_Free(p_power_pin_list);
            mem_Free(p_sw_i2c_address_list);
            return status;
        }
    }

    return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_IO_DeInit()
{
   hwGPIO_OpResult gpio_status;

    if(p_power_pin_list == NULL)
    {
        return VL53L0X_NotInit;
    }

    if(p_sw_i2c_address_list == NULL)
    {
        return VL53L0X_NotInit;
    }

    for(uint8_t i = 0; i<g_num_of_sensor; i++)
    {
        gpio_status = GPIO_Pin_DeInit(p_power_pin_list[i]);
        if(gpio_status<hwGPIO_OK)
        {
            return VL53L0X_IO_Map_GPIO_Error(gpio_status);
        }
    }

    mem_Free(p_power_pin_list);
    mem_Free(p_sw_i2c_address_list);

    g_num_of_sensor = 0;

    return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_IO_Write_Multi(uint8_t sensor_index, uint8_t RegisterAddr, uint8_t *pdata, uint32_t count)
{
   return VL53L0X_IO_I2C_Write(sensor_index, RegisterAddr, pdata, (uint16_t)count);
}

VL53L0X_OpResult VL53L0X_IO_Read_Multi(uint8_t sensor_index, uint8_t RegisterAddr, uint8_t *pdata, uint32_t count)
{
   return VL53L0X_IO_I2C_Read(sensor_index, RegisterAddr, pdata, (uint16_t)count);
}

VL53L0X_OpResult VL53L0X_IO_Write_Byte(uint8_t sensor_index, uint8_t RegisterAddr, uint8_t data)
{
   return VL53L0X_IO_I2C_Write(sensor_index, RegisterAddr, &data, 1);
}

VL53L0X_OpResult VL53L0X_IO_Write_Word(uint8_t sensor_index, uint8_t RegisterAddr, uint16_t data)
{
   uint8_t buffer[2];

   buffer[0] = data >> 8;
   buffer[1] = data & 0x00FF;

   return VL53L0X_IO_I2C_Write(sensor_index, RegisterAddr, (uint8_t *)buffer, 2);
}

VL53L0X_OpResult VL53L0X_IO_Write_DWord(uint8_t sensor_index, uint8_t RegisterAddr, uint32_t data)
{
   uint8_t buffer[4];

   buffer[0] = (data >> 24) & 0xFF;
   buffer[1] = (data >> 16) & 0xFF;
   buffer[2] = (data >>  8) & 0xFF;
   buffer[3] = (data >>  0) & 0xFF;

   return VL53L0X_IO_I2C_Write(sensor_index, RegisterAddr, (uint8_t *)buffer, 4);
}

VL53L0X_OpResult VL53L0X_IO_Read_Byte(uint8_t sensor_index, uint8_t RegisterAddr, uint8_t *data)
{
   return VL53L0X_IO_I2C_Read(sensor_index, RegisterAddr, data, 1);
}

VL53L0X_OpResult VL53L0X_IO_Read_Word(uint8_t sensor_index, uint8_t RegisterAddr, uint16_t *data)
{
   VL53L0X_OpResult status;
   uint8_t buffer[2] = {0,0};

   status = VL53L0X_IO_I2C_Read(sensor_index, RegisterAddr, buffer, 2);
   if (status<VL53L0X_OK)
   {
      return status;
   }

   *data = (buffer[0] << 8) + buffer[1];

   return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_IO_Read_DWord(uint8_t sensor_index, uint8_t RegisterAddr, uint32_t *data)
{
   VL53L0X_OpResult status;
   uint8_t buffer[4] = {0,0,0,0};

   status = VL53L0X_IO_I2C_Read(sensor_index, RegisterAddr, buffer, 4);
   if (status<VL53L0X_OK)
   {
      return status;
   }
   
   *data = ((uint32_t)buffer[0] << 24) + ((uint32_t)buffer[1] << 16) + ((uint32_t)buffer[2] << 8) + (uint32_t)buffer[3];
   
   return VL53L0X_OK;
}

VL53L0X_OpResult VL53L0X_IO_Update_Byte(uint8_t sensor_index, uint8_t RegisterAddr, uint8_t AndData, uint8_t OrData)
{
   VL53L0X_OpResult status;
   uint8_t buffer = 0;

   /* read data direct onto buffer */
   status = VL53L0X_IO_I2C_Read(sensor_index, RegisterAddr, &buffer, 1);
   if (status<VL53L0X_OK)
   {
      return status;
   }
   
   buffer = (buffer & AndData) | OrData;

   return VL53L0X_IO_I2C_Write(sensor_index, RegisterAddr, &buffer, (uint16_t)1);
}
