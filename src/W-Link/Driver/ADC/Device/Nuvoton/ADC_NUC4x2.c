#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "ADC/ADC.h"

#if defined(NUC442) || defined(NUC472)

#include "ADC_Nuvoton.h"

static uint32_t ADC_Channel_Index_To_Mask(hwADC_Channel_Index ch)
{
    switch(ch)
    {
        case hwADC_Channel_Index_0: return ADC_CH_0_MASK;
        case hwADC_Channel_Index_1: return ADC_CH_1_MASK;
        case hwADC_Channel_Index_2: return ADC_CH_2_MASK;
        case hwADC_Channel_Index_3: return ADC_CH_3_MASK;
        case hwADC_Channel_Index_4: return ADC_CH_4_MASK;
        case hwADC_Channel_Index_5: return ADC_CH_5_MASK;
        case hwADC_Channel_Index_6: return ADC_CH_6_MASK;
        case hwADC_Channel_Index_7: return ADC_CH_7_MASK;
        default: return 0;
    }
}

void ADC_IRQHandler(void)
{
    if(ADC_GET_INT_FLAG(ADC, ADC_ADF_INT))
    {
        ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

        uint16_t raw = ADC_GET_CONVERSION_DATA(ADC, 0);

        ADC_ConvCpltCallback(raw);
    }
}

void ADC_GPIO_ConfigAF(hwADC_Channel_Index ch)
{
    switch(ch)
    {
        case hwADC_Channel_Index_0:

            SYS->GPB_MFPL =
                (SYS->GPB_MFPL &
                ~SYS_GPB_MFPL_PB0MFP_Msk) |
                (0x1UL << SYS_GPB_MFPL_PB0MFP_Pos);

            GPIO_DISABLE_DIGITAL_PATH(PB, BIT0);

            break;

        case hwADC_Channel_Index_1:

            SYS->GPB_MFPL =
                (SYS->GPB_MFPL &
                ~SYS_GPB_MFPL_PB1MFP_Msk) |
                (0x1UL << SYS_GPB_MFPL_PB1MFP_Pos);

            GPIO_DISABLE_DIGITAL_PATH(PB, BIT1);

            break;

        case hwADC_Channel_Index_2:

            SYS->GPB_MFPL =
                (SYS->GPB_MFPL &
                ~SYS_GPB_MFPL_PB2MFP_Msk) |
                (0x1UL << SYS_GPB_MFPL_PB2MFP_Pos);

            GPIO_DISABLE_DIGITAL_PATH(PB, BIT2);

            break;

        case hwADC_Channel_Index_3:

            SYS->GPB_MFPL =
                (SYS->GPB_MFPL &
                ~SYS_GPB_MFPL_PB3MFP_Msk) |
                (0x1UL << SYS_GPB_MFPL_PB3MFP_Pos);

            GPIO_DISABLE_DIGITAL_PATH(PB, BIT3);

            break;

        case hwADC_Channel_Index_4:

            SYS->GPB_MFPL =
                (SYS->GPB_MFPL &
                ~SYS_GPB_MFPL_PB4MFP_Msk) |
                (0x1UL << SYS_GPB_MFPL_PB4MFP_Pos);

            GPIO_DISABLE_DIGITAL_PATH(PB, BIT4);

            break;

        case hwADC_Channel_Index_5:

            SYS->GPB_MFPL =
                (SYS->GPB_MFPL &
                ~SYS_GPB_MFPL_PB5MFP_Msk) |
                (0x1UL << SYS_GPB_MFPL_PB5MFP_Pos);

            GPIO_DISABLE_DIGITAL_PATH(PB, BIT5);

            break;

        case hwADC_Channel_Index_6:

            SYS->GPB_MFPL =
                (SYS->GPB_MFPL &
                ~SYS_GPB_MFPL_PB6MFP_Msk) |
                (0x1UL << SYS_GPB_MFPL_PB6MFP_Pos);

            GPIO_DISABLE_DIGITAL_PATH(PB, BIT6);

            break;

        case hwADC_Channel_Index_7:

            SYS->GPB_MFPL =
                (SYS->GPB_MFPL &
                ~SYS_GPB_MFPL_PB7MFP_Msk) |
                (0x1UL << SYS_GPB_MFPL_PB7MFP_Pos);

            GPIO_DISABLE_DIGITAL_PATH(PB, BIT7);

            break;
    }
}

void ADC_GPIO_DeConfigAF(hwADC_Channel_Index ch)
{
    switch(ch)
    {
        case hwADC_Channel_Index_0:
            SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB0MFP_Msk;
            GPIO_ENABLE_DIGITAL_PATH(PB, BIT0);
            break;

        case hwADC_Channel_Index_1:
            SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB1MFP_Msk;
            GPIO_ENABLE_DIGITAL_PATH(PB, BIT1);
            break;

        case hwADC_Channel_Index_2:
            SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB2MFP_Msk;
            GPIO_ENABLE_DIGITAL_PATH(PB, BIT2);
            break;

        case hwADC_Channel_Index_3:
            SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB3MFP_Msk;
            GPIO_ENABLE_DIGITAL_PATH(PB, BIT3);
            break;

        case hwADC_Channel_Index_4:
            SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB4MFP_Msk;
            GPIO_ENABLE_DIGITAL_PATH(PB, BIT4);
            break;

        case hwADC_Channel_Index_5:
            SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB5MFP_Msk;
            GPIO_ENABLE_DIGITAL_PATH(PB, BIT5);
            break;

        case hwADC_Channel_Index_6:
            SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB6MFP_Msk;
            GPIO_ENABLE_DIGITAL_PATH(PB, BIT6);
            break;

        case hwADC_Channel_Index_7:
            SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB7MFP_Msk;
            GPIO_ENABLE_DIGITAL_PATH(PB, BIT7);
            break;

        default:
            break;
    }
}

hwADC_OpResult ADC_Instance_Init(hwADC_Instance inst)
{
    if (inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    CLK_EnableModuleClock(ADC_MODULE);
    CLK_SetModuleClock(ADC_MODULE,
                    CLK_CLKSEL1_ADCSEL_PCLK,
                    CLK_CLKDIV0_ADC(8));

    SYS_ResetModule(ADC_RST);

    ADC_POWER_ON(ADC);

    ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

    ADC_EnableInt(ADC, ADC_ADF_INT);

    return hwADC_OK;
}

hwADC_OpResult ADC_Instance_DeInit(hwADC_Instance inst)
{
    if (inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    ADC_DisableInt(ADC, ADC_ADF_INT);
    ADC_POWER_DOWN(ADC);
    ADC_Close(ADC);

    CLK_DisableModuleClock(ADC_MODULE);

    return hwADC_OK;
}

hwADC_OpResult ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if (inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    if (ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    uint32_t channel_mask = ADC_Channel_Index_To_Mask(ch);

    if(channel_mask == 0)
    {
        return hwADC_InvalidParameter;
    }

    ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

    ADC_Open(ADC, ADC_INPUT_MODE_SINGLE_END, ADC_OPERATION_MODE_SINGLE, channel_mask);

    return hwADC_OK;
}

hwADC_OpResult ADC_ChannelStartConversion(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if (inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    if (ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    ADC_START_CONV(ADC);

    return hwADC_OK;
}

hwADC_OpResult ADC_ChannelStopConversion(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    if (inst >= hwADC_Instance_MAX)
    {
        return hwADC_InvalidParameter;
    }

    if (ch >= hwADC_Channel_Index_MAX)
    {
        return hwADC_InvalidParameter;
    }

    ADC_STOP_CONV(ADC);

    return hwADC_OK;
}

void ADC_NVIC_Init()
{
    NVIC_ClearPendingIRQ(ADC_IRQn);
    NVIC_EnableIRQ(ADC_IRQn);
}

void ADC_NVIC_DeInit()
{
    NVIC_DisableIRQ(ADC_IRQn);
    NVIC_ClearPendingIRQ(ADC_IRQn);
}

#endif // NUC442 || NUC472