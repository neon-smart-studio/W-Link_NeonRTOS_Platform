#ifndef ADC_MSP432P_INSTANCE_H
#define ADC_MSP432P_INSTANCE_H

#include "GPIO/GPIO.h"
#include "ADC/ADC.h"

#if defined(MSP432P)
typedef enum
{
    hwADC_Instance_1 = 0,   /* ADC14 */
    hwADC_Instance_MAX,
} hwADC_Instance;
#endif 

#if defined(MSP432E)
typedef enum
{
    hwADC_Instance_1 = 0,   /* ADC0 */
    hwADC_Instance_2,       /* ADC1 */
    hwADC_Instance_MAX,
} hwADC_Instance;
#endif 

#endif // ADC_MSP432P_INSTANCE_H