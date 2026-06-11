/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "HMC5883L.h"

#define HMC5883L_I2C_OP_TIMEOUT           500

typedef enum HMC5883L_Register_t
{
    HMC5883L_Register_Config_A            = 0x00,
    HMC5883L_Register_Config_B            = 0x01,
    HMC5883L_Register_Mode                = 0x02,

    HMC5883L_Register_Out_X_M             = 0x03,
    HMC5883L_Register_Out_X_L             = 0x04,
    HMC5883L_Register_Out_Z_M             = 0x05,
    HMC5883L_Register_Out_Z_L             = 0x06,
    HMC5883L_Register_Out_Y_M             = 0x07,
    HMC5883L_Register_Out_Y_L             = 0x08,

    HMC5883L_Register_Status              = 0x09,

    HMC5883L_Register_Identification_A    = 0x0A,
    HMC5883L_Register_Identification_B    = 0x0B,
    HMC5883L_Register_Identification_C    = 0x0C

} HMC5883L_Register;

#define HMC5883L_IDENTIFICATION_CODE_A     0x48
#define HMC5883L_IDENTIFICATION_CODE_B     0x34
#define HMC5883L_IDENTIFICATION_CODE_C     0x33

static int32_t hmc5883l_x_offset = 0;
static int32_t hmc5883l_y_offset = 0;
static float hmc5883l_mg_per_digit = 0.92f;

static HMC5883L_OpResult HMC5883L_Map_I2C_HW_Error_Code(hwI2C_OpResult error_code)
{
    switch(error_code)
    {
        case hwI2C_OK:
            return HMC5883L_OK;

        case hwI2C_NotInit:
            return HMC5883L_NotInit;

        case hwI2C_InvalidParameter:
            return HMC5883L_InvalidParameter;

        case hwI2C_MemoryError:
            return HMC5883L_MemoryError;

        case hwI2C_MutexTimeout:
            return HMC5883L_MutexTimeout;

        case hwI2C_SlaveTimeout:
        case hwI2C_BusError:
            return HMC5883L_IO_Error;

        case hwI2C_Unsupport:
        default:
            return HMC5883L_Unsupport;
    }
}

static HMC5883L_OpResult HMC5883L_Write_Byte(uint8_t reg, uint8_t value)
{
    hwI2C_OpResult i2c_op_result;
    HMC5883L_OpResult status;

    uint8_t tx_buf[2];

    tx_buf[0] = reg;
    tx_buf[1] = value;

    i2c_op_result = I2C_Master_Write(
        HMC5883L_I2C_INDEX,
        HMC5883L_I2C_ADDR >> 1,
        tx_buf,
        2,
        true,
        HMC5883L_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        HMC5883L_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return HMC5883L_OK;
}

static HMC5883L_OpResult HMC5883L_Read(uint8_t reg, uint8_t* data, uint8_t len)
{
    hwI2C_OpResult i2c_op_result;
    HMC5883L_OpResult status;

    if(data == NULL || len == 0)
    {
        return HMC5883L_InvalidParameter;
    }

    i2c_op_result = I2C_Master_Write(
        HMC5883L_I2C_INDEX,
        HMC5883L_I2C_ADDR >> 1,
        &reg,
        1,
        true,
        HMC5883L_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        HMC5883L_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    i2c_op_result = I2C_Master_Read(
        HMC5883L_I2C_INDEX,
        HMC5883L_I2C_ADDR >> 1,
        data,
        len,
        true,
        HMC5883L_I2C_OP_TIMEOUT
    );

    if(i2c_op_result < hwI2C_OK)
    {
        HMC5883L_Map_I2C_HW_Error_Code(i2c_op_result);
    }

    return HMC5883L_OK;
}

static HMC5883L_OpResult HMC5883L_Read_Byte(uint8_t reg, uint8_t* value)
{
    if(value == NULL)
    {
        return HMC5883L_InvalidParameter;
    }

    return HMC5883L_Read(reg, value, 1);
}

static int16_t HMC5883L_Make_Int16(uint8_t msb, uint8_t lsb)
{
    return (int16_t)(((uint16_t)msb << 8) | lsb);
}

static HMC5883L_OpResult HMC5883L_Read_Raw_Internal(int16_t* x, int16_t* y, int16_t* z)
{
    HMC5883L_OpResult status;
    uint8_t buf[6];

    if(x == NULL || y == NULL || z == NULL)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read(HMC5883L_Register_Out_X_M, buf, 6);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    *x = HMC5883L_Make_Int16(buf[0], buf[1]);
    *z = HMC5883L_Make_Int16(buf[2], buf[3]);
    *y = HMC5883L_Make_Int16(buf[4], buf[5]);

    return HMC5883L_OK;
}

HMC5883L_OpResult HMC5883L_ReadRaw(HMC5883L_Vector* vector)
{
    HMC5883L_OpResult status;
    int16_t raw_x;
    int16_t raw_y;
    int16_t raw_z;

    if(vector == NULL)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read_Raw_Internal(&raw_x, &raw_y, &raw_z);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    vector->XAxis = (float)raw_x - hmc5883l_x_offset;
    vector->YAxis = (float)raw_y - hmc5883l_y_offset;
    vector->ZAxis = (float)raw_z;

    return HMC5883L_OK;
}

HMC5883L_OpResult HMC5883L_ReadNormalize(HMC5883L_Vector* vector)
{
    HMC5883L_OpResult status;
    int16_t raw_x;
    int16_t raw_y;
    int16_t raw_z;

    if(vector == NULL)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read_Raw_Internal(&raw_x, &raw_y, &raw_z);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    vector->XAxis = ((float)raw_x - hmc5883l_x_offset) * hmc5883l_mg_per_digit;
    vector->YAxis = ((float)raw_y - hmc5883l_y_offset) * hmc5883l_mg_per_digit;
    vector->ZAxis = ((float)raw_z) * hmc5883l_mg_per_digit;

    return HMC5883L_OK;
}

HMC5883L_OpResult HMC5883L_SetOffset(int xOff, int yOff)
{
    hmc5883l_x_offset = xOff;
    hmc5883l_y_offset = yOff;

    return HMC5883L_OK;
}

HMC5883L_OpResult HMC5883L_SetRange(HMC5883L_Range range)
{
    HMC5883L_OpResult status;
    float mg_per_digit;

    if(range >= HMC5883L_Range_MAX)
    {
        return HMC5883L_InvalidParameter;
    }

    switch(range)
    {
        case HMC5883L_Range_0_88Ga:
            mg_per_digit = 0.073f;
            break;

        case HMC5883L_Range_1_3Ga:
            mg_per_digit = 0.92f;
            break;

        case HMC5883L_Range_1_9Ga:
            mg_per_digit = 1.22f;
            break;

        case HMC5883L_Range_2_5Ga:
            mg_per_digit = 1.52f;
            break;

        case HMC5883L_Range_4Ga:
            mg_per_digit = 2.27f;
            break;

        case HMC5883L_Range_4_7Ga:
            mg_per_digit = 2.56f;
            break;

        case HMC5883L_Range_5_6Ga:
            mg_per_digit = 3.03f;
            break;

        case HMC5883L_Range_8_1Ga:
            mg_per_digit = 4.35f;
            break;

        default:
            return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Write_Byte(
        HMC5883L_Register_Config_B,
        (uint8_t)(range << 5)
    );

    if(status < HMC5883L_OK)
    {
        return status;
    }

    hmc5883l_mg_per_digit = mg_per_digit;

    return HMC5883L_OK;
}

HMC5883L_OpResult HMC5883L_GetRange(HMC5883L_Range* range)
{
    HMC5883L_OpResult status;
    uint8_t value;

    if(range == NULL)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read_Byte(HMC5883L_Register_Config_B, &value);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    *range = (HMC5883L_Range)(value >> 5);

    return HMC5883L_OK;
}

HMC5883L_OpResult HMC5883L_SetMeasurementMode(HMC5883L_Mode mode)
{
    HMC5883L_OpResult status;
    uint8_t value;

    if(mode >= HMC5883L_Mode_MAX)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read_Byte(HMC5883L_Register_Mode, &value);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    value &= 0xFC;
    value |= (uint8_t)mode;

    return HMC5883L_Write_Byte(HMC5883L_Register_Mode, value);
}

HMC5883L_OpResult HMC5883L_GetMeasurementMode(HMC5883L_Mode* mode)
{
    HMC5883L_OpResult status;
    uint8_t value;

    if(mode == NULL)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read_Byte(HMC5883L_Register_Mode, &value);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    *mode = (HMC5883L_Mode)(value & 0x03);

    return HMC5883L_OK;
}

HMC5883L_OpResult HMC5883L_SetDataRate(HMC5883L_DataRate dataRate)
{
    HMC5883L_OpResult status;
    uint8_t value;

    if(dataRate >= HMC5883L_DataRate_MAX)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read_Byte(HMC5883L_Register_Config_A, &value);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    value &= 0xE3;
    value |= (uint8_t)(dataRate << 2);

    return HMC5883L_Write_Byte(HMC5883L_Register_Config_A, value);
}

HMC5883L_OpResult HMC5883L_GetDataRate(HMC5883L_DataRate* dataRate)
{
    HMC5883L_OpResult status;
    uint8_t value;

    if(dataRate == NULL)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read_Byte(HMC5883L_Register_Config_A, &value);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    *dataRate = (HMC5883L_DataRate)((value & 0x1C) >> 2);

    return HMC5883L_OK;
}

HMC5883L_OpResult HMC5883L_SetSamples(HMC5883L_Samples samples)
{
    HMC5883L_OpResult status;
    uint8_t value;

    if(samples >= HMC5883L_Samples_MAX)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read_Byte(HMC5883L_Register_Config_A, &value);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    value &= 0x9F;
    value |= (uint8_t)(samples << 5);

    return HMC5883L_Write_Byte(HMC5883L_Register_Config_A, value);
}

HMC5883L_OpResult HMC5883L_GetSamples(HMC5883L_Samples* samples)
{
    HMC5883L_OpResult status;
    uint8_t value;

    if(samples == NULL)
    {
        return HMC5883L_InvalidParameter;
    }

    status = HMC5883L_Read_Byte(HMC5883L_Register_Config_A, &value);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    *samples = (HMC5883L_Samples)((value & 0x60) >> 5);

    return HMC5883L_OK;
}

HMC5883L_OpResult HMC5883L_Init(void)
{
    HMC5883L_OpResult status;
    uint8_t id_a;
    uint8_t id_b;
    uint8_t id_c;

    status = HMC5883L_Read_Byte(HMC5883L_Register_Identification_A, &id_a);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    status = HMC5883L_Read_Byte(HMC5883L_Register_Identification_B, &id_b);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    status = HMC5883L_Read_Byte(HMC5883L_Register_Identification_C, &id_c);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    if(id_a != HMC5883L_IDENTIFICATION_CODE_A ||
       id_b != HMC5883L_IDENTIFICATION_CODE_B ||
       id_c != HMC5883L_IDENTIFICATION_CODE_C)
    {
        return HMC5883L_Unsupport;
    }

    status = HMC5883L_SetRange(HMC5883L_Range_1_3Ga);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    status = HMC5883L_SetMeasurementMode(HMC5883L_Mode_Continous);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    status = HMC5883L_SetDataRate(HMC5883L_DataRate_30Hz);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    status = HMC5883L_SetSamples(HMC5883L_Samples_8);
    if(status < HMC5883L_OK)
    {
        return status;
    }

    return HMC5883L_OK;
}