
#ifndef UART_PIN_H
#define UART_PIN_H

#ifdef DEVICE_STM32
#include "STM32/UART_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/UART_Pin_RP2.h"
#endif

#ifdef DEVICE_TM4C1294
#include "TI/UART_Pin_TM4C1294.h"
#endif

#endif //UART_PIN_H