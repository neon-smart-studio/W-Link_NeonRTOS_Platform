/*
This LPS22HB driver is based on STMicroelectronics stm32-LPS22HB / LPS22HB component driver.
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

#include "LPS22HB_IO.h"

#define LPS22HB_IO_TX_BUF_SIZE             32

static LPS22HB_OpStatus LPS22HB_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return LPS22HB_OK;

        case hwI2C_NotInit:
            return LPS22HB_NotInit;

        case hwI2C_InvalidParameter:
            return LPS22HB_InvalidParameter;

        case hwI2C_MemoryError:
            return LPS22HB_MemoryError;

        case hwI2C_MutexTimeout:
            return LPS22HB_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return LPS22HB_SlaveTimeout;

        case hwI2C_BusError:
            return LPS22HB_IO_Error;

        case hwI2C_Unsupport:
        default:
            return LPS22HB_Unsupport;
    }
}

LPS22HB_OpStatus LPS22HB_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
)
{
    LPS22HB_OpStatus status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return LPS22HB_InvalidParameter;
    }

    status = LPS22HB_IO_Map_I2C_Error(
        I2C_Master_Write(
            LPS22HB_I2C_INDEX,
            LPS22HB_I2C_ADDRESS >> 1,
            &RegisterAddr,
            1,
            false,
            LPS22HB_I2C_OP_TIMEOUT
        )
    );

    if(status < LPS22HB_OK)
    {
        return status;
    }

    status = LPS22HB_IO_Map_I2C_Error(
        I2C_Master_Read(
            LPS22HB_I2C_INDEX,
            LPS22HB_I2C_ADDRESS >> 1,
            pBuffer,
            NumByteToRead,
            true,
            LPS22HB_I2C_OP_TIMEOUT
        )
    );

    if(status < LPS22HB_OK)
    {
        return status;
    }

    return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
)
{
    LPS22HB_OpStatus status;
    uint8_t tx_buf[LPS22HB_IO_TX_BUF_SIZE];

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return LPS22HB_InvalidParameter;
    }

    if((NumByteToWrite + 1) > sizeof(tx_buf))
    {
        return LPS22HB_InvalidParameter;
    }

    tx_buf[0] = RegisterAddr;
    memcpy(&tx_buf[1], pBuffer, NumByteToWrite);

    status = LPS22HB_IO_Map_I2C_Error(
        I2C_Master_Write(
            LPS22HB_I2C_INDEX,
            LPS22HB_I2C_ADDRESS >> 1,
            tx_buf,
            NumByteToWrite + 1,
            true,
            LPS22HB_I2C_OP_TIMEOUT
        )
    );

    if(status < LPS22HB_OK)
    {
        return status;
    }

    return LPS22HB_OK;
}

LPS22HB_OpStatus LPS22HB_IO_ReadByte(uint8_t RegisterAddr, uint8_t* value)
{
    if(value == NULL)
    {
        return LPS22HB_InvalidParameter;
    }

    return LPS22HB_IO_Read(RegisterAddr, value, 1);
}

LPS22HB_OpStatus LPS22HB_IO_WriteByte(uint8_t RegisterAddr, uint8_t value)
{
    return LPS22HB_IO_Write(RegisterAddr, &value, 1);
}