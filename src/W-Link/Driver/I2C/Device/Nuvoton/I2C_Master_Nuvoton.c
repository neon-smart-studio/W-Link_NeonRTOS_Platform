#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#ifdef DEVICE_NUVOTON

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "I2C/Pin/Nuvoton/I2C_Pin_Nuvoton.h"

#include "I2C_Master_Nuvoton.h"

bool I2C_Master_Init_Status[hwI2C_Index_MAX] = {false};

static hwI2C_Speed_Mode I2C_Clock_Speed_Mode[hwI2C_Index_MAX] = {hwI2C_Standard_Mode};
static NeonRTOS_SyncObj_t I2C_Master_Done_SyncHandle[hwI2C_Index_MAX];

void I2C_MasterTxCpltCallback(hwI2C_Index index)
{
    if (index < hwI2C_Index_MAX) {
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
    }
}

void I2C_MasterRxCpltCallback(hwI2C_Index index)
{
    if (index < hwI2C_Index_MAX) {
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
    }
}

void I2C_ErrorCallback(hwI2C_Index index)
{
    if (index < hwI2C_Index_MAX) {
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
    }
}

hwI2C_OpResult I2C_Master_Init(hwI2C_Index index,
                               hwI2C_Speed_Mode speed_mode)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(I2C_Master_Init_Status[index])
        return hwI2C_OK;

    if(speed_mode >= hwI2C_Speed_Mode_MAX)
        return hwI2C_InvalidParameter;

    I2C_T *i2c = I2C_Map_Soc_Base(index);

    if(i2c == NULL)
        return hwI2C_InvalidParameter;

    I2C_Pin_Def def =
        I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]];

    if(def.scl_pin == hwGPIO_Pin_NC ||
       def.sda_pin == hwGPIO_Pin_NC)
    {
        return hwI2C_InvalidParameter;
    }

    if(NeonRTOS_SyncObjCreate(&I2C_Master_Done_SyncHandle[index]) != NeonRTOS_OK)
    {
        return hwI2C_MemoryError;
    }

    I2C_GPIO_ConfigAF(index);

    I2C_Instance_Init(index, speed_mode);

    I2C_NVIC_Init(index);

    gpio_pin_init_status[def.scl_pin] = true;
    gpio_pin_init_status[def.sda_pin] = true;

    I2C_Clock_Speed_Mode[index] = speed_mode;
    I2C_Master_Init_Status[index] = true;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_DeInit(hwI2C_Index index)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(!I2C_Master_Init_Status[index])
        return hwI2C_OK;

    I2C_T *i2c = I2C_Map_Soc_Base(index);

    if(i2c == NULL)
        return hwI2C_InvalidParameter;

    I2C_Pin_Def def =
        I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]];

    I2C_NVIC_DeInit(index);

    I2C_Instance_DeInit(index);

    I2C_GPIO_DeConfigAF(index);

    gpio_pin_init_status[def.scl_pin] = false;
    gpio_pin_init_status[def.sda_pin] = false;

    NeonRTOS_SyncObjDelete(&I2C_Master_Done_SyncHandle[index]);

    I2C_Master_Init_Status[index] = false;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_Reset(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }
    if (I2C_Master_Init_Status[index] == false) {
        return hwI2C_NotInit;
    }

    hwI2C_OpResult op_status = I2C_Master_DeInit(index);
    if (op_status < hwI2C_OK) {
        return op_status;
    }

    return I2C_Master_Init(index, I2C_Clock_Speed_Mode[index]);
}

hwI2C_OpResult I2C_Master_Write(hwI2C_Index index, uint8_t address, uint8_t *write_dat, uint8_t write_len, bool stop, NeonRTOS_Time_t timeoutMs)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(!I2C_Master_Init_Status[index])
        return hwI2C_NotInit;

    if(write_dat == NULL || write_len == 0)
        return hwI2C_InvalidParameter;

    I2C_T *i2c = I2C_Map_Soc_Base(index);

    if(i2c == NULL)
        return hwI2C_InvalidParameter;

    I2C_Transfer_Write(index, address, write_dat, write_len, stop, timeoutMs);

    if(NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index],
                            timeoutMs) != NeonRTOS_OK)
    {
        I2C_Transfer_Stop(index);
        return hwI2C_SlaveTimeout;
    }

    return I2C_Transfer_Get_Status(index);
}

hwI2C_OpResult I2C_Master_Read(hwI2C_Index index, uint8_t address, uint8_t *read_dat, uint8_t read_len, bool stop, NeonRTOS_Time_t timeoutMs)
{
    if(index >= hwI2C_Index_MAX)
        return hwI2C_InvalidParameter;

    if(!I2C_Master_Init_Status[index])
        return hwI2C_NotInit;

    if(read_dat == NULL || read_len == 0)
        return hwI2C_InvalidParameter;

    I2C_T *i2c = I2C_Map_Soc_Base(index);

    if(i2c == NULL)
        return hwI2C_InvalidParameter;

    I2C_Transfer_Read(index, address, read_dat, read_len, stop, timeoutMs);

    if(NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index],
                            timeoutMs) != NeonRTOS_OK)
    {
        I2C_Transfer_Stop(index);
        return hwI2C_SlaveTimeout;
    }

    return I2C_Transfer_Get_Status(index);
}

bool I2C_Master_isInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return false;
    }
    return I2C_Master_Init_Status[index];
}

#endif // DEVICE_NUVOTON