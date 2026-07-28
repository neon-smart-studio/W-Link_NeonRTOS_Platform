#include <stdbool.h>
#include <stdint.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#include "ADC/ADC.h"

#ifdef DEVICE_TIMSPM0

#include "ADC_TIMSPM0.h"

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"

#include "ADC/Pin/TIMSPM0/ADC_Pin_TIMSPM0.h"

typedef struct {
    uint32_t raw;
} ADC_QueueItem;

static bool ADC_NVIC_Init_Status[hwADC_Instance_MAX] = {false};
static bool ADC_Instance_Init_Status[hwADC_Instance_MAX] = {false};
static bool ADC_Channel_Init_Status[hwADC_Channel_Index_MAX] = {false};

static NeonRTOS_MsgQ_t ADC_Channel_SyncQueue[hwADC_Instance_MAX] = {NULL};

static const char *ADC_GetQueueName(hwADC_Instance inst)
{
#if defined(ADC0_BASE)
    if(inst == hwADC_Instance_1)
    {
        return "adc0";
    }
#endif
#if defined(ADC1_BASE)
    if(inst == hwADC_Instance_2)
    {
        return "adc1";
    }
#endif
}

static const DL_ADC12_ClockConfig ADC_Clock_Config = {
    .clockSel    = DL_ADC12_CLOCK_SYSOSC,
    .freqRange   = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
    .divideRatio = DL_ADC12_CLOCK_DIVIDE_8,
};

static ADC12_Regs *ADC_Map_Soc_Base(hwADC_Instance inst)
{
    switch (inst)
    {
#if defined(ADC0_BASE)
        case hwADC_Instance_1:
            return ADC0_BASE;
#endif

#if defined(ADC1_BASE)
        case hwADC_Instance_2:
            return ADC1_BASE;
#endif

        default:
            return NULL;
    }
}

static void ADC_NVIC_Init(hwADC_Instance inst)
{
    switch (inst)
    {
#if defined(ADC0_BASE)
        case hwADC_Instance_1:
            NVIC_ClearPendingIRQ(ADC0_INT_IRQn);
            NVIC_EnableIRQ(ADC0_INT_IRQn);
            break;
#endif

#if defined(ADC1_BASE)
        case hwADC_Instance_2:
            NVIC_ClearPendingIRQ(ADC1_INT_IRQn);
            NVIC_EnableIRQ(ADC1_INT_IRQn);
            break;
#endif

        default:
            break;
    }
}

static void ADC_NVIC_DeInit(hwADC_Instance inst)
{
    switch (inst)
    {
#if defined(ADC0_BASE)
        case hwADC_Instance_1:
            NVIC_DisableIRQ(ADC0_INT_IRQn);
            NVIC_ClearPendingIRQ(ADC0_INT_IRQn);
            break;
#endif

#if defined(ADC1_BASE)
        case hwADC_Instance_2:
            NVIC_DisableIRQ(ADC1_INT_IRQn);
            NVIC_ClearPendingIRQ(ADC1_INT_IRQn);
            break;
#endif

        default:
            break;
    }
}

static hwADC_OpResult ADC_Instance_Init(hwADC_Instance inst)
{
    ADC12_Regs *adc = ADC_Map_Soc_Base(inst);

    if (adc == NULL)
    {
        return hwADC_InvalidParameter;
    }

    if (ADC_Instance_Init_Status[inst])
    {
        return hwADC_OK;
    }

    DL_ADC12_reset(adc);
    DL_ADC12_enablePower(adc);
    delay_cycles(ADC_POWER_STARTUP_DELAY);

    DL_ADC12_setClockConfig(adc, &ADC_Clock_Config);

    DL_ADC12_initSingleSample(
        adc,
        DL_ADC12_REPEAT_MODE_DISABLED,
        DL_ADC12_SAMPLING_SOURCE_AUTO,
        DL_ADC12_TRIG_SRC_SOFTWARE,
        DL_ADC12_SAMP_CONV_RES_12_BIT,
        DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);

    DL_ADC12_setPowerDownMode(adc, DL_ADC12_POWER_DOWN_MODE_MANUAL);
    DL_ADC12_setSampleTime0(adc, ADC_SAMPLE_TIME);

    DL_ADC12_clearInterruptStatus(adc, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);
    DL_ADC12_enableInterrupt(adc, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);

    DL_ADC12_enableConversions(adc);

    return hwADC_OK;
}

static hwADC_OpResult ADC_Instance_DeInit(hwADC_Instance inst)
{
    ADC12_Regs *adc = ADC_Map_Soc_Base(inst);

    if (adc == NULL)
    {
        return hwADC_InvalidParameter;
    }

    DL_ADC12_stopConversion(adc);
    DL_ADC12_disableConversions(adc);
    DL_ADC12_disableInterrupt(adc, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);
    DL_ADC12_clearInterruptStatus(adc, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);
    DL_ADC12_reset(adc);
    DL_ADC12_disablePower(adc);

    return hwADC_OK;
}

static hwADC_OpResult ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if (inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    if (ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    ADC12_Regs *adc = ADC_Map_Soc_Base(inst);
    
    uint32_t adc_input = (ADC_Channel_Def_Table[ch].hw_Idx << ADC12_MEMCTL_CHANSEL_OFS) & ADC12_MEMCTL_CHANSEL_MASK;

    if (adc == NULL)
    {
        return hwADC_InvalidParameter;
    }

    DL_ADC12_stopConversion(adc);
    DL_ADC12_disableConversions(adc);

    DL_ADC12_configConversionMem(
        adc,
        DL_ADC12_MEM_IDX_0,
        adc_input,
        DL_ADC12_REFERENCE_VOLTAGE_VDDA,
        DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0,
        DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED,
        DL_ADC12_TRIGGER_MODE_AUTO_NEXT,
        DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

    DL_ADC12_clearInterruptStatus(adc, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);
    DL_ADC12_enableConversions(adc);

    return hwADC_OK;
}

bool ADC_IsInstanceChannelUsed(hwADC_Instance inst)
{
    if (inst >= hwADC_Instance_MAX)
    {
        return false;
    }

    for (hwADC_Channel_Index ch = 0; ch < hwADC_Channel_Index_MAX; ch++)
    {
        if (ADC_Channel_Init_Status[ch])
        {
            return true;
        }
    }

    return false;
}

static void ADC_ConvCpltCallback(hwADC_Instance inst, uint16_t raw)
{
    if ((inst >= hwADC_Instance_MAX) || (ADC_Channel_SyncQueue[inst] == NULL))
    {
        return;
    }

    ADC_QueueItem item = {
        .raw = raw,
    };

    (void) NeonRTOS_MsgQWrite(&ADC_Channel_SyncQueue[inst], &item, NEONRT_NO_WAIT);
}

static void ADC_IRQHandler_Process(hwADC_Instance inst, ADC12_Regs *adc)
{
    if (DL_ADC12_getPendingInterrupt(adc) == DL_ADC12_IIDX_MEM0_RESULT_LOADED)
    {
        uint16_t raw = DL_ADC12_getMemResult(adc, DL_ADC12_MEM_IDX_0);

        ADC_ConvCpltCallback(inst, raw);
    }
}

#if defined(ADC0_BASE)
void ADC0_IRQHandler(void)
{
    ADC_IRQHandler_Process(hwADC_Instance_1, ADC0);
}
#endif

#if defined(ADC1_BASE)
void ADC1_IRQHandler(void)
{
    ADC_IRQHandler_Process(hwADC_Instance_2, ADC1);
}
#endif

hwADC_OpResult ADC_Channel_Init(hwADC_Channel_Index ch)
{
    if(ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    hwGPIO_Pin adc_pin = ADC_Channel_Def_Table[ch].adc_pin;
    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;

    if (adc_pin == hwGPIO_Pin_NC)
    {
        return hwADC_InvalidParameter;
    }

    uint32_t pin_cm = GPIO_Map_Soc_Pin_IOMUX(adc_pin);
    if(pin_cm==GPIO_SOC_IOMUX_INVALID)
    {
        return hwADC_InvalidParameter;
    }
    
    if (ADC_Channel_Init_Status[ch])
    {
        return hwADC_OK;
    }

    /*
     * ADC pins are non-IOMUX analog functions.  They are analog by default
     * after reset, but the framework may have configured the pin as GPIO
     * earlier, so explicitly return it to analog mode.
     */
    DL_GPIO_initPeripheralAnalogFunction(pin_cm);

    if (!ADC_Instance_Init_Status[inst])
    {
        if (ADC_Channel_SyncQueue[inst] == NULL)
        {
            if (NeonRTOS_MsgQCreate(&ADC_Channel_SyncQueue[inst], ADC_GetQueueName(inst), sizeof(ADC_QueueItem), 1) != NeonRTOS_OK)
            {
                DL_GPIO_initDigitalInput(pin_cm);
                return hwADC_MemoryError;
            }
        }

        hwADC_OpResult result = ADC_Instance_Init(inst);

        if (result != hwADC_OK)
        {
            NeonRTOS_MsgQDelete(&ADC_Channel_SyncQueue[inst]);
            ADC_Channel_SyncQueue[inst] = NULL;
            DL_GPIO_initDigitalInput(pin_cm);
            return result;
        }

        ADC_NVIC_Init(inst);
        ADC_NVIC_Init_Status[inst] = true;
        ADC_Instance_Init_Status[inst] = true;
    }

    gpio_pin_init_status[adc_pin] = true;
    ADC_Channel_Init_Status[ch] = true;

    return hwADC_OK;
}

hwADC_OpResult ADC_Channel_DeInit(hwADC_Channel_Index ch)
{
    if(ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    hwGPIO_Pin adc_pin = ADC_Channel_Def_Table[ch].adc_pin;
    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;

    if (adc_pin == hwGPIO_Pin_NC)
    {
        return hwADC_InvalidParameter;
    }

    uint32_t pin_cm = GPIO_Map_Soc_Pin_IOMUX(adc_pin);
    if(pin_cm==GPIO_SOC_IOMUX_INVALID)
    {
        return hwADC_InvalidParameter;
    }

    if (!ADC_Channel_Init_Status[ch])
    {
        return hwADC_OK;
    }

    ADC_Channel_Init_Status[ch] = false;
    gpio_pin_init_status[adc_pin] = false;

    DL_GPIO_initDigitalInput(pin_cm);

    if (!ADC_IsInstanceChannelUsed(inst))
    {
        if (ADC_NVIC_Init_Status[inst])
        {
            ADC_NVIC_DeInit(inst);
            ADC_NVIC_Init_Status[inst] = false;
        }

        (void) ADC_Instance_DeInit(inst);

        if (ADC_Channel_SyncQueue[inst] != NULL)
        {
            NeonRTOS_MsgQDelete(&ADC_Channel_SyncQueue[inst]);
            ADC_Channel_SyncQueue[inst] = NULL;
        }

        ADC_Instance_Init_Status[inst] = false;
    }

    return hwADC_OK;
}

hwADC_OpResult ADC_Read_MiniVolt(hwADC_Channel_Index ch, float *readMv)
{
    if(ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    hwGPIO_Pin adc_pin = ADC_Channel_Def_Table[ch].adc_pin;
    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;

    if (adc_pin == hwGPIO_Pin_NC)
    {
        return hwADC_InvalidParameter;
    }

    if ((readMv == NULL))
    {
        return hwADC_InvalidParameter;
    }

    if (!ADC_Channel_Init_Status[ch] || !ADC_Instance_Init_Status[inst] || (ADC_Channel_SyncQueue[inst] == NULL))
    {
        return hwADC_NotInit;
    }

    ADC12_Regs *adc = ADC_Map_Soc_Base(inst);
    if (adc == NULL)
    {
        return hwADC_InvalidParameter;
    }

    /*
     * Remove a late result left by a previous timed-out conversion.
     */
    ADC_QueueItem item;

    NeonRTOS_MsgQRead(&ADC_Channel_SyncQueue[inst], &item, NEONRT_NO_WAIT);

    if (ADC_ConfigChannel(inst, ch) != hwADC_OK)
    {
        return hwADC_HwError;
    }

    DL_ADC12_startConversion(adc);

    if (NeonRTOS_MsgQRead(&ADC_Channel_SyncQueue[inst], &item, ADC_CONV_TIMEOUT_MS) != NeonRTOS_OK)
    {
        DL_ADC12_stopConversion(adc);
        return hwADC_HwError;
    }

    *readMv = ((float) item.raw * ADC_VREF_MV) / ADC_MAX_COUNT;

    return hwADC_OK;
}

#endif /* DEVICE_TIMSPM0 */