#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#include "SPI/SPI_Master.h"

#if defined(NUC442) || defined(NUC472)

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "SPI/Pin/Nuvoton/SPI_Pin_Nuvoton.h"

#include "SPI_Master_Nuvoton.h"

typedef enum {
    SPI_XFER_IDLE = 0,
    SPI_XFER_TX,
    SPI_XFER_RX,
    SPI_XFER_TXRX
} SPI_XferMode_t;

typedef struct {
    volatile SPI_XferMode_t mode;

    const uint8_t *tx_buf;
    uint8_t *rx_buf;

    volatile uint16_t len;
    volatile uint16_t tx_count;
    volatile uint16_t rx_count;
} SPI_Nuvoton_XferCtx_t;

static SPI_Nuvoton_XferCtx_t spi_xfer_ctx[hwSPI_Index_MAX];

SPI_T *SPI_Map_Soc_Base(hwSPI_Index index)
{
    switch(index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0: return SPI0;
#endif
#if defined(SPI1_BASE)
        case hwSPI_Index_1: return SPI1;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_2: return SPI2;
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_3: return SPI3;
#endif
        default: return NULL;
    }
}

void SPI_GPIO_ConfigAF(hwSPI_Index index, bool cs)
{
    SPI_Pin_Def def = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]];

    SYS_UnlockReg();

    switch(index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            if(def.mosi_pin == hwGPIO_Pin_A0 &&
               def.miso_pin == hwGPIO_Pin_A1 &&
               def.sclk_pin == hwGPIO_Pin_A2)
            {
                SYS->GPA_MFPL =
                    (SYS->GPA_MFPL &
                    ~(SYS_GPA_MFPL_PA0MFP_Msk |
                      SYS_GPA_MFPL_PA1MFP_Msk |
                      SYS_GPA_MFPL_PA2MFP_Msk)) |
                    (0x2UL << SYS_GPA_MFPL_PA0MFP_Pos) |
                    (0x2UL << SYS_GPA_MFPL_PA1MFP_Pos) |
                    (0x2UL << SYS_GPA_MFPL_PA2MFP_Pos);
            }

            if(cs && def.cs_pin == hwGPIO_Pin_A3)
            {
                SYS->GPA_MFPL =
                    (SYS->GPA_MFPL & ~SYS_GPA_MFPL_PA3MFP_Msk) |
                    (0x2UL << SYS_GPA_MFPL_PA3MFP_Pos);
            }
            break;
#endif

#if defined(SPI1_BASE)
        case hwSPI_Index_1:
            if(def.mosi_pin == hwGPIO_Pin_B0 &&
               def.miso_pin == hwGPIO_Pin_B1 &&
               def.sclk_pin == hwGPIO_Pin_B2)
            {
                SYS->GPB_MFPL =
                    (SYS->GPB_MFPL &
                    ~(SYS_GPB_MFPL_PB0MFP_Msk |
                      SYS_GPB_MFPL_PB1MFP_Msk |
                      SYS_GPB_MFPL_PB2MFP_Msk)) |
                    (0x2UL << SYS_GPB_MFPL_PB0MFP_Pos) |
                    (0x2UL << SYS_GPB_MFPL_PB1MFP_Pos) |
                    (0x2UL << SYS_GPB_MFPL_PB2MFP_Pos);
            }

            if(cs && def.cs_pin == hwGPIO_Pin_B3)
            {
                SYS->GPB_MFPL =
                    (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB3MFP_Msk) |
                    (0x2UL << SYS_GPB_MFPL_PB3MFP_Pos);
            }
            break;
#endif

#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            if(def.mosi_pin == hwGPIO_Pin_C0 &&
               def.miso_pin == hwGPIO_Pin_C1 &&
               def.sclk_pin == hwGPIO_Pin_C2)
            {
                SYS->GPC_MFPL =
                    (SYS->GPC_MFPL &
                    ~(SYS_GPC_MFPL_PC0MFP_Msk |
                      SYS_GPC_MFPL_PC1MFP_Msk |
                      SYS_GPC_MFPL_PC2MFP_Msk)) |
                    (0x2UL << SYS_GPC_MFPL_PC0MFP_Pos) |
                    (0x2UL << SYS_GPC_MFPL_PC1MFP_Pos) |
                    (0x2UL << SYS_GPC_MFPL_PC2MFP_Pos);
            }

            if(cs && def.cs_pin == hwGPIO_Pin_C3)
            {
                SYS->GPC_MFPL =
                    (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC3MFP_Msk) |
                    (0x2UL << SYS_GPC_MFPL_PC3MFP_Pos);
            }
            break;
#endif

#if defined(SPI3_BASE)
        case hwSPI_Index_3:
            if(def.mosi_pin == hwGPIO_Pin_D0 &&
               def.miso_pin == hwGPIO_Pin_D1 &&
               def.sclk_pin == hwGPIO_Pin_D2)
            {
                SYS->GPD_MFPL =
                    (SYS->GPD_MFPL &
                    ~(SYS_GPD_MFPL_PD0MFP_Msk |
                      SYS_GPD_MFPL_PD1MFP_Msk |
                      SYS_GPD_MFPL_PD2MFP_Msk)) |
                    (0x2UL << SYS_GPD_MFPL_PD0MFP_Pos) |
                    (0x2UL << SYS_GPD_MFPL_PD1MFP_Pos) |
                    (0x2UL << SYS_GPD_MFPL_PD2MFP_Pos);
            }

            if(cs && def.cs_pin == hwGPIO_Pin_D3)
            {
                SYS->GPD_MFPL =
                    (SYS->GPD_MFPL & ~SYS_GPD_MFPL_PD3MFP_Msk) |
                    (0x2UL << SYS_GPD_MFPL_PD3MFP_Pos);
            }
            break;
#endif

        default:
            break;
    }

    SYS_LockReg();
}

void SPI_GPIO_DeConfigAF(hwSPI_Index index, bool cs)
{
    SPI_Pin_Def def = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]];

    GPIO_T *mosi_port = GPIO_Map_Soc_Base(def.mosi_pin);
    GPIO_T *miso_port = GPIO_Map_Soc_Base(def.miso_pin);
    GPIO_T *sclk_port = GPIO_Map_Soc_Base(def.sclk_pin);
    GPIO_T *cs_port = NULL;
    if(cs)
    {
        cs_port = GPIO_Map_Soc_Base(def.cs_pin);
    }

    uint16_t mosi_mask = GPIO_Map_Soc_Pin(def.mosi_pin);
    uint16_t miso_mask = GPIO_Map_Soc_Pin(def.miso_pin);
    uint16_t sclk_mask = GPIO_Map_Soc_Pin(def.sclk_pin);
    uint16_t cs_mask = 0;
    if(cs)
    {
        cs_mask = GPIO_Map_Soc_Pin(def.cs_pin);
    }

    SYS_UnlockReg();

    if(mosi_port && mosi_mask) GPIO_SetMode(mosi_port, mosi_mask, GPIO_MODE_INPUT);
    if(miso_port && miso_mask) GPIO_SetMode(miso_port, miso_mask, GPIO_MODE_INPUT);
    if(sclk_port && sclk_mask) GPIO_SetMode(sclk_port, sclk_mask, GPIO_MODE_INPUT);
    if(cs_port   && cs_mask)   GPIO_SetMode(cs_port,   cs_mask,   GPIO_MODE_INPUT);

    switch(index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            if(def.mosi_pin == hwGPIO_Pin_A0 &&
               def.miso_pin == hwGPIO_Pin_A1 &&
               def.sclk_pin == hwGPIO_Pin_A2)
            {
                SYS->GPA_MFPL &=
                    ~(SYS_GPA_MFPL_PA0MFP_Msk |
                      SYS_GPA_MFPL_PA1MFP_Msk |
                      SYS_GPA_MFPL_PA2MFP_Msk);
            }

            if(cs)
            {
                if(def.cs_pin == hwGPIO_Pin_A3)
                {
                    SYS->GPA_MFPL &= ~SYS_GPA_MFPL_PA3MFP_Msk;
                }
            }
            break;
#endif

#if defined(SPI1_BASE)
        case hwSPI_Index_1:
            if(def.mosi_pin == hwGPIO_Pin_B0 &&
               def.miso_pin == hwGPIO_Pin_B1 &&
               def.sclk_pin == hwGPIO_Pin_B2)
            {
                SYS->GPB_MFPL &=
                    ~(SYS_GPB_MFPL_PB0MFP_Msk |
                      SYS_GPB_MFPL_PB1MFP_Msk |
                      SYS_GPB_MFPL_PB2MFP_Msk);
            }

            if(cs)
            {
                if(def.cs_pin == hwGPIO_Pin_B3)
                {
                    SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB3MFP_Msk;
                }
            }
            break;
#endif

#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            if(def.mosi_pin == hwGPIO_Pin_C0 &&
               def.miso_pin == hwGPIO_Pin_C1 &&
               def.sclk_pin == hwGPIO_Pin_C2)
            {
                SYS->GPC_MFPL &=
                    ~(SYS_GPC_MFPL_PC0MFP_Msk |
                      SYS_GPC_MFPL_PC1MFP_Msk |
                      SYS_GPC_MFPL_PC2MFP_Msk);
            }

            if(cs)
            {
                if(def.cs_pin == hwGPIO_Pin_C3)
                {
                    SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC3MFP_Msk;
                }
            }
            break;
#endif

#if defined(SPI3_BASE)
        case hwSPI_Index_3:
            if(def.mosi_pin == hwGPIO_Pin_D0 &&
               def.miso_pin == hwGPIO_Pin_D1 &&
               def.sclk_pin == hwGPIO_Pin_D2)
            {
                SYS->GPD_MFPL &=
                    ~(SYS_GPD_MFPL_PD0MFP_Msk |
                      SYS_GPD_MFPL_PD1MFP_Msk |
                      SYS_GPD_MFPL_PD2MFP_Msk);
            }

            if(cs)
            {
                if(def.cs_pin == hwGPIO_Pin_D3)
                {
                    SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD3MFP_Msk;
                }
            }
            break;
#endif

        default:
            break;
    }

    SYS_LockReg();
}

static void SPI_Nuvoton_IRQHandler(hwSPI_Index index)
{
    SPI_T *spi = SPI_Map_Soc_Base(index);
    if (spi == NULL)
        return;

    if (!(spi->STATUS & SPI_STATUS_UNITIF_Msk))
        return;

    spi->STATUS = SPI_STATUS_UNITIF_Msk;

    SPI_Nuvoton_XferCtx_t *ctx = &spi_xfer_ctx[index];

    if (ctx->mode == SPI_XFER_IDLE)
        return;

    if (ctx->mode == SPI_XFER_RX || ctx->mode == SPI_XFER_TXRX)
    {
        if (ctx->rx_count < ctx->len)
        {
            ctx->rx_buf[ctx->rx_count++] = (uint8_t)SPI_READ_RX(spi);
        }
        else
        {
            volatile uint32_t dummy = SPI_READ_RX(spi);
            (void)dummy;
        }
    }
    else
    {
        volatile uint32_t dummy = SPI_READ_RX(spi);
        (void)dummy;
    }

    if (ctx->tx_count < ctx->len)
    {
        uint8_t out = 0xFF;

        if (ctx->mode == SPI_XFER_TX || ctx->mode == SPI_XFER_TXRX)
            out = ctx->tx_buf[ctx->tx_count];

        SPI_WRITE_TX(spi, out);
        ctx->tx_count++;
        SPI_TRIGGER(spi);
        return;
    }

    if ((ctx->mode == SPI_XFER_RX || ctx->mode == SPI_XFER_TXRX) &&
        ctx->rx_count < ctx->len)
    {
        SPI_WRITE_TX(spi, 0xFF);
        ctx->tx_count++;
        SPI_TRIGGER(spi);
        return;
    }

    SPI_XferMode_t done_mode = ctx->mode;
    ctx->mode = SPI_XFER_IDLE;

    SPI_DisableInt(spi, SPI_STATUS_UNITIF_Msk);
    spi->STATUS = SPI_STATUS_UNITIF_Msk;

    if (done_mode == SPI_XFER_TX)
    {
        SPI_TxCpltCallback(index);
    }
    else if (done_mode == SPI_XFER_RX)
    {
        SPI_RxCpltCallback(index);
    }
    else
    {
        SPI_TxRxCpltCallback(index);
    }
}

#if defined(SPI0_BASE)
void SPI0_IRQHandler(void)
{
    SPI_Nuvoton_IRQHandler(hwSPI_Index_0);
}
#endif

#if defined(SPI1_BASE)
void SPI1_IRQHandler(void)
{
    SPI_Nuvoton_IRQHandler(hwSPI_Index_1);
}
#endif

#if defined(SPI2_BASE)
void SPI2_IRQHandler(void)
{
    SPI_Nuvoton_IRQHandler(hwSPI_Index_2);
}
#endif

#if defined(SPI3_BASE)
void SPI3_IRQHandler(void)
{
    SPI_Nuvoton_IRQHandler(hwSPI_Index_3);
}
#endif

static void SPI_EnableClock(hwSPI_Index index)
{
    switch(index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            CLK_EnableModuleClock(SPI0_MODULE);
            break;
#endif
#if defined(SPI1_BASE)
        case hwSPI_Index_1:
            CLK_EnableModuleClock(SPI1_MODULE);
            break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            CLK_EnableModuleClock(SPI2_MODULE);
            break;
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_3:
            CLK_EnableModuleClock(SPI3_MODULE);
            break;
#endif
        default:
            break;
    }
}

static void SPI_DisableClock(hwSPI_Index index)
{
    switch(index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            CLK_DisableModuleClock(SPI0_MODULE);
            break;
#endif
#if defined(SPI1_BASE)
        case hwSPI_Index_1:
            CLK_DisableModuleClock(SPI1_MODULE);
            break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            CLK_DisableModuleClock(SPI2_MODULE);
            break;
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_3:
            CLK_DisableModuleClock(SPI3_MODULE);
            break;
#endif
        default:
            break;
    }
}

hwSPI_OpResult SPI_Instance_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode, bool cs)
{
    if(index >= hwSPI_Index_MAX)
        return hwSPI_InvalidParameter;

    SPI_T *spi = SPI_Map_Soc_Base(index);

    if(spi == NULL)
        return hwSPI_InvalidParameter;

    if(clock_rate_hz == 0)
        return hwSPI_InvalidParameter;

    SPI_EnableClock(index);
    
    switch(index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            SYS_ResetModule(SPI0_RST);
            break;
#endif
#if defined(SPI1_BASE)
        case hwSPI_Index_1:
            SYS_ResetModule(SPI1_RST);
            break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            SYS_ResetModule(SPI2_RST);
            break;
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_3:
            SYS_ResetModule(SPI3_RST);
            break;
#endif
        default:
            break;
    }

    switch(opMode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            SPI_Open(spi, SPI_MASTER, SPI_MODE_0, 8, clock_rate_hz);
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            SPI_Open(spi, SPI_MASTER, SPI_MODE_1, 8, clock_rate_hz);
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            SPI_Open(spi, SPI_MASTER, SPI_MODE_2, 8, clock_rate_hz);
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            SPI_Open(spi, SPI_MASTER, SPI_MODE_3, 8, clock_rate_hz);
            break;
    }
    
    /*
     * Software SS.
     * 上層如果自己用 GPIO 控 CS，這裡不要自動控制 SS。
     */
    if(cs)
    {
        SPI_EnableAutoSS(spi, SPI_SS0, SPI_SS_ACTIVE_LOW);
    }
    else
    {
        SPI_DisableAutoSS(spi);
    }

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Instance_DeInit(hwSPI_Index index, bool cs)
{
    if(index >= hwSPI_Index_MAX)
        return hwSPI_InvalidParameter;

    SPI_T *spi = SPI_Map_Soc_Base(index);

    if(spi == NULL)
        return hwSPI_InvalidParameter;

    if(cs)
    {
        SPI_DisableAutoSS(spi);
    }

    SPI_Close(spi);

    SPI_DisableClock(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Instance_ChangeFrequency(hwSPI_Index index, uint32_t clock_rate_hz)
{
    if (index >= hwSPI_Index_MAX || clock_rate_hz == 0)
        return hwSPI_InvalidParameter;

    SPI_T *spi = SPI_Map_Soc_Base(index);
    if (spi == NULL)
        return hwSPI_InvalidParameter;

    SPI_SetBusClock(spi, clock_rate_hz);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Instance_ChangeMode(hwSPI_Index index, hwSPI_OpMode opMode)
{
    if (index >= hwSPI_Index_MAX || opMode >= hwSPI_OpMode_MAX)
        return hwSPI_InvalidParameter;

    SPI_T *spi = SPI_Map_Soc_Base(index);
    if (spi == NULL)
        return hwSPI_InvalidParameter;

    spi->CTL &= ~(SPI_CTL_TXNEG_Msk | SPI_CTL_RXNEG_Msk | SPI_CTL_CLKPOL_Msk);

    switch (opMode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            spi->CTL |= SPI_MODE_0;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            spi->CTL |= SPI_MODE_1;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            spi->CTL |= SPI_MODE_2;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            spi->CTL |= SPI_MODE_3;
            break;

        default:
            return hwSPI_InvalidParameter;
    }

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Instance_Transmit_IT(hwSPI_Index index, const uint8_t *buf, uint16_t len)
{
    if (index >= hwSPI_Index_MAX || buf == NULL || len == 0)
        return hwSPI_InvalidParameter;

    SPI_T *spi = SPI_Map_Soc_Base(index);
    if (spi == NULL)
        return hwSPI_InvalidParameter;

    SPI_Nuvoton_XferCtx_t *ctx = &spi_xfer_ctx[index];

    if (ctx->mode != SPI_XFER_IDLE)
        return hwSPI_HwError;

    ctx->mode = SPI_XFER_TX;
    ctx->tx_buf = buf;
    ctx->rx_buf = NULL;
    ctx->len = len;
    ctx->tx_count = 0;
    ctx->rx_count = 0;

    spi->STATUS = SPI_STATUS_UNITIF_Msk;
    SPI_EnableInt(spi, SPI_STATUS_UNITIF_Msk);

    SPI_WRITE_TX(spi, ctx->tx_buf[ctx->tx_count++]);
    SPI_TRIGGER(spi);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Instance_Receive_IT(hwSPI_Index index, uint8_t *buf, uint16_t len)
{
    if (index >= hwSPI_Index_MAX || buf == NULL || len == 0)
        return hwSPI_InvalidParameter;

    SPI_T *spi = SPI_Map_Soc_Base(index);
    if (spi == NULL)
        return hwSPI_InvalidParameter;

    SPI_Nuvoton_XferCtx_t *ctx = &spi_xfer_ctx[index];

    if (ctx->mode != SPI_XFER_IDLE)
        return hwSPI_HwError;

    ctx->mode = SPI_XFER_RX;
    ctx->tx_buf = NULL;
    ctx->rx_buf = buf;
    ctx->len = len;
    ctx->tx_count = 0;
    ctx->rx_count = 0;

    spi->STATUS = SPI_STATUS_UNITIF_Msk;
    SPI_EnableInt(spi, SPI_STATUS_UNITIF_Msk);

    SPI_WRITE_TX(spi, 0xFF);
    ctx->tx_count++;
    SPI_TRIGGER(spi);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Instance_TransmitReceive_IT(hwSPI_Index index, const uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len)
{
    if (index >= hwSPI_Index_MAX || tx_buf == NULL || rx_buf == NULL || len == 0)
        return hwSPI_InvalidParameter;

    SPI_T *spi = SPI_Map_Soc_Base(index);
    if (spi == NULL)
        return hwSPI_InvalidParameter;

    SPI_Nuvoton_XferCtx_t *ctx = &spi_xfer_ctx[index];

    if (ctx->mode != SPI_XFER_IDLE)
        return hwSPI_HwError;

    ctx->mode = SPI_XFER_TXRX;
    ctx->tx_buf = tx_buf;
    ctx->rx_buf = rx_buf;
    ctx->len = len;
    ctx->tx_count = 0;
    ctx->rx_count = 0;

    spi->STATUS = SPI_STATUS_UNITIF_Msk;
    SPI_EnableInt(spi, SPI_STATUS_UNITIF_Msk);

    SPI_WRITE_TX(spi, ctx->tx_buf[ctx->tx_count++]);
    SPI_TRIGGER(spi);

    return hwSPI_OK;
}

void SPI_NVIC_Init(hwSPI_Index index)
{
    if(index >= hwSPI_Index_MAX)
        return;

    switch(index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            NVIC_ClearPendingIRQ(SPI0_IRQn);
            NVIC_EnableIRQ(SPI0_IRQn);
            break;
#endif
#if defined(SPI1_BASE)
            case hwSPI_Index_1:
            NVIC_ClearPendingIRQ(SPI1_IRQn);
            NVIC_EnableIRQ(SPI1_IRQn);
            break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            NVIC_ClearPendingIRQ(SPI2_IRQn);
            NVIC_EnableIRQ(SPI2_IRQn);
            break;
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_3:
            NVIC_ClearPendingIRQ(SPI3_IRQn);
            NVIC_EnableIRQ(SPI3_IRQn);
            break;
#endif
    }
}

void SPI_NVIC_DeInit(hwSPI_Index index)
{
    if(index >= hwSPI_Index_MAX)
        return;

    switch(index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            NVIC_DisableIRQ(SPI0_IRQn);
            NVIC_ClearPendingIRQ(SPI0_IRQn);
            break;
#endif
#if defined(SPI1_BASE)
            case hwSPI_Index_1:
            NVIC_DisableIRQ(SPI1_IRQn);
            NVIC_ClearPendingIRQ(SPI1_IRQn);
            break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            NVIC_DisableIRQ(SPI2_IRQn);
            NVIC_ClearPendingIRQ(SPI2_IRQn);
            break;
#endif
#if defined(SPI3_BASE)
        case hwSPI_Index_3:
            NVIC_DisableIRQ(SPI3_IRQn);
            NVIC_ClearPendingIRQ(SPI3_IRQn);
            break;
#endif
    }
}

#endif // NUC442 || NUC472