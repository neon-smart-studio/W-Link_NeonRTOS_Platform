#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Timer/Timer.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSP432P

#include "Timer_TIMSP432.h"

static bool Timer_Init_Status[hwTimer_Index_MAX] = {false};
static bool Timer_IsPeriodic[hwTimer_Index_MAX] = {false};
static uint32_t Timer_Period_Us[hwTimer_Index_MAX] = {0};
static onTimerEventHandler Timer_Expired_Handler[hwTimer_Index_MAX] = {NULL};

static uint32_t Timer_Map_Base(hwTimer_Index index)
{
    switch (index)
    {
        case hwTimer_Index_0: return TIMER32_0_BASE;
        case hwTimer_Index_1: return TIMER32_1_BASE;
        default: return 0;
    }
}

static uint32_t Timer_Map_IRQ(hwTimer_Index index)
{
    switch (index)
    {
        case hwTimer_Index_0: return INT_T32_INT1;
        case hwTimer_Index_1: return INT_T32_INT2;
        default: return 0;
    }
}

static uint32_t Timer_Us_To_Ticks(uint32_t us)
{
    uint64_t clk = g_sys_clock_hz;
    uint64_t ticks = (clk * us) / 1000000ULL;

    if (ticks == 0)
    {
        ticks = 1;
    }

    if (ticks > 0xFFFFFFFFULL)
    {
        ticks = 0xFFFFFFFFULL;
    }

    return (uint32_t)ticks;
}

static void TIMSP432P_Timer_IRQ_Process(hwTimer_Index index)
{
    uint32_t base = Timer_Map_Base(index);

    if (base == 0)
    {
        return;
    }

    MAP_Timer32_clearInterruptFlag(base);

    if (Timer_Expired_Handler[index])
    {
        Timer_Expired_Handler[index](index);
    }

    if (!Timer_IsPeriodic[index])
    {
        MAP_Timer32_haltTimer(base);
        Timer_Period_Us[index] = 0;
    }
}

void T32_INT1_IRQHandler(void)
{
    TIMSP432P_Timer_IRQ_Process(hwTimer_Index_0);
}

void T32_INT2_IRQHandler(void)
{
    TIMSP432P_Timer_IRQ_Process(hwTimer_Index_1);
}

static void Timer_Register_IRQ(hwTimer_Index index)
{
    uint32_t irq = Timer_Map_IRQ(index);

    if (irq == 0)
    {
        return;
    }

    MAP_Interrupt_enableInterrupt(irq);
}

static void Timer_Unregister_IRQ(hwTimer_Index index)
{
    uint32_t base = Timer_Map_Base(index);
    uint32_t irq = Timer_Map_IRQ(index);

    if (base != 0)
    {
        MAP_Timer32_disableInterrupt(base);
        MAP_Timer32_clearInterruptFlag(base);
    }

    if (irq != 0)
    {
        MAP_Interrupt_disableInterrupt(irq);
    }
}

hwTimer_OpResult Timer_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index])
    {
        return hwTimer_OK;
    }

    uint32_t base = Timer_Map_Base(index);

    if (base == 0)
    {
        return hwTimer_InvalidParameter;
    }

    MAP_Timer32_haltTimer(base);
    MAP_Timer32_disableInterrupt(base);
    MAP_Timer32_clearInterruptFlag(base);

    MAP_Timer32_initModule(base,
                       TIMER32_PRESCALER_1,
                       TIMER32_32BIT,
                       TIMER32_PERIODIC_MODE);

    Timer_Register_IRQ(index);

    Timer_Init_Status[index] = true;
    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = 0;
    Timer_Expired_Handler[index] = NULL;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_DeInit(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_OK;
    }

    uint32_t base = Timer_Map_Base(index);
    if (base == 0)
    {
        return hwTimer_InvalidParameter;
    }

    MAP_Timer32_haltTimer(base);
    Timer_Unregister_IRQ(index);

    Timer_Init_Status[index] = false;
    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = 0;
    Timer_Expired_Handler[index] = NULL;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Start_OneShout(
    hwTimer_Index index,
    uint32_t duration_us,
    onTimerEventHandler timer_exp_cb
)
{
    if (index >= hwTimer_Index_MAX || duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    uint32_t base = Timer_Map_Base(index);
    if (base == 0)
    {
        return hwTimer_InvalidParameter;
    }

    uint32_t ticks = Timer_Us_To_Ticks(duration_us);

    MAP_Timer32_haltTimer(base);
    MAP_Timer32_disableInterrupt(base);
    MAP_Timer32_clearInterruptFlag(base);

    MAP_Timer32_initModule(base,
                       TIMER32_PRESCALER_1,
                       TIMER32_32BIT,
                       TIMER32_PERIODIC_MODE);

    MAP_Timer32_setCount(base, ticks);

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = duration_us;

    MAP_Timer32_enableInterrupt(base);
    MAP_Timer32_startTimer(base, true);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Start_Period(
    hwTimer_Index index,
    uint32_t duration_us,
    onTimerEventHandler timer_exp_cb
)
{
    if (index >= hwTimer_Index_MAX || duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    uint32_t base = Timer_Map_Base(index);
    if (base == 0) {
        return hwTimer_InvalidParameter;
    }

    uint32_t ticks = Timer_Us_To_Ticks(duration_us);

    MAP_Timer32_haltTimer(base);
    MAP_Timer32_disableInterrupt(base);
    MAP_Timer32_clearInterruptFlag(base);

    MAP_Timer32_initModule(base,
                       TIMER32_PRESCALER_1,
                       TIMER32_32BIT,
                       TIMER32_PERIODIC_MODE);

    MAP_Timer32_setCount(base, ticks);

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index] = true;
    Timer_Period_Us[index] = duration_us;

    MAP_Timer32_enableInterrupt(base);
    MAP_Timer32_startTimer(base, false);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Reload(hwTimer_Index index, uint32_t duration_us)
{
    if (index >= hwTimer_Index_MAX || duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    bool periodic = Timer_IsPeriodic[index];
    onTimerEventHandler cb = Timer_Expired_Handler[index];

    if (periodic)
    {
        return Timer_Start_Period(index, duration_us, cb);
    }

    return Timer_Start_OneShout(index, duration_us, cb);
}

hwTimer_OpResult Timer_Stop(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    uint32_t base = Timer_Map_Base(index);
    if (base == 0)
    {
        return hwTimer_InvalidParameter;
    }

    MAP_Timer32_haltTimer(base);
    MAP_Timer32_disableInterrupt(base);
    MAP_Timer32_clearInterruptFlag(base);

    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = 0;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_Ticks(hwTimer_Index index, uint32_t *ticks)
{
    if (index >= hwTimer_Index_MAX || ticks == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    uint32_t base = Timer_Map_Base(index);
    if (base == 0)
    {
        return hwTimer_InvalidParameter;
    }

    *ticks = MAP_Timer32_getValue(base);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_uSec(hwTimer_Index index, uint32_t *uSec)
{
    if (index >= hwTimer_Index_MAX || uSec == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    uint32_t ticks;

    hwTimer_OpResult ret = Timer_Read_Ticks(index, &ticks);
    if (ret != hwTimer_OK)
    {
        return ret;
    }

    uint64_t us = ((uint64_t)ticks * 1000000ULL) / g_sys_clock_hz;

    *uSec = (uint32_t)us;

    return hwTimer_OK;
}

bool Timer_is_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
    {
        return false;
    }

    return Timer_Init_Status[index];
}

#endif // DEVICE_TIMSP432P