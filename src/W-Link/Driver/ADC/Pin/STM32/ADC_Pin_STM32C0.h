#ifndef ADC_PIN_STM32C0_H
#define ADC_PIN_STM32C0_H

#include "ADC_Pin_STM32_Def.h"

const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
#if defined(ADC1_BASE)
    /* ===== ADC1 ===== */
    { hwGPIO_Pin_A0, hwADC_Instance_1 },   // ADC_IN0
    { hwGPIO_Pin_A1, hwADC_Instance_1 },   // ADC_IN1
    { hwGPIO_Pin_A2, hwADC_Instance_1 },   // ADC_IN2
    { hwGPIO_Pin_A3, hwADC_Instance_1 },   // ADC_IN3
    { hwGPIO_Pin_A4, hwADC_Instance_1 },   // ADC_IN4
    { hwGPIO_Pin_A5, hwADC_Instance_1 },   // ADC_IN5
    { hwGPIO_Pin_A6, hwADC_Instance_1 },   // ADC_IN6
    { hwGPIO_Pin_A7, hwADC_Instance_1 },   // ADC_IN7

    { hwGPIO_Pin_B0, hwADC_Instance_1 },   // ADC_IN8
    { hwGPIO_Pin_B1, hwADC_Instance_1 },   // ADC_IN9
#endif
};

#endif // ADC_PIN_STM32C0_H