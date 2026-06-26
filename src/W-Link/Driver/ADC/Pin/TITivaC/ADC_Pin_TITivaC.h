
#ifndef ADC_PIN_STM32H723xx_H
#define ADC_PIN_STM32H723xx_H

#include "ADC_Pin_TITivaC_Def.h"

const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
    /* ===== ADC1 ===== */
    { hwGPIO_Pin_E3, hwADC_Instance_1 },   // ADC_IN0
    { hwGPIO_Pin_E2, hwADC_Instance_1 },   // ADC_IN1
    { hwGPIO_Pin_E1, hwADC_Instance_1 },   // ADC_IN2
    { hwGPIO_Pin_E0, hwADC_Instance_1 },   // ADC_IN3
    { hwGPIO_Pin_D7, hwADC_Instance_2 },   // ADC_IN4
    { hwGPIO_Pin_D6, hwADC_Instance_2 },   // ADC_IN5
    { hwGPIO_Pin_D5, hwADC_Instance_2 },   // ADC_IN6
    { hwGPIO_Pin_D4, hwADC_Instance_2 },   // ADC_IN7
};

#endif //ADC_PIN_STM32H723xx_H