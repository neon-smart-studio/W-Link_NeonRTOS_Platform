
#ifndef ADC_PIN_TI_DEF_H
#define ADC_PIN_TI_DEF_H

#include "soc.h"

#include "GPIO/Device/TITivaC/GPIO_TITivaC.h"

#include "ADC/Device/TITivaC/ADC_TITivaC_Instance.h"

typedef struct {
    hwGPIO_Pin adc_pin;
    hwADC_Instance inst;
} ADC_Channel_Def;

#endif //ADC_PIN_TI_DEF_H