#ifndef DISPLAY_DEF_H
#define DISPLAY_DEF_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    Display_OK = 0,
    Display_NotInit = -1,
    Display_InvalidParameter = -2,
    Display_MemoryError = -3,
    Display_HwError = -4,
    Display_MutexTimeout = -5,
    Display_SlaveTimeout = -6,
    Display_SyncTimeout = -7,
    Display_Unsupport = -8,
} Display_OpResult;

typedef union {
    struct {
#ifdef CONFIG_COLOR_RGB565_SWAP
        uint16_t green_h : 3;
        uint16_t blue : 5;
        uint16_t red : 5;
        uint16_t green_l : 3;
#else //CONFIG_COLOR_RGB565_SWAP
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
#endif //CONFIG_COLOR_RGB565_SWAP
    };
    uint16_t full;
} Display_Color16_RGB565;

#endif // DISPLAY_DEF_H