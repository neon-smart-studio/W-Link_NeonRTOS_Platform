
#ifndef DMA_TIMSPM0_INDEX_H
#define DMA_TIMSPM0_INDEX_H

#include "soc.h"

/*
 * DMA_SYS_N_DMA_CHANNEL 由 TI 的型號專用 header 定義，
 * 代表目前 MCU 實際具備的 DMA 通道數。
 */
#if defined(DMA_SYS_N_DMA_CHANNEL)

#if (DMA_SYS_N_DMA_CHANNEL < 1) || (DMA_SYS_N_DMA_CHANNEL > 16)
#error "Unsupported MSPM0 DMA channel count"
#endif

typedef enum hwDMA_Channel_Index_t
{
    hwDMA_Channel_Index_0 = 0,

#if DMA_SYS_N_DMA_CHANNEL > 1
    hwDMA_Channel_Index_1,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 2
    hwDMA_Channel_Index_2,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 3
    hwDMA_Channel_Index_3,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 4
    hwDMA_Channel_Index_4,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 5
    hwDMA_Channel_Index_5,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 6
    hwDMA_Channel_Index_6,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 7
    hwDMA_Channel_Index_7,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 8
    hwDMA_Channel_Index_8,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 9
    hwDMA_Channel_Index_9,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 10
    hwDMA_Channel_Index_10,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 11
    hwDMA_Channel_Index_11,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 12
    hwDMA_Channel_Index_12,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 13
    hwDMA_Channel_Index_13,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 14
    hwDMA_Channel_Index_14,
#endif
#if DMA_SYS_N_DMA_CHANNEL > 15
    hwDMA_Channel_Index_15,
#endif

    hwDMA_Channel_Index_MAX = DMA_SYS_N_DMA_CHANNEL
} hwDMA_Channel_Index;

#endif /* DMA_SYS_N_DMA_CHANNEL */

#endif //DMA_TIMSPM0_INDEX_H