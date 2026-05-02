#ifndef ADC_PIN_STM32G4_H
#define ADC_PIN_STM32G4_H

#include "ADC_Pin_STM32_Def.h"

static const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
#if defined(ADC1_BASE)
    /* ===== ADC1 ===== */
    { hwGPIO_Pin_A0, hwADC_Instance_1 },   // ADC1_IN1
    { hwGPIO_Pin_A1, hwADC_Instance_1 },   // ADC1_IN2
    { hwGPIO_Pin_A2, hwADC_Instance_1 },   // ADC1_IN3
    { hwGPIO_Pin_A3, hwADC_Instance_1 },   // ADC1_IN4
    { hwGPIO_Pin_A4, hwADC_Instance_1 },   // ADC1_IN17
    { hwGPIO_Pin_A5, hwADC_Instance_1 },   // ADC1_IN13
    { hwGPIO_Pin_A6, hwADC_Instance_1 },   // ADC1_IN3 / ADC2_IN3，依型號可能不同
    { hwGPIO_Pin_A7, hwADC_Instance_1 },   // ADC1_IN4 / ADC2_IN4，依型號可能不同
    { hwGPIO_Pin_B0, hwADC_Instance_1 },   // ADC1_IN15
    { hwGPIO_Pin_B1, hwADC_Instance_1 },   // ADC1_IN12
#endif
};

#endif // ADC_PIN_STM32G4_H