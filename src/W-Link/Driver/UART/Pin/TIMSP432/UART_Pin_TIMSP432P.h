#ifndef UART_PIN_TIMSP432P_H
#define UART_PIN_TIMSP432P_H

#include "UART_Pin_TIMSP432_Def.h"

const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    /* TX             RX             CTS            RTS */

    /* EUSCI_A0 */
    { hwGPIO_Pin_A1,  hwGPIO_Pin_A0, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* EUSCI_A1 */
    { hwGPIO_Pin_A3,  hwGPIO_Pin_A2, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* EUSCI_A2 */
    { hwGPIO_Pin_D3,  hwGPIO_Pin_D2, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* EUSCI_A3 */
    { hwGPIO_Pin_D9,  hwGPIO_Pin_D8, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};

#endif // UART_PIN_TIMSP432P_H