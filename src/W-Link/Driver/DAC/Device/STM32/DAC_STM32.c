#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "soc.h"

#include "GPIO/GPIO.h"

#include "DAC/DAC.h"

#ifdef DEVICE_STM32

#include "GPIO/Device/STM32/GPIO_STM32.h"

#include "DAC/Pin/STM32/DAC_Pin_STM32.h"

#if defined(DAC_BASE) || defined(DAC1_BASE) || defined(DAC2_BASE) || defined(DAC3_BASE) || defined(DAC4_BASE)

#include "DAC_STM32.h"

static bool DAC_Instance_Init_Status[hwDAC_Instance_MAX] = {false};
static bool DAC_Channel_Init_Status[hwDAC_Channel_Index_MAX] = {false};

static bool DAC_IsInstanceUsed(hwDAC_Instance inst)
{
    for (size_t i = 0; i < hwDAC_Channel_Index_MAX; i++)
    {
        if (DAC_Channel_Init_Status[i] &&
            DAC_Channel_Def_Table[i].inst == inst)
        {
            return true;
        }
    }

    return false;
}

hwDAC_OpResult hwDAC_Channel_Init(hwDAC_Channel_Index ch)
{
    if (ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    hwDAC_Instance inst = DAC_Channel_Def_Table[ch].inst;

    if (inst >= hwDAC_Instance_MAX)
        return hwDAC_InvalidParameter;

    if (DAC_Channel_Init_Status[ch])
        return hwDAC_OK;

    GPIO_TypeDef *dac_pin_soc_base =
        GPIO_Map_Soc_Base(DAC_Channel_Def_Table[ch].dac_pin);

    uint16_t dac_soc_pin =
        GPIO_Map_Soc_Pin(DAC_Channel_Def_Table[ch].dac_pin);

    if (dac_soc_pin == 0 || dac_pin_soc_base == NULL)
        return hwDAC_InvalidParameter;

    GPIO_Enable_RCC_Clock(dac_pin_soc_base);

    GPIO_InitTypeDef g_dac_pin = {0};
    g_dac_pin.Pin  = dac_soc_pin;
    g_dac_pin.Mode = GPIO_MODE_ANALOG;
    g_dac_pin.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(dac_pin_soc_base, &g_dac_pin);

    if (!DAC_Instance_Init_Status[inst])
    {
        hwDAC_OpResult status = DAC_Instance_Init(inst);

        if (status != hwDAC_OK)
        {
            HAL_GPIO_DeInit(dac_pin_soc_base, dac_soc_pin);
            return status;
        }

        DAC_Instance_Init_Status[inst] = true;
    }

    hwDAC_OpResult status = DAC_ConfigChannel(inst, ch);
    if (status != hwDAC_OK)
    {
        HAL_GPIO_DeInit(dac_pin_soc_base, dac_soc_pin);
        return status;
    }

    status = DAC_StartChannel(inst, ch);
    if (status != hwDAC_OK)
    {
        HAL_GPIO_DeInit(dac_pin_soc_base, dac_soc_pin);
        return status;
    }

    gpio_pin_init_status[DAC_Channel_Def_Table[ch].dac_pin] = true;
    DAC_Channel_Init_Status[ch] = true;

    return hwDAC_OK;
}

hwDAC_OpResult hwDAC_Channel_DeInit(hwDAC_Channel_Index ch)
{
    if (ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    hwDAC_Instance inst = DAC_Channel_Def_Table[ch].inst;

    if (inst >= hwDAC_Instance_MAX)
        return hwDAC_InvalidParameter;

    if (!DAC_Channel_Init_Status[ch])
        return hwDAC_OK;

    GPIO_TypeDef *dac_pin_soc_base =
        GPIO_Map_Soc_Base(DAC_Channel_Def_Table[ch].dac_pin);

    uint16_t dac_soc_pin =
        GPIO_Map_Soc_Pin(DAC_Channel_Def_Table[ch].dac_pin);

    if (dac_soc_pin == 0 || dac_pin_soc_base == NULL)
        return hwDAC_InvalidParameter;

    DAC_StopChannel(inst, ch);

    DAC_Channel_Init_Status[ch] = false;

    if (!DAC_IsInstanceUsed(inst) && DAC_Instance_Init_Status[inst])
    {
        DAC_Instance_DeInit(inst);
        DAC_Instance_Init_Status[inst] = false;
    }

    HAL_GPIO_DeInit(dac_pin_soc_base, dac_soc_pin);
    gpio_pin_init_status[DAC_Channel_Def_Table[ch].dac_pin] = false;

    return hwDAC_OK;
}

hwDAC_OpResult hwDAC_Write_MiniVolt(hwDAC_Channel_Index ch, float mv)
{
    if (ch >= hwDAC_Channel_Index_MAX)
        return hwDAC_InvalidParameter;

    if (!DAC_Channel_Init_Status[ch])
        return hwDAC_NotInit;

    if (mv < 0.0f)
        mv = 0.0f;

    if (mv > DAC_VREF_MV)
        mv = DAC_VREF_MV;

    uint32_t raw = (uint32_t)((mv / DAC_VREF_MV) * DAC_MAX_COUNT);

    hwDAC_Instance inst = DAC_Channel_Def_Table[ch].inst;

    if (inst >= hwDAC_Instance_MAX)
        return hwDAC_InvalidParameter;

    return DAC_WriteRaw(inst, ch, raw);
}

#endif // DAC_BASE || DAC1_BASE || DAC2_BASE || DAC3_BASE || DAC4_BASE

#endif // DEVICE_STM32