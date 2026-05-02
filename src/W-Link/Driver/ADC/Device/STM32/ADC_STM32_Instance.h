
#ifndef ADC_STM32_INSTANCE_H
#define ADC_STM32_INSTANCE_H

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

#endif //ADC_STM32_INSTANCE_H