
#ifndef PWM_STM32_BASE_H
#define PWM_STM32_BASE_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
#if defined (TIM1_BASE)
    hwPWM_Base_1 = 0,
#endif
#if defined (TIM2_BASE)
    hwPWM_Base_2,
#endif
#if defined (TIM3_BASE)
    hwPWM_Base_3,
#endif
#if defined (TIM4_BASE)
    hwPWM_Base_4,
#endif
#if defined (TIM5_BASE)
    hwPWM_Base_5,
#endif
#if defined (TIM6_BASE)
    hwPWM_Base_6,
#endif
#if defined (TIM7_BASE)
    hwPWM_Base_7,
#endif
#if defined (TIM8_BASE)
    hwPWM_Base_8,
#endif
#if defined (TIM9_BASE)
    hwPWM_Base_9,
#endif
#if defined (TIM10_BASE)
    hwPWM_Base_10,
#endif
#if defined (TIM11_BASE)
    hwPWM_Base_11,
#endif
#if defined (TIM12_BASE)
    hwPWM_Base_12,
#endif
#if defined (TIM13_BASE)
    hwPWM_Base_13,
#endif
#if defined (TIM14_BASE)
    hwPWM_Base_14,
#endif
#if defined (TIM15_BASE)
    hwPWM_Base_15,
#endif
#if defined (TIM16_BASE)
    hwPWM_Base_16,
#endif
#if defined (TIM17_BASE)
    hwPWM_Base_17,
#endif
#if defined (TIM18_BASE)
    hwPWM_Base_18,
#endif
#if defined (TIM19_BASE)
    hwPWM_Base_19,
#endif
#if defined (TIM20_BASE)
    hwPWM_Base_20,
#endif
#if defined(TIM21_BASE)
    hwPWM_Base_21,
#endif
#if defined(TIM22_BASE)
    hwPWM_Base_22,
#endif
#if defined(TIM23_BASE)
    hwPWM_Base_23,
#endif
#if defined(TIM24_BASE)
    hwPWM_Base_24,
#endif
	hwPWM_Base_MAX
}hwPWM_Base_Index;

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_STM32_BASE_H
