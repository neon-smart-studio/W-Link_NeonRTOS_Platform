#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "soc.h"

#include "GPIO/GPIO.h"

#include "DAC/DAC.h"

#ifdef DEVICE_TIMSPM0

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"

#include "DAC/Pin/TIMSPM0/DAC_Pin_TIMSPM0.h"

#if defined(DAC0_BASE)

#ifndef DAC_MSPM0_POWER_STARTUP_DELAY
#define DAC_MSPM0_POWER_STARTUP_DELAY        (16U)
#endif

#ifndef DAC_MSPM0_VREF_MV
#ifdef DAC_VREF_MV
#define DAC_MSPM0_VREF_MV                    ((float) DAC_VREF_MV)
#else
#define DAC_MSPM0_VREF_MV                    (3300.0f)
#endif
#endif

#define DAC_MSPM0_MAX_COUNT                  (4095U)

static bool DAC_Instance_Init_Status[hwDAC_Instance_MAX] = {false};
static bool DAC_Channel_Init_Status[hwDAC_Channel_Index_MAX] = {false};

static const DL_DAC12_Config DAC_Config = {
    .outputEnable              = DL_DAC12_OUTPUT_DISABLED,
    .resolution                = DL_DAC12_RESOLUTION_12BIT,
    .representation            = DL_DAC12_REPRESENTATION_BINARY,
    .voltageReferenceSource    = DL_DAC12_VREF_SOURCE_VDDA_VSSA,
    .amplifierSetting          = DL_DAC12_AMP_ON,
    .fifoEnable                = DL_DAC12_FIFO_DISABLED,
    .fifoTriggerSource         = DL_DAC12_FIFO_TRIGGER_SAMPLETIMER,
    .dmaTriggerEnable          = DL_DAC12_DMA_TRIGGER_DISABLED,
    .dmaTriggerThreshold       = DL_DAC12_FIFO_THRESHOLD_ONE_QTR_EMPTY,
    .sampleTimeGeneratorEnable = DL_DAC12_SAMPLETIMER_DISABLE,
    .sampleRate                = DL_DAC12_SAMPLES_PER_SECOND_500,
};

static DAC12_Regs *DAC_Map_Soc_Base(hwDAC_Instance inst)
{
    switch (inst)
    {
        /*
         * W-Link instances are numbered from 1, while MSPM0 peripheral
         * register names are numbered from 0.
         */
        case hwDAC_Instance_1:
            return DAC0_BASE;

        default:
            return NULL;
    }
}

static bool DAC_IsValidChannel(hwDAC_Channel_Index ch)
{
    if (ch >= hwDAC_Channel_Index_MAX)
    {
        return false;
    }

    const DAC_Channel_Def *def = &DAC_Channel_Def_Table[ch];

    if ((def->dac_pin == hwGPIO_Pin_NC) ||
        (DAC_Map_Soc_Base(def->inst) == NULL))
    {
        return false;
    }

    return true;
}

hwDAC_OpResult DAC_Instance_Init(hwDAC_Instance inst)
{
    DAC12_Regs *dac = DAC_Map_Soc_Base(inst);

    if (dac == NULL)
    {
        return hwDAC_InvalidParameter;
    }

    if (DAC_Instance_Init_Status[inst])
    {
        return hwDAC_OK;
    }

    DL_DAC12_reset(dac);
    DL_DAC12_enablePower(dac);
    DL_Common_delayCycles(DAC_MSPM0_POWER_STARTUP_DELAY);

    DL_DAC12_init(dac, &DAC_Config);
    DL_DAC12_output12(dac, 0U);

    return hwDAC_OK;
}

hwDAC_OpResult DAC_Instance_DeInit(hwDAC_Instance inst)
{
    DAC12_Regs *dac = DAC_Map_Soc_Base(inst);

    if (dac == NULL)
    {
        return hwDAC_InvalidParameter;
    }

    DL_DAC12_output12(dac, 0U);
    DL_DAC12_disableOutputPin(dac);
    DL_DAC12_disable(dac);
    DL_DAC12_reset(dac);
    DL_DAC12_disablePower(dac);

    return hwDAC_OK;
}

hwDAC_OpResult DAC_ConfigChannel(
    hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (!DAC_IsValidChannel(ch) ||
        (DAC_Channel_Def_Table[ch].inst != inst))
    {
        return hwDAC_InvalidParameter;
    }

    /*
     * MSPM0 DAC12 has one OUT0 channel. Resolution, reference, amplifier,
     * FIFO and trigger settings are configured per DAC instance in
     * DAC_Instance_Init().
     */
    return hwDAC_OK;
}

hwDAC_OpResult DAC_StartChannel(
    hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (!DAC_IsValidChannel(ch) ||
        (DAC_Channel_Def_Table[ch].inst != inst) ||
        !DAC_Instance_Init_Status[inst])
    {
        return hwDAC_InvalidParameter;
    }

    DAC12_Regs *dac = DAC_Map_Soc_Base(inst);

    if (dac == NULL)
    {
        return hwDAC_InvalidParameter;
    }

    DL_DAC12_enable(dac);
    DL_DAC12_enableOutputPin(dac);

    return hwDAC_OK;
}

hwDAC_OpResult DAC_StopChannel(
    hwDAC_Instance inst, hwDAC_Channel_Index ch)
{
    if (!DAC_IsValidChannel(ch) ||
        (DAC_Channel_Def_Table[ch].inst != inst))
    {
        return hwDAC_InvalidParameter;
    }

    DAC12_Regs *dac = DAC_Map_Soc_Base(inst);

    if (dac == NULL)
    {
        return hwDAC_InvalidParameter;
    }

    DL_DAC12_output12(dac, 0U);
    DL_DAC12_disableOutputPin(dac);
    DL_DAC12_disable(dac);

    return hwDAC_OK;
}

hwDAC_OpResult DAC_WriteRaw(
    hwDAC_Instance inst,
    hwDAC_Channel_Index ch,
    uint32_t raw)
{
    if (!DAC_IsValidChannel(ch) ||
        (DAC_Channel_Def_Table[ch].inst != inst) ||
        (raw > DAC_MSPM0_MAX_COUNT))
    {
        return hwDAC_InvalidParameter;
    }

    if (!DAC_Channel_Init_Status[ch] ||
        !DAC_Instance_Init_Status[inst])
    {
        return hwDAC_NotInit;
    }

    DAC12_Regs *dac = DAC_Map_Soc_Base(inst);

    if (dac == NULL)
    {
        return hwDAC_InvalidParameter;
    }

    DL_DAC12_output12(dac, raw);

    return hwDAC_OK;
}

static bool DAC_IsInstanceUsed(hwDAC_Instance inst)
{
    if (inst >= hwDAC_Instance_MAX)
    {
        return false;
    }

    for (size_t i = 0U; i < (size_t) hwDAC_Channel_Index_MAX; i++)
    {
        if (DAC_Channel_Init_Status[i] &&
            (DAC_Channel_Def_Table[i].inst == inst))
        {
            return true;
        }
    }

    return false;
}

hwDAC_OpResult DAC_Channel_Init(hwDAC_Channel_Index ch)
{
    if (!DAC_IsValidChannel(ch))
    {
        return hwDAC_InvalidParameter;
    }

    if (DAC_Channel_Init_Status[ch])
    {
        return hwDAC_OK;
    }

    const DAC_Channel_Def *def = &DAC_Channel_Def_Table[ch];
    hwDAC_Instance inst = def->inst;
    uint32_t pin_cm = GPIO_Map_Soc_Pin_IOMUX(def->dac_pin);
    bool instance_started = false;

    if (pin_cm == 0U)
    {
        return hwDAC_InvalidParameter;
    }

    /*
     * DAC0_OUT is a non-IOMUX analog function on PA15. Explicitly restore
     * analog mode in case this pin was previously configured as GPIO.
     */
    DL_GPIO_initPeripheralAnalogFunction(pin_cm);

    if (!DAC_Instance_Init_Status[inst])
    {
        hwDAC_OpResult status = DAC_Instance_Init(inst);

        if (status != hwDAC_OK)
        {
            DL_GPIO_initDigitalInput(pin_cm);
            return status;
        }

        DAC_Instance_Init_Status[inst] = true;
        instance_started = true;
    }

    hwDAC_OpResult status = DAC_ConfigChannel(inst, ch);

    if (status == hwDAC_OK)
    {
        status = DAC_StartChannel(inst, ch);
    }

    if (status != hwDAC_OK)
    {
        if (instance_started)
        {
            (void) DAC_Instance_DeInit(inst);
            DAC_Instance_Init_Status[inst] = false;
        }

        DL_GPIO_initDigitalInput(pin_cm);
        return status;
    }

    gpio_pin_init_status[def->dac_pin] = true;
    DAC_Channel_Init_Status[ch] = true;

    return hwDAC_OK;
}

hwDAC_OpResult DAC_Channel_DeInit(hwDAC_Channel_Index ch)
{
    if (!DAC_IsValidChannel(ch))
    {
        return hwDAC_InvalidParameter;
    }

    if (!DAC_Channel_Init_Status[ch])
    {
        return hwDAC_OK;
    }

    const DAC_Channel_Def *def = &DAC_Channel_Def_Table[ch];
    hwDAC_Instance inst = def->inst;
    uint32_t pin_cm = GPIO_Map_Soc_Pin_IOMUX(def->dac_pin);

    if (pin_cm == 0U)
    {
        return hwDAC_InvalidParameter;
    }

    hwDAC_OpResult status = DAC_StopChannel(inst, ch);

    if (status != hwDAC_OK)
    {
        return status;
    }

    DAC_Channel_Init_Status[ch] = false;

    if (!DAC_IsInstanceUsed(inst) &&
        DAC_Instance_Init_Status[inst])
    {
        status = DAC_Instance_DeInit(inst);

        if (status != hwDAC_OK)
        {
            return status;
        }

        DAC_Instance_Init_Status[inst] = false;
    }

    DL_GPIO_initDigitalInput(pin_cm);
    gpio_pin_init_status[def->dac_pin] = false;

    return hwDAC_OK;
}

hwDAC_OpResult DAC_Write_MiniVolt(
    hwDAC_Channel_Index ch, float mv)
{
    if (!DAC_IsValidChannel(ch))
    {
        return hwDAC_InvalidParameter;
    }

    if (!DAC_Channel_Init_Status[ch])
    {
        return hwDAC_NotInit;
    }

    if (mv < 0.0f)
    {
        mv = 0.0f;
    }
    else if (mv > DAC_MSPM0_VREF_MV)
    {
        mv = DAC_MSPM0_VREF_MV;
    }

    uint32_t raw = (uint32_t)
        (((mv * (float) DAC_MSPM0_MAX_COUNT) /
          DAC_MSPM0_VREF_MV) + 0.5f);

    return DAC_WriteRaw(
        DAC_Channel_Def_Table[ch].inst, ch, raw);
}

#endif // DAC0_BASE

#endif // DEVICE_TIMSPM0