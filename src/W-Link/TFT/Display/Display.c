
#include "Display.h"

#include "GPIO/GPIO.h"

#if defined(CONFIG_DISPLAY_GC9A01)
static Display_OpResult Display_Map_Result(GC9A01_OpResult result)
{
    switch (result) {
    case GC9A01_OK:
        return Display_OK;

    case GC9A01_NotInit:
        return Display_NotInit;

    case GC9A01_InvalidParameter:
        return Display_InvalidParameter;

    case GC9A01_MemoryError:
        return Display_MemoryError;

    case GC9A01_HwError:
        return Display_HwError;

    case GC9A01_MutexTimeout:
        return Display_MutexTimeout;

    case GC9A01_SlaveTimeout:
        return Display_SlaveTimeout;

    case GC9A01_SyncTimeout:
        return Display_SyncTimeout;

    case GC9A01_Unsupport:
    default:
        return Display_Unsupport;
    }
}
#endif

#if defined(CONFIG_DISPLAY_HX8357B) || defined(CONFIG_DISPLAY_HX8357D)
static Display_OpResult Display_Map_Result(HX8357x_OpResult result)
{
    switch (result) {
    case HX8357x_OK:
        return Display_OK;

    case HX8357x_NotInit:
        return Display_NotInit;

    case HX8357x_InvalidParameter:
        return Display_InvalidParameter;

    case HX8357x_MemoryError:
        return Display_MemoryError;

    case HX8357x_HwError:
        return Display_HwError;

    case HX8357x_MutexTimeout:
        return Display_MutexTimeout;

    case HX8357x_SlaveTimeout:
        return Display_SlaveTimeout;

    case HX8357x_Unsupport:
    default:
        return Display_Unsupport;
    }
}
#endif

#if defined(CONFIG_DISPLAY_ILI9225) || defined(CONFIG_DISPLAY_ILI9163) || \
      defined(CONFIG_DISPLAY_ILI9341) || defined(CONFIG_DISPLAY_ILI9481) || \
      defined(CONFIG_DISPLAY_ILI9486) || defined(CONFIG_DISPLAY_ILI9488)
static Display_OpResult Display_Map_Result(ILI9xxx_OpResult result)
{
    switch (result) {
    case ILI9xxx_OK:
        return Display_OK;

    case ILI9xxx_NotInit:
        return Display_NotInit;

    case ILI9xxx_InvalidParameter:
        return Display_InvalidParameter;

    case ILI9xxx_MemoryError:
        return Display_MemoryError;

    case ILI9xxx_HwError:
        return Display_HwError;

    case ILI9xxx_MutexTimeout:
        return Display_MutexTimeout;

    case ILI9xxx_SlaveTimeout:
        return Display_SlaveTimeout;

    case ILI9xxx_Unsupport:
    default:
        return Display_Unsupport;
    }
}
#endif

#if defined(CONFIG_DISPLAY_ST7735) || defined(CONFIG_DISPLAY_ST7789) || \
      defined(CONFIG_DISPLAY_ST7796S) || defined(CONFIG_DISPLAY_ST7796HV)
static Display_OpResult Display_Map_Result(ST77xx_OpResult result)
{
    switch (result) {
    case ST77xx_OK:
        return Display_OK;

    case ST77xx_NotInit:
        return Display_NotInit;

    case ST77xx_InvalidParameter:
        return Display_InvalidParameter;

    case ST77xx_MemoryError:
        return Display_MemoryError;

    case ST77xx_HwError:
        return Display_HwError;

    case ST77xx_MutexTimeout:
        return Display_MutexTimeout;

    case ST77xx_SlaveTimeout:
        return Display_SlaveTimeout;

    case ST77xx_Unsupport:
    default:
        return Display_Unsupport;
    }
}
#endif

static bool Display_Backlight_Ready = false;

Display_OpResult Display_Init(void)
{
#if defined(CONFIG_DISPLAY_GC9A01)
    return Display_Map_Result(GC9A01_Init());
#elif defined(CONFIG_DISPLAY_HX8357B) || defined(CONFIG_DISPLAY_HX8357D)
    return Display_Map_Result(HX8357x_Init());
#elif defined(CONFIG_DISPLAY_ILI9225) || defined(CONFIG_DISPLAY_ILI9163) || \
      defined(CONFIG_DISPLAY_ILI9341) || defined(CONFIG_DISPLAY_ILI9481) || \
      defined(CONFIG_DISPLAY_ILI9486) || defined(CONFIG_DISPLAY_ILI9488)
    return Display_Map_Result(ILI9xxx_Init());
#elif defined(CONFIG_DISPLAY_ST7735) || defined(CONFIG_DISPLAY_ST7789) || \
      defined(CONFIG_DISPLAY_ST7796S) || defined(CONFIG_DISPLAY_ST7796HV)
    return Display_Map_Result(ST77xx_Init());
#else
    return Display_Unsupport;
#endif
}

Display_OpResult Display_Power_On(void)
{
#if defined(CONFIG_DISPLAY_GC9A01)
    return Display_Map_Result(GC9A01_Power_On());
#elif defined(CONFIG_DISPLAY_HX8357B) || defined(CONFIG_DISPLAY_HX8357D)
    return Display_Map_Result(HX8357x_Power_On());
#elif defined(CONFIG_DISPLAY_ILI9225) || defined(CONFIG_DISPLAY_ILI9163) || \
      defined(CONFIG_DISPLAY_ILI9341) || defined(CONFIG_DISPLAY_ILI9481) || \
      defined(CONFIG_DISPLAY_ILI9486) || defined(CONFIG_DISPLAY_ILI9488)
    return Display_Map_Result(ILI9xxx_Power_On());
#elif defined(CONFIG_DISPLAY_ST7735) || defined(CONFIG_DISPLAY_ST7789) || \
      defined(CONFIG_DISPLAY_ST7796S) || defined(CONFIG_DISPLAY_ST7796HV)
    return Display_Map_Result(ST77xx_Power_On());
#else
    return Display_Unsupport;
#endif
}

Display_OpResult Display_Power_Off(void)
{
#if defined(CONFIG_DISPLAY_GC9A01)
    return Display_Map_Result(GC9A01_Power_Off());
#elif defined(CONFIG_DISPLAY_HX8357B) || defined(CONFIG_DISPLAY_HX8357D)
    return Display_Map_Result(HX8357x_Power_Off());
#elif defined(CONFIG_DISPLAY_ILI9225) || defined(CONFIG_DISPLAY_ILI9163) || \
      defined(CONFIG_DISPLAY_ILI9341) || defined(CONFIG_DISPLAY_ILI9481) || \
      defined(CONFIG_DISPLAY_ILI9486) || defined(CONFIG_DISPLAY_ILI9488)
    return Display_Map_Result(ILI9xxx_Power_Off());
#elif defined(CONFIG_DISPLAY_ST7735) || defined(CONFIG_DISPLAY_ST7789) || \
      defined(CONFIG_DISPLAY_ST7796S) || defined(CONFIG_DISPLAY_ST7796HV)
    return Display_Map_Result(ST77xx_Power_Off());
#else
    return Display_Unsupport;
#endif
}

Display_OpResult Display_Backlight_Init(void)
{
    hwGPIO_OpResult ret;

    ret = GPIO_Pin_Init(CONFIG_DISPLAY_BACKLIGHT_PN,
                        hwGPIO_Direction_Output,
                        hwGPIO_Pull_Mode_Up);

    if (ret != hwGPIO_OK) {
        return Display_HwError;
    }

    Display_Backlight_Ready = true;

    return Display_Backlight_Set(false);
}

Display_OpResult Display_Backlight_DeInit(void)
{
    if (!Display_Backlight_Ready) {
        return Display_OK;
    }

    if (GPIO_Pin_DeInit(CONFIG_DISPLAY_BACKLIGHT_PN) != hwGPIO_OK) {
        return Display_HwError;
    }

    Display_Backlight_Ready = false;
    return Display_OK;
}

Display_OpResult Display_Backlight_Set(bool on)
{
    uint8_t level;

    if (!Display_Backlight_Ready) {
        return Display_NotInit;
    }

#if (CONFIG_DISPLAY_BACKLIGHT_ACTIVE_LEVEL == 1)
    level = on ? 1 : 0;
#else
    level = on ? 0 : 1;
#endif

    if (GPIO_Pin_Write(CONFIG_DISPLAY_BACKLIGHT_PN, level) != hwGPIO_OK) {
        return Display_HwError;
    }

    return Display_OK;
}

Display_OpResult Display_SetWindow(int16_t x1, int16_t x2, int16_t y1, int16_t y2)
{
#if defined(CONFIG_DISPLAY_GC9A01)
    return Display_Map_Result(GC9A01_SetWindow(x1, x2, y1, y2));
#elif defined(CONFIG_DISPLAY_HX8357B) || defined(CONFIG_DISPLAY_HX8357D)
    return Display_Map_Result(HX8357x_SetWindow(x1, x2, y1, y2));
#elif defined(CONFIG_DISPLAY_ILI9225) || defined(CONFIG_DISPLAY_ILI9163) || \
      defined(CONFIG_DISPLAY_ILI9341) || defined(CONFIG_DISPLAY_ILI9481) || \
      defined(CONFIG_DISPLAY_ILI9486) || defined(CONFIG_DISPLAY_ILI9488)
    return Display_Map_Result(ILI9xxx_SetWindow(x1, x2, y1, y2));
#elif defined(CONFIG_DISPLAY_ST7735) || defined(CONFIG_DISPLAY_ST7789) || \
      defined(CONFIG_DISPLAY_ST7796S) || defined(CONFIG_DISPLAY_ST7796HV)
    return Display_Map_Result(ST77xx_SetWindow(x1, x2, y1, y2));
#else
    return Display_Unsupport;
#endif
}

Display_OpResult Display_Draw(int16_t x1, int16_t x2, int16_t y1, int16_t y2, Display_Color16_RGB565 *data)
{
    if (data == 0) {
        return Display_InvalidParameter;
    }

#if defined(CONFIG_DISPLAY_GC9A01)
    return Display_Map_Result(GC9A01_Draw(x1, x2, y1, y2, (GC9A01_Color16_RGB565 *)data));
#elif defined(CONFIG_DISPLAY_HX8357B) || defined(CONFIG_DISPLAY_HX8357D)
    return Display_Map_Result(HX8357x_Draw(x1, x2, y1, y2, (HX8357x_Color16_RGB565 *)data));
#elif defined(CONFIG_DISPLAY_ILI9225) || defined(CONFIG_DISPLAY_ILI9163) || \
      defined(CONFIG_DISPLAY_ILI9341) || defined(CONFIG_DISPLAY_ILI9481) || \
      defined(CONFIG_DISPLAY_ILI9486) || defined(CONFIG_DISPLAY_ILI9488)
    return Display_Map_Result(ILI9xxx_Draw(x1, x2, y1, y2, (ILI9xxx_Color16_RGB565 *)data));
#elif defined(CONFIG_DISPLAY_ST7735) || defined(CONFIG_DISPLAY_ST7789) || \
      defined(CONFIG_DISPLAY_ST7796S) || defined(CONFIG_DISPLAY_ST7796HV)
    return Display_Map_Result(ST77xx_Draw(x1, x2, y1, y2, (ST77xx_Color16_RGB565 *)data));
#else
    return Display_Unsupport;
#endif
}

Display_OpResult Display_DrawPixel(int16_t x, int16_t y, Display_Color16_RGB565 *data)
{
    return Display_Draw(x, x, y, y, data);
}

Display_OpResult Display_VerticalScroll_Definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines)
{
#if defined(CONFIG_DISPLAY_GC9A01)
    return Display_Map_Result(GC9A01_VerticalScroll_Definition(topFixedLines, scrollLines, bottomFixedLines));
#elif defined(CONFIG_DISPLAY_ILI9225) || defined(CONFIG_DISPLAY_ILI9163) || \
      defined(CONFIG_DISPLAY_ILI9341) || defined(CONFIG_DISPLAY_ILI9481) || \
      defined(CONFIG_DISPLAY_ILI9486) || defined(CONFIG_DISPLAY_ILI9488)
    return Display_Map_Result(ILI9xxx_VerticalScroll_Definition(topFixedLines, scrollLines, bottomFixedLines));
#elif defined(CONFIG_DISPLAY_ST7735) || defined(CONFIG_DISPLAY_ST7789) || \
      defined(CONFIG_DISPLAY_ST7796S) || defined(CONFIG_DISPLAY_ST7796HV)
    return Display_Map_Result(ST77xx_VerticalScroll_Definition(topFixedLines, scrollLines, bottomFixedLines));
#else
    return Display_Unsupport;
#endif
}

Display_OpResult Display_VerticalScroll_StartLine(uint16_t startLine)
{
#if defined(CONFIG_DISPLAY_GC9A01)
    return Display_Map_Result(GC9A01_VerticalScroll_StartLine(startLine));
#elif defined(CONFIG_DISPLAY_ILI9225) || defined(CONFIG_DISPLAY_ILI9163) || \
      defined(CONFIG_DISPLAY_ILI9341) || defined(CONFIG_DISPLAY_ILI9481) || \
      defined(CONFIG_DISPLAY_ILI9486) || defined(CONFIG_DISPLAY_ILI9488)
    return Display_Map_Result(ILI9xxx_VerticalScroll_StartLine(startLine));
#elif defined(CONFIG_DISPLAY_ST7735) || defined(CONFIG_DISPLAY_ST7789) || \
      defined(CONFIG_DISPLAY_ST7796S) || defined(CONFIG_DISPLAY_ST7796HV)
    return Display_Map_Result(ST77xx_VerticalScroll_StartLine(startLine));
#else
    return Display_Unsupport;
#endif
}