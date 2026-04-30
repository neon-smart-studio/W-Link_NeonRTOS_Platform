#ifndef ADC_PIN_STM32H7_H
#define ADC_PIN_STM32H7_H

#include "ADC_Pin_STM32.h"

static const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
#if defined (ADC1_BASE)
    /* ===== ADC1 ===== */
    { hwGPIO_Pin_A0, hwADC_Instance_1 },
    { hwGPIO_Pin_A1, hwADC_Instance_1 },
    { hwGPIO_Pin_A2, hwADC_Instance_1 },
    { hwGPIO_Pin_A3, hwADC_Instance_1 },
    { hwGPIO_Pin_A4, hwADC_Instance_1 },
    { hwGPIO_Pin_A5, hwADC_Instance_1 },
    { hwGPIO_Pin_A6, hwADC_Instance_1 },
    { hwGPIO_Pin_A7, hwADC_Instance_1 },
#endif
#if defined (ADC2_BASE)
    /* ===== ADC2 ===== */
    { hwGPIO_Pin_B0, hwADC_Instance_2 },
    { hwGPIO_Pin_B1, hwADC_Instance_2 },
    { hwGPIO_Pin_C0, hwADC_Instance_2 },
    { hwGPIO_Pin_C1, hwADC_Instance_2 },
    { hwGPIO_Pin_C2, hwADC_Instance_2 },
    { hwGPIO_Pin_C3, hwADC_Instance_2 },
#endif
#if defined (ADC3_BASE)
    /* ===== ADC3 ===== */
    { hwGPIO_Pin_F4, hwADC_Instance_3 },
    { hwGPIO_Pin_F5, hwADC_Instance_3 },
#endif
};

#endif // ADC_PIN_STM32H7_H
