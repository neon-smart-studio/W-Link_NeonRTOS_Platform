
#ifndef DAC_STM32_INSTANCE_H
#define DAC_STM32_INSTANCE_H

#include "GPIO/GPIO.h"

#include "DAC/DAC.h"

typedef enum {
#if defined(DAC1_BASE) || defined(DAC_BASE)
    hwDAC_Instance_1 = 0,
#endif
#if defined(DAC2_BASE)
    hwDAC_Instance_2,
#endif
#if defined(DAC3_BASE)
    hwDAC_Instance_3,
#endif
#if defined(DAC4_BASE)
    hwDAC_Instance_4,
#endif
    hwDAC_Instance_MAX,
} hwDAC_Instance;

#endif //DAC_STM32_INSTANCE_H