#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef STM32F2

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
#if defined(TIM9_BASE)
        case hwTimer_Index_8: return TIM9;
#endif
#if defined(TIM10_BASE)
        case hwTimer_Index_9: return TIM10;
#endif
#if defined(TIM11_BASE)
        case hwTimer_Index_10: return TIM11;
#endif
#if defined(TIM12_BASE)
        case hwTimer_Index_11: return TIM12;
#endif
#if defined(TIM13_BASE)
        case hwTimer_Index_12: return TIM13;
#endif
#if defined(TIM14_BASE)
        case hwTimer_Index_13: return TIM14;
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
#if defined(TIM1_BASE)
        case hwTimer_Index_0:   // TIM1
#endif
#if defined(TIM8_BASE)
        case hwTimer_Index_7:   // TIM8
#endif
#if defined(TIM9_BASE)
        case hwTimer_Index_8:   // TIM9
#endif
#if defined(TIM10_BASE)
        case hwTimer_Index_9:   // TIM10
#endif
#if defined(TIM11_BASE)
        case hwTimer_Index_10:  // TIM11
#endif
            return GetAPB2TimerClock();

        default:
            return GetAPB1TimerClock();
    }
}

static void Timer_HAL_IRQHandler(hwTimer_Index index)
{
        HAL_TIM_IRQHandler(&g_timer[index]);
}

/* ================= IRQ Handlers ================= */

#if defined(TIM1_BASE) || defined(TIM10_BASE)
void TIM1_UP_TIM10_IRQHandler(void)
{
#if defined(TIM1_BASE)
    Timer_HAL_IRQHandler(hwTimer_Index_0);
#endif
#if defined(TIM10_BASE)
    Timer_HAL_IRQHandler(hwTimer_Index_9);
#endif
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

#if defined(TIM4_BASE)
void TIM4_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_3);
}
#endif

#if defined(TIM5_BASE)
void TIM5_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_4);
}
#endif

#if defined(TIM6_BASE)
void TIM6_DAC_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_5);
}
#endif

#if defined(TIM7_BASE)
void TIM7_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_6);
}
#endif

#if defined(TIM8_BASE) || defined(TIM13_BASE)
void TIM8_UP_TIM13_IRQHandler(void)
{
#if defined(TIM8_BASE)
    Timer_HAL_IRQHandler(hwTimer_Index_7);
#endif
#if defined(TIM13_BASE)
    Timer_HAL_IRQHandler(hwTimer_Index_12);
#endif
}
#endif

#if defined(TIM9_BASE)
void TIM1_BRK_TIM9_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_8);
}
#endif

#if defined(TIM11_BASE)
void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_10);
}
#endif

#if defined(TIM12_BASE)
void TIM8_BRK_TIM12_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_11);
}
#endif

#if defined(TIM14_BASE)
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_13);
}
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

#if defined(TIM5_BASE)
                case hwTimer_Index_4:
                        __HAL_RCC_TIM5_CLK_ENABLE();
                        break;
#endif

#if defined(TIM8_BASE)
                case hwTimer_Index_7:
                        __HAL_RCC_TIM8_CLK_ENABLE();
                        break;
#endif

#if defined(TIM9_BASE)
                case hwTimer_Index_8:
                        __HAL_RCC_TIM9_CLK_ENABLE();
                        break;
#endif

#if defined(TIM10_BASE)
                case hwTimer_Index_9:
                        __HAL_RCC_TIM10_CLK_ENABLE();
                        break;
#endif

#if defined(TIM11_BASE)
                case hwTimer_Index_10:
                        __HAL_RCC_TIM11_CLK_ENABLE();
                        break;
#endif

#if defined(TIM12_BASE)
                case hwTimer_Index_11:
                        __HAL_RCC_TIM12_CLK_ENABLE();
                        break;
#endif

#if defined(TIM13_BASE)
                case hwTimer_Index_12:
                        __HAL_RCC_TIM13_CLK_ENABLE();
                        break;
#endif

#if defined(TIM14_BASE)
                case hwTimer_Index_13:
                        __HAL_RCC_TIM14_CLK_ENABLE();
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

#if defined(TIM5_BASE)
                case hwTimer_Index_4:
                        __HAL_RCC_TIM5_CLK_DISABLE();
                        break;
#endif

#if defined(TIM8_BASE)
                case hwTimer_Index_7:
                        __HAL_RCC_TIM8_CLK_DISABLE();
                        break;
#endif

#if defined(TIM9_BASE)
                case hwTimer_Index_8:
                        __HAL_RCC_TIM9_CLK_DISABLE();
                        break;
#endif

#if defined(TIM10_BASE)
                case hwTimer_Index_9:
                        __HAL_RCC_TIM10_CLK_DISABLE();
                        break;
#endif

#if defined(TIM11_BASE)
                case hwTimer_Index_10:
                        __HAL_RCC_TIM11_CLK_DISABLE();
                        break;
#endif

#if defined(TIM12_BASE)
                case hwTimer_Index_11:
                        __HAL_RCC_TIM12_CLK_DISABLE();
                        break;
#endif

#if defined(TIM13_BASE)
                case hwTimer_Index_12:
                        __HAL_RCC_TIM13_CLK_DISABLE();
                        break;
#endif

#if defined(TIM14_BASE)
                case hwTimer_Index_13:
                        __HAL_RCC_TIM14_CLK_DISABLE();
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
        g_timer[index].Init.Prescaler = (Timer_GetInputClock(index) / 1000000) - 1; // 1 MHz = 1us
        g_timer[index].Init.CounterMode = TIM_COUNTERMODE_UP;
        g_timer[index].Init.Period = 1000 - 1; // 預設 1ms
        g_timer[index].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
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
#if defined(TIM1_BASE) || defined(TIM10_BASE)
                /* TIM1 / TIM10 */
                case hwTimer_Index_0:
                case hwTimer_Index_9:
                        HAL_NVIC_SetPriority(
                                TIM1_UP_TIM10_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
                        break;
#endif

#if defined(TIM2_BASE)
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
                case hwTimer_Index_3:
                        HAL_NVIC_SetPriority(
                                TIM4_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM4_IRQn);
                        break;
#endif

#if defined(TIM5_BASE)
                case hwTimer_Index_4:
                        HAL_NVIC_SetPriority(
                                TIM5_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM5_IRQn);
                        break;
#endif

#if defined(TIM6_BASE)
                case hwTimer_Index_5:
#if defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || defined (STM32F412Zx)
                        HAL_NVIC_SetPriority(
                                TIM6_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM6_IRQn);
#else
                        HAL_NVIC_SetPriority(
                                TIM6_DAC_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
#endif
                        break;
#endif

#if defined(TIM7_BASE)
                case hwTimer_Index_6:
                        HAL_NVIC_SetPriority(
                                TIM7_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM7_IRQn);
                        break;
#endif

#if defined(TIM8_BASE) || defined(TIM13_BASE)
                /* TIM8 / TIM13 */
                case hwTimer_Index_7:
                case hwTimer_Index_12:
                        HAL_NVIC_SetPriority(
                                TIM8_UP_TIM13_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
                        break;
#endif

#if defined(TIM9_BASE)
                case hwTimer_Index_8:
                        HAL_NVIC_SetPriority(
                                TIM1_BRK_TIM9_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
                        break;
#endif

#if defined(TIM11_BASE)
                case hwTimer_Index_10:
                        HAL_NVIC_SetPriority(
                                TIM1_TRG_COM_TIM11_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
                        break;
#endif

#if defined(TIM12_BASE)
                case hwTimer_Index_11:
                        HAL_NVIC_SetPriority(
                                TIM8_BRK_TIM12_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
                        break;
#endif

#if defined(TIM14_BASE)
                case hwTimer_Index_13:
                        HAL_NVIC_SetPriority(
                                TIM8_TRG_COM_TIM14_IRQn,
                                TIMER_IRQ_NVIC_PRIORITY,
                                TIMER_IRQ_NVIC_SUB_PRIORITY
                        );
                        HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
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
#if defined(TIM1_BASE) || defined(TIM10_BASE)
                case hwTimer_Index_0:
                case hwTimer_Index_9:
#if defined(TIM1_BASE) && defined(TIM10_BASE)
                        if (!Timer_Init_Status[hwTimer_Index_0] && !Timer_Init_Status[hwTimer_Index_9])
#endif
                        {
                                HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
                        }
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
#if defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || defined (STM32F412Zx)
                        HAL_NVIC_DisableIRQ(TIM6_IRQn);
#else
                        HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
#endif
                        break;
#endif

#if defined(TIM7_BASE)
                case hwTimer_Index_6:
                        HAL_NVIC_DisableIRQ(TIM7_IRQn);
                        break;
#endif

#if defined(TIM8_BASE) || defined(TIM13_BASE)
                case hwTimer_Index_7:
                case hwTimer_Index_12:
#if defined(TIM8_BASE) && defined(TIM13_BASE)
                        if (!Timer_Init_Status[hwTimer_Index_7] && !Timer_Init_Status[hwTimer_Index_12])
#endif
                        {
                                HAL_NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn);
                        }
                        break;
#endif

#if defined(TIM9_BASE)
                case hwTimer_Index_8:
                        HAL_NVIC_DisableIRQ(TIM1_BRK_TIM9_IRQn);
                        break;
#endif

#if defined(TIM11_BASE)
                case hwTimer_Index_10:
                        HAL_NVIC_DisableIRQ(TIM1_TRG_COM_TIM11_IRQn);
                        break;
#endif

#if defined(TIM12_BASE)
                case hwTimer_Index_11:
                        HAL_NVIC_DisableIRQ(TIM8_BRK_TIM12_IRQn);
                        break;
#endif

#if defined(TIM14_BASE)
                case hwTimer_Index_13:
                        HAL_NVIC_DisableIRQ(TIM8_TRG_COM_TIM14_IRQn);
                        break;
#endif

                default:
                        break;
        }
}

#endif // STM32F2
