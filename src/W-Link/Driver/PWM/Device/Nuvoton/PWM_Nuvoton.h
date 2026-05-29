
#ifndef PWM_STM32_H
#define PWM_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "PWM/PWM.h"

#include "PWM/Pin/Nuvoton/PWM_Pin_Nuvoton_Def.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern bool PWM_Channel_Init_Status[hwPWM_Channel_MAX];

PWM_T *PWM_Map_Soc_Base(hwPWM_Base_Index pwm_base);

void PWM_Clock_Enable(hwPWM_Base_Index pwm_base, hwPWM_Channel channel_index);
void PWM_Clock_Disable(hwPWM_Base_Index pwm_base, hwPWM_Channel channel_index);

void PWM_GPIO_ConfigAF(const PWM_Pin_Def *def);
void PWM_GPIO_DeConfigAF(const PWM_Pin_Def *def);

uint32_t PWM_Channel_To_Mask(hwPWM_Channel channel_index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_STM32_H
