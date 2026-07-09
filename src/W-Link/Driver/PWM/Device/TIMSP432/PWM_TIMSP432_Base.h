
#ifndef PWM_TIMSP432_BASE_H
#define PWM_TIMSP432_BASE_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef	__cplusplus
extern "C" {
#endif

#if defined(MSP432P)
typedef enum
{
    hwPWM_Base_0 = 0,   // TIMER_A0
    hwPWM_Base_1,       // TIMER_A1
    hwPWM_Base_2,       // TIMER_A2
    hwPWM_Base_3,       // TIMER_A3
	hwPWM_Base_MAX
}hwPWM_Base_Index;
#endif

#if defined(MSP432E)
typedef enum
{
    hwPWM_Base_0 = 0,   // PWM0
    hwPWM_Base_1,       // PWM1
	hwPWM_Base_MAX
}hwPWM_Base_Index;
#endif

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_TIMSP432_BASE_H
