
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#ifdef DEVICE_STM32

#define DMA_IRQ_NVIC_PRIORITY 5
#define DMA_IRQ_NVIC_SUB_PRIORITY 0

#define DMA_WAIT_ALLOCATED_TIMEOUT  1000
#define DMA_WAIT_TRANSFER_TIMEOUT   1000

DMA_HandleTypeDef g_dma[hwDMA_Stream_Index_MAX];

bool DMA_Stream_Init_Status[hwDMA_Stream_Index_MAX] = {false};

hwDMA_OpResult DMA_Stream_Init(hwDMA_Stream_Index stream_index)
{
        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Stream_Init_Status[stream_index]==true)
        {
                return hwDMA_OK;
        }

        DMA_Instance_Init(stream_index);

        DMA_NVIC_Init(stream_index);

        DMA_Stream_Init_Status[stream_index] = true;

        return hwDMA_OK;
}

hwDMA_OpResult DMA_Stream_DeInit(hwDMA_Stream_Index stream_index)
{
        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Stream_Init_Status[stream_index]==false)
        {
                return hwDMA_OK;
        }

        DMA_NVIC_DeInit(stream_index);

        DMA_Instance_DeInit(stream_index);

        return hwDMA_OK;
}

#endif //DEVICE_STM32
