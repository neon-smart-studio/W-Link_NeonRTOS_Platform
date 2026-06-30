
#ifndef I2C_MASTER_TITIVAC_H
#define I2C_MASTER_TITIVAC_H

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

uint32_t I2C_Map_Soc_Base(hwI2C_Index index);
uint32_t I2C_Map_Soc_Periph(hwI2C_Index index);
uint32_t I2C_Map_Soc_Int(hwI2C_Index index);
uint32_t I2C_Map_PinConfig(hwI2C_Index index, hwGPIO_Pin pin);

void I2C_IRQ_Process(hwI2C_Index index);

void I2C_NVIC_Init(hwI2C_Index index);
void I2C_NVIC_DeInit(hwI2C_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //I2C_MASTER_TITIVAC_H