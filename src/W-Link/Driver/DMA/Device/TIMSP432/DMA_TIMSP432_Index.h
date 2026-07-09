
#ifndef DMA_TIMSP432_INDEX_H
#define DMA_TIMSP432_INDEX_H

#include "soc.h"

#ifdef DEVICE_TIMSP432P
typedef enum hwDMA_Channel_Index_t
{
    hwDMA_Channel_Index_0 = 0,
    hwDMA_Channel_Index_1,
    hwDMA_Channel_Index_2,
    hwDMA_Channel_Index_3,
    hwDMA_Channel_Index_4,
    hwDMA_Channel_Index_5,
    hwDMA_Channel_Index_6,
    hwDMA_Channel_Index_7,
    hwDMA_Channel_Index_MAX
} hwDMA_Channel_Index;
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
typedef enum hwDMA_Channel_Index_t
{
    hwDMA_Channel_Index_0 = 0,
    hwDMA_Channel_Index_1,
    hwDMA_Channel_Index_2,
    hwDMA_Channel_Index_3,
    hwDMA_Channel_Index_4,
    hwDMA_Channel_Index_5,
    hwDMA_Channel_Index_6,
    hwDMA_Channel_Index_7,
    hwDMA_Channel_Index_8,
    hwDMA_Channel_Index_9,
    hwDMA_Channel_Index_10,
    hwDMA_Channel_Index_11,
    hwDMA_Channel_Index_12,
    hwDMA_Channel_Index_13,
    hwDMA_Channel_Index_14,
    hwDMA_Channel_Index_15,
    hwDMA_Channel_Index_16,
    hwDMA_Channel_Index_17,
    hwDMA_Channel_Index_18,
    hwDMA_Channel_Index_19,
    hwDMA_Channel_Index_20,
    hwDMA_Channel_Index_21,
    hwDMA_Channel_Index_22,
    hwDMA_Channel_Index_23,
    hwDMA_Channel_Index_24,
    hwDMA_Channel_Index_25,
    hwDMA_Channel_Index_26,
    hwDMA_Channel_Index_27,
    hwDMA_Channel_Index_28,
    hwDMA_Channel_Index_29,
    hwDMA_Channel_Index_30,
    hwDMA_Channel_Index_31,

    hwDMA_Channel_Index_MAX
} hwDMA_Channel_Index;
#endif // DEVICE_TIMSP432E

#endif //DMA_TIMSP432_INDEX_H