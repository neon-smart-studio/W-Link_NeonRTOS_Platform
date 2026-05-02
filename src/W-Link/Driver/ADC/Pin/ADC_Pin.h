
#ifndef ADC_PIN_H
#define ADC_PIN_H

#ifdef DEVICE_STM32
#include "STM32/ADC_Pin_STM32.h"
#endif

#ifdef DEVICE_TM4C1294
#include "TI/ADC_Pin_TM4C1294.h"
#endif

#endif //ADC_PIN_H