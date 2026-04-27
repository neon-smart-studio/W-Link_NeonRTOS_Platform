#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#ifdef STM32F0

#include "RTC/RTC.h"
#include "RTC_STM32.h"

RTC_HandleTypeDef g_rtc[hwRTC_Index_MAX];

void RTC_Enable_Clock_Source(void)
{
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_PWR_EnableBkUpAccess();
}

hwRTC_OpResult RTC_Instance_Init(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    RTC_Enable_Clock_Source();

    g_rtc[index].Instance = RTC;

    g_rtc[index].Init.HourFormat = RTC_HOURFORMAT_24;
    g_rtc[index].Init.AsynchPrediv = 127;
    g_rtc[index].Init.SynchPrediv  = 255;

    g_rtc[index].Init.OutPut = RTC_OUTPUT_DISABLE;
    g_rtc[index].Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    g_rtc[index].Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&g_rtc[index]) != HAL_OK)
        return hwRTC_HwError;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Instance_DeInit(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    if (HAL_RTC_DeInit(&g_rtc[index]) != HAL_OK)
        return hwRTC_HwError;

    return hwRTC_OK;
}

void RTC_NVIC_Init(void)
{
}

void RTC_NVIC_DeInit(void)
{
}

hwRTC_OpResult RTC_Device_SetAlarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch,
    RTC_TimeTypeDef *time
)
{
    RTC_AlarmTypeDef alarm = {0};

    alarm.AlarmTime = *time;
    alarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;

#if defined(RTC_ALARMSUBSECONDMASK_ALL)
    alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
#endif

    switch (alarm_ch)
    {
        case hwRTC_Alarm_Channel_Index_0:
            alarm.Alarm = RTC_ALARM_A;
            break;

#if defined(RTC_ALARM_B)
        case hwRTC_Alarm_Channel_Index_1:
            alarm.Alarm = RTC_ALARM_B;
            break;
#endif

        default:
            return hwRTC_InvalidParameter;
    }

    if (HAL_RTC_SetAlarm_IT(&g_rtc[index], &alarm, RTC_FORMAT_BIN) != HAL_OK)
        return hwRTC_HwError;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Device_ClearAlarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch
)
{
    switch (alarm_ch)
    {
        case hwRTC_Alarm_Channel_Index_0:
            HAL_RTC_DeactivateAlarm(&g_rtc[index], RTC_ALARM_A);
            break;

#if defined(RTC_ALARM_B)
        case hwRTC_Alarm_Channel_Index_1:
            HAL_RTC_DeactivateAlarm(&g_rtc[index], RTC_ALARM_B);
            break;
#endif

        default:
            return hwRTC_InvalidParameter;
    }

    return hwRTC_OK;
}

#endif // STM32F0