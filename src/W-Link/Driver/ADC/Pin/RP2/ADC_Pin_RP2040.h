#ifndef ADC_PIN_RP2040_H
#define ADC_PIN_RP2040_H

#include "ADC_Pin_RP2_Def.h"

const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
    { hwGPIO_Pin_26, hwADC_Instance_1 },
    { hwGPIO_Pin_27, hwADC_Instance_1 },
    { hwGPIO_Pin_28, hwADC_Instance_1 },
    { hwGPIO_Pin_29, hwADC_Instance_1 },
};

#endif // ADC_PIN_RP2040_H