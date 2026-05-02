#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Timer/Timer.h"

#include "PWM/PWM.h"

#ifdef STM32L0

#include "PWM_STM32.h"

/* ================= Clock Enable ================= */

void PWM_Clock_Enable(hwPWM_Channel ch)
{
    if (ch >= hwPWM_Channel_MAX)
        return;

    const PWM_Pin_Def *def = PWM_Find_PinDef(ch);
    if (!def || def->pin == hwGPIO_Pin_NC)
        return;

    if (Timer_is_Init(def->timer))
        return;

    switch (def->timer)
    {
#if defined(TIM2_BASE)
        case hwTimer_Index_1:
            __HAL_RCC_TIM2_CLK_ENABLE();
            break;
#endif

#if defined(TIM3_BASE)
        case hwTimer_Index_2:
            __HAL_RCC_TIM3_CLK_ENABLE();
            break;
#endif

#if defined(TIM21_BASE)
        case hwTimer_Index_20:
            __HAL_RCC_TIM21_CLK_ENABLE();
            break;
#endif

#if defined(TIM22_BASE)
        case hwTimer_Index_21:
            __HAL_RCC_TIM22_CLK_ENABLE();
            break;
#endif

        default:
            break;
    }
}

/* ================= Clock Disable ================= */

void PWM_Clock_Disable(hwPWM_Channel ch)
{
    if (ch >= hwPWM_Channel_MAX)
        return;

    const PWM_Pin_Def *def = PWM_Find_PinDef(ch);
    if (!def || def->pin == hwGPIO_Pin_NC)
        return;

    if (Timer_is_Init(def->timer))
        return;

    bool all_off = true;

    switch(def->timer)
    {
#if defined(TIM2_BASE)
        case hwTimer_Index_1:
                all_off = true;
                for(hwPWM_Channel i = hwPWM_Channel_5; i <= hwPWM_Channel_8; i++)
                {
                        {
                                if(PWM_Channel_Init_Status[i])
                                {
                                    all_off = false;    
                                }
                        }
                }
                if(all_off)
                {
                        __HAL_RCC_TIM2_CLK_DISABLE();
                }
                break;
#endif
#if defined(TIM3_BASE)
        case hwTimer_Index_2:
                all_off = true;
                for(hwPWM_Channel i = hwPWM_Channel_9; i <= hwPWM_Channel_12; i++)
                {
                        {
                                if(PWM_Channel_Init_Status[i])
                                {
                                    all_off = false;    
                                }
                        }
                }
                if(all_off)
                {
                        __HAL_RCC_TIM3_CLK_DISABLE();
                }
                break;
#endif
#if defined(TIM21_BASE)
        case hwTimer_Index_20:
                all_off = true;
                if(PWM_Channel_Init_Status[hwPWM_Channel_41] || PWM_Channel_Init_Status[hwPWM_Channel_42])
                {
                        all_off = false;  
                }
                if(all_off)
                {
                        __HAL_RCC_TIM21_CLK_DISABLE();
                }
                break;
#endif
#if defined(TIM22_BASE)
        case hwTimer_Index_21:
                all_off = true;
                if(PWM_Channel_Init_Status[hwPWM_Channel_43] || PWM_Channel_Init_Status[hwPWM_Channel_44])
                {
                        all_off = false;  
                }
                if(all_off)
                {
                        __HAL_RCC_TIM22_CLK_DISABLE();
                }
                break;
#endif

        default:
                break;
    }
}

#endif // STM32L0