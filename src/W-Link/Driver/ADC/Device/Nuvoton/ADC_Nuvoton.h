#ifndef ADC_NUC4x2_H
#define ADC_NUC4x2_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "ADC/ADC.h"

#include "ADC_NUC4x2_Instance.h"

#define ADC_VREF_MV                3300.0f
#define ADC_MAX_COUNT              4095.0f
#define ADC_CONV_TIMEOUT_MS        100

typedef struct {
    uint32_t raw;
} ADC_QueueItem;

void ADC_GPIO_ConfigAF(hwADC_Channel_Index ch);
void ADC_GPIO_DeConfigAF(hwADC_Channel_Index ch);

void ADC_ConvCpltCallback(uint16_t raw);

hwADC_OpResult ADC_Instance_Init(hwADC_Instance inst);
hwADC_OpResult ADC_Instance_DeInit(hwADC_Instance inst);
hwADC_OpResult ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch);
hwADC_OpResult ADC_ChannelStartConversion(hwADC_Instance inst, hwADC_Channel_Index ch);
hwADC_OpResult ADC_ChannelStopConversion(hwADC_Instance inst, hwADC_Channel_Index ch);

void ADC_NVIC_Init();
void ADC_NVIC_DeInit();

extern bool ADC_Instance_Init_Status[hwADC_Instance_MAX];
extern bool ADC_Channel_Init_Status[hwADC_Channel_Index_MAX];

#endif // ADC_NUC4x2_H