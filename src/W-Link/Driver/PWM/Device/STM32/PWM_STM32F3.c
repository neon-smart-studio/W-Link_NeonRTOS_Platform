
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Timer/Timer.h"

#include "PWM/PWM.h"

#ifdef STM32F3

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
                case hwTimer_Index_0: __HAL_RCC_TIM1_CLK_ENABLE(); break;
#endif
#if defined(TIM2_BASE)
                case hwTimer_Index_1: __HAL_RCC_TIM2_CLK_ENABLE(); break;
#endif
#if defined(TIM3_BASE)
                case hwTimer_Index_2: __HAL_RCC_TIM3_CLK_ENABLE(); break;
#endif
#if defined(TIM4_BASE)
                case hwTimer_Index_3: __HAL_RCC_TIM4_CLK_ENABLE(); break;
#endif
#if defined(TIM5_BASE)
                case hwTimer_Index_4: __HAL_RCC_TIM5_CLK_ENABLE(); break;
#endif
#if defined(TIM6_BASE)
                case hwTimer_Index_5: __HAL_RCC_TIM6_CLK_ENABLE(); break;
#endif
#if defined(TIM7_BASE)
                case hwTimer_Index_6: __HAL_RCC_TIM7_CLK_ENABLE(); break;
#endif
#if defined(TIM8_BASE)
                case hwTimer_Index_7: __HAL_RCC_TIM8_CLK_ENABLE(); break;
#endif
#if defined(TIM9_BASE)
                case hwTimer_Index_8: __HAL_RCC_TIM9_CLK_ENABLE(); break;
#endif
#if defined(TIM10_BASE)
                case hwTimer_Index_9: __HAL_RCC_TIM10_CLK_ENABLE(); break;
#endif
#if defined(TIM11_BASE)
                case hwTimer_Index_10: __HAL_RCC_TIM11_CLK_ENABLE(); break;
#endif
#if defined(TIM12_BASE)
                case hwTimer_Index_11: __HAL_RCC_TIM12_CLK_ENABLE(); break;
#endif
#if defined(TIM13_BASE)
                case hwTimer_Index_12: __HAL_RCC_TIM13_CLK_ENABLE(); break;
#endif
#if defined(TIM14_BASE)
                case hwTimer_Index_13: __HAL_RCC_TIM14_CLK_ENABLE(); break;
#endif
#if defined(TIM15_BASE)
                case hwTimer_Index_14: __HAL_RCC_TIM15_CLK_ENABLE(); break;
#endif
#if defined(TIM16_BASE)
                case hwTimer_Index_15: __HAL_RCC_TIM16_CLK_ENABLE(); break;
#endif
#if defined(TIM17_BASE)
                case hwTimer_Index_16: __HAL_RCC_TIM17_CLK_ENABLE(); break;
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
#if defined(TIM4_BASE)
                case hwTimer_Index_3:
                        all_off = true;
                        for(hwPWM_Channel i = hwPWM_Channel_13; i <= hwPWM_Channel_16; i++)
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
                                __HAL_RCC_TIM4_CLK_DISABLE();
                        }
                        break;
#endif
#if defined(TIM5_BASE)
                case hwTimer_Index_4:
                        all_off = true;
                        for(hwPWM_Channel i = hwPWM_Channel_17; i <= hwPWM_Channel_20; i++)
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
                                __HAL_RCC_TIM5_CLK_DISABLE();
                        }
                        break;
#endif
#if defined(TIM8_BASE)
                case hwTimer_Index_7: //
                        all_off = true;
                        for(hwPWM_Channel i = hwPWM_Channel_21; i <= hwPWM_Channel_24; i++)
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
                                __HAL_RCC_TIM8_CLK_DISABLE();
                        }
                        break;
#endif
#if defined(TIM9_BASE)
                case hwTimer_Index_8:
                        all_off = true;
                        for(hwPWM_Channel i = hwPWM_Channel_25; i <= hwPWM_Channel_26; i++)
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
                                __HAL_RCC_TIM9_CLK_DISABLE();
                        }
                        break;
#endif
#if defined(TIM10_BASE)
                case hwTimer_Index_9:
                        all_off = true;
                        if(PWM_Channel_Init_Status[hwPWM_Channel_27])
                        {
                                all_off = false;  
                        }
                        if(all_off)
                        {
                                __HAL_RCC_TIM10_CLK_DISABLE();
                        }
                        break;
#endif
#if defined(TIM11_BASE)
                case hwTimer_Index_10:
                        all_off = true;
                        if(PWM_Channel_Init_Status[hwPWM_Channel_28])
                        {
                                all_off = false;  
                        }
                        if(all_off)
                        {
                                __HAL_RCC_TIM12_CLK_DISABLE();
                        }
                        break;
#endif
#if defined(TIM12_BASE)
                case hwTimer_Index_11:
                        all_off = true;
                        for(hwPWM_Channel i = hwPWM_Channel_29; i <= hwPWM_Channel_30; i++)
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
                                __HAL_RCC_TIM12_CLK_DISABLE();
                        }
                        break;
#endif
#if defined(TIM13_BASE)
                case hwTimer_Index_12:
                        all_off = true;
                        if(PWM_Channel_Init_Status[hwPWM_Channel_31])
                        {
                                all_off = false;  
                        }
                        if(all_off)
                        {
                                __HAL_RCC_TIM13_CLK_DISABLE();
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
#if defined(TIM15_BASE)
                case hwTimer_Index_14:
                        all_off = true;
                        if(PWM_Channel_Init_Status[hwPWM_Channel_33] || PWM_Channel_Init_Status[hwPWM_Channel_34])
                        {
                                all_off = false;  
                        }
                        if(all_off)
                        {
                                __HAL_RCC_TIM15_CLK_DISABLE();
                        }
                        break;
#endif
#if defined(TIM16_BASE)
                case hwTimer_Index_15:
                        all_off = true;
                        if(PWM_Channel_Init_Status[hwPWM_Channel_35])
                        {
                                all_off = false;  
                        }
                        if(all_off)
                        {
                                __HAL_RCC_TIM16_CLK_DISABLE();
                        }
                        break;
#endif
#if defined(TIM17_BASE)
                case hwTimer_Index_16:
                        all_off = true;
                        if(PWM_Channel_Init_Status[hwPWM_Channel_36])
                        {
                                all_off = false;  
                        }
                        if(all_off)
                        {
                                __HAL_RCC_TIM17_CLK_DISABLE();
                        }
                        break;
#endif
        }
}

#endif //STM32F3
