
#ifndef DMA_H
#define DMA_H

#include "DMA_Def.h"

#include "DMA_Index.h"

hwDMA_OpResult DMA_Stream_Init(hwDMA_Stream_Index stream_index);
hwDMA_OpResult DMA_Stream_DeInit(hwDMA_Stream_Index stream_index);

#endif //DMA_H