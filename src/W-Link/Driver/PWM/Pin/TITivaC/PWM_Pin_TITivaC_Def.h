
#ifndef PWM_PIN_TITIVAC_DEF_H
#define PWM_PIN_TITIVAC_DEF_H

#include "GPIO/GPIO.h"

#include "PWM/PWM.h"

#include "PWM/Device/TITivaC/PWM_TITivaC_Base.h"

typedef struct {
    hwPWM_Channel channel;
    hwPWM_Base_Index base;
    hwGPIO_Pin    pin;
} PWM_Pin_Def;

#endif //PWM_PIN_TITIVAC_DEF_H