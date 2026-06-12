/*
This LSM6DSL driver is based on STMicroelectronics stm32-LSM6DSL / LSM6DSL component driver.
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

#include "LSM6DSL_IO.h"

#define LSM6DSL_IO_TX_BUF_SIZE             32

static LSM6DSL_OpResult LSM6DSL_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return LSM6DSL_OK;

        case hwI2C_NotInit:
            return LSM6DSL_NotInit;

        case hwI2C_InvalidParameter:
            return LSM6DSL_InvalidParameter;

        case hwI2C_MemoryError:
            return LSM6DSL_MemoryError;

        case hwI2C_MutexTimeout:
            return LSM6DSL_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return LSM6DSL_SlaveTimeout;

        case hwI2C_BusError:
            return LSM6DSL_IO_Error;

        case hwI2C_Unsupport:
        default:
            return LSM6DSL_Unsupport;
    }
}

LSM6DSL_OpResult LSM6DSL_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
)
{
    LSM6DSL_OpResult status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return LSM6DSL_InvalidParameter;
    }

    status = LSM6DSL_IO_Map_I2C_Error(
        I2C_Master_Write(
            LSM6DSL_I2C_INDEX,
            LSM6DSL_I2C_ADDRESS >> 1,
            &RegisterAddr,
            1,
            false,
            LSM6DSL_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM6DSL_OK)
    {
        return status;
    }

    status = LSM6DSL_IO_Map_I2C_Error(
        I2C_Master_Read(
            LSM6DSL_I2C_INDEX,
            LSM6DSL_I2C_ADDRESS >> 1,
            pBuffer,
            NumByteToRead,
            true,
            LSM6DSL_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM6DSL_OK)
    {
        return status;
    }

    return LSM6DSL_OK;
}

LSM6DSL_OpResult LSM6DSL_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
)
{
    LSM6DSL_OpResult status;
    uint8_t tx_buf[LSM6DSL_IO_TX_BUF_SIZE];

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return LSM6DSL_InvalidParameter;
    }

    if((NumByteToWrite + 1) > sizeof(tx_buf))
    {
        return LSM6DSL_InvalidParameter;
    }

    tx_buf[0] = RegisterAddr;
    memcpy(&tx_buf[1], pBuffer, NumByteToWrite);

    status = LSM6DSL_IO_Map_I2C_Error(
        I2C_Master_Write(
            LSM6DSL_I2C_INDEX,
            LSM6DSL_I2C_ADDRESS >> 1,
            tx_buf,
            NumByteToWrite + 1,
            true,
            LSM6DSL_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM6DSL_OK)
    {
        return status;
    }

    return LSM6DSL_OK;
}

LSM6DSL_OpResult LSM6DSL_IO_ReadByte(uint8_t RegisterAddr, uint8_t* value)
{
    if(value == NULL)
    {
        return LSM6DSL_InvalidParameter;
    }

    return LSM6DSL_IO_Read(RegisterAddr, value, 1);
}

LSM6DSL_OpResult LSM6DSL_IO_WriteByte(uint8_t RegisterAddr, uint8_t value)
{
    return LSM6DSL_IO_Write(RegisterAddr, &value, 1);
}