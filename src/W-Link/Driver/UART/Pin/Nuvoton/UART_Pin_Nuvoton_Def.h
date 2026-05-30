
#ifndef UART_PIN_NUVOTON_DEF_H
#define UART_PIN_NUVOTON_DEF_H

#include "GPIO/GPIO.h"

#include "UART/UART.h"

typedef struct { 
    hwGPIO_Pin tx_pin;
    hwGPIO_Pin rx_pin;
    hwGPIO_Pin rts_pin; // 可為 NC
    hwGPIO_Pin cts_pin; // 可為 NC
} UART_Pin_Def;

#endif //UART_PIN_NUVOTON_DEF_H