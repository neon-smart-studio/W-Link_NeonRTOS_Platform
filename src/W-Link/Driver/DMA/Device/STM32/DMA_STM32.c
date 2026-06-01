
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#ifdef DEVICE_STM32

#include "DMA_STM32.h"

#include "DMA_STM32_Index.h"

#define DMA_IRQ_NVIC_PRIORITY 5
#define DMA_IRQ_NVIC_SUB_PRIORITY 0

#define DMA_WAIT_ALLOCATED_TIMEOUT  1000
#define DMA_WAIT_TRANSFER_TIMEOUT   1000

#if defined (STM32F0) || defined (STM32F1) || \
    defined (STM32F3) || defined (STM32L0) || \
    defined (STM32G0) || defined (STM32G4) || \
    defined (STM32C0) || defined (STM32U0) || \
    defined (STM32U5) || defined (STM32H5) || \
    defined (STM32L5)
DMA_HandleTypeDef g_dma[hwDMA_Channel_Index_MAX];

bool DMA_Channel_Init_Status[hwDMA_Channel_Index_MAX] = {false};
NeonRTOS_LockObj_t DMA_Channel_Mutex[hwDMA_Channel_Index_MAX] = {NULL};
#endif

#if defined (STM32F2) || defined (STM32F4) || \
    defined (STM32F7) || defined (STM32H7)
DMA_HandleTypeDef g_dma[hwDMA_Stream_Index_MAX];

bool DMA_Stream_Init_Status[hwDMA_Stream_Index_MAX] = {false};
NeonRTOS_LockObj_t DMA_Stream_Mutex[hwDMA_Stream_Index_MAX] = {NULL};
#endif

hwDMA_OpResult DMA_Init()
{
#if defined (STM32F0) || defined (STM32F1) || \
    defined (STM32F3) || defined (STM32L0) || \
    defined (STM32G0) || defined (STM32G4) || \
    defined (STM32C0) || defined (STM32U0) || \
    defined (STM32U5) || defined (STM32H5) || \
    defined (STM32L5)
    for (hwDMA_Channel_Index i = 0; i < hwDMA_Channel_Index_MAX; i++)
    {
        if (NeonRTOS_LockObjCreate(&DMA_Channel_Mutex[i]) != NeonRTOS_OK)
        {
            return hwDMA_MemoryError;
        }

        DMA_Channel_Init_Status[i] = true;
    }
#endif
#if defined (STM32F2) || defined (STM32F4) || \
    defined (STM32F7) || defined (STM32H7)
    for (hwDMA_Stream_Index i = 0; i < hwDMA_Stream_Index_MAX; i++)
    {
        if (NeonRTOS_LockObjCreate(&DMA_Stream_Mutex[i]) != NeonRTOS_OK)
        {
            return hwDMA_MemoryError;
        }

        DMA_Stream_Init_Status[i] = true;
    }
#endif
    
    DMA_Clock_Enable();

    return hwDMA_OK;
}

hwDMA_OpResult DMA_DeInit()
{
#if defined (STM32F0) || defined (STM32F1) || \
    defined (STM32F3) || defined (STM32L0) || \
    defined (STM32G0) || defined (STM32G4) || \
    defined (STM32C0) || defined (STM32U0) || \
    defined (STM32U5) || defined (STM32H5) || \
    defined (STM32L5)
    for (hwDMA_Channel_Index i = 0; i < hwDMA_Channel_Index_MAX; i++)
    {
        NeonRTOS_LockObjDelete(&DMA_Channel_Mutex[i]);

        DMA_Channel_Init_Status[i] = false;
    }
#endif
#if defined (STM32F2) || defined (STM32F4) || \
    defined (STM32F7) || defined (STM32H7)
    for (hwDMA_Stream_Index i = 0; i < hwDMA_Stream_Index_MAX; i++)
    {
        NeonRTOS_LockObjDelete(&DMA_Stream_Mutex[i]);

        DMA_Stream_Init_Status[i] = false;
    }
#endif

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

hwDMA_OpResult DMA_I2C_Write(hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len)
{
    return DMA_Xfer_I2C(index, hwDMA_Peripheral_Direction_TX, dev_addr, buf, len);
}

hwDMA_OpResult DMA_I2C_Read(hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len)
{
    return DMA_Xfer_I2C(index, hwDMA_Peripheral_Direction_RX, dev_addr, buf, len);
}

hwDMA_OpResult DMA_SPI_Write(hwSPI_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_SPI(index, hwDMA_Peripheral_Direction_TX, buf, len);
}

hwDMA_OpResult DMA_SPI_Read(hwSPI_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_SPI(index, hwDMA_Peripheral_Direction_RX, buf, len);
}

#endif //DEVICE_STM32
