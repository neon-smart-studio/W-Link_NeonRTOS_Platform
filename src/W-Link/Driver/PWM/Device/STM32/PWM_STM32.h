
#ifndef PWM_STM32_H
#define PWM_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "PWM/PWM.h"

#include "PWM/Pin/STM32/PWM_Pin_STM32_Def.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern bool PWM_Channel_Init_Status[hwPWM_Channel_MAX];

const PWM_Pin_Def* PWM_Find_PinDef(hwPWM_Channel channel_index);

void PWM_Clock_Enable(hwPWM_Channel channel_index);
void PWM_Clock_Disable(hwPWM_Channel channel_index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_STM32_H
