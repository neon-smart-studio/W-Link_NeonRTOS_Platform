#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#include "ADC/ADC.h"

#ifdef DEVICE_TIMSP432E

#include "ADC_TIMSP432.h"

#include "GPIO/Device/TIMSP432/GPIO_TIMSP432E.h"

#include "ADC/Pin/TIMSP432/ADC_Pin_TIMSP432E.h"

typedef struct {
    uint32_t raw;
} ADC_QueueItem;

static bool ADC_NVIC_Init_Status = false;
static bool ADC_Instance_Init_Status[hwADC_Instance_MAX] = {false};
static bool ADC_Channel_Init_Status[hwADC_Channel_Index_MAX] = {false};

NeonRTOS_MsgQ_t ADC_Channel_SyncQueue[hwADC_Instance_MAX] = {NULL};

static volatile hwADC_Instance adc_active_inst = hwADC_Instance_1;

static uint32_t ADC_Instance_To_Base(hwADC_Instance inst)
{
    switch(inst)
    {
        case hwADC_Instance_1: return ADC0_BASE;
        case hwADC_Instance_2: return ADC1_BASE;
        default: return 0;
    }
}

static uint32_t ADC_Instance_To_Periph(hwADC_Instance inst)
{
    switch(inst)
    {
        case hwADC_Instance_1: return SYSCTL_PERIPH_ADC0;
        case hwADC_Instance_2: return SYSCTL_PERIPH_ADC1;
        default: return 0;
    }
}

static uint32_t ADC_Channel_Index_To_Ctl(hwADC_Channel_Index ch)
{
    switch(ch)
    {
        case hwADC_Channel_Index_0: return ADC_CTL_CH0;
        case hwADC_Channel_Index_1: return ADC_CTL_CH1;
        case hwADC_Channel_Index_2: return ADC_CTL_CH2;
        case hwADC_Channel_Index_3: return ADC_CTL_CH3;
        case hwADC_Channel_Index_4: return ADC_CTL_CH4;
        case hwADC_Channel_Index_5: return ADC_CTL_CH5;
        case hwADC_Channel_Index_6: return ADC_CTL_CH6;
        case hwADC_Channel_Index_7: return ADC_CTL_CH7;
        default: return 0;
    }
}

static hwADC_OpResult ADC_Instance_Init(hwADC_Instance inst)
{
    uint32_t periph = ADC_Instance_To_Periph(inst);
    uint32_t base   = ADC_Instance_To_Base(inst);

    if(base == 0 || periph == 0)
        return hwADC_InvalidParameter;

    MAP_SysCtlPeripheralEnable(periph);
    while(!MAP_SysCtlPeripheralReady(periph));

    MAP_ADCSequenceDisable(base, 3);

    MAP_ADCSequenceConfigure(base, 3, ADC_TRIGGER_PROCESSOR, 0);

    MAP_ADCIntClear(base, 3);
    MAP_ADCIntEnable(base, 3);

    MAP_ADCSequenceEnable(base, 3);

    return hwADC_OK;
}

static hwADC_OpResult ADC_Instance_DeInit(hwADC_Instance inst)
{
    uint32_t periph = ADC_Instance_To_Periph(inst);
    uint32_t base   = ADC_Instance_To_Base(inst);

    if(base == 0 || periph == 0)
        return hwADC_InvalidParameter;

    MAP_ADCIntDisable(base, 3);
    MAP_ADCSequenceDisable(base, 3);

    MAP_SysCtlPeripheralDisable(periph);

    return hwADC_OK;
}

static hwADC_OpResult ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    uint32_t base = ADC_Instance_To_Base(inst);
    uint32_t ctl_ch = ADC_Channel_Index_To_Ctl(ch);

    if(base == 0 || ctl_ch == 0)
        return hwADC_InvalidParameter;

    MAP_ADCSequenceDisable(base, 3);

    MAP_ADCSequenceStepConfigure(base, 3, 0, ctl_ch | ADC_CTL_IE | ADC_CTL_END);

    MAP_ADCIntClear(base, 3);
    MAP_ADCSequenceEnable(base, 3);

    return hwADC_OK;
}

static void ADC_NVIC_Init(void)
{
    MAP_IntEnable(INT_ADC0SS3);
    MAP_IntEnable(INT_ADC1SS3);
}

static void ADC_NVIC_DeInit(void)
{
    MAP_IntDisable(INT_ADC0SS3);
    MAP_IntDisable(INT_ADC1SS3);
}

static bool ADC_IsInstanceChannelUsed(hwADC_Instance inst)
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

static bool ADC_IsAnyInstanceUsed(void)
{
    for(hwADC_Instance inst = 0; inst < hwADC_Instance_MAX; inst++)
    {
        if(ADC_Instance_Init_Status[inst])
            return true;
    }

    return false;
}

static void ADC_ConvCpltCallback(uint16_t raw)
{
    ADC_QueueItem item;

    item.raw = raw;
    
    NeonRTOS_MsgQWrite(&ADC_Channel_SyncQueue[adc_active_inst],
                        &item,
                        NEONRT_NO_WAIT);
}

static void ADC0SS3_Handler(void)
{
    uint32_t raw;

    MAP_ADCIntClear(ADC0_BASE, 3);
    MAP_ADCSequenceDataGet(ADC0_BASE, 3, &raw);

    ADC_ConvCpltCallback((uint16_t)raw);
}

static void ADC1SS3_Handler(void)
{
    uint32_t raw;

    MAP_ADCIntClear(ADC1_BASE, 3);
    MAP_ADCSequenceDataGet(ADC1_BASE, 3, &raw);

    ADC_ConvCpltCallback((uint16_t)raw);
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

    GPIO_Enable_Port_Clock(portBase);

    MAP_GPIOPinTypeADC(portBase, pinMask);

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

    MAP_GPIOPinTypeGPIOInput(portBase, pinMask);

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

    uint32_t base = ADC_Instance_To_Base(inst);
    if(base == 0)
    {
        return hwADC_InvalidParameter;
    }

    if (ADC_ConfigChannel(inst, ch) < hwADC_OK)
    {
        return hwADC_HwError;
    }

    adc_active_inst = inst;

    MAP_ADCProcessorTrigger(base, 3);

    ADC_QueueItem item;

    if(NeonRTOS_MsgQRead(&ADC_Channel_SyncQueue[inst],
                         &item,
                         ADC_CONV_TIMEOUT_MS) != NeonRTOS_OK)
    {
        uint32_t base = ADC_Instance_To_Base(inst);
        MAP_ADCSequenceDisable(base, 3);
        MAP_ADCSequenceEnable(base, 3);
        return hwADC_HwError;
    }

    uint32_t raw = item.raw;

    *readMv = ((float)raw * ADC_VREF_MV) / ADC_MAX_COUNT;

    return hwADC_OK;
}

#endif // DEVICE_TIMSP432E