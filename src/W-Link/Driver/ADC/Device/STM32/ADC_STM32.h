#ifndef ADC_STM32_H
#define ADC_STM32_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "ADC/ADC.h"

#include "ADC_STM32_Instance.h"

#define ADC_IRQ_NVIC_PRIORITY      5
#define ADC_IRQ_NVIC_SUB_PRIORITY  0

#define ADC_VREF_MV                3300.0f
#define ADC_MAX_COUNT              4095.0f
#define ADC_CONV_TIMEOUT_MS        100

typedef struct {
    uint32_t raw;
} ADC_QueueItem;

extern bool ADC_NVIC_Init_Status;
extern bool ADC_Instance_Init_Status[hwADC_Instance_MAX];
extern bool ADC_Channel_Init_Status[hwADC_Channel_Index_MAX];
extern NeonRTOS_MsgQ_t ADC_Channel_SyncQueue[hwADC_Instance_MAX];
extern ADC_HandleTypeDef g_adc[hwADC_Instance_MAX];

hwADC_OpStatus ADC_Instance_Init(hwADC_Instance inst);
hwADC_OpStatus ADC_Instance_DeInit(hwADC_Instance inst);
hwADC_OpStatus ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch);
void ADC_NVIC_Init(void);
void ADC_NVIC_DeInit(void);

#endif // ADC_STM32_H