#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#if defined(TM4C1294)

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
        case hwI2C_Index_4: return I2C4_BASE;
        case hwI2C_Index_5: return I2C5_BASE;
        case hwI2C_Index_6: return I2C6_BASE;
        case hwI2C_Index_7: return I2C7_BASE;
        case hwI2C_Index_8: return I2C8_BASE;
        case hwI2C_Index_9: return I2C9_BASE;
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
        case hwI2C_Index_4: return SYSCTL_PERIPH_I2C4;
        case hwI2C_Index_5: return SYSCTL_PERIPH_I2C5;
        case hwI2C_Index_6: return SYSCTL_PERIPH_I2C6;
        case hwI2C_Index_7: return SYSCTL_PERIPH_I2C7;
        case hwI2C_Index_8: return SYSCTL_PERIPH_I2C8;
        case hwI2C_Index_9: return SYSCTL_PERIPH_I2C9;
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
        case hwI2C_Index_4: return INT_I2C4;
        case hwI2C_Index_5: return INT_I2C5;
        case hwI2C_Index_6: return INT_I2C6;
        case hwI2C_Index_7: return INT_I2C7;
        case hwI2C_Index_8: return INT_I2C8;
        case hwI2C_Index_9: return INT_I2C9;
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
            if (pin == hwGPIO_Pin_G0) return GPIO_PG0_I2C1SCL;
            if (pin == hwGPIO_Pin_G1) return GPIO_PG1_I2C1SDA;
            break;
        case hwI2C_Index_2:
            if (pin == hwGPIO_Pin_N5) return GPIO_PN5_I2C2SCL;
            if (pin == hwGPIO_Pin_N4) return GPIO_PN4_I2C2SDA;
            break;
        case hwI2C_Index_3:
            if (pin == hwGPIO_Pin_K4) return GPIO_PK4_I2C3SCL;
            if (pin == hwGPIO_Pin_K5) return GPIO_PK5_I2C3SDA;
            break;
        case hwI2C_Index_4:
            if (pin == hwGPIO_Pin_K6) return GPIO_PK6_I2C4SCL;
            if (pin == hwGPIO_Pin_K7) return GPIO_PK7_I2C4SDA;
            break;
        case hwI2C_Index_5:
            if (pin == hwGPIO_Pin_B0) return GPIO_PB0_I2C5SCL;
            if (pin == hwGPIO_Pin_B1) return GPIO_PB1_I2C5SDA;
            break;
        case hwI2C_Index_6:
            if (pin == hwGPIO_Pin_A6) return GPIO_PA6_I2C6SCL;
            if (pin == hwGPIO_Pin_A7) return GPIO_PA7_I2C6SDA;
            break;
        case hwI2C_Index_7:
            if (pin == hwGPIO_Pin_D0) return GPIO_PD0_I2C7SCL;
            if (pin == hwGPIO_Pin_D1) return GPIO_PD1_I2C7SDA;
            break;
        case hwI2C_Index_8:
            if (pin == hwGPIO_Pin_A2) return GPIO_PA2_I2C8SCL;
            if (pin == hwGPIO_Pin_A3) return GPIO_PA3_I2C8SDA;
            break;
        case hwI2C_Index_9:
            if (pin == hwGPIO_Pin_A0) return GPIO_PA0_I2C9SCL;
            if (pin == hwGPIO_Pin_A1) return GPIO_PA1_I2C9SDA;
            break;

        default:
            break;
    }

    return 0;
}

static void I2C0_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_0); }
static void I2C1_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_1); }
static void I2C2_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_2); }
static void I2C3_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_3); }
static void I2C4_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_4); }
static void I2C5_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_5); }
static void I2C6_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_6); }
static void I2C7_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_7); }
static void I2C8_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_8); }
static void I2C9_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_9); }

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
        case hwI2C_Index_4: IntRegister(irq, I2C4_IRQ_Handler); break;
        case hwI2C_Index_5: IntRegister(irq, I2C5_IRQ_Handler); break;
        case hwI2C_Index_6: IntRegister(irq, I2C6_IRQ_Handler); break;
        case hwI2C_Index_7: IntRegister(irq, I2C7_IRQ_Handler); break;
        case hwI2C_Index_8: IntRegister(irq, I2C8_IRQ_Handler); break;
        case hwI2C_Index_9: IntRegister(irq, I2C9_IRQ_Handler); break;
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

#endif //TM4C1294