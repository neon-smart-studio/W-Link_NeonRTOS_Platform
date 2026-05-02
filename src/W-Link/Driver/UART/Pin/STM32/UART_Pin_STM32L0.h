#ifndef UART_PIN_STM32L0_H
#define UART_PIN_STM32L0_H

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
#ifndef CONFIG_UART_L1_PINSET
#define CONFIG_UART_L1_PINSET UART_Pinset_DEFAULT
#endif

static const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] = {
#if defined(UART1_BASE) || defined(USART1_BASE)
    CONFIG_UART0_PINSET,
#endif
#if defined(UART2_BASE) || defined(USART2_BASE)
    CONFIG_UART1_PINSET,
#endif
#if defined(LPUART1_BASE)
    CONFIG_UART_L1_PINSET,
#endif
};

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
#if defined(UART1_BASE) || defined(USART1_BASE)
    {
        { hwGPIO_Pin_A9, hwGPIO_Pin_A10, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_B6, hwGPIO_Pin_B7,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART2_BASE) || defined(USART2_BASE)
    {
        { hwGPIO_Pin_A2,  hwGPIO_Pin_A3,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_A14, hwGPIO_Pin_A15, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(LPUART1_BASE)
    {
        { hwGPIO_Pin_A2, hwGPIO_Pin_A3, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif
};

static const UART_AF_Map UART_Pin_AF_Map[] = {
#if defined(UART1_BASE) || defined(USART1_BASE)
    { hwUART_Index_0, hwGPIO_Pin_A9,  GPIO_AF4_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A10, GPIO_AF4_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B6,  GPIO_AF4_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B7,  GPIO_AF4_USART1 },
#endif

#if defined(UART2_BASE) || defined(USART2_BASE)
    { hwUART_Index_1, hwGPIO_Pin_A2,  GPIO_AF4_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A3,  GPIO_AF4_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A14, GPIO_AF4_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A15, GPIO_AF4_USART2 },
#endif

#if defined(LPUART1_BASE)
    { hwUART_Index_L1, hwGPIO_Pin_A2,  GPIO_AF6_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_A3,  GPIO_AF6_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B10, GPIO_AF6_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B11, GPIO_AF6_LPUART1 },
#endif
};

#endif // UART_PIN_STM32L0_H