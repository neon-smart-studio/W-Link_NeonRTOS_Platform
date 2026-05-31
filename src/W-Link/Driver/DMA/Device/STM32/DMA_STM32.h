
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

extern DMA_HandleTypeDef g_dma[hwDMA_Stream_Index_MAX];
extern bool DMA_Stream_Init_Status[hwDMA_Stream_Index_MAX];
extern NeonRTOS_LockObj_t DMA_Stream_Mutex[hwDMA_Stream_Index_MAX];

DMA_TypeDef * DMA_Map_Soc_Base(hwDMA_Stream_Index index);
DMA_Stream_TypeDef * DMA_Map_Soc_Stream_Base(hwDMA_Stream_Index index);

#if defined (BDMA_BASE)
BDMA_TypeDef * BDMA_Map_Soc_Base(hwDMA_Stream_Index index);
BDMA_Channel_TypeDef * BDMA_Map_Soc_Channel_Base(hwDMA_Stream_Index index);
#endif //BDMA_BASE

void DMA_Clock_Enable();
void DMA_Clock_Disable();

hwDMA_OpResult DMA_NVIC_Init(hwDMA_Stream_Index stream_index);
hwDMA_OpResult DMA_NVIC_DeInit(hwDMA_Stream_Index stream_index);

hwDMA_OpResult DMA_Xfer_UART(hwUART_Index index, hwDMA_Peripheral_Direction dir, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_Xfer_I2C(hwI2C_Index index, hwDMA_Peripheral_Direction dir, uint16_t dev_addr, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_Xfer_SPI(hwSPI_Index index, hwDMA_Peripheral_Direction dir, uint8_t* buf, size_t len);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //DMA_STM32_H