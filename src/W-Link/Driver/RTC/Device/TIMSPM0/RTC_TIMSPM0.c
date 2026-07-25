#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "RTC/RTC.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSPM0

#include "RTC_TIMSPM0.h"

#if defined(RTC_BASE) || defined(RTC_A_BASE) || defined(RTC_B_BASE)

#if defined(RTC_A_BASE)
#define RTC_TIMSPM0_HW             RTC_A_BASE
#define RTC_TIMSPM0_IRQn           LFSS_INT_IRQn
#endif

#if defined(RTC_B_BASE)
#define RTC_TIMSPM0_HW             RTC_B_BASE
#define RTC_TIMSPM0_IRQn           LFSS_INT_IRQn
#endif

#if defined(RTC_BASE)
#define RTC_TIMSPM0_HW             RTC_BASE
#define RTC_TIMSPM0_IRQn           RTC_INT_IRQn
#endif

#define RTC_ALARM_INTERRUPT_MASK                                \
    (DL_RTC_COMMON_INTERRUPT_CALENDAR_ALARM1 |                   \
     DL_RTC_COMMON_INTERRUPT_CALENDAR_ALARM2)

/*===========================================================
 * Driver state
 *==========================================================*/

static bool RTC_HW_Init_Status[hwRTC_Index_MAX] = {false};

static NeonRTOS_LockObj_t rtc_access_mutex[hwRTC_Index_MAX];

static onAlarmEventCallback volatile
    Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_MAX] = {NULL};

/*===========================================================
 * Mutex
 *==========================================================*/

#define RTC_MUTEX_LOCK(index, timeout)                           \
    do {                                                         \
        if (NeonRTOS_LockObjLock(                                \
                &rtc_access_mutex[(index)],                      \
                (timeout)) != NeonRTOS_OK) {                     \
            return hwRTC_MutexTimeout;                           \
        }                                                        \
    } while (0)

#define RTC_MUTEX_UNLOCK(index)                                  \
    do {                                                         \
        NeonRTOS_LockObjUnlock(&rtc_access_mutex[(index)]);      \
    } while (0)

/*===========================================================
 * Calendar conversion
 *==========================================================*/

static int64_t RTC_DaysFromCivil(
    int year,
    unsigned month,
    unsigned day
)
{
    year -= (month <= 2U);

    const int era =
        (year >= 0 ? year : year - 399) / 400;

    const unsigned year_of_era =
        (unsigned)(year - era * 400);

    const unsigned month_position =
        (month > 2U) ? (month - 3U) : (month + 9U);

    const unsigned day_of_year =
        (153U * month_position + 2U) / 5U + day - 1U;

    const unsigned day_of_era =
        year_of_era * 365U +
        year_of_era / 4U -
        year_of_era / 100U +
        day_of_year;

    return
        (int64_t)era * 146097LL +
        (int64_t)day_of_era -
        719468LL;
}

static time_t RTC_Calendar_To_Unix(
    const DL_RTC_Common_Calendar *calendar
)
{
    int64_t days = RTC_DaysFromCivil(
        calendar->year,
        calendar->month,
        calendar->dayOfMonth
    );

    int64_t unix_time =
        days * 86400LL +
        (int64_t)calendar->hours * 3600LL +
        (int64_t)calendar->minutes * 60LL +
        calendar->seconds;

    return (time_t)unix_time;
}

static bool RTC_Unix_To_Calendar(
    time_t unix_time,
    DL_RTC_Common_Calendar *calendar
)
{
    if (calendar == NULL)
    {
        return false;
    }

    int64_t unix_seconds = (int64_t)unix_time;

    int64_t days = unix_seconds / 86400LL;
    int64_t seconds_of_day = unix_seconds % 86400LL;

    if (seconds_of_day < 0)
    {
        seconds_of_day += 86400LL;
        days--;
    }

    int64_t date_value = days + 719468LL;

    const int64_t era =
        (date_value >= 0 ?
            date_value :
            date_value - 146096LL) / 146097LL;

    const unsigned day_of_era =
        (unsigned)(date_value - era * 146097LL);

    const unsigned year_of_era =
        (day_of_era -
         day_of_era / 1460U +
         day_of_era / 36524U -
         day_of_era / 146096U) / 365U;

    int year =
        (int)year_of_era + (int)(era * 400LL);

    const unsigned day_of_year =
        day_of_era -
        (365U * year_of_era +
         year_of_era / 4U -
         year_of_era / 100U);

    const unsigned month_position =
        (5U * day_of_year + 2U) / 153U;

    const unsigned day =
        day_of_year -
        (153U * month_position + 2U) / 5U +
        1U;

    const unsigned month =
        (month_position < 10U) ?
            (month_position + 3U) :
            (month_position - 9U);

    year += (month <= 2U);

    if (year < 0 || year > 4095)
    {
        return false;
    }

    int day_of_week = (int)((days + 4LL) % 7LL);

    if (day_of_week < 0)
    {
        day_of_week += 7;
    }

    calendar->seconds =
        (uint8_t)(seconds_of_day % 60LL);

    calendar->minutes =
        (uint8_t)((seconds_of_day / 60LL) % 60LL);

    calendar->hours =
        (uint8_t)(seconds_of_day / 3600LL);

    calendar->dayOfWeek =
        (uint8_t)day_of_week;

    calendar->dayOfMonth =
        (uint8_t)day;

    calendar->month =
        (uint8_t)month;

    calendar->year =
        (uint16_t)year;

    return true;
}

/*===========================================================
 * Alarm hardware abstraction
 *==========================================================*/

static uint32_t RTC_Alarm_GetInterruptMask(
    hwRTC_Alarm_Channel_Index alarm_ch
)
{
    switch (alarm_ch)
    {
        case hwRTC_Alarm_Channel_Index_0:
            return DL_RTC_COMMON_INTERRUPT_CALENDAR_ALARM1;

        case hwRTC_Alarm_Channel_Index_1:
            return DL_RTC_COMMON_INTERRUPT_CALENDAR_ALARM2;

        default:
            return 0U;
    }
}

static void RTC_Alarm_DisableHardware(
    hwRTC_Alarm_Channel_Index alarm_ch
)
{
    uint32_t interrupt_mask =
        RTC_Alarm_GetInterruptMask(alarm_ch);

    DL_RTC_Common_disableInterrupt(
        RTC_TIMSPM0_HW,
        interrupt_mask
    );

    DL_RTC_Common_clearInterruptStatus(
        RTC_TIMSPM0_HW,
        interrupt_mask
    );

    switch (alarm_ch)
    {
        case hwRTC_Alarm_Channel_Index_0:
            DL_RTC_Common_disableCalendarAlarm1(
                RTC_TIMSPM0_HW
            );
            break;

        case hwRTC_Alarm_Channel_Index_1:
            DL_RTC_Common_disableCalendarAlarm2(
                RTC_TIMSPM0_HW
            );
            break;

        default:
            break;
    }
}

static void RTC_Alarm_SetHardware(
    hwRTC_Alarm_Channel_Index alarm_ch,
    const DL_RTC_Common_Calendar *calendar
)
{
    DL_RTC_Common_CalendarAlarm alarm_time = {
        .minutes    = calendar->minutes,
        .hours      = calendar->hours,
        .dayOfWeek  = calendar->dayOfWeek,
        .dayOfMonth = calendar->dayOfMonth
    };

    RTC_Alarm_DisableHardware(alarm_ch);

    switch (alarm_ch)
    {
        case hwRTC_Alarm_Channel_Index_0:
            DL_RTC_Common_setCalendarAlarm1(
                RTC_TIMSPM0_HW,
                alarm_time
            );

            /*
             * 維持原 MSP432E 行為：
             * 比較 Minute、Hour、DayOfMonth，
             * 不比較 DayOfWeek。
             */
            DL_RTC_Common_enableAlarm1MinutesBinary(
                RTC_TIMSPM0_HW
            );

            DL_RTC_Common_enableAlarm1HoursBinary(
                RTC_TIMSPM0_HW
            );

            DL_RTC_Common_disableAlarm1DayOfWeekBinary(
                RTC_TIMSPM0_HW
            );

            DL_RTC_Common_enableAlarm1DayOfMonthBinary(
                RTC_TIMSPM0_HW
            );
            break;

        case hwRTC_Alarm_Channel_Index_1:
            DL_RTC_Common_setCalendarAlarm2(
                RTC_TIMSPM0_HW,
                alarm_time
            );

            DL_RTC_Common_enableAlarm2MinutesBinary(
                RTC_TIMSPM0_HW
            );

            DL_RTC_Common_enableAlarm2HoursBinary(
                RTC_TIMSPM0_HW
            );

            DL_RTC_Common_disableAlarm2DayOfWeekBinary(
                RTC_TIMSPM0_HW
            );

            DL_RTC_Common_enableAlarm2DayOfMonthBinary(
                RTC_TIMSPM0_HW
            );
            break;

        default:
            return;
    }

    DL_RTC_Common_clearInterruptStatus(
        RTC_TIMSPM0_HW,
        RTC_Alarm_GetInterruptMask(alarm_ch)
    );

    DL_RTC_Common_enableInterrupt(
        RTC_TIMSPM0_HW,
        RTC_Alarm_GetInterruptMask(alarm_ch)
    );
}

/*===========================================================
 * Interrupt handler
 *==========================================================*/

#if defined(RTC_BASE)
void RTC_IRQHandler(void)
{
    uint32_t status =
        DL_RTC_Common_getEnabledInterruptStatus(
            RTC_TIMSPM0_HW,
            RTC_ALARM_INTERRUPT_MASK
        );

    if ((status &
         DL_RTC_COMMON_INTERRUPT_CALENDAR_ALARM1) != 0U)
    {
        RTC_Alarm_DisableHardware(
            hwRTC_Alarm_Channel_Index_0
        );

        onAlarmEventCallback callback =
            Alarm_Event_Callback[
                hwRTC_Alarm_Channel_Index_0
            ];

        if (callback != NULL)
        {
            callback(
                hwRTC_Index_0,
                hwRTC_Alarm_Channel_Index_0
            );
        }
    }

    if ((status &
         DL_RTC_COMMON_INTERRUPT_CALENDAR_ALARM2) != 0U)
    {
        RTC_Alarm_DisableHardware(
            hwRTC_Alarm_Channel_Index_1
        );

        onAlarmEventCallback callback =
            Alarm_Event_Callback[
                hwRTC_Alarm_Channel_Index_1
            ];

        if (callback != NULL)
        {
            callback(
                hwRTC_Index_0,
                hwRTC_Alarm_Channel_Index_1
            );
        }
    }
}
#endif
#if defined(RTC_A_BASE) || defined(RTC_B_BASE)
void LFSS_IRQHandler(void)
{
    uint32_t status =
        DL_RTC_Common_getEnabledInterruptStatus(
            RTC_TIMSPM0_HW,
            RTC_ALARM_INTERRUPT_MASK
        );

    if ((status &
         DL_RTC_COMMON_INTERRUPT_CALENDAR_ALARM1) != 0U)
    {
        RTC_Alarm_DisableHardware(
            hwRTC_Alarm_Channel_Index_0
        );

        onAlarmEventCallback callback =
            Alarm_Event_Callback[
                hwRTC_Alarm_Channel_Index_0
            ];

        if (callback != NULL)
        {
            callback(
                hwRTC_Index_0,
                hwRTC_Alarm_Channel_Index_0
            );
        }
    }

    if ((status &
         DL_RTC_COMMON_INTERRUPT_CALENDAR_ALARM2) != 0U)
    {
        RTC_Alarm_DisableHardware(
            hwRTC_Alarm_Channel_Index_1
        );

        onAlarmEventCallback callback =
            Alarm_Event_Callback[
                hwRTC_Alarm_Channel_Index_1
            ];

        if (callback != NULL)
        {
            callback(
                hwRTC_Index_0,
                hwRTC_Alarm_Channel_Index_1
            );
        }
    }
}
#endif

/*===========================================================
 * Public API
 *==========================================================*/

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

    if (NeonRTOS_LockObjCreate(
            &rtc_access_mutex[index]) != NeonRTOS_OK)
    {
        return hwRTC_MemoryError;
    }

    NeonRTOS_LockObjUnlock(
        &rtc_access_mutex[index]
    );

    DL_RTC_Common_enablePower(
        RTC_TIMSPM0_HW
    );

    /*
     * Peripheral power startup delay。
     * TI DriverLib 一般要求至少等待數個 CPU cycles。
     */
    DL_Common_delayCycles(16U);

    DL_RTC_Common_reset(
        RTC_TIMSPM0_HW
    );

    while (!DL_RTC_Common_isReset(
        RTC_TIMSPM0_HW))
    {
    }

    /*
     * SysCtrl 必須事先準備 LFCLK。
     * 這裡只打開 RTC 的 32 kHz clock gate。
     */
    DL_RTC_Common_enableClockControl(
        RTC_TIMSPM0_HW
    );

    DL_RTC_Common_Calendar init_time = {
        .seconds    = 0,
        .minutes    = 0,
        .hours      = 0,
        .dayOfWeek  = 4,
        .dayOfMonth = 1,
        .month      = 1,
        .year       = 1970
    };

    DL_RTC_Common_initCalendar(
        RTC_TIMSPM0_HW,
        init_time,
        DL_RTC_COMMON_FORMAT_BINARY
    );

    DL_RTC_Common_disableCalendarAlarm1(
        RTC_TIMSPM0_HW
    );

    DL_RTC_Common_disableCalendarAlarm2(
        RTC_TIMSPM0_HW
    );

    DL_RTC_Common_disableInterrupt(
        RTC_TIMSPM0_HW,
        RTC_ALARM_INTERRUPT_MASK
    );

    DL_RTC_Common_clearInterruptStatus(
        RTC_TIMSPM0_HW,
        RTC_ALARM_INTERRUPT_MASK
    );

    for (uint32_t i = 0;
         i < hwRTC_Alarm_Channel_Index_MAX;
         i++)
    {
        Alarm_Event_Callback[i] = NULL;
    }

    NVIC_ClearPendingIRQ(
        RTC_TIMSPM0_IRQn
    );

    NVIC_EnableIRQ(
        RTC_TIMSPM0_IRQn
    );

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

    RTC_MUTEX_LOCK(
        index,
        RTC_MUTEX_ACCESS_TIMEOUT
    );

    NVIC_DisableIRQ(
        RTC_TIMSPM0_IRQn
    );

    RTC_Alarm_DisableHardware(
        hwRTC_Alarm_Channel_Index_0
    );

    RTC_Alarm_DisableHardware(
        hwRTC_Alarm_Channel_Index_1
    );

    for (uint32_t i = 0;
         i < hwRTC_Alarm_Channel_Index_MAX;
         i++)
    {
        Alarm_Event_Callback[i] = NULL;
    }

    DL_RTC_Common_disableClockControl(
        RTC_TIMSPM0_HW
    );

    DL_RTC_Common_disablePower(
        RTC_TIMSPM0_HW
    );

    NVIC_ClearPendingIRQ(
        RTC_TIMSPM0_IRQn
    );

    RTC_MUTEX_UNLOCK(index);

    NeonRTOS_LockObjDelete(
        &rtc_access_mutex[index]
    );

    rtc_access_mutex[index] = NULL;

    RTC_HW_Init_Status[index] = false;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Read(
    hwRTC_Index index,
    time_t *unix_time
)
{
    if (index >= hwRTC_Index_MAX ||
        unix_time == NULL)
    {
        return hwRTC_InvalidParameter;
    }

    if (!RTC_HW_Init_Status[index])
    {
        return hwRTC_NotInit;
    }

    RTC_MUTEX_LOCK(
        index,
        RTC_MUTEX_ACCESS_TIMEOUT
    );

    /*
     * RTC 在每秒更新附近有短暫 keep-out window。
     */
    while (!DL_RTC_Common_isSafeToRead(
        RTC_TIMSPM0_HW))
    {
    }

    DL_RTC_Common_Calendar calendar =
        DL_RTC_Common_getCalendarTime(
            RTC_TIMSPM0_HW
        );

    *unix_time =
        RTC_Calendar_To_Unix(&calendar);

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Write(
    hwRTC_Index index,
    time_t unix_time
)
{
    if (index >= hwRTC_Index_MAX)
    {
        return hwRTC_InvalidParameter;
    }

    if (!RTC_HW_Init_Status[index])
    {
        return hwRTC_NotInit;
    }

    DL_RTC_Common_Calendar calendar;

    if (!RTC_Unix_To_Calendar(
            unix_time,
            &calendar))
    {
        return hwRTC_InvalidParameter;
    }

    RTC_MUTEX_LOCK(
        index,
        RTC_MUTEX_ACCESS_TIMEOUT
    );

    DL_RTC_Common_initCalendar(
        RTC_TIMSPM0_HW,
        calendar,
        DL_RTC_COMMON_FORMAT_BINARY
    );

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Set_Alarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch,
    time_t alarm_unix_time,
    onAlarmEventCallback callback
)
{
    if (index >= hwRTC_Index_MAX ||
        alarm_ch >= hwRTC_Alarm_Channel_Index_MAX)
    {
        return hwRTC_InvalidParameter;
    }

    if (!RTC_HW_Init_Status[index])
    {
        return hwRTC_NotInit;
    }

    DL_RTC_Common_Calendar alarm_calendar;

    if (!RTC_Unix_To_Calendar(
            alarm_unix_time,
            &alarm_calendar))
    {
        return hwRTC_InvalidParameter;
    }

    RTC_MUTEX_LOCK(
        index,
        RTC_MUTEX_ACCESS_TIMEOUT
    );

    Alarm_Event_Callback[alarm_ch] =
        callback;

    RTC_Alarm_SetHardware(
        alarm_ch,
        &alarm_calendar
    );

    NVIC_ClearPendingIRQ(
        RTC_TIMSPM0_IRQn
    );

    NVIC_EnableIRQ(
        RTC_TIMSPM0_IRQn
    );

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Clear_Alarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch
)
{
    if (index >= hwRTC_Index_MAX ||
        alarm_ch >= hwRTC_Alarm_Channel_Index_MAX)
    {
        return hwRTC_InvalidParameter;
    }

    if (!RTC_HW_Init_Status[index])
    {
        return hwRTC_NotInit;
    }

    RTC_MUTEX_LOCK(
        index,
        RTC_MUTEX_ACCESS_TIMEOUT
    );

    Alarm_Event_Callback[alarm_ch] =
        NULL;

    RTC_Alarm_DisableHardware(
        alarm_ch
    );

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

#endif

#endif // DEVICE_TIMSPM0