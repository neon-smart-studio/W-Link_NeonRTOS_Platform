#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#include "ADC/ADC.h"

#include "ADC/ADC_Pin.h"

#ifdef DEVICE_STM32

#include "ADC_STM32.h"

#include "GPIO/Device/GPIO_STM32.h"

bool ADC_NVIC_Init_Status = false;
bool ADC_Instance_Init_Status[hwADC_Instance_MAX] = {false};
bool ADC_Channel_Init_Status[hwADC_Channel_Index_MAX] = {false};
NeonRTOS_MsgQ_t ADC_Channel_SyncQueue[hwADC_Instance_MAX] = {NULL};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    ADC_QueueItem item;

    for (hwADC_Instance inst = 0; inst < hwADC_Instance_MAX; inst++)
    {
        if (hadc == &g_adc[inst])
        {
            item.raw = HAL_ADC_GetValue(hadc);
            NeonRTOS_MsgQWrite(&ADC_Channel_SyncQueue[inst], &item, NEONRT_NO_WAIT);
            break;
        }
    }
}

bool ADC_IsInstanceChannelUsed(hwADC_Instance inst)
{
    for (hwADC_Channel_Index ch = 0; ch < hwADC_Channel_Index_MAX; ch++)
    {
        if (ADC_Channel_Init_Status[ch] &&
            ADC_Channel_Def_Table[ch].inst == inst)
        {
            return true;
        }
    }

    return false;
}

bool ADC_IsAnyInstanceUsed(void)
{
    for (hwADC_Instance inst = 0; inst < hwADC_Instance_MAX; inst++)
    {
        if (ADC_Instance_Init_Status[inst])
            return true;
    }

    return false;
}

hwADC_OpStatus ADC_Channel_Init(hwADC_Channel_Index ch)
{
    if (ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;

    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    GPIO_TypeDef *adc_pin_soc_base =
        GPIO_Map_Soc_Base(ADC_Channel_Def_Table[ch].adc_pin);

    uint16_t adc_soc_pin =
        GPIO_Map_Soc_Pin(ADC_Channel_Def_Table[ch].adc_pin);

    if (adc_soc_pin == 0 || adc_pin_soc_base == NULL)
        return hwADC_InvalidParameter;

    if (!ADC_Instance_Init_Status[inst])
    {
        if (NeonRTOS_MsgQCreate(&ADC_Channel_SyncQueue[inst],
                                "adc",
                                sizeof(ADC_QueueItem),
                                1) != NeonRTOS_OK)
        {
            return hwADC_MemoryError;
        }
    }

    GPIO_Enable_RCC_Clock(adc_pin_soc_base);

    GPIO_InitTypeDef g_adc_pin = {0};
    g_adc_pin.Pin  = adc_soc_pin;
    g_adc_pin.Mode = GPIO_MODE_ANALOG;
    g_adc_pin.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(adc_pin_soc_base, &g_adc_pin);

    if (!ADC_Instance_Init_Status[inst])
    {
        hwADC_OpStatus status = ADC_Instance_Init(inst);

        if (status != hwADC_OK)
        {
            HAL_GPIO_DeInit(adc_pin_soc_base, adc_soc_pin);
            NeonRTOS_MsgQDelete(&ADC_Channel_SyncQueue[inst]);
            ADC_Channel_SyncQueue[inst] = NULL;
            return status;
        }

        if (!ADC_NVIC_Init_Status)
        {
            ADC_NVIC_Init();
            ADC_NVIC_Init_Status = true;
        }

        ADC_Instance_Init_Status[inst] = true;
    }

    gpio_pin_init_status[ADC_Channel_Def_Table[ch].adc_pin] = true;
    ADC_Channel_Init_Status[ch] = true;

    return hwADC_OK;
}

hwADC_OpStatus ADC_Channel_DeInit(hwADC_Channel_Index ch)
{
    if (ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;

    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    if (!ADC_Channel_Init_Status[ch])
        return hwADC_OK;

    GPIO_TypeDef *adc_pin_soc_base =
        GPIO_Map_Soc_Base(ADC_Channel_Def_Table[ch].adc_pin);

    uint16_t adc_soc_pin =
        GPIO_Map_Soc_Pin(ADC_Channel_Def_Table[ch].adc_pin);

    if (adc_soc_pin == 0 || adc_pin_soc_base == NULL)
        return hwADC_InvalidParameter;

    ADC_Channel_Init_Status[ch] = false;

    HAL_GPIO_DeInit(adc_pin_soc_base, adc_soc_pin);
    gpio_pin_init_status[ADC_Channel_Def_Table[ch].adc_pin] = false;

    if (!ADC_IsInstanceChannelUsed(inst))
    {
        ADC_Instance_DeInit(inst);

        if (ADC_Channel_SyncQueue[inst] != NULL)
        {
            NeonRTOS_MsgQDelete(&ADC_Channel_SyncQueue[inst]);
            ADC_Channel_SyncQueue[inst] = NULL;
        }

        ADC_Instance_Init_Status[inst] = false;
    }

    if (!ADC_IsAnyInstanceUsed() && ADC_NVIC_Init_Status)
    {
        ADC_NVIC_DeInit();
        ADC_NVIC_Init_Status = false;
    }

    return hwADC_OK;
}

hwADC_OpStatus ADC_Read_MiniVolt(hwADC_Channel_Index ch, float *readMv)
{
    if (!readMv || ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;

    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    if (!ADC_Channel_Init_Status[ch])
        return hwADC_NotInit;

    ADC_HandleTypeDef *hadc = &g_adc[inst];

    if (ADC_ConfigChannel(inst, ch) != hwADC_OK)
        return hwADC_HwError;

    if (HAL_ADC_Start_IT(hadc) != HAL_OK)
        return hwADC_HwError;

    ADC_QueueItem item;

    if (NeonRTOS_MsgQRead(&ADC_Channel_SyncQueue[inst],
                          &item,
                          ADC_CONV_TIMEOUT_MS) != NeonRTOS_OK)
    {
        HAL_ADC_Stop_IT(hadc);
        return hwADC_HwError;
    }

    HAL_ADC_Stop_IT(hadc);

    *readMv = ((float)item.raw * ADC_VREF_MV) / ADC_MAX_COUNT;

    return hwADC_OK;
}

#endif //DEVICE_STM32