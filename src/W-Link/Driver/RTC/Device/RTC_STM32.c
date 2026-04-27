#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "RTC/RTC.h"

#ifdef DEVICE_STM32

#include "RTC_STM32.h"

static bool RTC_HW_Init_Status[hwRTC_Index_MAX] = {false};
static NeonRTOS_LockObj_t rtc_access_mutex[hwRTC_Index_MAX];

onAlarmEventCallback Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_MAX] = {NULL};

#define RTC_MUTEX_LOCK(a, b)  \
    if (NeonRTOS_LockObjLock(&rtc_access_mutex[a], b) != NeonRTOS_OK) { return hwRTC_MutexTimeout; }

#define RTC_MUTEX_UNLOCK(a)   \
    NeonRTOS_LockObjUnlock(&rtc_access_mutex[a]);

static void UnixToRTC(time_t t, RTC_TimeTypeDef *time, RTC_DateTypeDef *date)
{
    struct tm tm_time;
    gmtime_r(&t, &tm_time);

    time->Hours          = tm_time.tm_hour;
    time->Minutes        = tm_time.tm_min;
    time->Seconds        = tm_time.tm_sec;
#ifndef STM32F1
    time->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    time->StoreOperation = RTC_STOREOPERATION_RESET;
#endif

    date->Year    = tm_time.tm_year - 100;
    date->Month   = tm_time.tm_mon + 1;
    date->Date    = tm_time.tm_mday;
    date->WeekDay = tm_time.tm_wday + 1;

    if (date->WeekDay == 1)
        date->WeekDay = RTC_WEEKDAY_SUNDAY;
}

static time_t RTCToUnix(RTC_TimeTypeDef *time, RTC_DateTypeDef *date)
{
    struct tm tm_time = {0};

    tm_time.tm_year = date->Year + 100;
    tm_time.tm_mon  = date->Month - 1;
    tm_time.tm_mday = date->Date;
    tm_time.tm_hour = time->Hours;
    tm_time.tm_min  = time->Minutes;
    tm_time.tm_sec  = time->Seconds;

    return mktime(&tm_time);
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
    if (index >= hwRTC_Index_MAX || unix_time == NULL)
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    HAL_RTC_GetTime(&g_rtc[index], &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&g_rtc[index], &date, RTC_FORMAT_BIN);

    *unix_time = RTCToUnix(&time, &date);

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Write(hwRTC_Index index, time_t unix_time)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    UnixToRTC(unix_time, &time, &date);

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    if (HAL_RTC_SetTime(&g_rtc[index], &time, RTC_FORMAT_BIN) != HAL_OK)
    {
        RTC_MUTEX_UNLOCK(index);
        return hwRTC_HwError;
    }

    if (HAL_RTC_SetDate(&g_rtc[index], &date, RTC_FORMAT_BIN) != HAL_OK)
    {
        RTC_MUTEX_UNLOCK(index);
        return hwRTC_HwError;
    }

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
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    UnixToRTC(alarm_unix_time, &time, &date);

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    Alarm_Event_Callback[alarm_ch] = cb;

    hwRTC_OpResult result = RTC_Device_SetAlarm(index, alarm_ch, &time);

    if (result != hwRTC_OK)
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

#endif // DEVICE_STM32