
#ifndef DMA_STM32_H
#define DMA_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "DMA/DMA.h"

extern DMA_HandleTypeDef g_dma[hwDMA_Stream_Index_MAX];
extern bool DMA_Stream_Init_Status[hwDMA_Stream_Index_MAX];

DMA_TypeDef * DMA_Map_Soc_Base(hwDMA_Stream_Index index);
DMA_Stream_TypeDef * DMA_Map_Soc_Stream_Base(hwDMA_Stream_Index index);

#if defined (BDMA_BASE)
BDMA_TypeDef * BDMA_Map_Soc_Base(hwDMA_Stream_Index index);
BDMA_Channel_TypeDef * BDMA_Map_Soc_Channel_Base(hwDMA_Stream_Index index);
#endif //BDMA_BASE

hwDMA_OpResult DMA_Instance_Init(hwDMA_Stream_Index stream_index);
hwDMA_OpResult DMA_Instance_DeInit(hwDMA_Stream_Index stream_index);

hwDMA_OpResult DMA_NVIC_Init(hwDMA_Stream_Index stream_index);
hwDMA_OpResult DMA_NVIC_DeInit(hwDMA_Stream_Index stream_index);

#endif //DMA_STM32_H