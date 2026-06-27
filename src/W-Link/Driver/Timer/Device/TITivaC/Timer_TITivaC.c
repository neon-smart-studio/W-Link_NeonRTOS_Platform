#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef DEVICE_TITIVAC

#include "Timer_TITivaC.h"

static bool Timer_Init_Status[hwTimer_Index_MAX] = {false};
static bool Timer_IsPeriodic[hwTimer_Index_MAX] = {false};
static uint32_t Timer_Period_Us[hwTimer_Index_MAX] = {0};
static onTimerEventHandler Timer_Expired_Handler[hwTimer_Index_MAX] = {NULL};

static uint32_t Timer_Map_Base(hwTimer_Index index)
{
    switch (index)
    {
        case hwTimer_Index_0: return TIMER0_BASE;
        case hwTimer_Index_1: return TIMER1_BASE;
        case hwTimer_Index_2: return TIMER2_BASE;
        case hwTimer_Index_3: return TIMER3_BASE;
        case hwTimer_Index_4: return TIMER4_BASE;
        case hwTimer_Index_5: return TIMER5_BASE;
        case hwTimer_Index_6: return TIMER6_BASE;
        case hwTimer_Index_7: return TIMER7_BASE;
        default: return 0;
    }
}

static uint32_t Timer_Map_Periph(hwTimer_Index index)
{
    switch (index)
    {
        case hwTimer_Index_0: return SYSCTL_PERIPH_TIMER0;
        case hwTimer_Index_1: return SYSCTL_PERIPH_TIMER1;
        case hwTimer_Index_2: return SYSCTL_PERIPH_TIMER2;
        case hwTimer_Index_3: return SYSCTL_PERIPH_TIMER3;
        case hwTimer_Index_4: return SYSCTL_PERIPH_TIMER4;
        case hwTimer_Index_5: return SYSCTL_PERIPH_TIMER5;
        case hwTimer_Index_6: return SYSCTL_PERIPH_TIMER6;
        case hwTimer_Index_7: return SYSCTL_PERIPH_TIMER7;
        default: return 0;
    }
}

static uint32_t Timer_Map_IRQ(hwTimer_Index index)
{
    switch (index)
    {
        case hwTimer_Index_0: return INT_TIMER0A;
        case hwTimer_Index_1: return INT_TIMER1A;
        case hwTimer_Index_2: return INT_TIMER2A;
        case hwTimer_Index_3: return INT_TIMER3A;
        case hwTimer_Index_4: return INT_TIMER4A;
        case hwTimer_Index_5: return INT_TIMER5A;
        case hwTimer_Index_6: return INT_TIMER6A;
        case hwTimer_Index_7: return INT_TIMER7A;
        default: return 0;
    }
}

static uint32_t Timer_Us_To_Ticks(uint32_t us)
{
    uint64_t clk = MAP_SysCtlClockGet();
    uint64_t ticks = (clk * us) / 1000000ULL;

    if (ticks == 0) {
        ticks = 1;
    }

    if (ticks > 0xFFFFFFFFULL) {
        ticks = 0xFFFFFFFFULL;
    }

    return (uint32_t)ticks;
}

static void TITivaC_Timer_IRQ_Process(hwTimer_Index index)
{
    uint32_t base = Timer_Map_Base(index);

    if (base == 0) {
        return;
    }

    MAP_TimerIntClear(base, TIMER_TIMA_TIMEOUT);

    if (Timer_Expired_Handler[index]) {
        Timer_Expired_Handler[index](index);
    }

    if (!Timer_IsPeriodic[index]) {
        MAP_TimerDisable(base, TIMER_A);
        Timer_Period_Us[index] = 0;
    }
}

static void TIMER0A_IRQ_Handler(void)
{
    TITivaC_Timer_IRQ_Process(hwTimer_Index_0);
}

static void TIMER1A_IRQ_Handler(void)
{
    TITivaC_Timer_IRQ_Process(hwTimer_Index_1);
}

static void TIMER2A_IRQ_Handler(void)
{
    TITivaC_Timer_IRQ_Process(hwTimer_Index_2);
}

static void TIMER3A_IRQ_Handler(void)
{
    TITivaC_Timer_IRQ_Process(hwTimer_Index_3);
}

static void TIMER4A_IRQ_Handler(void)
{
    TITivaC_Timer_IRQ_Process(hwTimer_Index_4);
}

static void TIMER5A_IRQ_Handler(void)
{
    TITivaC_Timer_IRQ_Process(hwTimer_Index_5);
}

static void TIMER6A_IRQ_Handler(void)
{
    TITivaC_Timer_IRQ_Process(hwTimer_Index_6);
}

static void TIMER7A_IRQ_Handler(void)
{
    TITivaC_Timer_IRQ_Process(hwTimer_Index_7);
}

static void Timer_Register_IRQ(hwTimer_Index index)
{
    uint32_t base = Timer_Map_Base(index);
    uint32_t irq = Timer_Map_IRQ(index);

    if (base == 0 || irq == 0) {
        return;
    }

    switch (index)
    {
        case hwTimer_Index_0:
            TimerIntRegister(base, TIMER_A, TIMER0A_IRQ_Handler);
            break;

        case hwTimer_Index_1:
            TimerIntRegister(base, TIMER_A, TIMER1A_IRQ_Handler);
            break;

        case hwTimer_Index_2:
            TimerIntRegister(base, TIMER_A, TIMER2A_IRQ_Handler);
            break;

        case hwTimer_Index_3:
            TimerIntRegister(base, TIMER_A, TIMER3A_IRQ_Handler);
            break;

        case hwTimer_Index_4:
            TimerIntRegister(base, TIMER_A, TIMER4A_IRQ_Handler);
            break;

        case hwTimer_Index_5:
            TimerIntRegister(base, TIMER_A, TIMER5A_IRQ_Handler);
            break;

        case hwTimer_Index_6:
            TimerIntRegister(base, TIMER_A, TIMER6A_IRQ_Handler);
            break;

        case hwTimer_Index_7:
            TimerIntRegister(base, TIMER_A, TIMER7A_IRQ_Handler);
            break;

        default:
            break;
    }

    MAP_IntEnable(irq);
}

static void Timer_Unregister_IRQ(hwTimer_Index index)
{
    uint32_t base = Timer_Map_Base(index);
    uint32_t irq = Timer_Map_IRQ(index);

    if (base != 0) {
        MAP_TimerIntDisable(base, TIMER_TIMA_TIMEOUT);
        MAP_TimerIntClear(base, TIMER_TIMA_TIMEOUT);
        MAP_TimerIntUnregister(base, TIMER_A);
    }

    if (irq != 0) {
        MAP_IntDisable(irq);
    }
}

hwTimer_OpResult Timer_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index]) {
        return hwTimer_OK;
    }

    uint32_t base = Timer_Map_Base(index);
    uint32_t periph = Timer_Map_Periph(index);

    if (base == 0 || periph == 0) {
        return hwTimer_InvalidParameter;
    }

    MAP_SysCtlPeripheralEnable(periph);
    while (!MAP_SysCtlPeripheralReady(periph));

    MAP_TimerDisable(base, TIMER_A);
    MAP_TimerConfigure(base, TIMER_CFG_ONE_SHOT);
    MAP_TimerIntDisable(base, TIMER_TIMA_TIMEOUT);
    MAP_TimerIntClear(base, TIMER_TIMA_TIMEOUT);

    Timer_Register_IRQ(index);

    Timer_Init_Status[index] = true;
    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = 0;
    Timer_Expired_Handler[index] = NULL;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_DeInit(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_OK;
    }

    uint32_t base = Timer_Map_Base(index);

    if (base == 0) {
        return hwTimer_InvalidParameter;
    }

    MAP_TimerDisable(base, TIMER_A);
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
    if (index >= hwTimer_Index_MAX || duration_us == 0) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    uint32_t base = Timer_Map_Base(index);

    if (base == 0) {
        return hwTimer_InvalidParameter;
    }

    uint32_t ticks = Timer_Us_To_Ticks(duration_us);

    MAP_TimerDisable(base, TIMER_A);
    MAP_TimerIntDisable(base, TIMER_TIMA_TIMEOUT);
    MAP_TimerIntClear(base, TIMER_TIMA_TIMEOUT);

    MAP_TimerConfigure(base, TIMER_CFG_ONE_SHOT);
    MAP_TimerLoadSet(base, TIMER_A, ticks - 1);

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = duration_us;

    MAP_TimerIntEnable(base, TIMER_TIMA_TIMEOUT);
    MAP_TimerEnable(base, TIMER_A);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Start_Period(
    hwTimer_Index index,
    uint32_t duration_us,
    onTimerEventHandler timer_exp_cb
)
{
    if (index >= hwTimer_Index_MAX || duration_us == 0) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    uint32_t base = Timer_Map_Base(index);

    if (base == 0) {
        return hwTimer_InvalidParameter;
    }

    uint32_t ticks = Timer_Us_To_Ticks(duration_us);

    MAP_TimerDisable(base, TIMER_A);
    MAP_TimerIntDisable(base, TIMER_TIMA_TIMEOUT);
    MAP_TimerIntClear(base, TIMER_TIMA_TIMEOUT);

    MAP_TimerConfigure(base, TIMER_CFG_PERIODIC);
    MAP_TimerLoadSet(base, TIMER_A, ticks - 1);

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index] = true;
    Timer_Period_Us[index] = duration_us;

    MAP_TimerIntEnable(base, TIMER_TIMA_TIMEOUT);
    MAP_TimerEnable(base, TIMER_A);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Reload(hwTimer_Index index, uint32_t duration_us)
{
    if (index >= hwTimer_Index_MAX || duration_us == 0) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    bool periodic = Timer_IsPeriodic[index];
    onTimerEventHandler cb = Timer_Expired_Handler[index];

    if (periodic) {
        return Timer_Start_Period(index, duration_us, cb);
    }

    return Timer_Start_OneShout(index, duration_us, cb);
}

hwTimer_OpResult Timer_Stop(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    uint32_t base = Timer_Map_Base(index);

    if (base == 0) {
        return hwTimer_InvalidParameter;
    }

    MAP_TimerDisable(base, TIMER_A);
    MAP_TimerIntDisable(base, TIMER_TIMA_TIMEOUT);
    MAP_TimerIntClear(base, TIMER_TIMA_TIMEOUT);

    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = 0;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_Ticks(hwTimer_Index index, uint32_t *ticks)
{
    if (index >= hwTimer_Index_MAX || ticks == NULL) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    uint32_t base = Timer_Map_Base(index);

    if (base == 0) {
        return hwTimer_InvalidParameter;
    }

    *ticks = MAP_TimerValueGet(base, TIMER_A);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_uSec(hwTimer_Index index, uint32_t *uSec)
{
    if (index >= hwTimer_Index_MAX || uSec == NULL) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    uint32_t ticks;

    hwTimer_OpResult ret = Timer_Read_Ticks(index, &ticks);

    if (ret != hwTimer_OK) {
        return ret;
    }

    uint64_t us = ((uint64_t)ticks * 1000000ULL) / MAP_SysCtlClockGet();

    *uSec = (uint32_t)us;

    return hwTimer_OK;
}

bool Timer_is_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX) {
        return false;
    }

    return Timer_Init_Status[index];
}

#endif // DEVICE_TITIVAC