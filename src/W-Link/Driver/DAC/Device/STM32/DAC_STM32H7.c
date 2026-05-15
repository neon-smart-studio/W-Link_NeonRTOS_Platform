#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#ifdef STM32H7

#if defined(DAC1_BASE) || defined(DAC_BASE)

#include "DAC/DAC.h"
#include "DAC_STM32.h"

DAC_HandleTypeDef g_dac[hwDAC_Instance_MAX];

static uint32_t DAC_Channel_To_HAL(hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (inst != hwDAC_Instance_1)
        return 0;

#if defined(DAC_CHANNEL_1)
    if (ch == hwDAC_Channel_Index_0)
        return DAC_CHANNEL_1;
#endif

#if defined(DAC_CHANNEL_2)
    if (ch == hwDAC_Channel_Index_1)
        return DAC_CHANNEL_2;
#endif

    return 0;
}

hwDAC_OpResult DAC_Instance_Init(hwDAC_Instance inst)
{
    if (inst != hwDAC_Instance_1)
        return hwDAC_InvalidParameter;

    DAC_TypeDef *instance = DAC_GetInstance();
    if (instance == NULL)
        return hwDAC_HwError;

    switch (inst)
    {
#if defined(DAC1_BASE)
        case hwDAC_Instance_1:
            __HAL_RCC_DAC12_CLK_ENABLE();
            g_dac[inst].Instance = DAC1;
            break;
#endif
    }
    
    if (HAL_DAC_Init(&g_dac[inst]) != HAL_OK)
        return hwDAC_HwError;

    return hwDAC_OK;
}

hwDAC_OpResult DAC_Instance_DeInit(hwDAC_Instance inst)
{
    if (inst != hwDAC_Instance_1)
        return hwDAC_InvalidParameter;

    if (HAL_DAC_DeInit(&g_dac[inst]) != HAL_OK)
        return hwDAC_HwError;

    switch (inst)
    {
#if defined(DAC1_BASE)
        case hwDAC_Instance_1:
            __HAL_RCC_DAC12_CLK_ENABLE();
            g_dac[inst].Instance = DAC1;
            break;
#endif
    }
    
    return hwDAC_OK;
}

hwDAC_OpResult DAC_ConfigChannel(hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (inst != hwDAC_Instance_1 || ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    uint32_t hal_ch = DAC_Channel_To_HAL(inst, ch);
    if (hal_ch == 0)
        return hwDAC_InvalidParameter;

    DAC_ChannelConfTypeDef cfg = {0};

    cfg.DAC_Trigger = DAC_TRIGGER_NONE;

    cfg.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    cfg.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    cfg.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
    cfg.DAC_UserTrimming = DAC_TRIMMING_FACTORY;

    return (HAL_DAC_ConfigChannel(&g_dac[inst], &cfg, hal_ch) == HAL_OK)
        ? hwDAC_OK
        : hwDAC_HwError;
}

hwDAC_OpResult DAC_StartChannel(hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (inst != hwDAC_Instance_1 || ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    uint32_t hal_ch = DAC_Channel_To_HAL(inst, ch);
    if (hal_ch == 0)
        return hwDAC_InvalidParameter;

    return (HAL_DAC_Start(&g_dac[inst], hal_ch) == HAL_OK)
        ? hwDAC_OK
        : hwDAC_HwError;
}

hwDAC_OpResult DAC_StopChannel(hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (inst != hwDAC_Instance_1 || ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    uint32_t hal_ch = DAC_Channel_To_HAL(inst, ch);
    if (hal_ch == 0)
        return hwDAC_InvalidParameter;

    return (HAL_DAC_Stop(&g_dac[inst], hal_ch) == HAL_OK)
        ? hwDAC_OK
        : hwDAC_HwError;
}

hwDAC_OpResult DAC_WriteRaw(hwDAC_Instance inst, hwDAC_Channel_Index ch, uint32_t raw)
{
    if (inst != hwDAC_Instance_1 || ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    uint32_t hal_ch = DAC_Channel_To_HAL(inst, ch);
    if (hal_ch == 0)
        return hwDAC_InvalidParameter;

    raw &= 0x0FFF;

    return (HAL_DAC_SetValue(&g_dac[inst], hal_ch, DAC_ALIGN_12B_R, raw) == HAL_OK)
        ? hwDAC_OK
        : hwDAC_HwError;
}

#endif // DAC1_BASE || DAC_BASE

#endif // STM32H7