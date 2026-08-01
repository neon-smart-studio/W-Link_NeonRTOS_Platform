#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Timer/Timer.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSPM0

#include "Timer_TIMSPM0.h"

static bool Timer_Init_Status[hwTimer_Index_MAX] = {false};
static bool Timer_IsPeriodic[hwTimer_Index_MAX] = {false};
static uint32_t Timer_Period_Us[hwTimer_Index_MAX] = {0};

static onTimerEventHandler Timer_Expired_Handler[hwTimer_Index_MAX] = {NULL};

static GPTIMER_Regs *Timer_Map_Soc_Base(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIMA0_BASE)
        case hwTimer_Index_0:
            return TIMA0_BASE;
#endif

#if defined(TIMA1_BASE)
        case hwTimer_Index_1:
            return TIMA1_BASE;
#endif

#if defined(TIMG0_BASE)
        case hwTimer_Index_2:
            return TIMG0_BASE;
#endif

#if defined(TIMG1_BASE)
        case hwTimer_Index_3:
            return TIMG1_BASE;
#endif

#if defined(TIMG2_BASE)
        case hwTimer_Index_4:
            return TIMG2_BASE;
#endif

#if defined(TIMG4_BASE)
        case hwTimer_Index_5:
            return TIMG4_BASE;
#endif

#if defined(TIMG5_BASE)
        case hwTimer_Index_6:
            return TIMG5_BASE;
#endif

#if defined(TIMG6_BASE)
        case hwTimer_Index_7:
            return TIMG6_BASE;
#endif

#if defined(TIMG7_BASE)
        case hwTimer_Index_8:
            return TIMG7_BASE;
#endif

#if defined(TIMG8_BASE)
#if !defined(MSPM0C1103) && !defined(MSPM0C1104) && !defined(MSPM0C1105) && !defined(MSPM0C1106)
        case hwTimer_Index_9:
            return TIMG8_BASE;
#endif
#endif

#if defined(TIMG9_BASE)
        case hwTimer_Index_10:
            return TIMG9_BASE;
#endif

#if defined(TIMG12_BASE)
        case hwTimer_Index_11:
            return TIMG12_BASE;
#endif

#if defined(TIMG14_BASE)
        case hwTimer_Index_12:
            return TIMG14_BASE;
#endif

        default:
            return NULL;
    }
}

static uint32_t Timer_Us_To_LoadValue(uint32_t us)
{
    uint64_t ticks;

    ticks =
        ((uint64_t)g_sys_clock_hz * (uint64_t)us) /
        1000000ULL;

    if (ticks == 0ULL)
    {
        ticks = 1ULL;
    }

    /*
     * MSPM0 period:
     *
     * actual ticks = period + 1
     */
    if (ticks > 0x100000000ULL)
    {
        ticks = 0x100000000ULL;
    }

    return (uint32_t)(ticks - 1ULL);
}

static void TIMSPM0_Timer_IRQ_Process(hwTimer_Index index)
{
    GPTIMER_Regs *base;
    DL_TIMER_IIDX interrupt_index;
    onTimerEventHandler callback;

    if (index >= hwTimer_Index_MAX)
    {
        return;
    }

    base = Timer_Map_Soc_Base(index);
    if (base == NULL)
    {
        return;
    }

    /*
     * 讀取 IIDX 取得並 acknowledge 最高優先權中斷。
     */
    interrupt_index = DL_Timer_getPendingInterrupt(base);

    if (interrupt_index != DL_TIMER_IIDX_ZERO)
    {
        return;
    }

    if (!Timer_IsPeriodic[index])
    {
        DL_Timer_stopCounter(base);

        Timer_Period_Us[index] = 0;
    }

    callback = Timer_Expired_Handler[index];

    if (callback != NULL)
    {
        callback(index);
    }
}

#if defined(TIMA0_BASE)
void TIMA0_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_0);
}
#endif

#if defined(TIMA1_BASE)
void TIMA1_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_1);
}
#endif

#if defined(TIMG0_BASE)
void TIMG0_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_2);
}
#endif

#if defined(TIMG1_BASE)
void TIMG1_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_3);
}
#endif

#if defined(TIMG2_BASE)
void TIMG2_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_4);
}
#endif

#if defined(TIMG4_BASE)
void TIMG4_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_5);
}
#endif

#if defined(TIMG5_BASE)
void TIMG5_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_6);
}
#endif

#if defined(TIMG6_BASE)
void TIMG6_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_7);
}
#endif

#if defined(TIMG7_BASE)
void TIMG7_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_8);
}
#endif

#if defined(TIMG8_BASE)
#if !defined(MSPM0C1103) && !defined(MSPM0C1104) && !defined(MSPM0C1105) && !defined(MSPM0C1106)
void TIMG8_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_9);
}
#endif
#endif

#if defined(TIMG9_BASE)
void TIMG9_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_10);
}
#endif

#if defined(TIMG12_BASE)
void TIMG12_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_11);
}
#endif

#if defined(TIMG14_BASE)
void TIMG14_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(hwTimer_Index_12);
}
#endif

static void Timer_NVIC_Init(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIMA0_BASE)
        case hwTimer_Index_0:
            NVIC_ClearPendingIRQ(TIMA0_INT_IRQn);
            NVIC_EnableIRQ(TIMA0_INT_IRQn);
            break;
#endif

#if defined(TIMA1_BASE)
        case hwTimer_Index_1:
            NVIC_ClearPendingIRQ(TIMA1_INT_IRQn);
            NVIC_EnableIRQ(TIMA1_INT_IRQn);
            break;
#endif

#if defined(TIMG0_BASE)
        case hwTimer_Index_2:
            NVIC_ClearPendingIRQ(TIMG0_INT_IRQn);
            NVIC_EnableIRQ(TIMG0_INT_IRQn);
            break;
#endif

#if defined(TIMG1_BASE)
        case hwTimer_Index_3:
            NVIC_ClearPendingIRQ(TIMG1_INT_IRQn);
            NVIC_EnableIRQ(TIMG1_INT_IRQn);
            break;
#endif

#if defined(TIMG2_BASE)
        case hwTimer_Index_4:
            NVIC_ClearPendingIRQ(TIMG2_INT_IRQn);
            NVIC_EnableIRQ(TIMG2_INT_IRQn);
            break;
#endif

#if defined(TIMG4_BASE)
        case hwTimer_Index_5:
            NVIC_ClearPendingIRQ(TIMG4_INT_IRQn);
            NVIC_EnableIRQ(TIMG4_INT_IRQn);
            break;
#endif

#if defined(TIMG5_BASE)
        case hwTimer_Index_6:
            NVIC_ClearPendingIRQ(TIMG5_INT_IRQn);
            NVIC_EnableIRQ(TIMG5_INT_IRQn);
            break;
#endif

#if defined(TIMG6_BASE)
        case hwTimer_Index_7:
            NVIC_ClearPendingIRQ(TIMG6_INT_IRQn);
            NVIC_EnableIRQ(TIMG6_INT_IRQn);
            break;
#endif

#if defined(TIMG7_BASE)
        case hwTimer_Index_8:
            NVIC_ClearPendingIRQ(TIMG7_INT_IRQn);
            NVIC_EnableIRQ(TIMG7_INT_IRQn);
            break;
#endif

#if defined(TIMG8_BASE)
#if !defined(MSPM0C1103) && !defined(MSPM0C1104) && !defined(MSPM0C1105) && !defined(MSPM0C1106)
        case hwTimer_Index_9:
            NVIC_ClearPendingIRQ(TIMG8_INT_IRQn);
            NVIC_EnableIRQ(TIMG8_INT_IRQn);
            break;
#endif
#endif

#if defined(TIMG9_BASE)
        case hwTimer_Index_10:
            NVIC_ClearPendingIRQ(TIMG9_INT_IRQn);
            NVIC_EnableIRQ(TIMG9_INT_IRQn);
            break;
#endif

#if defined(TIMG12_BASE)
        case hwTimer_Index_11:
            NVIC_ClearPendingIRQ(TIMG12_INT_IRQn);
            NVIC_EnableIRQ(TIMG12_INT_IRQn);
            break;
#endif

#if defined(TIMG14_BASE)
        case hwTimer_Index_12:
            NVIC_ClearPendingIRQ(TIMG14_INT_IRQn);
            NVIC_EnableIRQ(TIMG14_INT_IRQn);
            break;
#endif
    }
}

static void Timer_NVIC_DeInit(hwTimer_Index index)
{
    switch (index)
    {
#if defined(TIMA0_BASE)
        case hwTimer_Index_0:
            NVIC_DisableIRQ(TIMA0_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMA0_INT_IRQn);
            break;
#endif

#if defined(TIMA1_BASE)
        case hwTimer_Index_1:
            NVIC_DisableIRQ(TIMA1_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMA1_INT_IRQn);
            break;
#endif

#if defined(TIMG0_BASE)
        case hwTimer_Index_2:
            NVIC_DisableIRQ(TIMG0_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG0_INT_IRQn);
            break;
#endif

#if defined(TIMG1_BASE)
        case hwTimer_Index_3:
            NVIC_DisableIRQ(TIMG1_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG1_INT_IRQn);
            break;
#endif

#if defined(TIMG2_BASE)
        case hwTimer_Index_4:
            NVIC_DisableIRQ(TIMG2_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG2_INT_IRQn);
            break;
#endif

#if defined(TIMG4_BASE)
        case hwTimer_Index_5:
            NVIC_DisableIRQ(TIMG4_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG4_INT_IRQn);
            break;
#endif

#if defined(TIMG5_BASE)
        case hwTimer_Index_6:
            NVIC_DisableIRQ(TIMG5_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG5_INT_IRQn);
            break;
#endif

#if defined(TIMG6_BASE)
        case hwTimer_Index_7:
            NVIC_DisableIRQ(TIMG6_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG6_INT_IRQn);
            break;
#endif

#if defined(TIMG7_BASE)
        case hwTimer_Index_8:
            NVIC_DisableIRQ(TIMG7_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG7_INT_IRQn);
            break;
#endif

#if defined(TIMG8_BASE)
#if !defined(MSPM0C1103) && !defined(MSPM0C1104) && !defined(MSPM0C1105) && !defined(MSPM0C1106)
        case hwTimer_Index_9:
            NVIC_DisableIRQ(TIMG8_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG8_INT_IRQn);
            break;
#endif
#endif

#if defined(TIMG9_BASE)
        case hwTimer_Index_10:
            NVIC_DisableIRQ(TIMG9_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG9_INT_IRQn);
            break;
#endif

#if defined(TIMG12_BASE)
        case hwTimer_Index_11:
            NVIC_DisableIRQ(TIMG12_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG12_INT_IRQn);
            break;
#endif

#if defined(TIMG14_BASE)
        case hwTimer_Index_12:
            NVIC_DisableIRQ(TIMG14_INT_IRQn);
            NVIC_ClearPendingIRQ(TIMG14_INT_IRQn);
            break;
#endif
    }
}

hwTimer_OpResult Timer_Init(hwTimer_Index index)
{
    GPTIMER_Regs *base;
    DL_Timer_ClockConfig clock_config;
    DL_Timer_TimerConfig timer_config;

    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index])
    {
        return hwTimer_OK;
    }

    base = Timer_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    DL_Timer_enablePower(base);

    while (!DL_Timer_isPowerEnabled(base))
    {
    }

    DL_Timer_reset(base);

    clock_config.clockSel     = DL_TIMER_CLOCK_BUSCLK;
    clock_config.divideRatio  = DL_TIMER_CLOCK_DIVIDE_1;
    clock_config.prescale     = 0;

    DL_Timer_setClockConfig(base, &clock_config);

    timer_config.timerMode    = DL_TIMER_TIMER_MODE_PERIODIC;
    timer_config.period       = 0;
    timer_config.startTimer   = DL_TIMER_STOP;
    timer_config.genIntermInt = DL_TIMER_INTERM_INT_DISABLED;
    timer_config.counterVal   = 0;

    DL_Timer_initTimerMode(base, &timer_config);

    DL_Timer_disableInterrupt(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_clearInterruptStatus(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_enableClock(base);
    DL_Timer_stopCounter(base);

    Timer_NVIC_Init(index);

    Timer_Init_Status[index]     = true;
    Timer_IsPeriodic[index]      = false;
    Timer_Period_Us[index]       = 0;
    Timer_Expired_Handler[index] = NULL;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_DeInit(hwTimer_Index index)
{
    GPTIMER_Regs *base;

    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_OK;
    }

    base = Timer_Map_Soc_Base(index);

    if (base == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    DL_Timer_stopCounter(base);

    DL_Timer_disableInterrupt(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_clearInterruptStatus(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_disableClock(base);

    DL_Timer_reset(base);

    DL_Timer_disablePower(base);

    Timer_NVIC_DeInit(index);

    Timer_Init_Status[index]     = false;
    Timer_IsPeriodic[index]      = false;
    Timer_Period_Us[index]       = 0;
    Timer_Expired_Handler[index] = NULL;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Start_OneShout(hwTimer_Index index, uint32_t duration_us, onTimerEventHandler timer_exp_cb
)
{
    GPTIMER_Regs *base;
    DL_Timer_TimerConfig timer_config;
    uint32_t load_value;

    if (index >= hwTimer_Index_MAX || duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    base = Timer_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    load_value = Timer_Us_To_LoadValue(duration_us);

    DL_Timer_stopCounter(base);

    DL_Timer_disableInterrupt(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_clearInterruptStatus(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    timer_config.timerMode = DL_TIMER_TIMER_MODE_ONE_SHOT;

    timer_config.period       = load_value;
    timer_config.startTimer   = DL_TIMER_STOP;
    timer_config.genIntermInt = DL_TIMER_INTERM_INT_DISABLED;
    timer_config.counterVal   = 0;

    DL_Timer_initTimerMode(base, &timer_config);

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index]      = false;
    Timer_Period_Us[index]       = duration_us;

    DL_Timer_clearInterruptStatus(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_enableInterrupt(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_startCounter(base);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Start_Period(hwTimer_Index index, uint32_t duration_us, onTimerEventHandler timer_exp_cb)
{
    GPTIMER_Regs *base;
    DL_Timer_TimerConfig timer_config;
    uint32_t load_value;

    if (index >= hwTimer_Index_MAX || duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    base = Timer_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    load_value = Timer_Us_To_LoadValue(duration_us);

    DL_Timer_stopCounter(base);

    DL_Timer_disableInterrupt(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_clearInterruptStatus(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    timer_config.timerMode = DL_TIMER_TIMER_MODE_PERIODIC;

    timer_config.period       = load_value;
    timer_config.startTimer   = DL_TIMER_STOP;
    timer_config.genIntermInt = DL_TIMER_INTERM_INT_DISABLED;
    timer_config.counterVal   = 0;

    DL_Timer_initTimerMode(base, &timer_config);

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index]      = true;
    Timer_Period_Us[index]       = duration_us;

    DL_Timer_clearInterruptStatus(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_enableInterrupt(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_startCounter(base);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Reload(hwTimer_Index index, uint32_t duration_us)
{
    GPTIMER_Regs *base;
    DL_Timer_TimerConfig timer_config;
    uint32_t load_value;

    if (index >= hwTimer_Index_MAX || duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    if (index >= hwTimer_Index_MAX || duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    base = Timer_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    load_value = Timer_Us_To_LoadValue(duration_us);

    DL_Timer_stopCounter(base);

    DL_Timer_disableInterrupt(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_clearInterruptStatus(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    timer_config.timerMode = Timer_IsPeriodic[index] ? DL_TIMER_TIMER_MODE_PERIODIC : DL_TIMER_TIMER_MODE_ONE_SHOT;

    timer_config.period       = load_value;
    timer_config.startTimer   = DL_TIMER_STOP;
    timer_config.genIntermInt = DL_TIMER_INTERM_INT_DISABLED;
    timer_config.counterVal   = 0;

    DL_Timer_initTimerMode(base, &timer_config);

    DL_Timer_clearInterruptStatus(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_enableInterrupt(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_startCounter(base);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Stop(hwTimer_Index index)
{
    GPTIMER_Regs *base;

    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    base = Timer_Map_Soc_Base(index);

    if (base == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    DL_Timer_stopCounter(base);

    DL_Timer_disableInterrupt(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_Timer_clearInterruptStatus(base, DL_TIMER_INTERRUPT_ZERO_EVENT);

    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index]  = 0;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_Ticks(hwTimer_Index index, uint32_t *ticks)
{
    GPTIMER_Regs *base;

    if (index >= hwTimer_Index_MAX || ticks == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    base = Timer_Map_Soc_Base(index);

    if (base == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    *ticks = DL_Timer_getTimerCount(base);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_uSec(hwTimer_Index index, uint32_t *uSec)
{
    uint32_t ticks;
    uint64_t us;
    hwTimer_OpResult result;

    if (index >= hwTimer_Index_MAX || uSec == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    result = Timer_Read_Ticks(index, &ticks);

    if (result != hwTimer_OK)
    {
        return result;
    }

    if (g_sys_clock_hz == 0)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Period_Us[index] == 0)
    {
        *uSec = 0;

        return hwTimer_OK;
    }

    us = (((uint64_t)ticks + 1ULL) * 1000000ULL) /
         (uint64_t)g_sys_clock_hz;

    if (us > 0xFFFFFFFFULL)
    {
        us = 0xFFFFFFFFULL;
    }

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

#endif /* DEVICE_TIMSPM0 */