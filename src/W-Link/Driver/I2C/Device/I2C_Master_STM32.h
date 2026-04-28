
#ifndef I2C_MASTER_STM32_H
#define I2C_MASTER_STM32_H

#include "soc.h"

#include "I2C/I2C_Def.h"

#include "I2C/I2C_Index.h"

#define I2C_IRQ_NVIC_PRIORITY      5
#define I2C_IRQ_NVIC_SUB_PRIORITY  0

extern I2C_HandleTypeDef g_i2c[hwI2C_Index_MAX];
extern bool I2C_Master_Init_Status[];

I2C_TypeDef *I2C_Map_Soc_Base(hwI2C_Index index);
hwI2C_Index I2C_IndexFromHandle(I2C_HandleTypeDef *hI2C);

hwI2C_OpResult I2C_Instance_Init(
    hwI2C_Index index,
    hwI2C_Speed_Mode speed_mode
);

hwI2C_OpResult I2C_Instance_DeInit(hwI2C_Index index);

void I2C_NVIC_Init(hwI2C_Index index);
void I2C_NVIC_DeInit(hwI2C_Index index);

uint32_t I2C_Master_Get_Timing(hwI2C_Index index, hwI2C_Speed_Mode speed_mode);

#endif //I2C_MASTER_STM32_H