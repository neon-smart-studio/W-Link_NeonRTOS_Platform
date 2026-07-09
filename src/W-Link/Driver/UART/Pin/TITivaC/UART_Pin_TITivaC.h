
#ifndef UART_PIN_TITIVAC_H
#define UART_PIN_TITIVAC_H

#include "soc.h"

#if defined(TM4C123)
#include "UART_Pin_TM4C123.h"
#endif //TM4C123

#if defined(TM4C1294)
#include "UART_Pin_TM4C1294.h"
#endif //TM4C1294

#endif //UART_PIN_TITIVAC_H