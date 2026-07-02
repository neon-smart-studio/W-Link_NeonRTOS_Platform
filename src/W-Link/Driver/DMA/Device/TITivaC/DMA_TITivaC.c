
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#ifdef DEVICE_TITIVAC

#include "DMA_TITivaC.h"

#include "DMA_TITivaC_Index.h"

bool DMA_Channel_Init_Status[hwDMA_Channel_Index_MAX] = {false};
NeonRTOS_LockObj_t DMA_Channel_Mutex[hwDMA_Channel_Index_MAX] = {NULL};

hwDMA_OpResult DMA_Init(void)
{
    DMA_Clock_Enable();

    for (hwDMA_Channel_Index i = 0; i < hwDMA_Channel_Index_MAX; i++)
    {
        if (NeonRTOS_LockObjCreate(&DMA_Channel_Mutex[i]) != NeonRTOS_OK)
        {
            return hwDMA_MemoryError;
        }

        DMA_Channel_Init_Status[i] = true;
    }

    DMA_HW_Init();

    return hwDMA_OK;
}

hwDMA_OpResult DMA_DeInit(void)
{
    for (hwDMA_Channel_Index i = 0; i < hwDMA_Channel_Index_MAX; i++)
    {
        if (DMA_Channel_Init_Status[i])
        {
            NeonRTOS_LockObjDelete(&DMA_Channel_Mutex[i]);
            DMA_Channel_Init_Status[i] = false;
        }
    }

    DMA_HW_DeInit();
    DMA_Clock_Disable();

    return hwDMA_OK;
}

hwDMA_OpResult DMA_Uart_Tx(hwUART_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_UART(index, hwDMA_Peripheral_Direction_TX, buf, len);
}

hwDMA_OpResult DMA_Uart_Rx(hwUART_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_UART(index, hwDMA_Peripheral_Direction_RX, buf, len);
}

#if defined(TM4C1294)
hwDMA_OpResult DMA_I2C_Write(hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len)
{
    return DMA_Xfer_I2C(index, hwDMA_Peripheral_Direction_TX, dev_addr, buf, len);
}

hwDMA_OpResult DMA_I2C_Read(hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len)
{
    return DMA_Xfer_I2C(index, hwDMA_Peripheral_Direction_RX, dev_addr, buf, len);
}
#endif

hwDMA_OpResult DMA_SPI_Write(hwSPI_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_SPI(index, hwDMA_Peripheral_Direction_TX, buf, len);
}

hwDMA_OpResult DMA_SPI_Read(hwSPI_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_SPI(index, hwDMA_Peripheral_Direction_RX, buf, len);
}

hwDMA_OpResult DMA_QSPI_Write(hwQSPI_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_QSPI(index, hwDMA_Peripheral_Direction_TX, buf, len);
}

hwDMA_OpResult DMA_QSPI_Read(hwQSPI_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_QSPI(index, hwDMA_Peripheral_Direction_RX, buf, len);
}

#endif //DEVICE_TITIVAC
