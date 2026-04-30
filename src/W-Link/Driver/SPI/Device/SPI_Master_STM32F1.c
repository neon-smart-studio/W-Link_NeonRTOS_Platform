#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#ifdef STM32F1

#include "SPI/SPI_Master.h"
#include "SPI/SPI_Pin.h"

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
    switch(index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0: return SPI1;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_1: return SPI2;
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

        default:
            break;
    }
}

int SPI_Master_Get_Clock_Freq(hwSPI_Index index)
{
    switch(index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
#endif
            return HAL_RCC_GetPCLK2Freq();

#if defined(SPI2_BASE)
        case hwSPI_Index_1:
#endif
            return HAL_RCC_GetPCLK1Freq();

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
    g_spi[index].Init.TIMode     = SPI_TIMODE_DISABLE;
    g_spi[index].Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

    switch(opMode)
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

#if defined(SPI1_BASE)
void SPI1_IRQHandler(void){ SPI_HAL_IRQHandler(hwSPI_Index_0); }
#endif
#if defined(SPI2_BASE)
void SPI2_IRQHandler(void){ SPI_HAL_IRQHandler(hwSPI_Index_1); }
#endif

void SPI_NVIC_Init(hwSPI_Index index)
{
    IRQn_Type irq;

    switch(index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0: irq = SPI1_IRQn; break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_1: irq = SPI2_IRQn; break;
#endif
        default: return;
    }

    HAL_NVIC_SetPriority(irq, 5, 0);
    HAL_NVIC_EnableIRQ(irq);
}

void SPI_NVIC_DeInit(hwSPI_Index index)
{
    switch(index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0: HAL_NVIC_DisableIRQ(SPI1_IRQn); break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_1: HAL_NVIC_DisableIRQ(SPI2_IRQn); break;
#endif
    }
}

hwSPI_OpResult SPI_ApplyRemap(
    hwSPI_Index index,
    hwGPIO_Pin mosi_pin,
    hwGPIO_Pin miso_pin,
    hwGPIO_Pin sck_pin,
    hwGPIO_Pin nss_pin,
    bool use_hw_nss
)
{
    __HAL_RCC_AFIO_CLK_ENABLE();

    hwGPIO_Pin default_mosi_pin = SPI_Pin_Def_Table[index][0].mosi_pin;
    hwGPIO_Pin default_miso_pin = SPI_Pin_Def_Table[index][0].miso_pin;
    hwGPIO_Pin default_sck_pin  = SPI_Pin_Def_Table[index][0].sclk_pin;
    hwGPIO_Pin default_nss_pin  = SPI_Pin_Def_Table[index][0].cs_pin;

    hwGPIO_Pin alt_mosi_pin = SPI_Pin_Def_Table[index][1].mosi_pin;
    hwGPIO_Pin alt_miso_pin = SPI_Pin_Def_Table[index][1].miso_pin;
    hwGPIO_Pin alt_sck_pin  = SPI_Pin_Def_Table[index][1].sclk_pin;
    hwGPIO_Pin alt_nss_pin  = SPI_Pin_Def_Table[index][1].cs_pin;

    bool is_default =
        mosi_pin == default_mosi_pin &&
        miso_pin == default_miso_pin &&
        sck_pin  == default_sck_pin &&
        (!use_hw_nss || nss_pin == default_nss_pin);

    bool is_alt =
        mosi_pin == alt_mosi_pin &&
        miso_pin == alt_miso_pin &&
        sck_pin  == alt_sck_pin &&
        (!use_hw_nss || nss_pin == alt_nss_pin);

    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
            if (is_default) {
#if defined(__HAL_AFIO_REMAP_SPI1_DISABLE)
                __HAL_AFIO_REMAP_SPI1_DISABLE();
#else
                __HAL_AFIO_REMAP_SPI1_DISABLE();
#endif
                return hwSPI_OK;
            }

            if (is_alt) {
#if defined(__HAL_AFIO_REMAP_SPI1_ENABLE)
                __HAL_AFIO_REMAP_SPI1_ENABLE();
#else
                __HAL_AFIO_REMAP_SPI1_ENABLE();
#endif
                return hwSPI_OK;
            }

            return hwSPI_InvalidParameter;
#endif

#if defined(SPI2_BASE)
        case hwSPI_Index_1:
            if (is_default) {
                return hwSPI_OK;
            }

            /*
             * STM32F1 SPI2 通常沒有 AFIO SPI2 remap。
             * 你 pin table 裡的 PC3/PC2/PB10/PB9 不建議直接當 ALT 用。
             */
            if (is_alt) {
#if defined(__HAL_AFIO_REMAP_SPI2_ENABLE)
                __HAL_AFIO_REMAP_SPI2_ENABLE();
                return hwSPI_OK;
#else
                return hwSPI_InvalidParameter;
#endif
            }

            return hwSPI_InvalidParameter;
#endif

        default:
            return hwSPI_OK;
    }
}

void SPI_RestoreRemap(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI1_BASE)
        case hwSPI_Index_0:
#if defined(__HAL_AFIO_REMAP_SPI1_DISABLE)
            __HAL_AFIO_REMAP_SPI1_DISABLE();
#endif
            break;
#endif

#if defined(SPI2_BASE)
        case hwSPI_Index_1:
#if defined(__HAL_AFIO_REMAP_SPI2_DISABLE)
            __HAL_AFIO_REMAP_SPI2_DISABLE();
#endif
            break;
#endif

        default:
            break;
    }

    __HAL_RCC_AFIO_CLK_DISABLE();
}

#endif // STM32F1