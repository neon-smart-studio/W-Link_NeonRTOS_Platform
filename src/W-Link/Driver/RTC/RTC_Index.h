
#ifndef RTC_INDEX_H
#define RTC_INDEX_H

#ifdef DEVICE_STM32
typedef enum hwRTC_Index_t
{
  hwRTC_Index_0,
  hwRTC_Index_MAX,
}hwRTC_Index;
#endif

#ifdef DEVICE_RPI
typedef enum hwRTC_Index_t
{
  hwRTC_Index_MAX,
}hwRTC_Index;
#endif

#endif //RTC_INDEX_H