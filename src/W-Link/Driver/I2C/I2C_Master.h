
#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"  

#include "I2C_Def.h"

#include "I2C_Index.h"

hwI2C_OpResult I2C_Master_Init(hwI2C_Index index, hwI2C_Speed_Mode speed_mode);
hwI2C_OpResult I2C_Master_DeInit(hwI2C_Index index);
hwI2C_OpResult I2C_Master_Reset(hwI2C_Index index);
hwI2C_OpResult I2C_Master_Read(hwI2C_Index index, uint8_t address, uint8_t *read_dat, uint8_t read_len, bool stop, NeonRTOS_Time_t timeoutMs);
hwI2C_OpResult I2C_Master_Write(hwI2C_Index index, uint8_t address, uint8_t *write_dat, uint8_t write_len, bool stop, NeonRTOS_Time_t timeoutMs);
hwI2C_OpResult I2C_Master_Read_FromISR(hwI2C_Index index, uint8_t address, uint8_t *read_dat, uint8_t read_len, bool stop, NeonRTOS_Time_t timeoutMs);
hwI2C_OpResult I2C_Master_Write_FromISR(hwI2C_Index index, uint8_t address, uint8_t *write_dat, uint8_t write_len, bool stop, NeonRTOS_Time_t timeoutMs);
bool I2C_Master_isInit(hwI2C_Index index);

#endif // I2C_MASTER_H