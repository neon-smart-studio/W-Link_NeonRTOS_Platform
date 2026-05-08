/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "HDC1080.h"

#define HDC1080_I2C_OP_TIMEOUT           500

typedef enum HDC1080_Register_t
{
    HDC1080_Register_Temperature         = 0x00,
    HDC1080_Register_Humidity            = 0x01,
    HDC1080_Register_Configuration       = 0x02,

    HDC1080_Register_Serial_First        = 0xFB,
    HDC1080_Register_Serial_Mid          = 0xFC,
    HDC1080_Register_Serial_Last         = 0xFD,

    HDC1080_Register_Manufacturer_ID     = 0xFE,
    HDC1080_Register_Device_ID           = 0xFF

} HDC1080_Register;

typedef union HDC1080_Configuration_t
{
    uint16_t rawData;

    struct
    {
        uint16_t Reserved0                         : 8;
        uint16_t HumidityMeasurementResolution    : 2;
        uint16_t TemperatureMeasurementResolution : 1;
        uint16_t BatteryStatus                    : 1;
        uint16_t ModeOfAcquisition                : 1;
        uint16_t Heater                           : 1;
        uint16_t Reserved1                        : 1;
        uint16_t SoftwareReset                    : 1;
    };

} HDC1080_Configuration;

static HDC1080_OpStatus HDC1080_Map_I2C_HW_Error_Code(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return HDC1080_OK;

        case hwI2C_NotInit:
            return HDC1080_NotInit;

        case hwI2C_InvalidParameter:
            return HDC1080_InvalidParameter;

        case hwI2C_MemoryError:
            return HDC1080_MemoryError;

        case hwI2C_MutexTimeout:
            return HDC1080_MutexTimeout;

        case hwI2C_SlaveTimeout:
        case hwI2C_BusError:
            return HDC1080_IO_Error;

        case hwI2C_Unsupport:
        default:
            return HDC1080_Unsupport;
    }
}

static HDC1080_OpStatus HDC1080_Write_Register(uint8_t reg, const uint8_t* data, uint8_t len)
{
    hwI2C_OpResult i2c_op_result;

    uint8_t tx_buf[8];

    if(data == NULL || len == 0)
    {
        return HDC1080_InvalidParameter;
    }

    if((len + 1) > sizeof(tx_buf))
    {
        return HDC1080_InvalidParameter;
    }

    tx_buf[0] = reg;
    memcpy(&tx_buf[1], data, len);

    i2c_op_result = I2C_Master_Write(
        HDC1080_I2C_INDEX,
        HDC1080_I2C_ADDR >> 1,
        tx_buf,
        len + 1,
        true,
        HDC1080_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return HDC1080_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return HDC1080_OK;
}

static HDC1080_OpStatus HDC1080_Read_Register(uint8_t reg, uint8_t* data, uint8_t len)
{
    hwI2C_OpResult i2c_op_result;

    if(data == NULL || len == 0)
    {
        return HDC1080_InvalidParameter;
    }

    i2c_op_result = I2C_Master_Write(
        HDC1080_I2C_INDEX,
        HDC1080_I2C_ADDR >> 1,
        &reg,
        1,
        true,
        HDC1080_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return HDC1080_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    i2c_op_result = I2C_Master_Read(
        HDC1080_I2C_INDEX,
        HDC1080_I2C_ADDR >> 1,
        data,
        len,
        true,
        HDC1080_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return HDC1080_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return HDC1080_OK;
}

static HDC1080_OpStatus HDC1080_Read_Register16(uint8_t reg, uint16_t* value)
{
    HDC1080_OpStatus status;

    uint8_t buf[2];

    if(value == NULL)
    {
        return HDC1080_InvalidParameter;
    }

    status = HDC1080_Read_Register(reg, buf, 2);
    if(status < HDC1080_OK)
    {
        return status;
    }

    *value = ((uint16_t)buf[0] << 8) | buf[1];

    return HDC1080_OK;
}

static HDC1080_OpStatus HDC1080_Write_Register16(uint8_t reg, uint16_t value)
{
    uint8_t buf[2];

    buf[0] = (uint8_t)((value >> 8) & 0xFF);
    buf[1] = (uint8_t)(value & 0xFF);

    return HDC1080_Write_Register(reg, buf, 2);
}

static HDC1080_OpStatus HDC1080_Read_Measurement16(uint8_t reg, uint16_t* value)
{
    hwI2C_OpResult i2c_op_result;

    uint8_t buf[2];
    uint16_t ms_cnt;

    if(value == NULL)
    {
        return HDC1080_InvalidParameter;
    }

    i2c_op_result = I2C_Master_Write(
        HDC1080_I2C_INDEX,
        HDC1080_I2C_ADDR >> 1,
        &reg,
        1,
        true,
        HDC1080_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return HDC1080_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    ms_cnt = 0;

    while(1)
    {
        i2c_op_result = I2C_Master_Read(
                HDC1080_I2C_INDEX,
                HDC1080_I2C_ADDR >> 1,
                buf,
                2,
                true,
                HDC1080_I2C_OP_TIMEOUT
        );

        if(i2c_op_result == hwI2C_OK)
        {
            break;
        }

        ms_cnt++;

        if(ms_cnt >= HDC1080_I2C_OP_TIMEOUT)
        {
            return HDC1080_IO_Error;
        }

        NeonRTOS_Sleep(1);
    }

    *value = ((uint16_t)buf[0] << 8) | buf[1];

    return HDC1080_OK;
}

static HDC1080_OpStatus HDC1080_ReadConfiguration(HDC1080_Configuration* config_data)
{
    HDC1080_OpStatus status;

    uint16_t raw_config;

    if(config_data == NULL)
    {
        return HDC1080_InvalidParameter;
    }

    status = HDC1080_Read_Register16(HDC1080_Register_Configuration, &raw_config);
    if(status < HDC1080_OK)
    {
        return status;
    }

    config_data->rawData = raw_config;

    return HDC1080_OK;
}

static HDC1080_OpStatus HDC1080_WriteConfiguration(const HDC1080_Configuration* config_data)
{
    HDC1080_OpStatus status;

    if(config_data == NULL)
    {
        return HDC1080_InvalidParameter;
    }

    status = HDC1080_Write_Register16(
        HDC1080_Register_Configuration,
        config_data->rawData
    );

    if(status < HDC1080_OK)
    {
        return status;
    }

    NeonRTOS_Sleep(10);

    return HDC1080_OK;
}

HDC1080_OpStatus HDC1080_Init(void)
{
    return HDC1080_SetResolution(
        HDC1080_MeasurementResolution_14Bit,
        HDC1080_MeasurementResolution_14Bit
    );
}

HDC1080_OpStatus HDC1080_SetResolution(
    HDC1080_MeasurementResolution ms_temp_resolution,
    HDC1080_MeasurementResolution ms_hum_resolution
)
{
    HDC1080_OpStatus status;
    HDC1080_Configuration config;

    if(ms_temp_resolution >= HDC1080_MeasurementResolution_MAX ||
       ms_hum_resolution >= HDC1080_MeasurementResolution_MAX)
    {
        return HDC1080_InvalidParameter;
    }

    if(ms_temp_resolution == HDC1080_MeasurementResolution_8Bit)
    {
        return HDC1080_InvalidParameter;
    }

    status = HDC1080_ReadConfiguration(&config);
    if(status < HDC1080_OK)
    {
        return status;
    }

    switch(ms_temp_resolution)
    {
        case HDC1080_MeasurementResolution_14Bit:
            config.TemperatureMeasurementResolution = 0;
            break;

        case HDC1080_MeasurementResolution_11Bit:
            config.TemperatureMeasurementResolution = 1;
            break;

        default:
            return HDC1080_InvalidParameter;
    }

    switch(ms_hum_resolution)
    {
        case HDC1080_MeasurementResolution_14Bit:
            config.HumidityMeasurementResolution = 0;
            break;

        case HDC1080_MeasurementResolution_11Bit:
            config.HumidityMeasurementResolution = 1;
            break;

        case HDC1080_MeasurementResolution_8Bit:
            config.HumidityMeasurementResolution = 2;
            break;

        default:
            return HDC1080_InvalidParameter;
    }

    return HDC1080_WriteConfiguration(&config);
}

HDC1080_OpStatus HDC1080_ReadSerialNumber(HDC1080_SerialNumber* sn)
{
    HDC1080_OpStatus status;

    if(sn == NULL)
    {
        return HDC1080_InvalidParameter;
    }

    status = HDC1080_Read_Register16(HDC1080_Register_Serial_First, &sn->serialFirst);
    if(status < HDC1080_OK)
    {
        return status;
    }

    status = HDC1080_Read_Register16(HDC1080_Register_Serial_Mid, &sn->serialMid);
    if(status < HDC1080_OK)
    {
        return status;
    }

    status = HDC1080_Read_Register16(HDC1080_Register_Serial_Last, &sn->serialLast);
    if(status < HDC1080_OK)
    {
        return status;
    }

    return HDC1080_OK;
}

HDC1080_OpStatus HDC1080_HeatUp(uint8_t seconds)
{
    HDC1080_OpStatus status;
    hwI2C_OpResult i2c_op_result;

    HDC1080_Configuration config;
    uint8_t cmd;
    uint8_t buf[4];

    if(seconds == 0)
    {
        return HDC1080_InvalidParameter;
    }

    status = HDC1080_ReadConfiguration(&config);
    if(status < HDC1080_OK)
    {
        return status;
    }

    config.Heater = 1;
    config.ModeOfAcquisition = 1;

    status = HDC1080_WriteConfiguration(&config);
    if(status < HDC1080_OK)
    {
        return status;
    }

    for(uint8_t i = 0; i < seconds; i++)
    {
        cmd = HDC1080_Register_Temperature;

        i2c_op_result = I2C_Master_Write(
                HDC1080_I2C_INDEX,
                HDC1080_I2C_ADDR >> 1,
                &cmd,
                1,
                true,
                HDC1080_I2C_OP_TIMEOUT
        );

        if(i2c_op_result < hwI2C_OK)
        {
                return HDC1080_Map_I2C_HW_Error_Code(i2c_op_result);
        }

        i2c_op_result = I2C_Master_Read(
                HDC1080_I2C_INDEX,
                HDC1080_I2C_ADDR >> 1,
                buf,
                4,
                true,
                HDC1080_I2C_OP_TIMEOUT
        );

        if(i2c_op_result < hwI2C_OK)
        {
                return HDC1080_Map_I2C_HW_Error_Code(i2c_op_result);
        }

        NeonRTOS_Sleep(1000);
    }

    config.Heater = 0;
    config.ModeOfAcquisition = 0;

    status = HDC1080_WriteConfiguration(&config);
    if(status < HDC1080_OK)
    {
        return status;
    }

    return HDC1080_OK;
}

HDC1080_OpStatus HDC1080_ReadTemperature(double* temperature)
{
    HDC1080_OpStatus status;

    uint16_t rawT;

    if(temperature == NULL)
    {
        return HDC1080_InvalidParameter;
    }

    status = HDC1080_Read_Measurement16(HDC1080_Register_Temperature, &rawT);
    if(status < HDC1080_OK)
    {
        return status;
    }

    *temperature = ((double)rawT * 165.0 / 65536.0) - 40.0;

    return HDC1080_OK;
}

HDC1080_OpStatus HDC1080_ReadHumidity(double* humidity)
{
    HDC1080_OpStatus status;

    uint16_t rawH;

    if(humidity == NULL)
    {
        return HDC1080_InvalidParameter;
    }

    status = HDC1080_Read_Measurement16(HDC1080_Register_Humidity, &rawH);
    if(status < HDC1080_OK)
    {
        return status;
    }

    *humidity = ((double)rawH * 100.0 / 65536.0);

    return HDC1080_OK;
}

HDC1080_OpStatus HDC1080_ReadManufacturerId(uint16_t* manuID)
{
    if(manuID == NULL)
    {
        return HDC1080_InvalidParameter;
    }

    return HDC1080_Read_Register16(HDC1080_Register_Manufacturer_ID, manuID);
}

HDC1080_OpStatus HDC1080_ReadDeviceId(uint16_t* devID)
{
    if(devID == NULL)
    {
        return HDC1080_InvalidParameter;
    }

    return HDC1080_Read_Register16(HDC1080_Register_Device_ID, devID);
}