#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Timer/Timer.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSPM0

#include "Timer_TIMSPM0.h"

#define TIMER_TIMSPM0_POWER_STARTUP_DELAY    (16U)
#define TIMER_TIMSPM0_16BIT_MAX_COUNTS       (65536ULL)
#define TIMER_TIMSPM0_32BIT_MAX_COUNTS       (4294967296ULL)
#define TIMER_TIMSPM0_MAX_CLOCK_DIVIDE       (8U)
#define TIMER_TIMSPM0_MAX_PRESCALE_DIVIDE    (256U)

static bool Timer_Init_Status[hwTimer_Index_MAX] = {false};
static bool Timer_IsPeriodic[hwTimer_Index_MAX] = {false};
static uint32_t Timer_Period_Us[hwTimer_Index_MAX] = {0U};
static uint32_t Timer_Tick_Divisor[hwTimer_Index_MAX] = {0U};
static onTimerEventHandler
    Timer_Expired_Handler[hwTimer_Index_MAX] = {NULL};

static bool Timer_IsValidIndex(hwTimer_Index index)
{
    return ((int32_t) index >= 0) &&
           (index < hwTimer_Index_MAX) &&
           ((size_t) index <
            TIMSPM0_TimerResource_GetCount()) &&
           (TIMSPM0_TimerResource_GetTimer(
                (size_t) index) != NULL);
}

static GPTIMER_Regs *Timer_Map_Base(hwTimer_Index index)
{
    if (!Timer_IsValidIndex(index))
    {
        return NULL;
    }

    return TIMSPM0_TimerResource_GetTimer(
        (size_t) index);
}

static hwTimer_Index Timer_Map_Index(
    GPTIMER_Regs *timer)
{
    for (hwTimer_Index index = (hwTimer_Index) 0;
         index < hwTimer_Index_MAX;
         index = (hwTimer_Index) (index + 1))
    {
        if (Timer_Map_Base(index) == timer)
        {
            return index;
        }
    }

    return hwTimer_Index_MAX;
}

static DL_TIMER_CLOCK_DIVIDE Timer_Map_ClockDivide(
    uint32_t divide)
{
    switch (divide)
    {
        case 1U:
            return DL_TIMER_CLOCK_DIVIDE_1;

        case 2U:
            return DL_TIMER_CLOCK_DIVIDE_2;

        case 3U:
            return DL_TIMER_CLOCK_DIVIDE_3;

        case 4U:
            return DL_TIMER_CLOCK_DIVIDE_4;

        case 5U:
            return DL_TIMER_CLOCK_DIVIDE_5;

        case 6U:
            return DL_TIMER_CLOCK_DIVIDE_6;

        case 7U:
            return DL_TIMER_CLOCK_DIVIDE_7;

        case 8U:
        default:
            return DL_TIMER_CLOCK_DIVIDE_8;
    }
}

static bool Timer_Is32Bit(GPTIMER_Regs *timer)
{
#if defined(TIMG12_BASE)
    return (timer == TIMG12);
#else
    (void) timer;
    return false;
#endif
}

static bool Timer_CalculateConfig(
    GPTIMER_Regs *timer,
    uint32_t duration_us,
    DL_TIMER_CLOCK_DIVIDE *clock_divide,
    uint8_t *prescale,
    uint32_t *load_value,
    uint32_t *tick_divisor)
{
    if ((timer == NULL) ||
        (duration_us == 0U) ||
        (clock_divide == NULL) ||
        (prescale == NULL) ||
        (load_value == NULL) ||
        (tick_divisor == NULL) ||
        (g_sys_clock_hz == 0U))
    {
        return false;
    }

    uint64_t source_ticks =
        (((uint64_t) g_sys_clock_hz * duration_us) +
         999999ULL) /
        1000000ULL;

    if (source_ticks == 0U)
    {
        source_ticks = 1U;
    }

    bool is_32_bit = Timer_Is32Bit(timer);
    uint64_t max_counts =
        is_32_bit ?
            TIMER_TIMSPM0_32BIT_MAX_COUNTS :
            TIMER_TIMSPM0_16BIT_MAX_COUNTS;
    uint32_t max_prescale_divide =
        is_32_bit ?
            1U :
            TIMER_TIMSPM0_MAX_PRESCALE_DIVIDE;
    uint32_t best_total_divide = UINT32_MAX;
    uint32_t best_clock_divide = 0U;
    uint32_t best_prescale_divide = 0U;
    uint64_t best_counts = 0U;

    for (uint32_t divide = 1U;
         divide <= TIMER_TIMSPM0_MAX_CLOCK_DIVIDE;
         divide++)
    {
        for (uint32_t prescale_divide = 1U;
             prescale_divide <= max_prescale_divide;
             prescale_divide++)
        {
            uint32_t total_divide =
                divide * prescale_divide;

            if (total_divide >= best_total_divide)
            {
                continue;
            }

            uint64_t counts =
                (source_ticks + total_divide - 1U) /
                total_divide;

            if ((counts > 0U) &&
                (counts <= max_counts))
            {
                best_total_divide = total_divide;
                best_clock_divide = divide;
                best_prescale_divide =
                    prescale_divide;
                best_counts = counts;
            }
        }
    }

    if ((best_clock_divide == 0U) ||
        (best_prescale_divide == 0U) ||
        (best_counts == 0U))
    {
        return false;
    }

    *clock_divide =
        Timer_Map_ClockDivide(best_clock_divide);
    *prescale =
        (uint8_t) (best_prescale_divide - 1U);
    *load_value = (uint32_t) (best_counts - 1U);
    *tick_divisor = best_total_divide;

    return true;
}

static void Timer_StopHardware(GPTIMER_Regs *timer)
{
    if (timer == NULL)
    {
        return;
    }

    DL_Timer_stopCounter(timer);
    DL_Timer_disableInterrupt(
        timer,
        DL_TIMER_INTERRUPT_ZERO_EVENT);
    DL_Timer_clearInterruptStatus(
        timer,
        DL_TIMER_INTERRUPT_ZERO_EVENT);
}

static void Timer_Register_IRQ(GPTIMER_Regs *timer)
{
    IRQn_Type irq =
        TIMSPM0_TimerResource_GetIRQ(timer);

    if ((int32_t) irq < 0)
    {
        return;
    }

    NVIC_ClearPendingIRQ(irq);
    NVIC_EnableIRQ(irq);
}

static void Timer_Unregister_IRQ(GPTIMER_Regs *timer)
{
    IRQn_Type irq =
        TIMSPM0_TimerResource_GetIRQ(timer);

    if ((int32_t) irq < 0)
    {
        return;
    }

    NVIC_DisableIRQ(irq);
    NVIC_ClearPendingIRQ(irq);
}

static hwTimer_OpResult Timer_Start(
    hwTimer_Index index,
    uint32_t duration_us,
    onTimerEventHandler timer_exp_cb,
    bool periodic)
{
    if (!Timer_IsValidIndex(index) ||
        (duration_us == 0U))
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    GPTIMER_Regs *timer = Timer_Map_Base(index);
    DL_TIMER_CLOCK_DIVIDE clock_divide;
    uint8_t prescale;
    uint32_t load_value;
    uint32_t tick_divisor;

    if (!Timer_CalculateConfig(
            timer,
            duration_us,
            &clock_divide,
            &prescale,
            &load_value,
            &tick_divisor))
    {
        return hwTimer_InvalidParameter;
    }

    Timer_StopHardware(timer);

    const DL_Timer_ClockConfig clock_config = {
        .clockSel = DL_TIMER_CLOCK_BUSCLK,
        .divideRatio = clock_divide,
        .prescale = prescale,
    };

    const DL_Timer_TimerConfig timer_config = {
        .timerMode =
            periodic ?
                DL_TIMER_TIMER_MODE_PERIODIC :
                DL_TIMER_TIMER_MODE_ONE_SHOT,
        .period = load_value,
        .startTimer = DL_TIMER_STOP,
        .genIntermInt =
            DL_TIMER_INTERM_INT_DISABLED,
        .counterVal = 0U,
    };

    DL_Timer_setClockConfig(timer, &clock_config);
    DL_Timer_initTimerMode(timer, &timer_config);
    DL_Timer_clearInterruptStatus(
        timer,
        DL_TIMER_INTERRUPT_ZERO_EVENT);

    IRQn_Type irq =
        TIMSPM0_TimerResource_GetIRQ(timer);
    if ((int32_t) irq >= 0)
    {
        NVIC_ClearPendingIRQ(irq);
    }

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index] = periodic;
    Timer_Period_Us[index] = duration_us;
    Timer_Tick_Divisor[index] = tick_divisor;

    DL_Timer_enableInterrupt(
        timer,
        DL_TIMER_INTERRUPT_ZERO_EVENT);
    DL_Timer_enableClock(timer);
    DL_Timer_startCounter(timer);

    return hwTimer_OK;
}

static void TIMSPM0_Timer_IRQ_Process(
    GPTIMER_Regs *timer)
{
    hwTimer_Index index = Timer_Map_Index(timer);

    if (!Timer_IsValidIndex(index) ||
        !Timer_Init_Status[index])
    {
        if (timer != NULL)
        {
            DL_Timer_clearInterruptStatus(
                timer,
                DL_TIMER_INTERRUPT_ZERO_EVENT);
        }
        return;
    }

    if (DL_Timer_getPendingInterrupt(timer) !=
        DL_TIMER_IIDX_ZERO)
    {
        return;
    }

    onTimerEventHandler callback =
        Timer_Expired_Handler[index];

    if (!Timer_IsPeriodic[index])
    {
        Timer_StopHardware(timer);
        Timer_Period_Us[index] = 0U;
        Timer_Tick_Divisor[index] = 0U;
    }

    if (callback != NULL)
    {
        callback(index);
    }
}

#if defined(TIMA0_BASE)
void TIMA0_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMA0);
}
#endif

#if defined(TIMA1_BASE)
void TIMA1_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMA1);
}
#endif

#if defined(TIMG0_BASE)
void TIMG0_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG0);
}
#endif

#if defined(TIMG1_BASE)
void TIMG1_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG1);
}
#endif

#if defined(TIMG2_BASE)
void TIMG2_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG2);
}
#endif

#if defined(TIMG4_BASE)
void TIMG4_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG4);
}
#endif

#if defined(TIMG5_BASE)
void TIMG5_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG5);
}
#endif

#if defined(TIMG6_BASE)
void TIMG6_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG6);
}
#endif

#if defined(TIMG7_BASE)
void TIMG7_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG7);
}
#endif

#if defined(TIMG8_BASE)
void TIMG8_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG8);
}
#endif

#if defined(TIMG9_BASE)
void TIMG9_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG9);
}
#endif

#if defined(TIMG12_BASE)
void TIMG12_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG12);
}
#endif

#if defined(TIMG14_BASE)
void TIMG14_IRQHandler(void)
{
    TIMSPM0_Timer_IRQ_Process(TIMG14);
}
#endif

hwTimer_OpResult Timer_Init(hwTimer_Index index)
{
    if (!Timer_IsValidIndex(index))
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index])
    {
        return hwTimer_OK;
    }

    GPTIMER_Regs *timer = Timer_Map_Base(index);

    /*
     * A GPTIMER instance cannot be reconfigured while PWM owns it.
     */
    if (!TIMSPM0_TimerResource_Claim(
            timer,
            TIMSPM0_TimerOwner_Timer))
    {
        return hwTimer_InvalidParameter;
    }

    IRQn_Type irq =
        TIMSPM0_TimerResource_GetIRQ(timer);
    if ((int32_t) irq < 0)
    {
        TIMSPM0_TimerResource_Release(
            timer,
            TIMSPM0_TimerOwner_Timer);
        return hwTimer_InvalidParameter;
    }

    DL_Timer_reset(timer);
    DL_Timer_enablePower(timer);
    DL_Common_delayCycles(
        TIMER_TIMSPM0_POWER_STARTUP_DELAY);

    Timer_Register_IRQ(timer);

    Timer_Init_Status[index] = true;
    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = 0U;
    Timer_Tick_Divisor[index] = 0U;
    Timer_Expired_Handler[index] = NULL;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_DeInit(hwTimer_Index index)
{
    if (!Timer_IsValidIndex(index))
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_OK;
    }

    GPTIMER_Regs *timer = Timer_Map_Base(index);

    Timer_StopHardware(timer);
    Timer_Unregister_IRQ(timer);
    DL_Timer_disableClock(timer);
    DL_Timer_reset(timer);
    DL_Timer_disablePower(timer);

    Timer_Init_Status[index] = false;
    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = 0U;
    Timer_Tick_Divisor[index] = 0U;
    Timer_Expired_Handler[index] = NULL;

    TIMSPM0_TimerResource_Release(
        timer,
        TIMSPM0_TimerOwner_Timer);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Start_OneShout(
    hwTimer_Index index,
    uint32_t duration_us,
    onTimerEventHandler timer_exp_cb)
{
    return Timer_Start(
        index,
        duration_us,
        timer_exp_cb,
        false);
}

hwTimer_OpResult Timer_Start_Period(
    hwTimer_Index index,
    uint32_t duration_us,
    onTimerEventHandler timer_exp_cb)
{
    return Timer_Start(
        index,
        duration_us,
        timer_exp_cb,
        true);
}

hwTimer_OpResult Timer_Reload(
    hwTimer_Index index,
    uint32_t duration_us)
{
    if (!Timer_IsValidIndex(index) ||
        (duration_us == 0U))
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    return Timer_Start(
        index,
        duration_us,
        Timer_Expired_Handler[index],
        Timer_IsPeriodic[index]);
}

hwTimer_OpResult Timer_Stop(hwTimer_Index index)
{
    if (!Timer_IsValidIndex(index))
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    Timer_StopHardware(Timer_Map_Base(index));
    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = 0U;
    Timer_Tick_Divisor[index] = 0U;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_Ticks(
    hwTimer_Index index,
    uint32_t *ticks)
{
    if (!Timer_IsValidIndex(index) ||
        (ticks == NULL))
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    GPTIMER_Regs *timer = Timer_Map_Base(index);
    if (timer == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    *ticks = DL_Timer_getTimerCount(timer);
    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_uSec(
    hwTimer_Index index,
    uint32_t *uSec)
{
    if (!Timer_IsValidIndex(index) ||
        (uSec == NULL))
    {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index])
    {
        return hwTimer_NotInit;
    }

    if (g_sys_clock_hz == 0U)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Tick_Divisor[index] == 0U)
    {
        *uSec = 0U;
        return hwTimer_OK;
    }

    uint32_t ticks;
    hwTimer_OpResult result =
        Timer_Read_Ticks(index, &ticks);

    if (result != hwTimer_OK)
    {
        return result;
    }

    uint64_t remaining_us =
        ((uint64_t) ticks *
         Timer_Tick_Divisor[index] *
         1000000ULL) /
        g_sys_clock_hz;

    if (remaining_us > UINT32_MAX)
    {
        remaining_us = UINT32_MAX;
    }

    *uSec = (uint32_t) remaining_us;
    return hwTimer_OK;
}

bool Timer_is_Init(hwTimer_Index index)
{
    return Timer_IsValidIndex(index) &&
           Timer_Init_Status[index];
}

#endif // DEVICE_TIMSPM0