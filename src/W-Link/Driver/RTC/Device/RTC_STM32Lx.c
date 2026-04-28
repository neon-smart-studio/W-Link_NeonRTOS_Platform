#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#if defined(STM32L0) || defined(STM32L1) || defined(STM32L4) || defined(STM32L5)

#include "RTC/RTC.h"
#include "RTC_STM32.h"

RTC_HandleTypeDef g_rtc[hwRTC_Index_MAX];

/* ================= IRQ Handler ================= */

#if defined(RTC_TAMP_IRQn)
void RTC_TAMP_IRQHandler(void)
{
    HAL_RTC_AlarmIRQHandler(&g_rtc[hwRTC_Index_0]);
}
#elif defined(RTC_Alarm_IRQn)
void RTC_Alarm_IRQHandler(void)
{
    HAL_RTC_AlarmIRQHandler(&g_rtc[hwRTC_Index_0]);
}
#endif

/* ================= Init / DeInit ================= */

hwRTC_OpResult RTC_Instance_Init(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

#if defined(__HAL_RCC_LSI_ENABLE)
    __HAL_RCC_LSI_ENABLE();
#endif

#if defined(__HAL_RCC_RTC_CONFIG)
    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSI);
#endif

    __HAL_RCC_RTC_ENABLE();

    g_rtc[index].Instance = RTC;

    g_rtc[index].Init.HourFormat   = RTC_HOURFORMAT_24;
    g_rtc[index].Init.AsynchPrediv = 127;
    g_rtc[index].Init.SynchPrediv  = 255;

#if defined(RTC_OUTPUT_DISABLE)
    g_rtc[index].Init.OutPut = RTC_OUTPUT_DISABLE;
#endif

#if defined(RTC_OUTPUT_POLARITY_HIGH)
    g_rtc[index].Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
#endif

#if defined(RTC_OUTPUT_TYPE_OPENDRAIN)
    g_rtc[index].Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
#endif

#if defined(RTC_OUTPUT_REMAP_NONE)
    g_rtc[index].Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
#endif

    if (HAL_RTC_Init(&g_rtc[index]) != HAL_OK)
        return hwRTC_HwError;

    return hwRTC_OK;
}

hwRTC_OpResult RTC_Instance_DeInit(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

#if defined(RTC_ALARM_A)
    HAL_RTC_DeactivateAlarm(&g_rtc[index], RTC_ALARM_A);
#endif

#if defined(RTC_ALARM_B)
    HAL_RTC_DeactivateAlarm(&g_rtc[index], RTC_ALARM_B);
#endif

    if (HAL_RTC_DeInit(&g_rtc[index]) != HAL_OK)
        return hwRTC_HwError;

    __HAL_RCC_RTC_DISABLE();

    return hwRTC_OK;
}

/* ================= NVIC ================= */

void RTC_NVIC_Init(void)
{
#if defined(RTC_TAMP_IRQn)
    HAL_NVIC_SetPriority(
        RTC_TAMP_IRQn,
        RTC_IRQ_NVIC_PRIORITY,
        RTC_IRQ_NVIC_SUB_PRIORITY
    );
    HAL_NVIC_EnableIRQ(RTC_TAMP_IRQn);

#elif defined(RTC_Alarm_IRQn)
    HAL_NVIC_SetPriority(
        RTC_Alarm_IRQn,
        RTC_IRQ_NVIC_PRIORITY,
        RTC_IRQ_NVIC_SUB_PRIORITY
    );
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
#endif
}

void RTC_NVIC_DeInit(void)
{
#if defined(RTC_TAMP_IRQn)
    HAL_NVIC_DisableIRQ(RTC_TAMP_IRQn);
#elif defined(RTC_Alarm_IRQn)
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
#endif
}

/* ================= Alarm ================= */

hwRTC_OpResult RTC_Device_SetAlarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch,
    RTC_TimeTypeDef *time
)
{
    if (index >= hwRTC_Index_MAX || time == NULL)
        return hwRTC_InvalidParameter;

    RTC_AlarmTypeDef alarm = {0};

    alarm.AlarmTime = *time;
    alarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;

#if defined(RTC_ALARMSUBSECONDMASK_ALL)
    alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
#endif

    switch (alarm_ch)
    {
#if defined(RTC_ALARM_A)
        case hwRTC_Alarm_Channel_Index_0:
            alarm.Alarm = RTC_ALARM_A;
            break;
#endif

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
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    uint32_t alarm;

    switch (alarm_ch)
    {
#if defined(RTC_ALARM_A)
        case hwRTC_Alarm_Channel_Index_0:
            alarm = RTC_ALARM_A;
            break;
#endif

#if defined(RTC_ALARM_B)
        case hwRTC_Alarm_Channel_Index_1:
            alarm = RTC_ALARM_B;
            break;
#endif

        default:
            return hwRTC_InvalidParameter;
    }

    if (HAL_RTC_DeactivateAlarm(&g_rtc[index], alarm) != HAL_OK)
        return hwRTC_HwError;

    return hwRTC_OK;
}

#endif /* STM32L0 || STM32L1 || STM32L4 || STM32L5 */