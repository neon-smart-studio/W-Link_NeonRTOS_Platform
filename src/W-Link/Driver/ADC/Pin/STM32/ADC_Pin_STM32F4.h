#ifndef ADC_PIN_STM32F4_H
#define ADC_PIN_STM32F4_H

#include "ADC_Pin_STM32_Def.h"

static const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
#if defined(ADC1_BASE)
    /* ===== ADC1 ===== */
    { hwGPIO_Pin_A0, hwADC_Instance_1 },   // ADC123_IN0
    { hwGPIO_Pin_A1, hwADC_Instance_1 },   // ADC123_IN1
    { hwGPIO_Pin_A2, hwADC_Instance_1 },   // ADC123_IN2
    { hwGPIO_Pin_A3, hwADC_Instance_1 },   // ADC123_IN3
    { hwGPIO_Pin_A4, hwADC_Instance_1 },   // ADC12_IN4
    { hwGPIO_Pin_A5, hwADC_Instance_1 },   // ADC12_IN5
#endif

#if defined(ADC2_BASE)
    /* ===== ADC2 ===== */
    { hwGPIO_Pin_A6, hwADC_Instance_2 },   // ADC12_IN6
    { hwGPIO_Pin_A7, hwADC_Instance_2 },   // ADC12_IN7
    { hwGPIO_Pin_B0, hwADC_Instance_2 },   // ADC12_IN8
    { hwGPIO_Pin_B1, hwADC_Instance_2 },   // ADC12_IN9
    { hwGPIO_Pin_C0, hwADC_Instance_2 },   // ADC123_IN10
    { hwGPIO_Pin_C1, hwADC_Instance_2 },   // ADC123_IN11
#endif

#if defined(ADC3_BASE)
    /* ===== ADC3 ===== */
    { hwGPIO_Pin_C2, hwADC_Instance_3 },   // ADC123_IN12
    { hwGPIO_Pin_C3, hwADC_Instance_3 },   // ADC123_IN13
    { hwGPIO_Pin_C4, hwADC_Instance_3 },   // ADC12_IN14
    { hwGPIO_Pin_C5, hwADC_Instance_3 },   // ADC12_IN15
#endif
};

#endif // ADC_PIN_STM32F4_H