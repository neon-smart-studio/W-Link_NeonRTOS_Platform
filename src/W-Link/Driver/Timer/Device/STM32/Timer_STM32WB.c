#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef STM32WB

#include "Timer_STM32.h"

TIM_TypeDef *Timer_Map_Soc_Base(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0: return TIM1;
#endif
#if defined(TIM2_BASE)
        case hwTimer_Index_1: return TIM2;
#endif
#if defined(TIM16_BASE)
        case hwTimer_Index_15: return TIM16;
#endif
#if defined(TIM17_BASE)
        case hwTimer_Index_16: return TIM17;
#endif
        default: return NULL;
    }
}

static uint32_t Timer_GetInputClock(hwTimer_Index index)
{
#if defined(TIM1_BASE)
    if (index == hwTimer_Index_0)
        return HAL_RCC_GetPCLK2Freq();
#endif

#if defined(TIM16_BASE)
    if (index == hwTimer_Index_15)
        return HAL_RCC_GetPCLK2Freq();
#endif

#if defined(TIM17_BASE)
    if (index == hwTimer_Index_16)
        return HAL_RCC_GetPCLK2Freq();
#endif

    return HAL_RCC_GetPCLK1Freq();
}

static void Timer_HAL_IRQHandler(hwTimer_Index index)
{
    HAL_TIM_IRQHandler(&g_timer[index]);
}

#if defined(STM32WB1Mxx) || defined (STM32WB5Mxx) || defined(STM32WB10xx) || defined(STM32WB15xx)
#if defined(TIM1_BASE)
void TIM1_UP_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_0);
}
#endif
#endif
#if defined(STM32WB30xx) || defined (STM32WB35xx) || defined (STM32WB50xx) || defined (STM32WB55xx)
#if defined(TIM1_BASE) || defined(TIM16_BASE)
void TIM1_UP_TIM16_IRQHandler(void)
{
#if defined(TIM1_BASE)
    Timer_HAL_IRQHandler(hwTimer_Index_0);
#endif
#if defined(TIM16_BASE)
    Timer_HAL_IRQHandler(hwTimer_Index_15);
#endif
}
#endif
#endif

#if defined(TIM2_BASE)
void TIM2_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_1);
}
#endif

#if defined(STM32WB30xx) || defined (STM32WB35xx) || defined (STM32WB50xx) || defined (STM32WB55xx)
#if defined(TIM17_BASE) 
void TIM1_TRG_COM_TIM17_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_16);
}
#endif
#endif

static void Timer_Enable_Clock(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0: __HAL_RCC_TIM1_CLK_ENABLE(); break;
#endif
#if defined(TIM2_BASE)
        case hwTimer_Index_1: __HAL_RCC_TIM2_CLK_ENABLE(); break;
#endif
#if defined(TIM16_BASE)
        case hwTimer_Index_15: __HAL_RCC_TIM16_CLK_ENABLE(); break;
#endif
#if defined(TIM17_BASE)
        case hwTimer_Index_16: __HAL_RCC_TIM17_CLK_ENABLE(); break;
#endif
        default: break;
    }
}

static void Timer_Disable_Clock(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0: __HAL_RCC_TIM1_CLK_DISABLE(); break;
#endif
#if defined(TIM2_BASE)
        case hwTimer_Index_1: __HAL_RCC_TIM2_CLK_DISABLE(); break;
#endif
#if defined(TIM16_BASE)
        case hwTimer_Index_15: __HAL_RCC_TIM16_CLK_DISABLE(); break;
#endif
#if defined(TIM17_BASE)
        case hwTimer_Index_16: __HAL_RCC_TIM17_CLK_DISABLE(); break;
#endif
        default: break;
    }
}

hwTimer_OpResult Timer_Instance_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
        return hwTimer_InvalidParameter;

    TIM_TypeDef *timer_soc_base = Timer_Map_Soc_Base(index);
    if (timer_soc_base == NULL)
        return hwTimer_InvalidParameter;

    Timer_Enable_Clock(index);

    g_timer[index].Instance = timer_soc_base;
    g_timer[index].Init.Prescaler =
        (Timer_GetInputClock(index) / 1000000U) - 1U;

    g_timer[index].Init.CounterMode = TIM_COUNTERMODE_UP;
    g_timer[index].Init.Period = 1000U - 1U;
    g_timer[index].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    g_timer[index].Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&g_timer[index]) != HAL_OK)
        return hwTimer_HwError;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Instance_DeInit(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
        return hwTimer_InvalidParameter;

    HAL_TIM_Base_Stop_IT(&g_timer[index]);
    HAL_TIM_Base_DeInit(&g_timer[index]);

    Timer_Disable_Clock(index);

    return hwTimer_OK;
}

void Timer_NVIC_Enable(hwTimer_Index index)
{
    switch (index)
    {
#if defined(STM32WB1Mxx) || defined (STM32WB5Mxx) || defined(STM32WB10xx) || defined(STM32WB15xx)
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
            HAL_NVIC_SetPriority(TIM1_UP_IRQn, TIMER_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
            break;
#endif
#endif
#if defined(STM32WB30xx) || defined (STM32WB35xx) || defined (STM32WB50xx) || defined (STM32WB55xx)
#if defined(TIM1_BASE) || defined(TIM16_BASE)
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
#endif
#if defined(TIM16_BASE)
        case hwTimer_Index_15:
#endif
            HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, TIMER_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
            break;
#endif
#endif

#if defined(TIM2_BASE)
        case hwTimer_Index_1:
            HAL_NVIC_SetPriority(TIM2_IRQn, TIMER_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(TIM2_IRQn);
            break;
#endif

#if defined(STM32WB30xx) || defined (STM32WB35xx) || defined (STM32WB50xx) || defined (STM32WB55xx)
#if defined(TIM17_BASE) 
        case hwTimer_Index_16:
#endif
            HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM17_IRQn, TIMER_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM17_IRQn);
            break;
#endif

        default:
            break;
    }
}

void Timer_NVIC_Disable(hwTimer_Index index)
{
    switch (index)
    {
#if defined(STM32WB1Mxx) || defined (STM32WB5Mxx) || defined(STM32WB10xx) || defined(STM32WB15xx)
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
            HAL_NVIC_DisableIRQ(TIM1_UP_IRQn);
            break;
#endif
#endif
#if defined(STM32WB30xx) || defined (STM32WB35xx) || defined (STM32WB50xx) || defined (STM32WB55xx)
#if defined(TIM1_BASE) || defined(TIM16_BASE)
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
#endif
#if defined(TIM16_BASE)
        case hwTimer_Index_15:
#endif
#if defined(TIM1_BASE) && defined(TIM16_BASE)
            if(!Timer_Init_Status[hwTimer_Index_0] && !Timer_Init_Status[hwTimer_Index_15])
#endif
            {
                HAL_NVIC_DisableIRQ(TIM1_UP_TIM16_IRQn);
            }
            break;
#endif
#endif

#if defined(TIM2_BASE)
        case hwTimer_Index_1:
            HAL_NVIC_DisableIRQ(TIM2_IRQn);
            break;
#endif

#if defined(STM32WB30xx) || defined (STM32WB35xx) || defined (STM32WB50xx) || defined (STM32WB55xx)
#if defined(TIM17_BASE) 
        case hwTimer_Index_16:
#endif
            HAL_NVIC_DisableIRQ(TIM1_TRG_COM_TIM17_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif // STM32WB