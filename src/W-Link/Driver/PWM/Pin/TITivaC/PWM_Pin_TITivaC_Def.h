
#ifndef PWM_PIN_TITIVAC_DEF_H
#define PWM_PIN_TITIVAC_DEF_H

#include "GPIO/GPIO.h"

#include "Timer/Timer.h"

#include "PWM/PWM.h"

typedef struct {
    hwPWM_Channel channel;
    hwGPIO_Pin    pin;
} PWM_Pin_Def;

#endif //PWM_PIN_TITIVAC_DEF_H