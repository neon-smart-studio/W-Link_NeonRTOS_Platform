#ifndef UART_PIN_TIMSP432E_H
#define UART_PIN_TIMSP432E_H

#include "UART_Pin_TIMSP432_Def.h"

const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    { hwGPIO_Pin_A1, hwGPIO_Pin_A0, hwGPIO_Pin_H0, hwGPIO_Pin_H1 },
    { hwGPIO_Pin_B1, hwGPIO_Pin_B0, hwGPIO_Pin_N0, hwGPIO_Pin_N1 },
    { hwGPIO_Pin_D5, hwGPIO_Pin_D4, hwGPIO_Pin_N2, hwGPIO_Pin_N3 },
    { hwGPIO_Pin_J1, hwGPIO_Pin_J0, hwGPIO_Pin_N4, hwGPIO_Pin_N5 },
    { hwGPIO_Pin_K1, hwGPIO_Pin_K0, hwGPIO_Pin_K2, hwGPIO_Pin_K3 },
    { hwGPIO_Pin_C7, hwGPIO_Pin_C6, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    { hwGPIO_Pin_P1, hwGPIO_Pin_P0, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    { hwGPIO_Pin_C5, hwGPIO_Pin_C4, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};

#endif //UART_PIN_TIMSP432E_H