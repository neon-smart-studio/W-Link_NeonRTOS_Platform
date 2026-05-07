
#ifndef ILI9xxx_H
#define ILI9xxx_H

#include <stdint.h>

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "Display_Config.h"

#ifdef CONFIG_DISPLAY_ILI9225
#define DISPLAY_ILI9225
#endif //CONFIG_DISPLAY_ILI9225

#ifdef CONFIG_DISPLAY_ILI9163
#define DISPLAY_ILI9163
#endif //CONFIG_DISPLAY_ILI9163

#ifdef CONFIG_DISPLAY_ILI9341
#define DISPLAY_ILI9341
#endif //CONFIG_DISPLAY_ILI9341

#ifdef CONFIG_DISPLAY_ILI9481
#define DISPLAY_ILI9481
#endif //CONFIG_DISPLAY_ILI9481

#ifdef CONFIG_DISPLAY_ILI9486
#define DISPLAY_ILI9486
#endif //CONFIG_DISPLAY_ILI9486

#ifdef CONFIG_DISPLAY_ILI9488
#define DISPLAY_ILI9488
#endif //CONFIG_DISPLAY_ILI9488

#ifndef CONFIG_ILI9XXX_SPI_MODE
#define CONFIG_ILI9XXX_SPI_MODE 1
#endif
#ifndef CONFIG_ILI9XXX_LTDC_MODE
#define CONFIG_ILI9XXX_LTDC_MODE 0
#endif

#ifndef CONFIG_ILI9XXX_SPI_INDEX
#define CONFIG_ILI9XXX_SPI_INDEX hwSPI_Index_0
#endif

#ifndef CONFIG_ILI9XXX_SPI_CLOCK
#define CONFIG_ILI9XXX_SPI_CLOCK 1000000
#endif

#ifndef CONFIG_ILI9XXX_SPI_MODE
#define CONFIG_ILI9XXX_SPI_MODE hwSPI_OpMode_Polarity0_Phase0
#endif

#ifndef CONFIG_ILI9XXX_CS_PN
#define CONFIG_ILI9XXX_CS_PN hwGPIO_Pin_NC
#endif

#ifndef CONFIG_ILI9XXX_DC_PN
#define CONFIG_ILI9XXX_DC_PN hwGPIO_Pin_NC
#endif

#ifndef CONFIG_ILI9XXX_RST_PN
#define CONFIG_ILI9XXX_RST_PN hwGPIO_Pin_NC
#endif

typedef enum {
    ILI9xxx_OK = 0,
    ILI9xxx_NotInit = -1,
    ILI9xxx_InvalidParameter = -2,
    ILI9xxx_MemoryError = -3,
    ILI9xxx_HwError = -4,
    ILI9xxx_MutexTimeout = -5,
    ILI9xxx_SlaveTimeout = -6,
    ILI9xxx_Unsupport = -7,
} ILI9xxx_OpResult;

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
} ILI9xxx_Color16_RGB565;

#ifdef	__cplusplus
extern "C" {
#endif

ILI9xxx_OpResult ILI9xxx_Init(void);
ILI9xxx_OpResult ILI9xxx_SetWindow(int16_t x1, int16_t x2, int16_t y1, int16_t y2);

ILI9xxx_OpResult ILI9xxx_Power_On();
ILI9xxx_OpResult ILI9xxx_Power_Off();

ILI9xxx_OpResult ILI9xxx_VerticalScroll_Definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines);
ILI9xxx_OpResult ILI9xxx_VerticalScroll_StartLine(uint16_t startLine);

ILI9xxx_OpResult ILI9xxx_DrawPixel(int16_t x, int16_t y, ILI9xxx_Color16_RGB565* data);
ILI9xxx_OpResult ILI9xxx_Draw(int16_t x1, int16_t x2, int16_t y1, int16_t y2, ILI9xxx_Color16_RGB565* data);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // ILI9xxx_H
