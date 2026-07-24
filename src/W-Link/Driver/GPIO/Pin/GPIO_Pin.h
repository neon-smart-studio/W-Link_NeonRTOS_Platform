
#ifndef GPIO_PIN_H
#define GPIO_PIN_H

#include "soc.h"

#include "Driver_Config.h"

#ifdef DEVICE_NUVOTON
#include "Nuvoton/GPIO_Pin_Nuvoton.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/GPIO_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/GPIO_Pin_RP2.h"
#endif

#ifdef DEVICE_TITIVAC
#include "TITivaC/GPIO_Pin_TITivaC.h"
#endif

#ifdef DEVICE_TIMSP432P
#include "TIMSP432/GPIO_Pin_TIMSP432P.h"
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
#include "TIMSP432/GPIO_Pin_TIMSP432E.h"
#endif // DEVICE_TIMSP432E

#ifdef DEVICE_TIMSPM0
#include "TIMSPM0/GPIO_Pin_TIMSPM0.h"
#endif // DEVICE_TIMSPM0

#endif //GPIO_PIN_H