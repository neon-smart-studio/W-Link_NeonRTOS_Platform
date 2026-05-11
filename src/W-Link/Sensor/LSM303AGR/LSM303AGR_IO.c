/*
This LSM303AGR driver is based on STMicroelectronics stm32-LSM303AGR / LSM303AGR component driver.
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

#include "LSM303AGR_IO.h"

#define LSM303AGR_IO_TX_BUF_SIZE             32

static LSM303AGR_OpStatus LSM303AGR_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return LSM303AGR_OK;

        case hwI2C_NotInit:
            return LSM303AGR_NotInit;

        case hwI2C_InvalidParameter:
            return LSM303AGR_InvalidParameter;

        case hwI2C_MemoryError:
            return LSM303AGR_MemoryError;

        case hwI2C_MutexTimeout:
            return LSM303AGR_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return LSM303AGR_SlaveTimeout;

        case hwI2C_BusError:
            return LSM303AGR_IO_Error;

        case hwI2C_Unsupport:
        default:
            return LSM303AGR_Unsupport;
    }
}

LSM303AGR_OpStatus LSM303AGR_ACC_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
)
{
    LSM303AGR_OpStatus status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return LSM303AGR_InvalidParameter;
    }

    status = LSM303AGR_IO_Map_I2C_Error(
        I2C_Master_Write(
            LSM303AGR_I2C_INDEX,
            LSM303AGR_ACC_I2C_ADDRESS >> 1,
            &RegisterAddr,
            1,
            false,
            LSM303AGR_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM303AGR_OK)
    {
        return status;
    }

    status = LSM303AGR_IO_Map_I2C_Error(
        I2C_Master_Read(
            LSM303AGR_I2C_INDEX,
            LSM303AGR_ACC_I2C_ADDRESS >> 1,
            pBuffer,
            NumByteToRead,
            true,
            LSM303AGR_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM303AGR_OK)
    {
        return status;
    }

    return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
)
{
    LSM303AGR_OpStatus status;
    uint8_t tx_buf[LSM303AGR_IO_TX_BUF_SIZE];

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return LSM303AGR_InvalidParameter;
    }

    if((NumByteToWrite + 1) > sizeof(tx_buf))
    {
        return LSM303AGR_InvalidParameter;
    }

    tx_buf[0] = RegisterAddr;
    memcpy(&tx_buf[1], pBuffer, NumByteToWrite);

    status = LSM303AGR_IO_Map_I2C_Error(
        I2C_Master_Write(
            LSM303AGR_I2C_INDEX,
            LSM303AGR_ACC_I2C_ADDRESS >> 1,
            tx_buf,
            NumByteToWrite + 1,
            true,
            LSM303AGR_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM303AGR_OK)
    {
        return status;
    }

    return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_ACC_IO_ReadByte(uint8_t RegisterAddr, uint8_t* value)
{
    if(value == NULL)
    {
        return LSM303AGR_InvalidParameter;
    }

    return LSM303AGR_ACC_IO_Read(RegisterAddr, value, 1);
}

LSM303AGR_OpStatus LSM303AGR_ACC_IO_WriteByte(uint8_t RegisterAddr, uint8_t value)
{
    return LSM303AGR_ACC_IO_Write(RegisterAddr, &value, 1);
}

LSM303AGR_OpStatus LSM303AGR_MAG_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
)
{
    LSM303AGR_OpStatus status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return LSM303AGR_InvalidParameter;
    }

    status = LSM303AGR_IO_Map_I2C_Error(
        I2C_Master_Write(
            LSM303AGR_I2C_INDEX,
            LSM303AGR_MAG_I2C_ADDRESS,
            &RegisterAddr,
            1,
            false,
            LSM303AGR_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM303AGR_OK)
    {
        return status;
    }

    status = LSM303AGR_IO_Map_I2C_Error(
        I2C_Master_Read(
            LSM303AGR_I2C_INDEX,
            LSM303AGR_MAG_I2C_ADDRESS,
            pBuffer,
            NumByteToRead,
            true,
            LSM303AGR_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM303AGR_OK)
    {
        return status;
    }

    return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_MAG_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
)
{
    LSM303AGR_OpStatus status;
    uint8_t tx_buf[LSM303AGR_IO_TX_BUF_SIZE];

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return LSM303AGR_InvalidParameter;
    }

    if((NumByteToWrite + 1) > sizeof(tx_buf))
    {
        return LSM303AGR_InvalidParameter;
    }

    tx_buf[0] = RegisterAddr;
    memcpy(&tx_buf[1], pBuffer, NumByteToWrite);

    status = LSM303AGR_IO_Map_I2C_Error(
        I2C_Master_Write(
            LSM303AGR_I2C_INDEX,
            LSM303AGR_MAG_I2C_ADDRESS,
            tx_buf,
            NumByteToWrite + 1,
            true,
            LSM303AGR_I2C_OP_TIMEOUT
        )
    );

    if(status < LSM303AGR_OK)
    {
        return status;
    }

    return LSM303AGR_OK;
}

LSM303AGR_OpStatus LSM303AGR_MAG_IO_ReadByte(uint8_t RegisterAddr, uint8_t* value)
{
    if(value == NULL)
    {
        return LSM303AGR_InvalidParameter;
    }

    return LSM303AGR_MAG_IO_Read(RegisterAddr, value, 1);
}

LSM303AGR_OpStatus LSM303AGR_MAG_IO_WriteByte(uint8_t RegisterAddr, uint8_t value)
{
    return LSM303AGR_MAG_IO_Write(RegisterAddr, &value, 1);
}