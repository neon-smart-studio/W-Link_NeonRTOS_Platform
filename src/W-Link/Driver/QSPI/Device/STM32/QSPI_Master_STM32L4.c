#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"
#include "QSPI/QSPI_Master.h"

#ifdef STM32L4

#if defined(QUADSPI)

#include "QSPI_Master_STM32.h"

QSPI_HandleTypeDef g_qspi[hwQSPI_Index_MAX];

QUADSPI_TypeDef *QSPI_Map_Soc_Base(hwQSPI_Index index)
{
    switch(index)
    {
        case hwQSPI_Index_0:
            return QUADSPI;

        default:
            return NULL;
    }
}

static void QSPI_EnableClock(hwQSPI_Index index)
{
    switch(index)
    {
        case hwQSPI_Index_0:
            __HAL_RCC_QSPI_CLK_ENABLE();
            break;

        default:
            break;
    }
}

static void QSPI_DisableClock(hwQSPI_Index index)
{
    switch(index)
    {
        case hwQSPI_Index_0:
            __HAL_RCC_QSPI_CLK_DISABLE();
            break;

        default:
            break;
    }
}

int QSPI_Master_Get_Clock_Freq(hwQSPI_Index index)
{
    switch(index)
    {
        case hwQSPI_Index_0:
            return HAL_RCC_GetHCLKFreq();

        default:
            return 0;
    }
}

hwQSPI_OpResult QSPI_Instance_Init(hwQSPI_Index index,
                                   uint32_t clock_rate_hz,
                                   hwQSPI_OpMode opMode)
{
    QUADSPI_TypeDef *qspi = QSPI_Map_Soc_Base(index);

    if (qspi == NULL || clock_rate_hz == 0 || opMode >= hwQSPI_OpMode_MAX)
        return hwQSPI_InvalidParameter;

    QSPI_EnableClock(index);

    g_qspi[index].Instance = qspi;

    uint32_t qspi_clk = QSPI_Master_Get_Clock_Freq(index);
    if (qspi_clk == 0)
        return hwQSPI_HwError;

    uint32_t prescaler = qspi_clk / clock_rate_hz;

    if ((qspi_clk % clock_rate_hz) != 0)
        prescaler++;

    if (prescaler == 0)
        prescaler = 1;

    prescaler--;

    if (prescaler > 255)
        prescaler = 255;

    g_qspi[index].Init.ClockPrescaler     = prescaler;
    g_qspi[index].Init.FifoThreshold      = 4;
    g_qspi[index].Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_NONE;
    g_qspi[index].Init.FlashSize          = 27;
    g_qspi[index].Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
    g_qspi[index].Init.ClockMode          = QSPI_CLOCK_MODE_0;
    g_qspi[index].Init.FlashID            = QSPI_FLASH_ID_1;
    g_qspi[index].Init.DualFlash          = QSPI_DUALFLASH_DISABLE;

    switch (opMode)
    {
        case hwQSPI_OpMode_Polarity0_Phase0:
            g_qspi[index].Init.ClockMode = QSPI_CLOCK_MODE_0;
            break;

        case hwQSPI_OpMode_Polarity1_Phase1:
            g_qspi[index].Init.ClockMode = QSPI_CLOCK_MODE_3;
            break;

        default:
            QSPI_DisableClock(index);
            return hwQSPI_InvalidParameter;
    }

    if (HAL_QSPI_Init(&g_qspi[index]) != HAL_OK)
        return hwQSPI_HwError;

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Instance_DeInit(hwQSPI_Index index)
{
    if (index >= hwQSPI_Index_MAX)
        return hwQSPI_InvalidParameter;

    if (HAL_QSPI_DeInit(&g_qspi[index]) != HAL_OK)
        return hwQSPI_HwError;

    QSPI_DisableClock(index);

    return hwQSPI_OK;
}

static void QSPI_HAL_IRQHandler(hwQSPI_Index index)
{
    HAL_QSPI_IRQHandler(&g_qspi[index]);
}

void QUADSPI_IRQHandler(void)
{
    QSPI_HAL_IRQHandler(hwQSPI_Index_0);
}

void QSPI_NVIC_Init(hwQSPI_Index index)
{
    IRQn_Type irq;

    switch (index)
    {
        case hwQSPI_Index_0:
            irq = QUADSPI_IRQn;
            break;

        default:
            return;
    }

    HAL_NVIC_SetPriority(irq, 5, 0);
    HAL_NVIC_EnableIRQ(irq);
}

void QSPI_NVIC_DeInit(hwQSPI_Index index)
{
    switch (index)
    {
        case hwQSPI_Index_0:
            HAL_NVIC_DisableIRQ(QUADSPI_IRQn);
            break;

        default:
            break;
    }
}

#endif /* QUADSPI */

#endif /* STM32L4 */