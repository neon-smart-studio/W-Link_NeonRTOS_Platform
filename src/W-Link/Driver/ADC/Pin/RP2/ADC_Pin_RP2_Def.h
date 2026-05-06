
#ifndef ADC_PIN_RP2_DEF_H
#define ADC_PIN_RP2_DEF_H

#include "soc.h"

#include "GPIO/Device/RP2/GPIO_RP2.h"

#include "ADC/ADC_Channel.h"

typedef struct {
    hwGPIO_Pin adc_pin;
    hwADC_Instance inst;
} ADC_Channel_Def;

#endif //ADC_PIN_RP2_DEF_H