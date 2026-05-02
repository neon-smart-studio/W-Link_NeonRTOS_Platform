
#ifndef CAN_PIN_STM32_DEF_H
#define CAN_PIN_STM32_DEF_H

#include "soc.h"

#include "GPIO/Device/STM32/GPIO_STM32.h"

#include "CAN/Device/STM32/CAN_STM32.h"

typedef struct {
    hwGPIO_Pin tx_pin;
    hwGPIO_Pin rx_pin;
} CAN_Pin_Def;

typedef struct {
    hwCAN_Index can;
    hwGPIO_Pin pin;
    uint32_t af;
} CAN_AF_Map;

#endif //CAN_PIN_STM32_DEF_H