#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "ADC/ADC.h"

#ifdef STM32L1

#include "ADC_STM32.h"

ADC_HandleTypeDef g_adc[hwADC_Instance_MAX];

static uint32_t ADC_Channel_To_HAL(hwADC_Channel_Index ch)
{
    switch (ch) {
        case hwADC_Channel_Index_0:  return ADC_CHANNEL_0;
        case hwADC_Channel_Index_1:  return ADC_CHANNEL_1;
        case hwADC_Channel_Index_2:  return ADC_CHANNEL_2;
        case hwADC_Channel_Index_3:  return ADC_CHANNEL_3;
        case hwADC_Channel_Index_4:  return ADC_CHANNEL_4;
        case hwADC_Channel_Index_5:  return ADC_CHANNEL_5;
        case hwADC_Channel_Index_6:  return ADC_CHANNEL_6;
        case hwADC_Channel_Index_7:  return ADC_CHANNEL_7;
        case hwADC_Channel_Index_8:  return ADC_CHANNEL_8;
        case hwADC_Channel_Index_9:  return ADC_CHANNEL_9;
        case hwADC_Channel_Index_10: return ADC_CHANNEL_10;
        case hwADC_Channel_Index_11: return ADC_CHANNEL_11;
        case hwADC_Channel_Index_12: return ADC_CHANNEL_12;
        case hwADC_Channel_Index_13: return ADC_CHANNEL_13;
        case hwADC_Channel_Index_14: return ADC_CHANNEL_14;
        case hwADC_Channel_Index_15: return ADC_CHANNEL_15;
        case hwADC_Channel_Index_16: return ADC_CHANNEL_16;
        case hwADC_Channel_Index_17: return ADC_CHANNEL_17;
        case hwADC_Channel_Index_18: return ADC_CHANNEL_18;
        case hwADC_Channel_Index_19: return ADC_CHANNEL_19;
        case hwADC_Channel_Index_20: return ADC_CHANNEL_20;
        case hwADC_Channel_Index_21: return ADC_CHANNEL_21;
        case hwADC_Channel_Index_22: return ADC_CHANNEL_22;
        case hwADC_Channel_Index_23: return ADC_CHANNEL_23;
        case hwADC_Channel_Index_24: return ADC_CHANNEL_24;
        case hwADC_Channel_Index_25: return ADC_CHANNEL_25;
        case hwADC_Channel_Index_26: return ADC_CHANNEL_26;
#if defined(STM32L151xCA) || defined (STM32L151xD) || defined (STM32L152xCA) || defined (STM32L152xD) || defined (STM32L162xCA) || defined (STM32L162xD) || defined(STM32L151xE) || defined(STM32L151xDX) || defined (STM32L152xE) || defined (STM32L152xDX) || defined (STM32L162xE) || defined (STM32L162xDX)
        case hwADC_Channel_Index_27: return ADC_CHANNEL_27;
        case hwADC_Channel_Index_28: return ADC_CHANNEL_28;
        case hwADC_Channel_Index_29: return ADC_CHANNEL_29;
        case hwADC_Channel_Index_30: return ADC_CHANNEL_30;
        case hwADC_Channel_Index_31: return ADC_CHANNEL_31;
#endif
        default: return 0;
    }
}

void ADC1_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_1]);
}

hwADC_OpResult ADC_Instance_Init(hwADC_Instance inst)
{
    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    switch (inst)
    {
#if defined(ADC1_BASE)
        case hwADC_Instance_1:
            __HAL_RCC_ADC1_CLK_ENABLE();
            g_adc[inst].Instance = ADC1;
            break;
#endif
    }

    g_adc[inst].Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;
    g_adc[inst].Init.Resolution            = ADC_RESOLUTION_12B;
    g_adc[inst].Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    g_adc[inst].Init.ScanConvMode          = ADC_SCAN_DISABLE;
    g_adc[inst].Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    g_adc[inst].Init.LowPowerAutoWait      = DISABLE;
    g_adc[inst].Init.LowPowerAutoPowerOff  = DISABLE;
    g_adc[inst].Init.ChannelsBank          = ADC_CHANNELS_BANK_A;
    g_adc[inst].Init.ContinuousConvMode    = DISABLE;
    g_adc[inst].Init.NbrOfConversion       = 1;
    g_adc[inst].Init.DiscontinuousConvMode = DISABLE;
    g_adc[inst].Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    g_adc[inst].Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    g_adc[inst].Init.DMAContinuousRequests = DISABLE;

    if (HAL_ADC_Init(&g_adc[inst]) != HAL_OK)
        return hwADC_HwError;

    return hwADC_OK;
}

hwADC_OpResult ADC_Instance_DeInit(hwADC_Instance inst)
{
    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    HAL_ADC_DeInit(&g_adc[inst]);

    switch (inst)
    {
#if defined(ADC1_BASE)
        case hwADC_Instance_1:
            __HAL_RCC_ADC1_CLK_DISABLE();
            break;
#endif
    }

    return hwADC_OK;
}

hwADC_OpResult ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if (inst >= hwADC_Instance_MAX || ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel = ADC_Channel_To_HAL(ch);

    if (cfg.Channel == 0 && ch != hwADC_Channel_Index_0)
        return hwADC_InvalidParameter;

    cfg.Rank         = ADC_REGULAR_RANK_1;
    cfg.SamplingTime = ADC_SAMPLETIME_384CYCLES;

    if (HAL_ADC_ConfigChannel(&g_adc[inst], &cfg) != HAL_OK)
        return hwADC_HwError;

    return hwADC_OK;
}

void ADC_NVIC_Init(void)
{
    HAL_NVIC_SetPriority(ADC1_IRQn, ADC_IRQ_NVIC_PRIORITY, ADC_IRQ_NVIC_SUB_PRIORITY);
    HAL_NVIC_EnableIRQ(ADC1_IRQn);
}

void ADC_NVIC_DeInit(void)
{
    HAL_NVIC_DisableIRQ(ADC1_IRQn);
}

#endif // STM32L1