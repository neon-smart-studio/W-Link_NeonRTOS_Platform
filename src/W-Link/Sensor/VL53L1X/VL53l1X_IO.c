/**
 ******************************************************************************
 * @file    vl53l0x_class.cpp
 * @author  IMG
 * @version V0.0.1
 * @date    14-December-2018
 * @brief   Implementation file for the VL53L1X driver class
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
 * Based on STMicroelectronics VL53L1X driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "I2C/I2C_Master.h"

#include "VL53L1X_Def.h"
#include "VL53L1X_IO.h"

static uint8_t g_num_of_sensor = 0;
static hwGPIO_Pin* p_power_pin_list = NULL;
static hwGPIO_Int_Pin* p_interrupt_pin_list = NULL;
static uint8_t* p_sw_i2c_address_list = NULL;

static VL53L1X_Interrupt_Handler sensor_int_callback = NULL;

static VL53L1X_OpResult VL53L1X_IO_Map_GPIO_Error(hwGPIO_OpResult error_code)
{
    switch(error_code)
    {
        case hwGPIO_OK:
            return VL53L1X_OK;

        case hwGPIO_InvalidParameter:
            return VL53L1X_InvalidParameter;

        case hwGPIO_PinConflict:
            return VL53L1X_IO_Error;

        case hwGPIO_HW_Error:
            return VL53L1X_IO_Error;

        case hwGPIO_Unsupport:
        default:
            return VL53L1X_Unsupport;
    }
}

static VL53L1X_OpResult VL53L1X_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return VL53L1X_OK;

        case hwI2C_NotInit:
            return VL53L1X_NotInit;

        case hwI2C_InvalidParameter:
            return VL53L1X_InvalidParameter;

        case hwI2C_MemoryError:
            return VL53L1X_MemoryError;

        case hwI2C_MutexTimeout:
            return VL53L1X_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return VL53L1X_SlaveTimeout;

        case hwI2C_BusError:
            return VL53L1X_IO_Error;

        case hwI2C_Unsupport:
        default:
            return VL53L1X_Unsupport;
    }
}

static VL53L1X_OpResult VL53L1X_IO_I2C_Write(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
    VL53L1X_OpResult status;

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return VL53L1X_InvalidParameter;
    }

    if(p_sw_i2c_address_list == NULL)
    {
        return VL53L1X_NotInit;
    }

    if(sensor_index >= g_num_of_sensor)
    {
        return VL53L1X_InvalidParameter;
    }

    uint8_t buffer[NumByteToWrite+2];
    buffer[0]=(uint8_t) (RegisterAddr>>8);
    buffer[1]=(uint8_t) (RegisterAddr&0xFF);
    memcpy(&buffer[2], pBuffer, NumByteToWrite);

    status = VL53L1X_IO_Map_I2C_Error(
        I2C_Master_Write(
            VL53L1X_I2C_INDEX,
            p_sw_i2c_address_list[sensor_index] >> 1,
            buffer,
            NumByteToWrite+2,
            true,
            VL53L1X_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L1X_OK)
    {
        return status;
    }

    return VL53L1X_OK;
}

static VL53L1X_OpResult VL53L1X_IO_I2C_Read(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t* pBuffer, uint16_t NumByteToRead)
{
    VL53L1X_OpResult status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return VL53L1X_InvalidParameter;
    }

    if(p_sw_i2c_address_list == NULL)
    {
        return VL53L1X_NotInit;
    }

    if(sensor_index >= g_num_of_sensor)
    {
        return VL53L1X_InvalidParameter;
    }

    uint8_t buffer[2];
    buffer[0]=(uint8_t) (RegisterAddr>>8);
    buffer[1]=(uint8_t) (RegisterAddr&0xFF);

    status = VL53L1X_IO_Map_I2C_Error(
        I2C_Master_Write(
            VL53L1X_I2C_INDEX,
            p_sw_i2c_address_list[sensor_index] >> 1,
            buffer,
            2,
            false,
            VL53L1X_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L1X_OK)
    {
        return status;
    }

    status = VL53L1X_IO_Map_I2C_Error(
        I2C_Master_Read(
            VL53L1X_I2C_INDEX,
            p_sw_i2c_address_list[sensor_index] >> 1,
            pBuffer,
            NumByteToRead,
            true,
            VL53L1X_I2C_OP_TIMEOUT
        )
    );

    if(status < VL53L1X_OK)
    {
        return status;
    }

    return VL53L1X_OK;
}

static void VL53L1X_GPIO_Int_Event_PendingFunctionCall(void* p1, uint32_t p2)
{
    uint8_t sensor_index = (uint8_t)p2;

    if(sensor_int_callback!=NULL)
    {
        sensor_int_callback(sensor_index);
    }
}

static void VL53L1X_GPIO_Int_Event_Handler(hwGPIO_Int_Pin pin, hwGPIO_Interrupt_Action action)
{
    int sensor_index = -1;

    for(uint8_t i = 0; i<g_num_of_sensor; i++)
    {
        if(pin==p_interrupt_pin_list[i])
        {
            sensor_index = i;
            break;
        }
    }

    if(sensor_index>=0)
    {
        NeonRTOS_PendingFunctionCall(VL53L1X_GPIO_Int_Event_PendingFunctionCall, NULL, sensor_index);
    }
}

VL53L1X_OpResult VL53L1X_IO_Init(uint8_t num_of_sensor, hwGPIO_Pin* p_pwr_pin_list, hwGPIO_Int_Pin* p_int_pin_list, VL53L1X_Interrupt_Handler callback)
{
    hwGPIO_OpResult gpio_status;

    VL53L1X_OpResult status;
   
    g_num_of_sensor = num_of_sensor;

    p_power_pin_list = mem_Malloc(sizeof(hwGPIO_Pin)*g_num_of_sensor);
    if(p_power_pin_list==NULL)
    {
        g_num_of_sensor = 0;
        return VL53L1X_MemoryError;
    }

    p_interrupt_pin_list = mem_Malloc(sizeof(hwGPIO_Int_Pin)*g_num_of_sensor);
    if(p_interrupt_pin_list==NULL)
    {
        g_num_of_sensor = 0;
        mem_Free(p_power_pin_list);
        return VL53L1X_MemoryError;
    }

    p_sw_i2c_address_list = mem_Malloc(sizeof(uint8_t)*g_num_of_sensor);
    if(p_sw_i2c_address_list==NULL)
    {
        g_num_of_sensor = 0;
        mem_Free(p_power_pin_list);
        mem_Free(p_interrupt_pin_list);
        return VL53L1X_MemoryError;
    }
    
    for(uint8_t i = 0; i<g_num_of_sensor; i++)
    {
        p_sw_i2c_address_list[i] = VL53L1X_ACC_I2C_ADDRESS;
        p_power_pin_list[i] = p_pwr_pin_list[i];
        p_interrupt_pin_list[i] = p_int_pin_list[i];
    }

    for(uint8_t i = 0; i<g_num_of_sensor; i++)
    {
        gpio_status = GPIO_Pin_Init(p_power_pin_list[i], hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
        if(gpio_status<hwGPIO_OK)
        {
            g_num_of_sensor = 0;
            mem_Free(p_power_pin_list);
            mem_Free(p_sw_i2c_address_list);
            mem_Free(p_interrupt_pin_list);
            return VL53L1X_IO_Map_GPIO_Error(gpio_status);
        }
        
        if(callback!=NULL)
        {
            gpio_status = GPIO_Interrupt_Init(p_interrupt_pin_list[i], hwGPIO_Interrupt_Mode_Falling_Edge);
            if(gpio_status<hwGPIO_OK)
            {
                g_num_of_sensor = 0;
                mem_Free(p_power_pin_list);
                mem_Free(p_sw_i2c_address_list);
                mem_Free(p_interrupt_pin_list);
                return VL53L1X_IO_Map_GPIO_Error(gpio_status);
            }
            
            gpio_status = GPIO_Register_Interrupt_Handler(p_interrupt_pin_list[i], VL53L1X_GPIO_Int_Event_Handler);
            if(gpio_status<hwGPIO_OK)
            {
                g_num_of_sensor = 0;
                mem_Free(p_power_pin_list);
                mem_Free(p_sw_i2c_address_list);
                mem_Free(p_interrupt_pin_list);
                return VL53L1X_IO_Map_GPIO_Error(gpio_status);
            }
            
            gpio_status = GPIO_Interrupt_Enable(p_interrupt_pin_list[i]);
            if(gpio_status<hwGPIO_OK)
            {
                g_num_of_sensor = 0;
                mem_Free(p_power_pin_list);
                mem_Free(p_sw_i2c_address_list);
                mem_Free(p_interrupt_pin_list);
                return VL53L1X_IO_Map_GPIO_Error(gpio_status);
            }
        }
    }

    sensor_int_callback = callback;

    return VL53L1X_OK;
}

VL53L1X_OpResult VL53L1X_IO_DeInit()
{
   hwGPIO_OpResult gpio_status;

    if(p_power_pin_list == NULL)
    {
        return VL53L1X_NotInit;
    }

    if(p_sw_i2c_address_list == NULL)
    {
        return VL53L1X_NotInit;
    }

    if(p_interrupt_pin_list == NULL)
    {
        return VL53L1X_NotInit;
    }

    for(uint8_t i = 0; i<g_num_of_sensor; i++)
    {
        gpio_status = GPIO_Pin_DeInit(p_power_pin_list[i]);
        if(gpio_status<hwGPIO_OK)
        {
            return VL53L1X_IO_Map_GPIO_Error(gpio_status);
        }
        
        if(sensor_int_callback!=NULL)
        {
            gpio_status = GPIO_Interrupt_Disable(p_interrupt_pin_list[i]);
            if(gpio_status<hwGPIO_OK)
            {
                return VL53L1X_IO_Map_GPIO_Error(gpio_status);
            }

            gpio_status = GPIO_Interrupt_DeInit(p_interrupt_pin_list[i]);
            if(gpio_status<hwGPIO_OK)
            {
                return VL53L1X_IO_Map_GPIO_Error(gpio_status);
            }
        }
    }

    mem_Free(p_power_pin_list);
    mem_Free(p_sw_i2c_address_list);
    mem_Free(p_interrupt_pin_list);

    g_num_of_sensor = 0;

    sensor_int_callback = NULL;

    return VL53L1X_OK;
}

VL53L1X_OpResult VL53L1X_IO_Power_On(uint8_t sensor_index)
{
    hwGPIO_OpResult gpio_status;

    if(p_power_pin_list == NULL)
    {
        return VL53L1X_NotInit;
    }

    gpio_status = GPIO_Pin_Write(p_power_pin_list[sensor_index], true);
    if(gpio_status<hwGPIO_OK)
    {
        return VL53L1X_IO_Map_GPIO_Error(gpio_status);
    }

    NeonRTOS_Sleep(10);

    return VL53L1X_OK;
}

VL53L1X_OpResult VL53L1X_IO_Power_Off(uint8_t sensor_index)
{
    hwGPIO_OpResult gpio_status;

    if(p_power_pin_list == NULL)
    {
        return VL53L1X_NotInit;
    }

    gpio_status = GPIO_Pin_Write(p_power_pin_list[sensor_index], false);
    if(gpio_status<hwGPIO_OK)
    {
        return VL53L1X_IO_Map_GPIO_Error(gpio_status);
    }

    NeonRTOS_Sleep(10);

    return VL53L1X_OK;
}

VL53L1X_OpResult VL53L1X_IO_Set_I2C_Address(uint8_t sensor_index, uint8_t new_address)
{
   VL53L1X_OpResult status;
   
   status = VL53L1X_IO_Write_Byte(sensor_index, VL53L1X_I2C_SLAVE__DEVICE_ADDRESS, new_address >> 1);
   if(status < VL53L1X_OK)
   {
      return status;
   }

   p_sw_i2c_address_list[sensor_index] = new_address;

   return VL53L1X_OK;
}

VL53L1X_OpResult VL53L1X_IO_Write_Multi(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t *pdata, uint32_t count)
{
   return VL53L1X_IO_I2C_Write(sensor_index, RegisterAddr, pdata, (uint16_t)count);
}

VL53L1X_OpResult VL53L1X_IO_Read_Multi(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t *pdata, uint32_t count)
{
   return VL53L1X_IO_I2C_Read(sensor_index, RegisterAddr, pdata, (uint16_t)count);
}

VL53L1X_OpResult VL53L1X_IO_Write_Byte(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t data)
{
   return VL53L1X_IO_I2C_Write(sensor_index, RegisterAddr, &data, 1);
}

VL53L1X_OpResult VL53L1X_IO_Write_Word(uint8_t sensor_index, uint16_t RegisterAddr, uint16_t data)
{
   uint8_t buffer[2];

   buffer[0] = data >> 8;
   buffer[1] = data & 0x00FF;

   return VL53L1X_IO_I2C_Write(sensor_index, RegisterAddr, (uint8_t *)buffer, 2);
}

VL53L1X_OpResult VL53L1X_IO_Write_DWord(uint8_t sensor_index, uint16_t RegisterAddr, uint32_t data)
{
   uint8_t buffer[4];

   buffer[0] = (data >> 24) & 0xFF;
   buffer[1] = (data >> 16) & 0xFF;
   buffer[2] = (data >>  8) & 0xFF;
   buffer[3] = (data >>  0) & 0xFF;

   return VL53L1X_IO_I2C_Write(sensor_index, RegisterAddr, (uint8_t *)buffer, 4);
}

VL53L1X_OpResult VL53L1X_IO_Read_Byte(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t *data)
{
   return VL53L1X_IO_I2C_Read(sensor_index, RegisterAddr, data, 1);
}

VL53L1X_OpResult VL53L1X_IO_Read_Word(uint8_t sensor_index, uint16_t RegisterAddr, uint16_t *data)
{
   VL53L1X_OpResult status;
   uint8_t buffer[2] = {0,0};

   status = VL53L1X_IO_I2C_Read(sensor_index, RegisterAddr, buffer, 2);
   if (status<VL53L1X_OK)
   {
      return status;
   }

   *data = (buffer[0] << 8) + buffer[1];

   return VL53L1X_OK;
}

VL53L1X_OpResult VL53L1X_IO_Read_DWord(uint8_t sensor_index, uint16_t RegisterAddr, uint32_t *data)
{
   VL53L1X_OpResult status;
   uint8_t buffer[4] = {0,0,0,0};

   status = VL53L1X_IO_I2C_Read(sensor_index, RegisterAddr, buffer, 4);
   if (status<VL53L1X_OK)
   {
      return status;
   }
   
   *data = ((uint32_t)buffer[0] << 24) + ((uint32_t)buffer[1] << 16) + ((uint32_t)buffer[2] << 8) + (uint32_t)buffer[3];
   
   return VL53L1X_OK;
}

VL53L1X_OpResult VL53L1X_IO_Update_Byte(uint8_t sensor_index, uint16_t RegisterAddr, uint8_t AndData, uint8_t OrData)
{
   VL53L1X_OpResult status;
   uint8_t buffer = 0;

   /* read data direct onto buffer */
   status = VL53L1X_IO_I2C_Read(sensor_index, RegisterAddr, &buffer, 1);
   if (status<VL53L1X_OK)
   {
      return status;
   }
   
   buffer = (buffer & AndData) | OrData;

   return VL53L1X_IO_I2C_Write(sensor_index, RegisterAddr, &buffer, (uint16_t)1);
}
