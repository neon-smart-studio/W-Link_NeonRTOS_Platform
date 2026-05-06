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

extern bool UART_Init_Status[hwUART_Index_MAX];

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif