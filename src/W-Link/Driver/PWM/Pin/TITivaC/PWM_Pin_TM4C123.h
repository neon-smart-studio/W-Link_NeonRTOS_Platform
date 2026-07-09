
#ifndef PWM_PIN_TM4C123_H
#define PWM_PIN_TM4C123_H

#include "PWM_Pin_TITivaC_Def.h"

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Channel_MAX] =
{
    /* ================= PWM0 ================= */
    { hwPWM_Channel_1,  hwPWM_Base_0, hwGPIO_Pin_B6 }, // M0PWM0
    { hwPWM_Channel_2,  hwPWM_Base_0, hwGPIO_Pin_B7 }, // M0PWM1
    { hwPWM_Channel_3,  hwPWM_Base_0, hwGPIO_Pin_B4 }, // M0PWM2
    { hwPWM_Channel_4,  hwPWM_Base_0, hwGPIO_Pin_B5 }, // M0PWM3
    { hwPWM_Channel_5,  hwPWM_Base_0, hwGPIO_Pin_E4 }, // M0PWM4
    { hwPWM_Channel_6,  hwPWM_Base_0, hwGPIO_Pin_E5 }, // M0PWM5
    { hwPWM_Channel_7,  hwPWM_Base_0, hwGPIO_Pin_C4 }, // M0PWM6
    { hwPWM_Channel_8,  hwPWM_Base_0, hwGPIO_Pin_C5 }, // M0PWM7

    /* ================= PWM1 ================= */
    { hwPWM_Channel_9,  hwPWM_Base_1, hwGPIO_Pin_D0 }, // M1PWM0
    { hwPWM_Channel_10, hwPWM_Base_1, hwGPIO_Pin_D1 }, // M1PWM1
    { hwPWM_Channel_11, hwPWM_Base_1, hwGPIO_Pin_A6 }, // M1PWM2
    { hwPWM_Channel_12, hwPWM_Base_1, hwGPIO_Pin_A7 }, // M1PWM3
    { hwPWM_Channel_13, hwPWM_Base_1, hwGPIO_Pin_F0 }, // M1PWM4
    { hwPWM_Channel_14, hwPWM_Base_1, hwGPIO_Pin_F1 }, // M1PWM5
    { hwPWM_Channel_15, hwPWM_Base_1, hwGPIO_Pin_F2 }, // M1PWM6
    { hwPWM_Channel_16, hwPWM_Base_1, hwGPIO_Pin_F3 }, // M1PWM7
};

#endif //PWM_PIN_TM4C123_H