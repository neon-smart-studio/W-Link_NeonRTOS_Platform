
#ifndef PWM_PIN_H
#define PWM_PIN_H

#include "soc.h"

#include "Driver_Config.h"

#ifdef DEVICE_NUVOTON
#include "Nuvoton/PWM_Pin_Nuvoton.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/PWM_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/PWM_Pin_RP2.h"
#endif

#ifdef DEVICE_TITIVAC
#include "TITivaC/PWM_Pin_TITivaC.h"
#endif

#ifdef DEVICE_TIMSP432P
#include "TIMSP432/PWM_Pin_TIMSP432P.h"
#endif

#ifdef DEVICE_TIMSP432E
#include "TIMSP432/PWM_Pin_TIMSP432E.h"
#endif

#ifdef DEVICE_TIMSPM0
#include "TIMSPM0/PWM_Pin_TIMSPM0.h"
#endif

#endif //PWM_PIN_H