#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#include "SPI/SPI_Master.h"

#ifdef STM32H7

#include "SPI_Master_STM32.h"

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
#if defined(SPI4_BASE)
        case hwSPI_Index_3: return SPI4;
#endif
#if defined(SPI5_BASE)
        case hwSPI_Index_4: return SPI5;
#endif
#if defined(SPI6_BASE)
        case hwSPI_Index_5: return SPI6;
#endif
        default: break;
    }

    return NULL;
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
#if defined(SPI4_BASE)
        case hwSPI_Index_3:
            __HAL_RCC_SPI4_CLK_ENABLE();
            break;
#endif
#if defined(SPI5_BASE)
        case hwSPI_Index_4:
            __HAL_RCC_SPI5_CLK_ENABLE();
            break;
#endif
#if defined(SPI6_BASE)
        case hwSPI_Index_5:
            __HAL_RCC_SPI6_CLK_ENABLE();
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
#if defined(SPI4_BASE)
        case hwSPI_Index_3:
            __HAL_RCC_SPI4_CLK_DISABLE();
            break;
#endif
#if defined(SPI5_BASE)
        case hwSPI_Index_4:
            __HAL_RCC_SPI5_CLK_DISABLE();
            break;
#endif
#if defined(SPI6_BASE)
        case hwSPI_Index_5:
            __HAL_RCC_SPI6_CLK_DISABLE();
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
        case hwSPI_Index_0:   /* SPI1 */
            return HAL_RCC_GetPCLK2Freq();
#endif

#if defined(SPI2_BASE)
        case hwSPI_Index_1:   /* SPI2 */
            return HAL_RCC_GetPCLK1Freq();
#endif

#if defined(SPI3_BASE)
        case hwSPI_Index_2:   /* SPI3 */
            return HAL_RCC_GetPCLK1Freq();
#endif

#if defined(SPI4_BASE)
        case hwSPI_Index_3:   /* SPI4 */
            return HAL_RCC_GetPCLK2Freq();
#endif

#if defined(SPI5_BASE)
        case hwSPI_Index_4:   /* SPI5 */
            return HAL_RCC_GetPCLK2Freq();
#endif

#if defined(SPI6_BASE)
        case hwSPI_Index_5:   /* SPI6 */
            return HAL_RCC_GetHCLKFreq();
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
    if (index >= hwSPI_Index_MAX)
        return hwSPI_InvalidParameter;

    SPI_TypeDef *spi_soc_base = SPI_Map_Soc_Base(index);

    if (spi_soc_base == NULL)
        return hwSPI_InvalidParameter;

    SPI_EnableClock(index);

    g_spi[index].Instance               = spi_soc_base;
    g_spi[index].Init.Mode              = SPI_MODE_MASTER;
    g_spi[index].Init.Direction         = SPI_DIRECTION_2LINES;
    g_spi[index].Init.DataSize          = SPI_DATASIZE_8BIT;
    g_spi[index].Init.NSS               = SPI_NSS_SOFT;
    g_spi[index].Init.FirstBit          = SPI_FIRSTBIT_MSB;
    g_spi[index].Init.TIMode            = SPI_TIMODE_DISABLE;
    g_spi[index].Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    g_spi[index].Init.CRCPolynomial     = 7;

#if defined(SPI_NSS_PULSE_DISABLE)
    g_spi[index].Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
#endif

    switch (opMode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            g_spi[index].Init.CLKPolarity = SPI_POLARITY_LOW;
            g_spi[index].Init.CLKPhase    = SPI_PHASE_1EDGE;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            g_spi[index].Init.CLKPolarity = SPI_POLARITY_LOW;
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

    int spi_input_hz = SPI_Master_Get_Clock_Freq(index);
    if (spi_input_hz <= 0 || clock_rate_hz == 0)
        return hwSPI_InvalidParameter;

    int spi_hz = spi_input_hz / 2;
    uint8_t prescaler_rank = 0;
    uint8_t last_index =
        sizeof(Spi_Master_Baudrate_Prescaler_Table) /
        sizeof(Spi_Master_Baudrate_Prescaler_Table[0]) - 1;

    while ((spi_hz > (int)clock_rate_hz) && (prescaler_rank < last_index))
    {
        spi_hz /= 2;
        prescaler_rank++;
    }

    g_spi[index].Init.BaudRatePrescaler =
        Spi_Master_Baudrate_Prescaler_Table[prescaler_rank];

    if (HAL_SPI_Init(&g_spi[index]) != HAL_OK)
        return hwSPI_HwError;

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Instance_DeInit(hwSPI_Index index)
{
    if (index >= hwSPI_Index_MAX)
        return hwSPI_InvalidParameter;

    if (SPI_Map_Soc_Base(index) == NULL)
        return hwSPI_InvalidParameter;

    if (HAL_SPI_DeInit(&g_spi[index]) != HAL_OK)
        return hwSPI_HwError;

    SPI_DisableClock(index);

    return hwSPI_OK;
}

static void SPI_HAL_IRQHandler(hwSPI_Index index)
{
    if (index < hwSPI_Index_MAX)
        HAL_SPI_IRQHandler(&g_spi[index]);
}

#if defined(SPI1_BASE)
void SPI1_IRQHandler(void)
{
    SPI_HAL_IRQHandler(hwSPI_Index_0);
}
#endif

#if defined(SPI2_BASE)
void SPI2_IRQHandler(void)
{
    SPI_HAL_IRQHandler(hwSPI_Index_1);
}
#endif

#if defined(SPI3_BASE)
void SPI3_IRQHandler(void)
{
    SPI_HAL_IRQHandler(hwSPI_Index_2);
}
#endif

#if defined(SPI4_BASE)
void SPI4_IRQHandler(void)
{
    SPI_HAL_IRQHandler(hwSPI_Index_3);
}
#endif

#if defined(SPI5_BASE)
void SPI5_IRQHandler(void)
{
    SPI_HAL_IRQHandler(hwSPI_Index_4);
}
#endif

#if defined(SPI6_BASE)
void SPI6_IRQHandler(void)
{
    SPI_HAL_IRQHandler(hwSPI_Index_5);
}
#endif

void SPI_NVIC_Init(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
            HAL_NVIC_SetPriority(SPI1_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(SPI1_IRQn);
            break;
#endif

#if defined(SPI2_BASE)
        case hwSPI_Index_1:
            HAL_NVIC_SetPriority(SPI2_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(SPI2_IRQn);
            break;
#endif

#if defined(SPI3_BASE)
        case hwSPI_Index_2:
            HAL_NVIC_SetPriority(SPI3_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(SPI3_IRQn);
            break;
#endif

#if defined(SPI4_BASE)
        case hwSPI_Index_3:
            HAL_NVIC_SetPriority(SPI4_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(SPI4_IRQn);
            break;
#endif

#if defined(SPI5_BASE)
        case hwSPI_Index_4:
            HAL_NVIC_SetPriority(SPI5_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(SPI5_IRQn);
            break;
#endif

#if defined(SPI6_BASE)
        case hwSPI_Index_5:
            HAL_NVIC_SetPriority(SPI6_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(SPI6_IRQn);
            break;
#endif

        default:
            break;
    }
}

void SPI_NVIC_DeInit(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
            HAL_NVIC_DisableIRQ(SPI1_IRQn);
            break;
#endif

#if defined(SPI2_BASE)
        case hwSPI_Index_1:
            HAL_NVIC_DisableIRQ(SPI2_IRQn);
            break;
#endif

#if defined(SPI3_BASE)
        case hwSPI_Index_2:
            HAL_NVIC_DisableIRQ(SPI3_IRQn);
            break;
#endif

#if defined(SPI4_BASE)
        case hwSPI_Index_3:
            HAL_NVIC_DisableIRQ(SPI4_IRQn);
            break;
#endif

#if defined(SPI5_BASE)
        case hwSPI_Index_4:
            HAL_NVIC_DisableIRQ(SPI5_IRQn);
            break;
#endif

#if defined(SPI6_BASE)
        case hwSPI_Index_5:
            HAL_NVIC_DisableIRQ(SPI6_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif // STM32H7