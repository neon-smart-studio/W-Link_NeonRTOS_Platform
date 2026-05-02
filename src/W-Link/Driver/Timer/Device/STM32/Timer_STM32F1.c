#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef STM32F1

#include "Timer_STM32.h"

TIM_TypeDef * Timer_Map_Soc_Base(hwTimer_Index index)
{
    switch(index)
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
#if defined(TIM4_BASE)
        case hwTimer_Index_3: return TIM4;
#endif
        default: break;
    }
    return NULL;
}

static uint32_t GetAPB1TimerClock(void)
{
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();

    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
        return pclk1 * 2;

    return pclk1;
}

static uint32_t GetAPB2TimerClock(void)
{
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();

    if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1)
        return pclk2 * 2;

    return pclk2;
}

static uint32_t Timer_GetInputClock(hwTimer_Index index)
{
    switch (index)
    {
        /* ---------- APB2 ---------- */
#if defined(TIM1_BASE)
        case hwTimer_Index_0:   // TIM1
            return GetAPB2TimerClock();
#endif

        /* ---------- APB1 ---------- */
        default:
            return GetAPB1TimerClock();
    }
}

static void Timer_HAL_IRQHandler(hwTimer_Index index)
{
        HAL_TIM_IRQHandler(&g_timer[index]);
}

/* ================= IRQ Handlers ================= */

#if defined(TIM1_BASE)
void TIM1_UP_IRQHandler(void)            { Timer_HAL_IRQHandler(hwTimer_Index_0); }
#endif
#if defined(TIM2_BASE)
void TIM2_IRQHandler(void)               { Timer_HAL_IRQHandler(hwTimer_Index_1); }
#endif
#if defined(TIM3_BASE)
void TIM3_IRQHandler(void)               { Timer_HAL_IRQHandler(hwTimer_Index_2); }
#endif
#if defined(TIM4_BASE)
void TIM4_IRQHandler(void)               { Timer_HAL_IRQHandler(hwTimer_Index_3); }
#endif

static void Timer_Enable_Clock(hwTimer_Index index)
{
        switch (index)
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
#if defined(TIM4_BASE)
                case hwTimer_Index_3:
                        __HAL_RCC_TIM4_CLK_ENABLE();
                        break;
#endif
        }

}

static void Timer_Disable_Clock(hwTimer_Index index)
{
        switch (index)
        {
#if defined(TIM1_BASE)
                case hwTimer_Index_0:
                        __HAL_RCC_TIM1_CLK_DISABLE();
                        break;
#endif
#if defined(TIM2_BASE)
                case hwTimer_Index_1:
                        __HAL_RCC_TIM2_CLK_DISABLE();
                        break;
#endif
#if defined(TIM3_BASE)
                case hwTimer_Index_2:
                        __HAL_RCC_TIM3_CLK_DISABLE();
                        break;
#endif
#if defined(TIM4_BASE)
                case hwTimer_Index_3:
                        __HAL_RCC_TIM4_CLK_DISABLE();
                        break;
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
#if defined(TIM1_BASE)
                /* ---------- TIM1 ---------- */
                case hwTimer_Index_0:   // TIM1
                        HAL_NVIC_SetPriority(
                                TIM1_UP_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
                        break;
#endif

#if defined(TIM2_BASE)
                /* ---------- TIM2 ---------- */
                case hwTimer_Index_1:
                        HAL_NVIC_SetPriority(
                                TIM2_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM2_IRQn);
                        break;
#endif

#if defined(TIM3_BASE)
                /* ---------- TIM3 ---------- */
                case hwTimer_Index_2:
                        HAL_NVIC_SetPriority(
                                TIM3_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM3_IRQn);
                        break;
#endif

#if defined(TIM4_BASE)
                /* ---------- TIM4 ---------- */
                case hwTimer_Index_3:
                        HAL_NVIC_SetPriority(
                                TIM4_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM4_IRQn);
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
#if defined(TIM1_BASE)
                /* ---------- TIM1 ---------- */
                case hwTimer_Index_0:   // TIM1
                        if (!Timer_Init_Status[hwTimer_Index_0])
                        {
                                HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
                        }
                        break;
#endif

#if defined(TIM2_BASE)
                /* ---------- TIM2 ---------- */
                case hwTimer_Index_1:
                        HAL_NVIC_DisableIRQ(TIM2_IRQn);
                        break;
#endif

#if defined(TIM3_BASE)
                /* ---------- TIM3 ---------- */
                case hwTimer_Index_2:
                        HAL_NVIC_DisableIRQ(TIM3_IRQn);
                        break;
#endif

#if defined(TIM4_BASE)
                /* ---------- TIM4 ---------- */
                case hwTimer_Index_3:
                        HAL_NVIC_DisableIRQ(TIM4_IRQn);
                        break;
#endif

                default:
                        break;
        }
}

#endif // STM32F1
