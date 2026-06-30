
#ifndef UART_PIN_TM4C123_H
#define UART_PIN_TM4C123_H

#include "UART_Pin_TITivaC_Def.h"

#if defined(TM4C123)

const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    { hwGPIO_Pin_A1, hwGPIO_Pin_A0, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    { hwGPIO_Pin_B1, hwGPIO_Pin_B0, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    { hwGPIO_Pin_D7, hwGPIO_Pin_D6, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    { hwGPIO_Pin_C7, hwGPIO_Pin_C6, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    { hwGPIO_Pin_C5, hwGPIO_Pin_C4, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    { hwGPIO_Pin_E5, hwGPIO_Pin_E4, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    { hwGPIO_Pin_D5, hwGPIO_Pin_D4, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    { hwGPIO_Pin_E1, hwGPIO_Pin_E0, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};

#endif //TM4C123

#endif //UART_PIN_TM4C123_H