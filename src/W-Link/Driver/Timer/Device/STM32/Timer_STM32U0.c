#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef STM32U0

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
#if defined(TIM6_BASE)
        case hwTimer_Index_5: return TIM6;
#endif
#if defined(TIM7_BASE)
        case hwTimer_Index_6: return TIM7;
#endif
#if defined(TIM15_BASE)
        case hwTimer_Index_14: return TIM15;
#endif
#if defined(TIM16_BASE)
        case hwTimer_Index_15: return TIM16;
#endif
        default:
            break;
    }

    return NULL;
}

static uint32_t Timer_GetAPB1TimerClock(void)
{
    uint32_t pclk = HAL_RCC_GetPCLK1Freq();

#if defined(RCC_CFGR_PPRE1) && defined(RCC_CFGR_PPRE1_DIV1)
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
        pclk *= 2U;
#elif defined(RCC_CFGR_PPRE1)
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0U)
        pclk *= 2U;
#endif

    return pclk;
}

static uint32_t Timer_GetAPB2TimerClock(void)
{
#if defined(RCC_CFGR_PPRE2)
    uint32_t pclk = HAL_RCC_GetPCLK2Freq();

#if defined(RCC_CFGR_PPRE2_DIV1)
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1)
        pclk *= 2U;
#else
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != 0U)
        pclk *= 2U;
#endif

    return pclk;
#else
    return Timer_GetAPB1TimerClock();
#endif
}

static uint32_t Timer_GetInputClock(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
            return Timer_GetAPB2TimerClock();
#endif

#if defined(TIM15_BASE)
        case hwTimer_Index_14:
            return Timer_GetAPB2TimerClock();
#endif

#if defined(TIM16_BASE)
        case hwTimer_Index_15:
            return Timer_GetAPB2TimerClock();
#endif

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
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
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

#if defined(TIM6_BASE)
void TIM6_DAC_LPTIM1_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_5);
}
#endif

#if defined(TIM7_BASE)
void TIM7_LPTIM2_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_6);
}
#endif

#if defined(TIM15_BASE)
#if defined (STM32U073xx) || defined (STM32U083xx)
void TIM15_LPTIM3_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_14);
}
#else
void TIM15_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_14);
}
#endif
#endif

#if defined(TIM16_BASE)
void TIM16_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_15);
}
#endif

/* ================= Clock ================= */

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

#if defined(TIM6_BASE)
        case hwTimer_Index_5:
            __HAL_RCC_TIM6_CLK_ENABLE();
            break;
#endif

#if defined(TIM7_BASE)
        case hwTimer_Index_6:
            __HAL_RCC_TIM7_CLK_ENABLE();
            break;
#endif

#if defined(TIM15_BASE)
        case hwTimer_Index_14:
            __HAL_RCC_TIM15_CLK_ENABLE();
            break;
#endif

#if defined(TIM16_BASE)
        case hwTimer_Index_15:
            __HAL_RCC_TIM16_CLK_ENABLE();
            break;
#endif

        default:
            break;
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

#if defined(TIM6_BASE)
        case hwTimer_Index_5:
            __HAL_RCC_TIM6_CLK_DISABLE();
            break;
#endif

#if defined(TIM7_BASE)
        case hwTimer_Index_6:
            __HAL_RCC_TIM7_CLK_DISABLE();
            break;
#endif

#if defined(TIM15_BASE)
        case hwTimer_Index_14:
            __HAL_RCC_TIM15_CLK_DISABLE();
            break;
#endif

#if defined(TIM16_BASE)
        case hwTimer_Index_15:
            __HAL_RCC_TIM16_CLK_DISABLE();
            break;
#endif

        default:
            break;
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
    {
        Timer_Disable_Clock(index);
        return hwTimer_HwError;
    }

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

/* ================= NVIC ================= */

void Timer_NVIC_Enable(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM1_BASE)
        case hwTimer_Index_0:
            HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
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

#if defined(TIM6_BASE)
        case hwTimer_Index_5:
            HAL_NVIC_SetPriority(TIM6_DAC_LPTIM1_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn);
            break;
#endif

#if defined(TIM7_BASE)
        case hwTimer_Index_6:
            HAL_NVIC_SetPriority(TIM7_LPTIM2_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM7_LPTIM2_IRQn);
            break;
#endif

#if defined(TIM15_BASE)
        case hwTimer_Index_14:
#if defined (STM32U073xx) || defined (STM32U083xx)
            HAL_NVIC_SetPriority(TIM15_LPTIM3_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM15_LPTIM3_IRQn);
#else
            HAL_NVIC_SetPriority(TIM15_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM15_IRQn);
#endif
            break;
#endif

#if defined(TIM16_BASE)
        case hwTimer_Index_15:
            HAL_NVIC_SetPriority(TIM16_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM16_IRQn);
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
            HAL_NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
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

#if defined(TIM6_BASE)
        case hwTimer_Index_5:
            HAL_NVIC_DisableIRQ(TIM6_DAC_LPTIM1_IRQn);
            break;
#endif

#if defined(TIM7_BASE)
        case hwTimer_Index_6:
            HAL_NVIC_DisableIRQ(TIM7_LPTIM2_IRQn);
            break;
#endif

#if defined(TIM15_BASE)
        case hwTimer_Index_14:
#if defined (STM32U073xx) || defined (STM32U083xx)
            HAL_NVIC_DisableIRQ(TIM15_LPTIM3_IRQn);
#else
            HAL_NVIC_DisableIRQ(TIM15_IRQn);
#endif
            break;
#endif

#if defined(TIM16_BASE)
        case hwTimer_Index_15:
            HAL_NVIC_DisableIRQ(TIM16_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif // STM32U0