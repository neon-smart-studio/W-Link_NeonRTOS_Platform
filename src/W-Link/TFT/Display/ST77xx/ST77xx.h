
#ifndef ST77xx_H
#define ST77xx_H

#include <stdint.h>

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "Display_Config.h"

#ifdef CONFIG_DISPLAY_ST7735
#define DISPLAY_ST7735
#endif //CONFIG_DISPLAY_ST7735

#ifdef CONFIG_DISPLAY_ST7789
#define DISPLAY_ST7789
#endif //CONFIG_DISPLAY_ST7789

#ifdef CONFIG_DISPLAY_ST7796S
#define DISPLAY_ST7796S
#endif //CONFIG_DISPLAY_ST7796S

#ifndef CONFIG_ST77XX_SPI_MODE
#define CONFIG_ST77XX_SPI_MODE 1
#endif
#ifndef CONFIG_ST77XX_LTDC_MODE
#define CONFIG_ST77XX_LTDC_MODE 0
#endif

#ifndef CONFIG_ST77XX_SPI_INDEX
#define CONFIG_ST77XX_SPI_INDEX hwSPI_Index_0
#endif

#ifndef CONFIG_ST77XX_SPI_CLOCK
#define CONFIG_ST77XX_SPI_CLOCK 1000000
#endif

#ifndef CONFIG_ST77XX_SPI_MODE
#define CONFIG_ST77XX_SPI_MODE hwSPI_OpMode_Polarity0_Phase0
#endif

#ifndef CONFIG_ST77XX_CS_PN
#define CONFIG_ST77XX_CS_PN hwGPIO_Pin_NC
#endif

#ifndef CONFIG_ST77XX_DC_PN
#define CONFIG_ST77XX_DC_PN hwGPIO_Pin_NC
#endif

#ifndef CONFIG_ST77XX_RST_PN
#define CONFIG_ST77XX_RST_PN hwGPIO_Pin_NC
#endif

/*
ST7735 0.96
#define X_OFFSET 26
#define Y_OFFSET 1
#define LCD_W 160
#define LCD_H 80
*/

typedef enum {
    ST77xx_OK = 0,
    ST77xx_NotInit = -1,
    ST77xx_InvalidParameter = -2,
    ST77xx_MemoryError = -3,
    ST77xx_HwError = -4,
    ST77xx_MutexTimeout = -5,
    ST77xx_SlaveTimeout = -6,
    ST77xx_Unsupport = -7,
} ST77xx_OpResult;

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
} ST77xx_Color16_RGB565;

#ifdef	__cplusplus
extern "C" {
#endif

ST77xx_OpResult ST77xx_Init(void);
ST77xx_OpResult ST77xx_SetWindow(int16_t x1, int16_t x2, int16_t y1, int16_t y2);

ST77xx_OpResult ST77xx_Power_On();
ST77xx_OpResult ST77xx_Power_Off();

ST77xx_OpResult ST77xx_VerticalScroll_Definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines);
ST77xx_OpResult ST77xx_VerticalScroll_StartLine(uint16_t startLine);

ST77xx_OpResult ST77xx_DrawPixel(int16_t x, int16_t y, ST77xx_Color16_RGB565* data);
ST77xx_OpResult ST77xx_Draw(int16_t x1, int16_t x2, int16_t y1, int16_t y2, ST77xx_Color16_RGB565* data);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // ST77xx_H
