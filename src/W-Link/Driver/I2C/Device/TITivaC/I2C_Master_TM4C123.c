#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#if defined(TM4C123)

#include "GPIO/Device/TITivaC/GPIO_TITivaC.h"

#include "I2C/Pin/TITivaC/I2C_Pin_TITivaC.h"

#include "I2C_Master_TITivaC.h"

uint32_t I2C_Map_Soc_Base(hwI2C_Index index)
{
    switch (index) {
        case hwI2C_Index_0: return I2C0_BASE;
        case hwI2C_Index_1: return I2C1_BASE;
        case hwI2C_Index_2: return I2C2_BASE;
        case hwI2C_Index_3: return I2C3_BASE;
        default: break;
    }

    return 0;
}

uint32_t I2C_Map_Soc_Periph(hwI2C_Index index)
{
    switch (index) {
        case hwI2C_Index_0: return SYSCTL_PERIPH_I2C0;
        case hwI2C_Index_1: return SYSCTL_PERIPH_I2C1;
        case hwI2C_Index_2: return SYSCTL_PERIPH_I2C2;
        case hwI2C_Index_3: return SYSCTL_PERIPH_I2C3;
        default: break;
    }

    return 0;
}

uint32_t I2C_Map_Soc_Int(hwI2C_Index index)
{
    switch (index) {
        case hwI2C_Index_0: return INT_I2C0;
        case hwI2C_Index_1: return INT_I2C1;
        case hwI2C_Index_2: return INT_I2C2;
        case hwI2C_Index_3: return INT_I2C3;
        default: break;
    }

    return 0;
}

uint32_t I2C_Map_PinConfig(hwI2C_Index index, hwGPIO_Pin pin)
{
    switch (index) {
        case hwI2C_Index_0:
            if (pin == hwGPIO_Pin_B2) return GPIO_PB2_I2C0SCL;
            if (pin == hwGPIO_Pin_B3) return GPIO_PB3_I2C0SDA;
            break;
        case hwI2C_Index_1:
            if (pin == hwGPIO_Pin_A6) return GPIO_PA6_I2C1SCL;
            if (pin == hwGPIO_Pin_A7) return GPIO_PA7_I2C1SDA;
            break;
        case hwI2C_Index_2:
            if (pin == hwGPIO_Pin_E4) return GPIO_PE4_I2C2SCL;
            if (pin == hwGPIO_Pin_E5) return GPIO_PE5_I2C2SDA;
            break;
        case hwI2C_Index_3:
            if (pin == hwGPIO_Pin_D0) return GPIO_PD0_I2C3SCL;
            if (pin == hwGPIO_Pin_D1) return GPIO_PD1_I2C3SDA;
            break;

        default:
            break;
    }

    return 0;
}

static void I2C0_IRQ_Handler(void) { TIVA_I2C_IRQ_Process(hwI2C_Index_0); }
static void I2C1_IRQ_Handler(void) { TIVA_I2C_IRQ_Process(hwI2C_Index_1); }
static void I2C2_IRQ_Handler(void) { TIVA_I2C_IRQ_Process(hwI2C_Index_2); }
static void I2C3_IRQ_Handler(void) { TIVA_I2C_IRQ_Process(hwI2C_Index_3); }

void I2C_NVIC_Init(hwI2C_Index index)
{
    uint32_t irq = I2C_Map_Soc_Int(index);
    if(irq==0)
    {
        return;
    }

    switch (index) {
        case hwI2C_Index_0: IntRegister(irq, I2C0_IRQ_Handler); break;
        case hwI2C_Index_1: IntRegister(irq, I2C1_IRQ_Handler); break;
        case hwI2C_Index_2: IntRegister(irq, I2C2_IRQ_Handler); break;
        case hwI2C_Index_3: IntRegister(irq, I2C3_IRQ_Handler); break;
        default: break;
    }

    MAP_IntEnable(irq);

    return hwI2C_OK;
}

void I2C_NVIC_DeInit(hwI2C_Index index)
{
    uint32_t irq = I2C_Map_Soc_Int(index);
    if(irq==0)
    {
        return;
    }

    MAP_IntDisable(irq);
}

#endif //TM4C123