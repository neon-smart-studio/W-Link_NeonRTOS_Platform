#ifndef UART_RP2_H
#define UART_RP2_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "UART/UART.h"
#include "GPIO/GPIO.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern bool UART_Init_Status[];

uint32_t UART_Map_Soc_Base(hwUART_Index index);
uint32_t UART_Map_Soc_Periph(hwUART_Index index);
uint32_t UART_Map_IRQ(hwUART_Index index);
uint32_t UART_Map_PinConfig(hwUART_Index index, hwGPIO_Pin pin);

void UART_IRQ_Process(hwUART_Index index);

void UART_NVIC_Init(hwUART_Index index);
void UART_NVIC_DeInit(hwUART_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // UART_RP2_H