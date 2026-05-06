
#ifndef RTC_INDEX_H
#define RTC_INDEX_H

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

#endif //RTC_INDEX_H