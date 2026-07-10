#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#if defined(TM4C123)

#include "DMA_TITivaC.h"
#include "DMA_TITivaC_Index.h"

#include "UART/Device/TITivaC/UART_TITivaC.h"
#include "SPI/Device/TITivaC/SPI_Master_TITivaC.h"
#include "I2C/Device/TITivaC/I2C_Master_TITivaC.h"

#define DMA_WAIT_ALLOCATED_TIMEOUT  1000
#define DMA_WAIT_TRANSFER_TIMEOUT   1000

#define DMA_CHANNEL_LOCK(channel_index) if (NeonRTOS_LockObjLock(&DMA_Channel_Mutex[(channel_index)], DMA_WAIT_ALLOCATED_TIMEOUT) != NeonRTOS_OK) { return hwDMA_MutexTimeout; }

#define DMA_CHANNEL_UNLOCK(channel_index) if (NeonRTOS_LockObjUnlock(&DMA_Channel_Mutex[(channel_index)]) != NeonRTOS_OK) { return hwDMA_MutexTimeout; }

#ifndef UDMA_CONTROL_TABLE_SIZE
#define UDMA_CONTROL_TABLE_SIZE 1024
#endif

static uint8_t g_udma_control_table[UDMA_CONTROL_TABLE_SIZE] __attribute__((aligned(1024)));

static const uint32_t UART_DMA_Channel_Map[hwUART_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(UART0_BASE)
    { UDMA_CH9_UART0TX,  UDMA_CH8_UART0RX  },
#endif
#if defined(UART1_BASE)
    { UDMA_CH23_UART1TX, UDMA_CH22_UART1RX },
#endif
#if defined(UART2_BASE)
    { UDMA_CH13_UART2TX, UDMA_CH12_UART2RX },
#endif
#if defined(UART3_BASE)
    { UDMA_CH17_UART3TX, UDMA_CH16_UART3RX },
#endif
#if defined(UART4_BASE)
    { UDMA_CH19_UART4TX, UDMA_CH18_UART4RX },
#endif
#if defined(UART5_BASE)
    { UDMA_CH7_UART5TX,  UDMA_CH6_UART5RX  },
#endif
#if defined(UART6_BASE)
    { UDMA_CH11_UART6TX, UDMA_CH10_UART6RX },
#endif
#if defined(UART7_BASE)
    { UDMA_CH21_UART7TX, UDMA_CH20_UART7RX },
#endif
};

static const uint32_t SPI_DMA_Channel_Map[hwSPI_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(SSI0_BASE)
    { UDMA_CH11_SSI0TX, UDMA_CH10_SSI0RX },
#endif
#if defined(SSI1_BASE)
    { UDMA_CH25_SSI1TX, UDMA_CH24_SSI1RX },
#endif
#if defined(SSI2_BASE)
    { UDMA_CH13_SSI2TX, UDMA_CH12_SSI2RX },
#endif
#if defined(SSI3_BASE)
    { UDMA_CH15_SSI3TX, UDMA_CH14_SSI3RX },
#endif
};

static uint32_t DMA_Map_UART_Base(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0: return UART0_BASE;
#endif
#if defined(UART1_BASE)
        case hwUART_Index_1: return UART1_BASE;
#endif
#if defined(UART2_BASE)
        case hwUART_Index_2: return UART2_BASE;
#endif
#if defined(UART3_BASE)
        case hwUART_Index_3: return UART3_BASE;
#endif
#if defined(UART4_BASE)
        case hwUART_Index_4: return UART4_BASE;
#endif
#if defined(UART5_BASE)
        case hwUART_Index_5: return UART5_BASE;
#endif
#if defined(UART6_BASE)
        case hwUART_Index_6: return UART6_BASE;
#endif
#if defined(UART7_BASE)
        case hwUART_Index_7: return UART7_BASE;
#endif
        default: return 0;
    }
}

static uint32_t DMA_Map_SPI_Base(hwSPI_Index index)
{
    switch (index)
    {
#if defined(SSI0_BASE)
        case hwSPI_Index_0: return SSI0_BASE;
#endif
#if defined(SSI1_BASE)
        case hwSPI_Index_1: return SSI1_BASE;
#endif
#if defined(SSI2_BASE)
        case hwSPI_Index_2: return SSI2_BASE;
#endif
#if defined(SSI3_BASE)
        case hwSPI_Index_3: return SSI3_BASE;
#endif
        default: return 0;
    }
}

static hwDMA_Channel_Index DMA_Get_Channel_Index(uint32_t udma_assign)
{
    return (hwDMA_Channel_Index)(udma_assign & 0x1FU);
}

void DMA_Clock_Enable(void)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA))
    {
    }
}

void DMA_Clock_Disable(void)
{
    MAP_uDMADisable();
    MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_UDMA);
}

hwDMA_OpResult DMA_HW_Init(void)
{
    MAP_uDMAEnable();
    MAP_uDMAControlBaseSet(g_udma_control_table);

    return hwDMA_OK;
}

hwDMA_OpResult DMA_HW_DeInit(void)
{
    MAP_uDMADisable();

    return hwDMA_OK;
}

static hwDMA_OpResult DMA_Wait_Channel_Done(uint32_t udma_channel)
{
    uint32_t timeout = DMA_WAIT_TRANSFER_TIMEOUT;

    while (MAP_uDMAChannelIsEnabled(udma_channel))
    {
        if (timeout-- == 0)
        {
            return hwDMA_XferTimeout;
        }

        NeonRTOS_DelayMs(1);
    }

    return hwDMA_OK;
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

    uint32_t udma_channel = UART_DMA_Channel_Map[index][dir];

    hwDMA_Channel_Index channel_index = DMA_Get_Channel_Index(udma_channel);
    if (channel_index >= hwDMA_Channel_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    DMA_CHANNEL_LOCK(channel_index);

    MAP_uDMAChannelAssign(udma_channel);
    MAP_uDMAChannelAttributeDisable(
        udma_channel,
        UDMA_ATTR_ALTSELECT |
        UDMA_ATTR_USEBURST |
        UDMA_ATTR_HIGH_PRIORITY |
        UDMA_ATTR_REQMASK
    );

    if (dir == hwDMA_Peripheral_Direction_TX)
    {
        MAP_uDMAChannelControlSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_8 |
            UDMA_DST_INC_NONE |
            UDMA_ARB_4
        );

        MAP_uDMAChannelTransferSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            buf,
            (void *)(uart_base + UART_O_DR),
            len
        );

        MAP_UARTDMAEnable(uart_base, UART_DMA_TX);
        MAP_uDMAChannelEnable(udma_channel);
    }
    if (dir == hwDMA_Peripheral_Direction_RX)
    {
        MAP_uDMAChannelControlSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_NONE |
            UDMA_DST_INC_8 |
            UDMA_ARB_4
        );

        MAP_uDMAChannelTransferSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            (void *)(uart_base + UART_O_DR),
            buf,
            len
        );

        MAP_UARTDMAEnable(uart_base, UART_DMA_RX);
        MAP_uDMAChannelEnable(udma_channel);
    }

    hwDMA_OpResult status = DMA_Wait_Channel_Done(udma_channel);

    if (dir == hwDMA_Peripheral_Direction_TX)
    {
        MAP_UARTDMADisable(uart_base, UART_DMA_TX);
    }
    if (dir == hwDMA_Peripheral_Direction_RX)
    {
        MAP_UARTDMADisable(uart_base, UART_DMA_RX);
    }

    DMA_CHANNEL_UNLOCK(channel_index);

    return status;
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

    uint32_t ssi_base = DMA_Map_SPI_Base(index);
    if (ssi_base == 0)
    {
        return hwDMA_InvalidParameter;
    }

    uint32_t udma_channel = SPI_DMA_Channel_Map[index][dir];

    hwDMA_Channel_Index channel_index = DMA_Get_Channel_Index(udma_channel);
    if (channel_index >= hwDMA_Channel_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    DMA_CHANNEL_LOCK(channel_index);

    MAP_uDMAChannelAssign(udma_channel);
    MAP_uDMAChannelAttributeDisable(
        udma_channel,
        UDMA_ATTR_ALTSELECT |
        UDMA_ATTR_USEBURST |
        UDMA_ATTR_HIGH_PRIORITY |
        UDMA_ATTR_REQMASK
    );

    if (dir == hwDMA_Peripheral_Direction_TX)
    {
        MAP_uDMAChannelControlSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_8 |
            UDMA_DST_INC_NONE |
            UDMA_ARB_4
        );

        MAP_uDMAChannelTransferSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            buf,
            (void *)(ssi_base + SSI_O_DR),
            len
        );

        MAP_SSIDMAEnable(ssi_base, SSI_DMA_TX);
        MAP_uDMAChannelEnable(udma_channel);
    }
    if (dir == hwDMA_Peripheral_Direction_RX)
    {
        MAP_uDMAChannelControlSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_NONE |
            UDMA_DST_INC_8 |
            UDMA_ARB_4
        );

        MAP_uDMAChannelTransferSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            (void *)(ssi_base + SSI_O_DR),
            buf,
            len
        );

        MAP_SSIDMAEnable(ssi_base, SSI_DMA_RX);
        MAP_uDMAChannelEnable(udma_channel);
    }

    hwDMA_OpResult status = DMA_Wait_Channel_Done(udma_channel);

    if (dir == hwDMA_Peripheral_Direction_TX)
    {
        MAP_SSIDMADisable(ssi_base, SSI_DMA_TX);
    }
    if (dir == hwDMA_Peripheral_Direction_RX)
    {
        MAP_SSIDMADisable(ssi_base, SSI_DMA_RX);
    }

    DMA_CHANNEL_UNLOCK(channel_index);

    return status;
}

hwDMA_OpResult DMA_Xfer_QSPI(hwQSPI_Index index,
                            hwDMA_Peripheral_Direction dir,
                            uint8_t *buf,
                            size_t len)
{
    if ((buf == NULL) || (len == 0))
    {
        return hwDMA_InvalidParameter;
    }

    if (index >= hwQSPI_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    if (dir >= hwDMA_Peripheral_Direction_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    uint32_t ssi_base = DMA_Map_SPI_Base(index);
    if (ssi_base == 0)
    {
        return hwDMA_InvalidParameter;
    }

    uint32_t udma_channel = SPI_DMA_Channel_Map[index][dir];

    hwDMA_Channel_Index channel_index = DMA_Get_Channel_Index(udma_channel);
    if (channel_index >= hwDMA_Channel_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    DMA_CHANNEL_LOCK(channel_index);

    MAP_uDMAChannelAssign(udma_channel);
    MAP_uDMAChannelAttributeDisable(
        udma_channel,
        UDMA_ATTR_ALTSELECT |
        UDMA_ATTR_USEBURST |
        UDMA_ATTR_HIGH_PRIORITY |
        UDMA_ATTR_REQMASK
    );

    if (dir == hwDMA_Peripheral_Direction_TX)
    {
        MAP_uDMAChannelControlSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_8 |
            UDMA_DST_INC_NONE |
            UDMA_ARB_4
        );

        MAP_uDMAChannelTransferSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            buf,
            (void *)(ssi_base + SSI_O_DR),
            len
        );

        MAP_SSIDMAEnable(ssi_base, SSI_DMA_TX);
        MAP_uDMAChannelEnable(udma_channel);
    }
    if (dir == hwDMA_Peripheral_Direction_RX)
    {
        MAP_uDMAChannelControlSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_SIZE_8 |
            UDMA_SRC_INC_NONE |
            UDMA_DST_INC_8 |
            UDMA_ARB_4
        );

        MAP_uDMAChannelTransferSet(
            udma_channel | UDMA_PRI_SELECT,
            UDMA_MODE_BASIC,
            (void *)(ssi_base + SSI_O_DR),
            buf,
            len
        );

        MAP_SSIDMAEnable(ssi_base, SSI_DMA_RX);
        MAP_uDMAChannelEnable(udma_channel);
    }

    hwDMA_OpResult status = DMA_Wait_Channel_Done(udma_channel);

    if (dir == hwDMA_Peripheral_Direction_TX)
    {
        MAP_SSIDMADisable(ssi_base, SSI_DMA_TX);
    }
    if (dir == hwDMA_Peripheral_Direction_RX)
    {
        MAP_SSIDMADisable(ssi_base, SSI_DMA_RX);
    }

    DMA_CHANNEL_UNLOCK(channel_index);

    return status;
}

#endif // TM4C123