
#ifndef ADC_PIN_STM32_H
#define ADC_PIN_STM32_H

#include "GPIO/GPIO.h"

#include "ADC/ADC.h"

typedef enum {
#if defined(ADC_BASE) || defined (ADC1_BASE)
    hwADC_Instance_1 = 0,
#endif
#if defined (ADC2_BASE)
    hwADC_Instance_2,
#endif
#if defined (ADC3_BASE)
    hwADC_Instance_3,
#endif
#if defined (ADC4_BASE)
    hwADC_Instance_4,
#endif
#if defined (ADC5_BASE)
    hwADC_Instance_5,
#endif
    hwADC_Instance_MAX,
} hwADC_Instance;

typedef struct {
    hwGPIO_Pin adc_pin;
    hwADC_Instance inst;
} ADC_Channel_Def;

#endif //ADC_PIN_STM32_H