/**
  ******************************************************************************
  * @file    m24sr.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions to interface with the M24SR
  *          device.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/*
 * Based on STMicroelectronics M24SR driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdint.h>

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "I2C/I2C_Master.h"

#include "NFC/NFC_Def.h"

#include "M24SR_IO.h"

static M24SR_GPO_Event_Handler M24SR_GPO_Event_Handler_CB = NULL;

static NFC_OpResult NFC_M24SR_Map_GPIO_Error_Code(hwGPIO_OpResult error_code)
{
    switch (error_code)
    {
        case hwGPIO_OK:
            return NFC_OK;

        case hwGPIO_InvalidParameter:
            return NFC_InvalidParameter;

        case hwGPIO_PinConflict:
            return NFC_IO_Error;

        case hwGPIO_HW_Error:
            return NFC_IO_Error;

        case hwGPIO_Unsupport:
            return NFC_Unsupport;

        default:
            return NFC_IO_Error;
    }
}

static NFC_OpResult NFC_M24SR_Map_I2C_Error_Code(hwI2C_OpResult error_code)
{
    switch (error_code)
    {
        case hwI2C_OK:
            return NFC_OK;

        case hwI2C_NotInit:
            return NFC_NotInit;

        case hwI2C_InvalidParameter:
            return NFC_InvalidParameter;

        case hwI2C_MemoryError:
            return NFC_MemoryError;

        case hwI2C_MutexTimeout:
            return NFC_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return NFC_SlaveTimeout;

        default:
            return NFC_IO_Error;
    }
}

void M24SR_IO_Event_Handler(void *pParameter1, uint32_t uParameter2)
{
    if(M24SR_GPO_Event_Handler_CB!=NULL)
    {
        M24SR_GPO_Event_Handler_CB();
    }
}

void M24SR_IO_Interrupt_Handler(hwGPIO_Int_Pin pin, hwGPIO_Interrupt_Action action)
{
    NeonRTOS_PendingFunctionCall(M24SR_IO_Event_Handler, NULL, 0);
}

NFC_OpResult M24SR_IO_Init(M24SR_GPO_Event_Handler cb)
{
    hwGPIO_OpResult gpio_op_status;

    gpio_op_status = GPIO_Interrupt_Init(M24SR_GPIO_GPO_PIN, hwGPIO_Interrupt_Mode_Falling_Edge);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }
    
    gpio_op_status = GPIO_Register_Interrupt_Handler(M24SR_GPIO_GPO_PIN, M24SR_IO_Interrupt_Handler);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_Enable(M24SR_GPIO_GPO_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Init(M24SR_GPIO_RFDISABLE_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_None);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(M24SR_GPIO_RFDISABLE_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    M24SR_GPO_Event_Handler_CB = cb;

    return NFC_OK;
}

NFC_OpResult M24SR_IO_DeInit()
{
    hwGPIO_OpResult gpio_op_status;

    gpio_op_status = GPIO_DeInit(M24SR_GPIO_RFDISABLE_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }
    
    gpio_op_status = GPIO_UnRegister_Interrupt_Handler(M24SR_GPIO_GPO_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_Disable(M24SR_GPIO_GPO_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_DeInit(M24SR_GPIO_GPO_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    return NFC_OK;
}

NFC_OpResult M24SR_IO_ReadMultiple(uint8_t *pBuffer, uint16_t len)
{
    hwI2C_OpResult i2c_result;

    if (pBuffer == NULL || len == 0 || len > UINT8_MAX)
    {
        return NFC_InvalidParameter;
    }

    i2c_result = I2C_Master_Read(
        M24SR_I2C_INDEX,
        M24SR_I2C_ADDR >> 1,
        pBuffer,
        (uint8_t)len,
        true,
        M24SR_I2C_OP_TIMEOUT
    );

    return NFC_M24SR_Map_I2C_Error_Code(i2c_result);
}

NFC_OpResult M24SR_IO_WriteMultiple(uint8_t *pBuffer, uint16_t len)
{
    hwI2C_OpResult i2c_result;

    if (pBuffer == NULL || len == 0 || len > UINT8_MAX)
    {
        return NFC_InvalidParameter;
    }

    i2c_result = I2C_Master_Write(
        M24SR_I2C_INDEX,
        M24SR_I2C_ADDR >> 1,
        pBuffer,
        (uint8_t)len,
        true,
        M24SR_I2C_OP_TIMEOUT
    );

    return NFC_M24SR_Map_I2C_Error_Code(i2c_result);
}

NFC_OpResult M24SR_IO_RF_Enable()
{
  return NFC_M24SR_Map_GPIO_Error_Code(GPIO_Pin_Write(M24SR_GPIO_RFDISABLE_PIN, 0));
}

NFC_OpResult M24SR_IO_RF_Disable()
{
  return NFC_M24SR_Map_GPIO_Error_Code(GPIO_Pin_Write(M24SR_GPIO_RFDISABLE_PIN, 1));
}