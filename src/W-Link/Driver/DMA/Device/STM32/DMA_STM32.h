
#ifndef DMA_STM32_H
#define DMA_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "DMA/DMA.h"

#include "DMA_STM32_Index.h"

#ifdef	__cplusplus
extern "C" {
#endif

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
extern DMA_HandleTypeDef g_dma[hwDMA_Channel_Index_MAX];
extern bool DMA_Channel_Init_Status[hwDMA_Channel_Index_MAX];
extern NeonRTOS_LockObj_t DMA_Channel_Mutex[hwDMA_Channel_Index_MAX];
#endif

#if defined (STM32F2) || defined (STM32F4) || \
    defined (STM32F7) || defined (STM32H7)
extern DMA_HandleTypeDef g_dma[hwDMA_Stream_Index_MAX];
extern bool DMA_Stream_Init_Status[hwDMA_Stream_Index_MAX];
extern NeonRTOS_LockObj_t DMA_Stream_Mutex[hwDMA_Stream_Index_MAX];
#endif

#if defined (STM32F0) || defined (STM32F1) || \
    defined (STM32F3) || defined (STM32L0) || \
    defined (STM32G0) || defined (STM32G4) || \
    defined (STM32C0) || defined (STM32U0) || \
    defined (STM32U5) || defined (STM32H5) || \
    defined (STM32L5)
DMA_TypeDef * DMA_Map_Soc_Base(hwDMA_Channel_Index index);
DMA_Channel_TypeDef * DMA_Map_Soc_Stream_Base(hwDMA_Channel_Index index);
#endif

#if defined (STM32F2) || defined (STM32F4) || \
    defined (STM32F7) || defined (STM32H7)
DMA_TypeDef * DMA_Map_Soc_Base(hwDMA_Stream_Index index);
DMA_Stream_TypeDef * DMA_Map_Soc_Stream_Base(hwDMA_Stream_Index index);

#if defined (BDMA_BASE)
BDMA_TypeDef * BDMA_Map_Soc_Base(hwDMA_Stream_Index index);
BDMA_Channel_TypeDef * BDMA_Map_Soc_Channel_Base(hwDMA_Stream_Index index);
#endif //BDMA_BASE
#endif

void DMA_Clock_Enable();
void DMA_Clock_Disable();

#if defined (STM32F0) || defined (STM32F1) || \
    defined (STM32F3) || defined (STM32L0) || \
    defined (STM32G0) || defined (STM32G4) || \
    defined (STM32C0) || defined (STM32U0) || \
    defined (STM32U5) || defined (STM32H5) || \
    defined (STM32L5)
hwDMA_OpResult DMA_NVIC_Init(hwDMA_Channel_Index stream_index);
hwDMA_OpResult DMA_NVIC_DeInit(hwDMA_Channel_Index stream_index);
#endif

#if defined (STM32F2) || defined (STM32F4) || \
    defined (STM32F7) || defined (STM32H7)
hwDMA_OpResult DMA_NVIC_Init(hwDMA_Stream_Index stream_index);
hwDMA_OpResult DMA_NVIC_DeInit(hwDMA_Stream_Index stream_index);
#endif

hwDMA_OpResult DMA_Xfer_UART(hwUART_Index index, hwDMA_Peripheral_Direction dir, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_Xfer_I2C(hwI2C_Index index, hwDMA_Peripheral_Direction dir, uint16_t dev_addr, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_Xfer_SPI(hwSPI_Index index, hwDMA_Peripheral_Direction dir, uint8_t* buf, size_t len);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //DMA_STM32_H