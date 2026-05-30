
#ifndef I2C_MASTER_NUVOTON_H
#define I2C_MASTER_NUVOTON_H

#include "soc.h"

#include "I2C/I2C_Master.h"
#include "GPIO/GPIO.h"

#define I2C_MASTER_STANDARD_MODE_CLK_FREQUENCY         100000
#define I2C_MASTER_FAST_MODE_CLK_FREQUENCY             400000
#define I2C_MASTER_HIGH_SPEED_MODE_CLK_FREQUENCY       1000000

#ifdef	__cplusplus
extern "C" {
#endif

extern bool I2C_Master_Init_Status[];

I2C_T *I2C_Map_Soc_Base(hwI2C_Index index);

void I2C_MasterTxCpltCallback(hwI2C_Index index);
void I2C_MasterRxCpltCallback(hwI2C_Index index);
void I2C_ErrorCallback(hwI2C_Index index);

void I2C_GPIO_ConfigAF(hwI2C_Index index);
void I2C_GPIO_DeConfigAF(hwI2C_Index index);

hwI2C_OpResult I2C_Instance_Init(hwI2C_Index index, hwI2C_Speed_Mode speed_mode);
hwI2C_OpResult I2C_Instance_DeInit(hwI2C_Index index);
hwI2C_OpResult I2C_Transfer_Write(hwI2C_Index index, uint8_t address, uint8_t *write_dat, uint8_t write_len, bool stop, NeonRTOS_Time_t timeoutMs);
hwI2C_OpResult I2C_Transfer_Read(hwI2C_Index index, uint8_t address, uint8_t *read_dat, uint8_t read_len, bool stop, NeonRTOS_Time_t timeoutMs);
hwI2C_OpResult I2C_Transfer_Get_Status(hwI2C_Index index);
hwI2C_OpResult I2C_Transfer_Stop(hwI2C_Index index);

void I2C_NVIC_Init(hwI2C_Index index);
void I2C_NVIC_DeInit(hwI2C_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //I2C_MASTER_NUVOTON_H