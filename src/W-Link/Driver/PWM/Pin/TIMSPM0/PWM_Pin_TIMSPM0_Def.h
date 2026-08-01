
#ifndef PWM_PIN_TIMSPM0_DEF_H
#define PWM_PIN_TIMSPM0_DEF_H

#include "GPIO/GPIO.h"

#include "Timer/Timer.h"

#include "PWM/PWM.h"

typedef struct {
    hwPWM_Channel channel;
    hwTimer_Index timer;
    hwGPIO_Pin pin;
    uint8_t compare_index;
} PWM_Pin_Def;

#endif //PWM_PIN_TIMSPM0_DEF_H