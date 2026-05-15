
/*********************
 *      INCLUDES
 *********************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "NeonRTOS.h"

#include "GC9A01.h"

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

/*
 * Initialization sequence based on vendor datasheet
 * and publicly available examples.
 */
/* Level 1 Commands*/
#define GC9A01_CMD_READ_DISPLAY_ID                0x04
#define GC9A01_CMD_READ_DISPLAY_STATUS            0x09

#define GC9A01_CMD_ENTER_SLEEP_MODE               0x10
#define GC9A01_CMD_SLEEP_OUT                      0x11
#define GC9A01_CMD_PARTIAL_MODE_ON                0x12
#define GC9A01_CMD_NORMAL_DISP_MODE_ON            0x13

#define GC9A01_CMD_DISP_INVERSION_OFF             0x20
#define GC9A01_CMD_DISP_INVERSION_ON              0x21
#define GC9A01_CMD_DISPLAY_OFF                    0x28
#define GC9A01_CMD_DISPLAY_ON                     0x29
#define GC9A01_CMD_COLUMN_ADDRESS_SET             0x2A
#define GC9A01_CMD_PAGE_ADDRESS_SET               0x2B
#define GC9A01_CMD_MEMORY_WRITE                   0x2C

#define GC9A01_CMD_PARTIAL_AREA                   0x30
#define GC9A01_CMD_VERT_SCROLL_DEFINITION         0x33 // Vertical scrolling definition (GC9A01_CMDV)
#define GC9A01_CMD_TEARING_EFFECT_LINE_OFF        0x34 // Tearing effect line off
#define GC9A01_CMD_TEARING_EFFECT_LINE_ON         0x35 // Tearing effect line on
#define GC9A01_CMD_MEMORY_ACCESS_CONTROL          0x36 // Memory data access control bits 7:3,1:0 only
#define GC9A01_CMD_VERT_SCROLL_START_ADDRESS      0x37
#define GC9A01_CMD_IDLE_MODE_OFF                  0x38 // Idle mode off
#define GC9A01_CMD_IDLE_MODE_ON                   0x39 // Idle mode on
#define GC9A01_CMD_COLMOD_PIXEL_FORMAT_SET        0x3A
#define GC9A01_CMD_WRITE_MEMORY_CONTINUE          0x3C // Memory write continue

#define GC9A01_CMD_SET_TEAR_SCANLINE              0x44
#define GC9A01_CMD_GET_SCANLINE                   0x45

#define GC9A01_CMD_WRITE_DISPLAY_BRIGHTNESS       0x51
#define GC9A01_CMD_WRITE_CTRL_DISPLAY             0x53

#define GC9A01_CMD_READ_ID1                       0xDA
#define GC9A01_CMD_READ_ID2                       0xDB
#define GC9A01_CMD_READ_ID3                       0xDC

/* Level 2 Commands */

#define GC9A01_CMD_RGB_INTERFACE_SIGNAL_CONTROL     0xB0
#define GC9A01_CMD_BLANKING_PORCH_CONTROL           0xB5
#define GC9A01_CMD_DISPLAY_FUNCTION_CONTROL         0xB6
#define GC9A01_CMD_TEARING_EFFECT_CONTROL           0xBA
#define GC9A01_CMD_INTERFACE_CONTROL                0xF6

/* Level 3 Commands */

#define GC9A01_CMD_FRAME_RATE                       0xE8
#define GC9A01_CMD_SPI_2_DATA_CONTROL               0xE9

#define GC9A01_CMD_POWER_CONTROL_1                  0xC1
#define GC9A01_CMD_POWER_CONTROL_2                  0xC3
#define GC9A01_CMD_POWER_CONTROL_3                  0xC4
#define GC9A01_CMD_POWER_CONTROL_4                  0xC9
#define GC9A01_CMD_POWER_CONTROL_7                  0xA7

#define GC9A01_CMD_INTER_REGISTER_ENABLE_1          0xFE
#define GC9A01_CMD_INTER_REGISTER_ENABLE_2          0xEF

#define GC9A01_CMD_SET_GAMMA_1                      0xF0
#define GC9A01_CMD_SET_GAMMA_2                      0xF1
#define GC9A01_CMD_SET_GAMMA_3                      0xF2
#define GC9A01_CMD_SET_GAMMA_4                      0xF3

typedef struct GC9A01_Init_Command_t{
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes;
} GC9A01_Init_Command;

uint16_t GC9A01_VerticalScrollingStartAddress = 0;

static const GC9A01_Init_Command GC9A01_Init_Cmds[]={
    {GC9A01_CMD_INTER_REGISTER_ENABLE_2, {0}, 0},
    {0xEB, {0x14}, 1},

    {GC9A01_CMD_INTER_REGISTER_ENABLE_1, {0}, 0},
    {GC9A01_CMD_INTER_REGISTER_ENABLE_2, {0}, 0},

    {0xEB, {0x14}, 1},
    {0x84, {0x40}, 1},
    {0x85, {0xFF}, 1},
    {0x86, {0xFF}, 1},
    {0x87, {0xFF}, 1},
    {0x88, {0x0A}, 1},
    {0x89, {0x21}, 1},
    {0x8A, {0x00}, 1},
    {0x8B, {0x80}, 1},
    {0x8C, {0x01}, 1},
    {0x8D, {0x01}, 1},
    {0x8E, {0xFF}, 1},
    {0x8F, {0xFF}, 1},
    {GC9A01_CMD_DISPLAY_FUNCTION_CONTROL, {0x00, 0x20}, 2},
    //call orientation
#ifdef GC9A01_ROTATE_180
    {GC9A01_CMD_MEMORY_ACCESS_CONTROL, {0x88}, 1},
#else //GC9A01_ROTATE_180
    {GC9A01_CMD_MEMORY_ACCESS_CONTROL, {0x48}, 1},
#endif //GC9A01_ROTATE_180
    {GC9A01_CMD_COLMOD_PIXEL_FORMAT_SET, {0x05}, 1},
#ifdef CONFIG_DISPLAY_INVERTED
    {GC9A01_CMD_DISP_INVERSION_ON, {0}, 0}, // set inverted mode
#else //CONFIG_DISPLAY_INVERTED
    {GC9A01_CMD_DISP_INVERSION_OFF, {0}, 0}, // set non-inverted mode
#endif //CONFIG_DISPLAY_INVERTED
    {0x90, {0x08, 0x08, 0X08, 0X08}, 4},
    {0xBD, {0x06}, 1},
    {0xBC, {0x00}, 1},
    {0xFF, {0x60, 0x01, 0x04}, 3},
    {GC9A01_CMD_POWER_CONTROL_2, {0x13}, 1},
    {GC9A01_CMD_POWER_CONTROL_3, {0x13}, 1},
    {GC9A01_CMD_POWER_CONTROL_4, {0x22}, 1},
    {0xBE, {0x11}, 1},
    {0xE1, {0x10, 0x0E}, 2},
    {0xDF, {0x21, 0x0C, 0x02}, 3},
    {GC9A01_CMD_SET_GAMMA_1, {0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6},
    {GC9A01_CMD_SET_GAMMA_2, {0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6},
    {GC9A01_CMD_SET_GAMMA_3, {0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6},
    {GC9A01_CMD_SET_GAMMA_4, {0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6},
    {0xED, {0x1B, 0x0B}, 2},
    {0xAE, {0x77}, 1},
    {0xCD, {0x63}, 1},
    {0x70, {0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0X08, 0x03}, 9},
    {GC9A01_CMD_FRAME_RATE, {0x34}, 1},
    {0x62, {0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0X0F, 0x71, 0xEF, 0x70, 0x70}, 12},
    {0x63, {0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0X13, 0x71, 0xF3, 0x70, 0x70}, 12},
    {0x64, {0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07}, 7},
    {0x66, {0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0X00, 0x00, 0x00}, 10},
    {0x67, {0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0X10, 0x32, 0x98}, 10},
    {0x74, {0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00}, 7},
    {0x98, {0x3E, 0x07}, 2},
    {GC9A01_CMD_TEARING_EFFECT_LINE_ON, {0}, 0},
    {GC9A01_CMD_SLEEP_OUT, {0}, 0x80},	//0x80 delay flag
    {GC9A01_CMD_DISPLAY_ON, {0}, 0x80},	//0x80 delay flag
    {0, {0}, 0xff},		//init end flag
};

static GC9A01_OpResult GC9A01_Map_GPIO_Error_Code(hwGPIO_OpResult error_code)
{
  switch(error_code)
  {
  case hwGPIO_OK:
    return GC9A01_OK;
    
  case hwGPIO_InvalidParameter:
    return GC9A01_InvalidParameter;
    
  case hwGPIO_HW_Error:
  case hwGPIO_PinConflict:
    return GC9A01_HwError;
    
  case hwGPIO_Unsupport:
  default:
    return GC9A01_Unsupport;
  }
}

static GC9A01_OpResult GC9A01_Map_SPI_Error_Code(hwSPI_OpResult error_code)
{
  switch(error_code)
  {
  case hwSPI_OK:
    return GC9A01_OK;
    
  case hwSPI_NotInit:
    return GC9A01_NotInit;
    
  case hwSPI_InvalidParameter:
    return GC9A01_InvalidParameter;
    
  case hwSPI_MemoryError:
    return GC9A01_MemoryError;
    
  case hwSPI_MutexTimeout:
    return GC9A01_MutexTimeout;
    
  case hwSPI_SlaveTimeout:
    return GC9A01_SlaveTimeout;
    
  case hwSPI_Unsupport:
  default:
    return GC9A01_Unsupport;
  }
}

static GC9A01_OpResult GC9A01_IO_Init()
{
    hwGPIO_OpResult gpio_op_result;
    hwSPI_OpResult spi_op_result;
    
    gpio_op_result = GPIO_Pin_Init(CONFIG_GC9A01_CS_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Init(CONFIG_GC9A01_DC_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_result<hwGPIO_OK)
    {
        GPIO_Pin_DeInit(CONFIG_GC9A01_CS_PN);
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Init(CONFIG_GC9A01_RST_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_result<hwGPIO_OK)
    {
        GPIO_Pin_DeInit(CONFIG_GC9A01_CS_PN);
        GPIO_Pin_DeInit(CONFIG_GC9A01_DC_PN);
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    spi_op_result = SPI_Master_Init(CONFIG_GC9A01_SPI_INDEX, CONFIG_GC9A01_SPI_CLOCK, CONFIG_GC9A01_SPI_MODE, false);
    if(spi_op_result<hwSPI_OK)
    {
        GPIO_Pin_DeInit(CONFIG_GC9A01_CS_PN);
        GPIO_Pin_DeInit(CONFIG_GC9A01_DC_PN);
        GPIO_Pin_DeInit(CONFIG_GC9A01_RST_PN);
        return GC9A01_Map_SPI_Error_Code(spi_op_result);
    }

    return GC9A01_OK;
}

static GC9A01_OpResult GC9A01_Reset()
{
    hwGPIO_OpResult gpio_op_result;

    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_RST_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    NeonRTOS_Sleep(10);
    
    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_RST_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    NeonRTOS_Sleep(100);

    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_RST_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    NeonRTOS_Sleep(100);

    return GC9A01_OK;
}

static GC9A01_OpResult GC9A01_IO_Write(uint8_t cmd, uint8_t* param_data, uint16_t param_length)
{
    hwGPIO_OpResult gpio_op_result;
    hwSPI_OpResult spi_op_result;

    if(param_data==NULL && param_length>0)
    {
        return GC9A01_InvalidParameter;
    }
	
    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_CS_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_DC_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    spi_op_result = SPI_Master_WriteByte(CONFIG_GC9A01_SPI_INDEX, cmd);
    if(spi_op_result<hwSPI_OK)
    {
        return GC9A01_Map_SPI_Error_Code(spi_op_result);
    }

    if(param_length==0)
    {
        gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_CS_PN, 1);
        if(gpio_op_result<hwGPIO_OK)
        {
                return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
        }

        return GC9A01_OK; 
    }
	
    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_DC_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    for(uint16_t i = 0; i<param_length; i++)
    {
        spi_op_result = SPI_Master_WriteByte(CONFIG_GC9A01_SPI_INDEX, param_data[i]);
        if(spi_op_result<hwSPI_OK)
        {
            return GC9A01_Map_SPI_Error_Code(spi_op_result);
        }
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_CS_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    return GC9A01_OK;
}

static GC9A01_OpResult GC9A01_Nenory_Write(GC9A01_Color16_RGB565* data, uint16_t data_length)
{
    hwGPIO_OpResult gpio_op_result;
    hwSPI_OpResult spi_op_result;

    if(data==NULL && data_length>0)
    {
        return GC9A01_InvalidParameter;
    }
            
    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_CS_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_DC_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    spi_op_result = SPI_Master_WriteByte(CONFIG_GC9A01_SPI_INDEX, GC9A01_CMD_MEMORY_WRITE);
    if(spi_op_result<hwSPI_OK)
    {
        return GC9A01_Map_SPI_Error_Code(spi_op_result);
    }
  
    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_DC_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    for(uint16_t i = 0; i<data_length; i++)
    {
        uint8_t buff[2];
        buff[0] = ((uint16_t)data[i].full)>>8;
        buff[1] = ((uint16_t)data[i].full)&0xff;
        spi_op_result = SPI_Master_WriteByte(CONFIG_GC9A01_SPI_INDEX, buff[0]);
        if(spi_op_result<hwSPI_OK)
        {
            return GC9A01_Map_SPI_Error_Code(spi_op_result);
        }
        spi_op_result = SPI_Master_WriteByte(CONFIG_GC9A01_SPI_INDEX, buff[1]);
        if(spi_op_result<hwSPI_OK)
        {
            return GC9A01_Map_SPI_Error_Code(spi_op_result);
        }
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_GC9A01_CS_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return GC9A01_Map_GPIO_Error_Code(gpio_op_result);
    }

    return GC9A01_OK;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

GC9A01_OpResult GC9A01_Init(void)
{
    GC9A01_OpResult op_status;

    op_status = GC9A01_IO_Init();
    if(op_status<GC9A01_OK) { return op_status; }

    //Reset the display
    op_status = GC9A01_Reset();
    if(op_status<GC9A01_OK) { return op_status; }

    uint16_t cmd = 0;
    while (GC9A01_Init_Cmds[cmd].databytes!=0xff)
    {
        op_status = GC9A01_IO_Write(GC9A01_Init_Cmds[cmd].cmd, GC9A01_Init_Cmds[cmd].data, GC9A01_Init_Cmds[cmd].databytes&0x1F);
        if(op_status<GC9A01_OK) { break; }

        if (GC9A01_Init_Cmds[cmd].databytes & 0x80) {
            NeonRTOS_Sleep(100);
        }

        cmd++;
    }

    return op_status;
}

GC9A01_OpResult GC9A01_Power_On()
{
    GC9A01_OpResult op_status;
    
    uint8_t data[] = {0x08};

    op_status = GC9A01_IO_Write(GC9A01_CMD_SLEEP_OUT, data, 1);
    if(op_status<GC9A01_OK) { return op_status; }

    NeonRTOS_Sleep(120);

    op_status = GC9A01_IO_Write(GC9A01_CMD_DISPLAY_ON, NULL, 0);
    if(op_status<GC9A01_OK) { return op_status; }

    NeonRTOS_Sleep(120);
    
    return GC9A01_OK;
}

GC9A01_OpResult GC9A01_Power_Off()
{
    GC9A01_OpResult op_status;
    
    op_status = GC9A01_IO_Write(GC9A01_CMD_DISPLAY_OFF, NULL, 0);
    if(op_status<GC9A01_OK) { return op_status; }

    NeonRTOS_Sleep(120);
    
    uint8_t data[] = {0x08};

    op_status = GC9A01_IO_Write(GC9A01_CMD_ENTER_SLEEP_MODE, data, 1);
    if(op_status<GC9A01_OK) { return op_status; }

    return GC9A01_OK;
}

GC9A01_OpResult GC9A01_SetWindow(int16_t x1, int16_t x2, int16_t y1, int16_t y2)
{
    uint8_t set_x_index[4] = {0};
    uint8_t set_y_index[4] = {0};
        
    GC9A01_OpResult op_status = GC9A01_Unsupport;

	set_x_index[0] = (x1 >> 8) & 0xFF;
	set_x_index[1] = x1 & 0xFF;
	set_x_index[2] = (x2 >> 8) & 0xFF;
	set_x_index[3] = x2 & 0xFF;

	set_y_index[0] = (y1 >> 8) & 0xFF;
	set_y_index[1] = y1 & 0xFF;
	set_y_index[2] = (y2 >> 8) & 0xFF;
	set_y_index[3] = y2 & 0xFF;

	/*Column addresses*/
	op_status = GC9A01_IO_Write(GC9A01_CMD_COLUMN_ADDRESS_SET, set_x_index, 4);
	if(op_status<GC9A01_OK) { return op_status; }
	
	/*Page addresses*/
	op_status = GC9A01_IO_Write(GC9A01_CMD_PAGE_ADDRESS_SET, set_y_index, 4);
	if(op_status<GC9A01_OK) { return op_status; }
	
    return GC9A01_OK;
}

GC9A01_OpResult GC9A01_VerticalScroll_Definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines)
{
    uint8_t scroll_cfg[6];

    scroll_cfg[0] = topFixedLines >> 8;
    scroll_cfg[1] = topFixedLines & 0xFF;
    scroll_cfg[2] = scrollLines >> 8;
    scroll_cfg[3] = scrollLines & 0xFF;
    scroll_cfg[4] = bottomFixedLines >> 8;
    scroll_cfg[5] = bottomFixedLines & 0xFF;

    return GC9A01_IO_Write(GC9A01_CMD_VERT_SCROLL_DEFINITION, scroll_cfg, 6);
}

GC9A01_OpResult GC9A01_VerticalScroll_StartLine(uint16_t startLine)
{
    GC9A01_OpResult op_status;

	uint8_t scroll_cfg[2];

    scroll_cfg[0] = startLine >> 8;
    scroll_cfg[1] = startLine & 0xFF;

    op_status = GC9A01_IO_Write(GC9A01_CMD_VERT_SCROLL_START_ADDRESS, scroll_cfg, 2);
    if(op_status<GC9A01_OK) { return op_status; }

    GC9A01_VerticalScrollingStartAddress = startLine;
    
    return GC9A01_OK;
}

GC9A01_OpResult GC9A01_Draw(int16_t x1, int16_t x2, int16_t y1, int16_t y2, GC9A01_Color16_RGB565* data)
{
    uint32_t size = (abs(x2-x1) + 1) * (abs(y2-y1) + 1);
        
    GC9A01_OpResult op_status;

    op_status = GC9A01_SetWindow(x1, x2, y1, y2);
	if(op_status<GC9A01_OK) { return op_status; }

	op_status = GC9A01_Nenory_Write(data, size);
	if(op_status<GC9A01_OK) { return op_status; }
    
    return GC9A01_OK;
}

GC9A01_OpResult GC9A01_DrawPixel(int16_t x, int16_t y, GC9A01_Color16_RGB565* data)
{
    return GC9A01_Draw(x, x, y, y, data);
}