#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#ifdef STM32U0

#include "ADC/ADC.h"
#include "ADC_STM32.h"
#include "GPIO/Device/GPIO_STM32.h"

ADC_HandleTypeDef g_adc[hwADC_Instance_MAX];

static uint32_t ADC_Channel_To_HAL(hwADC_Channel_Index ch)
{
    switch (ch) {
#if defined(ADC_CHANNEL_0)
        case hwADC_Channel_Index_0:  return ADC_CHANNEL_0;
#endif
#if defined(ADC_CHANNEL_1)
        case hwADC_Channel_Index_1:  return ADC_CHANNEL_1;
#endif
#if defined(ADC_CHANNEL_2)
        case hwADC_Channel_Index_2:  return ADC_CHANNEL_2;
#endif
#if defined(ADC_CHANNEL_3)
        case hwADC_Channel_Index_3:  return ADC_CHANNEL_3;
#endif
#if defined(ADC_CHANNEL_4)
        case hwADC_Channel_Index_4:  return ADC_CHANNEL_4;
#endif
#if defined(ADC_CHANNEL_5)
        case hwADC_Channel_Index_5:  return ADC_CHANNEL_5;
#endif
#if defined(ADC_CHANNEL_6)
        case hwADC_Channel_Index_6:  return ADC_CHANNEL_6;
#endif
#if defined(ADC_CHANNEL_7)
        case hwADC_Channel_Index_7:  return ADC_CHANNEL_7;
#endif
#if defined(ADC_CHANNEL_8)
        case hwADC_Channel_Index_8:  return ADC_CHANNEL_8;
#endif
#if defined(ADC_CHANNEL_9)
        case hwADC_Channel_Index_9:  return ADC_CHANNEL_9;
#endif
#if defined(ADC_CHANNEL_10)
        case hwADC_Channel_Index_10: return ADC_CHANNEL_10;
#endif
#if defined(ADC_CHANNEL_11)
        case hwADC_Channel_Index_11: return ADC_CHANNEL_11;
#endif
#if defined(ADC_CHANNEL_12)
        case hwADC_Channel_Index_12: return ADC_CHANNEL_12;
#endif
#if defined(ADC_CHANNEL_13)
        case hwADC_Channel_Index_13: return ADC_CHANNEL_13;
#endif
#if defined(ADC_CHANNEL_14)
        case hwADC_Channel_Index_14: return ADC_CHANNEL_14;
#endif
#if defined(ADC_CHANNEL_15)
        case hwADC_Channel_Index_15: return ADC_CHANNEL_15;
#endif
#if defined(ADC_CHANNEL_16)
        case hwADC_Channel_Index_16: return ADC_CHANNEL_16;
#endif
#if defined(ADC_CHANNEL_17)
        case hwADC_Channel_Index_17: return ADC_CHANNEL_17;
#endif
#if defined(ADC_CHANNEL_18)
        case hwADC_Channel_Index_18: return ADC_CHANNEL_18;
#endif
        default: return 0;
    }
}

#if defined(ADC1_COMP_IRQn)
void ADC1_COMP_IRQHandler(void)
{
#if defined(ADC1_BASE)
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_1]);
#endif
}
#elif defined(ADC1_IRQn)
void ADC1_IRQHandler(void)
{
#if defined(ADC1_BASE)
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_1]);
#endif
}
#elif defined(ADC_IRQn)
void ADC_IRQHandler(void)
{
#if defined(ADC1_BASE)
    HAL_ADC_IRQHandler(&g_adc[hwADC_Instance_1]);
#endif
}
#endif

static void ADC_EnableClock(void)
{
#if defined(__HAL_RCC_ADC_CLK_ENABLE)
    __HAL_RCC_ADC_CLK_ENABLE();
#elif defined(__HAL_RCC_ADC1_CLK_ENABLE)
    __HAL_RCC_ADC1_CLK_ENABLE();
#endif
}

static void ADC_DisableClock(void)
{
#if defined(__HAL_RCC_ADC_CLK_DISABLE)
    __HAL_RCC_ADC_CLK_DISABLE();
#elif defined(__HAL_RCC_ADC1_CLK_DISABLE)
    __HAL_RCC_ADC1_CLK_DISABLE();
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
            ADC_EnableClock();
            g_adc[inst].Instance = ADC1;
            break;
#endif
        default:
            return hwADC_InvalidParameter;
    }

#if defined(ADC_CLOCK_SYNC_PCLK_DIV4)
    g_adc[inst].Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
#elif defined(ADC_CLOCK_ASYNC_DIV4)
    g_adc[inst].Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
#endif

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

#if defined(ADC_CFGR1_DMAEN) || defined(ADC_CFGR_DMAEN)
    g_adc[inst].Init.DMAContinuousRequests = DISABLE;
#endif

#if defined(ADC_OVR_DATA_OVERWRITTEN)
    g_adc[inst].Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
#endif

#if defined(ADC_OVERSAMPLING_MODE_DISABLE)
    g_adc[inst].Init.OversamplingMode = ADC_OVERSAMPLING_MODE_DISABLE;
#elif defined(DISABLE)
    g_adc[inst].Init.OversamplingMode = DISABLE;
#endif

#if defined(ADC_TRIGGER_FREQ_HIGH)
    g_adc[inst].Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
#endif

#if defined(ADC_SAMPLINGTIME_COMMON_1)
    g_adc[inst].Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
#endif

#if defined(ADC_SAMPLINGTIME_COMMON_2)
    g_adc[inst].Init.SamplingTimeCommon2 = ADC_SAMPLETIME_160CYCLES_5;
#endif

    if (HAL_ADC_Init(&g_adc[inst]) != HAL_OK)
        return hwADC_HwError;

#if defined(ADC_SINGLE_ENDED)
    if (HAL_ADCEx_Calibration_Start(&g_adc[inst], ADC_SINGLE_ENDED) != HAL_OK)
        return hwADC_HwError;
#else
    if (HAL_ADCEx_Calibration_Start(&g_adc[inst]) != HAL_OK)
        return hwADC_HwError;
#endif

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
            ADC_DisableClock();
            break;
#endif
        default:
            return hwADC_InvalidParameter;
    }

    return hwADC_OK;
}

void ADC_NVIC_Init(void)
{
#if defined(ADC1_COMP_IRQn)
    HAL_NVIC_SetPriority(ADC1_COMP_IRQn, ADC_IRQ_NVIC_PRIORITY, ADC_IRQ_NVIC_SUB_PRIORITY);
    HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);
#elif defined(ADC1_IRQn)
    HAL_NVIC_SetPriority(ADC1_IRQn, ADC_IRQ_NVIC_PRIORITY, ADC_IRQ_NVIC_SUB_PRIORITY);
    HAL_NVIC_EnableIRQ(ADC1_IRQn);
#elif defined(ADC_IRQn)
    HAL_NVIC_SetPriority(ADC_IRQn, ADC_IRQ_NVIC_PRIORITY, ADC_IRQ_NVIC_SUB_PRIORITY);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
#endif
}

void ADC_NVIC_DeInit(void)
{
#if defined(ADC1_COMP_IRQn)
    HAL_NVIC_DisableIRQ(ADC1_COMP_IRQn);
#elif defined(ADC1_IRQn)
    HAL_NVIC_DisableIRQ(ADC1_IRQn);
#elif defined(ADC_IRQn)
    HAL_NVIC_DisableIRQ(ADC_IRQn);
#endif
}

hwADC_OpStatus ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if (inst >= hwADC_Instance_MAX || ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel = ADC_Channel_To_HAL(ch);

    if (cfg.Channel == 0 && ch != hwADC_Channel_Index_0)
        return hwADC_InvalidParameter;

    cfg.Rank = ADC_REGULAR_RANK_1;

#if defined(ADC_SAMPLETIME_160CYCLES_5)
    cfg.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;
#elif defined(ADC_SAMPLETIME_247CYCLES_5)
    cfg.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
#elif defined(ADC_SAMPLETIME_79CYCLES_5)
    cfg.SamplingTime = ADC_SAMPLETIME_79CYCLES_5;
#endif

#if defined(ADC_SINGLE_ENDED)
    cfg.SingleDiff = ADC_SINGLE_ENDED;
#endif

#if defined(ADC_OFFSET_NONE)
    cfg.OffsetNumber = ADC_OFFSET_NONE;
#endif

#if defined(ADC_CHANNEL_CONF_OFFSET)
    cfg.Offset = 0;
#elif defined(ADC_OFFSET_NONE)
    cfg.Offset = 0;
#endif

    if (HAL_ADC_ConfigChannel(&g_adc[inst], &cfg) != HAL_OK)
        return hwADC_HwError;

    return hwADC_OK;
}

#endif // STM32U0