
#ifndef I2C_PIN_RP2_Def_H
#define I2C_PIN_RP2_Def_H

#include "GPIO/GPIO.h"

#include "I2C/I2C_Master.h"

typedef struct {
    hwGPIO_Pin scl_pin;
    hwGPIO_Pin sda_pin;
} I2C_Pin_Def;

#endif //I2C_PIN_RP2_Def_H