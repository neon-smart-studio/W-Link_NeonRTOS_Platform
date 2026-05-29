#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#ifdef NUC472

#include "RTC/RTC.h"
#include "RTC_Nuvoton.h"

void RTC_IRQHandler(void)
{
    if(RTC_GET_TICK_INT_FLAG)
    {
        RTC_CLEAR_TICK_INT_FLAG;
    }

    if(RTC_GET_ALARM_INT_FLAG)
    {
        RTC_CLEAR_ALARM_INT_FLAG;

        /*
         * 如果你上層有 alarm callback，
         * 可以在這裡呼叫。
         */
        RTC_AlarmBEventCallback(hwRTC_Index_0);
    }
}

void RTC_Enable_Clock_Source(void)
{
    SYS_UnlockReg();

    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);

    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);

    CLK_EnableModuleClock(RTC_MODULE);

    SYS_LockReg();
}

hwRTC_OpResult RTC_Instance_Init(hwRTC_Index index)
{
    if(index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    RTC_Enable_Clock_Source();

    /*
     * Open RTC with 24-hour format.
     * 初始時間可依你上層 API 再設定。
     */
    S_RTC_TIME_DATA_T time = {0};

    time.u32Year       = 2026;
    time.u32Month      = 1;
    time.u32Day        = 1;
    time.u32Hour       = 0;
    time.u32Minute     = 0;
    time.u32Second     = 0;
    time.u32DayOfWeek  = RTC_WEDNESDAY;
    time.u32TimeScale  = RTC_CLOCK_24;

    RTC_Open(&time);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Instance_DeInit(hwRTC_Index index)
{
    if(index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    RTC_DisableInt(RTC_INTEN_ALMIEN_Msk |
                   RTC_INTEN_TICKIEN_Msk);

    RTC_CLEAR_ALARM_INT_FLAG;
    RTC_CLEAR_TICK_INT_FLAG;

    CLK_DisableModuleClock(RTC_MODULE);

    return hwRTC_OK;
}

void RTC_NVIC_Init(void)
{
    NVIC_SetPriority(RTC_IRQn, RTC_IRQ_NVIC_PRIORITY);
    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_EnableIRQ(RTC_IRQn);
}

void RTC_NVIC_DeInit(void)
{
    NVIC_DisableIRQ(RTC_IRQn);
    NVIC_ClearPendingIRQ(RTC_IRQn);
}

hwRTC_OpResult RTC_Device_SetAlarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch,
    S_RTC_TIME_DATA_T *time
)
{
    if(index >= hwRTC_Index_MAX || time == NULL)
        return hwRTC_InvalidParameter;

    if(alarm_ch != hwRTC_Alarm_Channel_Index_0)
        return hwRTC_InvalidParameter;

    RTC_SetAlarmDateAndTime(time);

    RTC_CLEAR_ALARM_INT_FLAG;

    RTC_EnableInt(RTC_INTEN_ALMIEN_Msk);

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Device_ClearAlarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch
)
{
    if(index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if(alarm_ch != hwRTC_Alarm_Channel_Index_0)
        return hwRTC_InvalidParameter;

    RTC_DisableInt(RTC_INTEN_ALMIEN_Msk);
    RTC_CLEAR_ALARM_INT_FLAG;

    return hwRTC_OK;
}

#endif /* NUC472 */