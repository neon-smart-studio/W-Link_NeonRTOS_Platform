
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

#ifdef DEVICE_TM4C1294
#include "TI/PWM_Pin_TM4C1294.h"
#endif

#endif //PWM_PIN_H