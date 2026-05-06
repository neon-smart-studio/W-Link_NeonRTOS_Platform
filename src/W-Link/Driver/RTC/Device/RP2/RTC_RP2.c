#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "RTC/RTC.h"

#ifdef DEVICE_RP2

#include "RTC_RP2.h"

static bool RTC_HW_Init_Status[hwRTC_Index_MAX] = {false};
static NeonRTOS_LockObj_t rtc_access_mutex[hwRTC_Index_MAX];

onAlarmEventCallback Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_MAX] = {NULL};

#define RTC_MUTEX_LOCK(a, b)  \
    if (NeonRTOS_LockObjLock(&rtc_access_mutex[a], b) != NeonRTOS_OK) { return hwRTC_MutexTimeout; }

#define RTC_MUTEX_UNLOCK(a)   \
    NeonRTOS_LockObjUnlock(&rtc_access_mutex[a]);

static void UnixToRP2RTC(time_t t, datetime_t *dt)
{
    struct tm tm_time;
    gmtime_r(&t, &tm_time);

    dt->year  = tm_time.tm_year + 1900;
    dt->month = tm_time.tm_mon + 1;
    dt->day   = tm_time.tm_mday;
    dt->dotw  = tm_time.tm_wday;
    dt->hour  = tm_time.tm_hour;
    dt->min   = tm_time.tm_min;
    dt->sec   = tm_time.tm_sec;
}

static time_t RP2RTCToUnix(const datetime_t *dt)
{
    struct tm tm_time = {0};

    tm_time.tm_year = dt->year - 1900;
    tm_time.tm_mon  = dt->month - 1;
    tm_time.tm_mday = dt->day;
    tm_time.tm_hour = dt->hour;
    tm_time.tm_min  = dt->min;
    tm_time.tm_sec  = dt->sec;

    return mktime(&tm_time);
}

#ifdef RP2040
static void RTC_IRQ_Handler(void)
{
    rtc_disable_alarm();

    if (Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_0]) {
        Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_0](
            hwRTC_Index_0,
            hwRTC_Alarm_Channel_Index_0
        );
    }
}
#endif

hwRTC_OpResult RTC_Timer_Init(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if (RTC_HW_Init_Status[index])
        return hwRTC_OK;

    if (NeonRTOS_LockObjCreate(&rtc_access_mutex[index]) != NeonRTOS_OK)
        return hwRTC_MemoryError;

    NeonRTOS_LockObjUnlock(&rtc_access_mutex[index]);

    rtc_init();

    RTC_HW_Init_Status[index] = true;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_DeInit(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_OK;

#ifdef RP2040
    rtc_disable_alarm();
    irq_set_enabled(RTC_IRQ, false);
#endif

    NeonRTOS_LockObjDelete(&rtc_access_mutex[index]);
    rtc_access_mutex[index] = NULL;

    RTC_HW_Init_Status[index] = false;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Read(hwRTC_Index index, time_t *unix_time)
{
    if (index >= hwRTC_Index_MAX || unix_time == NULL)
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    datetime_t dt;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    if (!rtc_get_datetime(&dt)) {
        RTC_MUTEX_UNLOCK(index);
        return hwRTC_HwError;
    }

    *unix_time = RP2RTCToUnix(&dt);

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Write(hwRTC_Index index, time_t unix_time)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    datetime_t dt;
    UnixToRP2RTC(unix_time, &dt);

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    if (!rtc_set_datetime(&dt)) {
        RTC_MUTEX_UNLOCK(index);
        return hwRTC_HwError;
    }

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

#ifdef RP2040
hwRTC_OpResult RTC_Timer_Set_Alarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch,
    time_t alarm_unix_time,
    onAlarmEventCallback cb
)
{
    if (index >= hwRTC_Index_MAX || alarm_ch >= hwRTC_Alarm_Channel_Index_MAX)
        return hwRTC_InvalidParameter;

    if (alarm_ch != hwRTC_Alarm_Channel_Index_0)
        return hwRTC_Unsupport;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    datetime_t dt;
    UnixToRP2RTC(alarm_unix_time, &dt);

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    Alarm_Event_Callback[alarm_ch] = cb;

    irq_set_exclusive_handler(RTC_IRQ, RTC_IRQ_Handler);
    irq_set_enabled(RTC_IRQ, true);

    rtc_disable_alarm();
    rtc_set_alarm(&dt, RTC_IRQ_Handler);

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Clear_Alarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch
)
{
    if (index >= hwRTC_Index_MAX || alarm_ch >= hwRTC_Alarm_Channel_Index_MAX)
        return hwRTC_InvalidParameter;

    if (alarm_ch != hwRTC_Alarm_Channel_Index_0)
        return hwRTC_Unsupport;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    Alarm_Event_Callback[alarm_ch] = NULL;
    
    rtc_disable_alarm();

    irq_set_enabled(RTC_IRQ, false);

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}
#endif

#endif // DEVICE_STM32