
#ifndef PWM_PIN_TIMSPM0_DEF_H
#define PWM_PIN_TIMSPM0_DEF_H

#include "GPIO/GPIO.h"

#include "PWM/PWM.h"

#include "PWM/Device/TIMSPM0/PWM_TIMSPM0_Base.h"

typedef struct {
    hwPWM_Channel channel;
    hwPWM_Base_Index base;
    hwGPIO_Pin pin;
    uint8_t compare_index;
} PWM_Pin_Def;

#endif //PWM_PIN_TIMSPM0_DEF_H