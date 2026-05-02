
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Timer/Timer.h"

#include "PWM/PWM.h"

#ifdef STM32C0

#include "PWM_STM32.h"

void PWM_Clock_Enable(hwPWM_Channel channel_index)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return;
        }
        
        const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
        if (!def || def->pin == hwGPIO_Pin_NC)
        {
                return;
        }

        if(Timer_is_Init(def->timer))
        {
                return;
        }

        switch(def->timer)
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
#if defined(TIM3_BASE)
                case hwTimer_Index_2:
                        __HAL_RCC_TIM3_CLK_ENABLE();
                        break;
#endif
#if defined(TIM14_BASE)
                case hwTimer_Index_13:
                        __HAL_RCC_TIM14_CLK_ENABLE();
                        break;
#endif
        }
}

void PWM_Clock_Disable(hwPWM_Channel channel_index)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return;
        }
        
        const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
        if (!def || def->pin == hwGPIO_Pin_NC)
        {
                return;
        }

        if(Timer_is_Init(def->timer))
        {
                return;
        }

        bool all_off;

        switch(def->timer)
        {
#if defined(TIM1_BASE)
                case hwTimer_Index_0:
                        all_off = true;
                        for(hwPWM_Channel i = hwPWM_Channel_1; i <= hwPWM_Channel_4; i++)
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
                                __HAL_RCC_TIM1_CLK_DISABLE();
                        }
                        break;
#endif
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
#if defined(TIM14_BASE)
                case hwTimer_Index_13:
                        all_off = true;
                        if(PWM_Channel_Init_Status[hwPWM_Channel_32])
                        {
                                all_off = false;  
                        }
                        if(all_off)
                        {
                                __HAL_RCC_TIM14_CLK_DISABLE();
                        }
                        break;
#endif
        }
}

#endif //STM32C0
