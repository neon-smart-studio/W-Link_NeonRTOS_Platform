/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "BH1750FVI.h"

typedef enum BH1750_Power_Command_t
{
    BH1750_Power_Off   = 0x00,
    BH1750_Power_On    = 0x01,
    BH1750_Power_Reset = 0x07
} BH1750_Power_Command;

typedef enum BH1750_Mode_t
{
    BH1750_Mode_Continuous_HighRes  = 0x10,
    BH1750_Mode_Continuous_HighRes2 = 0x11,
    BH1750_Mode_Continuous_LowRes   = 0x13,

    BH1750_Mode_OneTime_HighRes     = 0x20,
    BH1750_Mode_OneTime_HighRes2    = 0x21,
    BH1750_Mode_OneTime_LowRes      = 0x23
} BH1750_Mode;

typedef enum BH1750_MT_Command_t
{
    BH1750_MT_High_Bit = 0x40,
    BH1750_MT_Low_Bit  = 0x60
} BH1750_MT_Command;

static BH1750FVI_OpStatus BH1750FVI_Map_I2C_HW_Error_Code(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return BH1750FVI_OK;

        case hwI2C_NotInit:
            return BH1750FVI_NotInit;

        case hwI2C_InvalidParameter:
            return BH1750FVI_InvalidParameter;

        case hwI2C_MemoryError:
            return BH1750FVI_MemoryError;

        case hwI2C_MutexTimeout:
            return BH1750FVI_MutexTimeout;

        case hwI2C_SlaveTimeout:
        case hwI2C_BusError:
            return BH1750FVI_IO_Error;

        case hwI2C_Unsupport:
        default:
            return BH1750FVI_Unsupport;
    }
}

static BH1750FVI_OpStatus BH1750FVI_Send_CMD(uint8_t cmd)
{
    hwI2C_OpResult i2c_op_result;

    i2c_op_result = I2C_Master_Write(
        BH1750FVI_I2C_INDEX,
        BH1750FVI_I2C_ADDRESS >> 1,
        &cmd,
        1,
        true,
        BH1750FVI_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return BH1750FVI_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return BH1750FVI_OK;
}

static BH1750FVI_OpStatus BH1750FVI_Read_Data_16bit(uint8_t* byte1, uint8_t* byte2)
{
    hwI2C_OpResult i2c_op_result;

    uint8_t rdBuff[2];

    if(byte1 == NULL || byte2 == NULL)
    {
        return BH1750FVI_InvalidParameter;
    }

    i2c_op_result = I2C_Master_Read(
        BH1750FVI_I2C_INDEX,
        BH1750FVI_I2C_ADDRESS >> 1,
        rdBuff,
        2,
        true,
        BH1750FVI_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return BH1750FVI_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    *byte1 = rdBuff[0];
    *byte2 = rdBuff[1];

    return BH1750FVI_OK;
}

static BH1750FVI_OpStatus BH1750FVI_Get_Mode(bool continuous, BH1750FVI_Resolution resolution, BH1750_Mode* mode)
{
    if(mode == NULL)
    {
        return BH1750FVI_InvalidParameter;
    }

    if(resolution >= BH1750FVI_Resolution_MAX)
    {
        return BH1750FVI_InvalidParameter;
    }

    if(continuous == true)
    {
        switch(resolution)
        {
            case BH1750FVI_Resolution_11x:
                *mode = BH1750_Mode_Continuous_HighRes;
                break;

            case BH1750FVI_Resolution_0_511x:
                *mode = BH1750_Mode_Continuous_HighRes2;
                break;

            case BH1750FVI_Resolution_411x:
                *mode = BH1750_Mode_Continuous_LowRes;
                break;

            default:
                return BH1750FVI_InvalidParameter;
        }
    }
    else
    {
        switch(resolution)
        {
            case BH1750FVI_Resolution_11x:
                *mode = BH1750_Mode_OneTime_HighRes;
                break;

            case BH1750FVI_Resolution_0_511x:
                *mode = BH1750_Mode_OneTime_HighRes2;
                break;

            case BH1750FVI_Resolution_411x:
                *mode = BH1750_Mode_OneTime_LowRes;
                break;

            default:
                return BH1750FVI_InvalidParameter;
        }
    }

    return BH1750FVI_OK;
}

static uint16_t BH1750FVI_Get_Mode_Delay_ms(BH1750_Mode mode)
{
    switch(mode)
    {
        case BH1750_Mode_Continuous_HighRes:
        case BH1750_Mode_Continuous_HighRes2:
        case BH1750_Mode_OneTime_HighRes:
        case BH1750_Mode_OneTime_HighRes2:
            return 180;

        case BH1750_Mode_Continuous_LowRes:
        case BH1750_Mode_OneTime_LowRes:
            return 24;

        default:
            return 0;
    }
}

BH1750FVI_OpStatus BH1750FVI_Init(void)
{
    BH1750FVI_OpStatus status;

    status = BH1750FVI_Send_CMD((uint8_t)BH1750_Power_On);
    if(status < BH1750FVI_OK)
    {
        return status;
    }

    status = BH1750FVI_Send_CMD((uint8_t)BH1750_Power_Reset);
    if(status < BH1750FVI_OK)
    {
        return status;
    }

    return BH1750FVI_OK;
}

BH1750FVI_OpStatus BH1750FVI_Read_Lux(bool continuous, BH1750FVI_Resolution resolution, uint16_t* lux)
{
    BH1750FVI_OpStatus status;
    BH1750_Mode mode;
    uint16_t delay_ms;
    uint8_t rd16b[2];

    if(lux == NULL)
    {
        return BH1750FVI_InvalidParameter;
    }

    status = BH1750FVI_Get_Mode(continuous, resolution, &mode);
    if(status < BH1750FVI_OK)
    {
        return status;
    }

    delay_ms = BH1750FVI_Get_Mode_Delay_ms(mode);
    if(delay_ms == 0)
    {
        return BH1750FVI_InvalidParameter;
    }

    status = BH1750FVI_Send_CMD((uint8_t)mode);
    if(status < BH1750FVI_OK)
    {
        return status;
    }

    NeonRTOS_Sleep(delay_ms);

    status = BH1750FVI_Read_Data_16bit(&rd16b[0], &rd16b[1]);
    if(status < BH1750FVI_OK)
    {
        return status;
    }

    *lux = ((uint16_t)rd16b[0] << 8) | rd16b[1];

    return BH1750FVI_OK;
}