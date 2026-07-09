
#ifndef PWM_PIN_TM4C1294_H
#define PWM_PIN_TM4C1294_H

#include "PWM_Pin_TITivaC_Def.h"

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Channel_MAX] =
{
    /* ================= PWM0 ================= */
    { hwPWM_Channel_1, hwPWM_Base_0, hwGPIO_Pin_F0 },
    { hwPWM_Channel_2, hwPWM_Base_0, hwGPIO_Pin_F1 },
    { hwPWM_Channel_3, hwPWM_Base_0, hwGPIO_Pin_F2 },
    { hwPWM_Channel_4, hwPWM_Base_0, hwGPIO_Pin_F3 },
    { hwPWM_Channel_5, hwPWM_Base_0, hwGPIO_Pin_G0 },
    { hwPWM_Channel_6, hwPWM_Base_0, hwGPIO_Pin_G1 },
    { hwPWM_Channel_7, hwPWM_Base_0, hwGPIO_Pin_K4 },
    { hwPWM_Channel_8, hwPWM_Base_0, hwGPIO_Pin_K5 },
};

#endif //PWM_PIN_TM4C1294_H