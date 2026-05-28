#ifndef ADC_PIN_NUC4x2_H
#define ADC_PIN_NUC4x2_H

#include "ADC_Pin_Nuvoton_Def.h"

const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
    { hwGPIO_Pin_B0, hwADC_Instance_1 },
    { hwGPIO_Pin_B1, hwADC_Instance_1 },
    { hwGPIO_Pin_B2, hwADC_Instance_1 },
    { hwGPIO_Pin_B3, hwADC_Instance_1 },
    { hwGPIO_Pin_B4, hwADC_Instance_1 },
    { hwGPIO_Pin_B5, hwADC_Instance_1 },
    { hwGPIO_Pin_B6, hwADC_Instance_1 },
    { hwGPIO_Pin_B7, hwADC_Instance_1 },
};

#endif // ADC_PIN_NUC4x2_H