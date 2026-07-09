
#ifndef PWM_PIN_TIMSP432P_H
#define PWM_PIN_TIMSP432P_H

#include "PWM_Pin_TIMSP432_Def.h"

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Channel_MAX] =
{
    /* ================= TIMER_A0 ================= */
    { hwPWM_Channel_1,  hwPWM_Base_0, hwGPIO_Pin_A12 }, // TA0.1
    { hwPWM_Channel_2,  hwPWM_Base_0, hwGPIO_Pin_A13 }, // TA0.2
    { hwPWM_Channel_3,  hwPWM_Base_0, hwGPIO_Pin_A14 }, // TA0.3
    { hwPWM_Channel_4,  hwPWM_Base_0, hwGPIO_Pin_A15 }, // TA0.4

    /* ================= TIMER_A1 ================= */
    { hwPWM_Channel_5,  hwPWM_Base_1, hwGPIO_Pin_D7 }, // TA1.1
    { hwPWM_Channel_6,  hwPWM_Base_1, hwGPIO_Pin_D6 }, // TA1.2
    { hwPWM_Channel_7,  hwPWM_Base_1, hwGPIO_Pin_D5 }, // TA1.3
    { hwPWM_Channel_8,  hwPWM_Base_1, hwGPIO_Pin_D4 }, // TA1.4

    /* ================= TIMER_A2 ================= */
    { hwPWM_Channel_9,  hwPWM_Base_2, hwGPIO_Pin_C6 }, // TA2.1
    { hwPWM_Channel_10, hwPWM_Base_2, hwGPIO_Pin_C7 }, // TA2.2
    { hwPWM_Channel_11, hwPWM_Base_2, hwGPIO_Pin_C6 }, // TA2.3
    { hwPWM_Channel_12, hwPWM_Base_2, hwGPIO_Pin_C7 }, // TA2.4

    /* ================= TIMER_A3 ================= */
    { hwPWM_Channel_13, hwPWM_Base_3, hwGPIO_Pin_E13 }, // TA3.1
    { hwPWM_Channel_14, hwPWM_Base_3, hwGPIO_Pin_D10 }, // TA3.2
    { hwPWM_Channel_15, hwPWM_Base_3, hwGPIO_Pin_E2  }, // TA3.3
    { hwPWM_Channel_16, hwPWM_Base_3, hwGPIO_Pin_E3  }, // TA3.4
};

#endif //PWM_PIN_TIMSP432P_H