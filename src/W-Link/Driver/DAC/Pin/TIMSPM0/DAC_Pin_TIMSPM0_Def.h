
#ifndef DAC_PIN_TIMSPM0_DEF_H
#define DAC_PIN_TIMSPM0_DEF_H

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"

#include "DAC/Device/TIMSPM0/DAC_TIMSPM0_Instance.h"

typedef struct {
    hwGPIO_Pin dac_pin;
    hwDAC_Instance inst;
} DAC_Channel_Def;

#endif //DAC_PIN_TIMSPM0_DEF_H