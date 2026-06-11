
#ifndef INA219_H
#define INA219_H

#include "Sensor_Config.h"

#define INA219_Address_GND_GND  0x80
#define INA219_Address_GND_VCC  0x82
#define INA219_Address_VCC_GND  0x84
#define INA219_Address_VCC_VCC  0x86

#ifndef CONFIG_INA219_I2C_INDEX
#define INA219_I2C_INDEX           hwI2C_Index_0
#else
#define INA219_I2C_INDEX           CONFIG_INA219_I2C_INDEX
#endif

#ifdef CONFIG_INA219_I2C_ADDR
#define INA219_I2C_ADDR CONFIG_INA219_I2C_ADDR
#else
#define INA219_I2C_ADDR INA219_Address_GND_GND
#endif

#include <stdint.h>

typedef enum {
    INA219_OK = 0,
    INA219_NotInit = -1,
    INA219_InvalidParameter = -2,
    INA219_MemoryError = -3,
    INA219_MutexTimeout = -4,
    INA219_IO_Error = -5,
    INA219_Unsupport = -6,
    INA219_NotCalibrated = -7
} INA219_OpResult;

typedef enum {
    INA219_Register_Config       = 0x00,
    INA219_Register_ShuntVoltage = 0x01,
    INA219_Register_BusVoltage   = 0x02,
    INA219_Register_Power        = 0x03,
    INA219_Register_Current      = 0x04,
    INA219_Register_Calibration  = 0x05
} INA219_Register;

typedef enum {
    INA219_ModeMask = 0x0007
} INA219_Mask;

typedef enum {
    INA219_BusVoltageRange_16V = 0x0000,
    INA219_BusVoltageRange_32V = 0x2000
} INA219_BusVoltageRange;

typedef enum {
    INA219_Gain_1_40mV  = 0x0000,
    INA219_Gain_2_80mV  = 0x0800,
    INA219_Gain_4_160mV = 0x1000,
    INA219_Gain_8_320mV = 0x1800
} INA219_Gain;

typedef enum {
    INA219_BusADC_12Bit = 0x0180
} INA219_BusADCResolution;

typedef enum {
    INA219_ShuntADC_12Bit_1S_532us = 0x0018
} INA219_ShuntADCResolution;

typedef enum {
    INA219_Mode_PowerDown          = 0x0000,
    INA219_Mode_ShuntBusContinuous = 0x0007
} INA219_Mode;

typedef enum {
    INA219_Calibration_32V_2A = 0,
    INA219_Calibration_32V_1A,
    INA219_Calibration_16V_400mA
} INA219_CalibrationMode;

INA219_OpResult INA219_Init(void);

INA219_OpResult INA219_SetCalibration(
    INA219_CalibrationMode mode
);

INA219_OpResult INA219_EnterPowerSave(void);
INA219_OpResult INA219_ExitPowerSave(void);

INA219_OpResult INA219_GetShuntVoltage_mV(float *value);
INA219_OpResult INA219_GetBusVoltage_V(float *value);
INA219_OpResult INA219_GetAmperage_mA(float *value);
INA219_OpResult INA219_GetPower_mW(float *value);

#endif