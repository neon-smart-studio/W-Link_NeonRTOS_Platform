/*
This LSM6DSO driver is based on STMicroelectronics stm32-lsm6dso / LSM6DSO component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "LSM6DSO_IO.h"

#define LSM6DSO_IO_TX_BUF_SIZE             32

static LSM6DSO_OpStatus LSM6DSO_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return LSM6DSO_OK;

        case hwI2C_NotInit:
            return LSM6DSO_NotInit;

        case hwI2C_InvalidParameter:
            return LSM6DSO_InvalidParameter;

        case hwI2C_MemoryError:
            return LSM6DSO_MemoryError;

        case hwI2C_MutexTimeout:
            return LSM6DSO_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return LSM6DSO_SlaveTimeout;

        case hwI2C_BusError:
            return LSM6DSO_IO_Error;

        case hwI2C_Unsupport:
        default:
            return LSM6DSO_Unsupport;
    }
}

LSM6DSO_OpStatus LSM6DSO_Register_Interrupt_Handler(LSM6DSO_IO_Event_IRQ irq_callback)
{
    (void)irq_callback;

    return LSM6DSO_Unsupport;
}

LSM6DSO_OpStatus LSM6DSO_UnRegister_Interrupt_Handler(void)
{
    return LSM6DSO_Unsupport;
}

LSM6DSO_OpStatus LSM6DSO_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
)
{
    LSM6DSO_OpStatus status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return LSM6DSO_InvalidParameter;
    }

    status = LSM6DSO_IO_Map_I2C_Error(
        I2C_Master_Write(
            LSM6DSO_I2C_INDEX,
            LSM6DSO_I2C_ADDRESS,
            &RegisterAddr,
            1,
            false,
            LSM6DSO_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM6DSO_OK)
    {
        return status;
    }

    status = LSM6DSO_IO_Map_I2C_Error(
        I2C_Master_Read(
            LSM6DSO_I2C_INDEX,
            LSM6DSO_I2C_ADDRESS,
            pBuffer,
            NumByteToRead,
            true,
            LSM6DSO_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM6DSO_OK)
    {
        return status;
    }

    return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
)
{
    LSM6DSO_OpStatus status;
    uint8_t tx_buf[LSM6DSO_IO_TX_BUF_SIZE];

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return LSM6DSO_InvalidParameter;
    }

    if((NumByteToWrite + 1) > sizeof(tx_buf))
    {
        return LSM6DSO_InvalidParameter;
    }

    tx_buf[0] = RegisterAddr;
    memcpy(&tx_buf[1], pBuffer, NumByteToWrite);

    status = LSM6DSO_IO_Map_I2C_Error(
        I2C_Master_Write(
            LSM6DSO_I2C_INDEX,
            LSM6DSO_I2C_ADDRESS,
            tx_buf,
            NumByteToWrite + 1,
            true,
            LSM6DSO_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM6DSO_OK)
    {
        return status;
    }

    return LSM6DSO_OK;
}

LSM6DSO_OpStatus LSM6DSO_IO_ReadByte(uint8_t RegisterAddr, uint8_t* value)
{
    if(value == NULL)
    {
        return LSM6DSO_InvalidParameter;
    }

    return LSM6DSO_IO_Read(RegisterAddr, value, 1);
}

LSM6DSO_OpStatus LSM6DSO_IO_WriteByte(uint8_t RegisterAddr, uint8_t value)
{
    return LSM6DSO_IO_Write(RegisterAddr, &value, 1);
}