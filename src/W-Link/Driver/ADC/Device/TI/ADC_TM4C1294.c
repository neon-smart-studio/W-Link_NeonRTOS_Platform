#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "ADC/ADC.h"

#ifdef TM4C1294

#include "ADC_TI.h"

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"

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

void ADC0SS3_Handler(void)
{
    uint32_t raw;

    ADCIntClear(ADC0_BASE, 3);
    ADCSequenceDataGet(ADC0_BASE, 3, &raw);

    ADC_ConvCpltCallback((uint16_t)raw);
}

void ADC1SS3_Handler(void)
{
    uint32_t raw;

    ADCIntClear(ADC1_BASE, 3);
    ADCSequenceDataGet(ADC1_BASE, 3, &raw);

    ADC_ConvCpltCallback((uint16_t)raw);
}

void ADC_GPIO_ConfigAF(hwADC_Channel_Index ch)
{
    switch(ch)
    {
        case hwADC_Channel_Index_0:   /* AIN0  PE3 */
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
            while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
            GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
            break;

        case hwADC_Channel_Index_1:   /* AIN1  PE2 */
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
            while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
            GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
            break;

        case hwADC_Channel_Index_2:   /* AIN2  PE1 */
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
            while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
            GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);
            break;

        case hwADC_Channel_Index_3:   /* AIN3  PE0 */
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
            while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
            GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
            break;

        case hwADC_Channel_Index_4:   /* AIN4  PD7 */
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
            while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
            GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
            break;

        case hwADC_Channel_Index_5:   /* AIN5  PD6 */
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
            while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
            GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_6);
            break;

        case hwADC_Channel_Index_6:   /* AIN6  PD5 */
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
            while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
            GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_5);
            break;

        case hwADC_Channel_Index_7:   /* AIN7  PD4 */
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
            while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
            GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_4);
            break;

        default:
            break;
    }
}

void ADC_GPIO_DeConfigAF(hwADC_Channel_Index ch)
{
    /* TM4C GPIOPinTypeADC 會把腳位設成 analog input。
       DeInit 時若你要恢復 GPIO，這裡可改回 GPIOPinTypeGPIOInput。 */

    switch(ch)
    {
        case hwADC_Channel_Index_0:
            GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3);
            break;

        case hwADC_Channel_Index_1:
            GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_2);
            break;

        case hwADC_Channel_Index_2:
            GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1);
            break;

        case hwADC_Channel_Index_3:
            GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0);
            break;

        case hwADC_Channel_Index_4:
            GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_7);
            break;

        case hwADC_Channel_Index_5:
            GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_6);
            break;

        case hwADC_Channel_Index_6:
            GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_5);
            break;

        case hwADC_Channel_Index_7:
            GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_4);
            break;

        default:
            break;
    }
}

hwADC_OpResult ADC_Instance_Init(hwADC_Instance inst)
{
    uint32_t periph = ADC_Instance_To_Periph(inst);
    uint32_t base   = ADC_Instance_To_Base(inst);

    if(base == 0 || periph == 0)
        return hwADC_InvalidParameter;

    SysCtlPeripheralEnable(periph);
    while(!SysCtlPeripheralReady(periph));

    ADCSequenceDisable(base, 3);

    ADCSequenceConfigure(base,
                         3,
                         ADC_TRIGGER_PROCESSOR,
                         0);

    ADCIntClear(base, 3);
    ADCIntEnable(base, 3);

    ADCSequenceEnable(base, 3);

    return hwADC_OK;
}

hwADC_OpResult ADC_Instance_DeInit(hwADC_Instance inst)
{
    uint32_t periph = ADC_Instance_To_Periph(inst);
    uint32_t base   = ADC_Instance_To_Base(inst);

    if(base == 0 || periph == 0)
        return hwADC_InvalidParameter;

    ADCIntDisable(base, 3);
    ADCSequenceDisable(base, 3);

    SysCtlPeripheralDisable(periph);

    return hwADC_OK;
}

hwADC_OpResult ADC_ConfigChannel(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    uint32_t base = ADC_Instance_To_Base(inst);
    uint32_t ctl_ch = ADC_Channel_Index_To_Ctl(ch);

    if(base == 0 || ctl_ch == 0)
        return hwADC_InvalidParameter;

    ADCSequenceDisable(base, 3);

    ADCSequenceStepConfigure(base,
                             3,
                             0,
                             ctl_ch | ADC_CTL_IE | ADC_CTL_END);

    ADCIntClear(base, 3);
    ADCSequenceEnable(base, 3);

    return hwADC_OK;
}

hwADC_OpResult ADC_ChannelStartConversion(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    uint32_t base = ADC_Instance_To_Base(inst);

    if(base == 0)
        return hwADC_InvalidParameter;

    ADCProcessorTrigger(base, 3);

    return hwADC_OK;
}

hwADC_OpResult ADC_ChannelStopConversion(hwADC_Instance inst, hwADC_Channel_Index ch)
{
    uint32_t base = ADC_Instance_To_Base(inst);

    if(base == 0)
        return hwADC_InvalidParameter;

    ADCSequenceDisable(base, 3);
    ADCSequenceEnable(base, 3);

    return hwADC_OK;
}

void ADC_NVIC_Init(void)
{
    IntEnable(INT_ADC0SS3);
    IntEnable(INT_ADC1SS3);
}

void ADC_NVIC_DeInit(void)
{
    IntDisable(INT_ADC0SS3);
    IntDisable(INT_ADC1SS3);
}

#endif /* DEVICE_TM4C1294 */