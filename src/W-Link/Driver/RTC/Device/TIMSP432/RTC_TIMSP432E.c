#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "RTC/RTC.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSP432E

#include "RTC_TIMSP432.h"

static bool RTC_HW_Init_Status[hwRTC_Index_MAX] = {false};
static NeonRTOS_LockObj_t rtc_access_mutex[hwRTC_Index_MAX];

static onAlarmEventCallback Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_MAX] = {NULL};

#define RTC_MUTEX_LOCK(a, b)  \
    if (NeonRTOS_LockObjLock(&rtc_access_mutex[a], b) != NeonRTOS_OK) { return hwRTC_MutexTimeout; }

#define RTC_MUTEX_UNLOCK(a)   \
    NeonRTOS_LockObjUnlock(&rtc_access_mutex[a]);

static void RTC_IRQ_Handler(void)
{
    uint32_t status = MAP_HibernateIntStatus(true);

    MAP_HibernateIntClear(status);

    if (status & HIBERNATE_INT_RTC_MATCH_0) {
        MAP_HibernateIntDisable(HIBERNATE_INT_RTC_MATCH_0);

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
        return hwRTC_InvalidParameter;

    if (RTC_HW_Init_Status[index])
        return hwRTC_OK;

    if (index != hwRTC_Index_0)
        return hwRTC_Unsupport;

    if (NeonRTOS_LockObjCreate(&rtc_access_mutex[index]) != NeonRTOS_OK)
        return hwRTC_MemoryError;

    NeonRTOS_LockObjUnlock(&rtc_access_mutex[index]);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_HIBERNATE));

    MAP_HibernateEnableExpClk(g_sys_clock_hz);

    /*
     * 使用 32.768 kHz 外部晶振。
     * 如果你的板子沒有接 32k 晶振，這裡要改成其他 clock config。
     */
    MAP_HibernateClockConfig(HIBERNATE_OSC_LOWDRIVE);

    MAP_HibernateRTCEnable();

    MAP_HibernateIntRegister(RTC_IRQ_Handler);
    MAP_HibernateIntClear(HibernateIntStatus(false));

    RTC_HW_Init_Status[index] = true;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_DeInit(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_OK;

    if (index != hwRTC_Index_0)
        return hwRTC_Unsupport;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    MAP_HibernateIntDisable(HIBERNATE_INT_RTC_MATCH_0);
    MAP_HibernateIntClear(MAP_HibernateIntStatus(false));
    MAP_HibernateRTCDisable();

    Alarm_Event_Callback[hwRTC_Alarm_Channel_Index_0] = NULL;

    RTC_MUTEX_UNLOCK(index);

    HibernateIntUnregister();

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

    if (index != hwRTC_Index_0)
        return hwRTC_Unsupport;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    *unix_time = (time_t)MAP_HibernateRTCGet();

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Timer_Write(hwRTC_Index index, time_t unix_time)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    if (index != hwRTC_Index_0)
        return hwRTC_Unsupport;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    MAP_HibernateRTCSet((uint32_t)unix_time);

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

    if (index != hwRTC_Index_0)
        return hwRTC_Unsupport;

    if (alarm_ch != hwRTC_Alarm_Channel_Index_0)
        return hwRTC_Unsupport;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    Alarm_Event_Callback[alarm_ch] = cb;

    MAP_HibernateIntDisable(HIBERNATE_INT_RTC_MATCH_0);
    MAP_HibernateIntClear(HIBERNATE_INT_RTC_MATCH_0);

    MAP_HibernateRTCMatchSet(0, (uint32_t)alarm_unix_time);

    MAP_HibernateIntEnable(HIBERNATE_INT_RTC_MATCH_0);
    MAP_IntEnable(INT_HIBERNATE);

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

    if (index != hwRTC_Index_0)
        return hwRTC_Unsupport;

    if (alarm_ch != hwRTC_Alarm_Channel_Index_0)
        return hwRTC_Unsupport;

    if (!RTC_HW_Init_Status[index])
        return hwRTC_NotInit;

    RTC_MUTEX_LOCK(index, RTC_MUTEX_ACCESS_TIMEOUT);

    Alarm_Event_Callback[alarm_ch] = NULL;

    MAP_HibernateIntDisable(HIBERNATE_INT_RTC_MATCH_0);
    MAP_HibernateIntClear(HIBERNATE_INT_RTC_MATCH_0);

    RTC_MUTEX_UNLOCK(index);

    return hwRTC_OK;
}

#endif // DEVICE_TIMSP432E