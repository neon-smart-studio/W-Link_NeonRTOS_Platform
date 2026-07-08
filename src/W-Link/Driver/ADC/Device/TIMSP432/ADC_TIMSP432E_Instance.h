#ifndef ADC_MSP432E_INSTANCE_H
#define ADC_MSP432E_INSTANCE_H

#include "GPIO/GPIO.h"
#include "ADC/ADC.h"

typedef enum
{
    hwADC_Instance_1 = 0,   /* ADC0 */
    hwADC_Instance_2,       /* ADC1 */
    hwADC_Instance_MAX,
} hwADC_Instance;

#endif // ADC_MSP432E_INSTANCE_H