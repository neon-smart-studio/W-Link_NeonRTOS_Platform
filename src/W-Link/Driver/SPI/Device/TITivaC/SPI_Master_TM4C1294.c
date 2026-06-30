#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#include "SPI/SPI_Master.h"

#if defined(TM4C1294)

#include "SPI_Master_TITivaC.h"

uint32_t SPI_Map_Soc_Base(hwSPI_Index index)
{
    switch (index)
    {
        case hwSPI_Index_0: return SSI0_BASE;
        case hwSPI_Index_1: return SSI1_BASE;
        case hwSPI_Index_2: return SSI2_BASE;
        case hwSPI_Index_3: return SSI3_BASE;
        default: return 0;
    }
}

uint32_t SPI_Map_Soc_Periph(hwSPI_Index index)
{
    switch (index)
    {
        case hwSPI_Index_0: return SYSCTL_PERIPH_SSI0;
        case hwSPI_Index_1: return SYSCTL_PERIPH_SSI1;
        case hwSPI_Index_2: return SYSCTL_PERIPH_SSI2;
        case hwSPI_Index_3: return SYSCTL_PERIPH_SSI3;
        default: return 0;
    }
}

uint32_t SPI_Map_IRQ(hwSPI_Index index)
{
    switch (index)
    {
        case hwSPI_Index_0: return INT_SSI0;
        case hwSPI_Index_1: return INT_SSI1;
        case hwSPI_Index_2: return INT_SSI2;
        case hwSPI_Index_3: return INT_SSI3;
        default: return 0;
    }
}

uint32_t SPI_Map_PinConfig(hwSPI_Index index, hwGPIO_Pin pin)
{
    switch (index) {
        case hwSPI_Index_0:
            if (pin == hwGPIO_Pin_A2) return GPIO_PA2_SSI0CLK;
            if (pin == hwGPIO_Pin_A4) return GPIO_PA4_SSI0XDAT0;
            if (pin == hwGPIO_Pin_A5) return GPIO_PA5_SSI0XDAT1;
            if (pin == hwGPIO_Pin_A3) return GPIO_PA3_SSI0FSS;
            break;
        case hwSPI_Index_1:
            if (pin == hwGPIO_Pin_B5) return GPIO_PB5_SSI1CLK;
            if (pin == hwGPIO_Pin_E4) return GPIO_PE4_SSI1XDAT0;
            if (pin == hwGPIO_Pin_E5) return GPIO_PE5_SSI1XDAT1;
            if (pin == hwGPIO_Pin_B4) return GPIO_PB4_SSI1FSS;
            break;
        case hwSPI_Index_2:
            if (pin == hwGPIO_Pin_D3) return GPIO_PD3_SSI2CLK;
            if (pin == hwGPIO_Pin_D1) return GPIO_PD1_SSI2XDAT0;
            if (pin == hwGPIO_Pin_D0) return GPIO_PD0_SSI2XDAT1;
            if (pin == hwGPIO_Pin_D2) return GPIO_PD2_SSI2FSS;
            break;
        case hwSPI_Index_3:
            if (pin == hwGPIO_Pin_Q0) return GPIO_PQ0_SSI3CLK;
            if (pin == hwGPIO_Pin_Q2) return GPIO_PQ2_SSI3XDAT0;
            if (pin == hwGPIO_Pin_Q3) return GPIO_PQ3_SSI3XDAT1;
            if (pin == hwGPIO_Pin_Q1) return GPIO_PQ1_SSI3FSS;
            break;

        default:
            break;
    }

    return 0;
}

static void SPI0_IRQ_Handler(void)
{
    SPI_IRQ_Process(hwSPI_Index_0);
}

static void SPI1_IRQ_Handler(void)
{
    SPI_IRQ_Process(hwSPI_Index_1);
}

static void SPI2_IRQ_Handler(void)
{
    SPI_IRQ_Process(hwSPI_Index_2);
}

static void SPI3_IRQ_Handler(void)
{
    SPI_IRQ_Process(hwSPI_Index_3);
}

void SPI_NVIC_Enable(hwSPI_Index index)
{
    uint32_t base = SPI_Map_Soc_Base(index);
    uint32_t irq = SPI_Map_IRQ(index);

    if (base == 0)
    {
        return;
    }

    switch (index)
    {
        case hwSPI_Index_0:
            SSIIntRegister(base, SPI0_IRQ_Handler);
            break;

        case hwSPI_Index_1:
            SSIIntRegister(base, SPI1_IRQ_Handler);
            break;

        case hwSPI_Index_2:
            SSIIntRegister(base, SPI2_IRQ_Handler);
            break;

        case hwSPI_Index_3:
            SSIIntRegister(base, SPI3_IRQ_Handler);
            break;

        default:
            return;
    }

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    if (irq != 0)
    {
        MAP_IntEnable(irq);
    }
}

void SPI_NVIC_Disable(hwSPI_Index index)
{
    uint32_t base = SPI_Map_Soc_Base(index);
    uint32_t irq = SPI_Map_IRQ(index);

    if (base != 0)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        SSIIntUnregister(base);
    }

    if (irq != 0)
    {
        MAP_IntDisable(irq);
    }
}

#endif // TM4C1294