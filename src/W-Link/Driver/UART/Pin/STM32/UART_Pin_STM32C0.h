#ifndef UART_PIN_STM32C0_H
#define UART_PIN_STM32C0_H

#include "UART_Pin_STM32.h"

typedef enum {
    UART_Pinset_DEFAULT = 0,
    UART_Pinset_ALT1,
    UART_Pinset_MAX
} UART_Pinset_t;

#ifndef CONFIG_UART0_PINSET
#define CONFIG_UART0_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART1_PINSET
#define CONFIG_UART1_PINSET UART_Pinset_DEFAULT
#endif

const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] = {
#if defined(UART1_BASE) || defined(USART1_BASE)
    CONFIG_UART0_PINSET,
#endif
#if defined(UART2_BASE) || defined(USART2_BASE)
    CONFIG_UART1_PINSET,
#endif
};

const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
#if defined(UART1_BASE) || defined(USART1_BASE)
    {
        /* DEFAULT: TX, RX, RTS, CTS */
        { hwGPIO_Pin_A9, hwGPIO_Pin_A10, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

        /* ALT1 */
        { hwGPIO_Pin_B6, hwGPIO_Pin_B7, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART2_BASE) || defined(USART2_BASE)
    {
        /* DEFAULT */
        { hwGPIO_Pin_A2, hwGPIO_Pin_A3, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

        /* ALT1 */
        { hwGPIO_Pin_A14, hwGPIO_Pin_A15, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif
};

const UART_AF_Map UART_Pin_AF_Map[] = {
#if defined(UART1_BASE) || defined(USART1_BASE)
    { hwUART_Index_0, hwGPIO_Pin_A9,  GPIO_AF1_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A10, GPIO_AF1_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B6,  GPIO_AF0_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B7,  GPIO_AF0_USART1 },
#endif

#if defined(UART2_BASE) || defined(USART2_BASE)
    { hwUART_Index_1, hwGPIO_Pin_A2,  GPIO_AF1_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A3,  GPIO_AF1_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A14, GPIO_AF1_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A15, GPIO_AF1_USART2 },
#endif
};

#endif // UART_PIN_STM32C0_H