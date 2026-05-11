/*
This LIS3MDL driver is based on STMicroelectronics stm32-LIS3MDL / LIS3MDL component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "LIS3MDL_IO.h"

#define LIS3MDL_IO_TX_BUF_SIZE             32

static LIS3MDL_OpStatus LIS3MDL_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return LIS3MDL_OK;

        case hwI2C_NotInit:
            return LIS3MDL_NotInit;

        case hwI2C_InvalidParameter:
            return LIS3MDL_InvalidParameter;

        case hwI2C_MemoryError:
            return LIS3MDL_MemoryError;

        case hwI2C_MutexTimeout:
            return LIS3MDL_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return LIS3MDL_SlaveTimeout;

        case hwI2C_BusError:
            return LIS3MDL_IO_Error;

        case hwI2C_Unsupport:
        default:
            return LIS3MDL_Unsupport;
    }
}

LIS3MDL_OpStatus LIS3MDL_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
)
{
    LIS3MDL_OpStatus status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return LIS3MDL_InvalidParameter;
    }

    status = LIS3MDL_IO_Map_I2C_Error(
        I2C_Master_Write(
            LIS3MDL_I2C_INDEX,
            LIS3MDL_I2C_ADDRESS >> 1,
            &RegisterAddr,
            1,
            false,
            LIS3MDL_I2C_OP_TIMEOUT
        )
    );

    if(status < LIS3MDL_OK)
    {
        return status;
    }

    status = LIS3MDL_IO_Map_I2C_Error(
        I2C_Master_Read(
            LIS3MDL_I2C_INDEX,
            LIS3MDL_I2C_ADDRESS >> 1,
            pBuffer,
            NumByteToRead,
            true,
            LIS3MDL_I2C_OP_TIMEOUT
        )
    );

    if(status < LIS3MDL_OK)
    {
        return status;
    }

    return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
)
{
    LIS3MDL_OpStatus status;
    uint8_t tx_buf[LIS3MDL_IO_TX_BUF_SIZE];

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return LIS3MDL_InvalidParameter;
    }

    if((NumByteToWrite + 1) > sizeof(tx_buf))
    {
        return LIS3MDL_InvalidParameter;
    }

    tx_buf[0] = RegisterAddr;
    memcpy(&tx_buf[1], pBuffer, NumByteToWrite);

    status = LIS3MDL_IO_Map_I2C_Error(
        I2C_Master_Write(
            LIS3MDL_I2C_INDEX,
            LIS3MDL_I2C_ADDRESS >> 1,
            tx_buf,
            NumByteToWrite + 1,
            true,
            LIS3MDL_I2C_OP_TIMEOUT
        )
    );

    if(status < LIS3MDL_OK)
    {
        return status;
    }

    return LIS3MDL_OK;
}

LIS3MDL_OpStatus LIS3MDL_IO_ReadByte(uint8_t RegisterAddr, uint8_t* value)
{
    if(value == NULL)
    {
        return LIS3MDL_InvalidParameter;
    }

    return LIS3MDL_IO_Read(RegisterAddr, value, 1);
}

LIS3MDL_OpStatus LIS3MDL_IO_WriteByte(uint8_t RegisterAddr, uint8_t value)
{
    return LIS3MDL_IO_Write(RegisterAddr, &value, 1);
}