#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Timer/Timer.h"

#include "PWM/PWM.h"

#ifdef STM32WB

#include "PWM_STM32.h"

void PWM_Clock_Enable(hwPWM_Channel channel_index)
{
    if (channel_index >= hwPWM_Channel_MAX)
        return;

    const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
    if (!def || def->pin == hwGPIO_Pin_NC)
        return;

    if (Timer_is_Init(def->timer))
        return;

    switch (def->timer)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
            __HAL_RCC_TIM1_CLK_ENABLE();
            break;
#endif

#if defined(TIM2_BASE)
        case hwTimer_Index_1:
            __HAL_RCC_TIM2_CLK_ENABLE();
            break;
#endif

#if defined(TIM16_BASE)
        case hwTimer_Index_15:
            __HAL_RCC_TIM16_CLK_ENABLE();
            break;
#endif

#if defined(TIM17_BASE)
        case hwTimer_Index_16:
            __HAL_RCC_TIM17_CLK_ENABLE();
            break;
#endif

        default:
            break;
    }
}

void PWM_Clock_Disable(hwPWM_Channel channel_index)
{
    if (channel_index >= hwPWM_Channel_MAX)
        return;

    const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
    if (!def || def->pin == hwGPIO_Pin_NC)
        return;

    if (Timer_is_Init(def->timer))
        return;

    bool all_off = true;

    switch (def->timer)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
            for (hwPWM_Channel i = hwPWM_Channel_1; i <= hwPWM_Channel_4; i++) {
                if (PWM_Channel_Init_Status[i]) {
                    all_off = false;
                    break;
                }
            }
            if (all_off)
                __HAL_RCC_TIM1_CLK_DISABLE();
            break;
#endif

#if defined(TIM2_BASE)
        case hwTimer_Index_1:
            for (hwPWM_Channel i = hwPWM_Channel_5; i <= hwPWM_Channel_8; i++) {
                if (PWM_Channel_Init_Status[i]) {
                    all_off = false;
                    break;
                }
            }
            if (all_off)
                __HAL_RCC_TIM2_CLK_DISABLE();
            break;
#endif

#if defined(TIM16_BASE)
        case hwTimer_Index_15:
            if (PWM_Channel_Init_Status[hwPWM_Channel_35])
                all_off = false;

            if (all_off)
                __HAL_RCC_TIM16_CLK_DISABLE();
            break;
#endif

#if defined(TIM17_BASE)
        case hwTimer_Index_16:
            if (PWM_Channel_Init_Status[hwPWM_Channel_36])
                all_off = false;

            if (all_off)
                __HAL_RCC_TIM17_CLK_DISABLE();
            break;
#endif

        default:
            break;
    }
}

#endif // STM32WB