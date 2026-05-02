#ifndef UART_PIN_STM32WB_H
#define UART_PIN_STM32WB_H

#include "UART_Pin_STM32.h"

typedef enum {
    UART_Pinset_DEFAULT = 0,
    UART_Pinset_ALT1,
    UART_Pinset_MAX
} UART_Pinset_t;

#ifndef CONFIG_UART0_PINSET
#define CONFIG_UART0_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART_L1_PINSET
#define CONFIG_UART_L1_PINSET UART_Pinset_DEFAULT
#endif

const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] = {
#if defined(USART1_BASE) || defined(UART1_BASE)
    CONFIG_UART0_PINSET,
#endif
#if defined(LPUART1_BASE)
    CONFIG_UART_L1_PINSET,
#endif
};

const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
#if defined(USART1_BASE) || defined(UART1_BASE)
    {
        /* DEFAULT: TX, RX, RTS, CTS */
        { hwGPIO_Pin_A9, hwGPIO_Pin_A10, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

        /* ALT1 */
        { hwGPIO_Pin_B6, hwGPIO_Pin_B7, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(LPUART1_BASE)
    {
        /* DEFAULT: TX, RX, RTS, CTS */
        { hwGPIO_Pin_B7, hwGPIO_Pin_B6, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

        /* ALT1 */
        { hwGPIO_Pin_C1, hwGPIO_Pin_C0, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif
};

const UART_AF_Map UART_Pin_AF_Map[] = {
#if defined(USART1_BASE) || defined(UART1_BASE)
    { hwUART_Index_0, hwGPIO_Pin_A9,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A10, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B6,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B7,  GPIO_AF7_USART1 },
#endif

#if defined(LPUART1_BASE)
    { hwUART_Index_L1, hwGPIO_Pin_B6, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B7, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_C0, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_C1, GPIO_AF8_LPUART1 },
#endif
};

#endif // UART_PIN_STM32WB_H