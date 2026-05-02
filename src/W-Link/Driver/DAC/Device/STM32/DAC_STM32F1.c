#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#ifdef STM32F1
#if defined(DAC_BASE)

#include "DAC/DAC.h"
#include "DAC_STM32.h"

DAC_HandleTypeDef g_dac[hwDAC_Instance_MAX];

static uint32_t DAC_Channel_To_HAL(hwDAC_Channel_Index ch)
{
    switch (ch)
    {
#if defined(DAC_CHANNEL_1)
        case hwDAC_Channel_Index_0: return DAC_CHANNEL_1;
#endif
#if defined(DAC_CHANNEL_2)
        case hwDAC_Channel_Index_1: return DAC_CHANNEL_2;
#endif
        default: return 0;
    }
}

hwDAC_OpStatus DAC_Instance_Init(hwDAC_Instance inst)
{
    if (inst >= hwDAC_Instance_MAX)
        return hwDAC_InvalidParameter;

    switch (inst)
    {
        case hwDAC_Instance_1:
            __HAL_RCC_DAC_CLK_ENABLE();
            g_dac[inst].Instance = DAC;
            break;

        default:
            return hwDAC_InvalidParameter;
    }

    if (HAL_DAC_Init(&g_dac[inst]) != HAL_OK)
        return hwDAC_HwError;

    return hwDAC_OK;
}

hwDAC_OpStatus DAC_Instance_DeInit(hwDAC_Instance inst)
{
    if (inst >= hwDAC_Instance_MAX)
        return hwDAC_InvalidParameter;

    HAL_DAC_DeInit(&g_dac[inst]);

    switch (inst)
    {
        case hwDAC_Instance_1:
            __HAL_RCC_DAC_CLK_DISABLE();
            break;

        default:
            return hwDAC_InvalidParameter;
    }

    return hwDAC_OK;
}

hwDAC_OpStatus DAC_ConfigChannel(hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (inst >= hwDAC_Instance_MAX || ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    uint32_t hal_ch = DAC_Channel_To_HAL(ch);

    if (hal_ch == 0)
        return hwDAC_InvalidParameter;

    DAC_ChannelConfTypeDef cfg = {0};
    cfg.DAC_Trigger      = DAC_TRIGGER_NONE;
    cfg.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

    if (HAL_DAC_ConfigChannel(&g_dac[inst], &cfg, hal_ch) != HAL_OK)
        return hwDAC_HwError;

    return hwDAC_OK;
}

hwDAC_OpStatus DAC_StartChannel(hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (inst >= hwDAC_Instance_MAX || ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    uint32_t hal_ch = DAC_Channel_To_HAL(ch);

    if (hal_ch == 0)
        return hwDAC_InvalidParameter;

    if (HAL_DAC_Start(&g_dac[inst], hal_ch) != HAL_OK)
        return hwDAC_HwError;

    return hwDAC_OK;
}

hwDAC_OpStatus DAC_StopChannel(hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (inst >= hwDAC_Instance_MAX || ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    uint32_t hal_ch = DAC_Channel_To_HAL(ch);

    if (hal_ch == 0)
        return hwDAC_InvalidParameter;

    if (HAL_DAC_Stop(&g_dac[inst], hal_ch) != HAL_OK)
        return hwDAC_HwError;

    return hwDAC_OK;
}

hwDAC_OpStatus DAC_WriteRaw(hwDAC_Instance inst, hwDAC_Channel_Index ch, uint32_t raw)
{
    if (inst >= hwDAC_Instance_MAX || ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    uint32_t hal_ch = DAC_Channel_To_HAL(ch);

    if (hal_ch == 0)
        return hwDAC_InvalidParameter;

    if (HAL_DAC_SetValue(&g_dac[inst], hal_ch, DAC_ALIGN_12B_R, raw) != HAL_OK)
        return hwDAC_HwError;

    return hwDAC_OK;
}

#endif // defined(DAC_BASE)
#endif // STM32F1