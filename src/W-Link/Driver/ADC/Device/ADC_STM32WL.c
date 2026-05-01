#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#ifdef STM32WL

#include "ADC/ADC.h"
#include "ADC_STM32.h"
#include "GPIO/Device/GPIO_STM32.h"

ADC_HandleTypeDef g_adc[hwADC_Instance_MAX];

static uint32_t ADC_Channel_To_HAL(hwADC_Channel_Index ch)
{
    switch (ch) {
        case hwADC_Channel_Index_0: return ADC_CHANNEL_0;
        case hwADC_Channel_Index_1: return ADC_CHANNEL_1;
        case hwADC_Channel_Index_2: return ADC_CHANNEL_2;
        case hwADC_Channel_Index_3: return ADC_CHANNEL_3;
        case hwADC_Channel_Index_4: return ADC_CHANNEL_4;
        case hwADC_Channel_Index_5: return ADC_CHANNEL_5;
        case hwADC_Channel_Index_6: return ADC_CHANNEL_6;
        case hwADC_Channel_Index_7: return ADC_CHANNEL_7;
        case hwADC_Channel_Index_8: return ADC_CHANNEL_8;
        case hwADC_Channel_Index_9: return ADC_CHANNEL_9;
        case hwADC_Channel_Index_10: return ADC_CHANNEL_10;
        case hwADC_Channel_Index_11: return ADC_CHANNEL_11;
        case hwADC_Channel_Index_12: return ADC_CHANNEL_12;
        case hwADC_Channel_Index_13: return ADC_CHANNEL_13;
        case hwADC_Channel_Index_14: return ADC_CHANNEL_14;
        case hwADC_Channel_Index_15: return ADC_CHANNEL_15;
        case hwADC_Channel_Index_16: return ADC_CHANNEL_16;
        case hwADC_Channel_Index_17: return ADC_CHANNEL_17;
        default: return 0;
    }
}

void ADC_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_1]);
}

hwADC_OpStatus ADC_Instance_Init(hwADC_Instance inst)
{
    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    __HAL_RCC_ADC_CLK_ENABLE();

    g_adc[inst].Instance = ADC;

    g_adc[inst].Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
    g_adc[inst].Init.Resolution            = ADC_RESOLUTION_12B;
    g_adc[inst].Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    g_adc[inst].Init.ScanConvMode          = ADC_SCAN_DISABLE;
    g_adc[inst].Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    g_adc[inst].Init.LowPowerAutoWait      = DISABLE;
    g_adc[inst].Init.ContinuousConvMode    = DISABLE;
    g_adc[inst].Init.NbrOfConversion       = 1;
    g_adc[inst].Init.DiscontinuousConvMode = DISABLE;
    g_adc[inst].Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    g_adc[inst].Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    g_adc[inst].Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;

    if (HAL_ADC_Init(&g_adc[inst]) != HAL_OK)
        return hwADC_HwError;

    if (HAL_ADCEx_Calibration_Start(&g_adc[inst]) != HAL_OK)
        return hwADC_HwError;

    return hwADC_OK;
}

hwADC_OpStatus ADC_Instance_DeInit(hwADC_Instance inst)
{
    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    HAL_ADC_DeInit(&g_adc[inst]);

    __HAL_RCC_ADC_CLK_DISABLE();

    return hwADC_OK;
}

hwADC_OpStatus ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if (inst != hwADC_Instance_1)
        return hwADC_InvalidParameter;

    ADC_ChannelConfTypeDef cfg = {0};

    cfg.Channel = ADC_Channel_To_HAL(ch);
    cfg.Rank = ADC_REGULAR_RANK_1;
    cfg.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;

    if (HAL_ADC_ConfigChannel(&g_adc[inst], &cfg) != HAL_OK)
        return hwADC_HwError;

    return hwADC_OK;
}

void ADC_NVIC_Init(void)
{
    HAL_NVIC_SetPriority(ADC_IRQn, ADC_IRQ_NVIC_PRIORITY, ADC_IRQ_NVIC_SUB_PRIORITY);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
}

void ADC_NVIC_DeInit(void)
{
    HAL_NVIC_DisableIRQ(ADC_IRQn);
}

#endif