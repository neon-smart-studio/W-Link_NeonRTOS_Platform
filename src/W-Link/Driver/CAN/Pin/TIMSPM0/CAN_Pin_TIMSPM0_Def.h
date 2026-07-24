
#ifndef CAN_PIN_TIMSPM0_DEF_H
#define CAN_PIN_TIMSPM0_DEF_H

#include "soc.h"

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"

#include "CAN/Device/TIMSPM0/CAN_TIMSPM0.h"

typedef struct {
    hwGPIO_Pin tx_pin;
    hwGPIO_Pin rx_pin;
} CAN_Pin_Def;

#endif //CAN_PIN_TIMSPM0_DEF_H