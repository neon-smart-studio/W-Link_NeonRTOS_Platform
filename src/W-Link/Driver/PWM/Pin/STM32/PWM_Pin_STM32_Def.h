
#ifndef PWM_PIN_STM32_DEF_H
#define PWM_PIN_STM32_DEF_H

#include "GPIO/GPIO.h"

#include "Timer/Timer.h"

#include "PWM/PWM.h"

typedef struct {
    hwPWM_Channel channel;
    hwTimer_Index timer;
    hwGPIO_Pin    pin;
    uint8_t       chx;   // 1~4
    uint8_t       is_n;       // CHxN
} PWM_Pin_Def;

typedef struct {
    hwTimer_Index timer;
    hwGPIO_Pin   pin;
    uint8_t      af;
} PWM_AF_Map;

#endif //PWM_PIN_STM32_DEF_H