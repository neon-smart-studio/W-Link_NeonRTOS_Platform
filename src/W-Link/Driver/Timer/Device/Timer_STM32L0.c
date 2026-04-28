#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef STM32L0

#include "Timer_STM32.h"

TIM_TypeDef *Timer_Map_Soc_Base(hwTimer_Index index)
{
    switch (index)
    {
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
#if defined(TIM21_BASE)
        case hwTimer_Index_20: return TIM21;
#endif
#if defined(TIM22_BASE)
        case hwTimer_Index_21: return TIM22;
#endif
        default:
            break;
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
#if defined(TIM21_BASE)
        case hwTimer_Index_20:
#endif
#if defined(TIM22_BASE)
        case hwTimer_Index_21:
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

#if defined(TIM2_BASE)
void TIM2_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_1); }
#endif

#if defined(TIM3_BASE)
void TIM3_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_2); }
#endif

#if defined(TIM6_BASE)
#if defined (STM32L052xx) || defined (STM32L053xx) || defined (STM32L062xx) || defined (STM32L063xx) || \
    defined (STM32L072xx) || defined (STM32L073xx) || defined (STM32L082xx) || defined (STM32L083xx)
void TIM6_DAC_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_5); }
#else
void TIM6_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_5); }
#endif
#endif

#if defined(TIM7_BASE)
void TIM7_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_6); }
#endif

#if defined(TIM21_BASE)
void TIM21_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_20); }
#endif

#if defined(TIM22_BASE)
void TIM22_IRQHandler(void) { Timer_HAL_IRQHandler(hwTimer_Index_21); }
#endif

/* ================= Clock ================= */

static void Timer_Enable_Clock(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM2_BASE)
        case hwTimer_Index_1:  __HAL_RCC_TIM2_CLK_ENABLE();  break;
#endif
#if defined(TIM3_BASE)
        case hwTimer_Index_2:  __HAL_RCC_TIM3_CLK_ENABLE();  break;
#endif
#if defined(TIM6_BASE)
        case hwTimer_Index_5:  __HAL_RCC_TIM6_CLK_ENABLE();  break;
#endif
#if defined(TIM7_BASE)
        case hwTimer_Index_6:  __HAL_RCC_TIM7_CLK_ENABLE();  break;
#endif
#if defined(TIM21_BASE)
        case hwTimer_Index_20: __HAL_RCC_TIM21_CLK_ENABLE(); break;
#endif
#if defined(TIM22_BASE)
        case hwTimer_Index_21: __HAL_RCC_TIM22_CLK_ENABLE(); break;
#endif
        default:
            break;
    }
}

static void Timer_Disable_Clock(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIM2_BASE)
        case hwTimer_Index_1:  __HAL_RCC_TIM2_CLK_DISABLE();  break;
#endif
#if defined(TIM3_BASE)
        case hwTimer_Index_2:  __HAL_RCC_TIM3_CLK_DISABLE();  break;
#endif
#if defined(TIM6_BASE)
        case hwTimer_Index_5:  __HAL_RCC_TIM6_CLK_DISABLE();  break;
#endif
#if defined(TIM7_BASE)
        case hwTimer_Index_6:  __HAL_RCC_TIM7_CLK_DISABLE();  break;
#endif
#if defined(TIM21_BASE)
        case hwTimer_Index_20: __HAL_RCC_TIM21_CLK_DISABLE(); break;
#endif
#if defined(TIM22_BASE)
        case hwTimer_Index_21: __HAL_RCC_TIM22_CLK_DISABLE(); break;
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

/* ================= NVIC ================= */

void Timer_NVIC_Enable(hwTimer_Index index)
{
    switch (index)
    {
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
#if defined (STM32L052xx) || defined (STM32L053xx) || defined (STM32L062xx) || defined (STM32L063xx) || \
    defined (STM32L072xx) || defined (STM32L073xx) || defined (STM32L082xx) || defined (STM32L083xx)
        case hwTimer_Index_5:
            HAL_NVIC_SetPriority(TIM6_DAC_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
            break;
#else
        case hwTimer_Index_5:
            HAL_NVIC_SetPriority(TIM6_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM6_IRQn);
            break;
#endif
#endif

#if defined(TIM7_BASE)
        case hwTimer_Index_6:
            HAL_NVIC_SetPriority(TIM7_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM7_IRQn);
            break;
#endif

#if defined(TIM21_BASE)
        case hwTimer_Index_20:
            HAL_NVIC_SetPriority(TIM21_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM21_IRQn);
            break;
#endif

#if defined(TIM22_BASE)
        case hwTimer_Index_21:
            HAL_NVIC_SetPriority(TIM22_IRQn, TIMER_IRQ_NVIC_PRIORITY, TIMER_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(TIM22_IRQn);
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
#if defined (STM32L052xx) || defined (STM32L053xx) || defined (STM32L062xx) || defined (STM32L063xx) || \
    defined (STM32L072xx) || defined (STM32L073xx) || defined (STM32L082xx) || defined (STM32L083xx)
        case hwTimer_Index_5:
            HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
            break;
#else
        case hwTimer_Index_5:
            HAL_NVIC_DisableIRQ(TIM6_IRQn);
            break;
#endif
#endif

#if defined(TIM7_BASE)
        case hwTimer_Index_6:
            HAL_NVIC_DisableIRQ(TIM7_IRQn);
            break;
#endif

#if defined(TIM21_BASE)
        case hwTimer_Index_20:
            HAL_NVIC_DisableIRQ(TIM21_IRQn);
            break;
#endif

#if defined(TIM22_BASE)
        case hwTimer_Index_21:
            HAL_NVIC_DisableIRQ(TIM22_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif // STM32L0