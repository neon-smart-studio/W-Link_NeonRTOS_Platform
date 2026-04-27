#ifndef UART_STM32_H
#define UART_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "UART/UART.h"

#define UART_IRQ_NVIC_PRIORITY      5
#define UART_IRQ_NVIC_SUB_PRIORITY  0

extern UART_HandleTypeDef g_uart[hwUART_Index_MAX];
extern bool UART_Init_Status[hwUART_Index_MAX];

USART_TypeDef *UART_Map_Soc_Base(hwUART_Index index);

hwUART_OpResult UART_Instance_Init(
    hwUART_Index index,
    uint32_t baudrate,
    bool rts_cts,
    uint8_t data_bits,
    UART_Parity parity,
    uint8_t stop_bits
);
hwUART_OpResult UART_Instance_DeInit(hwUART_Index index);

void UART_NVIC_Init(hwUART_Index index);
void UART_NVIC_DeInit(hwUART_Index index);

#endif