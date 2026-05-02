#ifndef UART_PIN_STM32U0_H
#define UART_PIN_STM32U0_H

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
#ifndef CONFIG_UART_L1_PINSET
#define CONFIG_UART_L1_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART_L2_PINSET
#define CONFIG_UART_L2_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART_L3_PINSET
#define CONFIG_UART_L3_PINSET UART_Pinset_DEFAULT
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
#if defined(LPUART1_BASE)
    CONFIG_UART_L1_PINSET,
#endif
#if defined(LPUART2_BASE)
    CONFIG_UART_L2_PINSET,
#endif
#if defined(LPUART3_BASE)
    CONFIG_UART_L3_PINSET,
#endif
};


static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
#if defined(UART1_BASE) || defined(USART1_BASE)
    {
        { hwGPIO_Pin_A9,  hwGPIO_Pin_A10, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif

#if defined(USART2_BASE) || defined(USART2_BASE)
    {
        { hwGPIO_Pin_A2,  hwGPIO_Pin_A3,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_A15, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_D5,  hwGPIO_Pin_D6,  hwGPIO_Pin_D4,  hwGPIO_Pin_D3  },
    },
#endif

#if defined(UART3_BASE) || defined(USART3_BASE)
    {
        { hwGPIO_Pin_A5,  hwGPIO_Pin_A7,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_B8,  hwGPIO_Pin_B9,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif

#if defined(LPUART1_BASE)
    {
        { hwGPIO_Pin_A2,  hwGPIO_Pin_A3,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_B11, hwGPIO_Pin_B10, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_C1,  hwGPIO_Pin_C0,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif

#if defined(LPUART2_BASE)
    {
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_C6,  hwGPIO_Pin_C7,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_B11, hwGPIO_Pin_B10, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(LPUART3_BASE)
    {
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_C4,  hwGPIO_Pin_C5,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_D8,  hwGPIO_Pin_D9,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
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
    { hwUART_Index_1, hwGPIO_Pin_A0,  GPIO_AF7_USART2 }, /* CTS */
    { hwUART_Index_1, hwGPIO_Pin_A1,  GPIO_AF7_USART2 }, /* RTS/DE */
    { hwUART_Index_1, hwGPIO_Pin_A2,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A3,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A4,  GPIO_AF7_USART2 }, /* CK */
    { hwUART_Index_1, hwGPIO_Pin_A15, GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D3,  GPIO_AF7_USART2 }, /* CTS */
    { hwUART_Index_1, hwGPIO_Pin_D4,  GPIO_AF7_USART2 }, /* RTS/DE */
    { hwUART_Index_1, hwGPIO_Pin_D5,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D6,  GPIO_AF7_USART2 },
#endif

#if defined(UART3_BASE) || defined(USART3_BASE)
    { hwUART_Index_2, hwGPIO_Pin_A5,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_A6,  GPIO_AF7_USART3 }, /* CTS */
    { hwUART_Index_2, hwGPIO_Pin_A7,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B0,  GPIO_AF7_USART3 }, /* CK */
    { hwUART_Index_2, hwGPIO_Pin_B1,  GPIO_AF7_USART3 }, /* RTS/DE */
    { hwUART_Index_2, hwGPIO_Pin_B8,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B9,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B12, GPIO_AF7_USART3 }, /* CK */
    { hwUART_Index_2, hwGPIO_Pin_B13, GPIO_AF7_USART3 }, /* CTS */
    { hwUART_Index_2, hwGPIO_Pin_B14, GPIO_AF7_USART3 }, /* RTS/DE */
    { hwUART_Index_2, hwGPIO_Pin_C4,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C5,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C12, GPIO_AF7_USART3 }, /* CK */
    { hwUART_Index_2, hwGPIO_Pin_D2,  GPIO_AF7_USART3 }, /* RTS/DE */
    { hwUART_Index_2, hwGPIO_Pin_D8,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D9,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D10, GPIO_AF7_USART3 }, /* CK */
    { hwUART_Index_2, hwGPIO_Pin_D11, GPIO_AF7_USART3 }, /* CTS */
    { hwUART_Index_2, hwGPIO_Pin_D12, GPIO_AF7_USART3 }, /* RTS/DE */
#endif

#if defined(LPUART1_BASE)
    { hwUART_Index_L1, hwGPIO_Pin_A2,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_A3,  GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B10, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_B11, GPIO_AF8_LPUART1 },
#endif

#if defined(LPUART2_BASE)
    { hwUART_Index_L2, hwGPIO_Pin_B6,  GPIO_AF10_LPUART2 },
    { hwUART_Index_L2, hwGPIO_Pin_B7,  GPIO_AF10_LPUART2 },
    { hwUART_Index_L2, hwGPIO_Pin_B10, GPIO_AF10_LPUART2 },
    { hwUART_Index_L2, hwGPIO_Pin_B11, GPIO_AF10_LPUART2 },
    { hwUART_Index_L2, hwGPIO_Pin_C0,  GPIO_AF10_LPUART2 },
    { hwUART_Index_L2, hwGPIO_Pin_C1,  GPIO_AF10_LPUART2 },
#endif

#if defined(LPUART3_BASE)
    { hwUART_Index_L3, hwGPIO_Pin_A4,  GPIO_AF8_LPUART3 },
    { hwUART_Index_L3, hwGPIO_Pin_A5,  GPIO_AF8_LPUART3 },
    { hwUART_Index_L3, hwGPIO_Pin_B6,  GPIO_AF6_LPUART3 },
    { hwUART_Index_L3, hwGPIO_Pin_B7,  GPIO_AF6_LPUART3 },
    { hwUART_Index_L3, hwGPIO_Pin_C4,  GPIO_AF8_LPUART3 },
    { hwUART_Index_L3, hwGPIO_Pin_C5,  GPIO_AF8_LPUART3 },
#endif
};

#endif // UART_PIN_STM32U0_H