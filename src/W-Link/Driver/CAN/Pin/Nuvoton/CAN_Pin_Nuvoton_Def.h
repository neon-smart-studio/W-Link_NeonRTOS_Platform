
#ifndef CAN_PIN_NUVOTON_DEF_H
#define CAN_PIN_NUVOTON_DEF_H

#include "soc.h"

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "CAN/Device/Nuvoton/CAN_Nuvoton.h"

typedef struct {
    hwGPIO_Pin tx_pin;
    hwGPIO_Pin rx_pin;
} CAN_Pin_Def;

#endif //CAN_PIN_NUVOTON_DEF_H