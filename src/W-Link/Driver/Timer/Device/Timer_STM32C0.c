#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef STM32C0

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
#if defined(TIM3_BASE)
        case hwTimer_Index_2: return TIM3;
#endif
#if defined(TIM14_BASE)
        case hwTimer_Index_13: return TIM14;
#endif
        default: return NULL;
    }
}

/* C0 直接用 PCLK，不用 F0 那種倍頻判斷 */
static uint32_t Timer_GetInputClock(hwTimer_Index index)
{
#if defined(TIM1_BASE)
    if (index == hwTimer_Index_0)
        return HAL_RCC_GetPCLK2Freq();
#endif

    return HAL_RCC_GetPCLK1Freq();
}

/* ===== IRQ Callback ===== */

static void Timer_HAL_IRQHandler(hwTimer_Index index)
{
    HAL_TIM_IRQHandler(&g_timer[index]);
}

/* ================= IRQ Handlers ================= */

#if defined(TIM1_BASE)
void TIM1_UP_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_0);
}
#endif

#if defined(TIM2_BASE)
void TIM2_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_1);
}
#endif

#if defined(TIM3_BASE)
void TIM3_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_2);
}
#endif

#if defined(TIM14_BASE)
void TIM14_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_13);
}
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
#if defined(TIM3_BASE)
        case hwTimer_Index_2: __HAL_RCC_TIM3_CLK_ENABLE(); break;
#endif
#if defined(TIM14_BASE)
        case hwTimer_Index_13: __HAL_RCC_TIM14_CLK_ENABLE(); break;
#endif
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
#if defined(TIM3_BASE)
        case hwTimer_Index_2: __HAL_RCC_TIM3_CLK_DISABLE(); break;
#endif
#if defined(TIM14_BASE)
        case hwTimer_Index_13: __HAL_RCC_TIM14_CLK_DISABLE(); break;
#endif
    }
}

hwTimer_OpResult Timer_Instance_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    TIM_TypeDef *timer_soc_base = Timer_Map_Soc_Base(index);
    if (timer_soc_base == NULL) {
        return hwTimer_InvalidParameter;
    }

    Timer_Enable_Clock(index);

    g_timer[index].Instance = timer_soc_base;
    g_timer[index].Init.Prescaler         = (Timer_GetInputClock(index) / 1000000U) - 1U;
    g_timer[index].Init.CounterMode       = TIM_COUNTERMODE_UP;
    g_timer[index].Init.Period            = 1000U - 1U;
    g_timer[index].Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    g_timer[index].Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&g_timer[index]) != HAL_OK) {
        return hwTimer_HwError;
    }

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Instance_DeInit(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    HAL_TIM_Base_Stop_IT(&g_timer[index]);
    HAL_TIM_Base_DeInit(&g_timer[index]);

    Timer_Disable_Clock(index);

    return hwTimer_OK;
}

void Timer_NVIC_Enable(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_UP_IRQn)
        case hwTimer_Index_0:
            HAL_NVIC_SetPriority(TIM1_UP_IRQn, TIMER_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
            break;
#endif

#if defined(TIM2_IRQn)
        case hwTimer_Index_1:
            HAL_NVIC_SetPriority(TIM2_IRQn, TIMER_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(TIM2_IRQn);
            break;
#endif

#if defined(TIM3_IRQn)
        case hwTimer_Index_2:
            HAL_NVIC_SetPriority(TIM3_IRQn, TIMER_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(TIM3_IRQn);
            break;
#endif

#if defined(TIM14_IRQn)
        case hwTimer_Index_13:
            HAL_NVIC_SetPriority(TIM14_IRQn, TIMER_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(TIM14_IRQn);
            break;
#endif
    }
}

void Timer_NVIC_Disable(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_UP_IRQn)
        case hwTimer_Index_0:
            HAL_NVIC_DisableIRQ(TIM1_UP_IRQn);
            break;
#endif

#if defined(TIM2_IRQn)
        case hwTimer_Index_1:
            HAL_NVIC_DisableIRQ(TIM2_IRQn);
            break;
#endif

#if defined(TIM3_IRQn)
        case hwTimer_Index_2:
            HAL_NVIC_DisableIRQ(TIM3_IRQn);
            break;
#endif

#if defined(TIM14_IRQn)
        case hwTimer_Index_13:
            HAL_NVIC_DisableIRQ(TIM14_IRQn);
            break;
#endif
    }
}

#endif // STM32C0