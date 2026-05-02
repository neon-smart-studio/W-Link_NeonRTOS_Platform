#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#ifdef STM32WB

#include "RTC/RTC.h"
#include "RTC_STM32.h"

RTC_HandleTypeDef g_rtc[hwRTC_Index_MAX];

void RTC_Alarm_IRQHandler(void)
{
    HAL_RTC_AlarmIRQHandler(&g_rtc[hwRTC_Index_0]);
}

void RTC_Enable_Clock_Source(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_LSI_ENABLE();

#if defined(RCC_FLAG_LSI1RDY)
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSI1RDY) == RESET) {
    }
#elif defined(RCC_FLAG_LSI2RDY)
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSI2RDY) == RESET) {
    }
#endif

    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSI);
    __HAL_RCC_RTC_ENABLE();
}

hwRTC_OpResult RTC_Instance_Init(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    RTC_Enable_Clock_Source();

    g_rtc[index].Instance = RTC;

    g_rtc[index].Init.HourFormat = RTC_HOURFORMAT_24;

    /*
     * LSI 約 32kHz:
     * 32000 / (127 + 1) / (249 + 1) = 1Hz
     *
     * LSE 32768Hz:
     * AsynchPrediv = 127
     * SynchPrediv  = 255
     */
    g_rtc[index].Init.AsynchPrediv = 127;
    g_rtc[index].Init.SynchPrediv  = 249;

    g_rtc[index].Init.OutPut         = RTC_OUTPUT_DISABLE;
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

    HAL_RTC_DeactivateAlarm(&g_rtc[index], RTC_ALARM_A);

    HAL_RTC_DeactivateAlarm(&g_rtc[index], RTC_ALARM_B);

    if (HAL_RTC_DeInit(&g_rtc[index]) != HAL_OK)
        return hwRTC_HwError;

    return hwRTC_OK;
}

void RTC_NVIC_Init(void)
{
    HAL_NVIC_SetPriority(
        RTC_Alarm_IRQn,
        RTC_IRQ_NVIC_PRIORITY,
        RTC_IRQ_NVIC_SUB_PRIORITY
    );

    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

void RTC_NVIC_DeInit(void)
{
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
}

hwRTC_OpResult RTC_Device_SetAlarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch,
    RTC_TimeTypeDef *time
)
{
    if (index >= hwRTC_Index_MAX || time == NULL)
        return hwRTC_InvalidParameter;

    if (alarm_ch != hwRTC_Alarm_Channel_Index_0)
        return hwRTC_InvalidParameter;

    RTC_AlarmTypeDef alarm = {0};

    alarm.AlarmTime = *time;
    alarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;

    alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;

    switch (alarm_ch)
    {
        case hwRTC_Alarm_Channel_Index_0:
            alarm.Alarm = RTC_ALARM_A;
            break;

        case hwRTC_Alarm_Channel_Index_1:
            alarm.Alarm = RTC_ALARM_B;
            break;

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
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    uint32_t alarm;

    switch (alarm_ch)
    {
        case hwRTC_Alarm_Channel_Index_0:
            alarm = RTC_ALARM_A;
            break;

        case hwRTC_Alarm_Channel_Index_1:
            alarm = RTC_ALARM_B;
            break;

        default:
            return hwRTC_InvalidParameter;
    }

    if (HAL_RTC_DeactivateAlarm(&g_rtc[index], alarm) != HAL_OK)
        return hwRTC_HwError;

    return hwRTC_OK;
}

#endif // STM32WB