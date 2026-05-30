
#ifndef UART_PIN_STM32_H
#define UART_PIN_STM32_H

#include "UART_Pin_Nuvoton_Def.h"

#if defined(NUC442) || defined(NUC472)
#include "UART_Pin_NUC4x2.h"
#endif //NUC442 || NUC472

#endif //UART_PIN_STM32_H