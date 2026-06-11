
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "INA219.h"

#define INA219_I2C_OP_TIMEOUT                   500

static uint16_t INA219_CalValue = 0;
static float INA219_CurrentDivider_mA = 0.0f;
static float INA219_PowerMultiplier_mW = 0.0f;

static uint8_t INA219_Initialized = 0;

static INA219_OpResult INA219_Map_I2C_HW_Error_Code(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return INA219_OK;

        case hwI2C_NotInit:
            return INA219_NotInit;

        case hwI2C_InvalidParameter:
            return INA219_InvalidParameter;

        case hwI2C_MemoryError:
            return INA219_MemoryError;

        case hwI2C_MutexTimeout:
            return INA219_MutexTimeout;

        case hwI2C_SlaveTimeout:
        case hwI2C_BusError:
            return INA219_IO_Error;

        case hwI2C_Unsupport:
        default:
            return INA219_Unsupport;
    }
}

static INA219_OpResult INA219_WriteRegister(
    INA219_Register reg,
    uint16_t value
)
{
    hwI2C_OpResult i2c_op_result;

    uint8_t buf[3];

    buf[0] = (uint8_t)reg;
    buf[1] = (uint8_t)(value >> 8);
    buf[2] = (uint8_t)(value & 0xFF);

    i2c_op_result = I2C_Master_Write(
        INA219_I2C_INDEX,
        INA219_I2C_ADDR >> 1,
        buf,
        3,
        true,
        INA219_I2C_OP_TIMEOUT
    );

    return INA219_Map_I2C_HW_Error_Code(i2c_op_result);
}

static INA219_OpResult INA219_ReadRegister(
    INA219_Register reg,
    uint16_t *value
)
{
    hwI2C_OpResult i2c_op_result;

    uint8_t buf[2];

    if(value == NULL)
    {
        return INA219_InvalidParameter;
    }

    buf[0] = (uint8_t)reg;

    i2c_op_result = I2C_Master_Write(
        INA219_I2C_INDEX,
        INA219_I2C_ADDR >> 1,
        buf,
        1,
        true,
        INA219_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return INA219_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    i2c_op_result = I2C_Master_Read(
        INA219_I2C_INDEX,
        INA219_I2C_ADDR >> 1,
        buf,
        2,
        true,
        INA219_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return INA219_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    *value = ((uint16_t)buf[0] << 8) | buf[1];

    return INA219_OK;
}

static int INA219_RewriteCalibration(void)
{
    if (INA219_CalValue == 0) {
        return INA219_NotCalibrated;
    }

    return INA219_WriteRegister(
        INA219_Register_Calibration,
        INA219_CalValue
    );
}

INA219_OpResult INA219_SetCalibration(INA219_CalibrationMode mode)
{
    int status;
    uint16_t config;

    switch (mode) {
        case INA219_Calibration_32V_2A:
            INA219_CalValue = 4096;
            INA219_CurrentDivider_mA = 10.0f;
            INA219_PowerMultiplier_mW = 2.0f;

            config = INA219_BusVoltageRange_32V |
                     INA219_Gain_8_320mV |
                     INA219_BusADC_12Bit |
                     INA219_ShuntADC_12Bit_1S_532us |
                     INA219_Mode_ShuntBusContinuous;
            break;

        case INA219_Calibration_32V_1A:
            INA219_CalValue = 10240;
            INA219_CurrentDivider_mA = 25.0f;
            INA219_PowerMultiplier_mW = 0.8f;

            config = INA219_BusVoltageRange_32V |
                     INA219_Gain_8_320mV |
                     INA219_BusADC_12Bit |
                     INA219_ShuntADC_12Bit_1S_532us |
                     INA219_Mode_ShuntBusContinuous;
            break;

        case INA219_Calibration_16V_400mA:
            INA219_CalValue = 8192;
            INA219_CurrentDivider_mA = 20.0f;
            INA219_PowerMultiplier_mW = 1.0f;

            config = INA219_BusVoltageRange_16V |
                     INA219_Gain_1_40mV |
                     INA219_BusADC_12Bit |
                     INA219_ShuntADC_12Bit_1S_532us |
                     INA219_Mode_ShuntBusContinuous;
            break;

        default:
            return INA219_InvalidParameter;
    }

    status = INA219_WriteRegister(
        INA219_Register_Calibration,
        INA219_CalValue
    );

    if (status != INA219_OK) {
        return status;
    }

    return INA219_WriteRegister(
        INA219_Register_Config,
        config
    );
}

INA219_OpResult INA219_Init(void)
{
    INA219_CalValue = 0;
    INA219_CurrentDivider_mA = 0.0f;
    INA219_PowerMultiplier_mW = 0.0f;

    INA219_Initialized = 1;

    return INA219_SetCalibration(INA219_Calibration_32V_2A);
}

INA219_OpResult INA219_EnterPowerSave(void)
{
    int status;
    uint16_t config;

    status = INA219_ReadRegister(INA219_Register_Config, &config);
    if (status != INA219_OK) {
        return status;
    }

    config &= ~INA219_ModeMask;
    config |= INA219_Mode_PowerDown;

    return INA219_WriteRegister(
        INA219_Register_Config,
        config
    );
}

INA219_OpResult INA219_ExitPowerSave(void)
{
    int status;
    uint16_t config;

    status = INA219_ReadRegister(INA219_Register_Config, &config);
    if (status != INA219_OK) {
        return status;
    }

    config &= ~INA219_ModeMask;
    config |= INA219_Mode_ShuntBusContinuous;

    return INA219_WriteRegister(
        INA219_Register_Config,
        config
    );
}

INA219_OpResult INA219_GetShuntVoltage_mV(float *value)
{
    int status;
    uint16_t reg;
    int16_t raw;

    if (value == 0) {
        return INA219_InvalidParameter;
    }

    status = INA219_ReadRegister(
        INA219_Register_ShuntVoltage,
        &reg
    );

    if (status != INA219_OK) {
        return status;
    }

    raw = (int16_t)reg;

    *value = (float)raw * 0.01f;

    return INA219_OK;
}

INA219_OpResult INA219_GetBusVoltage_V(float *value)
{
    int status;
    uint16_t reg;

    if (value == 0) {
        return INA219_InvalidParameter;
    }

    status = INA219_ReadRegister(
        INA219_Register_BusVoltage,
        &reg
    );

    if (status != INA219_OK) {
        return status;
    }

    *value = (float)((reg >> 3) * 4) * 0.001f;

    return INA219_OK;
}

INA219_OpResult INA219_GetAmperage_mA(float *value)
{
    int status;
    uint16_t reg;
    int16_t raw;

    if (value == 0) {
        return INA219_InvalidParameter;
    }

    if (INA219_CurrentDivider_mA <= 0.0f) {
        return INA219_NotCalibrated;
    }

    status = INA219_RewriteCalibration();
    if (status != INA219_OK) {
        return status;
    }

    status = INA219_ReadRegister(
        INA219_Register_Current,
        &reg
    );

    if (status != INA219_OK) {
        return status;
    }

    raw = (int16_t)reg;

    *value = (float)raw / INA219_CurrentDivider_mA;

    return INA219_OK;
}

INA219_OpResult INA219_GetPower_mW(float *value)
{
    int status;
    uint16_t reg;

    if (value == 0) {
        return INA219_InvalidParameter;
    }

    if (INA219_PowerMultiplier_mW <= 0.0f) {
        return INA219_NotCalibrated;
    }

    status = INA219_RewriteCalibration();
    if (status != INA219_OK) {
        return status;
    }

    status = INA219_ReadRegister(
        INA219_Register_Power,
        &reg
    );

    if (status != INA219_OK) {
        return status;
    }

    *value = (float)reg * INA219_PowerMultiplier_mW;

    return INA219_OK;
}