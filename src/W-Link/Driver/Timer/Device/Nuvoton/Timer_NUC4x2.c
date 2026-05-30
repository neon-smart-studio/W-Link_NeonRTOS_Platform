#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef NUC472

#include "Timer_Nuvoton.h"

TIMER_T *Timer_Map_Soc_Base(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIMER0_BASE)
        case hwTimer_Index_0: return TIMER0;
#endif
#if defined(TIMER1_BASE)
        case hwTimer_Index_1: return TIMER1;
#endif
#if defined(TIMER2_BASE)
        case hwTimer_Index_2: return TIMER2;
#endif
#if defined(TIMER3_BASE)
        case hwTimer_Index_3: return TIMER3;
#endif
        default: return NULL;
    }
}

static void Timer_HAL_IRQHandler(hwTimer_Index index)
{
    TIMER_T *timer = Timer_Map_Soc_Base(index);

    if (timer == NULL)
        return;

    if (TIMER_GetIntFlag(timer))
    {
        TIMER_ClearIntFlag(timer);
        Timer_PeriodElapsedCallback(index);
    }
}

#if defined(TIMER0_BASE)
void TMR0_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_0);
}
#endif

#if defined(TIMER1_BASE)
void TMR1_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_1);
}
#endif

#if defined(TIMER2_BASE)
void TMR2_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_2);
}
#endif

#if defined(TIMER3_BASE)
void TMR3_IRQHandler(void)
{
    Timer_HAL_IRQHandler(hwTimer_Index_3);
}
#endif

static void Timer_Enable_Clock(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIMER0_BASE)
        case hwTimer_Index_0:
            CLK_EnableModuleClock(TMR0_MODULE);
            CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_PCLK, 0);
            break;
#endif

#if defined(TIMER1_BASE)
        case hwTimer_Index_1:
            CLK_EnableModuleClock(TMR1_MODULE);
            CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_PCLK, 0);
            break;
#endif

#if defined(TIMER2_BASE)
        case hwTimer_Index_2:
            CLK_EnableModuleClock(TMR2_MODULE);
            CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_PCLK, 0);
            break;
#endif

#if defined(TIMER3_BASE)
        case hwTimer_Index_3:
            CLK_EnableModuleClock(TMR3_MODULE);
            CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_PCLK, 0);
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
#if defined(TIMER0_BASE)
        case hwTimer_Index_0:
            CLK_DisableModuleClock(TMR0_MODULE);
            break;
#endif

#if defined(TIMER1_BASE)
        case hwTimer_Index_1:
            CLK_DisableModuleClock(TMR1_MODULE);
            break;
#endif

#if defined(TIMER2_BASE)
        case hwTimer_Index_2:
            CLK_DisableModuleClock(TMR2_MODULE);
            break;
#endif

#if defined(TIMER3_BASE)
        case hwTimer_Index_3:
            CLK_DisableModuleClock(TMR3_MODULE);
            break;
#endif

        default:
            break;
    }
}

hwTimer_OpResult Timer_Instance_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
        return hwTimer_InvalidParameter;

    TIMER_T *timer = Timer_Map_Soc_Base(index);

    if (timer == NULL)
        return hwTimer_InvalidParameter;

    Timer_Enable_Clock(index);

    TIMER_EnableInt(timer);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Instance_DeInit(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
        return hwTimer_InvalidParameter;

    TIMER_T *timer = Timer_Map_Soc_Base(index);

    if (timer == NULL)
        return hwTimer_InvalidParameter;

    TIMER_DisableInt(timer);
    TIMER_Stop(timer);
    TIMER_Close(timer);

    Timer_Disable_Clock(index);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Instance_Start(hwTimer_Index index, uint32_t duration_us)
{
    if (index >= hwTimer_Index_MAX)
        return hwTimer_InvalidParameter;

    TIMER_T *timer = Timer_Map_Soc_Base(index);

    if (timer == NULL)
        return hwTimer_InvalidParameter;

    uint32_t freq = 1000000UL / duration_us;

    if (freq == 0)
        freq = 1;

    TIMER_Open(timer, TIMER_PERIODIC_MODE, freq);

    TIMER_Start(timer);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Instance_Stop(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
        return hwTimer_InvalidParameter;

    TIMER_T *timer = Timer_Map_Soc_Base(index);

    if (timer == NULL)
        return hwTimer_InvalidParameter;

    TIMER_Stop(timer);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Instance_Read_Ticks(hwTimer_Index index, uint32_t *ticks)
{
    if (index >= hwTimer_Index_MAX)
        return hwTimer_InvalidParameter;

    if (ticks == NULL)
        return hwTimer_InvalidParameter;

    TIMER_T *timer = Timer_Map_Soc_Base(index);

    if (timer == NULL)
        return hwTimer_InvalidParameter;

    *ticks = TIMER_GetCounter(timer);

    return hwTimer_OK;
}

void Timer_NVIC_Enable(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIMER0_BASE)
        case hwTimer_Index_0:
            NVIC_SetPriority(TMR0_IRQn, TIMER_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(TMR0_IRQn);
            break;
#endif

#if defined(TIMER1_BASE)
        case hwTimer_Index_1:
            NVIC_SetPriority(TMR1_IRQn, TIMER_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(TMR1_IRQn);
            break;
#endif

#if defined(TIMER2_BASE)
        case hwTimer_Index_2:
            NVIC_SetPriority(TMR2_IRQn, TIMER_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(TMR2_IRQn);
            break;
#endif

#if defined(TIMER3_BASE)
        case hwTimer_Index_3:
            NVIC_SetPriority(TMR3_IRQn, TIMER_IRQ_NVIC_PRIORITY);
            NVIC_EnableIRQ(TMR3_IRQn);
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
#if defined(TIMER0_BASE)
        case hwTimer_Index_0:
            NVIC_DisableIRQ(TMR0_IRQn);
            break;
#endif

#if defined(TIMER1_BASE)
        case hwTimer_Index_1:
            NVIC_DisableIRQ(TMR1_IRQn);
            break;
#endif

#if defined(TIMER2_BASE)
        case hwTimer_Index_2:
            NVIC_DisableIRQ(TMR2_IRQn);
            break;
#endif

#if defined(TIMER3_BASE)
        case hwTimer_Index_3:
            NVIC_DisableIRQ(TMR3_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif /* NUC472 */