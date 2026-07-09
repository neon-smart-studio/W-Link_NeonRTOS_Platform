#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#ifdef DEVICE_TIMSP432P

#include "DMA_TIMSP432.h"
#include "DMA_TIMSP432_Index.h"

#include "UART/Device/TIMSP432/UART_TIMSP432.h"
#include "SPI/Device/TIMSP432/SPI_Master_TIMSP432.h"
#include "I2C/Device/TIMSP432/I2C_Master_TIMSP432.h"

#define DMA_WAIT_ALLOCATED_TIMEOUT  1000
#define DMA_WAIT_TRANSFER_TIMEOUT   1000

#define DMA_CHANNEL_LOCK(channel_index)                                      \
    if (NeonRTOS_LockObjLock(&DMA_Channel_Mutex[(channel_index)],            \
                             DMA_WAIT_ALLOCATED_TIMEOUT) != NeonRTOS_OK)     \
    {                                                                        \
        return hwDMA_MutexTimeout;                                           \
    }

#define DMA_CHANNEL_UNLOCK(channel_index)                                    \
    if (NeonRTOS_LockObjUnlock(&DMA_Channel_Mutex[(channel_index)])          \
        != NeonRTOS_OK)                                                      \
    {                                                                        \
        return hwDMA_MutexTimeout;                                           \
    }

#ifndef DMA_CONTROL_TABLE_SIZE
#define DMA_CONTROL_TABLE_SIZE 1024
#endif

static uint8_t g_dma_control_table[DMA_CONTROL_TABLE_SIZE]
    __attribute__((aligned(1024)));

#ifndef EUSCI_RXBUF_OFFSET
#define EUSCI_RXBUF_OFFSET 0x0C
#endif

#ifndef EUSCI_TXBUF_OFFSET
#define EUSCI_TXBUF_OFFSET 0x0E
#endif

typedef struct
{
    uint32_t dma_assign;
    uint32_t dma_channel;
    hwDMA_Channel_Index channel_index;
} TIMSP432P_DMA_ChannelDef;

static const TIMSP432P_DMA_ChannelDef UART_DMA_Channel_Map
    [hwUART_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(EUSCI_A0_BASE)
    [hwUART_Index_0] =
    {
        [hwDMA_Peripheral_Direction_TX] =
        {
            DMA_CH0_EUSCIA0TX,
            DMA_CHANNEL_0,
            hwDMA_Channel_Index_0
        },
        [hwDMA_Peripheral_Direction_RX] =
        {
            DMA_CH1_EUSCIA0RX,
            DMA_CHANNEL_1,
            hwDMA_Channel_Index_1
        },
    },
#endif

#if defined(EUSCI_A1_BASE)
    [hwUART_Index_1] =
    {
        [hwDMA_Peripheral_Direction_TX] =
        {
            DMA_CH2_EUSCIA1TX,
            DMA_CHANNEL_2,
            hwDMA_Channel_Index_2
        },
        [hwDMA_Peripheral_Direction_RX] =
        {
            DMA_CH3_EUSCIA1RX,
            DMA_CHANNEL_3,
            hwDMA_Channel_Index_3
        },
    },
#endif

#if defined(EUSCI_A2_BASE)
    [hwUART_Index_2] =
    {
        [hwDMA_Peripheral_Direction_TX] =
        {
            DMA_CH4_EUSCIA2TX,
            DMA_CHANNEL_4,
            hwDMA_Channel_Index_4
        },
        [hwDMA_Peripheral_Direction_RX] =
        {
            DMA_CH5_EUSCIA2RX,
            DMA_CHANNEL_5,
            hwDMA_Channel_Index_5
        },
    },
#endif

#if defined(EUSCI_A3_BASE)
    [hwUART_Index_3] =
    {
        [hwDMA_Peripheral_Direction_TX] =
        {
            DMA_CH6_EUSCIA3TX,
            DMA_CHANNEL_6,
            hwDMA_Channel_Index_6
        },
        [hwDMA_Peripheral_Direction_RX] =
        {
            DMA_CH7_EUSCIA3RX,
            DMA_CHANNEL_7,
            hwDMA_Channel_Index_7
        },
    },
#endif
};

static const TIMSP432P_DMA_ChannelDef I2C_DMA_Channel_Map
    [hwI2C_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(EUSCI_B0_BASE)
    [hwI2C_Index_0] =
    {
        { DMA_CH0_EUSCIB0TX0, DMA_CHANNEL_0, hwDMA_Channel_Index_0 },
        { DMA_CH1_EUSCIB0RX0, DMA_CHANNEL_1, hwDMA_Channel_Index_1 },
    },
#endif

#if defined(EUSCI_B1_BASE)
    [hwI2C_Index_1] =
    {
        { DMA_CH2_EUSCIB1TX0, DMA_CHANNEL_2, hwDMA_Channel_Index_2 },
        { DMA_CH3_EUSCIB1RX0, DMA_CHANNEL_3, hwDMA_Channel_Index_3 },
    },
#endif

#if defined(EUSCI_B2_BASE)
    [hwI2C_Index_2] =
    {
        { DMA_CH4_EUSCIB2TX0, DMA_CHANNEL_4, hwDMA_Channel_Index_4 },
        { DMA_CH5_EUSCIB2RX0, DMA_CHANNEL_5, hwDMA_Channel_Index_5 },
    },
#endif

#if defined(EUSCI_B3_BASE)
    [hwI2C_Index_3] =
    {
        { DMA_CH6_EUSCIB3TX0, DMA_CHANNEL_6, hwDMA_Channel_Index_6 },
        { DMA_CH7_EUSCIB3RX0, DMA_CHANNEL_7, hwDMA_Channel_Index_7 },
    },
#endif
};

static const TIMSP432P_DMA_ChannelDef SPI_DMA_Channel_Map
    [hwSPI_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(EUSCI_B0_BASE)
    [hwSPI_Index_0] =
    {
        [hwDMA_Peripheral_Direction_TX] =
        {
            DMA_CH0_EUSCIB0TX0,
            DMA_CHANNEL_0,
            hwDMA_Channel_Index_0
        },
        [hwDMA_Peripheral_Direction_RX] =
        {
            DMA_CH1_EUSCIB0RX0,
            DMA_CHANNEL_1,
            hwDMA_Channel_Index_1
        },
    },
#endif

#if defined(EUSCI_B1_BASE)
    [hwSPI_Index_1] =
    {
        [hwDMA_Peripheral_Direction_TX] =
        {
            DMA_CH2_EUSCIB1TX0,
            DMA_CHANNEL_2,
            hwDMA_Channel_Index_2
        },
        [hwDMA_Peripheral_Direction_RX] =
        {
            DMA_CH3_EUSCIB1RX0,
            DMA_CHANNEL_3,
            hwDMA_Channel_Index_3
        },
    },
#endif

#if defined(EUSCI_B2_BASE)
    [hwSPI_Index_2] =
    {
        [hwDMA_Peripheral_Direction_TX] =
        {
            DMA_CH4_EUSCIB2TX0,
            DMA_CHANNEL_4,
            hwDMA_Channel_Index_4
        },
        [hwDMA_Peripheral_Direction_RX] =
        {
            DMA_CH5_EUSCIB2RX0,
            DMA_CHANNEL_5,
            hwDMA_Channel_Index_5
        },
    },
#endif

#if defined(EUSCI_B3_BASE)
    [hwSPI_Index_3] =
    {
        [hwDMA_Peripheral_Direction_TX] =
        {
            DMA_CH6_EUSCIB3TX0,
            DMA_CHANNEL_6,
            hwDMA_Channel_Index_6
        },
        [hwDMA_Peripheral_Direction_RX] =
        {
            DMA_CH7_EUSCIB3RX0,
            DMA_CHANNEL_7,
            hwDMA_Channel_Index_7
        },
    },
#endif
};

static uint32_t DMA_Map_UART_Base(hwUART_Index index)
{
    switch (index)
    {
#if defined(EUSCI_A0_BASE)
        case hwUART_Index_0: return EUSCI_A0_BASE;
#endif
#if defined(EUSCI_A1_BASE)
        case hwUART_Index_1: return EUSCI_A1_BASE;
#endif
#if defined(EUSCI_A2_BASE)
        case hwUART_Index_2: return EUSCI_A2_BASE;
#endif
#if defined(EUSCI_A3_BASE)
        case hwUART_Index_3: return EUSCI_A3_BASE;
#endif
        default: return 0;
    }
}

static uint32_t DMA_Map_I2C_Base(hwI2C_Index index)
{
    switch (index)
    {
#if defined(EUSCI_B0_BASE)
        case hwI2C_Index_0: return EUSCI_B0_BASE;
#endif
#if defined(EUSCI_B1_BASE)
        case hwI2C_Index_1: return EUSCI_B1_BASE;
#endif
#if defined(EUSCI_B2_BASE)
        case hwI2C_Index_2: return EUSCI_B2_BASE;
#endif
#if defined(EUSCI_B3_BASE)
        case hwI2C_Index_3: return EUSCI_B3_BASE;
#endif
        default: return 0;
    }
}

static uint32_t DMA_Map_SPI_Base(hwSPI_Index index)
{
    switch (index)
    {
#if defined(EUSCI_B0_BASE)
        case hwSPI_Index_0: return EUSCI_B0_BASE;
#endif
#if defined(EUSCI_B1_BASE)
        case hwSPI_Index_1: return EUSCI_B1_BASE;
#endif
#if defined(EUSCI_B2_BASE)
        case hwSPI_Index_2: return EUSCI_B2_BASE;
#endif
#if defined(EUSCI_B3_BASE)
        case hwSPI_Index_3: return EUSCI_B3_BASE;
#endif
        default: return 0;
    }
}

static bool DMA_Is_Valid_Channel(hwDMA_Channel_Index channel_index)
{
    return (channel_index < hwDMA_Channel_Index_MAX);
}

static uint32_t DMA_Get_RXBUF_Address(uint32_t base)
{
    return base + EUSCI_RXBUF_OFFSET;
}

static uint32_t DMA_Get_TXBUF_Address(uint32_t base)
{
    return base + EUSCI_TXBUF_OFFSET;
}

void DMA_Clock_Enable(void)
{
    /*
     * MSP432P DMA 沒有像 TM4C 那種 SYSCTL_PERIPH_UDMA clock gate。
     * enableModule 即可。
     */
    MAP_DMA_enableModule();
}

void DMA_Clock_Disable(void)
{
    MAP_DMA_disableModule();
}

hwDMA_OpResult DMA_HW_Init(void)
{
    MAP_DMA_enableModule();
    MAP_DMA_setControlBase(g_dma_control_table);

    return hwDMA_OK;
}

hwDMA_OpResult DMA_HW_DeInit(void)
{
    MAP_DMA_disableModule();

    return hwDMA_OK;
}

static hwDMA_OpResult DMA_Wait_Channel_Done(uint32_t dma_channel)
{
    uint32_t timeout = DMA_WAIT_TRANSFER_TIMEOUT;

    while (MAP_DMA_getChannelMode(dma_channel | UDMA_PRI_SELECT)
           != UDMA_MODE_STOP)
    {
        if (timeout-- == 0)
        {
            MAP_DMA_disableChannel(dma_channel);
            return hwDMA_XferTimeout;
        }

        NeonRTOS_DelayMs(1);
    }

    return hwDMA_OK;
}

static void DMA_Prepare_Channel(uint32_t dma_channel)
{
    MAP_DMA_assignChannel(dma_channel);

    MAP_DMA_disableChannelAttribute(
        dma_channel,
        UDMA_ATTR_ALTSELECT |
        UDMA_ATTR_USEBURST |
        UDMA_ATTR_HIGH_PRIORITY |
        UDMA_ATTR_REQMASK
    );
}

hwDMA_OpResult DMA_Xfer_UART(hwUART_Index index,
                             hwDMA_Peripheral_Direction dir,
                             uint8_t *buf,
                             size_t len)
{
    if ((buf == NULL) || (len == 0))
    {
        return hwDMA_InvalidParameter;
    }

    if (index >= hwUART_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    if (dir >= hwDMA_Peripheral_Direction_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    uint32_t uart_base = DMA_Map_UART_Base(index);
    if (uart_base == 0)
    {
        return hwDMA_InvalidParameter;
    }

    TIMSP432P_DMA_ChannelDef dma_def = UART_DMA_Channel_Map[index][dir];

    if (!DMA_Is_Valid_Channel(dma_def.channel_index))
    {
        return hwDMA_InvalidParameter;
    }

    uint32_t dma_channel = dma_def.dma_channel;

    DMA_CHANNEL_LOCK(dma_def.channel_index);

    DMA_Prepare_Channel(dma_channel);

    if (dir == hwDMA_Peripheral_Direction_TX)
    {
        MAP_DMA_setChannelControl(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_8 |
            UDMA_DST_INC_NONE |
            UDMA_ARB_1
        );

        MAP_DMA_setChannelTransfer(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            buf,
            (void *)DMA_Get_TXBUF_Address(uart_base),
            len
        );

        MAP_DMA_enableChannel(dma_channel);
    }
    else if (dir == hwDMA_Peripheral_Direction_RX)
    {
        MAP_DMA_setChannelControl(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_NONE |
            UDMA_DST_INC_8 |
            UDMA_ARB_1
        );

        MAP_DMA_setChannelTransfer(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            (void *)DMA_Get_RXBUF_Address(uart_base),
            buf,
            len
        );

        MAP_DMA_enableChannel(dma_channel);
    }
    else
    {
        DMA_CHANNEL_UNLOCK(dma_def.channel_index);
        return hwDMA_InvalidParameter;
    }

    hwDMA_OpResult status = DMA_Wait_Channel_Done(dma_channel);

    MAP_DMA_disableChannel(dma_channel);

    DMA_CHANNEL_UNLOCK(dma_def.channel_index);

    return status;
}

hwDMA_OpResult DMA_Xfer_I2C(hwI2C_Index index,
                            hwDMA_Peripheral_Direction dir,
                            uint16_t dev_addr,
                            uint8_t *buf,
                            size_t len)
{
    if ((buf == NULL) || (len == 0))
    {
        return hwDMA_InvalidParameter;
    }

    if (index >= hwI2C_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    if (dir >= hwDMA_Peripheral_Direction_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    if (len > 1024)
    {
        return hwDMA_InvalidParameter;
    }

    uint32_t i2c_base = DMA_Map_I2C_Base(index);
    if (i2c_base == 0)
    {
        return hwDMA_InvalidParameter;
    }

    TIMSP432P_DMA_ChannelDef dma_def = I2C_DMA_Channel_Map[index][dir];

    if (!DMA_Is_Valid_Channel(dma_def.channel_index))
    {
        return hwDMA_InvalidParameter;
    }

    uint32_t dma_channel = dma_def.dma_channel;

    DMA_CHANNEL_LOCK(dma_def.channel_index);

    MAP_DMA_assignChannel(dma_def.dma_assign);

    MAP_DMA_disableChannelAttribute(
        dma_channel,
        UDMA_ATTR_ALTSELECT |
        UDMA_ATTR_USEBURST |
        UDMA_ATTR_HIGH_PRIORITY |
        UDMA_ATTR_REQMASK
    );

    MAP_I2C_setSlaveAddress(i2c_base, dev_addr);

    if (dir == hwDMA_Peripheral_Direction_TX)
    {
        MAP_DMA_setChannelControl(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_8 |
            UDMA_DST_INC_NONE |
            UDMA_ARB_1
        );

        MAP_DMA_setChannelTransfer(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            buf,
            (void *)DMA_Get_TXBUF_Address(i2c_base),
            len
        );

        MAP_DMA_enableChannel(dma_channel);

        MAP_I2C_masterSendMultiByteStart(i2c_base, buf[0]);

        hwDMA_OpResult status = DMA_Wait_Channel_Done(dma_channel);

        MAP_I2C_masterSendMultiByteStop(i2c_base);

        MAP_DMA_disableChannel(dma_channel);

        DMA_CHANNEL_UNLOCK(dma_def.channel_index);

        return status;
    }
    else if (dir == hwDMA_Peripheral_Direction_RX)
    {
        MAP_DMA_setChannelControl(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_NONE |
            UDMA_DST_INC_8 |
            UDMA_ARB_1
        );

        MAP_DMA_setChannelTransfer(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            (void *)DMA_Get_RXBUF_Address(i2c_base),
            buf,
            len
        );

        MAP_DMA_enableChannel(dma_channel);

        MAP_I2C_masterReceiveStart(i2c_base);

        hwDMA_OpResult status = DMA_Wait_Channel_Done(dma_channel);

        MAP_I2C_masterReceiveMultiByteStop(i2c_base);

        MAP_DMA_disableChannel(dma_channel);

        DMA_CHANNEL_UNLOCK(dma_def.channel_index);

        return status;
    }

    MAP_DMA_disableChannel(dma_channel);

    DMA_CHANNEL_UNLOCK(dma_def.channel_index);

    return hwDMA_InvalidParameter;
}

hwDMA_OpResult DMA_Xfer_SPI(hwSPI_Index index,
                            hwDMA_Peripheral_Direction dir,
                            uint8_t *buf,
                            size_t len)
{
    if ((buf == NULL) || (len == 0))
    {
        return hwDMA_InvalidParameter;
    }

    if (index >= hwSPI_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    if (dir >= hwDMA_Peripheral_Direction_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    uint32_t spi_base = DMA_Map_SPI_Base(index);
    if (spi_base == 0)
    {
        return hwDMA_InvalidParameter;
    }

    TIMSP432P_DMA_ChannelDef dma_def = SPI_DMA_Channel_Map[index][dir];

    if (!DMA_Is_Valid_Channel(dma_def.channel_index))
    {
        return hwDMA_InvalidParameter;
    }

    uint32_t dma_channel = dma_def.dma_channel;

    DMA_CHANNEL_LOCK(dma_def.channel_index);

    DMA_Prepare_Channel(dma_channel);

    if (dir == hwDMA_Peripheral_Direction_TX)
    {
        MAP_DMA_setChannelControl(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_8 |
            UDMA_DST_INC_NONE |
            UDMA_ARB_1
        );

        MAP_DMA_setChannelTransfer(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            buf,
            (void *)DMA_Get_TXBUF_Address(spi_base),
            len
        );

        MAP_DMA_enableChannel(dma_channel);
    }
    else if (dir == hwDMA_Peripheral_Direction_RX)
    {
        MAP_DMA_setChannelControl(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_NONE |
            UDMA_DST_INC_8 |
            UDMA_ARB_1
        );

        MAP_DMA_setChannelTransfer(
            dma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            (void *)DMA_Get_RXBUF_Address(spi_base),
            buf,
            len
        );

        MAP_DMA_enableChannel(dma_channel);
    }
    else
    {
        DMA_CHANNEL_UNLOCK(dma_def.channel_index);
        return hwDMA_InvalidParameter;
    }

    hwDMA_OpResult status = DMA_Wait_Channel_Done(dma_channel);

    MAP_DMA_disableChannel(dma_channel);

    DMA_CHANNEL_UNLOCK(dma_def.channel_index);

    return status;
}

#endif // DEVICE_TIMSP432P