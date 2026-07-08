#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#include "ADC/ADC.h"

#ifdef DEVICE_TIMSP432P

#include "ADC_TIMSP432P.h"

#include "ADC/Pin/TIMSP432/ADC_Pin_TIMSP432P.h"

bool ADC_NVIC_Init_Status = false;
bool ADC_Instance_Init_Status[hwADC_Instance_MAX] = {false};
bool ADC_Channel_Init_Status[hwADC_Channel_Index_MAX] = {false};

NeonRTOS_MsgQ_t ADC_Channel_SyncQueue[hwADC_Instance_MAX] = {NULL};

static volatile hwADC_Instance adc_active_inst = hwADC_Instance_1;

static uint32_t ADC_Channel_Index_To_Input(hwADC_Channel_Index ch)
{
    switch(ch)
    {
        case hwADC_Channel_Index_0:  return ADC_INPUT_A0;
        case hwADC_Channel_Index_1:  return ADC_INPUT_A1;
        case hwADC_Channel_Index_2:  return ADC_INPUT_A2;
        case hwADC_Channel_Index_3:  return ADC_INPUT_A3;
        case hwADC_Channel_Index_4:  return ADC_INPUT_A4;
        case hwADC_Channel_Index_5:  return ADC_INPUT_A5;
        case hwADC_Channel_Index_6:  return ADC_INPUT_A6;
        case hwADC_Channel_Index_7:  return ADC_INPUT_A7;
        case hwADC_Channel_Index_8:  return ADC_INPUT_A8;
        case hwADC_Channel_Index_9:  return ADC_INPUT_A9;
        case hwADC_Channel_Index_10: return ADC_INPUT_A10;
        case hwADC_Channel_Index_11: return ADC_INPUT_A11;
        case hwADC_Channel_Index_12: return ADC_INPUT_A12;
        case hwADC_Channel_Index_13: return ADC_INPUT_A13;
        case hwADC_Channel_Index_14: return ADC_INPUT_A14;
        case hwADC_Channel_Index_15: return ADC_INPUT_A15;
        case hwADC_Channel_Index_16: return ADC_INPUT_A16;
        case hwADC_Channel_Index_17: return ADC_INPUT_A17;
        case hwADC_Channel_Index_18: return ADC_INPUT_A18;
        case hwADC_Channel_Index_19: return ADC_INPUT_A19;
        case hwADC_Channel_Index_20: return ADC_INPUT_A20;
        case hwADC_Channel_Index_21: return ADC_INPUT_A21;
        case hwADC_Channel_Index_22: return ADC_INPUT_A22;
        case hwADC_Channel_Index_23: return ADC_INPUT_A23;
        default: return 0xFFFFFFFF;
    }
}

hwADC_OpResult ADC_Instance_Init(hwADC_Instance inst)
{
    if(inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    MAP_ADC14_enableModule();

    MAP_ADC14_initModule(
        ADC_CLOCKSOURCE_MCLK,
        ADC_PREDIVIDER_1,
        ADC_DIVIDER_1,
        0
    );

    MAP_ADC14_setResolution(ADC_14BIT);

    MAP_ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    MAP_ADC14_clearInterruptFlag(ADC_INT0);
    MAP_ADC14_enableInterrupt(ADC_INT0);

    MAP_ADC14_enableConversion();

    return hwADC_OK;
}

hwADC_OpResult ADC_Instance_DeInit(hwADC_Instance inst)
{
    if(inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    MAP_ADC14_disableInterrupt(ADC_INT0);
    MAP_ADC14_disableConversion();
    MAP_ADC14_disableModule();

    return hwADC_OK;
}

hwADC_OpResult ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if(inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    uint32_t adc_input = ADC_Channel_Index_To_Input(ch);

    if(adc_input == 0xFFFFFFFF)
    {
        return hwADC_InvalidParameter;
    }

    MAP_ADC14_disableConversion();

    MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);

    MAP_ADC14_configureConversionMemory(
        ADC_MEM0,
        ADC_VREFPOS_AVCC_VREFNEG_VSS,
        adc_input,
        false
    );

    MAP_ADC14_clearInterruptFlag(ADC_INT0);
    MAP_ADC14_enableConversion();

    return hwADC_OK;
}

void ADC_NVIC_Init(void)
{
    MAP_Interrupt_enableInterrupt(INT_ADC14);
}

void ADC_NVIC_DeInit(void)
{
    MAP_Interrupt_disableInterrupt(INT_ADC14);
}

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

void ADC14_IRQHandler(void)
{
    uint64_t status = MAP_ADC14_getEnabledInterruptStatus();

    MAP_ADC14_clearInterruptFlag(status);

    if(status & ADC_INT0)
    {
        uint16_t raw = (uint16_t)MAP_ADC14_getResult(ADC_MEM0);
        ADC_ConvCpltCallback(raw);
    }
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

    hwGPIO_Pin pin = ADC_Channel_Def_Table[ch].adc_pin;
    uint32_t portBase =  GPIO_Map_Soc_Port_Base(pin);
    uint32_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
        portBase,
        pinMask,
        GPIO_TERTIARY_MODULE_FUNCTION
    );

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
        
    hwGPIO_Pin pin = ADC_Channel_Def_Table[ch].adc_pin;
    uint32_t portBase =  GPIO_Map_Soc_Port_Base(pin);
    uint32_t pinMask = GPIO_Map_Soc_Pin_Mask(pin);

    if(portBase==0 || pinMask==0)
    {
      return hwGPIO_InvalidParameter;
    }

    MAP_GPIO_setAsInputPin(portBase, pinMask);

    ADC_Channel_Init_Status[ch] = false;

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

    if (ADC_ConfigChannel(inst, ch) < hwADC_OK)
    {
        return hwADC_HwError;
    }

    adc_active_inst = inst;

    MAP_ADC14_toggleConversionTrigger();

    ADC_QueueItem item;

    if(NeonRTOS_MsgQRead(&ADC_Channel_SyncQueue[inst],
                         &item,
                         ADC_CONV_TIMEOUT_MS) != NeonRTOS_OK)
    {
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

#endif // DEVICE_TIMSP432P