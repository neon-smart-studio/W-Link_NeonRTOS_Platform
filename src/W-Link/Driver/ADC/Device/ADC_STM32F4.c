#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#ifdef STM32F4

#include "ADC/ADC.h"

#include "ADC_STM32.h"

#include "GPIO/Device/GPIO_STM32.h"

ADC_HandleTypeDef g_adc[hwADC_Instance_MAX];

static uint32_t ADC_Channel_To_HAL(hwADC_Channel_Index ch)
{
    switch (ch)
    {
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
        default: return 0;
    }
}

void ADC_IRQHandler(void)
{
#if defined(ADC1_BASE)
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_1]);
#endif
#if defined(ADC2_BASE)
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_2]);
#endif
#if defined(ADC3_BASE)
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_3]);
#endif
}

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

#if defined(ADC2_BASE)
        case hwADC_Instance_2:
            __HAL_RCC_ADC2_CLK_ENABLE();
            g_adc[inst].Instance = ADC2;
            break;
#endif

#if defined(ADC3_BASE)
        case hwADC_Instance_3:
            __HAL_RCC_ADC3_CLK_ENABLE();
            g_adc[inst].Instance = ADC3;
            break;
#endif
    }

    g_adc[inst].Init.Resolution            = ADC_RESOLUTION_12B;
    g_adc[inst].Init.ScanConvMode          = DISABLE;
    g_adc[inst].Init.ContinuousConvMode    = DISABLE;
    g_adc[inst].Init.DiscontinuousConvMode = DISABLE;
    g_adc[inst].Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    g_adc[inst].Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    g_adc[inst].Init.NbrOfConversion       = 1;

    if (HAL_ADC_Init(&g_adc[inst]) != HAL_OK)
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
#if defined(ADC2_BASE)
        case hwADC_Instance_2:
            __HAL_RCC_ADC2_CLK_DISABLE();
            break;
#endif
#if defined(ADC3_BASE)
        case hwADC_Instance_3:
            __HAL_RCC_ADC3_CLK_DISABLE();
            break;
#endif
    }

    return hwADC_OK;
}

hwADC_OpStatus ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    if (ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    ADC_HandleTypeDef *hadc = &g_adc[inst];

    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel = ADC_Channel_To_HAL(ch);

    if (cfg.Channel == 0 && ch != hwADC_Channel_Index_0)
        return hwADC_InvalidParameter;

#if defined(ADC_REGULAR_RANK_1)
    cfg.Rank = ADC_REGULAR_RANK_1;
#endif

    cfg.SamplingTime = ADC_SAMPLETIME_480CYCLES;

    if (HAL_ADC_ConfigChannel(hadc, &cfg) != HAL_OK)
        return hwADC_HwError;

    return hwADC_OK;
}

void ADC_NVIC_Init(void)
{
#if defined(ADC_IRQn)
    HAL_NVIC_SetPriority(ADC_IRQn,
                         ADC_IRQ_NVIC_PRIORITY,
                         ADC_IRQ_NVIC_SUB_PRIORITY);

    HAL_NVIC_EnableIRQ(ADC_IRQn);
#endif
}

void ADC_NVIC_DeInit(void)
{
#if defined(ADC_IRQn)
    HAL_NVIC_DisableIRQ(ADC_IRQn);
#endif
}

#endif // STM32F4