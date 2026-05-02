
#ifndef CAN_PIN_H
#define CAN_PIN_H

#ifdef DEVICE_STM32
#include "STM32/CAN_Pin_STM32.h"
#endif

#ifdef DEVICE_TM4C1294
#include "TI/CAN_Pin_TM4C1294.h"
#endif

#endif //CAN_PIN_H