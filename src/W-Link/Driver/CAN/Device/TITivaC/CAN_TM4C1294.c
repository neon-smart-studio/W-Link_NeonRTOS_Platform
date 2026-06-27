#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "CAN/CAN.h"

#if defined(TM4C1294)

#include "GPIO/Device/TITivaC/GPIO_TITivaC.h"

#include "CAN/Pin/TITivaC/CAN_Pin_TITivaC.h"

#include "CAN_TITivaC.h"

uint32_t CAN_Map_Soc_Base(hwCAN_Index index)
{
    switch (index)
    {
        case hwCAN_Index_0: return CAN0_BASE;
        case hwCAN_Index_1: return CAN1_BASE;
        default: break;
    }

    return 0;
}

uint32_t CAN_Map_Soc_Periph(hwCAN_Index index)
{
    switch (index)
    {
        case hwCAN_Index_0: return SYSCTL_PERIPH_CAN0;
        case hwCAN_Index_1: return SYSCTL_PERIPH_CAN1;
        default: break;
    }

    return 0;
}

uint32_t CAN_Map_Soc_Int(hwCAN_Index index)
{
    switch (index)
    {
        case hwCAN_Index_0: return INT_CAN0;
        case hwCAN_Index_1: return INT_CAN1;
        default: break;
    }

    return 0;
}

uint32_t CAN_Map_PinConfig(hwCAN_Index can, hwGPIO_Pin pin)
{
    switch (can)
    {
        case hwCAN_Index_0:
            if (pin == hwGPIO_Pin_A1) return GPIO_PA1_CAN0TX;
            if (pin == hwGPIO_Pin_A0) return GPIO_PA0_CAN0RX;
            break;

        case hwCAN_Index_1:
            if (pin == hwGPIO_Pin_B1) return GPIO_PB1_CAN1TX;
            if (pin == hwGPIO_Pin_B0) return GPIO_PB0_CAN1RX;
            break;

        default:
            break;
    }

    return 0;
}

void CAN0_IRQHandler(void)
{
    CAN_IRQ_Process(hwCAN_Index_0);
}

void CAN1_IRQHandler(void)
{
    CAN_IRQ_Process(hwCAN_Index_1);
}

void CAN_NVIC_Init(hwCAN_Index index)
{
    uint32_t irq = CAN_Map_Soc_Int(index);
    if(irq == 0)
    {
        return;
    }

    switch (index)
    {
        case hwCAN_Index_0:
            IntRegister(irq, CAN0_IRQHandler);
            break;

        case hwCAN_Index_1:
            IntRegister(irq, CAN1_IRQHandler);
            break;

        default:
            break;
    }

    MAP_IntEnable(irq);
}

void CAN_NVIC_DeInit(hwCAN_Index index)
{
    uint32_t irq = CAN_Map_Soc_Int(index);
    if(irq == 0)
    {
        return;
    }

    MAP_IntDisable(irq);
}

#endif