
#ifndef ADC_PIN_TIMSP432P_DEF_H
#define ADC_PIN_TIMSP432P_DEF_H

#include "soc.h"

#include "GPIO/Device/TIMSP432/GPIO_TIMSP432P.h"

#include "ADC/Device/TIMSP432/ADC_TIMSP432P_Instance.h"

typedef struct {
    hwGPIO_Pin adc_pin;
    hwADC_Instance inst;
} ADC_Channel_Def;

#endif //ADC_PIN_TIMSP432P_DEF_H