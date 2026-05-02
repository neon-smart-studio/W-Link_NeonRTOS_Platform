#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef STM32L5

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
#if defined(TIM4_BASE)
        case hwTimer_Index_3: return TIM4;
#endif
#if defined(TIM5_BASE)
        case hwTimer_Index_4: return TIM5;
#endif
#if defined(TIM6_BASE)
        case hwTimer_Index_5: return TIM6;
#endif
#if defined(TIM7_BASE)
        case hwTimer_Index_6: return TIM7;
#endif
#if defined(TIM8_BASE)
        case hwTimer_Index_7: return TIM8;
#endif
#if defined(TIM15_BASE)
        case hwTimer_Index_14: return TIM15;
#endif
#if defined(TIM16_BASE)
        case hwTimer_Index_15: return TIM16;
#endif
#if defined(TIM17_BASE)
        case hwTimer_Index_16: return TIM17;
#endif
        default: break;
    }

    return NULL;
}

static uint32_t Timer_GetAPB1TimerClock(void)
{
    uint32_t pclk = HAL_RCC_GetPCLK1Freq();

#if defined(RCC_CFGR_PPRE1)
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0U)
        pclk *= 2U;
#endif

    return pclk;
}

static uint32_t Timer_GetAPB2TimerClock(void)
{
    uint32_t pclk = HAL_RCC_GetPCLK2Freq();

#if defined(RCC_CFGR_PPRE2)
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != 0U)
        pclk *= 2U;
#endif

    return pclk;
}

static uint32_t Timer_GetInputClock(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
#endif
#if defined(TIM8_BASE)
        case hwTimer_Index_7:
#endif
#if defined(TIM15_BASE)
        case hwTimer_Index_14:
#endif
#if defined(TIM16_BASE)
        case hwTimer_Index_15:
#endif
#if defined(TIM17_BASE)
        case hwTimer_Index_16:
#endif
            return Timer_GetAPB2TimerClock();

        default:
            return Timer_GetAPB1TimerClock();
    }
}

static void Timer_HAL_IRQHandler(hwTimer_Index index)
{
    if (index < hwTimer_Index_MAX)
        HAL_TIM_IRQHandler(&g_timer[index]);
}

/* ================= IRQ Handlers ================= */

#if defined(TIM1_BASE)
void TIM1_UP_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_0); }
#endif

#if defined(TIM2_BASE)
void TIM2_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_1); }
#endif

#if defined(TIM3_BASE)
void TIM3_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_2); }
#endif

#if defined(TIM4_BASE)
void TIM4_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_3); }
#endif

#if defined(TIM5_BASE)
void TIM5_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_4); }
#endif

#if defined(TIM6_BASE)
void TIM6_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_5); }
#endif

#if defined(TIM7_BASE)
void TIM7_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_6); }
#endif

#if defined(TIM8_BASE)
void TIM8_UP_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_7); }
#endif

#if defined(TIM15_BASE)
void TIM15_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_14); }
#endif

#if defined(TIM16_IRQn)
void TIM16_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_15); }
#endif

#if defined(TIM17_IRQn)
void TIM17_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_16); }
#endif

/* ================= Clock ================= */

static void Timer_Enable_Clock(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0:  __HAL_RCC_TIM1_CLK_ENABLE(); break;
#endif
#if defined(TIM2_BASE)
        case hwTimer_Index_1:  __HAL_RCC_TIM2_CLK_ENABLE(); break;
#endif
#if defined(TIM3_BASE)
        case hwTimer_Index_2:  __HAL_RCC_TIM3_CLK_ENABLE(); break;
#endif
#if defined(TIM4_BASE)
        case hwTimer_Index_3:  __HAL_RCC_TIM4_CLK_ENABLE(); break;
#endif
#if defined(TIM5_BASE)
        case hwTimer_Index_4:  __HAL_RCC_TIM5_CLK_ENABLE(); break;
#endif
#if defined(TIM6_BASE)
        case hwTimer_Index_5:  __HAL_RCC_TIM6_CLK_ENABLE(); break;
#endif
#if defined(TIM7_BASE)
        case hwTimer_Index_6:  __HAL_RCC_TIM7_CLK_ENABLE(); break;
#endif
#if defined(TIM8_BASE)
        case hwTimer_Index_7:  __HAL_RCC_TIM8_CLK_ENABLE(); break;
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
        default: break;
    }
}

static void Timer_Disable_Clock(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0:  __HAL_RCC_TIM1_CLK_DISABLE(); break;
#endif
#if defined(TIM2_BASE)
        case hwTimer_Index_1:  __HAL_RCC_TIM2_CLK_DISABLE(); break;
#endif
#if defined(TIM3_BASE)
        case hwTimer_Index_2:  __HAL_RCC_TIM3_CLK_DISABLE(); break;
#endif
#if defined(TIM4_BASE)
        case hwTimer_Index_3:  __HAL_RCC_TIM4_CLK_DISABLE(); break;
#endif
#if defined(TIM5_BASE)
        case hwTimer_Index_4:  __HAL_RCC_TIM5_CLK_DISABLE(); break;
#endif
#if defined(TIM6_BASE)
        case hwTimer_Index_5:  __HAL_RCC_TIM6_CLK_DISABLE(); break;
#endif
#if defined(TIM7_BASE)
        case hwTimer_Index_6:  __HAL_RCC_TIM7_CLK_DISABLE(); break;
#endif
#if defined(TIM8_BASE)
        case hwTimer_Index_7:  __HAL_RCC_TIM8_CLK_DISABLE(); break;
#endif
#if defined(TIM15_BASE)
        case hwTimer_Index_14: __HAL_RCC_TIM15_CLK_DISABLE(); break;
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

/* ================= Init / DeInit ================= */

hwTimer_OpResult Timer_Instance_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
        return hwTimer_InvalidParameter;

    TIM_TypeDef *timer_soc_base = Timer_Map_Soc_Base(index);
    if (timer_soc_base == NULL)
        return hwTimer_InvalidParameter;

    Timer_Enable_Clock(index);

    uint32_t input_clk = Timer_GetInputClock(index);
    if (input_clk == 0U)
        return hwTimer_HwError;

    g_timer[index].Instance = timer_soc_base;
    g_timer[index].Init.Prescaler         = (input_clk / 1000000U) - 1U;
    g_timer[index].Init.CounterMode       = TIM_COUNTERMODE_UP;
    g_timer[index].Init.Period            = 1000U - 1U;
    g_timer[index].Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    g_timer[index].Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&g_timer[index]) != HAL_OK)
        return hwTimer_HwError;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Instance_DeInit(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
        return hwTimer_InvalidParameter;

    if (Timer_Map_Soc_Base(index) == NULL)
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
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
            HAL_NVIC_SetPriority(TIM1_UP_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
            break;
#endif

#if defined(TIM2_BASE)
        case hwTimer_Index_1:
            HAL_NVIC_SetPriority(TIM2_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM2_IRQn);
            break;
#endif

#if defined(TIM3_BASE)
        case hwTimer_Index_2:
            HAL_NVIC_SetPriority(TIM3_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM3_IRQn);
            break;
#endif

#if defined(TIM4_BASE)
        case hwTimer_Index_3:
            HAL_NVIC_SetPriority(TIM4_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM4_IRQn);
            break;
#endif

#if defined(TIM5_BASE)
        case hwTimer_Index_4:
            HAL_NVIC_SetPriority(TIM5_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM5_IRQn);
            break;
#endif

#if defined(TIM6_BASE)
        case hwTimer_Index_5:
            HAL_NVIC_SetPriority(TIM6_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM6_IRQn);
            break;
#endif

#if defined(TIM7_BASE)
        case hwTimer_Index_6:
            HAL_NVIC_SetPriority(TIM7_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM7_IRQn);
            break;
#endif

#if defined(TIM8_BASE)
        case hwTimer_Index_7:
            HAL_NVIC_SetPriority(TIM8_UP_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM8_UP_IRQn);
            break;
#endif

#if defined(TIM15_BASE)
        case hwTimer_Index_14:
            HAL_NVIC_SetPriority(TIM15_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM15_IRQn);
            break;
#endif

#if defined(TIM16_BASE)
        case hwTimer_Index_15:
            HAL_NVIC_SetPriority(TIM16_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM16_IRQn);
            break;
#endif

#if defined(TIM17_BASE)
        case hwTimer_Index_16:
            HAL_NVIC_SetPriority(TIM17_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM17_IRQn);
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
        case hwTimer_Index_0:
            HAL_NVIC_DisableIRQ(TIM1_UP_IRQn);
            break;
#endif

#if defined(TIM2_BASE)
        case hwTimer_Index_1:
            HAL_NVIC_DisableIRQ(TIM2_IRQn);
            break;
#endif

#if defined(TIM3_BASE)
        case hwTimer_Index_2:
            HAL_NVIC_DisableIRQ(TIM3_IRQn);
            break;
#endif

#if defined(TIM4_BASE)
        case hwTimer_Index_3:
            HAL_NVIC_DisableIRQ(TIM4_IRQn);
            break;
#endif

#if defined(TIM5_BASE)
        case hwTimer_Index_4:
            HAL_NVIC_DisableIRQ(TIM5_IRQn);
            break;
#endif

#if defined(TIM6_BASE)
        case hwTimer_Index_5:
            HAL_NVIC_DisableIRQ(TIM6_IRQn);
            break;
#endif

#if defined(TIM7_BASE)
        case hwTimer_Index_6:
            HAL_NVIC_DisableIRQ(TIM7_IRQn);
            break;
#endif

#if defined(TIM8_BASE)
        case hwTimer_Index_7:
            HAL_NVIC_DisableIRQ(TIM8_UP_IRQn);
            break;
#endif

#if defined(TIM15_BASE)
        case hwTimer_Index_14:
            HAL_NVIC_DisableIRQ(TIM15_IRQn);
            break;
#endif

#if defined(TIM16_BASE)
        case hwTimer_Index_15:
            HAL_NVIC_DisableIRQ(TIM16_IRQn);
            break;
#endif

#if defined(TIM17_BASE)
        case hwTimer_Index_16:
            HAL_NVIC_DisableIRQ(TIM17_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif // STM32L5