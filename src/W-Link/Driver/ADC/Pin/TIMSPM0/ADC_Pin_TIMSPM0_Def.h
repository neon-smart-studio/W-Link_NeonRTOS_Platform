
#ifndef ADC_PIN_TIMSPM0_DEF_H
#define ADC_PIN_TIMSPM0_DEF_H

#include "soc.h"

#include "GPIO/GPIO.h"

#include "ADC/Device/TIMSPM0/ADC_TIMSPM0_Instance.h"

typedef struct {
    hwGPIO_Pin adc_pin;
    hwADC_Instance inst;
} ADC_Channel_Def;

#endif //ADC_PIN_TIMSPM0_DEF_H