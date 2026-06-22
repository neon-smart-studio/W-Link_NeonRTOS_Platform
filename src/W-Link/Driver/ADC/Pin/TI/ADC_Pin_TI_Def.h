
#ifndef ADC_PIN_TI_DEF_H
#define ADC_PIN_TI_DEF_H

#include "soc.h"

#include "GPIO/Device/TI/GPIO_TM4C1294.h"

#include "ADC/Device/TI/ADC_TM4C1294_Instance.h"

typedef struct {
    hwGPIO_Pin adc_pin;
    hwADC_Instance inst;
} ADC_Channel_Def;

#endif //ADC_PIN_TI_DEF_H