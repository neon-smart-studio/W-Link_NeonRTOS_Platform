
#ifndef RTC_ALARM_CHANNEL_H
#define RTC_ALARM_CHANNEL_H

#include "soc.h"

#ifdef DEVICE_NUVOTON
typedef enum {
    hwRTC_Alarm_Channel_Index_0 = 0,
    hwRTC_Alarm_Channel_Index_MAX
} hwRTC_Alarm_Channel_Index;
#endif // DEVICE_NUVOTON

#ifdef DEVICE_STM32
typedef enum {
#if defined(RTC_ALARM_A)
    hwRTC_Alarm_Channel_Index_0 = 0, // Alarm A
#endif
#if defined(RTC_ALARM_B)
    hwRTC_Alarm_Channel_Index_1,     // Alarm B
#endif
    hwRTC_Alarm_Channel_Index_MAX
} hwRTC_Alarm_Channel_Index;
#endif // DEVICE_RP2

#ifdef DEVICE_RP2
typedef enum {
#ifdef RP2040
    hwRTC_Alarm_Channel_Index_0 = 0,
#endif
    hwRTC_Alarm_Channel_Index_MAX
} hwRTC_Alarm_Channel_Index;
#endif // DEVICE_RP2

#ifdef DEVICE_TITIVAC
typedef enum {
    hwRTC_Alarm_Channel_Index_0 = 0,
    hwRTC_Alarm_Channel_Index_MAX
} hwRTC_Alarm_Channel_Index;
#endif // DEVICE_TITIVAC

#ifdef DEVICE_TIMSP432P
typedef enum {
    hwRTC_Alarm_Channel_Index_0 = 0,
    hwRTC_Alarm_Channel_Index_MAX
} hwRTC_Alarm_Channel_Index;
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
typedef enum {
    hwRTC_Alarm_Channel_Index_0 = 0,
    hwRTC_Alarm_Channel_Index_MAX
} hwRTC_Alarm_Channel_Index;
#endif // DEVICE_TIMSP432E

#ifdef DEVICE_TIMSPM0
typedef enum {
#if defined(RTC_BASE) || defined(RTC_A_BASE) || defined(RTC_B_BASE)
    hwRTC_Alarm_Channel_Index_0 = 0,
    hwRTC_Alarm_Channel_Index_1,
#endif
    hwRTC_Alarm_Channel_Index_MAX
} hwRTC_Alarm_Channel_Index;
#endif // DEVICE_TIMSPM0

#endif //RTC_ALARM_CHANNEL_H