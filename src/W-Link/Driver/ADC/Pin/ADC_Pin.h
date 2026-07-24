
#ifndef ADC_PIN_H
#define ADC_PIN_H

#include "soc.h"

#include "Driver_Config.h"

#ifdef DEVICE_NUVOTON
#include "Nuvoton/ADC_Pin_NUC4x2.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/ADC_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/ADC_Pin_RP2.h"
#endif

#ifdef DEVICE_TITIVAC
#include "TITivaC/ADC_Pin_TITivaC.h"
#endif

#ifdef DEVICE_TIMSP432P
#include "TIMSP432P/ADC_Pin_TIMSP432P.h"
#endif

#ifdef DEVICE_TIMSP432E
#include "TIMSP432E/ADC_Pin_TIMSP432E.h"
#endif

#ifdef DEVICE_TIMSPM0
#include "TIMSPM0/ADC_Pin_TIMSPM0.h"
#endif

#endif //ADC_PIN_H