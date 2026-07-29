
#ifndef UART_INDEX_H
#define UART_INDEX_H

#include "Driver_Config.h"

#include "soc.h"

#ifndef CONFIG_LOG_UART_INDEX
#define LOG_UART_INDEX hwUART_Index_0
#else
#define LOG_UART_INDEX CONFIG_LOG_UART_INDEX
#endif

#ifdef DEVICE_NUVOTON
typedef enum hwUART_Index_t
{
#if defined (UART0_BASE)
  hwUART_Index_0 = 0,
#endif
#if defined (UART1_BASE)
  hwUART_Index_1,
#endif
#if defined (UART2_BASE)
  hwUART_Index_2,
#endif
#if defined (UART3_BASE)
  hwUART_Index_3,
#endif
#if defined (UART4_BASE)
  hwUART_Index_4,
#endif
#if defined (UART5_BASE)
  hwUART_Index_5,
#endif
  hwUART_Index_MAX,
}hwUART_Index;
// 定義標準輸出的 UART 索引
#endif //DEVICE_NUVOTON

#ifdef DEVICE_STM32
typedef enum hwUART_Index_t
{
#if defined(UART1_BASE) || defined(USART1_BASE)
  hwUART_Index_0 = 0,
#endif
#if defined(UART2_BASE) || defined(USART2_BASE)
  hwUART_Index_1,
#endif
#if defined(UART3_BASE) || defined(USART3_BASE)
  hwUART_Index_2,
#endif
#if defined(UART4_BASE) || defined(USART4_BASE)
  hwUART_Index_3,
#endif
#if defined(UART5_BASE) || defined(USART5_BASE)
  hwUART_Index_4,
#endif
#if defined(UART6_BASE) || defined(USART6_BASE)
  hwUART_Index_5,
#endif
#if defined(UART7_BASE) || defined(USART7_BASE)
  hwUART_Index_6,
#endif
#if defined(UART8_BASE) || defined(USART8_BASE)
  hwUART_Index_7,
#endif
#if defined(UART9_BASE) || defined(USART9_BASE)
  hwUART_Index_8,
#endif
#if defined(UART10_BASE) || defined(USART10_BASE)
  hwUART_Index_9,
#endif
#if defined(UART11_BASE) || defined(USART11_BASE)
  hwUART_Index_10,
#endif
#if defined(UART12_BASE) || defined(USART12_BASE)
  hwUART_Index_11,
#endif
#if defined(LPUART1_BASE) || defined(LPUSART1_BASE)
  hwUART_Index_L1,
#endif
#if defined(LPUART2_BASE) || defined(LPUSART2_BASE)
  hwUART_Index_L2,
#endif
#if defined(LPUART3_BASE) || defined(LPUSART3_BASE)
  hwUART_Index_L3,
#endif
  hwUART_Index_MAX,
}hwUART_Index;
#endif //DEVICE_STM32

#ifdef DEVICE_RP2
typedef enum hwUART_Index_t
{
  hwUART_Index_0 = 0,
  hwUART_Index_1,
  hwUART_Index_MAX,
}hwUART_Index;
#endif // DEVICE_RP2

#ifdef DEVICE_TITIVAC
typedef enum hwUART_Index_t
{
  hwUART_Index_0 = 0,
  hwUART_Index_1,
  hwUART_Index_2,
  hwUART_Index_3,
  hwUART_Index_4,
  hwUART_Index_5,
  hwUART_Index_6,
  hwUART_Index_7,
  hwUART_Index_MAX,
}hwUART_Index;
#endif //DEVICE_TM4C1294

#ifdef DEVICE_TIMSP432P
typedef enum {
    hwUART_Index_0 = 0,   // EUSCI_A0
    hwUART_Index_1,       // EUSCI_A1
    hwUART_Index_2,       // EUSCI_A2
    hwUART_Index_3,       // EUSCI_A3
    hwUART_Index_MAX
} hwUART_Index;
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
typedef enum {
    hwUART_Index_0 = 0,   // UART0
    hwUART_Index_1,       // UART1
    hwUART_Index_2,       // UART2
    hwUART_Index_3,       // UART3
    hwUART_Index_4,       // UART4
    hwUART_Index_5,       // UART5
    hwUART_Index_6,       // UART6
    hwUART_Index_7,       // UART7
    hwUART_Index_MAX
} hwUART_Index;
#endif // DEVICE_TIMSP432E

#ifdef DEVICE_TIMSPM0
typedef enum
{
#if defined(UART0_BASE) || defined(UC0_UART_BASE)
    hwUART_Index_0 = 0,       // UART0 / UC0 UART
#endif

#if defined(UART1_BASE) || defined(UC1_UART_BASE)
    hwUART_Index_1,           // UART1 / UC1 UART
#endif

#if defined(UART2_BASE) || defined(UC2_UART_BASE)
    hwUART_Index_2,           // UART2 / UC2 UART
#endif

#if defined(UART3_BASE) || defined(UC3_UART_BASE)
    hwUART_Index_3,           // UART3 / UC3 UART
#endif

#if defined(UART4_BASE) || defined(UC4_UART_BASE)
    hwUART_Index_4,           // UART4 / UC4 UART
#endif

#if defined(UART5_BASE) || defined(UC5_UART_BASE)
    hwUART_Index_5,           // UART5 / UC5 UART
#endif

#if defined(UART6_BASE) || defined(UC6_UART_BASE)
    hwUART_Index_6,           // UART6 / UC6 UART
#endif

#if defined(UART7_BASE) || defined(UC7_UART_BASE)
    hwUART_Index_7,           // UART7 / UC7 UART
#endif

#if defined(UART8_BASE) || defined(UC8_UART_BASE)
    hwUART_Index_8,           // UART8 / UC8 UART
#endif

#if defined(UART9_BASE) || defined(UC9_UART_BASE)
    hwUART_Index_9,           // UART9 / UC9 UART
#endif

    hwUART_Index_MAX

} hwUART_Index;
#endif // DEVICE_TIMSPM0

#endif //UART_INDEX_H