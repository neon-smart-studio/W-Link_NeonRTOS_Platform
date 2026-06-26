
#ifndef CAN_PIN_TITIVAC_DEF_H
#define CAN_PIN_TITIVAC_DEF_H

#include "soc.h"

#include "GPIO/Device/TITivaC/GPIO_TITivaC.h"

#include "CAN/Device/TITivaC/CAN_TITivaC.h"

typedef struct {
    hwGPIO_Pin tx_pin;
    hwGPIO_Pin rx_pin;
} CAN_Pin_Def;

#endif //CAN_PIN_TITIVAC_DEF_H