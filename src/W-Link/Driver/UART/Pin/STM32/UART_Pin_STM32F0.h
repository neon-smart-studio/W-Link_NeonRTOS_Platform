#ifndef UART_PIN_STM32F0_H
#define UART_PIN_STM32F0_H

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

#ifndef CONFIG_UART5_PINSET
#define CONFIG_UART5_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART6_PINSET
#define CONFIG_UART6_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART7_PINSET
#define CONFIG_UART7_PINSET UART_Pinset_DEFAULT
#endif

const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] = {
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
};

const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
#if defined(UART1_BASE) || defined(USART1_BASE)
    /* ================= USART1 (UART0) ================= */
    {
        { hwGPIO_Pin_A9,  hwGPIO_Pin_A10, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART2_BASE) || defined(USART2_BASE)
    /* ================= USART2 (UART1) ================= */
    {
        { hwGPIO_Pin_A2,  hwGPIO_Pin_A3,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_A14, hwGPIO_Pin_A15, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART3_BASE) || defined(USART3_BASE)
    /* ================= USART3 (UART2) ================= */
    {
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART4_BASE) || defined(USART4_BASE)
    /* ================= UART4 / USART4 (UART3) ================= */
    {
        { hwGPIO_Pin_C10, hwGPIO_Pin_C11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_A0,  hwGPIO_Pin_A1,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART5_BASE) || defined(USART5_BASE)
    /* ================= UART5 / USART5 (UART4) ================= */
    {
        { hwGPIO_Pin_C12, hwGPIO_Pin_D2,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART6_BASE) || defined(USART6_BASE)
    /* ================= USART6 / UART6 (UART5) ================= */
    {
        { hwGPIO_Pin_F9,  hwGPIO_Pin_F10, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_C0,  hwGPIO_Pin_C1,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART7_BASE) || defined(USART7_BASE)
    /* ================= USART7 / UART7 (UART6) ================= */
    {
        { hwGPIO_Pin_C4,  hwGPIO_Pin_C5,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_E8,  hwGPIO_Pin_E7,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },
#endif

#if defined(UART8_BASE) || defined(USART8_BASE)
    /* ================= USART8 / UART8 (UART7) ================= */
    {
        { hwGPIO_Pin_C8,  hwGPIO_Pin_C9,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_E1,  hwGPIO_Pin_E0,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
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

#if defined(UART3_BASE) || defined(USART3_BASE)
    { hwUART_Index_2, hwGPIO_Pin_B10, GPIO_AF4_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_B11, GPIO_AF4_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C10, GPIO_AF1_USART3 },
    { hwUART_Index_2, hwGPIO_Pin_C11, GPIO_AF1_USART3 },
#endif

#if defined(UART4_BASE) || defined(USART4_BASE)
    { hwUART_Index_3, hwGPIO_Pin_C10, GPIO_AF0_USART4 },
    { hwUART_Index_3, hwGPIO_Pin_C11, GPIO_AF0_USART4 },
    { hwUART_Index_3, hwGPIO_Pin_A0,  GPIO_AF4_USART4 },
    { hwUART_Index_3, hwGPIO_Pin_A1,  GPIO_AF4_USART4 },
#endif

#if defined(UART5_BASE) || defined(USART5_BASE)
    { hwUART_Index_4, hwGPIO_Pin_C12, GPIO_AF2_USART5 },
    { hwUART_Index_4, hwGPIO_Pin_D2,  GPIO_AF2_USART5 },
#endif

#if defined(UART6_BASE) || defined(USART6_BASE)
    { hwUART_Index_5, hwGPIO_Pin_F9,  GPIO_AF2_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_F10, GPIO_AF2_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_C0,  GPIO_AF2_USART6 },
    { hwUART_Index_5, hwGPIO_Pin_C1,  GPIO_AF2_USART6 },
#endif

#if defined(UART7_BASE) || defined(USART7_BASE)
    { hwUART_Index_6, hwGPIO_Pin_C4,  GPIO_AF1_USART7 },
    { hwUART_Index_6, hwGPIO_Pin_C5,  GPIO_AF1_USART7 },
    { hwUART_Index_6, hwGPIO_Pin_E8,  GPIO_AF1_USART7 },
    { hwUART_Index_6, hwGPIO_Pin_E7,  GPIO_AF1_USART7 },
#endif

#if defined(UART8_BASE) || defined(USART8_BASE)
    { hwUART_Index_7, hwGPIO_Pin_C8,  GPIO_AF1_USART8 },
    { hwUART_Index_7, hwGPIO_Pin_C9,  GPIO_AF1_USART8 },
    { hwUART_Index_7, hwGPIO_Pin_E1,  GPIO_AF1_USART8 },
    { hwUART_Index_7, hwGPIO_Pin_E0,  GPIO_AF1_USART8 },
#endif
};

#endif // UART_PIN_STM32F0_H