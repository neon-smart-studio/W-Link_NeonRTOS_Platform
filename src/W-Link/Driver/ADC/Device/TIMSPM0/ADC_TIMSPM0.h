#ifndef ADC_TIMSPM0
#define ADC_TIMSPM0

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "ADC/ADC.h"

#include "ADC_TIMSPM0_Instance.h"

#define ADC_VREF_MV                3300.0f
#define ADC_MAX_COUNT              4095.0f
#define ADC_CONV_TIMEOUT_MS        100

#define ADC_SAMPLE_TIME               (64U)
#define ADC_POWER_STARTUP_DELAY       (16U)

#endif // ADC_TIMSPM0