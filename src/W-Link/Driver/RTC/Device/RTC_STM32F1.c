#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#ifdef STM32F1

#include "RTC/RTC.h"
#include "RTC_STM32.h"

RTC_HandleTypeDef g_rtc[hwRTC_Index_MAX];

void RTC_Enable_Clock_Source(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();

    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);

    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET)
    {
    }

    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
    __HAL_RCC_RTC_ENABLE();
}

hwRTC_OpResult RTC_Instance_Init(hwRTC_Index index)
{
    if (index >= hwRTC_Index_MAX)
        return hwRTC_InvalidParameter;

    RTC_Enable_Clock_Source();

    g_rtc[index].Instance = RTC;

    if (HAL_RTC_Init(&g_rtc[index]) != HAL_OK)
        return hwRTC_HwError;

    if (HAL_RTC_WaitForSynchro(&g_rtc[index]) != HAL_OK)
        return hwRTC_HwError;

    __HAL_RTC_WRITEPROTECTION_DISABLE(&g_rtc[index]);

    if (RTC_EnterInitMode(&g_rtc[index]) != HAL_OK)
    {
        __HAL_RTC_WRITEPROTECTION_ENABLE(&g_rtc[index]);
        return hwRTC_HwError;
    }

    g_rtc[index].Instance->PRLH = (uint16_t)(32767 >> 16);
    g_rtc[index].Instance->PRLL = (uint16_t)(32767 & 0xFFFF);

    if (RTC_ExitInitMode(&g_rtc[index]) != HAL_OK)
    {
        __HAL_RTC_WRITEPROTECTION_ENABLE(&g_rtc[index]);
        return hwRTC_HwError;
    }

    __HAL_RTC_WRITEPROTECTION_ENABLE(&g_rtc[index]);

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
    /*
     * STM32F1 basic RTC read/write path does not need alarm IRQ.
     * Add RTCAlarm_IRQn here only if RTC_SUPPORT_ALARM is enabled later.
     */
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
    (void)index;
    (void)alarm_ch;
    (void)time;

    return hwRTC_Unsupport;
}

hwRTC_OpResult RTC_Device_ClearAlarm(
    hwRTC_Index index,
    hwRTC_Alarm_Channel_Index alarm_ch
)
{
    (void)index;
    (void)alarm_ch;

    return hwRTC_Unsupport;
}

#endif // STM32F1