#ifndef UART_PIN_STM32L5_H
#define UART_PIN_STM32L5_H

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
#ifndef CONFIG_UART2_PINSET
#define CONFIG_UART2_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART3_PINSET
#define CONFIG_UART3_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART4_PINSET
#define CONFIG_UART4_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART_L1_PINSET
#define CONFIG_UART_L1_PINSET UART_Pinset_DEFAULT
#endif

static const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] = {
#if defined(UART1_BASE) || defined(USART1_BASE)
    CONFIG_UART0_PINSET,
#endif
#if defined(USART2_BASE) || defined(USART2_BASE)
    CONFIG_UART1_PINSET,
#endif
#if defined(UART3_BASE) || defined(USART3_BASE)
    CONFIG_UART2_PINSET,
#endif
#if defined(USART4_BASE) || defined(UART4_BASE)
    CONFIG_UART3_PINSET,
#endif
#if defined(USART5_BASE) || defined(UART5_BASE)
    CONFIG_UART4_PINSET,
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

#if defined(USART2_BASE) || defined(USART2_BASE)
    {
        { hwGPIO_Pin_A2, hwGPIO_Pin_A3, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_D5, hwGPIO_Pin_D6, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART3_BASE) || defined(USART3_BASE)
    {
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(USART4_BASE) || defined(UART4_BASE)
    {
        { hwGPIO_Pin_A0,  hwGPIO_Pin_A1,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(USART5_BASE) || defined(UART5_BASE)
    {
        { hwGPIO_Pin_C12, hwGPIO_Pin_D2, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_B3,  hwGPIO_Pin_B4, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(LPUART1_BASE)
    {
        { hwGPIO_Pin_G7, hwGPIO_Pin_G8, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif
};

static const UART_AF_Map UART_Pin_AF_Map[] = {
#if defined(UART1_BASE) || defined(USART1_BASE)
    { hwUART_Index_0, hwGPIO_Pin_A9,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A10, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B6,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B7,  GPIO_AF7_USART1 },
#endif

#if defined(USART2_BASE) || defined(USART2_BASE)
    { hwUART_Index_1, hwGPIO_Pin_A2, GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A3, GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D5, GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D6, GPIO_AF7_USART2 },
#endif

#if defined(UART3_BASE) || defined(USART3_BASE)
    { hwUART_Index_2, hwGPIO_Pin_B10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C11, GPIO_AF7_USART3 },
#endif

#if defined(USART4_BASE) || defined(UART4_BASE)
    { hwUART_Index_3, hwGPIO_Pin_A0,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_A1,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C10, GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C11, GPIO_AF8_UART4 },
#endif

#if defined(USART5_BASE) || defined(UART5_BASE)
    { hwUART_Index_4, hwGPIO_Pin_C12, GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_D2,  GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_B3,  GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_B4,  GPIO_AF8_UART5 },
#endif

#if defined(LPUART1_BASE)
    { hwUART_Index_L1, hwGPIO_Pin_G7,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_G8,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B10, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B11, GPIO_AF8_LPUART1 },
#endif
};

#endif // UART_PIN_STM32L5_H