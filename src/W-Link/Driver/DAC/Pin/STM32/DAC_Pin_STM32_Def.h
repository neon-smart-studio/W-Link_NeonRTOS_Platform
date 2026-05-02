
#ifndef DAC_PIN_STM32_DEF_H
#define DAC_PIN_STM32_DEF_H

#include "GPIO/Device/STM32/GPIO_STM32.h"

#include "DAC/Device/STM32/DAC_STM32_Instance.h"

typedef struct {
    hwGPIO_Pin dac_pin;
    hwDAC_Instance inst;
} DAC_Channel_Def;

#endif //DAC_PIN_STM32_DEF_H