#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#ifdef STM32G0

#include "SPI/SPI_Master.h"
#include "SPI_Master_STM32.h"
#include "GPIO/Device/GPIO_STM32.h"

SPI_HandleTypeDef g_spi[hwSPI_Index_MAX];

static const uint32_t Spi_Master_Baudrate_Prescaler_Table[] = {
    SPI_BAUDRATEPRESCALER_2,
    SPI_BAUDRATEPRESCALER_4,
    SPI_BAUDRATEPRESCALER_8,
    SPI_BAUDRATEPRESCALER_16,
    SPI_BAUDRATEPRESCALER_32,
    SPI_BAUDRATEPRESCALER_64,
    SPI_BAUDRATEPRESCALER_128,
    SPI_BAUDRATEPRESCALER_256
};

SPI_TypeDef *SPI_Map_Soc_Base(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0: return SPI1;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_1: return SPI2;
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_2: return SPI3;
#endif
        default: return NULL;
    }
}

static void SPI_EnableClock(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
            __HAL_RCC_SPI1_CLK_ENABLE();
            break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_1:
            __HAL_RCC_SPI2_CLK_ENABLE();
            break;
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_2:
            __HAL_RCC_SPI3_CLK_ENABLE();
            break;
#endif

        default:
            break;
    }
}

static void SPI_DisableClock(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
            __HAL_RCC_SPI1_CLK_DISABLE();
            break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_1:
            __HAL_RCC_SPI2_CLK_DISABLE();
            break;
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_2:
            __HAL_RCC_SPI3_CLK_DISABLE();
            break;
#endif

        default:
            break;
    }
}

int SPI_Master_Get_Clock_Freq(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
            return HAL_RCC_GetPCLK2Freq();
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_1:
            return HAL_RCC_GetPCLK1Freq();
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_2:
            return HAL_RCC_GetPCLK1Freq();
#endif
        default:
            return 0;
    }
}

hwSPI_OpResult SPI_Instance_Init(
    hwSPI_Index index,
    uint32_t clock_rate_hz,
    hwSPI_OpMode opMode
)
{
    SPI_TypeDef *spi = SPI_Map_Soc_Base(index);
    if (!spi) return hwSPI_InvalidParameter;

    SPI_EnableClock(index);

    g_spi[index].Instance = spi;

    g_spi[index].Init.Mode       = SPI_MODE_MASTER;
    g_spi[index].Init.Direction  = SPI_DIRECTION_2LINES;
    g_spi[index].Init.DataSize   = SPI_DATASIZE_8BIT;
    g_spi[index].Init.NSS        = SPI_NSS_SOFT;
    g_spi[index].Init.FirstBit   = SPI_FIRSTBIT_MSB;

    switch (opMode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            g_spi[index].Init.CLKPolarity = SPI_POLARITY_LOW;
            g_spi[index].Init.CLKPhase    = SPI_PHASE_1EDGE;
            break;
        case hwSPI_OpMode_Polarity0_Phase1:
            g_spi[index].Init.CLKPhase    = SPI_PHASE_2EDGE;
            break;
        case hwSPI_OpMode_Polarity1_Phase0:
            g_spi[index].Init.CLKPolarity = SPI_POLARITY_HIGH;
            g_spi[index].Init.CLKPhase    = SPI_PHASE_1EDGE;
            break;
        case hwSPI_OpMode_Polarity1_Phase1:
            g_spi[index].Init.CLKPolarity = SPI_POLARITY_HIGH;
            g_spi[index].Init.CLKPhase    = SPI_PHASE_2EDGE;
            break;
        default:
            return hwSPI_InvalidParameter;
    }

    int spi_hz = SPI_Master_Get_Clock_Freq(index) / 2;
    uint8_t prescaler_rank = 0;
    uint8_t last_index = sizeof(Spi_Master_Baudrate_Prescaler_Table) / sizeof(Spi_Master_Baudrate_Prescaler_Table[0]) - 1;

    while ((spi_hz > (int)clock_rate_hz) && (prescaler_rank < last_index))
    {
        spi_hz /= 2;
        prescaler_rank++;
    }

    g_spi[index].Init.BaudRatePrescaler = Spi_Master_Baudrate_Prescaler_Table[prescaler_rank];

    if (HAL_SPI_Init(&g_spi[index]) != HAL_OK)
        return hwSPI_HwError;

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Instance_DeInit(hwSPI_Index index)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (HAL_SPI_DeInit(&g_spi[index]) != HAL_OK)
    {
        return hwSPI_HwError;
    }

    SPI_DisableClock(index);

    return hwSPI_OK;
}

static void SPI_HAL_IRQHandler(hwSPI_Index index)
{
    HAL_SPI_IRQHandler(&g_spi[index]);
}

void SPI1_IRQHandler(void)
{
    SPI_HAL_IRQHandler(hwSPI_Index_0);
}

void SPI2_3_IRQHandler(void)
{
#if defined(SPI2_BASE)
    SPI_HAL_IRQHandler(hwSPI_Index_1);
#endif
#if defined(SPI3_BASE)
    SPI_HAL_IRQHandler(hwSPI_Index_2);
#endif
}

void SPI_NVIC_Init(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
            HAL_NVIC_SetPriority(SPI1_IRQn, SPI_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(SPI1_IRQn);
            break;
#endif

#if defined (STM32G0B0xx) || defined (STM32G0B1xx) || defined (STM32G0C1xx)
#if defined(SPI2_BASE) || defined(SPI3_BASE)
        case hwSPI_Index_1:
        case hwSPI_Index_2:
            HAL_NVIC_SetPriority(SPI2_3_IRQn, SPI_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(SPI2_3_IRQn);
            break;
#endif
#else
#if defined(SPI2_BASE)
        case hwSPI_Index_1:
            HAL_NVIC_SetPriority(SPI2_IRQn, SPI_IRQ_NVIC_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(SPI2_IRQn);
            break;
#endif
#endif
    }
}

void SPI_NVIC_DeInit(hwSPI_Index index)
{
#if defined (STM32G0B0xx) || defined (STM32G0B1xx) || defined (STM32G0C1xx)

    if (index == hwSPI_Index_1 || index == hwSPI_Index_2)
    {
        if (!Spi_Master_Init_Status[hwSPI_Index_1] &&
            !Spi_Master_Init_Status[hwSPI_Index_2])
        {
            HAL_NVIC_DisableIRQ(SPI2_3_IRQn);
        }
        return;
    }

#endif

    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
            HAL_NVIC_DisableIRQ(SPI1_IRQn);
            break;
#endif
#if defined (STM32G0B0xx) || defined (STM32G0B1xx) || defined (STM32G0C1xx)
#if defined(SPI2_BASE) || defined(SPI3_BASE)
        case hwSPI_Index_1:
        case hwSPI_Index_2:
            HAL_NVIC_DisableIRQ(SPI2_3_IRQn);
            break;
#endif
#else
#if defined(SPI2_BASE)
        case hwSPI_Index_1:
            HAL_NVIC_DisableIRQ(SPI2_IRQn);
            break;
#endif
#endif
    }
}

#endif