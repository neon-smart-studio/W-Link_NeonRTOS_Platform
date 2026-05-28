#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "ADC/ADC.h"

#ifdef DEVICE_NUVOTON

#include "ADC_Nuvoton.h"

#include "ADC/Pin/Nuvoton/ADC_Pin_Nuvoton.h"

bool ADC_NVIC_Init_Status = false;
bool ADC_Instance_Init_Status[hwADC_Instance_MAX] = {false};
bool ADC_Channel_Init_Status[hwADC_Channel_Index_MAX] = {false};

NeonRTOS_MsgQ_t ADC_Channel_SyncQueue[hwADC_Instance_MAX] = {NULL};

static volatile hwADC_Instance adc_active_inst = hwADC_Instance_1;

bool ADC_IsInstanceChannelUsed(hwADC_Instance inst)
{
    for(hwADC_Channel_Index ch = 0; ch < hwADC_Channel_Index_MAX; ch++)
    {
        if(ADC_Channel_Init_Status[ch] &&
           ADC_Channel_Def_Table[ch].inst == inst)
        {
            return true;
        }
    }

    return false;
}

bool ADC_IsAnyInstanceUsed(void)
{
    for(hwADC_Instance inst = 0; inst < hwADC_Instance_MAX; inst++)
    {
        if(ADC_Instance_Init_Status[inst])
            return true;
    }

    return false;
}

void ADC_ConvCpltCallback(uint16_t raw)
{
    ADC_QueueItem item;

    item.raw = raw;
    
    NeonRTOS_MsgQWrite(&ADC_Channel_SyncQueue[adc_active_inst],
                        &item,
                        NEONRT_NO_WAIT);
}

hwADC_OpResult ADC_Channel_Init(hwADC_Channel_Index ch)
{
    if(ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;

    if(inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    ADC_GPIO_ConfigAF(ch);

    if(!ADC_Instance_Init_Status[inst])
    {
        if(NeonRTOS_MsgQCreate(&ADC_Channel_SyncQueue[inst],
                               "adc",
                               sizeof(ADC_QueueItem),
                               1) != NeonRTOS_OK)
        {
            return hwADC_MemoryError;
        }

        ADC_Instance_Init(inst);

        if(!ADC_NVIC_Init_Status)
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

hwADC_OpResult ADC_Channel_DeInit(hwADC_Channel_Index ch)
{
    if(ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;

    if(inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    if(!ADC_Channel_Init_Status[ch])
    {
        return hwADC_OK;
    }
        
    ADC_Channel_Init_Status[ch] = false;

    ADC_GPIO_DeConfigAF(ch);

    gpio_pin_init_status[ADC_Channel_Def_Table[ch].adc_pin] = false;

    if(!ADC_IsInstanceChannelUsed(inst))
    {
        ADC_Instance_DeInit(inst);

        if(ADC_Channel_SyncQueue[inst] != NULL)
        {
            NeonRTOS_MsgQDelete(&ADC_Channel_SyncQueue[inst]);
            ADC_Channel_SyncQueue[inst] = NULL;
        }

        ADC_Instance_Init_Status[inst] = false;
    }

    if(!ADC_IsAnyInstanceUsed() && ADC_NVIC_Init_Status)
    {
        ADC_NVIC_DeInit();
        ADC_NVIC_Init_Status = false;
    }

    return hwADC_OK;
}

hwADC_OpResult ADC_Read_MiniVolt(hwADC_Channel_Index ch, float *readMv)
{
    if(readMv == NULL)
    {
        return hwADC_InvalidParameter;
    }

    if(ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    if(!ADC_Channel_Init_Status[ch])
    {
        return hwADC_NotInit;
    }

    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;

    if(inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    adc_active_inst = inst;

    if (ADC_ConfigChannel(inst, ch) < hwADC_OK)
        return hwADC_HwError;

    if (ADC_ChannelStartConversion(inst, ch) < hwADC_OK)
        return hwADC_HwError;

    ADC_QueueItem item;

    if(NeonRTOS_MsgQRead(&ADC_Channel_SyncQueue[inst],
                         &item,
                         ADC_CONV_TIMEOUT_MS) != NeonRTOS_OK)
    {
        ADC_ChannelStopConversion(inst, ch);
        return hwADC_HwError;
    }

    uint32_t raw = item.raw;

    /*
     * NUC4x2 ADC is 12-bit.
     * Vref = AVDD, normally 3300mV.
     */
    *readMv = ((float)raw * ADC_VREF_MV) / ADC_MAX_COUNT;

    return hwADC_OK;
}

#endif // DEVICE_NUVOTON