
#ifndef PWM_STM32_H
#define PWM_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "PWM/PWM.h"

#include "PWM/Pin/RP2/PWM_Pin_RP2_Def.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern bool PWM_Channel_Init_Status[hwPWM_Channel_MAX];

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_STM32_H
