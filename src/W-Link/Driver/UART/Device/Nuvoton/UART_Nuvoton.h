#ifndef UART_NUC4x2_H
#define UART_NUC4x2_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "UART/UART.h"
#include "GPIO/GPIO.h"

#define UART_IRQ_NVIC_PRIORITY      5
#define UART_IRQ_NVIC_SUB_PRIORITY  0

#ifdef	__cplusplus
extern "C" {
#endif

extern bool UART_Init_Status[hwUART_Index_MAX];

UART_T *UART_Map_Soc_Base(hwUART_Index index);

void UART_RxCpltCallback(hwUART_Index index);
void UART_TxCpltCallback(hwUART_Index index);

void UART_GPIO_ConfigAF(hwUART_Index index, bool rts_cts);
void UART_GPIO_DeConfigAF(hwUART_Index index, bool rts_cts);

hwUART_OpResult UART_Instance_Init(
    hwUART_Index index,
    uint32_t baudrate,
    bool rts_cts,
    uint8_t data_bits,
    UART_Parity parity,
    uint8_t stop_bits
);
hwUART_OpResult UART_Instance_DeInit(hwUART_Index index);
hwUART_OpResult UART_Instance_Read_IT(hwUART_Index index, uint8_t *data_rd, size_t size);
hwUART_OpResult UART_Instance_Write_IT(hwUART_Index index, uint8_t *data_wr, size_t size);
hwUART_OpResult UART_Instance_Stop_Read(hwUART_Index index);
hwUART_OpResult UART_Instance_Stop_Write(hwUART_Index index);

void UART_NVIC_Init(hwUART_Index index);
void UART_NVIC_DeInit(hwUART_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // UART_NUC4x2_H