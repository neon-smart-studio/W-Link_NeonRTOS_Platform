
#ifndef RTC_INDEX_H
#define RTC_INDEX_H

#include "soc.h"

#ifdef DEVICE_NUVOTON
typedef enum hwRTC_Index_t
{
  hwRTC_Index_0 = 0,
  hwRTC_Index_MAX,
}hwRTC_Index;
#endif // DEVICE_NUVOTON

#ifdef DEVICE_STM32
typedef enum hwRTC_Index_t
{
  hwRTC_Index_0 = 0,
  hwRTC_Index_MAX,
}hwRTC_Index;
#endif // DEVICE_STM32

#ifdef DEVICE_RP2
typedef enum hwRTC_Index_t
{
  hwRTC_Index_0 = 0,
  hwRTC_Index_MAX,
}hwRTC_Index;
#endif // DEVICE_RP2

#ifdef DEVICE_TITIVAC
typedef enum hwRTC_Index_t
{
  hwRTC_Index_0 = 0,
  hwRTC_Index_MAX,
}hwRTC_Index;
#endif // DEVICE_TITIVAC

#ifdef DEVICE_TIMSP432P
typedef enum {
    hwRTC_Index_0 = 0,
    hwRTC_Index_MAX
} hwRTC_Index;
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
typedef enum {
    hwRTC_Index_0 = 0,
    hwRTC_Index_MAX
} hwRTC_Index;
#endif // DEVICE_TIMSP432E

#endif //RTC_INDEX_H