#ifndef DAC_STM32_H
#define DAC_STM32_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "DAC/DAC.h"

#if defined(DAC_BASE) || defined(DAC1_BASE) || defined(DAC2_BASE) || defined(DAC3_BASE) || defined(DAC4_BASE)

#include "DAC_STM32_Instance.h"

#define DAC_VREF_MV   3300.0f
#define DAC_MAX_COUNT 4095.0f

extern DAC_HandleTypeDef g_dac[hwDAC_Instance_MAX];

hwDAC_OpResult DAC_Instance_Init(hwDAC_Instance inst);
hwDAC_OpResult DAC_Instance_DeInit(hwDAC_Instance inst);

hwDAC_OpResult DAC_ConfigChannel(hwDAC_Instance inst, hwDAC_Channel_Index ch);
hwDAC_OpResult DAC_StartChannel(hwDAC_Instance inst, hwDAC_Channel_Index ch);
hwDAC_OpResult DAC_StopChannel(hwDAC_Instance inst, hwDAC_Channel_Index ch);
hwDAC_OpResult DAC_WriteRaw(hwDAC_Instance inst, hwDAC_Channel_Index ch, uint32_t raw);

#endif // DAC_BASE || DAC1_BASE || DAC2_BASE || DAC3_BASE || DAC4_BASE

#endif