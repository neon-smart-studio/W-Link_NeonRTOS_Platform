#ifndef HX8357x_H
#define HX8357x_H

#include <stdint.h>

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "Display_Config.h"

#ifdef CONFIG_DISPLAY_HX8357B
#define DISPLAY_HX8357B
#endif //CONFIG_DISPLAY_HX8357B

#ifdef CONFIG_DISPLAY_HX8357D
#define DISPLAY_HX8357D
#endif //CONFIG_DISPLAY_HX8357D

#ifndef CONFIG_HX8357X_SPI_INDEX
#define CONFIG_HX8357X_SPI_INDEX hwSPI_Index_0
#endif

#ifndef CONFIG_HX8357X_SPI_CLOCK
#define CONFIG_HX8357X_SPI_CLOCK 1000000
#endif

#ifndef CONFIG_HX8357X_SPI_MODE
#define CONFIG_HX8357X_SPI_MODE hwSPI_OpMode_Polarity0_Phase0
#endif

#ifndef CONFIG_HX8357X_CS_PN
#define CONFIG_HX8357X_CS_PN hwGPIO_Pin_NC
#endif

#ifndef CONFIG_HX8357X_DC_PN
#define CONFIG_HX8357X_DC_PN hwGPIO_Pin_NC
#endif

#ifndef CONFIG_HX8357X_RST_PN
#define CONFIG_HX8357X_RST_PN hwGPIO_Pin_NC
#endif

typedef enum {
    HX8357x_OK = 0,
    HX8357x_NotInit = -1,
    HX8357x_InvalidParameter = -2,
    HX8357x_MemoryError = -3,
    HX8357x_HwError = -4,
    HX8357x_MutexTimeout = -5,
    HX8357x_SlaveTimeout = -6,
    HX8357x_Unsupport = -7,
} HX8357x_OpResult;

typedef union {
    struct {
#ifdef CONFIG_COLOR_RGB565_SWAP
        uint16_t green_h : 3;
        uint16_t red : 5;
        uint16_t blue : 5;
        uint16_t green_l : 3;
#else //CONFIG_COLOR_RGB565_SWAP
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
#endif //CONFIG_COLOR_RGB565_SWAP
    };
    uint16_t full;
} HX8357x_Color16_RGB565;

#ifdef	__cplusplus
extern "C" {
#endif

HX8357x_OpResult HX8357x_Init(void);
HX8357x_OpResult HX8357x_SetWindow(int16_t x1, int16_t x2, int16_t y1, int16_t y2);

HX8357x_OpResult HX8357x_Power_On(void);
HX8357x_OpResult HX8357x_Power_Off(void);

HX8357x_OpResult HX8357x_Draw(int16_t x1, int16_t x2, int16_t y1, int16_t y2, HX8357x_Color16_RGB565 *data);
HX8357x_OpResult HX8357x_DrawPixel(int16_t x, int16_t y, HX8357x_Color16_RGB565 *data);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // HX8357x_H