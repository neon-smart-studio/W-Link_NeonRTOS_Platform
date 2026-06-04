
#ifndef DAC_PIN_H
#define DAC_PIN_H

#include "soc.h"

#include "Driver_Config.h"

#ifdef DEVICE_NUVOTON
//#include "Nuvoton/DAC_Pin_Nuvoton.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/DAC_Pin_STM32.h"
#endif

#ifdef DEVICE_TM4C1294
#include "TI/DAC_Pin_TM4C1294.h"
#endif

#endif //DAC_PIN_H