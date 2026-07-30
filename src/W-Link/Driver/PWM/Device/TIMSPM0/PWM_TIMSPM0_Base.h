
#ifndef PWM_TIMSP432_BASE_H
#define PWM_TIMSP432_BASE_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef	__cplusplus
extern "C" {
#endif


typedef enum {
    hwPWM_Base_Invalid = -1,

#if defined(TIMA0_BASE)
    hwPWM_Base_TIMA0,
#endif
#if defined(TIMA1_BASE)
    hwPWM_Base_TIMA1,
#endif
#if defined(TIMG0_BASE)
    hwPWM_Base_TIMG0,
#endif
#if defined(TIMG1_BASE)
    hwPWM_Base_TIMG1,
#endif
#if defined(TIMG2_BASE)
    hwPWM_Base_TIMG2,
#endif
#if defined(TIMG4_BASE)
    hwPWM_Base_TIMG4,
#endif
#if defined(TIMG5_BASE)
    hwPWM_Base_TIMG5,
#endif
#if defined(TIMG6_BASE)
    hwPWM_Base_TIMG6,
#endif
#if defined(TIMG7_BASE)
    hwPWM_Base_TIMG7,
#endif
#if defined(TIMG8_BASE)
#if !defined(MSPM0C1103) && !defined(MSPM0C1104) && !defined(MSPM0C1105) && !defined(MSPM0C1106)
    hwPWM_Base_TIMG8,
#endif
#endif
#if defined(TIMG9_BASE)
    hwPWM_Base_TIMG9,
#endif
#if defined(TIMG12_BASE)
    hwPWM_Base_TIMG12,
#endif
#if defined(TIMG14_BASE)
    hwPWM_Base_TIMG14,
#endif

    hwPWM_Base_MAX
} hwPWM_Base_Index;

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //PWM_TIMSP432_BASE_H
