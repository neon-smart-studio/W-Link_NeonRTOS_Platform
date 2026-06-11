/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "BMP085.h"

#define BMP085_I2C_OP_TIMEOUT            500

typedef enum BMP085_Register_t
{
    BMP085_Register_Cal_AC1       = 0xAA,
    BMP085_Register_Cal_AC2       = 0xAC,
    BMP085_Register_Cal_AC3       = 0xAE,
    BMP085_Register_Cal_AC4       = 0xB0,
    BMP085_Register_Cal_AC5       = 0xB2,
    BMP085_Register_Cal_AC6       = 0xB4,
    BMP085_Register_Cal_B1        = 0xB6,
    BMP085_Register_Cal_B2        = 0xB8,
    BMP085_Register_Cal_MB        = 0xBA,
    BMP085_Register_Cal_MC        = 0xBC,
    BMP085_Register_Cal_MD        = 0xBE,

    BMP085_Register_Control       = 0xF4,
    BMP085_Register_Data          = 0xF6

} BMP085_Register;

typedef enum BMP085_Measure_Command_t
{
    BMP085_Measure_Temperature    = 0x2E,
    BMP085_Measure_Pressure_Low   = 0x34,
    BMP085_Measure_Pressure_Std   = 0x74,
    BMP085_Measure_Pressure_High  = 0xB4,
    BMP085_Measure_Pressure_UHigh = 0xF4

} BMP085_Measure_Command;

typedef struct BMP085_Pressure_Mode_Config_t
{
    BMP085_Resolution resolution;
    BMP085_Measure_Command command;
    uint8_t oss;
    uint8_t delay_ms;

} BMP085_Pressure_Mode_Config;

typedef struct BMP085_Calibration_Data_t
{
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
    int32_t b5;
} BMP085_Calibration_Data;

static BMP085_Calibration_Data bmp085_cal;

static const BMP085_Pressure_Mode_Config BMP085_Pressure_Mode_Table[] =
{
    { BMP085_Resolution_Low,        BMP085_Measure_Pressure_Low,   0, 5  },
    { BMP085_Resolution_Standard,   BMP085_Measure_Pressure_Std,   1, 8  },
    { BMP085_Resolution_High,       BMP085_Measure_Pressure_High,  2, 14 },
    { BMP085_Resolution_Ultra_High, BMP085_Measure_Pressure_UHigh, 3, 26 },
};

static BMP085_OpResult BMP085_Map_I2C_HW_Error_Code(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return BMP085_OK;

        case hwI2C_NotInit:
            return BMP085_NotInit;

        case hwI2C_InvalidParameter:
            return BMP085_InvalidParameter;

        case hwI2C_MemoryError:
            return BMP085_MemoryError;

        case hwI2C_MutexTimeout:
            return BMP085_MutexTimeout;

        case hwI2C_SlaveTimeout:
        case hwI2C_BusError:
            return BMP085_IO_Error;

        case hwI2C_Unsupport:
        default:
            return BMP085_Unsupport;
    }
}

static BMP085_OpResult BMP085_Write_Reg(uint8_t reg, uint8_t data)
{
    hwI2C_OpResult i2c_op_result;

    uint8_t tx_buf[2];

    tx_buf[0] = reg;
    tx_buf[1] = data;

    i2c_op_result = I2C_Master_Write(
        BMP085_I2C_INDEX,
        BMP085_I2C_ADDR >> 1,
        tx_buf,
        2,
        true,
        BMP085_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return BMP085_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return BMP085_OK;
}

static BMP085_OpResult BMP085_Read_Reg(uint8_t reg, uint8_t* buf, uint8_t len)
{
    hwI2C_OpResult i2c_op_result;

    if(buf == NULL || len == 0)
    {
        return BMP085_InvalidParameter;
    }

    i2c_op_result = I2C_Master_Write(
        BMP085_I2C_INDEX,
        BMP085_I2C_ADDR >> 1,
        &reg,
        1,
        true,
        BMP085_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return BMP085_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    i2c_op_result = I2C_Master_Read(
        BMP085_I2C_INDEX,
        BMP085_I2C_ADDR >> 1,
        buf,
        len,
        true,
        BMP085_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return BMP085_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return BMP085_OK;
}

static BMP085_OpResult BMP085_Read_Reg_16Bit(uint8_t reg, uint16_t* data)
{
    BMP085_OpResult status;

    uint8_t buf[2];

    if(data == NULL)
    {
        return BMP085_InvalidParameter;
    }

    status = BMP085_Read_Reg(reg, buf, 2);
    if(status < BMP085_OK)
    {
        return status;
    }

    *data = ((uint16_t)buf[0] << 8) | buf[1];

    return BMP085_OK;
}

static BMP085_OpResult BMP085_Read_Reg_S16Bit(uint8_t reg, int16_t* data)
{
    BMP085_OpResult status;

    uint16_t raw;

    if(data == NULL)
    {
        return BMP085_InvalidParameter;
    }

    status = BMP085_Read_Reg_16Bit(reg, &raw);
    if(status < BMP085_OK)
    {
        return status;
    }

    *data = (int16_t)raw;

    return BMP085_OK;
}

static BMP085_OpResult BMP085_Get_Pressure_Mode_Config(
    BMP085_Resolution resolution,
    const BMP085_Pressure_Mode_Config** config
)
{
    uint32_t i;

    if(config == NULL)
    {
        return BMP085_InvalidParameter;
    }

    if(resolution >= BMP085_Resolution_MAX)
    {
        return BMP085_InvalidParameter;
    }

    for(i = 0; i < sizeof(BMP085_Pressure_Mode_Table) / sizeof(BMP085_Pressure_Mode_Table[0]); i++)
    {
        if(BMP085_Pressure_Mode_Table[i].resolution == resolution)
        {
            *config = &BMP085_Pressure_Mode_Table[i];
            return BMP085_OK;
        }
    }

    return BMP085_InvalidParameter;
}

BMP085_OpResult BMP085_Init(void)
{
    return BMP085_Calibration();
}

BMP085_OpResult BMP085_Read_Temperature(double* temperature)
{
    BMP085_OpResult status;

    uint16_t ut;
    int32_t x1;
    int32_t x2;
    int32_t temp;

    if(temperature == NULL)
    {
        return BMP085_InvalidParameter;
    }

    status = BMP085_Write_Reg(BMP085_Register_Control, BMP085_Measure_Temperature);
    if(status < BMP085_OK)
    {
        return status;
    }

    NeonRTOS_Sleep(5);

    status = BMP085_Read_Reg_16Bit(BMP085_Register_Data, &ut);
    if(status < BMP085_OK)
    {
        return status;
    }

    x1 = (((int32_t)ut - (int32_t)bmp085_cal.ac6) * (int32_t)bmp085_cal.ac5) >> 15;
    x2 = ((int32_t)bmp085_cal.mc << 11) / (x1 + bmp085_cal.md);

    bmp085_cal.b5 = x1 + x2;

    temp = (bmp085_cal.b5 + 8) >> 4;

    *temperature = (double)temp / 10.0;

    return BMP085_OK;
}

BMP085_OpResult BMP085_Read_Measure_Pressure(BMP085_Resolution resolution, long* pressure)
{
    BMP085_OpResult status;
    
    const BMP085_Pressure_Mode_Config* config;
    uint8_t buf[3];
    int32_t up;
    int32_t x1;
    int32_t x2;
    int32_t x3;
    int32_t b3;
    int32_t b6;
    int32_t p;
    uint32_t b4;
    uint32_t b7;

    if(pressure == NULL)
    {
        return BMP085_InvalidParameter;
    }

    status = BMP085_Get_Pressure_Mode_Config(resolution, &config);
    if(status < BMP085_OK)
    {
        return status;
    }

    status = BMP085_Write_Reg(BMP085_Register_Control, config->command);
    if(status < BMP085_OK)
    {
        return status;
    }

    NeonRTOS_Sleep(config->delay_ms);

    status = BMP085_Read_Reg(BMP085_Register_Data, buf, 3);
    if(status < BMP085_OK)
    {
        return status;
    }

    up = (((uint32_t)buf[0] << 16) |
          ((uint32_t)buf[1] << 8)  |
          ((uint32_t)buf[2])) >> (8 - config->oss);

    b6 = bmp085_cal.b5 - 4000;

    x1 = ((int32_t)bmp085_cal.b2 * ((b6 * b6) >> 12)) >> 11;
    x2 = ((int32_t)bmp085_cal.ac2 * b6) >> 11;
    x3 = x1 + x2;

    b3 = (((((int32_t)bmp085_cal.ac1 * 4) + x3) << config->oss) + 2) >> 2;

    x1 = ((int32_t)bmp085_cal.ac3 * b6) >> 13;
    x2 = ((int32_t)bmp085_cal.b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = (x1 + x2 + 2) >> 2;

    b4 = ((uint32_t)bmp085_cal.ac4 * (uint32_t)(x3 + 32768)) >> 15;
    b7 = ((uint32_t)(up - b3) * (uint32_t)(50000 >> config->oss));

    if(b4 == 0)
    {
        return BMP085_IO_Error;
    }

    if(b7 < 0x80000000UL)
    {
        p = (int32_t)((b7 << 1) / b4);
    }
    else
    {
        p = (int32_t)((b7 / b4) << 1);
    }

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;

    p = p + ((x1 + x2 + 3791) >> 4);

    *pressure = p;

    return BMP085_OK;
}

BMP085_OpResult BMP085_Calibration(void)
{
    BMP085_OpResult status;

    status = BMP085_Read_Reg_S16Bit(BMP085_Register_Cal_AC1, &bmp085_cal.ac1);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_S16Bit(BMP085_Register_Cal_AC2, &bmp085_cal.ac2);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_S16Bit(BMP085_Register_Cal_AC3, &bmp085_cal.ac3);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_16Bit(BMP085_Register_Cal_AC4, &bmp085_cal.ac4);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_16Bit(BMP085_Register_Cal_AC5, &bmp085_cal.ac5);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_16Bit(BMP085_Register_Cal_AC6, &bmp085_cal.ac6);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_S16Bit(BMP085_Register_Cal_B1, &bmp085_cal.b1);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_S16Bit(BMP085_Register_Cal_B2, &bmp085_cal.b2);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_S16Bit(BMP085_Register_Cal_MB, &bmp085_cal.mb);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_S16Bit(BMP085_Register_Cal_MC, &bmp085_cal.mc);
    if(status < BMP085_OK) { return status; }

    status = BMP085_Read_Reg_S16Bit(BMP085_Register_Cal_MD, &bmp085_cal.md);
    if(status < BMP085_OK) { return status; }

    bmp085_cal.b5 = 0;

    return BMP085_OK;
}