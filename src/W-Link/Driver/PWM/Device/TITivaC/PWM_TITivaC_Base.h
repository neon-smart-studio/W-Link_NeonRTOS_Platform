
#ifndef PWM_TITIVAC_BASE_H
#define PWM_TITIVAC_BASE_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
#if defined(SYSCTL_PERIPH_PWM0)
    hwPWM_Base_0 = 0,
#endif
#if defined(SYSCTL_PERIPH_PWM1)
    hwPWM_Base_1,
#endif
	hwPWM_Base_MAX
}hwPWM_Base_Index;

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_TITIVAC_BASE_H
