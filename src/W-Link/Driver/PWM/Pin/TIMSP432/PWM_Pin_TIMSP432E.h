
#ifndef PWM_PIN_TIMSP432E_H
#define PWM_PIN_TIMSP432E_H

#include "PWM_Pin_TIMSP432_Def.h"

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Channel_MAX] =
{
    /* ================= PWM0 Module ================= */
    { hwPWM_Channel_1,  hwPWM_Base_0, hwGPIO_Pin_F0 }, // M0PWM0
    { hwPWM_Channel_2,  hwPWM_Base_0, hwGPIO_Pin_F1 }, // M0PWM1
    { hwPWM_Channel_3,  hwPWM_Base_0, hwGPIO_Pin_F2 }, // M0PWM2
    { hwPWM_Channel_4,  hwPWM_Base_0, hwGPIO_Pin_F3 }, // M0PWM3
    { hwPWM_Channel_5,  hwPWM_Base_0, hwGPIO_Pin_G0 }, // M0PWM4
    { hwPWM_Channel_6,  hwPWM_Base_0, hwGPIO_Pin_G1 }, // M0PWM5
    { hwPWM_Channel_7,  hwPWM_Base_0, hwGPIO_Pin_K4 }, // M0PWM6
    { hwPWM_Channel_8,  hwPWM_Base_0, hwGPIO_Pin_K5 }, // M0PWM7

    /* ================= PWM1 Module ================= */
    { hwPWM_Channel_9,  hwPWM_Base_1, hwGPIO_Pin_G2 }, // M1PWM0
    { hwPWM_Channel_10, hwPWM_Base_1, hwGPIO_Pin_G3 }, // M1PWM1
    { hwPWM_Channel_11, hwPWM_Base_1, hwGPIO_Pin_G4 }, // M1PWM2
    { hwPWM_Channel_12, hwPWM_Base_1, hwGPIO_Pin_G5 }, // M1PWM3
    { hwPWM_Channel_13, hwPWM_Base_1, hwGPIO_Pin_L0 }, // M1PWM4
    { hwPWM_Channel_14, hwPWM_Base_1, hwGPIO_Pin_L1 }, // M1PWM5
    { hwPWM_Channel_15, hwPWM_Base_1, hwGPIO_Pin_L2 }, // M1PWM6
    { hwPWM_Channel_16, hwPWM_Base_1, hwGPIO_Pin_L3 }, // M1PWM7
};

#endif //PWM_PIN_TIMSP432E_H