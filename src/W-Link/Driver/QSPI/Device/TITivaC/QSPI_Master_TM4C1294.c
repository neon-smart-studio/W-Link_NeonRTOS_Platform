#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#include "QSPI/QSPI_Master.h"

#if defined(TM4C1294)

#include "QSPI_Master_TITivaC.h"

uint32_t QSPI_Map_Soc_Base(hwQSPI_Index index)
{
    switch (index)
    {
        case hwQSPI_Index_0: return SSI0_BASE;
        case hwQSPI_Index_1: return SSI1_BASE;
        case hwQSPI_Index_2: return SSI2_BASE;
        case hwQSPI_Index_3: return SSI3_BASE;
        default: return 0;
    }
}

uint32_t QSPI_Map_Soc_Periph(hwQSPI_Index index)
{
    switch (index)
    {
        case hwQSPI_Index_0: return SYSCTL_PERIPH_SSI0;
        case hwQSPI_Index_1: return SYSCTL_PERIPH_SSI1;
        case hwQSPI_Index_2: return SYSCTL_PERIPH_SSI2;
        case hwQSPI_Index_3: return SYSCTL_PERIPH_SSI3;
        default: return 0;
    }
}

uint32_t QSPI_Map_IRQ(hwQSPI_Index index)
{
    switch (index)
    {
        case hwQSPI_Index_0: return INT_SSI0;
        case hwQSPI_Index_1: return INT_SSI1;
        case hwQSPI_Index_2: return INT_SSI2;
        case hwQSPI_Index_3: return INT_SSI3;
        default: return 0;
    }
}

uint32_t QSPI_Map_PinConfig(hwQSPI_Index index, hwGPIO_Pin pin)
{
    switch (index) {
        case hwQSPI_Index_0:
            if (pin == hwGPIO_Pin_A4) return GPIO_PA4_SSI0XDAT0;
            if (pin == hwGPIO_Pin_A5) return GPIO_PA5_SSI0XDAT1;
            if (pin == hwGPIO_Pin_A6) return GPIO_PA6_SSI0XDAT2;
            if (pin == hwGPIO_Pin_A7) return GPIO_PA7_SSI0XDAT3;
            if (pin == hwGPIO_Pin_A2) return GPIO_PA2_SSI0CLK;
            if (pin == hwGPIO_Pin_A3) return GPIO_PA3_SSI0FSS;
            break;
        case hwQSPI_Index_1:
            if (pin == hwGPIO_Pin_E4) return GPIO_PE4_SSI1XDAT0;
            if (pin == hwGPIO_Pin_E5) return GPIO_PE5_SSI1XDAT1;
            if (pin == hwGPIO_Pin_D4) return GPIO_PD4_SSI1XDAT2;
            if (pin == hwGPIO_Pin_D5) return GPIO_PD5_SSI1XDAT3;
            if (pin == hwGPIO_Pin_B5) return GPIO_PB5_SSI1CLK;
            if (pin == hwGPIO_Pin_B4) return GPIO_PB4_SSI1FSS;
            break;
        case hwQSPI_Index_2:
            if (pin == hwGPIO_Pin_D1) return GPIO_PD1_SSI2XDAT0;
            if (pin == hwGPIO_Pin_D0) return GPIO_PD0_SSI2XDAT1;
            if (pin == hwGPIO_Pin_D7) return GPIO_PD7_SSI2XDAT2;
            if (pin == hwGPIO_Pin_D6) return GPIO_PD6_SSI2XDAT3;
            if (pin == hwGPIO_Pin_D3) return GPIO_PD3_SSI2CLK;
            if (pin == hwGPIO_Pin_D2) return GPIO_PD2_SSI2FSS;
            break;
        case hwQSPI_Index_3:
            if (pin == hwGPIO_Pin_Q2) return GPIO_PQ2_SSI3XDAT0;
            if (pin == hwGPIO_Pin_Q3) return GPIO_PQ3_SSI3XDAT1;
            if (pin == hwGPIO_Pin_F4) return GPIO_PF4_SSI3XDAT2;
            if (pin == hwGPIO_Pin_P1) return GPIO_PP1_SSI3XDAT3;
            if (pin == hwGPIO_Pin_Q0) return GPIO_PQ0_SSI3CLK;
            if (pin == hwGPIO_Pin_Q1) return GPIO_PQ1_SSI3FSS;
            break;

        default:
            break;
    }

    return 0;
}

static void QSPI0_IRQ_Handler(void)
{
    QSPI_IRQ_Process(hwQSPI_Index_0);
}

static void QSPI1_IRQ_Handler(void)
{
    QSPI_IRQ_Process(hwQSPI_Index_1);
}

static void QSPI2_IRQ_Handler(void)
{
    QSPI_IRQ_Process(hwQSPI_Index_2);
}

static void QSPI3_IRQ_Handler(void)
{
    QSPI_IRQ_Process(hwQSPI_Index_3);
}

void QSPI_NVIC_Init(hwQSPI_Index index)
{
    uint32_t base = QSPI_Map_Soc_Base(index);
    uint32_t irq = QSPI_Map_IRQ(index);

    if (base == 0)
    {
        return;
    }

    switch (index)
    {
        case hwQSPI_Index_0:
            SSIIntRegister(base, QSPI0_IRQ_Handler);
            break;

        case hwQSPI_Index_1:
            SSIIntRegister(base, QSPI1_IRQ_Handler);
            break;

        case hwQSPI_Index_2:
            SSIIntRegister(base, QSPI2_IRQ_Handler);
            break;

        case hwQSPI_Index_3:
            SSIIntRegister(base, QSPI3_IRQ_Handler);
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

void QSPI_NVIC_DeInit(hwQSPI_Index index)
{
    uint32_t base = QSPI_Map_Soc_Base(index);
    uint32_t irq = QSPI_Map_IRQ(index);

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