#ifndef ADC_TIMSP432_H
#define ADC_TIMSP432_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "ADC/ADC.h"

#include "ADC_TIMSP432_Instance.h"

#define ADC_VREF_MV                3300.0f
#define ADC_MAX_COUNT              4095.0f
#define ADC_CONV_TIMEOUT_MS        100

typedef struct {
    uint32_t raw;
} ADC_QueueItem;

extern bool ADC_Instance_Init_Status[hwADC_Instance_MAX];
extern bool ADC_Channel_Init_Status[hwADC_Channel_Index_MAX];

#endif // ADC_TIMSP432_H