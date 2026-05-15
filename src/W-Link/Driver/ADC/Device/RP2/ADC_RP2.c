#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#include "ADC/ADC.h"

#ifdef DEVICE_RP2

#include "ADC_RP2.h"

#include "GPIO/Device/RP2/GPIO_RP2.h"

#include "ADC/Pin/RP2/ADC_Pin_RP2.h"

bool ADC_NVIC_Init_Status = false;
bool ADC_Instance_Init_Status[hwADC_Instance_MAX] = {false};
bool ADC_Channel_Init_Status[hwADC_Channel_Index_MAX] = {false};
NeonRTOS_MsgQ_t ADC_Channel_SyncQueue[hwADC_Instance_MAX] = {NULL};

static void ADC_IRQ_Handler(void)
{
    ADC_QueueItem item;

    if (!adc_fifo_is_empty()) {
        item.raw = adc_fifo_get();
        adc_run(false);

        adc_fifo_drain();
        adc_irq_set_enabled(false);

        NeonRTOS_MsgQWrite(&ADC_Channel_SyncQueue[hwADC_Instance_1], &item, NEONRT_NO_WAIT);
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

hwADC_OpResult ADC_Channel_Init(hwADC_Channel_Index ch)
{
    if (ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;
    hwGPIO_Pin adc_pin = ADC_Channel_Def_Table[ch].adc_pin;

    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    // Select NULL function to make output driver hi-Z
    gpio_set_function(adc_pin, GPIO_FUNC_NULL);
    // Also disable digital pulls and digital receiver
    gpio_disable_pulls(adc_pin);
    gpio_set_input_enabled(adc_pin, false);

    if (!ADC_Instance_Init_Status[inst])
    {
        if (NeonRTOS_MsgQCreate(&ADC_Channel_SyncQueue[inst],
                                "adc",
                                sizeof(ADC_QueueItem),
                                1) != NeonRTOS_OK)
        {
            return hwADC_MemoryError;
        }

        // ADC is in an unknown state. We should start by resetting it
        reset_unreset_block_num_wait_blocking(RESET_ADC);

        // Now turn it back on. Staging of clock etc is handled internally
        adc_hw->cs = ADC_CS_EN_BITS;

        // Internal staging completes in a few cycles, but poll to be sure
        while (!(adc_hw->cs & ADC_CS_READY_BITS)) {
            tight_loop_contents();
        }

        adc_fifo_setup(
            true,   // enable fifo
            false,  // no dma
            1,      // irq when >= 1 sample
            false,  // no error bit
            false   // keep full 12-bit sample
        );

        adc_fifo_drain();

        if (!ADC_NVIC_Init_Status)
        {
            irq_set_exclusive_handler(ADC_IRQ_FIFO, ADC_IRQ_Handler);
            irq_set_enabled(ADC_IRQ_FIFO, false);
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
    if (ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;
    hwGPIO_Pin adc_pin = ADC_Channel_Def_Table[ch].adc_pin;

    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    if (!ADC_Channel_Init_Status[ch])
        return hwADC_OK;

    ADC_Channel_Init_Status[ch] = false;

    gpio_deinit(adc_pin);

    gpio_pin_init_status[ADC_Channel_Def_Table[ch].adc_pin] = false;

    if (!ADC_IsInstanceChannelUsed(inst))
    {
        adc_hw->cs = 0;

        reset_block_num(RESET_ADC);

        while (!(resets_hw->reset & (1u << RESET_ADC))) {
            tight_loop_contents();
        }

        if (ADC_Channel_SyncQueue[inst] != NULL)
        {
            NeonRTOS_MsgQDelete(&ADC_Channel_SyncQueue[inst]);
            ADC_Channel_SyncQueue[inst] = NULL;
        }

        ADC_Instance_Init_Status[inst] = false;
    }

    if (!ADC_IsAnyInstanceUsed() && ADC_NVIC_Init_Status)
    {
        irq_set_enabled(ADC_IRQ_FIFO, false);
        ADC_NVIC_Init_Status = false;
    }

    return hwADC_OK;
}

hwADC_OpResult ADC_Read_MiniVolt(hwADC_Channel_Index ch, float *readMv)
{
    if (!readMv || ch >= hwADC_Channel_Index_MAX)
        return hwADC_InvalidParameter;

    hwADC_Instance inst = ADC_Channel_Def_Table[ch].inst;
    hwGPIO_Pin adc_pin = ADC_Channel_Def_Table[ch].adc_pin;

    if (inst >= hwADC_Instance_MAX)
        return hwADC_InvalidParameter;

    if (!ADC_Channel_Init_Status[ch])
        return hwADC_NotInit;

    adc_fifo_drain();

    adc_select_input(ch);

    adc_irq_set_enabled(true);
    adc_run(true);

    ADC_QueueItem item;

    if (NeonRTOS_MsgQRead(&ADC_Channel_SyncQueue[inst],
                          &item,
                          ADC_CONV_TIMEOUT_MS) != NeonRTOS_OK)
    {
        adc_run(false);
        adc_irq_set_enabled(false);
        adc_fifo_drain();
        return hwADC_HwError;
    }

    adc_run(false);
    adc_irq_set_enabled(false);
    adc_fifo_drain();

    *readMv = ((float)item.raw * ADC_VREF_MV) / ADC_MAX_COUNT;

    return hwADC_OK;
}

#endif //DEVICE_RP2