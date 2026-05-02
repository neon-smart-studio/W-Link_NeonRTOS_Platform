
#ifndef UART_PIN_STM32H7_H
#define UART_PIN_STM32H7_H

#include "UART_Pin_STM32.h"

typedef enum {
    UART_Pinset_DEFAULT = 0,
    UART_Pinset_ALT1,
    UART_Pinset_ALT2,
    UART_Pinset_ALT3,
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

#ifndef CONFIG_UART6_PINSET
#define CONFIG_UART6_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART6_PINSET

#ifndef CONFIG_UART7_PINSET
#define CONFIG_UART7_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART7_PINSET

#ifndef CONFIG_UART8_PINSET
#define CONFIG_UART8_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART8_PINSET

#ifndef CONFIG_UART9_PINSET
#define CONFIG_UART9_PINSET UART_Pinset_DEFAULT
#endif //CONFIG_UART9_PINSET

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
#if defined(UART7_BASE) || defined(USART7_BASE)
    CONFIG_UART6_PINSET,
#endif
#if defined(UART8_BASE) || defined(USART8_BASE)
    CONFIG_UART7_PINSET,
#endif
#if defined(UART9_BASE) || defined(USART9_BASE)
    CONFIG_UART8_PINSET,
#endif
#if defined(UART10_BASE) || defined(USART10_BASE)
    CONFIG_UART9_PINSET,
#endif
#if defined(LPUART1_BASE)
    CONFIG_UART_L1_PINSET,
#endif
};

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
#if defined(UART1_BASE) || defined(USART1_BASE)
    /* ================= UART0 / USART1 ================= */
    {
        /* DEFAULT */
        { hwGPIO_Pin_A9,  hwGPIO_Pin_A10, hwGPIO_Pin_A12, hwGPIO_Pin_A11 },

        /* ALT1 */
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7,  hwGPIO_Pin_A12, hwGPIO_Pin_A11 },

        /* ALT2 */
        { hwGPIO_Pin_B14, hwGPIO_Pin_B15, hwGPIO_Pin_A12, hwGPIO_Pin_A11 },

        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
    },
#endif
#if defined(UART2_BASE) || defined(USART2_BASE)
    /* ================= UART1 / USART2 ================= */
    {
        { hwGPIO_Pin_A2, hwGPIO_Pin_A3, hwGPIO_Pin_A1, hwGPIO_Pin_A0 },
        { hwGPIO_Pin_D5, hwGPIO_Pin_D6, hwGPIO_Pin_D4, hwGPIO_Pin_D3 },
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif
#if defined(UART3_BASE) || defined(USART3_BASE)
    /* ================= UART2 / USART3 ================= */
    {
        { hwGPIO_Pin_D8,  hwGPIO_Pin_D9,  hwGPIO_Pin_B14, hwGPIO_Pin_B13 },
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11, hwGPIO_Pin_B14, hwGPIO_Pin_B13 },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_B14, hwGPIO_Pin_B13 },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
    },
#endif
#if defined(UART4_BASE) || defined(USART4_BASE)
    /* ================= UART3 / UART4 ================= */
    {
        { hwGPIO_Pin_B9,  hwGPIO_Pin_B8,  hwGPIO_Pin_A15, hwGPIO_Pin_B0 },
        { hwGPIO_Pin_A0,  hwGPIO_Pin_A1,  hwGPIO_Pin_A15, hwGPIO_Pin_B0 },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_A15, hwGPIO_Pin_B0 },
        { hwGPIO_Pin_D1,  hwGPIO_Pin_D0,  hwGPIO_Pin_A15, hwGPIO_Pin_B0 },
    },
#endif
#if defined(UART5_BASE) || defined(USART5_BASE)
    /* ================= UART4 / UART5 ================= */
    {
        { hwGPIO_Pin_B13, hwGPIO_Pin_B12, hwGPIO_Pin_C8,  hwGPIO_Pin_C9 },
        { hwGPIO_Pin_C12, hwGPIO_Pin_D2,  hwGPIO_Pin_C8,  hwGPIO_Pin_C9 },
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B5,  hwGPIO_Pin_C8,  hwGPIO_Pin_C9 },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
    },
#endif
#if defined(UART6_BASE) || defined(USART6_BASE)
    /* ================= UART5 / USART6 ================= */
    {
        { hwGPIO_Pin_C6,  hwGPIO_Pin_C7, hwGPIO_Pin_G8,  hwGPIO_Pin_G13 },
        { hwGPIO_Pin_G14, hwGPIO_Pin_G9, hwGPIO_Pin_G12, hwGPIO_Pin_G15 },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
    },
#endif
#if defined(UART7_BASE) || defined(USART7_BASE)
    /* ================= UART6 / UART7 ================= */
    {
        { hwGPIO_Pin_E8,  hwGPIO_Pin_E7, hwGPIO_Pin_E9,  hwGPIO_Pin_E10 },
        { hwGPIO_Pin_F7,  hwGPIO_Pin_F6, hwGPIO_Pin_F8,  hwGPIO_Pin_F9  },
        { hwGPIO_Pin_A15, hwGPIO_Pin_A8, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_B4,  hwGPIO_Pin_B3, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif
#if defined(UART8_BASE) || defined(USART8_BASE)
    /* ================= UART7 / UART8 ================= */
    {
        { hwGPIO_Pin_E1, hwGPIO_Pin_E0, hwGPIO_Pin_D15, hwGPIO_Pin_D14 },
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC,  hwGPIO_Pin_NC  },
    },
#endif
#if defined(UART9_BASE) || defined(USART9_BASE)
    /* ================= UART8 / UART9 ================= */
    {
        { hwGPIO_Pin_G1,  hwGPIO_Pin_G0,  hwGPIO_Pin_D13, hwGPIO_Pin_D0 },
        { hwGPIO_Pin_D15, hwGPIO_Pin_D14, hwGPIO_Pin_D13, hwGPIO_Pin_D0 },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
    },
#endif
#if defined(UART10_BASE) || defined(USART10_BASE)
    /* ================= UART9 / USART10 ================= */
    {
        { hwGPIO_Pin_E3,  hwGPIO_Pin_E2,  hwGPIO_Pin_G14, hwGPIO_Pin_G13 },
        { hwGPIO_Pin_G12, hwGPIO_Pin_G11, hwGPIO_Pin_G14, hwGPIO_Pin_G13 },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC },
    },
#endif
#if defined(LPUART1_BASE)
    /* ================= LPUART1 ================= */
    {
        { hwGPIO_Pin_G7, hwGPIO_Pin_G8, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_A9, hwGPIO_Pin_A10, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_C0, hwGPIO_Pin_C1, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif
};

static const UART_AF_Map UART_Pin_AF_Map[] =
{
#if defined(UART1_BASE) || defined(USART1_BASE)
    /* ================= USART1 ================= */
    { hwUART_Index_0, hwGPIO_Pin_A9,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A10, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A11, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_A12, GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B6,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B7,  GPIO_AF7_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B14, GPIO_AF4_USART1 },
    { hwUART_Index_0, hwGPIO_Pin_B15, GPIO_AF4_USART1 },
#endif
#if defined(UART2_BASE) || defined(USART2_BASE)
    /* ================= USART2 ================= */
    { hwUART_Index_1, hwGPIO_Pin_A2,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A3,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A0,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_A1,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D5,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D6,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D3,  GPIO_AF7_USART2 },
    { hwUART_Index_1, hwGPIO_Pin_D4,  GPIO_AF7_USART2 },
#endif
#if defined(UART3_BASE) || defined(USART3_BASE)
    /* ================= USART3 ================= */
    { hwUART_Index_2, hwGPIO_Pin_D8,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_D9,  GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C10, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C11, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B13, GPIO_AF7_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B14, GPIO_AF7_USART3 },
#endif
#if defined(UART4_BASE) || defined(USART4_BASE)
    /* ================= UART4 ================= */
    { hwUART_Index_3, hwGPIO_Pin_B9,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_B8,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_A0,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_A1,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C10, GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_C11, GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_D1,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_D0,  GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_A15, GPIO_AF8_UART4 },
    { hwUART_Index_3, hwGPIO_Pin_B0,  GPIO_AF8_UART4 },
#endif
#if defined(UART5_BASE) || defined(USART5_BASE)
    /* ================= UART5 ================= */
    { hwUART_Index_4, hwGPIO_Pin_B13, GPIO_AF14_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_B12, GPIO_AF14_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_C12, GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_D2,  GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_B6,  GPIO_AF14_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_B5,  GPIO_AF14_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_C8,  GPIO_AF8_UART5 },
    { hwUART_Index_4, hwGPIO_Pin_C9,  GPIO_AF8_UART5 },
#endif
#if defined(UART6_BASE) || defined(USART6_BASE)
    /* ================= USART6 ================= */
    { hwUART_Index_5, hwGPIO_Pin_C6,  GPIO_AF7_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_C7,  GPIO_AF7_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G14, GPIO_AF7_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G9,  GPIO_AF7_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G8,  GPIO_AF7_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G12, GPIO_AF7_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G13, GPIO_AF7_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_G15, GPIO_AF7_USART6 },
#endif
#if defined(UART7_BASE) || defined(USART7_BASE)
    /* ================= UART7 ================= */
    { hwUART_Index_6, hwGPIO_Pin_E8,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_E7,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_E9,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_E10, GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_F7,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_F6,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_F8,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_F9,  GPIO_AF7_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_A15, GPIO_AF11_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_A8,  GPIO_AF11_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_B4,  GPIO_AF11_UART7 },
    { hwUART_Index_6, hwGPIO_Pin_B3,  GPIO_AF11_UART7 },
#endif
#if defined(UART8_BASE) || defined(USART8_BASE)
    /* ================= UART8 ================= */
    { hwUART_Index_7, hwGPIO_Pin_E1,  GPIO_AF8_UART8 },
    { hwUART_Index_7, hwGPIO_Pin_E0,  GPIO_AF8_UART8 },
    { hwUART_Index_7, hwGPIO_Pin_D15, GPIO_AF8_UART8 },
    { hwUART_Index_7, hwGPIO_Pin_D14, GPIO_AF8_UART8 },
#endif
#if defined(UART9_BASE) || defined(USART9_BASE)
    /* ================= UART9 ================= */
    { hwUART_Index_8, hwGPIO_Pin_G1,  GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_G0,  GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_D15, GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_D14, GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_D13, GPIO_AF11_UART9 },
    { hwUART_Index_8, hwGPIO_Pin_D0,  GPIO_AF11_UART9 },
#endif
#if defined(UART10_BASE) || defined(USART10_BASE)
    /* ================= USART10 ================= */
    { hwUART_Index_9, hwGPIO_Pin_E3,  GPIO_AF11_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_E2,  GPIO_AF4_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_G12, GPIO_AF4_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_G11, GPIO_AF4_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_G14, GPIO_AF4_USART10 },
    { hwUART_Index_9, hwGPIO_Pin_G13, GPIO_AF4_USART10 },
#endif
#if defined(LPUART1_BASE)
    /* ================= LPUART1 ================= */
    { hwUART_Index_L1, hwGPIO_Pin_G7, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_G8, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_A9, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_A10, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_C0, GPIO_AF8_LPUART1 },
    { hwUART_Index_L1, hwGPIO_Pin_C1, GPIO_AF8_LPUART1 },
#endif
};

#endif //UART_PIN_STM32H7_H