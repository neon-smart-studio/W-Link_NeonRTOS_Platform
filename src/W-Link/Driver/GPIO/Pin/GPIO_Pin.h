
#ifndef GPIO_PIN_H
#define GPIO_PIN_H

#include "soc.h"

#ifdef DEVICE_STM32
#include "STM32/GPIO_Pin_STM32.h"
#endif

#ifdef DEVICE_TM4C1294
#include "TI/GPIO_Pin_TM4C1294.h"
#endif

#endif //GPIO_PIN_H