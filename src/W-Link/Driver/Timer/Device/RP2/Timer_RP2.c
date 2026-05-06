#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef DEVICE_RP2

#include "hardware/timer.h"
#include "hardware/irq.h"

static bool Timer_Init_Status[hwTimer_Index_MAX] = {false};
static bool Timer_IsPeriodic[hwTimer_Index_MAX] = {false};
static uint32_t Timer_Period_Us[hwTimer_Index_MAX] = {0};
static onTimerEventHandler Timer_Expired_Handler[hwTimer_Index_MAX] = {NULL};

static alarm_id_t Timer_Alarm_ID[hwTimer_Index_MAX] = {0};

static int64_t RP2_Timer_Alarm_Callback(alarm_id_t id, void *user_data)
{
    (void)id;

    hwTimer_Index index = (hwTimer_Index)(uintptr_t)user_data;

    if (index >= hwTimer_Index_MAX) {
        return 0;
    }

    if (Timer_Expired_Handler[index]) {
        Timer_Expired_Handler[index](index);
    }

    if (Timer_IsPeriodic[index]) {
        return (int64_t)Timer_Period_Us[index];
    }

    if (!Timer_IsPeriodic[index]) {
        Timer_Alarm_ID[index] = 0;
        return 0;
    }
    
    return 0;
}

hwTimer_OpResult Timer_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index]) {
        return hwTimer_OK;
    }

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

    Timer_Stop(index);

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
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    if (duration_us == 0) {
        return hwTimer_InvalidParameter;
    }

    Timer_Stop(index);

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = duration_us;

    alarm_id_t alarm_id = add_alarm_in_us(
        duration_us,
        RP2_Timer_Alarm_Callback,
        (void *)(uintptr_t)index,
        true
    );

    if (alarm_id < 0) {
        return hwTimer_HwError;
    }

    Timer_Alarm_ID[index] = alarm_id;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Start_Period(
    hwTimer_Index index,
    uint32_t duration_us,
    onTimerEventHandler timer_exp_cb
)
{
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    if (duration_us == 0) {
        return hwTimer_InvalidParameter;
    }

    Timer_Stop(index);

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index] = true;
    Timer_Period_Us[index] = duration_us;

    alarm_id_t alarm_id = add_alarm_in_us(
        duration_us,
        RP2_Timer_Alarm_Callback,
        (void *)(uintptr_t)index,
        true
    );

    if (alarm_id < 0) {
        return hwTimer_HwError;
    }

    Timer_Alarm_ID[index] = alarm_id;
    
    return hwTimer_OK;
}

hwTimer_OpResult Timer_Reload(hwTimer_Index index, uint32_t duration_us)
{
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    if (duration_us == 0) {
        return hwTimer_InvalidParameter;
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

    Timer_IsPeriodic[index] = false;
    Timer_Period_Us[index] = 0;

    if (Timer_Alarm_ID[index] > 0) {
        cancel_alarm(Timer_Alarm_ID[index]);
        Timer_Alarm_ID[index] = 0;
    }
    
    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_Ticks(hwTimer_Index index, uint32_t* ticks)
{
    if (index >= hwTimer_Index_MAX || ticks == NULL) {
        return hwTimer_InvalidParameter;
    }

    if (!Timer_Init_Status[index]) {
        return hwTimer_NotInit;
    }

    *ticks = (uint32_t)time_us_64();

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_uSec(hwTimer_Index index, uint32_t* uSec)
{
    return Timer_Read_Ticks(index, uSec);
}

bool Timer_is_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX) {
        return false;
    }

    return Timer_Init_Status[index];
}

#endif // DEVICE_RP2