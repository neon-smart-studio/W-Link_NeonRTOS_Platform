/*
This HTS221 driver is based on STMicroelectronics stm32-lsm6dso / HTS221 component driver.
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

#include "HTS221_IO.h"

#define HTS221_IO_TX_BUF_SIZE             32

static HTS221_OpStatus HTS221_IO_Map_I2C_Error(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return HTS221_OK;

        case hwI2C_NotInit:
            return HTS221_NotInit;

        case hwI2C_InvalidParameter:
            return HTS221_InvalidParameter;

        case hwI2C_MemoryError:
            return HTS221_MemoryError;

        case hwI2C_MutexTimeout:
            return HTS221_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return HTS221_SlaveTimeout;

        case hwI2C_BusError:
            return HTS221_IO_Error;

        case hwI2C_Unsupport:
        default:
            return HTS221_Unsupport;
    }
}

HTS221_OpStatus HTS221_IO_Init(void)
{
    /*
     * 若你的 W-Link I2C bus 已經由系統初始化，
     * 這裡可以保持空 init。
     *
     * 若 I2C_Master_Init() 是必要的，再打開下面這段。
     */

#if defined(CONFIG_HTS221_AUTO_INIT_I2C)
    if(I2C_Master_isInit(HTS221_I2C_INDEX) == false)
    {
        return HTS221_IO_Map_I2C_Error(
            I2C_Master_Init(
                HTS221_I2C_INDEX,
                hwI2C_Fast_Mode
            )
        );
    }
#endif

    return HTS221_OK;
}

HTS221_OpStatus HTS221_IO_DeInit(void)
{
#if defined(CONFIG_HTS221_AUTO_INIT_I2C)
    if(I2C_Master_isInit(HTS221_I2C_INDEX) == true)
    {
        return HTS221_IO_Map_I2C_Error(
            I2C_Master_DeInit(HTS221_I2C_INDEX)
        );
    }
#endif

    return HTS221_OK;
}

HTS221_OpStatus HTS221_Register_Interrupt_Handler(HTS221_IO_Event_IRQ irq_callback)
{
    (void)irq_callback;

    return HTS221_Unsupport;
}

HTS221_OpStatus HTS221_UnRegister_Interrupt_Handler(void)
{
    return HTS221_Unsupport;
}

HTS221_OpStatus HTS221_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
)
{
    HTS221_OpStatus status;

    if(pBuffer == NULL || NumByteToRead == 0)
    {
        return HTS221_InvalidParameter;
    }

    status = HTS221_IO_Map_I2C_Error(
        I2C_Master_Write(
            HTS221_I2C_INDEX,
            HTS221_I2C_ADDRESS,
            &RegisterAddr,
            1,
            false,
            HTS221_I2C_OP_TIMEOUT
        )
    );

    if(status < HTS221_OK)
    {
        return status;
    }

    status = HTS221_IO_Map_I2C_Error(
        I2C_Master_Read(
            HTS221_I2C_INDEX,
            HTS221_I2C_ADDRESS,
            pBuffer,
            NumByteToRead,
            true,
            HTS221_I2C_OP_TIMEOUT
        )
    );

    if(status < HTS221_OK)
    {
        return status;
    }

    return HTS221_OK;
}

HTS221_OpStatus HTS221_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
)
{
    HTS221_OpStatus status;
    uint8_t tx_buf[HTS221_IO_TX_BUF_SIZE];

    if(pBuffer == NULL || NumByteToWrite == 0)
    {
        return HTS221_InvalidParameter;
    }

    if((NumByteToWrite + 1) > sizeof(tx_buf))
    {
        return HTS221_InvalidParameter;
    }

    tx_buf[0] = RegisterAddr;
    memcpy(&tx_buf[1], pBuffer, NumByteToWrite);

    status = HTS221_IO_Map_I2C_Error(
        I2C_Master_Write(
            HTS221_I2C_INDEX,
            HTS221_I2C_ADDRESS,
            tx_buf,
            NumByteToWrite + 1,
            true,
            HTS221_I2C_OP_TIMEOUT
        )
    );

    if(status < HTS221_OK)
    {
        return status;
    }

    return HTS221_OK;
}

HTS221_OpStatus HTS221_IO_ReadByte(uint8_t RegisterAddr, uint8_t* value)
{
    if(value == NULL)
    {
        return HTS221_InvalidParameter;
    }

    return HTS221_IO_Read(RegisterAddr, value, 1);
}

HTS221_OpStatus HTS221_IO_WriteByte(uint8_t RegisterAddr, uint8_t value)
{
    return HTS221_IO_Write(RegisterAddr, &value, 1);
}