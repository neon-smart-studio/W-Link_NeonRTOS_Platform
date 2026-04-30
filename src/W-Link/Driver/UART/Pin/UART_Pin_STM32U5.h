#ifndef UART_PIN_STM32U5_H
#define UART_PIN_STM32U5_H

#include "UART_Pin_STM32.h"

typedef enum {
    UART_Pinset_DEFAULT = 0,
    UART_Pinset_ALT1,
    UART_Pinset_ALT2,
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
#ifndef CONFIG_UART5_PINSET
#define CONFIG_UART5_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART_L1_PINSET
#define CONFIG_UART_L1_PINSET UART_Pinset_DEFAULT
#endif

const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] = {
#if defined(USART1_BASE) || defined(UART1_BASE)
    CONFIG_UART0_PINSET,
#endif
#if defined(USART2_BASE) || defined(UART2_BASE)
    CONFIG_UART1_PINSET,
#endif
#if defined(USART3_BASE) || defined(UART3_BASE)
    CONFIG_UART2_PINSET,
#endif
#if defined(USART4_BASE) || defined(UART4_BASE)
    CONFIG_UART3_PINSET,
#endif
#if defined(USART5_BASE) || defined(UART5_BASE)
    CONFIG_UART4_PINSET,
#endif
#if defined(USART6_BASE) || defined(UART6_BASE)
    CONFIG_UART5_PINSET,
#endif
#if defined(LPUART1_BASE)
    CONFIG_UART_L1_PINSET,
#endif
};

const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
#if defined(USART1_BASE) || defined(UART1_BASE)
    {
        { hwGPIO_Pin_A9,  hwGPIO_Pin_A10, hwGPIO_Pin_A12, hwGPIO_Pin_A11 },
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7,  hwGPIO_Pin_B3,  hwGPIO_Pin_B4  },
        { hwGPIO_Pin_G9,  hwGPIO_Pin_G10, hwGPIO_Pin_G12, hwGPIO_Pin_NC  },
    },
#endif

#if defined(USART2_BASE) || defined(UART2_BASE)
    {
        { hwGPIO_Pin_A2,  hwGPIO_Pin_A3,  hwGPIO_Pin_A1,  hwGPIO_Pin_A0  },
        { hwGPIO_Pin_D5,  hwGPIO_Pin_D6,  hwGPIO_Pin_D4,  hwGPIO_Pin_D3  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_A15, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif

#if defined(USART3_BASE) || defined(UART3_BASE)
    {
        { hwGPIO_Pin_A7,  hwGPIO_Pin_A5,  hwGPIO_Pin_A15, hwGPIO_Pin_A6  },
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11, hwGPIO_Pin_B14, hwGPIO_Pin_B13 },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_D12, hwGPIO_Pin_D11 },
    },
#endif

#if defined(USART4_BASE) || defined(UART4_BASE)
    {
        { hwGPIO_Pin_A0,  hwGPIO_Pin_A1,  hwGPIO_Pin_A15, hwGPIO_Pin_B7  },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(USART5_BASE) || defined(UART5_BASE)
    {
        { hwGPIO_Pin_C12, hwGPIO_Pin_D2,  hwGPIO_Pin_B4,  hwGPIO_Pin_B5  },
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(USART6_BASE) || defined(UART6_BASE)
    {
        { hwGPIO_Pin_C6,  hwGPIO_Pin_C7,  hwGPIO_Pin_G8,  hwGPIO_Pin_G9  },
        { hwGPIO_Pin_G14, hwGPIO_Pin_G9,  hwGPIO_Pin_G12, hwGPIO_Pin_G13 },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif

#if defined(LPUART1_BASE)
    {
        { hwGPIO_Pin_A2,  hwGPIO_Pin_A3,  hwGPIO_Pin_B1,  hwGPIO_Pin_A6  },
        { hwGPIO_Pin_B11, hwGPIO_Pin_B10, hwGPIO_Pin_B12, hwGPIO_Pin_B13 },
        { hwGPIO_Pin_G7,  hwGPIO_Pin_G8,  hwGPIO_Pin_G6,  hwGPIO_Pin_G5  },
    },
#endif
};

const UART_AF_Map UART_Pin_AF_Map[] = {
#if defined(USART1_BASE) || defined(UART1_BASE)
    { hwUART_Index_0, hwGPIO_Pin_A9,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A10, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A11, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A12, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B3,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B4,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B6,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B7,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_G9,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_G10, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_G12, GPIO_AF7_USART1 },
#endif

#if defined(USART2_BASE) || defined(UART2_BASE)
    { hwUART_Index_1, hwGPIO_Pin_A0,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A1,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A2,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A3,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A15, GPIO_AF3_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D3,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D4,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D5,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D6,  GPIO_AF7_USART2 },
#endif

#if defined(USART3_BASE) || defined(UART3_BASE)
    { hwUART_Index_2, hwGPIO_Pin_A5,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_A6,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_A7,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_A15, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B1,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B13, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B14, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C4,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C5,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D2,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D8,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D9,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D12, GPIO_AF7_USART3 },
#endif

#if defined(USART4_BASE) || defined(UART4_BASE)
    { hwUART_Index_3, hwGPIO_Pin_A0,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_A1,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_A15, GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_B7,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C10, GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C11, GPIO_AF8_UART4 },
#endif

#if defined(USART5_BASE) || defined(UART5_BASE)
    { hwUART_Index_4, hwGPIO_Pin_B4,  GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_B5,  GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_C12, GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_D2,  GPIO_AF8_UART5 },
#endif

#if defined(USART6_BASE) || defined(UART6_BASE)
#if defined(GPIOC)
    { hwUART_Index_5, hwGPIO_Pin_C6,  GPIO_AF7_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_C7,  GPIO_AF7_USART6 },
#endif

#if defined(GPIOG)
    { hwUART_Index_5, hwGPIO_Pin_G8,  GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G9,  GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G12, GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G13, GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G14, GPIO_AF8_USART6 },
#endif
#endif

#if defined(LPUART1_BASE)
    { hwUART_Index_L1, hwGPIO_Pin_A2,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_A3,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_A6,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B1,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B10, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B11, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B12, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B13, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_C0,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_C1,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_G5,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_G6,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_G7,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_G8,  GPIO_AF8_LPUART1 },
#endif
};

#endif // UART_PIN_STM32U5_H