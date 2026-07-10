
#ifndef CAN_PIN_TIMSP432_DEF_H
#define CAN_PIN_TIMSP432_DEF_H

#include "soc.h"

#include "GPIO/Device/TIMSP432/GPIO_TIMSP432E.h"

#include "CAN/Device/TIMSP432/CAN_TIMSP432.h"

typedef struct {
    hwGPIO_Pin tx_pin;
    hwGPIO_Pin rx_pin;
} CAN_Pin_Def;

#endif //CAN_PIN_TIMSP432_DEF_H