
/*********************
 *      INCLUDES
 *********************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "NeonRTOS.h"

#include "ST77xx.h"

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

/*
 * Initialization sequence based on vendor datasheet
 * and publicly available examples.
 */
/* MIPI DCS Type1  */
#define ST77xx_CMD_NOP                          0x00
#define ST77xx_CMD_SOFTWARE_RESET               0x01
#define ST77xx_CMD_READ_DISPLAY_ID              0x04
#define ST77xx_CMD_READ_DISPLAY_STATUS          0x09
#define ST77xx_CMD_READ_DISP_POWER_MODE         0x0A // Read display power mode
#define ST77xx_CMD_READ_DISP_MADCTRL            0x0B // Read display MADCTL bits 7:3 only
#define ST77xx_CMD_READ_DISP_PIXEL_FORMAT       0x0C // Read display pixel format
#define ST77xx_CMD_READ_DISP_IMAGE_MODE         0x0D // Read display image mode
#define ST77xx_CMD_READ_DISP_SIGNAL_MODE        0x0E // Read display signal mode
#define ST77xx_CMD_READ_DISP_SELF_DIAGNOSTIC    0x0F // Read display self-diagnostic result bits 7:6 only

#define ST77xx_CMD_ENTER_SLEEP_MODE             0x10
#define ST77xx_CMD_SLEEP_OUT                    0x11
#define ST77xx_CMD_PARTIAL_MODE_ON              0x12
#define ST77xx_CMD_NORMAL_DISP_MODE_ON          0x13

#define ST77xx_CMD_DISP_INVERSION_OFF           0x20
#define ST77xx_CMD_DISP_INVERSION_ON            0x21
#if defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
#define ST77xx_CMD_GAMSET                       0x26    // Gamma set
#endif // DISPLAY_ST7789 || DISPLAY_ST7796S
#define ST77xx_CMD_DISPLAY_OFF                  0x28
#define ST77xx_CMD_DISPLAY_ON                   0x29
#define ST77xx_CMD_COLUMN_ADDRESS_SET           0x2A
#define ST77xx_CMD_PAGE_ADDRESS_SET             0x2B
#define ST77xx_CMD_MEMORY_WRITE                 0x2C
//#define ST77xx_CMD_RGBSET                       0x2D    // Color setting for 4096, 64K and 262K colors
#define ST77xx_CMD_MEMORY_READ                  0x2E

#define ST77xx_CMD_PARTIAL_AREA                 0x30
#define ST77xx_CMD_VERT_SCROLL_DEFINITION       0x33 // Vertical scrolling definition (ST77xx_CMDV)
#define ST77xx_CMD_TEARING_EFFECT_LINE_OFF      0x34 // Tearing effect line off
#define ST77xx_CMD_TEARING_EFFECT_LINE_ON       0x35 // Tearing effect line on
#define ST77xx_CMD_MEMORY_ACCESS_CONTROL        0x36 // Memory data access control bits 7:3,1:0 only
#if defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
#define ST77xx_CMD_VERT_SCROLL_START_ADDRESS    0x37
#endif // DISPLAY_ST7789 || DISPLAY_ST7796S
#define ST77xx_CMD_IDLE_MODE_OFF                0x38 // Idle mode off
#define ST77xx_CMD_IDLE_MODE_ON                 0x39 // Idle mode on
#define ST77xx_CMD_COLMOD_PIXEL_FORMAT_SET      0x3A
#if defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
#define ST77xx_CMD_WRITE_MEMORY_CONTINUE        0x3C // Memory write continue
#define ST77xx_CMD_READ_MEMORY_CONTINUE         0x3E // Memory read continue
#endif // DISPLAY_ST7789 || DISPLAY_ST7796S

#if defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
#define ST77xx_CMD_SET_TEAR_SCANLINE            0x44
#define ST77xx_CMD_GET_SCANLINE                 0x45
#endif // DISPLAY_ST7789 || DISPLAY_ST7796S

#if defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
#define ST77xx_DDB_START                        0xA1
#define ST77xx_DDB_CONTINUE                     0xA8
#endif // DISPLAY_ST7789 || DISPLAY_ST7796S

#ifdef DISPLAY_ST7735
#define ST77xx_CMD_FRAME_RATE_CONTROL_1         0xB1
#define ST77xx_CMD_FRAME_RATE_CONTROL_2         0xB2
#define ST77xx_CMD_FRAME_RATE_CONTROL_3         0xB3
#define ST77xx_CMD_INVERSION_CONTROL            0xB4
#define ST77xx_CMD_DISPLAY_FUNCTION_SET_5       0xB6

#define ST77xx_CMD_POWER_CONTROL_1              0xC0
#define ST77xx_CMD_POWER_CONTROL_2              0xC1
#define ST77xx_CMD_POWER_CONTROL_3              0xC2
#define ST77xx_CMD_POWER_CONTROL_4              0xC3
#define ST77xx_CMD_POWER_CONTROL_5              0xC4
#define ST77xx_CMD_VCOM_CONTROL_1               0xC5
#endif //DISPLAY_ST7735

#define ST77xx_CMD_READ_ID1                     0xDA
#define ST77xx_CMD_READ_ID2                     0xDB
#define ST77xx_CMD_READ_ID3                     0xDC

#ifdef DISPLAY_ST7735
#define ST77xx_CMD_GAMMA_CORRECTION_POSTIVE_1   0xE0
#define ST77xx_CMD_GAMMA_CORRECTION_NEGATIVE_1  0xE1
#endif //DISPLAY_ST7735

/* Level 2 Commands */
#ifdef DISPLAY_ST7789
#define ST77xx_CMD_RAM_CONTROL                      0xB0    // RAM control
#define ST77xx_CMD_RGB_CONTROL                      0xB1    // RGB control
#define ST77xx_CMD_PORCH_CONTROL                    0xB2    // Porch control
#define ST77xx_CMD_FRAME_RATE_CONTROL_1             0xB3    // Frame rate control
#define ST77xx_CMD_PARTIAL_CONTROL                  0xB5    // Partial mode control
#define ST77xx_CMD_DISPLAY_FUNCTION_CONTROL         0xB6
#define ST77xx_CMD_GATE_CONTROL_1                   0xB7    // Gate control
#define ST77xx_CMD_GATE_ON_TIMEING_ADJUSTMENT       0xB8    // Gate on timing adjustment
#define ST77xx_CMD_DIGITAL_GAMMA_EN                 0xBA    // Digital gamma enable
#define ST77xx_CMD_VCOMS_SETTING                    0xBB    // VCOMS setting

#define ST77xx_CMD_LCM_CONTROL                      0xC0    // LCM control
#define ST77xx_CMD_ID_SETTING                       0xC1    // ID setting
#define ST77xx_CMD_VDV_VRH_EN                       0xC2    // VDV and VRH command enable
#define ST77xx_CMD_VRH_SETTING                      0xC3    // VRH setting
#define ST77xx_CMD_VDV_SETTING                      0xC4    // VDV setting
#define ST77xx_CMD_VCOMS_OFFSET_SET                 0xC5    // VCOMS offset set
#define ST77xx_CMD_FRAME_RATE_CONTROL_2             0xC6    // FR Control 2
#define ST77xx_CMD_CABC_CONTROL                     0xC7    // CABC control
#define ST77xx_CMD_REGISTER_SECTION_1               0xC8    // Register value section 1
#define ST77xx_CMD_REGISTER_SECTION_2               0xCA    // Register value section 2
#define ST77xx_CMD_PWM_FREQUENCY_SELECTION          0xCC    // PWM frequency selection

#define ST77xx_CMD_POWER_CONTROL_1                  0xD0    // Power control 1
#define ST77xx_CMD_VAP_VAN_SIGNAL_OUT_EN            0xD2    // Enable VAP/VAN signal output
#define ST77xx_CMD_COMMAND_2_EN                     0xDF    // Command 2 enable

#define ST77xx_CMD_POSTIVE_VOLTAGE_GAMMA_CONTROL    0xE0    // Positive voltage gamma control
#define ST77xx_CMD_NEGATIVE_VOLTAGE_GAMMA_CONTROL   0xE1    // Negative voltage gamma control
#define ST77xx_CMD_DIGITAL_GAMMA_LOOKUP_TABLE_RED   0xE2    // Digital gamma look-up table for red
#define ST77xx_CMD_DIGITAL_GAMMA_LOOKUP_TABLE_BLUE  0xE3    // Digital gamma look-up table for blue
#define ST77xx_CMD_GATE_CONTROL_2                   0xE4    // Gate control
#define ST77xx_CMD_SPI2_EN                          0xE7    // SPI2 enable
#define ST77xx_CMD_POWER_CONTROL_2                  0xE8    // Power control 2
#define ST77xx_CMD_EQUALIZE_TIME_CONTROL            0xE9    // Equalize time control
#define ST77xx_CMD_PROGRAM_CONTROL                  0xEC    // Program control

#define ST77xx_CMD_PROGRAM_MODE_EN                  0xFA    // Program mode enable
#define ST77xx_CMD_NVM_SETTING                      0xFC    // NVM setting
#define ST77xx_CMD_PROGRAM_ACTION                   0xFE    // Program action
#endif //DISPLAY_ST7789

#if defined(DISPLAY_ST7796S)
#define ST77xx_CMD_INTERFACE_MODE_CONTROL           0xB0
#define ST77xx_CMD_FRAME_RATE_CONTROL_1             0xB1
#define ST77xx_CMD_FRAME_RATE_CONTROL_2             0xB2
#define ST77xx_CMD_FRAME_RATE_CONTROL_3             0xB3
#define ST77xx_CMD_DISPLAY_INVERSION_CONTROL        0xB4
#define ST77xx_CMD_BLANKING_PORCH_CONTROL           0xB5
#define ST77xx_CMD_DISPLAY_FUNCTION_CONTROL         0xB6
#define ST77xx_CMD_ENTRY_MODE_SET                   0xB7

#define ST77xx_CMD_POWER_CONTROL_1                  0xC0
#define ST77xx_CMD_POWER_CONTROL_2                  0xC1
#define ST77xx_CMD_POWER_CONTROL_3                  0xC2
#define ST77xx_CMD_VCOM_CONTROL                     0xC5
#define ST77xx_CMD_VCOM_OFFSET_SET                  0xC6

#define ST77xx_CMD_NVM_ADDRESS_DATA_WRITE           0xD0
#define ST77xx_CMD_NVM_BYTE_PROGRAM                 0xD1
#define ST77xx_CMD_NVM_READ                         0xD2
#define ST77xx_CMD_READ_ID4                         0xD3

#define ST77xx_CMD_GAMMA_CORRECTION_POSTIVE         0xE0
#define ST77xx_CMD_GAMMA_CORRECTION_NEGATIVE        0xE1
#define ST77xx_CMD_DIGITAL_GAMMA_CONTROL_1          0xE2
#define ST77xx_CMD_DIGITAL_GAMMA_CONTROL_2          0xE3
#define ST77xx_CMD_DISPLAY_OUTPUT_CONTROL_ADJUST    0xE8

#define ST77xx_CMD_COMMAND_SET_CONTROL              0xF0
#define ST77xx_CMD_SPI_READ_CONTROL                 0xFB
#endif //DISPLAY_ST7796S

#define ST77xx_MADCTL_MY    0x80
#define ST77xx_MADCTL_MX    0x40
#define ST77xx_MADCTL_MV    0x20
#define ST77xx_MADCTL_ML    0x10
#define ST77xx_MADCTL_RGB   0x00
#define ST77xx_MADCTL_BGR   0x08
// Delay between some initialisation commands

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes;
} ST77xx_Init_Command;

#ifdef DISPLAY_ST7735
ST77xx_Init_Command ST7735_Init_Cmds[]={
    {ST77xx_CMD_SOFTWARE_RESET, {0}, 0x80},         			// Software reset, 0 args, w/delay 150
    {ST77xx_CMD_SLEEP_OUT, {0}, 0x80},                 			// Out of sleep mode, 0 args, w/delay 500
    {ST77xx_CMD_FRAME_RATE_CONTROL_1, {0x01, 0x2C, 0x2D}, 3},   // Frame rate ctrl - normal mode, 3 args: Rate = fosc/(1x2+40) * (LINE+2C+2D)
    {ST77xx_CMD_FRAME_RATE_CONTROL_2, {0x01, 0x2C, 0x2D}, 3},   // Frame rate control - idle mode, 3 args:Rate = fosc/(1x2+40) * (LINE+2C+2D)
    {ST77xx_CMD_FRAME_RATE_CONTROL_3, {0x01, 0x2C, 0x2D,  		//Frame rate ctrl - partial mode, 6 args:Dot inversion mode. Line inversion mode
                                        0x01, 0x2C, 0x2D}, 6},
#ifdef CONFIG_DISPLAY_ROTATE_180
    {ST77xx_CMD_MEMORY_ACCESS_CONTROL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
    {ST77xx_CMD_MEMORY_ACCESS_CONTROL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
    {ST77xx_CMD_INVERSION_CONTROL, {0x07}, 1},                  // Display inversion ctrl, 1 arg, no delay:No inversion
    {ST77xx_CMD_POWER_CONTROL_1, {0xA2,0x02, 0x84}, 3},      	// Power control, 3 args, no delay:-4.6V AUTO mode
    {ST77xx_CMD_POWER_CONTROL_2, {0xC5}, 1},                 	// Power control, 1 arg, no delay:VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    {ST77xx_CMD_POWER_CONTROL_3, {0x0A, 0x00}, 2},           	// Power control, 2 args, no delay: Opamp current small, Boost frequency
    {ST77xx_CMD_POWER_CONTROL_4, {0x8A,0x2A }, 2},           	// Power control, 2 args, no delay: BCLK/2, Opamp current small & Medium low
    {ST77xx_CMD_POWER_CONTROL_5, {0x8A, 0xEE}, 2},           	// Power control, 2 args, no delay:
    {ST77xx_CMD_VCOM_CONTROL_1, {0x0E}, 1},                 	// Power control, 1 arg, no delay:
#ifdef CONFIG_DISPLAY_INVERTED
    {ST77xx_CMD_DISP_INVERSION_ON, {0}, 0},                     // set inverted mode
#else //CONFIG_DISPLAY_INVERTED
    {ST77xx_CMD_DISP_INVERSION_OFF, {0}, 0},                    // set non-inverted mode
#endif //CONFIG_DISPLAY_INVERTED
    {ST77xx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x05}, 1},            // set color mode, 1 arg, no delay: 16-bit color
    {ST77xx_CMD_GAMMA_CORRECTION_POSTIVE_1,						// 16 args, no delay:
        {0x02, 0x1c, 0x07, 0x12,
        0x37, 0x32, 0x29, 0x2d,
        0x29, 0x25, 0x2B, 0x39,
        0x00, 0x01, 0x03, 0x10}, 16},
    {ST77xx_CMD_GAMMA_CORRECTION_NEGATIVE_1,   			        // 16 args, no delay:
        {0x03, 0x1d, 0x07, 0x06,
        0x2E, 0x2C, 0x29, 0x2D,
        0x2E, 0x2E, 0x37, 0x3F,
        0x00, 0x00, 0x02, 0x10}, 16},
    {ST77xx_CMD_NORMAL_DISP_MODE_ON, {0}, 0x80},      // Normal display on, no args, w/delay 10 ms delay
    {ST77xx_CMD_DISPLAY_ON, {0}, 0x80},       		// Main screen turn on, no args w/delay 100 ms delay
    {0, {0}, 0xff}
};
#endif //DISPLAY_ST7735

#ifdef DISPLAY_ST7789
ST77xx_Init_Command ST7789_Init_Cmds[] = {
    {0xCF, {0x00, 0x83, 0X30}, 3},
    {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
    {ST77xx_CMD_POWER_CONTROL_2, {0x85, 0x01, 0x79}, 3},
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    {0xF7, {0x20}, 1},
    {0xEA, {0x00, 0x00}, 2},
    {ST77xx_CMD_LCM_CONTROL, {0x26}, 1},
    {ST77xx_CMD_ID_SETTING, {0x11}, 1},
    {ST77xx_CMD_VCOMS_OFFSET_SET, {0x35, 0x3E}, 2},
    {ST77xx_CMD_CABC_CONTROL, {0xBE}, 1},
#ifdef CONFIG_DISPLAY_ROTATE_180
        {ST77xx_CMD_MEMORY_ACCESS_CONTROL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
        {ST77xx_CMD_MEMORY_ACCESS_CONTROL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
    {ST77xx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x55}, 1},
#ifdef CONFIG_DISPLAY_INVERTED
    {ST77xx_CMD_DISP_INVERSION_ON, {0}, 0},                     // set inverted mode
#else //CONFIG_DISPLAY_INVERTED
    {ST77xx_CMD_DISP_INVERSION_OFF, {0}, 0},                    // set non-inverted mode
#endif //CONFIG_DISPLAY_INVERTED
    {ST77xx_CMD_RGB_CONTROL, {0x00, 0x1B}, 2},
    {0xF2, {0x08}, 1},
    {ST77xx_CMD_GAMSET, {0x01}, 1},
    {ST77xx_CMD_POSTIVE_VOLTAGE_GAMMA_CONTROL,
                {0xD0, 0x00, 0x02, 0x07, 
                0x0A, 0x28, 0x32, 0x44, 
                0x42, 0x06, 0x0E, 0x12, 
                0x14, 0x17}, 14},
    {ST77xx_CMD_NEGATIVE_VOLTAGE_GAMMA_CONTROL,
                {0xD0, 0x00, 0x02, 0x07, 
                0x0A, 0x28, 0x31, 0x54, 
                0x47, 0x0E, 0x1C, 0x17, 
                0x1B, 0x1E}, 14},
    {ST77xx_CMD_COLUMN_ADDRESS_SET, {0x00, 0x00, 0x00, 0xEF}, 4},
    {ST77xx_CMD_PAGE_ADDRESS_SET, {0x00, 0x00, 0x01, 0x3f}, 4},
    {ST77xx_CMD_MEMORY_WRITE, {0}, 0},
    {ST77xx_CMD_GATE_CONTROL_1, {0x07}, 1},
    {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
    {ST77xx_CMD_SLEEP_OUT, {0}, 0x80},
    {ST77xx_CMD_DISPLAY_ON, {0}, 0x80},
    {0, {0}, 0xff},
};
#endif //DISPLAY_ST7789

#ifdef DISPLAY_ST7796S
ST77xx_Init_Command ST7796S_Init_Cmds[] = {
    {0xCF, {0x00, 0x83, 0X30}, 3},
    {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
    {ST77xx_CMD_DISPLAY_OUTPUT_CONTROL_ADJUST, {0x85, 0x01, 0x79}, 3},
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    {0xF7, {0x20}, 1},
    {0xEA, {0x00, 0x00}, 2},
    {ST77xx_CMD_POWER_CONTROL_1, {0x26}, 1},		 /*Power control*/
    {ST77xx_CMD_POWER_CONTROL_2, {0x11}, 1},		 /*Power control */
    {ST77xx_CMD_VCOM_CONTROL, {0x35, 0x3E}, 2}, /*VCOM control*/
    {0xC7, {0xBE}, 1},		 /*VCOM control*/
#ifdef CONFIG_DISPLAY_ROTATE_180
    {ST77xx_CMD_MEMORY_ACCESS_CONTROL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
    {ST77xx_CMD_MEMORY_ACCESS_CONTROL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
#ifdef CONFIG_DISPLAY_INVERTED
    {ST77xx_CMD_DISP_INVERSION_ON, {0}, 0},                     // set inverted mode
#else //CONFIG_DISPLAY_INVERTED
    {ST77xx_CMD_DISP_INVERSION_OFF, {0}, 0},                    // set non-inverted mode
#endif //CONFIG_DISPLAY_INVERTED
    {ST77xx_CMD_COLMOD_PIXEL_FORMAT_SET, {0x55}, 1},		 /*Pixel Format Set*/
    {ST77xx_CMD_FRAME_RATE_CONTROL_1, {0x00, 0x1B}, 2},
    {0xF2, {0x08}, 1},
    {ST77xx_CMD_GAMSET, {0x01}, 1},
    {ST77xx_CMD_GAMMA_CORRECTION_POSTIVE, 
                    {0x1F, 0x1A, 0x18, 0x0A, 
                    0x0F, 0x06, 0x45, 0X87, 
                    0x32, 0x0A, 0x07, 0x02, 
                    0x07, 0x05, 0x00}, 15},
    {ST77xx_CMD_GAMMA_CORRECTION_NEGATIVE, 
                    {0x00, 0x25, 0x27, 0x05, 
                    0x10, 0x09, 0x3A, 0x78, 
                    0x4D, 0x05, 0x18, 0x0D, 
                    0x38, 0x3A, 0x1F}, 15},
    {ST77xx_CMD_COLUMN_ADDRESS_SET, {0x00, 0x00, 0x00, 0xEF}, 4},
    {ST77xx_CMD_PAGE_ADDRESS_SET, {0x00, 0x00, 0x01, 0x3f}, 4},
    {ST77xx_CMD_MEMORY_WRITE, {0}, 0},
    {ST77xx_CMD_ENTRY_MODE_SET, {0x07}, 1},
    {ST77xx_CMD_DISPLAY_FUNCTION_CONTROL, {0x0A, 0x82, 0x27, 0x00}, 4},
    {ST77xx_CMD_SLEEP_OUT, {0}, 0x80},
    {ST77xx_CMD_DISPLAY_ON, {0}, 0x80},
    {0, {0}, 0xff},
};
#endif //DISPLAY_ST7796S

#if defined(DISPLAY_ST7735) || defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
uint16_t ST77xx_VerticalScrollingStartAddress = 0;
#endif // DISPLAY_ST7735 || DISPLAY_ST7789 || DISPLAY_ST7796S

static ST77xx_OpResult ST77xx_Map_GPIO_Error_Code(hwGPIO_OpStatus error_code)
{
    switch(error_code)
    {
        case hwGPIO_OK:
                return ST77xx_OK;
        
        case hwGPIO_InvalidParameter:
                return ST77xx_InvalidParameter;
        
        case hwGPIO_HW_Error:
        case hwGPIO_PinConflict:
                return ST77xx_HwError;
        
        case hwGPIO_Unsupport:
        default:
                return ST77xx_Unsupport;
    }
}

static ST77xx_OpResult ST77xx_Map_SPI_Error_Code(hwSPI_OpResult error_code)
{
    switch(error_code)
    {
        case hwSPI_OK:
                return ST77xx_OK;
        
        case hwSPI_NotInit:
                return ST77xx_NotInit;
        
        case hwSPI_InvalidParameter:
                return ST77xx_InvalidParameter;
        
        case hwSPI_MemoryError:
                return ST77xx_MemoryError;
        
        case hwSPI_MutexTimeout:
                return ST77xx_MutexTimeout;
        
        case hwSPI_SlaveTimeout:
                return ST77xx_SlaveTimeout;
        
        case hwSPI_Unsupport:
        default:
                return ST77xx_Unsupport;
    }
}

static ST77xx_OpResult ST77xx_IO_Init()
{
    hwGPIO_OpStatus gpio_op_result;
    hwSPI_OpResult spi_op_result;
    
    gpio_op_result = GPIO_Pin_Init(CONFIG_ST77XX_CS_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Init(CONFIG_ST77XX_DC_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_result<hwGPIO_OK)
    {
        GPIO_Pin_DeInit(CONFIG_ST77XX_CS_PN);
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Init(CONFIG_ST77XX_RST_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_result<hwGPIO_OK)
    {
        GPIO_Pin_DeInit(CONFIG_ST77XX_CS_PN);
        GPIO_Pin_DeInit(CONFIG_ST77XX_DC_PN);
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    spi_op_result = SPI_Master_Init(CONFIG_ST77XX_SPI_INDEX, CONFIG_ST77XX_SPI_CLOCK, CONFIG_ST77XX_SPI_MODE, false);
    if(spi_op_result<hwSPI_OK)
    {
        GPIO_Pin_DeInit(CONFIG_ST77XX_CS_PN);
        GPIO_Pin_DeInit(CONFIG_ST77XX_DC_PN);
        GPIO_Pin_DeInit(CONFIG_ST77XX_RST_PN);
        return ST77xx_Map_SPI_Error_Code(spi_op_result);
    }

    return ST77xx_OK;
}

static ST77xx_OpResult ST77xx_Reset()
{
    hwGPIO_OpStatus gpio_op_result;

    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_RST_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    NeonRTOS_Sleep(10);
    
    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_RST_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    NeonRTOS_Sleep(100);

    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_RST_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    NeonRTOS_Sleep(100);

    return ST77xx_OK;
}

static ST77xx_OpResult ST77xx_IO_Write(uint8_t cmd, uint8_t* param_data, uint16_t param_length)
{
    hwGPIO_OpStatus gpio_op_result;
    hwSPI_OpResult spi_op_result;

    if(param_data==NULL && param_length>0)
    {
        return ST77xx_InvalidParameter;
    }
            
    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_CS_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_DC_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    spi_op_result = SPI_Master_WriteByte(CONFIG_ST77XX_SPI_INDEX, cmd);
    if(spi_op_result<hwSPI_OK)
    {
        return ST77xx_Map_SPI_Error_Code(spi_op_result);
    }

    if(param_length==0)
    {
        gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_CS_PN, 1);
        if(gpio_op_result<hwGPIO_OK)
        {
            return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
        }

        return ST77xx_OK; 
    }
            
    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_DC_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    for(uint16_t i = 0; i<param_length; i++)
    {
        spi_op_result = SPI_Master_WriteByte(CONFIG_ST77XX_SPI_INDEX, param_data[i]);
        if(spi_op_result<hwSPI_OK)
        {
            return ST77xx_Map_SPI_Error_Code(spi_op_result);
        }
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_CS_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    return ST77xx_OK;
}

static ST77xx_OpResult ST77xx_Nenory_Write(ST77xx_Color16_RGB565* data, uint16_t data_length)
{
    hwGPIO_OpStatus gpio_op_result;
    hwSPI_OpResult spi_op_result;

    if(data==NULL && data_length>0)
    {
        return ST77xx_InvalidParameter;
    }
            
    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_CS_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_DC_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    spi_op_result = SPI_Master_WriteByte(CONFIG_ST77XX_SPI_INDEX, ST77xx_CMD_MEMORY_WRITE);
    if(spi_op_result<hwSPI_OK)
    {
        return ST77xx_Map_SPI_Error_Code(spi_op_result);
    }
  
    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_DC_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    for(uint16_t i = 0; i<data_length; i++)
    {
        uint8_t buff[2];
        buff[0] = ((uint16_t)data[i].full)>>8;
        buff[1] = ((uint16_t)data[i].full)&0xff;
        spi_op_result = SPI_Master_WriteByte(CONFIG_ST77XX_SPI_INDEX, buff[0]);
        if(spi_op_result<hwSPI_OK)
        {
            return ST77xx_Map_SPI_Error_Code(spi_op_result);
        }
        spi_op_result = SPI_Master_WriteByte(CONFIG_ST77XX_SPI_INDEX, buff[1]);
        if(spi_op_result<hwSPI_OK)
        {
            return ST77xx_Map_SPI_Error_Code(spi_op_result);
        }
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_ST77XX_CS_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return ST77xx_Map_GPIO_Error_Code(gpio_op_result);
    }

    return ST77xx_OK;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

ST77xx_OpResult ST77xx_Init(void)
{
    ST77xx_OpResult op_status;

    op_status = ST77xx_IO_Init();
    if(op_status<ST77xx_OK) { return op_status; }

    //Reset the display
    op_status = ST77xx_Reset();
    if(op_status<ST77xx_OK) { return op_status; }

    //Send all the commands
    ST77xx_Init_Command* pST77xx_Init_Cmds = NULL;

#ifdef DISPLAY_ST7735
    pST77xx_Init_Cmds = ST7735_Init_Cmds;
#endif // DISPLAY_ST7735
#ifdef DISPLAY_ST7789
    pST77xx_Init_Cmds = ST7789_Init_Cmds;
#endif // DISPLAY_ST7789
#ifdef DISPLAY_ST7796S
    pST77xx_Init_Cmds = ST7796S_Init_Cmds;
#endif // DISPLAY_ST7796S

    if(pST77xx_Init_Cmds!=NULL)
    {
        uint16_t cmd = 0;
        while (pST77xx_Init_Cmds[cmd].databytes!=0xff)
        {
            op_status = ST77xx_IO_Write(pST77xx_Init_Cmds[cmd].cmd, pST77xx_Init_Cmds[cmd].data, pST77xx_Init_Cmds[cmd].databytes&0x1F);
            if(op_status<ST77xx_OK) { break; }

            if (pST77xx_Init_Cmds[cmd].databytes & 0x80) {
                NeonRTOS_Sleep(120);
            }

            cmd++;
        }
    }

    return op_status;
}

ST77xx_OpResult ST77xx_Power_On()
{
    ST77xx_OpResult op_status;
    
#if defined(DISPLAY_ST7735) || defined(DISPLAY_ST7789)
    op_status = ST77xx_IO_Write(ST77xx_CMD_SLEEP_OUT, NULL, 0);
    if(op_status<ST77xx_OK) { return op_status; }

    NeonRTOS_Sleep(120);
#endif // DISPLAY_ST7735 || DISPLAY_ST7789

#ifdef DISPLAY_ST7796S
    uint8_t data[] = {0x08};

    op_status = ST77xx_IO_Write(ST77xx_CMD_SLEEP_OUT, data, 1);
    if(op_status<ST77xx_OK) { return op_status; }

    NeonRTOS_Sleep(120);
#endif // DISPLAY_ST7796S

#if defined(DISPLAY_ST7735) || defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
    op_status = ST77xx_IO_Write(ST77xx_CMD_DISPLAY_ON, NULL, 0);
    if(op_status<ST77xx_OK) { return op_status; }
    
    NeonRTOS_Sleep(120);
#endif // DISPLAY_ST7735 || DISPLAY_ST7789 || DISPLAY_ST7796S

#if defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
	uint8_t scroll_cfg[2];

    scroll_cfg[0] = ST77xx_VerticalScrollingStartAddress >> 8;
    scroll_cfg[1] = ST77xx_VerticalScrollingStartAddress & 0xFF;

    op_status = ST77xx_IO_Write(ST77xx_CMD_VERT_SCROLL_START_ADDRESS, scroll_cfg, 2);
    if(op_status<ST77xx_OK) { return op_status; }
#endif // DISPLAY_ST7789 || DISPLAY_ST7796S
    
    return op_status;
}

ST77xx_OpResult ST77xx_Power_Off()
{
    ST77xx_OpResult op_status;
    
#if defined(DISPLAY_ST7735) || defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
    op_status = ST77xx_IO_Write(ST77xx_CMD_DISPLAY_OFF, NULL, 0);
    if(op_status<ST77xx_OK) { return op_status; }
    
    NeonRTOS_Sleep(120);
#endif // DISPLAY_ST7735 || DISPLAY_ST7789 || DISPLAY_ST7796S

#if defined(DISPLAY_ST7735) || defined(DISPLAY_ST7789)
    op_status = ST77xx_IO_Write(ST77xx_CMD_ENTER_SLEEP_MODE, NULL, 0);
    if(op_status<ST77xx_OK) { return op_status; }
#endif // DISPLAY_ST7735 || DISPLAY_ST7789

#ifdef DISPLAY_ST7796S
    uint8_t data[] = {0x08};

    op_status = ST77xx_IO_Write(ST77xx_CMD_ENTER_SLEEP_MODE, data, 1);
    if(op_status<ST77xx_OK) { return op_status; }
#endif // DISPLAY_ST7796S

    return ST77xx_OK;
}

ST77xx_OpResult ST77xx_SetWindow(int16_t x1, int16_t x2, int16_t y1, int16_t y2)
{
    uint8_t set_x_index[4] = {0};
    uint8_t set_y_index[4] = {0};
        
    ST77xx_OpResult op_status;

    set_x_index[0] = (x1 >> 8) & 0xFF;
    set_x_index[1] = x1 & 0xFF;
    set_x_index[2] = (x2 >> 8) & 0xFF;
    set_x_index[3] = x2 & 0xFF;

    set_y_index[0] = (y1 >> 8) & 0xFF;
    set_y_index[1] = y1 & 0xFF;
    set_y_index[2] = (y2 >> 8) & 0xFF;
    set_y_index[3] = y2 & 0xFF;

    /*Column addresses*/
    op_status = ST77xx_IO_Write(ST77xx_CMD_COLUMN_ADDRESS_SET, set_x_index, 4);
    if(op_status<ST77xx_OK) { return op_status; }
    
    /*Page addresses*/
    op_status = ST77xx_IO_Write(ST77xx_CMD_PAGE_ADDRESS_SET, set_y_index, 4);
    if(op_status<ST77xx_OK) { return op_status; }
    
    return ST77xx_OK;
}

ST77xx_OpResult ST77xx_VerticalScroll_Definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines)
{
    ST77xx_OpResult op_status;

    uint8_t scroll_cfg[6];

    scroll_cfg[0] = topFixedLines >> 8;
    scroll_cfg[1] = topFixedLines & 0xFF;
    scroll_cfg[2] = scrollLines >> 8;
    scroll_cfg[3] = scrollLines & 0xFF;
    scroll_cfg[4] = bottomFixedLines >> 8;
    scroll_cfg[5] = bottomFixedLines & 0xFF;

    op_status = ST77xx_IO_Write(ST77xx_CMD_VERT_SCROLL_DEFINITION, scroll_cfg, 6);

    return ST77xx_OK;
}

ST77xx_OpResult ST77xx_VerticalScroll_StartLine(uint16_t startLine)
{
    ST77xx_OpResult op_status;

#if defined(DISPLAY_ST7789) || defined(DISPLAY_ST7796S)
	uint8_t scroll_cfg[2];

    scroll_cfg[0] = startLine >> 8;
    scroll_cfg[1] = startLine & 0xFF;

    op_status = ST77xx_IO_Write(ST77xx_CMD_VERT_SCROLL_START_ADDRESS, scroll_cfg, 2);
    if(op_status<ST77xx_OK) { return op_status; }

    ST77xx_VerticalScrollingStartAddress = startLine;
#endif // DISPLAY_ST7789 || DISPLAY_ST7796S
    
    return ST77xx_OK;
}

ST77xx_OpResult ST77xx_Draw(int16_t x1, int16_t x2, int16_t y1, int16_t y2, ST77xx_Color16_RGB565* data)
{
    uint32_t size = (abs(x2-x1) + 1) * (abs(y2-y1) + 1);
    
    ST77xx_OpResult op_status;

    op_status = ST77xx_SetWindow(x1, x2, y1, y2);
    if(op_status<ST77xx_OK) { return op_status; }
    
    /*Memory write*/
    op_status = ST77xx_Nenory_Write(data, size);
    if(op_status<ST77xx_OK) { return op_status; }
    
    return ST77xx_OK;
}

ST77xx_OpResult ST77xx_DrawPixel(int16_t x, int16_t y, ST77xx_Color16_RGB565* data)
{
    return ST77xx_Draw(x, x, y, y, data);
}
