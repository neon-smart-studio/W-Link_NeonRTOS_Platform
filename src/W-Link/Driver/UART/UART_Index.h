
#ifndef UART_INDEX_H
#define UART_INDEX_H

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
// 定義標準輸出的 UART 索引
#define LOG_UART_INDEX hwUART_Index_0
#endif //DEVICE_STM32

#ifdef DEVICE_RP2
typedef enum hwUART_Index_t
{
  hwUART_Index_0 = 0,
  hwUART_Index_1,
  hwUART_Index_MAX,
}hwUART_Index;
#define LOG_UART_INDEX hwUART_Index_0
#endif // DEVICE_RP2

#ifdef DEVICE_TM4C1294
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
// 定義標準輸出的 UART 索引
#define LOG_UART_INDEX hwUART_Index_0
#endif //DEVICE_TM4C1294

#endif //UART_INDEX_H