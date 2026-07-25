
#ifndef DAC_TIMSPM0_INSTANCE_H
#define DAC_TIMSPM0_INSTANCE_H

#include "GPIO/GPIO.h"

#include "DAC/DAC.h"

typedef enum {
#if defined(DAC0_BASE)
    hwDAC_Instance_1 = 0,
#endif
    hwDAC_Instance_MAX,
} hwDAC_Instance;

#endif //DAC_TIMSPM0_INSTANCE_H