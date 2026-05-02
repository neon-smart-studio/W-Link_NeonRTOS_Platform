#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "ADC/ADC.h"

#ifdef STM32L0

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
#if defined (STM32L053xx) || defined (STM32L063xx) || defined (STM32L073xx) || defined (STM32L083xx)
        case hwADC_Channel_Index_16: return ADC_CHANNEL_16;
#endif
        case hwADC_Channel_Index_17: return ADC_CHANNEL_17;
        case hwADC_Channel_Index_18: return ADC_CHANNEL_18;
        default: return 0;
    }
}

#if defined (STM32L010x4) || defined (STM32L010x6) || defined (STM32L010x8) || defined (STM32L010xB)
void ADC1_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_1]);
}
#else
void ADC1_COMP_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_1]);
}
#endif

hwADC_OpStatus ADC_Instance_Init(hwADC_Instance inst)
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

    g_adc[inst].Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
    g_adc[inst].Init.Resolution            = ADC_RESOLUTION_12B;
    g_adc[inst].Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    g_adc[inst].Init.ScanConvMode          = ADC_SCAN_DIRECTION_FORWARD;
    g_adc[inst].Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    g_adc[inst].Init.LowPowerAutoWait      = DISABLE;
    g_adc[inst].Init.LowPowerAutoPowerOff  = DISABLE;
    g_adc[inst].Init.ContinuousConvMode    = DISABLE;
    g_adc[inst].Init.DiscontinuousConvMode = DISABLE;
    g_adc[inst].Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    g_adc[inst].Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    g_adc[inst].Init.DMAContinuousRequests = DISABLE;
    g_adc[inst].Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;
    g_adc[inst].Init.SamplingTime          = ADC_SAMPLETIME_160CYCLES_5;

    if (HAL_ADC_Init(&g_adc[inst]) != HAL_OK)
        return hwADC_HwError;

    if (HAL_ADCEx_Calibration_Start(&g_adc[inst], ADC_SINGLE_ENDED) != HAL_OK)
        return hwADC_HwError;

    return hwADC_OK;
}

hwADC_OpStatus ADC_Instance_DeInit(hwADC_Instance inst)
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

hwADC_OpStatus ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if (inst >= hwADC_Instance_MAX || ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel = ADC_Channel_To_HAL(ch);

    if (cfg.Channel == 0 && ch != hwADC_Channel_Index_0)
        return hwADC_InvalidParameter;

    cfg.Rank = ADC_RANK_CHANNEL_NUMBER;

    if (HAL_ADC_ConfigChannel(&g_adc[inst], &cfg) != HAL_OK)
        return hwADC_HwError;

    return hwADC_OK;
}

void ADC_NVIC_Init(void)
{
#if defined (STM32L010x4) || defined (STM32L010x6) || defined (STM32L010x8) || defined (STM32L010xB)
    HAL_NVIC_SetPriority(ADC1_IRQn, ADC_IRQ_NVIC_PRIORITY, ADC_IRQ_NVIC_SUB_PRIORITY);
    HAL_NVIC_EnableIRQ(ADC1_IRQn);
#else
    HAL_NVIC_SetPriority(ADC1_COMP_IRQn, ADC_IRQ_NVIC_PRIORITY, ADC_IRQ_NVIC_SUB_PRIORITY);
    HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);
#endif
}

void ADC_NVIC_DeInit(void)
{
#if defined (STM32L010x4) || defined (STM32L010x6) || defined (STM32L010x8) || defined (STM32L010xB)
    HAL_NVIC_DisableIRQ(ADC1_IRQn);
#else
    HAL_NVIC_DisableIRQ(ADC1_COMP_IRQn);
#endif
}

#endif // STM32L0