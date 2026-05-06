
#ifndef I2C_MASTER_STM32_H
#define I2C_MASTER_STM32_H

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

//hwI2C_OpResult I2C_Instance_Init(hwI2C_Index index, hwI2C_Speed_Mode speed_mode);

//hwI2C_OpResult I2C_Instance_DeInit(hwI2C_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //I2C_MASTER_STM32_H