#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "RTC/RTC.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSP432P

#include "RTC_TIMSP432.h"

static bool RTC_HW_Init_Status[hwRTC_Index_MAX] = {false};
static NeonRTOS_LockObj_t rtc_access_mutex[hwRTC_Index_MAX];

static onAlarmEventCallback Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_MAX] = {NULL};

#define RTC_MUTEX_LOCK(a, b)  \
    if (NeonRTOS_LockObjLock(&rtc_access_mutex[a], b) != NeonRTOS_OK) { return hwRTC_MutexTimeout; }

#define RTC_MUTEX_UNLOCK(a)   \
    NeonRTOS_LockObjUnlock(&rtc_access_mutex[a]);

static bool RTC_IsLeapYear(int y)
{
    return ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
}

static int64_t RTC_DaysFromCivil(int y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

    return era * 146097 + (int64_t)doe - 719468;
}

static time_t RTC_Calendar_To_Unix(RTC_C_Calendar *cal)
{
    int64_t days = RTC_DaysFromCivil(cal->year, cal->month, cal->dayOfmonth);

    return (time_t)(
        days * 86400 +
        cal->hours * 3600 +
        cal->minutes * 60 +
        cal->seconds
    );
}

static bool RTC_Unix_To_Calendar(time_t unix_time, RTC_C_Calendar *cal)
{
    struct tm *tm_time = gmtime(&unix_time);

    if (tm_time == NULL)
        return false;

    cal->seconds    = tm_time->tm_sec;
    cal->minutes    = tm_time->tm_min;
    cal->hours      = tm_time->tm_hour;
    cal->dayOfWeek  = tm_time->tm_wday;
    cal->dayOfmonth = tm_time->tm_mday;
    cal->month      = tm_time->tm_mon + 1;
    cal->year       = tm_time->tm_year + 1900;

    return true;
}

void RTC_C_IRQHandler(void)
{
    uint32_t status = MAP_RTC_C_getEnabledInterruptStatus();

    MAP_RTC_C_clearInterruptFlag(status);

    if (status & RTC_C_CLOCK_ALARM_INTERRUPT) {
        MAP_RTC_C_disableInterrupt(RTC_C_CLOCK_ALARM_INTERRUPT);

        if (Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_0]) {
            Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_0](
                hwRTC_Index_0,
                hwRTC_Alarm_Channel_Index_0
            );
        }
    }
}

hwRTC_OpResult RTC_Timer_Init(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
    {
        return hwRTC_InvalidParameter;
    }

    if (RTC_HW_Init_Status[index])
    {
        return hwRTC_OK;
    }

    if (NeonRTOS_LockObjCreate(&rtc_access_mutex[index]) != NeonRTOS_OK)
    {
        return hwRTC_MemoryError;
    }

    NeonRTOS_LockObjUnlock(&rtc_access_mutex[index]);

    RTC_C_Calendar init_time = {
        .seconds    = 0,
        .minutes    = 0,
        .hours      = 0,
        .dayOfWeek  = 4,
        .dayOfmonth = 1,
        .month      = 1,
        .year       = 1970
    };

    MAP_RTC_C_holdClock();

    MAP_RTC_C_initCalendar(&init_time, RTC_C_FORMAT_BINARY);

    MAP_RTC_C_clearInterruptFlag(
        RTC_C_CLOCK_ALARM_INTERRUPT |
        RTC_C_TIME_EVENT_INTERRUPT |
        RTC_C_CLOCK_READ_READY_INTERRUPT
    );

    MAP_RTC_C_disableInterrupt(
        RTC_C_CLOCK_ALARM_INTERRUPT |
        RTC_C_TIME_EVENT_INTERRUPT |
        RTC_C_CLOCK_READ_READY_INTERRUPT
    );

    MAP_Interrupt_enableInterrupt(INT_RTC_C);

    MAP_RTC_C_startClock();

    RTC_HW_Init_Status[index] = true;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_DeInit(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
    {
        return hwRTC_InvalidParameter;
    }

    if (!RTC_HW_Init_Status[index])
    {
        return hwRTC_OK;
    }

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    MAP_RTC_C_disableInterrupt(RTC_C_CLOCK_ALARM_INTERRUPT);
    MAP_RTC_C_clearInterruptFlag(RTC_C_CLOCK_ALARM_INTERRUPT);

    MAP_RTC_C_holdClock();

    Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_0] = NULL;

    RTC_MUTEX_UNLOCK(index);

    MAP_Interrupt_disableInterrupt(INT_RTC_C);

    NeonRTOS_LockObjDelete(&rtc_access_mutex[index]);
    rtc_access_mutex[index] = NULL;

    RTC_HW_Init_Status[index] = false;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Read(hwRTC_Index index, time_t *unix_time)
{
    if (index >= hwRTC_Index_MAX || unix_time == NULL)
    {
        return hwRTC_InvalidParameter;
    }

    if (!RTC_HW_Init_Status[index])
    {
        return hwRTC_NotInit;
    }

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    RTC_C_Calendar cal = MAP_RTC_C_getCalendarTime();
    *unix_time = RTC_Calendar_To_Unix(&cal);

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Write(hwRTC_Index index, time_t unix_time)
{
    if (index >= hwRTC_Index_MAX)
    {
        return hwRTC_InvalidParameter;
    }

    if (!RTC_HW_Init_Status[index])
    {
        return hwRTC_NotInit;
    }

    RTC_C_Calendar cal;

    if (!RTC_Unix_To_Calendar(unix_time, &cal))
        return hwRTC_InvalidParameter;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    MAP_RTC_C_holdClock();
    MAP_RTC_C_initCalendar(&cal, RTC_C_FORMAT_BINARY);
    MAP_RTC_C_startClock();

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Set_Alarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch,
    time_t alarm_unix_time,
    onAlarmEventCallback cb
)
{
    if (index >= hwRTC_Index_MAX || alarm_ch >= hwRTC_Alarm_Channel_Index_MAX)
    {
        return hwRTC_InvalidParameter;
    }

    if (!RTC_HW_Init_Status[index])
    {
        return hwRTC_NotInit;
    }

    RTC_C_Calendar alarm_time;

    if (!RTC_Unix_To_Calendar(alarm_unix_time, &alarm_time))
        return hwRTC_InvalidParameter;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    Alarm_Event_Callback[alarm_ch] = cb;

    MAP_RTC_C_disableInterrupt(RTC_C_CLOCK_ALARM_INTERRUPT);
    MAP_RTC_C_clearInterruptFlag(RTC_C_CLOCK_ALARM_INTERRUPT);

    MAP_RTC_C_configureCalendarAlarm(
        alarm_time.minutes,
        alarm_time.hours,
        RTC_C_ALARMCONDITION_OFF,
        alarm_time.dayOfmonth
    );

    MAP_RTC_C_enableInterrupt(RTC_C_CLOCK_ALARM_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_RTC_C);

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Clear_Alarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch
)
{
    if (index >= hwRTC_Index_MAX || alarm_ch >= hwRTC_Alarm_Channel_Index_MAX)
    {
        return hwRTC_InvalidParameter;
    }

    if (alarm_ch != hwRTC_Alarm_Channel_Index_0)
    {
        return hwRTC_Unsupport;
    }

    if (!RTC_HW_Init_Status[index])
    {
        return hwRTC_NotInit;
    }

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    Alarm_Event_Callback[alarm_ch] = NULL;

    MAP_RTC_C_disableInterrupt(RTC_C_CLOCK_ALARM_INTERRUPT);
    MAP_RTC_C_clearInterruptFlag(RTC_C_CLOCK_ALARM_INTERRUPT);

    MAP_RTC_C_configureCalendarAlarm(
        RTC_C_ALARMCONDITION_OFF,
        RTC_C_ALARMCONDITION_OFF,
        RTC_C_ALARMCONDITION_OFF,
        RTC_C_ALARMCONDITION_OFF
    );

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

#endif // DEVICE_TIMSP432P