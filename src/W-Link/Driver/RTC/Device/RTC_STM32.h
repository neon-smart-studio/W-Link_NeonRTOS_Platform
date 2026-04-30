#ifndef RTC_STM32_H
#define RTC_STM32_H

#include <time.h>
#include "soc.h"
#include "RTC/RTC.h"

#define RTC_MUTEX_ACCESS_TIMEOUT     500

#define RTC_IRQ_NVIC_PRIORITY        5
#define RTC_IRQ_NVIC_SUB_PRIORITY    0

#ifdef	__cplusplus
extern "C" {
#endif

extern RTC_HandleTypeDef g_rtc[hwRTC_Index_MAX];

void RTC_Enable_Clock_Source(void);

hwRTC_OpResult RTC_Instance_Init(hwRTC_Index index);
hwRTC_OpResult RTC_Instance_DeInit(hwRTC_Index index);

void RTC_NVIC_Init(void);
void RTC_NVIC_DeInit(void);

hwRTC_Index RTC_IndexFromHandle(RTC_HandleTypeDef *hrtc);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif