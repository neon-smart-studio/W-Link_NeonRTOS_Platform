
#ifndef DAC_PIN_H
#define DAC_PIN_H

#include "soc.h"

#include "Driver_Config.h"

#ifdef DEVICE_STM32
#include "STM32/DAC_Pin_STM32.h"
#endif

#ifdef DEVICE_TIMSPM0
#include "TIMSPM0/DAC_Pin_TIMSPM0.h"
#endif

#endif //DAC_PIN_H