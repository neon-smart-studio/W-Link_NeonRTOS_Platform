
#ifndef PWM_PIN_RP2_DEF_H
#define PWM_PIN_RP2_DEF_H

#include "GPIO/GPIO.h"

#include "PWM/PWM.h"

#include "PWM/Device/RP2/PWM_RP2_Base.h"

typedef struct {
    hwPWM_Channel channel;
    hwPWM_Base_Index base;
    hwGPIO_Pin pin;
    uint8_t slice;
    uint8_t slice_ch;   // 0 = A, 1 = B
} PWM_Pin_Def;

#endif //PWM_PIN_RP2_DEF_H