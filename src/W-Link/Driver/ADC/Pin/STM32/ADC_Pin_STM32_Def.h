
#ifndef ADC_PIN_STM32_DEF_H
#define ADC_PIN_STM32_DEF_H

#include "soc.h"

#include "GPIO/Device/STM32/GPIO_STM32.h"

#include "ADC/Device/STM32/ADC_STM32_Instance.h"

typedef struct {
    hwGPIO_Pin adc_pin;
    hwADC_Instance inst;
} ADC_Channel_Def;

#endif //ADC_PIN_STM32_DEF_H