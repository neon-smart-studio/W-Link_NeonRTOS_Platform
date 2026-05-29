
#ifndef PWM_NUVOTON_BASE_H
#define PWM_NUVOTON_BASE_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
#if defined (PWM0_BASE)
    hwPWM_Base_1 = 0,
#endif
#if defined (PWM1_BASE)
    hwPWM_Base_2,
#endif
	hwPWM_Base_MAX
}hwPWM_Base_Index;

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_NUVOTON_BASE_H
