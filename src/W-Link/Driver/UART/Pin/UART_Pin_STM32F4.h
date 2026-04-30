
#ifndef UART_PIN_STM32F4_H
#define UART_PIN_STM32F4_H

#include "UART_Pin_STM32.h"

typedef enum {
    UART_Pinset_DEFAULT = 0,
    UART_Pinset_ALT1,
    UART_Pinset_ALT2,
    UART_Pinset_MAX
} UART_Pinset_t;

#ifndef CONFIG_UART0_PINSET
#define CONFIG_UART0_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART0_PINSET

#ifndef CONFIG_UART1_PINSET
#define CONFIG_UART1_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART1_PINSET

#ifndef CONFIG_UART2_PINSET
#define CONFIG_UART2_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART2_PINSET

#ifndef CONFIG_UART3_PINSET
#define CONFIG_UART3_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART3_PINSET

#ifndef CONFIG_UART4_PINSET
#define CONFIG_UART4_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART4_PINSET

#ifndef CONFIG_UART5_PINSET
#define CONFIG_UART5_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART5_PINSET

static const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] = {
#if defined(UART1_BASE) || defined(USART1_BASE)
    CONFIG_UART0_PINSET,
#endif
#if defined(UART2_BASE) || defined(USART2_BASE)
    CONFIG_UART1_PINSET,
#endif
#if defined(UART3_BASE) || defined(USART3_BASE)
    CONFIG_UART2_PINSET,
#endif
#if defined(UART4_BASE) || defined(USART4_BASE)
    CONFIG_UART3_PINSET,
#endif
#if defined(UART5_BASE) || defined(USART5_BASE)
    CONFIG_UART4_PINSET,
#endif
#if defined(UART6_BASE) || defined(USART6_BASE)
    CONFIG_UART5_PINSET,
#endif
};

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
#if defined(UART1_BASE) || defined(USART1_BASE)
    /* ================= USART1 (UART0) ================= */
    {
        { hwGPIO_Pin_A9,  hwGPIO_Pin_A10, hwGPIO_Pin_A12, hwGPIO_Pin_A11 },   // DEFAULT
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7,  hwGPIO_Pin_A12, hwGPIO_Pin_A11 },   // ALT1
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },   // ALT2
    },
#endif
#if defined(UART2_BASE) || defined(USART2_BASE)
    /* ================= USART2 (UART1) ================= */
    {
        { hwGPIO_Pin_A2,  hwGPIO_Pin_A3,  hwGPIO_Pin_A1,  hwGPIO_Pin_A0  },
        { hwGPIO_Pin_D5,  hwGPIO_Pin_D6,  hwGPIO_Pin_D4,  hwGPIO_Pin_D3  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif
#if defined(UART3_BASE) || defined(USART3_BASE)
    /* ================= USART3 (UART2) ================= */
    {
        { hwGPIO_Pin_D8,  hwGPIO_Pin_D9,  hwGPIO_Pin_B14, hwGPIO_Pin_B13 },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_B14, hwGPIO_Pin_B13 },
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11, hwGPIO_Pin_D12, hwGPIO_Pin_D11 },
    },
#endif
#if defined(UART4_BASE) || defined(USART4_BASE)
    /* ================= UART4 (UART3) ================= */
    {
        { hwGPIO_Pin_A0,  hwGPIO_Pin_A1,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif
#if defined(UART5_BASE) || defined(USART5_BASE)
    /* ================= UART5 (UART4) ================= */
    {
        { hwGPIO_Pin_C12, hwGPIO_Pin_D2,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif
#if defined(UART6_BASE) || defined(USART6_BASE)
    /* ================= USART6 (UART5) ================= */
    {
        { hwGPIO_Pin_C6,  hwGPIO_Pin_C7,  hwGPIO_Pin_G8,  hwGPIO_Pin_G13 },
        { hwGPIO_Pin_G14, hwGPIO_Pin_G9,  hwGPIO_Pin_G12, hwGPIO_Pin_G15 },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif
};

static const UART_AF_Map UART_Pin_AF_Map[] = {
#if defined(UART1_BASE) || defined(USART1_BASE)
    { hwUART_Index_0, hwGPIO_Pin_A9,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B6,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A10, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B7,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A11, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A12, GPIO_AF7_USART1 },
#endif
#if defined(UART2_BASE) || defined(USART2_BASE)
    { hwUART_Index_1, hwGPIO_Pin_A2,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D5,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A3,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D6,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A0,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D3,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A1,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D4,  GPIO_AF7_USART2 },
#endif
#if defined(UART3_BASE) || defined(USART3_BASE)
    { hwUART_Index_2, hwGPIO_Pin_B10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D8,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D9,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B13, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B14, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D12, GPIO_AF7_USART3 },
#endif
#if defined(UART4_BASE) || defined(USART4_BASE)
    { hwUART_Index_3, hwGPIO_Pin_A0,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C10, GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_A1,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C11, GPIO_AF8_UART4 },
#endif
#if defined(UART5_BASE) || defined(USART5_BASE)
    { hwUART_Index_4, hwGPIO_Pin_C12, GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_D2,  GPIO_AF8_UART5 },
#endif
#if defined(UART6_BASE) || defined(USART6_BASE)
    { hwUART_Index_5, hwGPIO_Pin_C6,  GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G14, GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_C7,  GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G9,  GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G12, GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G8,  GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G13, GPIO_AF8_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G15, GPIO_AF8_USART6 },
#endif
};

#endif //UART_PIN_STM32F4_H