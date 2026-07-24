#ifndef ADC_TIMSPM0_INSTANCE_H
#define ADC_TIMSPM0_INSTANCE_H

#include "GPIO/GPIO.h"
#include "ADC/ADC.h"

typedef enum
{
#if defined(ADC0_BASE)
    hwADC_Instance_1 = 0,   /* ADC0 */
#endif
#if defined(ADC1_BASE)
    hwADC_Instance_2,       /* ADC1 */
#endif
    hwADC_Instance_MAX,
} hwADC_Instance;

#endif // ADC_TIMSPM0_INSTANCE_H