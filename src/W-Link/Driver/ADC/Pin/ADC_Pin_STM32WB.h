#ifndef ADC_PIN_STM32WB_H
#define ADC_PIN_STM32WB_H

#include "ADC_Pin_STM32.h"

static const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
#if defined(ADC1_BASE)
    /* ===== ADC1 ===== */
    { hwGPIO_Pin_A0, hwADC_Instance_1 },   // ADC1_IN0
    { hwGPIO_Pin_A1, hwADC_Instance_1 },   // ADC1_IN1
    { hwGPIO_Pin_A2, hwADC_Instance_1 },   // ADC1_IN2
    { hwGPIO_Pin_A3, hwADC_Instance_1 },   // ADC1_IN3
    { hwGPIO_Pin_A4, hwADC_Instance_1 },   // ADC1_IN4
    { hwGPIO_Pin_A5, hwADC_Instance_1 },   // ADC1_IN5
    { hwGPIO_Pin_A6, hwADC_Instance_1 },   // ADC1_IN6
    { hwGPIO_Pin_A7, hwADC_Instance_1 },   // ADC1_IN7

    { hwGPIO_Pin_B0, hwADC_Instance_1 },   // ADC1_IN8
    { hwGPIO_Pin_B1, hwADC_Instance_1 },   // ADC1_IN9
    { hwGPIO_Pin_B2, hwADC_Instance_1 },   // ADC1_IN10
    { hwGPIO_Pin_B3, hwADC_Instance_1 },   // ADC1_IN11
    { hwGPIO_Pin_B4, hwADC_Instance_1 },   // ADC1_IN12

    { hwGPIO_Pin_C0, hwADC_Instance_1 },   // ADC1_IN13
    { hwGPIO_Pin_C1, hwADC_Instance_1 },   // ADC1_IN14
    { hwGPIO_Pin_C2, hwADC_Instance_1 },   // ADC1_IN15
    { hwGPIO_Pin_C3, hwADC_Instance_1 },   // ADC1_IN16
    { hwGPIO_Pin_C4, hwADC_Instance_1 },   // ADC1_IN17
    { hwGPIO_Pin_C5, hwADC_Instance_1 },   // ADC1_IN18
#endif
};

#endif // ADC_PIN_STM32WB_H