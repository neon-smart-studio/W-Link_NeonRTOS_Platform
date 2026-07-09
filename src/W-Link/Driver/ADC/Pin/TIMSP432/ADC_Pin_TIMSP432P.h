
#ifndef ADC_PIN_TIMSP432P_H
#define ADC_PIN_TIMSP432P_H

#include "ADC_Pin_TIMSP432_Def.h"

const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
    /* ===== ADC1 ===== */
    { hwGPIO_Pin_B5, hwADC_Instance_1 },    // A0
    { hwGPIO_Pin_B4, hwADC_Instance_1 },    // A1
    { hwGPIO_Pin_B3, hwADC_Instance_1 },    // A2
    { hwGPIO_Pin_B2, hwADC_Instance_1 },    // A3
    { hwGPIO_Pin_B1, hwADC_Instance_1 },    // A4
    { hwGPIO_Pin_B0, hwADC_Instance_1 },    // A5
    { hwGPIO_Pin_A15, hwADC_Instance_1 },   // A6
    { hwGPIO_Pin_A14, hwADC_Instance_1 },   // A7
    { hwGPIO_Pin_A13, hwADC_Instance_1 },   // A8
    { hwGPIO_Pin_A12, hwADC_Instance_1 },   // A9
    { hwGPIO_Pin_A11, hwADC_Instance_1 },   // A10
    { hwGPIO_Pin_A10, hwADC_Instance_1 },   // A11
    { hwGPIO_Pin_A9, hwADC_Instance_1 },    // A12
    { hwGPIO_Pin_A8, hwADC_Instance_1 },    // A13
    { hwGPIO_Pin_C1, hwADC_Instance_1 },    // A14
    { hwGPIO_Pin_C0, hwADC_Instance_1 },    // A15
    { hwGPIO_Pin_E1, hwADC_Instance_1 },    // A16
    { hwGPIO_Pin_E0, hwADC_Instance_1 },    // A17
    { hwGPIO_Pin_D15, hwADC_Instance_1 },   // A18
    { hwGPIO_Pin_D14, hwADC_Instance_1 },   // A19
    { hwGPIO_Pin_D13, hwADC_Instance_1 },   // A20
    { hwGPIO_Pin_D12, hwADC_Instance_1 },   // A21
    { hwGPIO_Pin_D11, hwADC_Instance_1 },   // A22
    { hwGPIO_Pin_D10, hwADC_Instance_1 },   // A23
};

#endif //ADC_PIN_TIMSP432P_H