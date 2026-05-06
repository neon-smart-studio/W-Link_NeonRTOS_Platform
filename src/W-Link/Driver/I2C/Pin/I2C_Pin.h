
#ifndef I2C_PIN_H
#define I2C_PIN_H

#ifdef DEVICE_STM32
#include "STM32/I2C_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/I2C_Pin_RP2.h"
#endif

#ifdef DEVICE_TM4C1294
#include "TI/I2C_Pin_TM4C1294.h"
#endif

#endif //I2C_PIN_H