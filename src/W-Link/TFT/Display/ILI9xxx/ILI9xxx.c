
/*********************
 *      INCLUDES
 *********************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "NeonRTOS.h"

#include "ILI9xxx.h"

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

/*
 * Initialization sequence based on vendor datasheet
 * and publicly available examples.
 */
#if defined(DISPLAY_ILI9225)
#define ILI9xxx_CMD_START_OSC               0x00  // Start Osc
#define ILI9xxx_CMD_DRIVER_OUTPUT_CTRL      0x01  // Driver Output Control
#define ILI9xxx_CMD_LCD_AC_DRIVING_CTRL     0x02  // LCD AC Driving Control
#define ILI9xxx_CMD_ENTRY_MODE              0x03  // Entry Mode
#define ILI9xxx_CMD_DISP_CTRL1              0x07  // Display Control 1
#define ILI9xxx_CMD_BLANK_PERIOD_CTRL1      0x08  // Blank Period Control
#define ILI9xxx_CMD_FRAME_CYCLE_CTRL        0x0B  // Frame Cycle Control
#define ILI9xxx_CMD_INTERFACE_CTRL          0x0C  // Interface / Pixel Format Control
#define ILI9xxx_CMD_OSC_CTRL                0x0F  // Osc Control
#define ILI9xxx_CMD_POWER_CTRL1             0x10  // Power Control 1
#define ILI9xxx_CMD_POWER_CTRL2             0x11  // Power Control 2
#define ILI9xxx_CMD_POWER_CTRL3             0x12  // Power Control 3
#define ILI9xxx_CMD_POWER_CTRL4             0x13  // Power Control 4
#define ILI9xxx_CMD_POWER_CTRL5             0x14  // Power Control 5
#define ILI9xxx_CMD_VCI_RECYCLING           0x15  // VCI Recycling
#define ILI9xxx_CMD_RAM_ADDR_SET1           0x20  // Horizontal GRAM Address Set
#define ILI9xxx_CMD_RAM_ADDR_SET2           0x21  // Vertical GRAM Address Set
#define ILI9xxx_CMD_GRAM_DATA_REG           0x22  // GRAM Data Register
#define ILI9xxx_CMD_GATE_SCAN_CTRL          0x30  // Gate Scan Control Register
#define ILI9xxx_CMD_VERTICAL_SCROLL_CTRL1   0x31  // Vertical Scroll Control 1 Register
#define ILI9xxx_CMD_VERTICAL_SCROLL_CTRL2   0x32  // Vertical Scroll Control 2 Register
#define ILI9xxx_CMD_VERTICAL_SCROLL_CTRL3   0x33  // Vertical Scroll Control 3 Register
#define ILI9xxx_CMD_PARTIAL_DRIVING_POS1    0x34  // Partial Driving Position 1 Register
#define ILI9xxx_CMD_PARTIAL_DRIVING_POS2    0x35  // Partial Driving Position 2 Register
#define ILI9xxx_CMD_HORIZONTAL_WINDOW_ADDR1 0x36  // Horizontal Address Start Position
#define ILI9xxx_CMD_HORIZONTAL_WINDOW_ADDR2 0x37  // Horizontal Address End Position
#define ILI9xxx_CMD_VERTICAL_WINDOW_ADDR1   0x38  // Vertical Address Start Position
#define ILI9xxx_CMD_VERTICAL_WINDOW_ADDR2   0x39  // Vertical Address End Position
#define ILI9xxx_CMD_GAMMA_CTRL1             0x50  // Gamma Control 1
#define ILI9xxx_CMD_GAMMA_CTRL2             0x51  // Gamma Control 2
#define ILI9xxx_CMD_GAMMA_CTRL3             0x52  // Gamma Control 3
#define ILI9xxx_CMD_GAMMA_CTRL4             0x53  // Gamma Control 4
#define ILI9xxx_CMD_GAMMA_CTRL5             0x54  // Gamma Control 5
#define ILI9xxx_CMD_GAMMA_CTRL6             0x55  // Gamma Control 6
#define ILI9xxx_CMD_GAMMA_CTRL7             0x56  // Gamma Control 7
#define ILI9xxx_CMD_GAMMA_CTRL8             0x57  // Gamma Control 8
#define ILI9xxx_CMD_GAMMA_CTRL9             0x58  // Gamma Control 9
#define ILI9xxx_CMD_GAMMA_CTRL10            0x59  // Gamma Control 10
#define ILI9xxx_DELAY                       0xFF  // Delay
#endif //DISPLAY_ILI9225


#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)

/* Level 1 Commands (from the display Datasheet) */
#define ILI9xxx_CMD_NOP                             0x00
#define ILI9xxx_CMD_SOFTWARE_RESET                  0x01
#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_READ_DISP_ID                    0x04
#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_READ_ERROR_DSI                  0x05
#endif //DISPLAY_ILI9488
#define ILI9xxx_CMD_READ_DISP_STATUS                0x09
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9488
#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_READ_DISP_POWER_MODE            0x0A
#define ILI9xxx_CMD_READ_DISP_MADCTRL               0x0B    // bits 7:3 only
#define ILI9xxx_CMD_READ_DISP_PIXEL_FORMAT          0x0C
#define ILI9xxx_CMD_READ_DISP_IMAGE_MODE            0x0D
#define ILI9xxx_CMD_READ_DISP_SIGNAL_MODE           0x0E
#define ILI9xxx_CMD_READ_DISP_SELF_DIAGNOSTIC       0x0F    // bits 7:6 only
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

#define ILI9xxx_CMD_ENTER_SLEEP_MODE                0x10
#define ILI9xxx_CMD_SLEEP_OUT                       0x11
#define ILI9xxx_CMD_PARTIAL_MODE_ON                 0x12
#define ILI9xxx_CMD_NORMAL_DISP_MODE_ON             0x13

#define ILI9xxx_CMD_DISP_INVERSION_OFF              0x20
#define ILI9xxx_CMD_DISP_INVERSION_ON               0x21
#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_PIXEL_OFF                       0x22
#define ILI9xxx_CMD_PIXEL_ON                        0x23
#endif //DISPLAY_ILI9488
#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341)
#define ILI9xxx_CMD_GAMMA_SET                       0x26
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341
#define ILI9xxx_CMD_DISPLAY_OFF                     0x28
#define ILI9xxx_CMD_DISPLAY_ON                      0x29
#define ILI9xxx_CMD_COLUMN_ADDRESS_SET              0x2A
#define ILI9xxx_CMD_PAGE_ADDRESS_SET                0x2B
#define ILI9xxx_CMD_MEMORY_WRITE                    0x2C
#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341)
#define ILI9xxx_CMD_COLOR_SET                       0x2D
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341
#define ILI9xxx_CMD_MEMORY_READ                     0x2E

#define ILI9xxx_CMD_PARTIAL_AREA                    0x30
#define ILI9xxx_CMD_VERT_SCROLL_DEFINITION          0x33
#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_TEARING_EFFECT_LINE_OFF         0x34
#define ILI9xxx_CMD_TEARING_EFFECT_LINE_ON          0x35
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488
#define ILI9xxx_CMD_MEMORY_ACCESS_CONTROL           0x36    // bits 7:3,1:0 only
#define ILI9xxx_CMD_VERT_SCROLL_START_ADDRESS       0x37
#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_IDLE_MODE_OFF                   0x38
#define ILI9xxx_CMD_IDLE_MODE_ON                    0x39
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488
#define ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET         0x3A
#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_WRITE_MEMORY_CONTINUE           0x3C
#define ILI9xxx_CMD_READ_MEMORY_CONTINUE            0x3E
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_SET_TEAR_SCANLINE               0x44
#define ILI9xxx_CMD_GET_SCANLINE                    0x45
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_WRITE_DISPLAY_BRIGHTNESS        0x51
#define ILI9xxx_CMD_READ_DISPLAY_BRIGHTNESS         0x52
#define ILI9xxx_CMD_WRITE_CTRL_DISPLAY              0x53
#define ILI9xxx_CMD_READ_CTRL_DISPLAY               0x54
#define ILI9xxx_CMD_WRITE_CONTENT_ADAPT_BRIGHTNESS  0x55
#define ILI9xxx_CMD_READ_CONTENT_ADAPT_BRIGHTNESS   0x56
#define ILI9xxx_CMD_WRITE_MIN_CAB_LEVEL             0x5E
#define ILI9xxx_CMD_READ_MIN_CAB_LEVEL              0x5F
#endif //DISPLAY_ILI9341 || DISPLAY_ILI9488

#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_READ_ABC_SELF_DIAG_RES          0x68
#endif //DISPLAY_ILI9488

#if defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486)
#define ILI9xxx_DDB_START                           0xA1
#define ILI9xxx_CMD_ACCESS_PROTECT                  0xB0
#define ILI9xxx_CMD_LOW_POWER_CONTROL               0xB1
#define ILI9xxx_CMD_DISPLAY_MODE                    0xB4
#define ILI9xxx_CMD_FRAME_MEMORY_ACCESS             0xB3
#define ILI9xxx_CMD_DEVICE_CODE                     0xBF
#define ILI9xxx_CMD_PANEL_DRIVE                     0xC0
#define ILI9xxx_CMD_DISP_TIMING_NORMAL              0xC1
#define ILI9xxx_CMD_DISP_TIMING_PARTIAL             0xC2
#define ILI9xxx_CMD_DISP_TIMING_IDLE                0xC3
#define ILI9xxx_CMD_FRAME_RATE                      0xC5
#define ILI9xxx_CMD_INTERFACE_CONTROL               0xC6
#define ILI9xxx_CMD_GAMMA_SETTING                   0xC8
#define ILI9xxx_CMD_POWER_SETTING                   0xD0
#define ILI9xxx_CMD_VCOM_CONTROL                    0xD1
#define ILI9xxx_CMD_POWER_CONTROL_NORMAL            0xD2
#define ILI9xxx_CMD_POWER_CONTROL_IDEL              0xD3
#define ILI9xxx_CMD_POWER_CONTROL_PARTIAL           0xD4
#define ILI9xxx_CMD_NVMEM_WRITE                     0xE0
#define ILI9xxx_CMD_NVMEM_PROTECTION_KEY            0xE1
#define ILI9xxx_CMD_NVMEM_STATUS_READ               0xE2
#define ILI9xxx_CMD_NVMEM_PROTECTION                0xE3
#endif //DISPLAY_ILI9481 || DISPLAY_ILI9486

#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_READ_ID1                        0xDA
#define ILI9xxx_CMD_READ_ID2                        0xDB
#define ILI9xxx_CMD_READ_ID3                        0xDC
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9488

/* Level 2 Commands (from the display Datasheet) */
#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9488)
#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_INTERFACE_MODE_CONTROL          0xB0
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9488
#define ILI9xxx_CMD_FRAME_RATE_CONTROL_NORMAL       0xB1
#define ILI9xxx_CMD_FRAME_RATE_CONTROL_IDLE_8COLOR  0xB2
#define ILI9xxx_CMD_FRAME_RATE_CONTROL_PARTIAL      0xB3
#define ILI9xxx_CMD_DISPLAY_INVERSION_CONTROL       0xB4
#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_BLANKING_PORCH_CONTROL          0xB5
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9488
#define ILI9xxx_CMD_DISPLAY_FUNCTION_CONTROL        0xB6
#if defined(DISPLAY_ILI9163)
#define ILI9xxx_CMD_SOURCE_DRIVER_DIRECTION_CONTROL 0xB7
#else // DISPLAY_ILI9163
#define ILI9xxx_CMD_ENTRY_MODE_SET                  0xB7
#endif // DISPLAY_ILI9163
#if defined(DISPLAY_ILI9341)
#define ILI9xxx_CMD_BACKLIGHT_CONTROL_1             0xB8
#define ILI9xxx_CMD_BACKLIGHT_CONTROL_2             0xB9
#define ILI9xxx_CMD_BACKLIGHT_CONTROL_3             0xBA
#define ILI9xxx_CMD_BACKLIGHT_CONTROL_4             0xBB
#define ILI9xxx_CMD_BACKLIGHT_CONTROL_5             0xBC
#define ILI9xxx_CMD_BACKLIGHT_CONTROL_7             0xBE
#define ILI9xxx_CMD_BACKLIGHT_CONTROL_8             0xBF
#endif //DISPLAY_ILI9341
#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_BACKLIGHT_CONTROL_1             0xB9
#define ILI9xxx_CMD_BACKLIGHT_CONTROL_2             0xBA
#define ILI9xxx_CMD_HS_LANES_CONTROL                0xBE
#endif //DISPLAY_ILI9488

#define ILI9xxx_CMD_POWER_CONTROL_1                 0xC0
#define ILI9xxx_CMD_POWER_CONTROL_2                 0xC1
#if defined(DISPLAY_ILI9163)
#define ILI9xxx_CMD_POWER_CONTROL_3                 0xC2
#define ILI9xxx_CMD_POWER_CONTROL_4                 0xC3
#define ILI9xxx_CMD_POWER_CONTROL_5                 0xC4
#endif // DISPLAY_ILI9163
#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_POWER_CONTROL_NORMAL_3          0xC2
#define ILI9xxx_CMD_POWER_CONTROL_IDEL_4            0xC3
#define ILI9xxx_CMD_POWER_CONTROL_PARTIAL_5         0xC4
#endif // DISPLAY_ILI9488
#define ILI9xxx_CMD_VCOM_CONTROL_1                  0xC5
#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341)
#define ILI9xxx_CMD_VCOM_CONTROL_2                  0xC7
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341
#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_CABC_CONTROL_1                  0xC6
#define ILI9xxx_CMD_CABC_CONTROL_2                  0xC8
#define ILI9xxx_CMD_CABC_CONTROL_3                  0xC9
#define ILI9xxx_CMD_CABC_CONTROL_4                  0xCA
#define ILI9xxx_CMD_CABC_CONTROL_5                  0xCB
#define ILI9xxx_CMD_CABC_CONTROL_6                  0xCC
#define ILI9xxx_CMD_CABC_CONTROL_7                  0xCD
#define ILI9xxx_CMD_CABC_CONTROL_8                  0xCE
#define ILI9xxx_CMD_CABC_CONTROL_9                  0xCF
#endif //DISPLAY_ILI9488

#define ILI9xxx_CMD_NVMEM_WRITE                     0xD0
#define ILI9xxx_CMD_NVMEM_PROTECTION_KEY            0xD1
#define ILI9xxx_CMD_NVMEM_STATUS_READ               0xD2
#define ILI9xxx_CMD_READ_ID4                        0xD3
#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_ADJUST_CONTROL_1                0xD7
#define ILI9xxx_CMD_READ_ID_VERSION                 0xD8
#endif //DISPLAY_ILI9488

#define ILI9xxx_CMD_POSITIVE_GAMMA_CORRECTION       0xE0
#define ILI9xxx_CMD_NEGATIVE_GAMMA_CORRECTION       0xE1
#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_DIGITAL_GAMMA_CONTROL_1         0xE2
#define ILI9xxx_CMD_DIGITAL_GAMMA_CONTROL_2         0xE3
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9488
#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_SET_IMAGE_FUNCTION              0xE9
#endif //DISPLAY_ILI9488

/* Extend register commands */
#define LCD_POWERA               0xCB   /* Power control A register */
#define LCD_POWERB               0xCF   /* Power control B register */
#define LCD_DTCA                 0xE8   /* Driver timing control A */
#define LCD_DTCB                 0xEA   /* Driver timing control B */
#define LCD_POWER_SEQ            0xED   /* Power on sequence register */
#define LCD_3GAMMA_EN            0xF2   /* 3 Gamma enable register */
#define LCD_PRC                  0xF7   /* Pump ratio control register */

#if defined(DISPLAY_ILI9163)
#define ILI9xxx_CMD_GAMMA_CONTROL                   0xF2
#endif //DISPLAY_ILI9163
#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_ADJUST_CONTROL_2                0xF2
#endif //DISPLAY_ILI9488
#if defined(DISPLAY_ILI9341)
#define ILI9xxx_CMD_INTERFACE_CONTROL               0xF6
#endif //DISPLAY_ILI9341
#if defined(DISPLAY_ILI9488)
#define ILI9xxx_CMD_ADJUST_CONTROL_3                0xF7
#define ILI9xxx_CMD_ADJUST_CONTROL_4                0xF8
#define ILI9xxx_CMD_ADJUST_CONTROL_5                0xF9
#define ILI9xxx_CMD_SPI_READ_SETTINGS               0xFB
#define ILI9xxx_CMD_ADJUST_CONTROL_6                0xFC
#define ILI9xxx_CMD_ADJUST_CONTROL_7                0xFF
#endif //DISPLAY_ILI9488
#endif // DISPLAY_ILI9341 || DISPLAY_ILI9488 || DISPLAY_ILI9163

#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

typedef struct {
    uint8_t cmd;
    uint8_t data[40];
    uint8_t databytes;
} ILI9xxx_Init_Command;

#ifdef DISPLAY_ILI9163
ILI9xxx_Init_Command ILI9163_Init_Cmds[] = {
        {ILI9xxx_CMD_SOFTWARE_RESET, {0}, 0x80}, // Software reset, 0 args, w/delay 120ms
        {ILI9xxx_CMD_SLEEP_OUT, {0}, 0x80}, // Out of sleep mode, 0 args, w/delay 5ms
        {ILI9xxx_CMD_GAMMA_SET, {0x04}, 1}, // Gamma Curve
        {ILI9xxx_CMD_FRAME_RATE_CONTROL_NORMAL, {0x0C, 0x14}, 2}, // Frame rate ctrl - normal mode
        {ILI9xxx_CMD_DISPLAY_INVERSION_CONTROL, {0x07}, 1}, // Display inversion ctrl, 1 arg, no delay:No inversion
        {ILI9xxx_CMD_POWER_CTRL1, {0x0C, 0x05}, 2}, // Power control, 2 args, no delay
        {ILI9xxx_CMD_POWER_CTRL2, {0x02}, 1}, // Power control, 1 arg
        {ILI9xxx_CMD_POWER_CTRL3, {0x02}, 1}, // Power control, 1 arg
        {ILI9xxx_CMD_VCOM_CONTROL_1, {0x20, 0x55}, 2}, // Power control, 1 arg, no delay:
        {ILI9xxx_CMD_VCOM_CONTROL_2, {0x40}, 1}, // VCOM Offset
#ifdef CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
#ifdef CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_ON, {0}, 0}, // set inverted mode
#else //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_OFF, {0}, 0}, // set non-inverted mode
#endif //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x5}, 1}, // set color mode, 1 arg, no delay: 16-bit color
        //{ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x6}, 1}, // set color mode, 1 arg, no delay: 18-bit color
        {ILI9xxx_CMD_SOURCE_DRIVER_DIRECTION_CONTROL, {0}, 1}, // set source driver direction control
        {ILI9xxx_CMD_GAMMA_CONTROL, {0x01}, 1}, // set source driver direction control
        {ILI9xxx_CMD_POSITIVE_GAMMA_CORRECTION, {0x36, 0x29, 0x12, 0x22, 0x1C, 0x15, 0x42, 0xB7, 0x2F, 0x13, 0x12, 0x0A, 0x11, 0x0B, 0x06}, 16}, // 16 args, no delay:
        {ILI9xxx_CMD_NEGATIVE_GAMMA_CORRECTION, {0x09, 0x16, 0x2D, 0x0D, 0x13, 0x15, 0x40, 0x48, 0x53, 0x0C, 0x1D, 0x25, 0x2E, 0x34, 0x39}, 16}, // 16 args, no delay:
        {ILI9xxx_CMD_NORMAL_DISP_MODE_ON, {0}, 0x80}, // Normal display on, no args, w/delay 10 ms delay
        {ILI9xxx_CMD_DISPLAY_ON, {0}, 0x80}, // Main screen turn on, no args w/delay 100 ms delay
        {0, {0}, 0xff}
};
#endif //DISPLAY_ILI9163

#ifdef DISPLAY_ILI9225
ILI9xxx_Init_Command ILI9225_Init_Cmds[]={
        {ILI9xxx_CMD_POWER_CTRL1, {0x00, 0x00}, 2}, // Set SAP,DSTB,STB
        {ILI9xxx_CMD_POWER_CTRL2, {0x00, 0x00}, 2}, // Set APON,PON,AON,VCI1EN,VC
        {ILI9xxx_CMD_POWER_CTRL3, {0x00, 0x00}, 2}, // Set BT,DC1,DC2,DC3
        {ILI9xxx_CMD_POWER_CTRL4, {0x00, 0x00}, 2}, // Set GVDD
        {ILI9xxx_CMD_POWER_CTRL5, {0x00, 0x00}, 2}, // Set VCOMH/VCOML voltage
        {ILI9xxx_DELAY, {40}, 1}, // Delay 40ms
        {ILI9xxx_CMD_POWER_CTRL2, {0x00, 0x18}, 2}, // Set APON,PON,AON,VCI1EN,VC
        {ILI9xxx_CMD_POWER_CTRL3, {0x61, 0x21}, 2}, // Set BT,DC1,DC2,DC3
        {ILI9xxx_CMD_POWER_CTRL4, {0x00, 0x6F}, 2}, // Set GVDD   /*007F 0088 */
        {ILI9xxx_CMD_POWER_CTRL5, {0x49, 0x5F}, 2}, // Set VCOMH/VCOML voltage
        {ILI9xxx_CMD_POWER_CTRL1, {0x08, 0x00}, 2}, // Set SAP,DSTB,STB
        {ILI9xxx_DELAY, {10}, 1}, // Delay 10ms
        {ILI9xxx_CMD_POWER_CTRL2, {0x10, 0x3B}, 2}, // Set APON,PON,AON,VCI1EN,VC
        {ILI9xxx_DELAY, {50}, 1}, // Delay 50ms
        {ILI9xxx_CMD_DRIVER_OUTPUT_CTRL, {0x01, 0x1C}, 2}, // set the display line number and display direction
        {ILI9xxx_CMD_LCD_AC_DRIVING_CTRL, {0x01, 0x00}, 2}, // set 1 line inversion
        {ILI9xxx_CMD_ENTRY_MODE, {0x10, 0x38}, 2}, // set GRAM write direction and BGR=1.
        {ILI9xxx_CMD_DISP_CTRL1, {0x00, 0x00}, 2}, // Display off
        {ILI9xxx_CMD_BLANK_PERIOD_CTRL1, {0x08, 0x08}, 2}, // set the back porch and front porch
        {ILI9xxx_CMD_FRAME_CYCLE_CTRL, {0x11, 0x00}, 2}, // set the clocks number per line
        {ILI9xxx_CMD_INTERFACE_CTRL, {0x00, 0x01}, 2}, // Pixel Format 16bit
        {ILI9xxx_CMD_OSC_CTRL, {0x0D, 0x01}, 2}, // Set Osc  /*0e01*/
        {ILI9xxx_CMD_VCI_RECYCLING, {0x00, 0x20}, 2}, // Set VCI recycling
        {ILI9xxx_CMD_RAM_ADDR_SET1, {0x00, 0x00}, 2}, // RAM Address
        {ILI9xxx_CMD_RAM_ADDR_SET2, {0x00, 0x00}, 2}, // RAM Address
        /* Set GRAM area */
        {ILI9xxx_CMD_GATE_SCAN_CTRL, {0x00, 0x00}, 2},
        {ILI9xxx_CMD_VERTICAL_SCROLL_CTRL1, {0x00, 0xDB}, 2},
        {ILI9xxx_CMD_VERTICAL_SCROLL_CTRL2, {0x00, 0x00}, 2},
        {ILI9xxx_CMD_VERTICAL_SCROLL_CTRL3, {0x00, 0x00}, 2},
        {ILI9xxx_CMD_PARTIAL_DRIVING_POS1, {0x00, 0xDB}, 2},
        {ILI9xxx_CMD_PARTIAL_DRIVING_POS2, {0x00, 0x00}, 2},
        {ILI9xxx_CMD_HORIZONTAL_WINDOW_ADDR1, {0x00, 0xAF}, 2},
        {ILI9xxx_CMD_HORIZONTAL_WINDOW_ADDR2, {0x00, 0x00}, 2},
        {ILI9xxx_CMD_VERTICAL_WINDOW_ADDR1, {0x00, 0xDB}, 2},
        {ILI9xxx_CMD_VERTICAL_WINDOW_ADDR2, {0x00, 0x00}, 2},
        /* Set GAMMA curve */
        {ILI9xxx_CMD_GAMMA_CTRL1, {0x00, 0x00}, 2},
        {ILI9xxx_CMD_GAMMA_CTRL2, {0x08, 0x08}, 2},
        {ILI9xxx_CMD_GAMMA_CTRL3, {0x08, 0x0A}, 2},
        {ILI9xxx_CMD_GAMMA_CTRL4, {0x00, 0x0A}, 2},
        {ILI9xxx_CMD_GAMMA_CTRL5, {0x0A, 0x08}, 2},
        {ILI9xxx_CMD_GAMMA_CTRL6, {0x08, 0x08}, 2},
        {ILI9xxx_CMD_GAMMA_CTRL7, {0x00, 0x00}, 2},
        {ILI9xxx_CMD_GAMMA_CTRL8, {0x0A, 0x00}, 2},
        {ILI9xxx_CMD_GAMMA_CTRL9, {0x07, 0x10}, 2},
        {ILI9xxx_CMD_GAMMA_CTRL10, {0x07, 0x10}, 2},
        {ILI9xxx_CMD_DISP_CTRL1, {0x00, 0x12}, 2},
        {ILI9xxx_DELAY, {50}, 1}, // Delay 50ms
        {ILI9xxx_CMD_DISP_CTRL1, {0x10, 0x17}, 2},
        {0, {0}, 0xff},
};
#endif //DISPLAY_ILI9225

#ifdef DISPLAY_ILI9341
ILI9xxx_Init_Command ILI9341_Init_Cmds[]={
        {0xCF, {0x00, 0x83, 0X30}, 3},
        {0xED, {0x64, 0x03, 0x12, 0x81}, 4},
        {0xE8, {0x85, 0x01, 0x79}, 3},
        {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
        {0xF7, {0x20}, 1},
        {0xEA, {0x00, 0x00}, 2},
        {ILI9xxx_CMD_POWER_CONTROL_1, {0x26}, 1}, /*Power control*/
        {ILI9xxx_CMD_POWER_CONTROL_2, {0x11}, 1}, /*Power control */
        {ILI9xxx_CMD_VCOM_CONTROL_1, {0x35, 0x3E}, 2}, /*VCOM control*/
        {ILI9xxx_CMD_VCOM_CONTROL_2, {0xBE}, 1}, /*VCOM control*/
#ifdef CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
#ifdef CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_ON, {0}, 0}, // set inverted mode
#else //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_OFF, {0}, 0}, // set non-inverted mode
#endif //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x05}, 1}, /*Pixel Format Set 16bit*/
        //{ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x06}, 1}, /*Pixel Format Set 18bit*/
        {ILI9xxx_CMD_FRAME_RATE_CONTROL_NORMAL, {0x00, 0x1B}, 2},
        {0xF2, {0x08}, 1},
        {ILI9xxx_CMD_GAMMA_SET, {0x01}, 1},
        {ILI9xxx_CMD_POSITIVE_GAMMA_CORRECTION, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
        {ILI9xxx_CMD_NEGATIVE_GAMMA_CORRECTION, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
        {ILI9xxx_CMD_COLUMN_ADDRESS_SET, {0x00, 0x00, 0x00, 0xEF}, 4},
        {ILI9xxx_CMD_PAGE_ADDRESS_SET, {0x00, 0x00, 0x01, 0x3f}, 4},
        {ILI9xxx_CMD_MEMORY_WRITE, {0}, 0},
        {ILI9xxx_CMD_ENTRY_MODE_SET, {0x07}, 1},
        {ILI9xxx_CMD_DISPLAY_FUNCTION_CONTROL, {0x0A, 0x82, 0x27, 0x00}, 4},
        {ILI9xxx_CMD_SLEEP_OUT, {0}, 0x80},
        {ILI9xxx_CMD_DISPLAY_ON, {0}, 0x80},
        {0, {0}, 0xff},
};
#endif //DISPLAY_ILI9341

#ifdef DISPLAY_ILI9481
ILI9xxx_Init_Command ILI9481_Init_Cmds[]={
        {ILI9xxx_CMD_SLEEP_OUT, {0x00}, 0x80},
        {ILI9xxx_CMD_POWER_SETTING, {0x07, 0x42, 0x18}, 3},
        {ILI9xxx_CMD_VCOM_CONTROL, {0x00, 0x07, 0x10}, 3},
#ifdef CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
#ifdef CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_ON, {0}, 0}, // set inverted mode
#else //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_OFF, {0}, 0}, // set non-inverted mode
#endif //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_POWER_CONTROL_NORMAL, {0x01, 0x02}, 2},
        {ILI9xxx_CMD_PANEL_DRIVE, {0x10, 0x3B, 0x00, 0x02, 0x11}, 5},
        {ILI9xxx_CMD_FRAME_RATE, {0x03}, 1},
        {ILI9xxx_CMD_FRAME_MEMORY_ACCESS, {0x0, 0x0, 0x0, 0x0}, 4},
        //{ILI9xxx_CMD_DISP_TIMING_NORMAL, {0x10, 0x10, 0x22}, 3},
        {ILI9xxx_CMD_GAMMA_SETTING, {0x00, 0x32, 0x36, 0x45, 0x06, 0x16, 0x37, 0x75, 0x77, 0x54, 0x0C, 0x00}, 12},
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x0A}, 1},
        {ILI9xxx_CMD_DISP_INVERSION_OFF, {0}, 0}, // set non-inverted mode
        {ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x05}, 1}, /*Pixel Format Set 16bit*/
        //{ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x06}, 1}, /*Pixel Format Set 18bit*/
        {ILI9xxx_CMD_NORMAL_DISP_MODE_ON, {0}, 0x80},
        {ILI9xxx_CMD_DISPLAY_ON, {0}, 0x80},
        {0, {0}, 0xff},
};
#endif //DISPLAY_ILI9481

#ifdef DISPLAY_ILI9486
ILI9xxx_Init_Command ILI9486_Init_Cmds[]={
        {ILI9xxx_CMD_SLEEP_OUT, {0}, 0x80},
#ifdef CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
#ifdef CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_ON, {0}, 0}, // set inverted mode
#else //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_OFF, {0}, 0}, // set non-inverted mode
#endif //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x05}, 1}, /*Pixel Format Set 16bit*/
        //{ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x06}, 1}, /*Pixel Format Set 18bit*/
        {ILI9xxx_CMD_MEMORY_WRITE, {0x44}, 1},
        {ILI9xxx_CMD_FRAME_RATE, {0x00, 0x00, 0x00, 0x00}, 4},
        {ILI9xxx_CMD_NVMEM_WRITE, {0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98, 0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00}, 15},
        {ILI9xxx_CMD_NVMEM_PROTECTION_KEY, {0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75, 0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00}, 15},
        {ILI9xxx_CMD_DISP_INVERSION_OFF, {0}, 0}, /* display inversion OFF */
        {ILI9xxx_CMD_DISPLAY_ON, {0}, 0x80}, /* display on */	
        {0x00, {0}, 0xff},
};
#endif //DISPLAY_ILI9486

#ifdef DISPLAY_ILI9488
ILI9xxx_Init_Command ILI9xxx_Init_Cmds[]={
        {ILI9xxx_CMD_SLEEP_OUT, {0x00}, 0x80},
        {ILI9xxx_CMD_POSITIVE_GAMMA_CORRECTION, {0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F}, 15},
        {ILI9xxx_CMD_NEGATIVE_GAMMA_CORRECTION, {0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F}, 15},
        {ILI9xxx_CMD_POWER_CONTROL_1, {0x17, 0x15}, 2},
        {ILI9xxx_CMD_POWER_CONTROL_2, {0x41}, 1},
        {ILI9xxx_CMD_VCOM_CONTROL_1, {0x00, 0x12, 0x80}, 3},
#ifdef CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
        {ILI9xxx_CMD_MEMORY_ACCESS_CONTROL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
#ifdef CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_ON, {0}, 0}, // set inverted mode
#else //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_DISP_INVERSION_OFF, {0}, 0}, // set non-inverted mode
#endif //CONFIG_DISPLAY_INVERTED
        {ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x05}, 1}, /*Pixel Format Set 16bit*/
        //{ILI9xxx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x06}, 1}, /*Pixel Format Set 18bit*/
        {ILI9xxx_CMD_INTERFACE_MODE_CONTROL, {0x00}, 1},
        {ILI9xxx_CMD_FRAME_RATE_CONTROL_NORMAL, {0xA0}, 1},
        {ILI9xxx_CMD_DISPLAY_INVERSION_CONTROL, {0x02}, 1},
        {ILI9xxx_CMD_DISPLAY_FUNCTION_CONTROL, {0x02, 0x02}, 2},
        {ILI9xxx_CMD_SET_IMAGE_FUNCTION, {0x00}, 1},
        {ILI9xxx_CMD_WRITE_CTRL_DISPLAY, {0x28}, 1},
        {ILI9xxx_CMD_WRITE_DISPLAY_BRIGHTNESS, {0x7F}, 1},
        {ILI9xxx_CMD_ADJUST_CONTROL_3, {0xA9, 0x51, 0x2C, 0x02}, 4},
        {ILI9xxx_CMD_DISPLAY_ON, {0x00}, 0x80},
        {0, {0}, 0xff},
};
#endif //DISPLAY_ILI9488

#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
uint16_t ILI9xxx_VerticalScrollingStartAddress = 0;
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

static ILI9xxx_OpResult ILI9xxx_Map_GPIO_Error_Code(hwGPIO_OpStatus error_code)
{
        switch(error_code)
        {
                case hwGPIO_OK:
                        return ILI9xxx_OK;
                
                case hwGPIO_InvalidParameter:
                        return ILI9xxx_InvalidParameter;
                
                case hwGPIO_HW_Error:
                case hwGPIO_PinConflict:
                        return ILI9xxx_HwError;
                
                case hwGPIO_Unsupport:
                default:
                        return ILI9xxx_Unsupport;
        }
}

static ILI9xxx_OpResult ILI9xxx_Map_SPI_Error_Code(hwSPI_OpResult error_code)
{
        switch(error_code)
        {
                case hwSPI_OK:
                        return ILI9xxx_OK;
                
                case hwSPI_NotInit:
                        return ILI9xxx_NotInit;
                
                case hwSPI_InvalidParameter:
                        return ILI9xxx_InvalidParameter;
                
                case hwSPI_MemoryError:
                        return ILI9xxx_MemoryError;
                
                case hwSPI_MutexTimeout:
                        return ILI9xxx_MutexTimeout;
                
                case hwSPI_SlaveTimeout:
                        return ILI9xxx_SlaveTimeout;
                
                case hwSPI_Unsupport:
                default:
                        return ILI9xxx_Unsupport;
        }
}

static ILI9xxx_OpResult ILI9xxx_IO_Init()
{
        hwGPIO_OpStatus gpio_op_result;
        hwSPI_OpResult spi_op_result;
        
        gpio_op_result = GPIO_Pin_Init(CONFIG_ILI9XXX_CS_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

        gpio_op_result = GPIO_Pin_Init(CONFIG_ILI9XXX_DC_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
        if(gpio_op_result<hwGPIO_OK)
        {
                GPIO_Pin_DeInit(CONFIG_ILI9XXX_CS_PN);
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

        gpio_op_result = GPIO_Pin_Init(CONFIG_ILI9XXX_RST_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
        if(gpio_op_result<hwGPIO_OK)
        {
                GPIO_Pin_DeInit(CONFIG_ILI9XXX_CS_PN);
                GPIO_Pin_DeInit(CONFIG_ILI9XXX_DC_PN);
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

        spi_op_result = SPI_Master_Init(CONFIG_ILI9XXX_SPI_INDEX, CONFIG_ILI9XXX_SPI_CLOCK, CONFIG_ILI9XXX_SPI_MODE);
        if(spi_op_result<hwSPI_OK)
        {
                GPIO_Pin_DeInit(CONFIG_ILI9XXX_CS_PN);
                GPIO_Pin_DeInit(CONFIG_ILI9XXX_DC_PN);
                GPIO_Pin_DeInit(CONFIG_ILI9XXX_RST_PN);
                return ILI9xxx_Map_SPI_Error_Code(spi_op_result);
        }

        return ILI9xxx_OK;
}

static ILI9xxx_OpResult ILI9xxx_Reset()
{
        hwGPIO_OpStatus gpio_op_result;

        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_RST_PN, 1);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

        NeonRTOS_Sleep(10);
        
        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_RST_PN, 0);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        NeonRTOS_Sleep(100);
#endif
#ifdef DISPLAY_ILI9163
        NeonRTOS_Sleep(100);
#endif
#ifdef DISPLAY_ILI9225
        NeonRTOS_Sleep(10);
#endif

        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_RST_PN, 1);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

#if defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        NeonRTOS_Sleep(100);
#endif
#ifdef DISPLAY_ILI9163
        NeonRTOS_Sleep(150);
#endif
#ifdef DISPLAY_ILI9225
        NeonRTOS_Sleep(50);
#endif

        return ILI9xxx_OK;
}

static ILI9xxx_OpResult ILI9xxx_IO_Write(uint8_t cmd, uint8_t* param_data, uint16_t param_length)
{
        hwGPIO_OpStatus gpio_op_result;
        hwSPI_OpResult spi_op_result;

        if(param_data==NULL && param_length>0)
        {
                return ILI9xxx_InvalidParameter;
        }
                
        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_CS_PN, 0);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_DC_PN, 0);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

#if CONFIG_ILI9XXX_SPI_MODE
        spi_op_result = SPI_Master_WriteByte(CONFIG_ILI9XXX_SPI_INDEX, cmd);
        if(spi_op_result<hwSPI_OK)
        {
                return ILI9xxx_Map_SPI_Error_Code(spi_op_result);
        }
#endif
#if CONFIG_ILI9XXX_LTDC_MODE
#endif

        if(param_length==0)
        {
                gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_CS_PN, 1);
                if(gpio_op_result<hwGPIO_OK)
                {
                        return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
                }

                return ILI9xxx_OK; 
        }
                
        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_DC_PN, 1);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

#if CONFIG_ILI9XXX_SPI_MODE
        for(uint16_t i = 0; i<param_length; i++)
        {
                spi_op_result = SPI_Master_WriteByte(CONFIG_ILI9XXX_SPI_INDEX, param_data[i]);
                if(spi_op_result<hwSPI_OK)
                {
                        return ILI9xxx_Map_SPI_Error_Code(spi_op_result);
                }
        }
#endif
#if CONFIG_ILI9XXX_LTDC_MODE
#endif

        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_CS_PN, 1);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

        return ILI9xxx_OK;
}

static ILI9xxx_OpResult ILI9xxx_Nenory_Write(ILI9xxx_Color16_RGB565* data, uint16_t data_length)
{
        hwGPIO_OpStatus gpio_op_result;
        hwSPI_OpResult spi_op_result;

        if(data==NULL && data_length>0)
        {
                return ILI9xxx_InvalidParameter;
        }
                
        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_CS_PN, 0);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_DC_PN, 0);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        spi_op_result = SPI_Master_WriteByte(CONFIG_ILI9XXX_SPI_INDEX, ILI9xxx_CMD_MEMORY_WRITE);
        if(spi_op_result<hwSPI_OK)
        {
                return ILI9xxx_Map_SPI_Error_Code(spi_op_result);
        }
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488
#if defined(DISPLAY_ILI9225)
        spi_op_result = SPI_Master_WriteByte(CONFIG_ILI9XXX_SPI_INDEX, ILI9xxx_CMD_GRAM_DATA_REG);
        if(spi_op_result<hwSPI_OK)
        {
                return ILI9xxx_Map_SPI_Error_Code(spi_op_result);
        }
#endif // DISPLAY_ILI9225
        
        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_DC_PN, 1);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

        for(uint16_t i = 0; i<data_length; i++)
        {
                uint8_t buff[2];
                buff[0] = ((uint16_t)data[i].full)>>8;
                buff[1] = ((uint16_t)data[i].full)&0xff;
                spi_op_result = SPI_Master_WriteByte(CONFIG_ILI9XXX_SPI_INDEX, buff[0]);
                if(spi_op_result<hwSPI_OK)
                {
                return ILI9xxx_Map_SPI_Error_Code(spi_op_result);
                }
                spi_op_result = SPI_Master_WriteByte(CONFIG_ILI9XXX_SPI_INDEX, buff[1]);
                if(spi_op_result<hwSPI_OK)
                {
                return ILI9xxx_Map_SPI_Error_Code(spi_op_result);
                }
        }

        gpio_op_result = GPIO_Pin_Write(CONFIG_ILI9XXX_CS_PN, 1);
        if(gpio_op_result<hwGPIO_OK)
        {
                return ILI9xxx_Map_GPIO_Error_Code(gpio_op_result);
        }

        return ILI9xxx_OK;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

ILI9xxx_OpResult ILI9xxx_Init(void)
{
        ILI9xxx_OpResult op_status;

        op_status = ILI9xxx_IO_Init();
        if(op_status<ILI9xxx_OK) { return op_status; }

        //Reset the display
        op_status = ILI9xxx_Reset();
        if(op_status<ILI9xxx_OK) { return op_status; }

        //Send all the commands
        ILI9xxx_Init_Command* pILI9xxx_Init_Cmds = NULL;

#ifdef DISPLAY_ILI9163
        pILI9xxx_Init_Cmds = ILI9163_Init_Cmds;
#endif // DISPLAY_ILI9163
#ifdef DISPLAY_ILI9225
        pILI9xxx_Init_Cmds = ILI9225_Init_Cmds;
#endif // DISPLAY_ILI9225
#ifdef DISPLAY_ILI9341
        pILI9xxx_Init_Cmds = ILI9341_Init_Cmds;
#endif // DISPLAY_ILI9341
#ifdef DISPLAY_ILI9481
        pILI9xxx_Init_Cmds = ILI9481_Init_Cmds;
#endif // DISPLAY_ILI9481
#ifdef DISPLAY_ILI9486
        pILI9xxx_Init_Cmds = ILI9486_Init_Cmds;
#endif // DISPLAY_ILI9486
#ifdef DISPLAY_ILI9488
        pILI9xxx_Init_Cmds = ILI9xxx_Init_Cmds;
#endif // DISPLAY_ILI9488

        if(pILI9xxx_Init_Cmds!=NULL)
        {
                uint16_t cmd = 0;
                while (pILI9xxx_Init_Cmds[cmd].databytes!=0xff)
                {
#ifdef DISPLAY_ILI9225
                        if(pILI9xxx_Init_Cmds[cmd].cmd==ILI9xxx_DELAY)
                        {
                                NeonRTOS_Sleep(pILI9xxx_Init_Cmds[cmd].data[0]);
                                cmd++;
                                
                                continue;
                        }
                
                        op_status = ILI9xxx_IO_Write(pILI9xxx_Init_Cmds[cmd].cmd, pILI9xxx_Init_Cmds[cmd].data, pILI9xxx_Init_Cmds[cmd].databytes&0x1F);
                        if(op_status<ILI9xxx_OK) { break; }
#else // DISPLAY_ILI9225
                        op_status = ILI9xxx_IO_Write(pILI9xxx_Init_Cmds[cmd].cmd, pILI9xxx_Init_Cmds[cmd].data, pILI9xxx_Init_Cmds[cmd].databytes&0x1F);
                        if(op_status<ILI9xxx_OK) { break; }
#endif // DISPLAY_ILI9225

                        if (pILI9xxx_Init_Cmds[cmd].databytes & 0x80)
                        {
#if defined(DISPLAY_ILI9163)
                                NeonRTOS_Sleep(150);
#else //DISPLAY_ILI9163
                                NeonRTOS_Sleep(100);
#endif //DISPLAY_ILI9163
                        }

                        cmd++;
                }
        }

        return op_status;
}

ILI9xxx_OpResult ILI9xxx_Power_On()
{
        ILI9xxx_OpResult op_status;
    
#if defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_SLEEP_OUT, NULL, 0);
        if(op_status<ILI9xxx_OK) { return op_status; }

        NeonRTOS_Sleep(120);
#endif // DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341)
        uint8_t data[] = {0x08};

        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_SLEEP_OUT, data, 1);
        if(op_status<ILI9xxx_OK) { return op_status; }

        NeonRTOS_Sleep(120);
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341

#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_DISPLAY_ON, NULL, 0);
        if(op_status<ILI9xxx_OK) { return op_status; }
        
        NeonRTOS_Sleep(120);
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        uint8_t scroll_cfg[2];

        scroll_cfg[0] = ILI9xxx_VerticalScrollingStartAddress >> 8;
        scroll_cfg[1] = ILI9xxx_VerticalScrollingStartAddress & 0xFF;

        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_VERT_SCROLL_START_ADDRESS, scroll_cfg, 2);
        if(op_status<ILI9xxx_OK) { return op_status; }
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488
    
        return ILI9xxx_OK;
}

ILI9xxx_OpResult ILI9xxx_Power_Off()
{
        ILI9xxx_OpResult op_status;
    
#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_DISPLAY_OFF, NULL, 0);
        if(op_status<ILI9xxx_OK) { return op_status; }
        
        NeonRTOS_Sleep(120);
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

#if defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_ENTER_SLEEP_MODE, NULL, 0);
        if(op_status<ILI9xxx_OK) { return op_status; }
#endif // DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341)
        uint8_t data[] = {0x08};

        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_ENTER_SLEEP_MODE, data, 1);
        if(op_status<ILI9xxx_OK) { return op_status; }
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341

        return ILI9xxx_OK;
}

ILI9xxx_OpResult ILI9xxx_SetWindow(int16_t x1, int16_t x2, int16_t y1, int16_t y2)
{
        uint8_t set_x_index[4] = {0};
        uint8_t set_y_index[4] = {0};
        
        ILI9xxx_OpResult op_status;

        set_x_index[0] = (x1 >> 8) & 0xFF;
        set_x_index[1] = x1 & 0xFF;
        set_x_index[2] = (x2 >> 8) & 0xFF;
        set_x_index[3] = x2 & 0xFF;

        set_y_index[0] = (y1 >> 8) & 0xFF;
        set_y_index[1] = y1 & 0xFF;
        set_y_index[2] = (y2 >> 8) & 0xFF;
        set_y_index[3] = y2 & 0xFF;

#if defined(DISPLAY_ILI9225)
        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_HORIZONTAL_WINDOW_ADDR1, &set_x_index[2], 2);
        if(op_status<ILI9xxx_OK){return op_status;}

        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_HORIZONTAL_WINDOW_ADDR2, &set_x_index[0], 2);
        if(op_status<ILI9xxx_OK){return op_status;}

        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_VERTICAL_WINDOW_ADDR1, &set_y_index[2], 2);
        if(op_status<ILI9xxx_OK){return op_status;}

        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_VERTICAL_WINDOW_ADDR2, &set_y_index[0], 2);
        if(op_status<ILI9xxx_OK){return op_status;}
#endif // DISPLAY_ILI9225
        
        /*Column addresses*/
#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_COLUMN_ADDRESS_SET, set_x_index, 4);
        if(op_status<ILI9xxx_OK){return op_status;}
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488
#if defined(DISPLAY_ILI9225)
        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_RAM_ADDR_SET1, set_x_index, 2);
        if(op_status<ILI9xxx_OK){return op_status;}
#endif // DISPLAY_ILI9225

        /*Page addresses*/
#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_PAGE_ADDRESS_SET, set_y_index, 4);
        if(op_status<ILI9xxx_OK){return op_status;}
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488
#if defined(DISPLAY_ILI9225)
        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_RAM_ADDR_SET2, set_y_index, 2);
        if(op_status<ILI9xxx_OK){return op_status;}
#endif // DISPLAY_ILI9225
    
        return ILI9xxx_OK;
}

ILI9xxx_OpResult ILI9xxx_VerticalScroll_Definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines)
{
        ILI9xxx_OpResult op_status;

#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        uint8_t scroll_cfg[6];

        scroll_cfg[0] = topFixedLines >> 8;
        scroll_cfg[1] = topFixedLines & 0xFF;
        scroll_cfg[2] = scrollLines >> 8;
        scroll_cfg[3] = scrollLines & 0xFF;
        scroll_cfg[4] = bottomFixedLines >> 8;
        scroll_cfg[5] = bottomFixedLines & 0xFF;

        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_VERT_SCROLL_DEFINITION, scroll_cfg, 6);
        if(op_status<ILI9xxx_OK){return op_status;}
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488

        return ILI9xxx_OK;
}

ILI9xxx_OpResult ILI9xxx_VerticalScroll_StartLine(uint16_t startLine)
{
        ILI9xxx_OpResult op_status;

#if defined(DISPLAY_ILI9163) || defined(DISPLAY_ILI9341) || defined(DISPLAY_ILI9481) || defined(DISPLAY_ILI9486) || defined(DISPLAY_ILI9488)
        scroll_cfg[2];

        scroll_cfg[0] = startLine >> 8;
        scroll_cfg[1] = startLine & 0xFF;

        op_status = ILI9xxx_IO_Write(ILI9xxx_CMD_VERT_SCROLL_START_ADDRESS, scroll_cfg, 2, NULL);
        if(op_status<ILI9xxx_OK) { return op_status; }

        ILI9xxx_VerticalScrollingStartAddress = startLine;
#endif // DISPLAY_ILI9163 || DISPLAY_ILI9341 || DISPLAY_ILI9481 || DISPLAY_ILI9486 || DISPLAY_ILI9488
    
        return ILI9xxx_OK;
}

ILI9xxx_OpResult ILI9xxx_Draw(int16_t x1, int16_t x2, int16_t y1, int16_t y2, ILI9xxx_Color16_RGB565* data)
{
        uint32_t size = (abs(x2-x1) + 1) * (abs(y2-y1) + 1);

        ILI9xxx_OpResult op_status;

        op_status = ILI9xxx_SetWindow(x1, x2, y1, y2);
        if(op_status<ILI9xxx_OK) { return op_status; }
        
        /*Memory write*/
        op_status = ILI9xxx_Nenory_Write(data, size);
        if(op_status<ILI9xxx_OK) { return op_status; }
    
        return op_status;
}

ILI9xxx_OpResult ILI9xxx_DrawPixel(int16_t x, int16_t y, ILI9xxx_Color16_RGB565* data)
{
      return ILI9xxx_Draw(x, x, y, y, data);
}