
#ifndef PWM_RP2_BASE_H
#define PWM_RP2_BASE_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
    hwPWM_Base_0 = 0,
    hwPWM_Base_1,
    hwPWM_Base_2,
    hwPWM_Base_3,
    hwPWM_Base_4,
    hwPWM_Base_5,
    hwPWM_Base_6,
    hwPWM_Base_7,
	hwPWM_Base_MAX
}hwPWM_Base_Index;

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_RP2_BASE_H
