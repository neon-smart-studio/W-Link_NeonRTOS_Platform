
#ifndef DMA_STM32_INDEX_H
#define DMA_STM32_INDEX_H

#include "soc.h"

/*
系列	DMA架構
F0	DMA Channel
F1	DMA Channel
F3	DMA Channel
L0	DMA Channel
G0	DMA Channel + DMAMUX
F2	DMA Stream + Channel
F4	DMA Stream + Channel
F7	DMA Stream + Channel
H7	DMA Stream + DMAMUX(Request)
G4	DMA + DMAMUX
U5	GPDMA
*/

#if defined (STM32F0) ||  defined (STM32F1) || defined (STM32F3) || defined (STM32L0) || defined (STM32G0) || defined (STM32C0) || defined (STM32U0)
typedef enum hwDMA_Channel_Index_t
{
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
  hwDMA_Channel_Index_1,
#endif
#if defined (DMA1_Channel2)
  hwDMA_Channel_Index_2,
#endif
#if defined (DMA1_Channel3)
  hwDMA_Channel_Index_3,
#endif
#if defined (DMA1_Channel4)
  hwDMA_Channel_Index_4,
#endif
#if defined (DMA1_Channel5)
  hwDMA_Channel_Index_5,
#endif
#if defined (DMA1_Channel6)
  hwDMA_Channel_Index_6,
#endif
#if defined (DMA1_Channel7)
  hwDMA_Channel_Index_7,
#endif
#endif //DMA1_BASE
#if defined (DMA2_BASE)
#if defined (DMA2_Channel1)
  hwDMA_Channel_Index_8,
#endif
#if defined (DMA2_Channel2)
  hwDMA_Channel_Index_9,
#endif
#if defined (DMA2_Channel3)
  hwDMA_Channel_Index_10,
#endif
#if defined (DMA2_Channel4)
  hwDMA_Channel_Index_11,
#endif
#if defined (DMA2_Channel5)
  hwDMA_Channel_Index_12,
#endif
#if defined (DMA2_Channel6)
  hwDMA_Channel_Index_13,
#endif
#if defined (DMA2_Channel7)
  hwDMA_Channel_Index_14,
#endif
#endif //DMA2_BASE
  hwDMA_Channel_Index_MAX,
}hwDMA_Channel_Index;
#endif
#if defined (STM32F2) ||  defined (STM32F4) || defined (STM32F7) || defined (STM32H7)
typedef enum hwDMA_Stream_Index_t
{
#if defined (DMA1_BASE)
#if defined (DMA1_Stream0)
  hwDMA_Stream_Index_0,
#endif
#if defined (DMA1_Stream1)
  hwDMA_Stream_Index_1,
#endif
#if defined (DMA1_Stream2)
  hwDMA_Stream_Index_2,
#endif
#if defined (DMA1_Stream3)
  hwDMA_Stream_Index_3,
#endif
#if defined (DMA1_Stream4)
  hwDMA_Stream_Index_4,
#endif
#if defined (DMA1_Stream5)
  hwDMA_Stream_Index_5,
#endif
#if defined (DMA1_Stream6)
  hwDMA_Stream_Index_6,
#endif
#if defined (DMA1_Stream7)
  hwDMA_Stream_Index_7,
#endif
#endif //DMA1_BASE
#if defined (DMA2_BASE)
#if defined (DMA2_Stream0)
  hwDMA_Stream_Index_8,
#endif
#if defined (DMA2_Stream1)
  hwDMA_Stream_Index_9,
#endif
#if defined (DMA2_Stream2)
  hwDMA_Stream_Index_10,
#endif
#if defined (DMA2_Stream3)
  hwDMA_Stream_Index_11,
#endif
#if defined (DMA2_Stream4)
  hwDMA_Stream_Index_12,
#endif
#if defined (DMA2_Stream5)
  hwDMA_Stream_Index_13,
#endif
#if defined (DMA2_Stream6)
  hwDMA_Stream_Index_14,
#endif
#if defined (DMA2_Stream7)
  hwDMA_Stream_Index_15,
#endif
#endif //DMA2_BASE
#if defined (BDMA_BASE)
#if defined (BDMA_Channel0)
  hwDMA_Stream_Index_16,
#endif
#if defined (BDMA_Channel1)
  hwDMA_Stream_Index_17,
#endif
#if defined (BDMA_Channel2)
  hwDMA_Stream_Index_18,
#endif
#if defined (BDMA_Channel3)
  hwDMA_Stream_Index_19,
#endif
#if defined (BDMA_Channel4)
  hwDMA_Stream_Index_20,
#endif
#if defined (BDMA_Channel5)
  hwDMA_Stream_Index_21,
#endif
#if defined (BDMA_Channel6)
  hwDMA_Stream_Index_22,
#endif
#if defined (BDMA_Channel7)
  hwDMA_Stream_Index_23,
#endif
#endif //BDMA_BASE
  hwDMA_Stream_Index_MAX,
}hwDMA_Stream_Index;
#endif

#endif //DMA_STM32_INDEX_H