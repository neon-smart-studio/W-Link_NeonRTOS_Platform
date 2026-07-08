
#ifndef ADC_PIN_TIMSP432E_H
#define ADC_PIN_TIMSP432E_H

#include "ADC_Pin_TIMSP432E_Def.h"

const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] =
{
    { hwGPIO_Pin_E3, hwADC_Instance_1 },   // AIN0
    { hwGPIO_Pin_E2, hwADC_Instance_1 },   // AIN1
    { hwGPIO_Pin_E1, hwADC_Instance_1 },   // AIN2
    { hwGPIO_Pin_E0, hwADC_Instance_1 },   // AIN3

    { hwGPIO_Pin_D7, hwADC_Instance_1 },   // AIN4
    { hwGPIO_Pin_D6, hwADC_Instance_1 },   // AIN5
    { hwGPIO_Pin_D5, hwADC_Instance_1 },   // AIN6
    { hwGPIO_Pin_D4, hwADC_Instance_1 },   // AIN7

    { hwGPIO_Pin_E5, hwADC_Instance_1 },   // AIN8
    { hwGPIO_Pin_E4, hwADC_Instance_1 },   // AIN9

    { hwGPIO_Pin_B4, hwADC_Instance_1 },   // AIN10
    { hwGPIO_Pin_B5, hwADC_Instance_1 },   // AIN11

    { hwGPIO_Pin_D3, hwADC_Instance_1 },   // AIN12
    { hwGPIO_Pin_D2, hwADC_Instance_1 },   // AIN13
    { hwGPIO_Pin_D1, hwADC_Instance_1 },   // AIN14
    { hwGPIO_Pin_D0, hwADC_Instance_1 },   // AIN15

    { hwGPIO_Pin_K0, hwADC_Instance_1 },   // AIN16
    { hwGPIO_Pin_K1, hwADC_Instance_1 },   // AIN17
    { hwGPIO_Pin_K2, hwADC_Instance_1 },   // AIN18
    { hwGPIO_Pin_K3, hwADC_Instance_1 },   // AIN19
};

#endif //ADC_PIN_TIMSP432E_H