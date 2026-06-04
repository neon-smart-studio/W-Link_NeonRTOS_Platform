
#ifndef PWM_H
#define PWM_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"  

#include "PWM_Def.h"

#include "PWM_Index.h"

#include "Driver_Config.h"

#ifdef	__cplusplus
extern "C" {
#endif

bool PWM_is_Init(hwPWM_Channel channel_index);
hwPWM_OpResult PWM_Channel_Init(hwPWM_Channel channel_index, bool inverse_PWM);
hwPWM_OpResult PWM_Channel_DeInit(hwPWM_Channel channel_index);
hwPWM_OpResult PWM_Turn_On(hwPWM_Channel channel_index);
hwPWM_OpResult PWM_Turn_On_And_Set_Duty(hwPWM_Channel channel_index, uint16_t duty);
hwPWM_OpResult PWM_Turn_Off(hwPWM_Channel channel_index);
hwPWM_OpResult PWM_Set_Duty(hwPWM_Channel channel_index, uint16_t duty);
hwPWM_OpResult PWM_Step_Duty(hwPWM_Channel channel_index, uint16_t step_duty, hwPWM_Step_Direction direction);
hwPWM_OpResult PWM_Get_Channel_OnOff_Status(hwPWM_Channel channel_index, bool* onoff_status);
hwPWM_OpResult PWM_Get_Channel_Current_Duty(hwPWM_Channel channel_index, uint16_t* current_duty);
hwPWM_OpResult PWM_Get_Channel_Current_OnOff_Duty(hwPWM_Channel channel_index, bool* onoff_status, uint16_t* current_duty);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_H