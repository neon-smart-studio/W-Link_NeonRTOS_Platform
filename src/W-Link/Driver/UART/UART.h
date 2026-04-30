
#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"  

#include "UART_Def.h"

#include "UART_Index.h"

hwUART_OpResult UART_Open(hwUART_Index index, uint32_t baudrate, bool rts_cts);
hwUART_OpResult UART_Open_Specific_Format(hwUART_Index index, uint32_t baudrate, bool rts_cts, uint8_t data_bits, UART_Parity parity, uint8_t stop_bits);
hwUART_OpResult UART_Close(hwUART_Index index);
hwUART_OpResult UART_Read(hwUART_Index index, uint8_t* data_rd, size_t size, uint32_t timeoutMs);
hwUART_OpResult UART_GetChar(hwUART_Index index, uint8_t* char_rd, uint32_t timeoutMs);
hwUART_OpResult UART_Write(hwUART_Index index, uint8_t* data_wr, size_t size, uint32_t timeoutMs);
hwUART_OpResult UART_PutChar(hwUART_Index index, uint8_t char_wr, uint32_t timeoutMs);
void UART_Printf(const char *format, ...);

#endif //UART_H