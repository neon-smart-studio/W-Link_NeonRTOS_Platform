/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "Si7021.h"

#define SI7021_I2C_OP_TIMEOUT                   500

typedef enum Si7021_Command_t
{
    Si7021_Command_Measure_RH_Hold             = 0xE5,
    Si7021_Command_Measure_RH_NoHold           = 0xF5,

    Si7021_Command_Measure_Temp_Hold           = 0xE3,
    Si7021_Command_Measure_Temp_NoHold         = 0xF3,

    Si7021_Command_Read_Previous_Temp          = 0xE0,

    Si7021_Command_Reset                       = 0xFE,

    Si7021_Command_Write_User_Register         = 0xE6,
    Si7021_Command_Read_User_Register          = 0xE7,

    Si7021_Command_Write_Heater_Register       = 0x51,
    Si7021_Command_Read_Heater_Register        = 0x11
} Si7021_Command;

typedef enum Si7021_Command16_t
{
    Si7021_Command_Read_ID1                    = 0xFA0F,
    Si7021_Command_Read_ID2                    = 0xFCC9,
    Si7021_Command_Read_Firmware_Revision      = 0x84B8
} Si7021_Command16;

typedef enum Si7021_Revision_Code_t
{
    Si7021_Revision_Code_1                     = 0xFF,
    Si7021_Revision_Code_2                     = 0x20
} Si7021_Revision_Code;

static Si7021_OpResult Si7021_Map_I2C_HW_Error_Code(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return Si7021_OK;

        case hwI2C_NotInit:
            return Si7021_NotInit;

        case hwI2C_InvalidParameter:
            return Si7021_InvalidParameter;

        case hwI2C_MemoryError:
            return Si7021_MemoryError;

        case hwI2C_MutexTimeout:
            return Si7021_MutexTimeout;

        case hwI2C_SlaveTimeout:
        case hwI2C_BusError:
            return Si7021_IO_Error;

        case hwI2C_Unsupport:
        default:
            return Si7021_Unsupport;
    }
}

static Si7021_OpResult Si7021_Write_Command(Si7021_Command command)
{
    hwI2C_OpResult i2c_op_result;

    i2c_op_result = I2C_Master_Write(
        SI7021_I2C_INDEX,
        SI7021_I2C_ADDR >> 1,
        &command,
        1,
        true,
        SI7021_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return Si7021_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return Si7021_OK;
}

static Si7021_OpResult Si7021_Write_Command16(Si7021_Command16 command)
{
    hwI2C_OpResult i2c_op_result;

    uint8_t tx_buf[2];

    tx_buf[0] = (uint8_t)(command >> 8);
    tx_buf[1] = (uint8_t)(command & 0xFF);

    i2c_op_result = I2C_Master_Write(
        SI7021_I2C_INDEX,
        SI7021_I2C_ADDR >> 1,
        tx_buf,
        2,
        true,
        SI7021_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return Si7021_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return Si7021_OK;
}

static Si7021_OpResult Si7021_Read(uint8_t* data, uint16_t len)
{
    hwI2C_OpResult i2c_op_result;

    if(data == NULL || len == 0)
    {
        return Si7021_InvalidParameter;
    }

    i2c_op_result = I2C_Master_Read(
        SI7021_I2C_INDEX,
        SI7021_I2C_ADDR >> 1,
        data,
        len,
        true,
        SI7021_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        return Si7021_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return Si7021_OK;
}

static Si7021_OpResult Si7021_Read_Measurement(uint8_t command, uint16_t* raw_value)
{
    Si7021_OpResult status;
    uint8_t rx_buf[3];
    uint16_t timeout_ms = 0;

    if(raw_value == NULL)
    {
        return Si7021_InvalidParameter;
    }

    status = Si7021_Write_Command(command);
    if(status < Si7021_OK)
    {
        return status;
    }

    while(1)
    {
        status = Si7021_Read(rx_buf, 3);

        if(status == Si7021_OK)
        {
            break;
        }

        timeout_ms++;

        if(timeout_ms >= SI7021_I2C_OP_TIMEOUT)
        {
            return Si7021_IO_Error;
        }

        NeonRTOS_Sleep(1);
    }

    *raw_value = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];

    return Si7021_OK;
}

Si7021_OpResult Si7021_Init(void)
{
    Si7021_OpResult status;

    status = Si7021_Write_Command(Si7021_Command_Reset);
    if(status < Si7021_OK)
    {
        return status;
    }

    NeonRTOS_Sleep(20);

    return Si7021_OK;
}

Si7021_OpResult Si7021_ReadMeasureTemperature(float* temperature)
{
    Si7021_OpResult status;
    uint16_t raw_temp;

    if(temperature == NULL)
    {
        return Si7021_InvalidParameter;
    }

    status = Si7021_Read_Measurement(
        Si7021_Command_Measure_Temp_NoHold,
        &raw_temp
    );

    if(status < Si7021_OK)
    {
        return status;
    }

    *temperature = ((175.72f * (float)raw_temp) / 65536.0f) - 46.85f;

    return Si7021_OK;
}

Si7021_OpResult Si7021_ReadMeasureHumidity(float* humidity)
{
    Si7021_OpResult status;
    uint16_t raw_humidity;

    if(humidity == NULL)
    {
        return Si7021_InvalidParameter;
    }

    status = Si7021_Read_Measurement(
        Si7021_Command_Measure_RH_NoHold,
        &raw_humidity
    );

    if(status < Si7021_OK)
    {
        return status;
    }

    *humidity = ((125.0f * (float)raw_humidity) / 65536.0f) - 6.0f;

    return Si7021_OK;
}

Si7021_OpResult Si7021_ReadPreviousMeasureTemperature(float* temperature)
{
    Si7021_OpResult status;
    uint8_t rx_buf[3];
    uint16_t raw_temp;

    if(temperature == NULL)
    {
        return Si7021_InvalidParameter;
    }

    status = Si7021_Write_Command(Si7021_Command_Read_Previous_Temp);
    if(status < Si7021_OK)
    {
        return status;
    }

    status = Si7021_Read(rx_buf, 3);
    if(status < Si7021_OK)
    {
        return status;
    }

    raw_temp = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];

    *temperature = ((175.72f * (float)raw_temp) / 65536.0f) - 46.85f;

    return Si7021_OK;
}

Si7021_OpResult Si7021_ReadRevision(Si_Sensor_Rev* revision)
{
    Si7021_OpResult status;
    uint8_t rx_buf[1];

    if(revision == NULL)
    {
        return Si7021_InvalidParameter;
    }

    status = Si7021_Write_Command16(Si7021_Command_Read_Firmware_Revision);
    if(status < Si7021_OK)
    {
        return status;
    }

    status = Si7021_Read(rx_buf, 1);
    if(status < Si7021_OK)
    {
        return status;
    }

    switch(rx_buf[0])
    {
        case Si7021_Revision_Code_1:
            *revision = Si_Sensor_Rev_1;
            break;

        case Si7021_Revision_Code_2:
            *revision = Si_Sensor_Rev_2;
            break;

        default:
            return Si7021_Unsupport;
    }

    return Si7021_OK;
}

Si7021_OpResult Si7021_ReadSerialNumber(
    Si_Sensor_Model* model,
    uint8_t ID_A[4],
    uint8_t ID_B[4]
)
{
    Si7021_OpResult status;
    uint8_t rx_buf[8];
    uint8_t sensor_model;

    if(model == NULL || ID_A == NULL || ID_B == NULL)
    {
        return Si7021_InvalidParameter;
    }

    status = Si7021_Write_Command16(Si7021_Command_Read_ID1);
    if(status < Si7021_OK)
    {
        return status;
    }

    status = Si7021_Read(rx_buf, 8);
    if(status < Si7021_OK)
    {
        return status;
    }

    ID_A[0] = rx_buf[0];
    ID_A[1] = rx_buf[2];
    ID_A[2] = rx_buf[4];
    ID_A[3] = rx_buf[6];

    status = Si7021_Write_Command16(Si7021_Command_Read_ID2);
    if(status < Si7021_OK)
    {
        return status;
    }

    status = Si7021_Read(rx_buf, 8);
    if(status < Si7021_OK)
    {
        return status;
    }

    ID_B[0] = rx_buf[0];
    ID_B[1] = rx_buf[2];
    ID_B[2] = rx_buf[4];
    ID_B[3] = rx_buf[6];

    sensor_model = rx_buf[0];

    switch(sensor_model)
    {
        case 0x00:
        case 0xFF:
            *model = Si_Sensor_Model_Engineering_Samples;
            break;

        case 0x0D:
            *model = Si_Sensor_Model_7013;
            break;

        case 0x14:
            *model = Si_Sensor_Model_7020;
            break;

        case 0x15:
            *model = Si_Sensor_Model_7021;
            break;

        default:
            return Si7021_Unsupport;
    }

    return Si7021_OK;
}