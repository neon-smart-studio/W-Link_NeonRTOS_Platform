
#ifndef PWM_TITIVAC_BASE_H
#define PWM_TITIVAC_BASE_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef	__cplusplus
extern "C" {
#endif

#if defined(TM4C123)
typedef enum
{
    hwPWM_Base_0 = 0,
    hwPWM_Base_1,
	hwPWM_Base_MAX
}hwPWM_Base_Index;
#endif

#if defined(TM4C1294)
typedef enum
{
    hwPWM_Base_0 = 0,
	hwPWM_Base_MAX
}hwPWM_Base_Index;
#endif

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_TITIVAC_BASE_H
