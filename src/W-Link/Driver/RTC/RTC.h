
#ifndef RTC_H
#define RTC_H

#include <time.h>

#include "soc.h"  

#include "RTC_Def.h"

#include "RTC_Index.h"

#include "RTC_Alarm_Channel.h"

hwRTC_OpResult RTC_Timer_Init(hwRTC_Index hw_index);
hwRTC_OpResult RTC_Timer_DeInit(hwRTC_Index hw_index);
hwRTC_OpResult RTC_Timer_Read(hwRTC_Index hw_index, time_t* unix_time);
hwRTC_OpResult RTC_Timer_Write(hwRTC_Index hw_index, time_t unix_time);

typedef void(* onAlarmEventCallback)(hwRTC_Index index, hwRTC_Alarm_Channel_Index alarm_ch_index);
hwRTC_OpResult RTC_Timer_Set_Alarm(hwRTC_Index hw_index, hwRTC_Alarm_Channel_Index alarm_ch_index, time_t alarm_unix_time, onAlarmEventCallback alarm_handler);
hwRTC_OpResult RTC_Timer_Clear_Alarm(hwRTC_Index hw_index, hwRTC_Alarm_Channel_Index alarm_ch_index);

#endif //RTC_H