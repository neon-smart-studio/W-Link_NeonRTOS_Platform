
#ifndef PWM_PIN_NUVOTON_DEF_H
#define PWM_PIN_NUVOTON_DEF_H

#include "GPIO/GPIO.h"

#include "PWM/PWM.h"

#include "PWM/Device/Nuvoton/PWM_Nuvoton_Base.h"

typedef struct {
    hwPWM_Channel channel;
    hwPWM_Base_Index base;
    hwGPIO_Pin    pin;
    uint8_t       chx;   // 1~6
} PWM_Pin_Def;

#endif //PWM_PIN_NUVOTON_DEF_H