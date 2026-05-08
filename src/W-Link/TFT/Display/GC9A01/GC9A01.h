
#ifndef GC9A01_H
#define GC9A01_H

#include <stdint.h>

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "Display_Config.h"

#ifndef CONFIG_GC9A01_SPI_INDEX
#define CONFIG_GC9A01_SPI_INDEX hwSPI_Index_0
#endif

#ifndef CONFIG_GC9A01_SPI_CLOCK
#define CONFIG_GC9A01_SPI_CLOCK 1000000
#endif

#ifndef CONFIG_GC9A01_SPI_MODE
#define CONFIG_GC9A01_SPI_MODE hwSPI_OpMode_Polarity0_Phase0
#endif

#ifndef CONFIG_GC9A01_CS_PN
#define CONFIG_GC9A01_CS_PN hwGPIO_Pin_NC
#endif

#ifndef CONFIG_GC9A01_DC_PN
#define CONFIG_GC9A01_DC_PN hwGPIO_Pin_NC
#endif

#ifndef CONFIG_GC9A01_RST_PN
#define CONFIG_GC9A01_RST_PN hwGPIO_Pin_NC
#endif

typedef enum GC9A01_OpResult_t
{
  GC9A01_OK = 0,
  GC9A01_NotInit = -1,
  GC9A01_InvalidParameter = -2,
  GC9A01_MemoryError = -3,
  GC9A01_HwError = -4,
  GC9A01_MutexTimeout = -5,
  GC9A01_SlaveTimeout = -6,
  GC9A01_SyncTimeout = -7,
  GC9A01_Unsupport = -8,
}GC9A01_OpResult;

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
} GC9A01_Color16_RGB565;

#ifdef	__cplusplus
extern "C" {
#endif

GC9A01_OpResult GC9A01_Init(void);
GC9A01_OpResult GC9A01_SetWindow(int16_t x1, int16_t x2, int16_t y1, int16_t y2);

GC9A01_OpResult GC9A01_Power_On();
GC9A01_OpResult GC9A01_Power_Off();

GC9A01_OpResult GC9A01_VerticalScroll_Definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines);
GC9A01_OpResult GC9A01_VerticalScroll_StartLine(uint16_t startLine);

GC9A01_OpResult GC9A01_DrawPixel(int16_t x, int16_t y, GC9A01_Color16_RGB565* data);
GC9A01_OpResult GC9A01_Draw(int16_t x1, int16_t x2, int16_t y1, int16_t y2, GC9A01_Color16_RGB565* data);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // GC9A01_H
