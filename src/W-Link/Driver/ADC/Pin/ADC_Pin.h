
#ifndef ADC_PIN_H
#define ADC_PIN_H

#include "soc.h"

#ifdef DEVICE_NUVOTON
#include "Nuvoton/ADC_Pin_NUC4x2.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/ADC_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/ADC_Pin_RP2.h"
#endif

#ifdef DEVICE_TM4C1294
#include "TI/ADC_Pin_TM4C1294.h"
#endif

#endif //ADC_PIN_H