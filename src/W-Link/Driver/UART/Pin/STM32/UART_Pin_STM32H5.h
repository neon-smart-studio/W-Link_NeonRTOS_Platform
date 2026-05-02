#ifndef UART_PIN_STM32H5_H
#define UART_PIN_STM32H5_H

#include "UART_Pin_STM32.h"

typedef enum {
    UART_Pinset_DEFAULT = 0,
    UART_Pinset_ALT1,
    UART_Pinset_ALT2,
    UART_Pinset_MAX
} UART_Pinset_t;

/* ===== CONFIG ===== */

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
#ifndef CONFIG_UART6_PINSET
#define CONFIG_UART6_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART7_PINSET
#define CONFIG_UART7_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART8_PINSET
#define CONFIG_UART8_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART9_PINSET
#define CONFIG_UART9_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART10_PINSET
#define CONFIG_UART10_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART11_PINSET
#define CONFIG_UART11_PINSET UART_Pinset_DEFAULT
#endif
#ifndef CONFIG_UART_L1_PINSET
#define CONFIG_UART_L1_PINSET UART_Pinset_DEFAULT
#endif

/* ===== Index Map ===== */

static const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] = {
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
#if defined(USART7_BASE) || defined(UART7_BASE)
    CONFIG_UART6_PINSET,
#endif
#if defined(USART8_BASE) || defined(UART8_BASE)
    CONFIG_UART7_PINSET,
#endif
#if defined(USART9_BASE) || defined(UART9_BASE)
    CONFIG_UART8_PINSET,
#endif
#if defined(USART10_BASE) || defined(UART10_BASE)
    CONFIG_UART9_PINSET,
#endif
#if defined(USART11_BASE) || defined(UART11_BASE)
    CONFIG_UART10_PINSET,
#endif
#if defined(USART12_BASE) || defined(UART12_BASE)
    CONFIG_UART11_PINSET,
#endif
#if defined(LPUART1_BASE)
    CONFIG_UART_L1_PINSET,
#endif
};

/*
 * UART_Pin_Def order:
 * { TX, RX, RTS, CTS }
 */
static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
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
#if defined(USART7_BASE) || defined(UART7_BASE)
    {
        { hwGPIO_Pin_F7,  hwGPIO_Pin_F6,  hwGPIO_Pin_F8,  hwGPIO_Pin_F9  }, // TX RX RTS CTS
        { hwGPIO_Pin_E8,  hwGPIO_Pin_E7,  hwGPIO_Pin_E9,  hwGPIO_Pin_E10 },
        { hwGPIO_Pin_B4,  hwGPIO_Pin_B3,  hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
    },
#endif

#if defined(USART8_BASE) || defined(UART8_BASE)
    {
        { hwGPIO_Pin_E1,  hwGPIO_Pin_E0,  hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
        { hwGPIO_Pin_E2,  hwGPIO_Pin_D5,  hwGPIO_Pin_D15, hwGPIO_Pin_D14 },
        { hwGPIO_Pin_H13, hwGPIO_Pin_NC,   hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
    },
#endif

#if defined(USART9_BASE) || defined(UART9_BASE)
    {
        { hwGPIO_Pin_F1,  hwGPIO_Pin_F0,  hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
        { hwGPIO_Pin_D15, hwGPIO_Pin_D14, hwGPIO_Pin_D13, hwGPIO_Pin_D0  },
        { hwGPIO_Pin_G1,  hwGPIO_Pin_G0,  hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
    },
#endif

#if defined(USART10_BASE) || defined(UART10_BASE)
    {
        { hwGPIO_Pin_E3,  hwGPIO_Pin_E2,  hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
        { hwGPIO_Pin_J6,  hwGPIO_Pin_J5,  hwGPIO_Pin_G14,  hwGPIO_Pin_G13 },
        { hwGPIO_Pin_G12, hwGPIO_Pin_G11, hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
    },
#endif

#if defined(USART11_BASE) || defined(UART11_BASE)
    {
        { hwGPIO_Pin_F3,  hwGPIO_Pin_F4,  hwGPIO_Pin_NC,   hwGPIO_Pin_F5  },
        { hwGPIO_Pin_A6,  hwGPIO_Pin_A7,  hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
        { hwGPIO_Pin_I11, hwGPIO_Pin_I10, hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
    },
#endif

#if defined(USART12_BASE) || defined(UART12_BASE)
    {
        { hwGPIO_Pin_E10, hwGPIO_Pin_E9,  hwGPIO_Pin_E7,  hwGPIO_Pin_E8  },
        { hwGPIO_Pin_G3,  hwGPIO_Pin_G2,  hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
        { hwGPIO_Pin_F2,  hwGPIO_Pin_F5,  hwGPIO_Pin_NC,   hwGPIO_Pin_NC   },
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

static const UART_AF_Map UART_Pin_AF_Map[] = {
#if (defined(USART1_BASE) || defined(UART1_BASE))
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

#if (defined(USART2_BASE) || defined(UART2_BASE))
    { hwUART_Index_1, hwGPIO_Pin_A0,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A1,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A2,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A3,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D3,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D4,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D5,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D6,  GPIO_AF7_USART2 },
#endif

#if (defined(USART3_BASE) || defined(UART3_BASE))
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

#if (defined(USART4_BASE) || defined(UART4_BASE))
    { hwUART_Index_3, hwGPIO_Pin_A0,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_A1,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_A15, GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_B7,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C10, GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C11, GPIO_AF8_UART4 },
#endif

#if (defined(USART5_BASE) || defined(UART5_BASE))
    { hwUART_Index_4, hwGPIO_Pin_B4,  GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_B5,  GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_C12, GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_D2,  GPIO_AF8_UART5 },
#endif

#if defined(USART6_BASE) || defined(UART6_BASE)
    { hwUART_Index_5, hwGPIO_Pin_C6,  GPIO_AF7_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_C7,  GPIO_AF7_USART6 },
#endif

#if defined(UART7_BASE) || defined(USART7_BASE)
    { hwUART_Index_6, hwGPIO_Pin_F6,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_F7,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_F8,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_F9,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_E7,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_E8,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_E9,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_E10, GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_B3,  GPIO_AF11_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_B4,  GPIO_AF11_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_G3,  GPIO_AF11_UART7 },
#endif

#if defined(UART8_BASE) || defined(USART8_BASE)
    { hwUART_Index_7, hwGPIO_Pin_E0,  GPIO_AF8_UART8 },
    { hwUART_Index_7, hwGPIO_Pin_E1,  GPIO_AF8_UART8 },
    { hwUART_Index_7, hwGPIO_Pin_E2,  GPIO_AF8_UART8 },
    { hwUART_Index_7, hwGPIO_Pin_D5,  GPIO_AF8_UART8 },
    { hwUART_Index_7, hwGPIO_Pin_D14, GPIO_AF8_UART8 },
    { hwUART_Index_7, hwGPIO_Pin_D15, GPIO_AF8_UART8 },
    { hwUART_Index_7, hwGPIO_Pin_H13, GPIO_AF8_UART8 },
#endif

#if defined(UART9_BASE) || defined(USART9_BASE)
    { hwUART_Index_8, hwGPIO_Pin_F0,  GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_F1,  GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_D0,  GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_D13, GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_D14, GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_D15, GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_G0,  GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_G1,  GPIO_AF11_UART9 },
#endif

#if defined(USART10_BASE) || defined(UART10_BASE)
    { hwUART_Index_9, hwGPIO_Pin_E2,  GPIO_AF7_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_E3,  GPIO_AF7_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_E15, GPIO_AF7_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_G11, GPIO_AF6_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_G12, GPIO_AF6_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_G13, GPIO_AF6_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_G14, GPIO_AF6_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_G15, GPIO_AF6_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_J5,  GPIO_AF6_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_J6,  GPIO_AF6_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_J7,  GPIO_AF6_USART10 },
#endif

#if defined(USART11_BASE) || defined(UART11_BASE)
    { hwUART_Index_10, hwGPIO_Pin_A6,  GPIO_AF7_USART11 },
    { hwUART_Index_10, hwGPIO_Pin_A7,  GPIO_AF7_USART11 },
    { hwUART_Index_10, hwGPIO_Pin_F2,  GPIO_AF7_USART11 },
    { hwUART_Index_10, hwGPIO_Pin_I10, GPIO_AF7_USART11 },
    { hwUART_Index_10, hwGPIO_Pin_I11, GPIO_AF7_USART11 },
#endif

#if defined(UART12_BASE) || defined(USART12_BASE)
    { hwUART_Index_11, hwGPIO_Pin_E7,  GPIO_AF6_UART12 },
    { hwUART_Index_11, hwGPIO_Pin_E8,  GPIO_AF6_UART12 },
    { hwUART_Index_11, hwGPIO_Pin_E9,  GPIO_AF6_UART12 },
    { hwUART_Index_11, hwGPIO_Pin_E10, GPIO_AF6_UART12 },
    { hwUART_Index_11, hwGPIO_Pin_F2,  GPIO_AF6_UART12 },
    { hwUART_Index_11, hwGPIO_Pin_F5,  GPIO_AF6_UART12 },
    { hwUART_Index_11, hwGPIO_Pin_G2,  GPIO_AF6_UART12 },
    { hwUART_Index_11, hwGPIO_Pin_G3,  GPIO_AF6_UART12 },
#endif

#if defined(LPUART1_BASE) && defined(GPIO_AF8_LPUART1)
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

#endif // UART_PIN_STM32H5_H