
#ifndef CAN_PIN_STM32_H
#define CAN_PIN_STM32_H

#include "soc.h"

#ifdef STM32F1
#include "CAN_Pin_STM32F1.h"
#endif //STM32F1

#ifdef STM32F2
#include "CAN_Pin_STM32F2.h"
#endif //STM32F2

#ifdef STM32F3
#include "CAN_Pin_STM32F3.h"
#endif //STM32F3

#ifdef STM32F4
#include "CAN_Pin_STM32F4.h"
#endif //STM32F4

#ifdef STM32F7
#include "CAN_Pin_STM32F7.h"
#endif //STM32F7

#ifdef STM32L4
#include "CAN_Pin_STM32L4.h"
#endif //STM32L4

#endif //CAN_PIN_STM32_H