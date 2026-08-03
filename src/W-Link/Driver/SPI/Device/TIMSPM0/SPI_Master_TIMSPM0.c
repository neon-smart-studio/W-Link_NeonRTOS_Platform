#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "DMA/DMA.h"
#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSPM0

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"
#include "SPI/Pin/TIMSPM0/SPI_Pin_TIMSPM0.h"

#if defined(SPI0_BASE) || defined(SPI1_BASE) || defined(SPI2_BASE)
#define SPI_TIMSPM0_HAS_LEGACY_SPI
#endif

#if defined(UC2_SPI_BASE) || defined(UC3_SPI_BASE) || \
    defined(UC4_SPI_BASE) || defined(UC8_SPI_BASE)
#define SPI_TIMSPM0_HAS_UNICOMM_SPI
#endif

#if defined(SPI_TIMSPM0_HAS_LEGACY_SPI) && \
    defined(SPI_TIMSPM0_HAS_UNICOMM_SPI)
#error "A TIMSPM0 device cannot mix legacy SPI and UNICOMM SPI instances"
#endif

#define SPI_MASTER_MUTEX_ACCESS_TIMEOUT      (500U)
#define SPI_MASTER_OP_TIMEOUT                (3000U)

#define SPI_TIMSPM0_SCR_MAX                  (1023U)
#define SPI_TIMSPM0_DUMMY_DATA               (0xFFU)

#define SPI_TIMSPM0_ERROR_INTERRUPTS          \
    (DL_SPI_INTERRUPT_RX_OVERFLOW |           \
     DL_SPI_INTERRUPT_PARITY_ERROR)

#define SPI_TIMSPM0_TRANSFER_INTERRUPTS       \
    (DL_SPI_INTERRUPT_IDLE |                  \
     DL_SPI_INTERRUPT_TX |                    \
     DL_SPI_INTERRUPT_RX |                    \
     SPI_TIMSPM0_ERROR_INTERRUPTS)

#define SPI_TIMSPM0_ALL_INTERRUPTS            \
    (SPI_TIMSPM0_TRANSFER_INTERRUPTS |         \
     DL_SPI_INTERRUPT_TX_EMPTY |              \
     DL_SPI_INTERRUPT_RX_TIMEOUT |            \
     DL_SPI_INTERRUPT_RX_FULL |               \
     DL_SPI_INTERRUPT_TX_UNDERFLOW)

typedef enum
{
    TIMSPM0_SPI_IDLE = 0,
    TIMSPM0_SPI_TX,
    TIMSPM0_SPI_RX,
    TIMSPM0_SPI_TXRX,
    TIMSPM0_SPI_DONE,
    TIMSPM0_SPI_ERROR
} TIMSPM0_SPI_State;

typedef struct
{
    volatile TIMSPM0_SPI_State state;
    const uint8_t *tx_buf;
    uint8_t *rx_buf;
    uint32_t len;
    volatile uint32_t tx_pos;
    volatile uint32_t rx_pos;
} TIMSPM0_SPI_Transfer;

#if defined(SPI_TIMSPM0_HAS_LEGACY_SPI)
typedef SPI_Regs TIMSPM0_SPI_Regs;
#elif defined(SPI_TIMSPM0_HAS_UNICOMM_SPI)
typedef UNICOMM_Inst_Regs TIMSPM0_SPI_Regs;
#else
#error "DEVICE_TIMSPM0 does not expose a supported SPI instance"
#endif

NeonRTOS_LockObj_t Spi_Master_Access_Mutex[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Send_SyncHandle[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Recv_SyncHandle[hwSPI_Index_MAX];

bool Spi_Master_Init_Status[hwSPI_Index_MAX] = {false};

static bool Spi_Master_Use_CS[hwSPI_Index_MAX] = {false};
static uint32_t Spi_Master_Clock_Hz[hwSPI_Index_MAX] = {0U};
static hwSPI_OpMode Spi_Master_Mode[hwSPI_Index_MAX];
static TIMSPM0_SPI_Transfer spi_xfer[hwSPI_Index_MAX];

#define SPI_MASTER_MUTEX_LOCK(index_, timeout_)                            \
    do                                                                    \
    {                                                                     \
        if (NeonRTOS_LockObjLock(                                         \
                &Spi_Master_Access_Mutex[(index_)],                        \
                (timeout_)) != NeonRTOS_OK)                                \
        {                                                                 \
            return hwSPI_MutexTimeout;                                    \
        }                                                                 \
    } while (0)

#define SPI_MASTER_MUTEX_UNLOCK(index_)                                   \
    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[(index_)])

static TIMSPM0_SPI_Regs *SPI_Map_Soc_Base(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            return SPI0_BASE;
#endif
#if defined(SPI1_BASE)
        case hwSPI_Index_1:
            return SPI1_BASE;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            return SPI2_BASE;
#endif

#if defined(UC2_SPI_BASE)
        case hwSPI_Index_2:
            return UC2_SPI_BASE;
#endif
#if defined(UC3_SPI_BASE)
        case hwSPI_Index_3:
            return UC3_SPI_BASE;
#endif
#if defined(UC4_SPI_BASE)
        case hwSPI_Index_4:
            return UC4_SPI_BASE;
#endif
#if defined(UC8_SPI_BASE)
        case hwSPI_Index_8:
            return UC8_SPI_BASE;
#endif

        default:
            return NULL;
    }
}

static uint32_t SPI_Map_Soc_Pin_Function(
    hwSPI_Index index,
    hwGPIO_Pin pin)
{
    switch (index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:

#if defined(MSPM0C110x) || defined(MSPM0S003Fx)

            switch (pin)
            {
                case hwGPIO_Pin_A18:
                    return IOMUX_PINCM19_PF_SPI0_PICO;

                case hwGPIO_Pin_A4:
                    return IOMUX_PINCM5_PF_SPI0_POCI;

                case hwGPIO_Pin_A6:
                    return IOMUX_PINCM7_PF_SPI0_SCLK;

                case hwGPIO_Pin_A2:
                    return IOMUX_PINCM3_PF_SPI0_CS0;

                default:
                    return 0U;
            }

#elif defined(MSPM0C031Cx) || defined(MSPM0G031Cx) || \
      defined(MSPM0C1105) || defined(MSPM0C1106) || \
      defined(MSPM0H321x)

            switch (pin)
            {
                case hwGPIO_Pin_A5:
                    return IOMUX_PINCM8_PF_SPI0_PICO;

                case hwGPIO_Pin_A4:
                    return IOMUX_PINCM7_PF_SPI0_POCI;

                case hwGPIO_Pin_A6:
                    return IOMUX_PINCM9_PF_SPI0_SCLK;

                case hwGPIO_Pin_A2:
                    return IOMUX_PINCM5_PF_SPI0_CS0;

                default:
                    return 0U;
            }

#elif defined(MSPM0L110x) || defined(MSPM0L130x) || \
      defined(MSPM0L134x)

            switch (pin)
            {
                case hwGPIO_Pin_A5:
                    return IOMUX_PINCM6_PF_SPI0_PICO;

                case hwGPIO_Pin_A4:
                    return IOMUX_PINCM5_PF_SPI0_POCI;

                case hwGPIO_Pin_A6:
                    return IOMUX_PINCM7_PF_SPI0_SCLK;

                case hwGPIO_Pin_A2:
                    return IOMUX_PINCM3_PF_SPI0_CS0;

                default:
                    return 0U;
            }

#else

            switch (pin)
            {
                case hwGPIO_Pin_A5:
                    return IOMUX_PINCM10_PF_SPI0_PICO;

                case hwGPIO_Pin_A4:
                    return IOMUX_PINCM9_PF_SPI0_POCI;

                case hwGPIO_Pin_A6:
                    return IOMUX_PINCM11_PF_SPI0_SCLK;

                case hwGPIO_Pin_A2:
                    return IOMUX_PINCM7_PF_SPI0_CS0;

                default:
                    return 0U;
            }

#endif
#endif

#if defined(SPI1_BASE)
        case hwSPI_Index_1:

#if defined(MSPM0L122x) || defined(MSPM0L222x)

            switch (pin)
            {
                case hwGPIO_Pin_B8:
                    return IOMUX_PINCM29_PF_SPI1_PICO;

                case hwGPIO_Pin_B7:
                    return IOMUX_PINCM28_PF_SPI1_POCI;

                case hwGPIO_Pin_B9:
                    return IOMUX_PINCM30_PF_SPI1_SCLK;

                case hwGPIO_Pin_B6:
                    return IOMUX_PINCM27_PF_SPI1_CS0;

                default:
                    return 0U;
            }

#else

            switch (pin)
            {
                case hwGPIO_Pin_B8:
                    return IOMUX_PINCM25_PF_SPI1_PICO;

                case hwGPIO_Pin_B7:
                    return IOMUX_PINCM24_PF_SPI1_POCI;

                case hwGPIO_Pin_B9:
                    return IOMUX_PINCM26_PF_SPI1_SCLK;

                case hwGPIO_Pin_B6:
                    return IOMUX_PINCM23_PF_SPI1_CS0;

                default:
                    return 0U;
            }

#endif
#endif

#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            switch (pin)
            {
                case hwGPIO_Pin_B4:
                    return IOMUX_PINCM17_PF_SPI2_PICO;

                case hwGPIO_Pin_B5:
                    return IOMUX_PINCM18_PF_SPI2_POCI;

                case hwGPIO_Pin_A10:
                    return IOMUX_PINCM21_PF_SPI2_SCLK;

                case hwGPIO_Pin_A4:
                    return IOMUX_PINCM9_PF_SPI2_CS0;

                default:
                    return 0U;
            }

#elif defined(UC2_SPI_BASE)
        case hwSPI_Index_2:
            switch (pin)
            {
                case hwGPIO_Pin_A5:
                    return IOMUX_PINCM10_PF_UC2_PICO;

                case hwGPIO_Pin_A4:
                    return IOMUX_PINCM9_PF_UC2_POCI;

                case hwGPIO_Pin_A6:
                    return IOMUX_PINCM11_PF_UC2_SCLK;

#if defined(MSPM0G511x) || defined(MSPM0G518x)
                case hwGPIO_Pin_A8:
                    return IOMUX_PINCM19_PF_UC2_CS0;
#else
                case hwGPIO_Pin_A2:
                    return IOMUX_PINCM7_PF_UC2_CS0;
#endif

                default:
                    return 0U;
            }
#endif

#if defined(UC3_SPI_BASE)
        case hwSPI_Index_3:
            switch (pin)
            {
                case hwGPIO_Pin_B8:
                    return IOMUX_PINCM25_PF_UC3_PICO;

                case hwGPIO_Pin_B7:
                    return IOMUX_PINCM24_PF_UC3_POCI;

                case hwGPIO_Pin_B9:
                    return IOMUX_PINCM26_PF_UC3_SCLK;

                case hwGPIO_Pin_B6:
                    return IOMUX_PINCM23_PF_UC3_CS0;

                default:
                    return 0U;
            }
#endif

#if defined(UC4_SPI_BASE)
        case hwSPI_Index_4:

#if defined(MSPM0L112x) || defined(MSPM0L211x)

            switch (pin)
            {
                case hwGPIO_Pin_A5:
                    return IOMUX_PINCM10_PF_UC4_PICO;

                case hwGPIO_Pin_A4:
                    return IOMUX_PINCM9_PF_UC4_POCI;

                case hwGPIO_Pin_A6:
                    return IOMUX_PINCM11_PF_UC4_SCLK;

                case hwGPIO_Pin_A2:
                    return IOMUX_PINCM7_PF_UC4_CS0;

                default:
                    return 0U;
            }

#else

            switch (pin)
            {
                case hwGPIO_Pin_B8:
                    return IOMUX_PINCM25_PF_UC4_PICO;

                case hwGPIO_Pin_B7:
                    return IOMUX_PINCM24_PF_UC4_POCI;

                case hwGPIO_Pin_B9:
                    return IOMUX_PINCM26_PF_UC4_SCLK;

                case hwGPIO_Pin_B6:
                    return IOMUX_PINCM23_PF_UC4_CS0;

                default:
                    return 0U;
            }

#endif
#endif

#if defined(UC8_SPI_BASE)
        case hwSPI_Index_8:
            switch (pin)
            {
                case hwGPIO_Pin_B8:
                    return IOMUX_PINCM25_PF_UC8_PICO;

                case hwGPIO_Pin_B7:
                    return IOMUX_PINCM24_PF_UC8_POCI;

                case hwGPIO_Pin_B9:
                    return IOMUX_PINCM26_PF_UC8_SCLK;

                case hwGPIO_Pin_B6:
                    return IOMUX_PINCM23_PF_UC8_CS0;

                default:
                    return 0U;
            }
#endif

        default:
            return 0U;
    }
}

static void SPI_NVIC_Init(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            NVIC_ClearPendingIRQ(SPI0_INT_IRQn);
            NVIC_EnableIRQ(SPI0_INT_IRQn);
            break;
#endif
#if defined(SPI1_BASE)
        case hwSPI_Index_1:
            NVIC_ClearPendingIRQ(SPI1_INT_IRQn);
            NVIC_EnableIRQ(SPI1_INT_IRQn);
            break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            NVIC_ClearPendingIRQ(SPI2_INT_IRQn);
            NVIC_EnableIRQ(SPI2_INT_IRQn);
            break;
#elif defined(UC2_SPI_BASE)
        case hwSPI_Index_2:
            NVIC_ClearPendingIRQ(UC2_INT_IRQn);
            NVIC_EnableIRQ(UC2_INT_IRQn);
            break;
#endif
#if defined(UC3_SPI_BASE)
        case hwSPI_Index_3:
            NVIC_ClearPendingIRQ(UC3_INT_IRQn);
            NVIC_EnableIRQ(UC3_INT_IRQn);
            break;
#endif
#if defined(UC4_SPI_BASE)
        case hwSPI_Index_4:
            NVIC_ClearPendingIRQ(UC4_INT_IRQn);
            NVIC_EnableIRQ(UC4_INT_IRQn);
            break;
#endif
#if defined(UC8_SPI_BASE)
        case hwSPI_Index_8:
            NVIC_ClearPendingIRQ(UC8_INT_IRQn);
            NVIC_EnableIRQ(UC8_INT_IRQn);
            break;
#endif
        default:
            break;
    }
}

static void SPI_NVIC_DeInit(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            NVIC_DisableIRQ(SPI0_INT_IRQn);
            NVIC_ClearPendingIRQ(SPI0_INT_IRQn);
            break;
#endif
#if defined(SPI1_BASE)
        case hwSPI_Index_1:
            NVIC_DisableIRQ(SPI1_INT_IRQn);
            NVIC_ClearPendingIRQ(SPI1_INT_IRQn);
            break;
#endif
#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            NVIC_DisableIRQ(SPI2_INT_IRQn);
            NVIC_ClearPendingIRQ(SPI2_INT_IRQn);
            break;
#elif defined(UC2_SPI_BASE)
        case hwSPI_Index_2:
            NVIC_DisableIRQ(UC2_INT_IRQn);
            NVIC_ClearPendingIRQ(UC2_INT_IRQn);
            break;
#endif
#if defined(UC3_SPI_BASE)
        case hwSPI_Index_3:
            NVIC_DisableIRQ(UC3_INT_IRQn);
            NVIC_ClearPendingIRQ(UC3_INT_IRQn);
            break;
#endif
#if defined(UC4_SPI_BASE)
        case hwSPI_Index_4:
            NVIC_DisableIRQ(UC4_INT_IRQn);
            NVIC_ClearPendingIRQ(UC4_INT_IRQn);
            break;
#endif
#if defined(UC8_SPI_BASE)
        case hwSPI_Index_8:
            NVIC_DisableIRQ(UC8_INT_IRQn);
            NVIC_ClearPendingIRQ(UC8_INT_IRQn);
            break;
#endif
        default:
            break;
    }
}

void SPI_IRQ_Process(hwSPI_Index index)
{
    TIMSPM0_SPI_Regs *base;
    TIMSPM0_SPI_Transfer *transfer;

    if (index >= hwSPI_Index_MAX)
    {
        return;
    }

    base = SPI_Map_Soc_Base(index);

    if (base == NULL)
    {
        return;
    }

    transfer = &spi_xfer[index];

    if((transfer->state != TIMSPM0_SPI_TX) &&
        (transfer->state != TIMSPM0_SPI_RX) &&
        (transfer->state != TIMSPM0_SPI_TXRX))
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        return;
    }

    for (;;)
    {
        DL_SPI_IIDX interrupt_index;

        interrupt_index = DL_SPI_getPendingInterrupt(base);

        if ((uint32_t) interrupt_index == 0U)
        {
            break;
        }

        switch (interrupt_index)
        {
            case DL_SPI_IIDX_RX:
            case DL_SPI_IIDX_RX_FULL:
            case DL_SPI_IIDX_RX_TIMEOUT:
                while (!DL_SPI_isRXFIFOEmpty(base))
                {
                    uint8_t value = DL_SPI_receiveData8(base);

                    if (transfer->rx_pos < transfer->len)
                    {
                        if (transfer->rx_buf != NULL)
                        {
                            transfer->rx_buf[transfer->rx_pos] = value;
                        }

                        transfer->rx_pos++;
                    }
                }
                break;

            case DL_SPI_IIDX_TX:
            case DL_SPI_IIDX_TX_EMPTY:
                while ((transfer->tx_pos < transfer->len) && !DL_SPI_isTXFIFOFull(base))
                {
                    uint8_t value = SPI_TIMSPM0_DUMMY_DATA;

                    if (transfer->tx_buf != NULL)
                    {
                        value = transfer->tx_buf[transfer->tx_pos];
                    }

                    DL_SPI_transmitData8(base, value);
                    transfer->tx_pos++;
                }

                if (transfer->tx_pos >= transfer->len)
                {
                    DL_SPI_disableInterrupt(base, DL_SPI_INTERRUPT_TX);
                }
                break;

            case DL_SPI_IIDX_IDLE:
                while (!DL_SPI_isRXFIFOEmpty(base))
                {
                    uint8_t value = DL_SPI_receiveData8(base);

                    if (transfer->rx_pos < transfer->len)
                    {
                        if (transfer->rx_buf != NULL)
                        {
                            transfer->rx_buf[transfer->rx_pos] = value;
                        }

                        transfer->rx_pos++;
                    }
                }
                break;

            case DL_SPI_IIDX_RX_OVERFLOW:
            case DL_SPI_IIDX_TX_UNDERFLOW:
            case DL_SPI_IIDX_PARITY_ERROR:
                DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);

                transfer->state = TIMSPM0_SPI_ERROR;

                NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[index]);
                NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[index]);
                return;

            default:
                break;
        }

        if((transfer->state == TIMSPM0_SPI_TX) ||
            (transfer->state == TIMSPM0_SPI_RX) ||
            (transfer->state == TIMSPM0_SPI_TXRX))
        {
            if ((transfer->tx_pos >= transfer->len) &&
                (transfer->rx_pos >= transfer->len) &&
                !DL_SPI_isBusy(base))
            {
                DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);

                transfer->state = TIMSPM0_SPI_DONE;

                NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[index]);
                NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[index]);
            }
        }

        if((transfer->state != TIMSPM0_SPI_TX) &&
            (transfer->state != TIMSPM0_SPI_RX) &&
            (transfer->state != TIMSPM0_SPI_TXRX))
        {
            return;
        }
    }

    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        uint8_t value = DL_SPI_receiveData8(base);

        if (transfer->rx_pos < transfer->len)
        {
            if (transfer->rx_buf != NULL)
            {
                transfer->rx_buf[transfer->rx_pos] = value;
            }

            transfer->rx_pos++;
        }
    }

    if((transfer->state == TIMSPM0_SPI_TX) ||
        (transfer->state == TIMSPM0_SPI_RX) ||
        (transfer->state == TIMSPM0_SPI_TXRX))
    {
        if ((transfer->tx_pos >= transfer->len) &&
            (transfer->rx_pos >= transfer->len) &&
            !DL_SPI_isBusy(base))
        {
            DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);

            transfer->state = TIMSPM0_SPI_DONE;

            NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[index]);
            NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[index]);
        }
    }
}

#if defined(SPI0_BASE)
void SPI0_IRQHandler(void)
{
    SPI_IRQ_Process(hwSPI_Index_0);
}
#endif

#if defined(SPI1_BASE)
void SPI1_IRQHandler(void)
{
    SPI_IRQ_Process(hwSPI_Index_1);
}
#endif

#if defined(SPI2_BASE)
void SPI2_IRQHandler(void)
{
    SPI_IRQ_Process(hwSPI_Index_2);
}
#endif

#if defined(UC2_SPI_BASE)
void UC2_IRQHandler(void)
{
    SPI_IRQ_Process(hwSPI_Index_2);
}
#endif

#if defined(UC3_SPI_BASE)
void UC3_IRQHandler(void)
{
    SPI_IRQ_Process(hwSPI_Index_3);
}
#endif

#if defined(UC4_SPI_BASE)
void UC4_IRQHandler(void)
{
    SPI_IRQ_Process(hwSPI_Index_4);
}
#endif

#if defined(UC8_SPI_BASE)
void UC8_IRQHandler(void)
{
    SPI_IRQ_Process(hwSPI_Index_8);
}
#endif

hwSPI_OpResult SPI_Master_Init(
    hwSPI_Index index,
    uint32_t clock_rate_hz,
    hwSPI_OpMode op_mode,
    bool cs)
{
    TIMSPM0_SPI_Regs *base;
    uint32_t serial_clock_divider;

    if ((index >= hwSPI_Index_MAX) ||
        (op_mode >= hwSPI_OpMode_MAX))
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index])
    {
        return hwSPI_OK;
    }

    base = SPI_Map_Soc_Base(index);

    if (base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    uint64_t functional_clock_hz;
    uint64_t denominator;
    uint64_t divider;

    if (clock_rate_hz == 0U)
    {
        return hwSPI_InvalidParameter;
    }

    functional_clock_hz = (uint64_t) g_sys_clock_hz;
    denominator = (uint64_t) clock_rate_hz * 2ULL;

    if ((functional_clock_hz == 0ULL) ||
        (denominator > functional_clock_hz))
    {
        return hwSPI_InvalidParameter;
    }

    /*
     * SPI bit rate = FUNCCLK / ((1 + SCR) * 2).
     * Round the divider upward so the generated clock never exceeds the
     * requested frequency.
     */
    divider = (functional_clock_hz + denominator - 1ULL) / denominator;

    if ((divider == 0ULL) ||
        (divider > ((uint64_t) SPI_TIMSPM0_SCR_MAX + 1ULL)))
    {
        return hwSPI_InvalidParameter;
    }

    serial_clock_divider = (uint32_t) (divider - 1ULL);

    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index].miso_pin;
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index].mosi_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index].cs_pin;

    GPIO_Regs *miso_port = GPIO_Map_Soc_Base(miso_pin);
    GPIO_Regs *mosi_port = GPIO_Map_Soc_Base(mosi_pin);
    GPIO_Regs *sclk_port = GPIO_Map_Soc_Base(sclk_pin);
    GPIO_Regs *cs_port = NULL;

    if ((miso_port == NULL) ||
        (mosi_port == NULL) ||
        (sclk_port == NULL))
    {
        return hwSPI_InvalidParameter;
    }

    uint32_t miso_iomux = GPIO_Map_Soc_Pin_IOMUX(miso_pin);
    uint32_t mosi_iomux = GPIO_Map_Soc_Pin_IOMUX(mosi_pin);
    uint32_t sclk_iomux = GPIO_Map_Soc_Pin_IOMUX(sclk_pin);
    uint32_t cs_iomux = 0U;

    uint32_t miso_function = SPI_Map_Soc_Pin_Function(index, miso_pin);
    uint32_t mosi_function = SPI_Map_Soc_Pin_Function(index, mosi_pin);
    uint32_t sclk_function = SPI_Map_Soc_Pin_Function(index, sclk_pin);
    uint32_t cs_function = 0U;

    if ((miso_function == 0U) ||
        (mosi_function == 0U) ||
        (sclk_function == 0U))
    {
        return hwSPI_InvalidParameter;
    }

    if (cs)
    {
        cs_port = GPIO_Map_Soc_Base(cs_pin);

        if (cs_port == NULL)
        {
            return hwSPI_InvalidParameter;
        }

        cs_iomux = GPIO_Map_Soc_Pin_IOMUX(cs_pin);
        cs_function = SPI_Map_Soc_Pin_Function(index, cs_pin);

        if (cs_function == 0U)
        {
            return hwSPI_InvalidParameter;
        }
    }

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Send_SyncHandle[index]) != NeonRTOS_OK)
    {
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Recv_SyncHandle[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_LockObjCreate(&Spi_Master_Access_Mutex[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
        return hwSPI_MemoryError;
    }

    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[index]);

    DL_GPIO_enablePower(miso_port);
    DL_Common_delayCycles(16U);
    DL_GPIO_enablePower(mosi_port);
    DL_Common_delayCycles(16U);
    DL_GPIO_enablePower(sclk_port);
    DL_Common_delayCycles(16U);

    DL_GPIO_initPeripheralInputFunction(miso_iomux, miso_function);
    DL_GPIO_initPeripheralOutputFunction(mosi_iomux, mosi_function);
    DL_GPIO_initPeripheralOutputFunction(sclk_iomux, sclk_function);

    if (cs)
    {
        DL_GPIO_enablePower(cs_port);
        DL_Common_delayCycles(16U);
        DL_GPIO_initPeripheralOutputFunction(cs_iomux, cs_function);
    }

    DL_SPI_FRAME_FORMAT frame_format;
    DL_SPI_Config spi_config;
    DL_SPI_ClockConfig clock_config;

    switch (op_mode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            frame_format = cs ? DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA0 : DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            frame_format = cs ? DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA1 : DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA1;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            frame_format = cs ? DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA0 : DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA0;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            frame_format = cs ? DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA1 : DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA1;
            break;
    }

    spi_config.mode = DL_SPI_MODE_CONTROLLER;
    spi_config.frameFormat = frame_format;
    spi_config.parity = DL_SPI_PARITY_NONE;
    spi_config.dataSize = DL_SPI_DATA_SIZE_8;
    spi_config.bitOrder = DL_SPI_BIT_ORDER_MSB_FIRST;
    spi_config.chipSelectPin = cs ? DL_SPI_CHIP_SELECT_0 : DL_SPI_CHIP_SELECT_NONE;

    clock_config.clockSel = DL_SPI_CLOCK_BUSCLK;
    clock_config.divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1;

    DL_SPI_disable(base);
    DL_SPI_reset(base);
    DL_SPI_enablePower(base);
    DL_Common_delayCycles(16U);

    DL_SPI_setClockConfig(base, &clock_config);
    DL_SPI_init(base, &spi_config);
    DL_SPI_setBitRateSerialClockDivider(base, serial_clock_divider);
    DL_SPI_setFIFOThreshold(base, DL_SPI_RX_FIFO_LEVEL_ONE_FRAME, DL_SPI_TX_FIFO_LEVEL_ONE_FRAME);

    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);

    DL_SPI_enable(base);

    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        (void) DL_SPI_receiveData8(base);
    }

    memset(&spi_xfer[index], 0, sizeof(spi_xfer[index]));
    spi_xfer[index].state = TIMSPM0_SPI_IDLE;

    Spi_Master_Clock_Hz[index] = clock_rate_hz;
    Spi_Master_Mode[index] = op_mode;
    Spi_Master_Use_CS[index] = cs;

    SPI_NVIC_Init(index);

    gpio_pin_init_status[miso_pin] = true;
    gpio_pin_init_status[mosi_pin] = true;
    gpio_pin_init_status[sclk_pin] = true;

    if (cs)
    {
        gpio_pin_init_status[cs_pin] = true;
    }

    Spi_Master_Init_Status[index] = true;

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_DeInit(hwSPI_Index index)
{
    TIMSPM0_SPI_Regs *base;

    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_OK;
    }

    base = SPI_Map_Soc_Base(index);

    if (base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index].miso_pin;
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index].mosi_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index].cs_pin;

    uint32_t miso_iomux = GPIO_Map_Soc_Pin_IOMUX(miso_pin);
    uint32_t mosi_iomux = GPIO_Map_Soc_Pin_IOMUX(mosi_pin);
    uint32_t sclk_iomux = GPIO_Map_Soc_Pin_IOMUX(sclk_pin);
    uint32_t cs_iomux = 0U;
    bool use_cs = Spi_Master_Use_CS[index];

    if (use_cs)
    {
        cs_iomux = GPIO_Map_Soc_Pin_IOMUX(cs_pin);
    }

    Spi_Master_Init_Status[index] = false;

    SPI_NVIC_DeInit(index);
    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_disable(base);
    DL_SPI_reset(base);
    DL_SPI_disablePower(base);

    NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
    NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);

    DL_GPIO_initDigitalInput(miso_iomux);
    DL_GPIO_initDigitalInput(mosi_iomux);
    DL_GPIO_initDigitalInput(sclk_iomux);

    gpio_pin_init_status[miso_pin] = false;
    gpio_pin_init_status[mosi_pin] = false;
    gpio_pin_init_status[sclk_pin] = false;

    if (use_cs)
    {
        DL_GPIO_initDigitalInput(cs_iomux);
        gpio_pin_init_status[cs_pin] = false;
    }

    memset(&spi_xfer[index], 0, sizeof(spi_xfer[index]));
    spi_xfer[index].state = TIMSPM0_SPI_IDLE;

    Spi_Master_Use_CS[index] = false;
    Spi_Master_Clock_Hz[index] = 0U;
    Spi_Master_Mode[index] = (hwSPI_OpMode) 0;

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Frequency(
    hwSPI_Index index,
    uint32_t clock_rate_hz)
{
    TIMSPM0_SPI_Regs *base;
    uint32_t serial_clock_divider;

    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    base = SPI_Map_Soc_Base(index);

    if (base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    uint64_t functional_clock_hz;
    uint64_t denominator;
    uint64_t divider;

    if (clock_rate_hz == 0U)
    {
        return hwSPI_InvalidParameter;
    }

    functional_clock_hz = (uint64_t) g_sys_clock_hz;
    denominator = (uint64_t) clock_rate_hz * 2ULL;

    if ((functional_clock_hz == 0ULL) || (denominator > functional_clock_hz))
    {
        return hwSPI_InvalidParameter;
    }

    /*
     * SPI bit rate = FUNCCLK / ((1 + SCR) * 2).
     * Round the divider upward so the generated clock never exceeds the
     * requested frequency.
     */
    divider = (functional_clock_hz + denominator - 1ULL) / denominator;

    if ((divider == 0ULL) ||
        (divider > ((uint64_t) SPI_TIMSPM0_SCR_MAX + 1ULL)))
    {
        return false;
    }

    serial_clock_divider = (uint32_t) (divider - 1ULL);

    DL_SPI_FRAME_FORMAT frame_format;
    DL_SPI_Config spi_config;
    DL_SPI_ClockConfig clock_config;

    switch (Spi_Master_Mode[index])
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            frame_format = Spi_Master_Use_CS[index] ? DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA0 : DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            frame_format = Spi_Master_Use_CS[index] ? DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA1 : DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA1;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            frame_format = Spi_Master_Use_CS[index] ? DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA0 : DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA0;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            frame_format = Spi_Master_Use_CS[index] ? DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA1 : DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA1;
            break;
    }

    spi_config.mode = DL_SPI_MODE_CONTROLLER;
    spi_config.frameFormat = frame_format;
    spi_config.parity = DL_SPI_PARITY_NONE;
    spi_config.dataSize = DL_SPI_DATA_SIZE_8;
    spi_config.bitOrder = DL_SPI_BIT_ORDER_MSB_FIRST;
    spi_config.chipSelectPin = Spi_Master_Use_CS[index] ? DL_SPI_CHIP_SELECT_0 : DL_SPI_CHIP_SELECT_NONE;

    clock_config.clockSel = DL_SPI_CLOCK_BUSCLK;
    clock_config.divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1;

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    DL_SPI_disable(base);
    DL_SPI_reset(base);
    DL_SPI_enablePower(base);
    DL_Common_delayCycles(16U);

    DL_SPI_setClockConfig(base, &clock_config);
    DL_SPI_init(base, &spi_config);
    DL_SPI_setBitRateSerialClockDivider(base, serial_clock_divider);
    DL_SPI_setFIFOThreshold(base, DL_SPI_RX_FIFO_LEVEL_ONE_FRAME, DL_SPI_TX_FIFO_LEVEL_ONE_FRAME);

    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);

    DL_SPI_enable(base);
        
    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        (void) DL_SPI_receiveData8(base);
    }

    memset(&spi_xfer[index], 0, sizeof(spi_xfer[index]));
    spi_xfer[index].state = TIMSPM0_SPI_IDLE;
    Spi_Master_Clock_Hz[index] = clock_rate_hz;

    SPI_NVIC_Init(index);

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Mode(
    hwSPI_Index index,
    hwSPI_OpMode op_mode)
{
    TIMSPM0_SPI_Regs *base;
    uint32_t serial_clock_divider;

    if ((index >= hwSPI_Index_MAX) ||
        (op_mode >= hwSPI_OpMode_MAX))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    base = SPI_Map_Soc_Base(index);

    if ((base == NULL))
    {
        return hwSPI_InvalidParameter;
    }

    uint64_t functional_clock_hz;
    uint64_t denominator;
    uint64_t divider;

    if (Spi_Master_Clock_Hz[index] == 0U)
    {
        return hwSPI_InvalidParameter;
    }

    functional_clock_hz = (uint64_t) g_sys_clock_hz;
    denominator = (uint64_t) Spi_Master_Clock_Hz[index] * 2ULL;

    if ((functional_clock_hz == 0ULL) ||
        (denominator > functional_clock_hz))
    {
        return hwSPI_InvalidParameter;
    }

    /*
     * SPI bit rate = FUNCCLK / ((1 + SCR) * 2).
     * Round the divider upward so the generated clock never exceeds the
     * requested frequency.
     */
    divider = (functional_clock_hz + denominator - 1ULL) / denominator;

    if ((divider == 0ULL) || (divider > ((uint64_t) SPI_TIMSPM0_SCR_MAX + 1ULL)))
    {
        return hwSPI_InvalidParameter;
    }

    serial_clock_divider = (uint32_t) (divider - 1ULL);

    DL_SPI_FRAME_FORMAT frame_format;
    DL_SPI_Config spi_config;
    DL_SPI_ClockConfig clock_config;

    switch (op_mode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            frame_format = Spi_Master_Use_CS[index] ? DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA0 : DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            frame_format = Spi_Master_Use_CS[index] ? DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA1 : DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA1;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            frame_format = Spi_Master_Use_CS[index] ? DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA0 : DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA0;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            frame_format = Spi_Master_Use_CS[index] ? DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA1 : DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA1;
            break;
    }

    spi_config.mode = DL_SPI_MODE_CONTROLLER;
    spi_config.frameFormat = frame_format;
    spi_config.parity = DL_SPI_PARITY_NONE;
    spi_config.dataSize = DL_SPI_DATA_SIZE_8;
    spi_config.bitOrder = DL_SPI_BIT_ORDER_MSB_FIRST;
    spi_config.chipSelectPin = Spi_Master_Use_CS[index] ? DL_SPI_CHIP_SELECT_0 : DL_SPI_CHIP_SELECT_NONE;

    clock_config.clockSel = DL_SPI_CLOCK_BUSCLK;
    clock_config.divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1;

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    DL_SPI_disable(base);
    DL_SPI_reset(base);
    DL_SPI_enablePower(base);
    DL_Common_delayCycles(16U);

    DL_SPI_setClockConfig(base, &clock_config);
    DL_SPI_init(base, &spi_config);
    DL_SPI_setBitRateSerialClockDivider(base, serial_clock_divider);
    DL_SPI_setFIFOThreshold(base, DL_SPI_RX_FIFO_LEVEL_ONE_FRAME, DL_SPI_TX_FIFO_LEVEL_ONE_FRAME);

    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);

    DL_SPI_enable(base);
        
    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        (void) DL_SPI_receiveData8(base);
    }

    memset(&spi_xfer[index], 0, sizeof(spi_xfer[index]));
    spi_xfer[index].state = TIMSPM0_SPI_IDLE;
    Spi_Master_Mode[index] = op_mode;

    SPI_NVIC_Init(index);

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_WriteByte(
    hwSPI_Index index,
    uint8_t data)
{
    if ((index >= hwSPI_Index_MAX))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    TIMSPM0_SPI_Regs *base = SPI_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSPM0_SPI_Transfer *transfer = &spi_xfer[index];

    memset(transfer, 0, sizeof(*transfer));

    transfer->state = TIMSPM0_SPI_TX;
    transfer->tx_buf = &data;
    transfer->rx_buf = NULL;
    transfer->len = 1;

    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    
    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        (void) DL_SPI_receiveData8(base);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    /*
     * Prime the FIFO while peripheral interrupts are masked. Any RX/IDLE
     * event that occurs here remains pending and becomes visible when the
     * interrupt mask is enabled below.
     */
    while ((transfer->tx_pos < transfer->len) && !DL_SPI_isTXFIFOFull(base))
    {
        uint8_t value = SPI_TIMSPM0_DUMMY_DATA;

        if (transfer->tx_buf != NULL)
        {
            value = transfer->tx_buf[transfer->tx_pos];
        }

        DL_SPI_transmitData8(base, value);
        transfer->tx_pos++;
    }

    if (transfer->tx_pos >= transfer->len)
    {
        DL_SPI_disableInterrupt(base, DL_SPI_INTERRUPT_TX);
    }

    uint32_t interrupt_mask =
        DL_SPI_INTERRUPT_IDLE |
        DL_SPI_INTERRUPT_RX |
        SPI_TIMSPM0_ERROR_INTERRUPTS;

    if (transfer->tx_pos < transfer->len)
    {
        interrupt_mask |= DL_SPI_INTERRUPT_TX;
    }

    DL_SPI_enableInterrupt(base, interrupt_mask);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (transfer->state != TIMSPM0_SPI_DONE)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_ReadByte(
    hwSPI_Index index,
    uint8_t *data)
{
    if ((index >= hwSPI_Index_MAX))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    TIMSPM0_SPI_Regs *base = SPI_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (data == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSPM0_SPI_Transfer *transfer = &spi_xfer[index];

    memset(transfer, 0, sizeof(*transfer));

    transfer->state = TIMSPM0_SPI_RX;
    transfer->tx_buf = NULL;
    transfer->rx_buf = data;
    transfer->len = 1;

    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    
    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        (void) DL_SPI_receiveData8(base);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    /*
     * Prime the FIFO while peripheral interrupts are masked. Any RX/IDLE
     * event that occurs here remains pending and becomes visible when the
     * interrupt mask is enabled below.
     */
    while ((transfer->tx_pos < transfer->len) && !DL_SPI_isTXFIFOFull(base))
    {
        uint8_t value = SPI_TIMSPM0_DUMMY_DATA;

        if (transfer->tx_buf != NULL)
        {
            value = transfer->tx_buf[transfer->tx_pos];
        }

        DL_SPI_transmitData8(base, value);
        transfer->tx_pos++;
    }

    if (transfer->tx_pos >= transfer->len)
    {
        DL_SPI_disableInterrupt(base, DL_SPI_INTERRUPT_TX);
    }

    uint32_t interrupt_mask =
        DL_SPI_INTERRUPT_IDLE |
        DL_SPI_INTERRUPT_RX |
        SPI_TIMSPM0_ERROR_INTERRUPTS;

    if (transfer->tx_pos < transfer->len)
    {
        interrupt_mask |= DL_SPI_INTERRUPT_TX;
    }

    DL_SPI_enableInterrupt(base, interrupt_mask);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }
    
    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }
    
    if (transfer->state != TIMSPM0_SPI_DONE)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_TransferByte(
    hwSPI_Index index,
    uint8_t write_data,
    uint8_t *read_data)
{
    if (read_data == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if ((index >= hwSPI_Index_MAX))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    TIMSPM0_SPI_Regs *base = SPI_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSPM0_SPI_Transfer *transfer = &spi_xfer[index];

    memset(transfer, 0, sizeof(*transfer));

    transfer->state = TIMSPM0_SPI_TXRX;
    transfer->tx_buf = &write_data;
    transfer->rx_buf = read_data;
    transfer->len = 1;

    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    
    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        (void) DL_SPI_receiveData8(base);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    /*
     * Prime the FIFO while peripheral interrupts are masked. Any RX/IDLE
     * event that occurs here remains pending and becomes visible when the
     * interrupt mask is enabled below.
     */
    while ((transfer->tx_pos < transfer->len) && !DL_SPI_isTXFIFOFull(base))
    {
        uint8_t value = SPI_TIMSPM0_DUMMY_DATA;

        if (transfer->tx_buf != NULL)
        {
            value = transfer->tx_buf[transfer->tx_pos];
        }

        DL_SPI_transmitData8(base, value);
        transfer->tx_pos++;
    }

    if (transfer->tx_pos >= transfer->len)
    {
        DL_SPI_disableInterrupt(base, DL_SPI_INTERRUPT_TX);
    }

    uint32_t interrupt_mask =
        DL_SPI_INTERRUPT_IDLE |
        DL_SPI_INTERRUPT_RX |
        SPI_TIMSPM0_ERROR_INTERRUPTS;

    if (transfer->tx_pos < transfer->len)
    {
        interrupt_mask |= DL_SPI_INTERRUPT_TX;
    }

    DL_SPI_enableInterrupt(base, interrupt_mask);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (transfer->state != TIMSPM0_SPI_DONE)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_DummyByte(hwSPI_Index index)
{
    uint8_t dummy = 0xFF;
    return SPI_Master_WriteByte(index, dummy);
}

hwSPI_OpResult SPI_Master_DummyBytes(hwSPI_Index index, uint32_t len)
{
    if (index >= hwSPI_Index_MAX || len == 0)
    {
        return hwSPI_InvalidParameter;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        hwSPI_OpResult ret = SPI_Master_DummyByte(index);

        if (ret != hwSPI_OK)
        {
            return ret;
        }
    }

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Stream_Write(
    hwSPI_Index index,
    const uint8_t *buf,
    uint16_t len)
{
    if ((index >= hwSPI_Index_MAX))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    TIMSPM0_SPI_Regs *base = SPI_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (buf == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSPM0_SPI_Transfer *transfer = &spi_xfer[index];

    memset(transfer, 0, sizeof(*transfer));

    transfer->state = TIMSPM0_SPI_TX;
    transfer->tx_buf = buf;
    transfer->rx_buf = NULL;
    transfer->len = len;

    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    
    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        (void) DL_SPI_receiveData8(base);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    /*
     * Prime the FIFO while peripheral interrupts are masked. Any RX/IDLE
     * event that occurs here remains pending and becomes visible when the
     * interrupt mask is enabled below.
     */
    while ((transfer->tx_pos < transfer->len) && !DL_SPI_isTXFIFOFull(base))
    {
        uint8_t value = SPI_TIMSPM0_DUMMY_DATA;

        if (transfer->tx_buf != NULL)
        {
            value = transfer->tx_buf[transfer->tx_pos];
        }

        DL_SPI_transmitData8(base, value);
        transfer->tx_pos++;
    }

    if (transfer->tx_pos >= transfer->len)
    {
        DL_SPI_disableInterrupt(base, DL_SPI_INTERRUPT_TX);
    }

    uint32_t interrupt_mask =
        DL_SPI_INTERRUPT_IDLE |
        DL_SPI_INTERRUPT_RX |
        SPI_TIMSPM0_ERROR_INTERRUPTS;

    if (transfer->tx_pos < transfer->len)
    {
        interrupt_mask |= DL_SPI_INTERRUPT_TX;
    }

    DL_SPI_enableInterrupt(base, interrupt_mask);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (transfer->state != TIMSPM0_SPI_DONE)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Stream_Read(
    hwSPI_Index index,
    uint8_t *buf,
    uint16_t len)
{
    if ((index >= hwSPI_Index_MAX))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    TIMSPM0_SPI_Regs *base = SPI_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (buf == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSPM0_SPI_Transfer *transfer = &spi_xfer[index];

    memset(transfer, 0, sizeof(*transfer));

    transfer->state = TIMSPM0_SPI_RX;
    transfer->tx_buf = NULL;
    transfer->rx_buf = buf;
    transfer->len = len;

    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    
    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        (void) DL_SPI_receiveData8(base);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    /*
     * Prime the FIFO while peripheral interrupts are masked. Any RX/IDLE
     * event that occurs here remains pending and becomes visible when the
     * interrupt mask is enabled below.
     */
    while ((transfer->tx_pos < transfer->len) && !DL_SPI_isTXFIFOFull(base))
    {
        uint8_t value = SPI_TIMSPM0_DUMMY_DATA;

        if (transfer->tx_buf != NULL)
        {
            value = transfer->tx_buf[transfer->tx_pos];
        }

        DL_SPI_transmitData8(base, value);
        transfer->tx_pos++;
    }

    if (transfer->tx_pos >= transfer->len)
    {
        DL_SPI_disableInterrupt(base, DL_SPI_INTERRUPT_TX);
    }

    uint32_t interrupt_mask =
        DL_SPI_INTERRUPT_IDLE |
        DL_SPI_INTERRUPT_RX |
        SPI_TIMSPM0_ERROR_INTERRUPTS;

    if (transfer->tx_pos < transfer->len)
    {
        interrupt_mask |= DL_SPI_INTERRUPT_TX;
    }

    DL_SPI_enableInterrupt(base, interrupt_mask);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (transfer->state != TIMSPM0_SPI_DONE)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Stream_Transfer(
    hwSPI_Index index,
    const uint8_t *tx_buf,
    uint8_t *rx_buf,
    uint16_t len)
{
    if ((index >= hwSPI_Index_MAX))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    TIMSPM0_SPI_Regs *base = SPI_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if ((tx_buf == NULL) || (rx_buf == NULL))
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSPM0_SPI_Transfer *transfer = &spi_xfer[index];

    memset(transfer, 0, sizeof(*transfer));

    transfer->state = TIMSPM0_SPI_TXRX;
    transfer->tx_buf = tx_buf;
    transfer->rx_buf = rx_buf;
    transfer->len = len;

    DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    DL_SPI_clearInterruptStatus(base, SPI_TIMSPM0_ALL_INTERRUPTS);
    
    while (!DL_SPI_isRXFIFOEmpty(base))
    {
        (void) DL_SPI_receiveData8(base);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    /*
     * Prime the FIFO while peripheral interrupts are masked. Any RX/IDLE
     * event that occurs here remains pending and becomes visible when the
     * interrupt mask is enabled below.
     */
    while ((transfer->tx_pos < transfer->len) && !DL_SPI_isTXFIFOFull(base))
    {
        uint8_t value = SPI_TIMSPM0_DUMMY_DATA;

        if (transfer->tx_buf != NULL)
        {
            value = transfer->tx_buf[transfer->tx_pos];
        }

        DL_SPI_transmitData8(base, value);
        transfer->tx_pos++;
    }

    if (transfer->tx_pos >= transfer->len)
    {
        DL_SPI_disableInterrupt(base, DL_SPI_INTERRUPT_TX);
    }

    uint32_t interrupt_mask =
        DL_SPI_INTERRUPT_IDLE |
        DL_SPI_INTERRUPT_RX |
        SPI_TIMSPM0_ERROR_INTERRUPTS;

    if (transfer->tx_pos < transfer->len)
    {
        interrupt_mask |= DL_SPI_INTERRUPT_TX;
    }

    DL_SPI_enableInterrupt(base, interrupt_mask);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_SlaveTimeout;
    }

    if (transfer->state != TIMSPM0_SPI_DONE)
    {
        DL_SPI_disableInterrupt(base, SPI_TIMSPM0_ALL_INTERRUPTS);
        transfer->state = TIMSPM0_SPI_ERROR;

        while (!DL_SPI_isRXFIFOEmpty(base))
        {
            (void) DL_SPI_receiveData8(base);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Burst_Write(
    hwSPI_Index index,
    uint8_t *buf,
    uint32_t size)
{
    hwDMA_OpResult dma_op_status;

    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    if ((size != 0U) && (buf == NULL))
    {
        return hwSPI_InvalidParameter;
    }

    if (size == 0U)
    {
        return hwSPI_OK;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    dma_op_status = DMA_SPI_Write(index, buf, size);

    if (dma_op_status < hwDMA_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return SPI_Map_DMA_HW_Error_Code(dma_op_status);
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Burst_Read(
    hwSPI_Index index,
    uint8_t *buf,
    uint32_t size)
{
    hwDMA_OpResult dma_op_status;

    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    if ((size != 0U) && (buf == NULL))
    {
        return hwSPI_InvalidParameter;
    }

    if (size == 0U)
    {
        return hwSPI_OK;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    dma_op_status = DMA_SPI_Read(index, buf, size);

    if (dma_op_status < hwDMA_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return SPI_Map_DMA_HW_Error_Code(dma_op_status);
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

#endif /* DEVICE_TIMSPM0 */
