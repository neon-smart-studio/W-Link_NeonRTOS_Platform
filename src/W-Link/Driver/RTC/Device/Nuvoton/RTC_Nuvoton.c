#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "RTC/RTC.h"

#ifdef DEVICE_NUVOTON

#include "RTC_Nuvoton.h"

static bool RTC_HW_Init_Status[hwRTC_Index_MAX] = {false};
static NeonRTOS_LockObj_t rtc_access_mutex[hwRTC_Index_MAX];

onAlarmEventCallback Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_MAX] = {NULL};

#define RTC_MUTEX_LOCK(a, b)  \
    if (NeonRTOS_LockObjLock(&rtc_access_mutex[a], b) != NeonRTOS_OK) { return hwRTC_MutexTimeout; }

#define RTC_MUTEX_UNLOCK(a)   \
    NeonRTOS_LockObjUnlock(&rtc_access_mutex[a]);

static void UnixToRTC(time_t t, S_RTC_TIME_DATA_T *rtc)
{
    struct tm tm_time;
    gmtime_r(&t, &tm_time);

    rtc->u32Year      = tm_time.tm_year + 1900;
    rtc->u32Month     = tm_time.tm_mon + 1;
    rtc->u32Day       = tm_time.tm_mday;
    rtc->u32Hour      = tm_time.tm_hour;
    rtc->u32Minute    = tm_time.tm_min;
    rtc->u32Second    = tm_time.tm_sec;
    rtc->u32DayOfWeek = tm_time.tm_wday;
    rtc->u32TimeScale = RTC_CLOCK_24;
}

static time_t RTCToUnix(S_RTC_TIME_DATA_T *rtc)
{
    struct tm tm_time = {0};

    tm_time.tm_year = rtc->u32Year - 1900;
    tm_time.tm_mon  = rtc->u32Month - 1;
    tm_time.tm_mday = rtc->u32Day;
    tm_time.tm_hour = rtc->u32Hour;
    tm_time.tm_min  = rtc->u32Minute;
    tm_time.tm_sec  = rtc->u32Second;

    return mktime(&tm_time);
}

void RTC_AlarmBEventCallback(hwRTC_Index index)
{
    if (Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_0])
	{
        Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_0](index, hwRTC_Alarm_Channel_Index_0);
	}
}

hwRTC_OpResult RTC_Timer_Init(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if (RTC_HW_Init_Status[index])
        return hwRTC_OK;

    if (NeonRTOS_LockObjCreate(&rtc_access_mutex[index]) != NeonRTOS_OK)
        return hwRTC_MemoryError;

    NeonRTOS_LockObjUnlock(&rtc_access_mutex[index]);

    hwRTC_OpResult result = RTC_Instance_Init(index);

    if (result != hwRTC_OK)
    {
        NeonRTOS_LockObjDelete(&rtc_access_mutex[index]);
        rtc_access_mutex[index] = NULL;
        return result;
    }

    RTC_NVIC_Init();

    RTC_HW_Init_Status[index] = true;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_DeInit(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_OK;

    RTC_NVIC_DeInit();

    hwRTC_OpResult result = RTC_Instance_DeInit(index);

    NeonRTOS_LockObjDelete(&rtc_access_mutex[index]);
    rtc_access_mutex[index] = NULL;

    RTC_HW_Init_Status[index] = false;

    return result;
}

hwRTC_OpResult RTC_Timer_Read(hwRTC_Index index, time_t *unix_time)
{
    if(index >= hwRTC_Index_MAX || unix_time == NULL)
        return hwRTC_InvalidParameter;

    if(!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    S_RTC_TIME_DATA_T rtc_time;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    RTC_GetDateAndTime(&rtc_time);

    *unix_time = RTCToUnix(&rtc_time);

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Write(hwRTC_Index index, time_t unix_time)
{
    if(index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if(!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    S_RTC_TIME_DATA_T rtc_time;

    UnixToRTC(unix_time, &rtc_time);

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    RTC_SetDateAndTime(&rtc_time);

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
    if(index >= hwRTC_Index_MAX || alarm_ch >= hwRTC_Alarm_Channel_Index_MAX)
        return hwRTC_InvalidParameter;

    if(!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    S_RTC_TIME_DATA_T alarm_time;

    UnixToRTC(alarm_unix_time, &alarm_time);

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    Alarm_Event_Callback[alarm_ch] = cb;

    hwRTC_OpResult result =
        RTC_Device_SetAlarm(index, alarm_ch, &alarm_time);

    if(result != hwRTC_OK)
        Alarm_Event_Callback[alarm_ch] = NULL;

    RTC_MUTEX_UNLOCK(index);

    return result;
}

hwRTC_OpResult RTC_Timer_Clear_Alarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch
)
{
    if (index >= hwRTC_Index_MAX || alarm_ch >= hwRTC_Alarm_Channel_Index_MAX)
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    Alarm_Event_Callback[alarm_ch] = NULL;

    hwRTC_OpResult result = RTC_Device_ClearAlarm(index, alarm_ch);

    RTC_MUTEX_UNLOCK(index);

    return result;
}

#endif // DEVICE_NUVOTON