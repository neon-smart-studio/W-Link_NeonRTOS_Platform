
/*********************
 *      INCLUDES
 *********************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "NeonRTOS.h"

#include "HX8357x.h"

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

/*
 * Initialization sequence based on vendor datasheet
 * and publicly available examples.
 */
#define HX8357X_CMD_SWRESET  0x01
#define HX8357X_CMD_SLPIN    0x10
#define HX8357X_CMD_SLPOUT   0x11
#define HX8357X_CMD_DISPOFF  0x28
#define HX8357X_CMD_DISPON   0x29
#define HX8357X_CMD_CASET    0x2A
#define HX8357X_CMD_PASET    0x2B
#define HX8357X_CMD_RAMWR    0x2C
#define HX8357X_CMD_MADCTL   0x36
#define HX8357X_CMD_COLMOD   0x3A
#define HX8357X_CMD_TEON     0x35
#define HX8357X_CMD_TEARLINE 0x44

#define HX8357B_CMD_SETPOWER         0xD0
#define HX8357B_CMD_SETVCOM          0xD1
#define HX8357B_CMD_SETPWRNORMAL     0xD2
#define HX8357B_CMD_SET_PANEL_DRIVING 0xC0
#define HX8357B_CMD_SETDISPLAYFRAME  0xC5
#define HX8357B_CMD_SETPANELRELATED  0xCC
#define HX8357B_CMD_SETGAMMA         0xC8
#define HX8357B_CMD_SETDISPMODE      0xB4

#define HX8357D_CMD_SETC     0xB9
#define HX8357D_CMD_SETCOM   0xB6
#define HX8357D_CMD_SETSTBA  0xC0
#define HX8357D_CMD_SETCYC   0xB4
#define HX8357D_CMD_SETGAMMA 0xE0
#define HX8357X_CMD_SETRGB   0xB3
#define HX8357X_CMD_SETOSC   0xB0
#define HX8357X_CMD_SETPANEL 0xCC
#define HX8357X_CMD_SETPWR1  0xB1

typedef struct {
    uint8_t cmd;
    uint8_t data[40];
    uint8_t databytes;
} HX8357X_Init_Command;

#ifdef DISPLAY_HX8357B
static const HX8357X_Init_Command HX8357B_Init_Cmds[] = {
    {HX8357B_CMD_SETPOWER, {0x44, 0x41, 0x06}, 3},
    {HX8357B_CMD_SETVCOM, {0x40, 0x10}, 2},
    {HX8357B_CMD_SETPWRNORMAL, {0x05, 0x12}, 2},
    {HX8357B_CMD_SET_PANEL_DRIVING, {0x14, 0x3b, 0x00, 0x02, 0x11}, 5},
    {HX8357B_CMD_SETDISPLAYFRAME, {0x0c}, 1},  // 6.8mhz
    {HX8357B_CMD_SETPANELRELATED, {0x01}, 1},  // BGR
    {0xEA, {0x03, 0x00, 0x00}, 3},         // seq_undefined1, 3 args
    {0xEB, {0x40, 0x54, 0x26, 0xdb}, 4},   // undef2, 4 args
    {HX8357B_CMD_SETGAMMA, {0x00, 0x15, 0x00, 0x22, 0x00, 0x08, 0x77, 0x26, 0x66, 0x22, 0x04, 0x00}, 12},
#ifdef CONFIG_DISPLAY_ROTATE_180
    {HX8357x_CMD_MADCTL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
    {HX8357x_CMD_MADCTL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
    {HX8357x_CMD_COLMOD, {0x55}, 1},
    {HX8357x_CMD_PASET, {0x00, 0x00, 0x01, 0xDF}, 4},
    {HX8357x_CMD_CASET, {0x00, 0x00, 0x01, 0x3F}, 4},
    {HX8357B_CMD_SETDISPMODE, {0x00}, 1},      // CPU (DBI) and internal oscillation ??
    {HX8357x_CMD_SLPOUT, {0}, 0x80},   // Exit sleep, then delay 120 ms
    {HX8357x_CMD_DISPON, {0}, 0x80},   // Main screen turn on, delay 10 ms
        {0, {0}, 0xff}
};
#endif //DISPLAY_HX8357B

#ifdef DISPLAY_HX8357D
static const HX8357X_Init_Command HX8357D_Init_Cmds[] = {
    {HX8357x_CMD_SWRESET, {0}, 0x80},          // Soft reset, then delay 10 ms
    {HX8357D_CMD_SETC, {0xFF, 0x83, 0x57}, 3},
    {0xFF, {0}, 0x80},                    // No command, just delay 300 ms
    {HX8357x_CMD_SETRGB, {0x80, 0x00, 0x06, 0x06}, 4}    // 0x80 enables SDO pin (0x00 disables)
    {HX8357D_CMD_SETCOM, {0x25}, 1},                    // -1.52V
    {HX8357x_CMD_SETOSC, {0x68}, 1},                     // Normal mode 70Hz, Idle mode 55 Hz
    {HX8357x_CMD_SETPANEL, {0x05}, 1},                   // BGR, Gate direction swapped
    {HX8357x_CMD_SETPWR1, {0x00, 0x15, 0x1C, 0x1C, 0x83, 0xAA}, 6},
                // {Not deep standby, BT, VSPR, VSNR, AP, FS}
    {HX8357D_CMD_SETSTBA, {0x50, 0x50, 0x01, 0x3C, 0x1E, 0x08}, 6},
                    // {OPON normal, OPON idle, STBA, STBA, STBA, GEN, STBA}
    {HX8357D_CMD_SETCYC, {0x02, 0x40, 0x00, 0x2A, 0x2A, 0x0D, 0x78}, 7},
                // {NW 0x02, RTN, DIV, DUM, DUM, GDON, GDOFF}
    {HX8357D_CMD_SETGAMMA, {0x02, 0x0A, 0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b, 0x4b, 0x42, 0x3A, 0x27, 0x1B, 0x08, 0x09, 0x03, 0x02, 0x0A, 0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b, 0x4b, 0x42, 0x3A, 0x27, 0x1B, 0x08, 0x09, 0x03, 0x00, 0x01}, 34},
    {HX8357x_CMD_COLMOD, {0x55}, 1},                     // 16 bit
#ifdef CONFIG_DISPLAY_ROTATE_180
    {HX8357x_CMD_MADCTL, {0x88}, 1},
#else //CONFIG_DISPLAY_ROTATE_180
    {HX8357x_CMD_MADCTL, {0x48}, 1},
#endif //CONFIG_DISPLAY_ROTATE_180
    {HX8357x_CMD_TEON, {0x00}, 1},                       // TW off
    {HX8357x_CMD_TEARLINE, {0x00, 0x02}, 2},
    {HX8357x_CMD_SLPOUT, {0}, 0x80},           // Exit Sleep, then delay 150 ms
    {HX8357x_CMD_DISPON, {0}, 0x80},           // Main screen turn on, delay 50 ms
        {0, {0}, 0xff}
};
#endif //DISPLAY_HX8357D

static HX8357x_OpResult HX8357x_Map_GPIO_Error_Code(hwGPIO_OpStatus error_code)
{
  switch(error_code)
  {
  case hwGPIO_OK:
    return HX8357x_OK;
    
  case hwGPIO_InvalidParameter:
    return HX8357x_InvalidParameter;
    
  case hwGPIO_HW_Error:
  case hwGPIO_PinConflict:
    return HX8357x_HwError;
    
  case hwGPIO_Unsupport:
  default:
    return HX8357x_Unsupport;
  }
}

static HX8357x_OpResult HX8357x_Map_SPI_Error_Code(hwSPI_OpResult error_code)
{
  switch(error_code)
  {
  case hwSPI_OK:
    return HX8357x_OK;
    
  case hwSPI_NotInit:
    return HX8357x_NotInit;
    
  case hwSPI_InvalidParameter:
    return HX8357x_InvalidParameter;
    
  case hwSPI_MemoryError:
    return HX8357x_MemoryError;
    
  case hwSPI_MutexTimeout:
    return HX8357x_MutexTimeout;
    
  case hwSPI_SlaveTimeout:
    return HX8357x_SlaveTimeout;
    
  case hwSPI_Unsupport:
  default:
    return HX8357x_Unsupport;
  }
}

static HX8357x_OpResult HX8357x_IO_Init()
{
    hwGPIO_OpStatus gpio_op_result;
    hwSPI_OpResult spi_op_result;
    
    gpio_op_result = GPIO_Pin_Init(CONFIG_HX8357X_CS_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Init(CONFIG_HX8357X_DC_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_result<hwGPIO_OK)
    {
        GPIO_Pin_DeInit(CONFIG_HX8357X_CS_PN);
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Init(CONFIG_HX8357X_RST_PN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_result<hwGPIO_OK)
    {
        GPIO_Pin_DeInit(CONFIG_HX8357X_CS_PN);
        GPIO_Pin_DeInit(CONFIG_HX8357X_DC_PN);
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    spi_op_result = SPI_Master_Init(CONFIG_HX8357X_SPI_INDEX, CONFIG_HX8357X_SPI_CLOCK, CONFIG_HX8357X_SPI_MODE);
    if(spi_op_result<hwSPI_OK)
    {
        GPIO_Pin_DeInit(CONFIG_HX8357X_CS_PN);
        GPIO_Pin_DeInit(CONFIG_HX8357X_DC_PN);
        GPIO_Pin_DeInit(CONFIG_HX8357X_RST_PN);
        return HX8357x_Map_SPI_Error_Code(spi_op_result);
    }

    return HX8357x_OK;
}

static HX8357x_OpResult HX8357x_Reset()
{
    hwGPIO_OpStatus gpio_op_result;

    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_RST_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    NeonRTOS_Sleep(10);
    
    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_RST_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    NeonRTOS_Sleep(10);

    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_RST_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    NeonRTOS_Sleep(120);

    return HX8357x_OK;
}

static HX8357x_OpResult HX8357x_IO_Write(uint8_t cmd, uint8_t* param_data, uint16_t param_length)
{
    hwGPIO_OpStatus gpio_op_result;
    hwSPI_OpResult spi_op_result;

    if(param_data==NULL && param_length>0)
    {
        return HX8357x_InvalidParameter;
    }
	
    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_CS_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_DC_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    spi_op_result = SPI_Master_WriteByte(CONFIG_HX8357X_SPI_INDEX, cmd);
    if(spi_op_result<hwSPI_OK)
    {
        return HX8357x_Map_SPI_Error_Code(spi_op_result);
    }

    if(param_length==0)
    {
        gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_CS_PN, 1);
        if(gpio_op_result<hwGPIO_OK)
        {
                return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
        }

        return HX8357x_OK; 
    }
	
    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_DC_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    for(uint16_t i = 0; i<param_length; i++)
    {
        spi_op_result = SPI_Master_WriteByte(CONFIG_HX8357X_SPI_INDEX, param_data[i]);
        if(spi_op_result<hwSPI_OK)
        {
            return HX8357x_Map_SPI_Error_Code(spi_op_result);
        }
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_CS_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    return HX8357x_OK;
}

static HX8357x_OpResult HX8357x_Nenory_Write(HX8357x_Color16_RGB565* data, uint16_t data_length)
{
    hwGPIO_OpStatus gpio_op_result;
    hwSPI_OpResult spi_op_result;

    if(data==NULL && data_length>0)
    {
        return HX8357x_InvalidParameter;
    }
            
    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_CS_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_DC_PN, 0);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    spi_op_result = SPI_Master_WriteByte(CONFIG_HX8357X_SPI_INDEX, HX8357X_CMD_RAMWR);
    if(spi_op_result<hwSPI_OK)
    {
        return HX8357x_Map_SPI_Error_Code(spi_op_result);
    }
  
    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_DC_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    for(uint16_t i = 0; i<data_length; i++)
    {
        uint8_t buff[2];
        buff[0] = ((uint16_t)data[i].full)>>8;
        buff[1] = ((uint16_t)data[i].full)&0xff;
        spi_op_result = SPI_Master_WriteByte(CONFIG_HX8357X_SPI_INDEX, buff[0]);
        if(spi_op_result<hwSPI_OK)
        {
            return HX8357x_Map_SPI_Error_Code(spi_op_result);
        }
        spi_op_result = SPI_Master_WriteByte(CONFIG_HX8357X_SPI_INDEX, buff[1]);
        if(spi_op_result<hwSPI_OK)
        {
            return HX8357x_Map_SPI_Error_Code(spi_op_result);
        }
    }

    gpio_op_result = GPIO_Pin_Write(CONFIG_HX8357X_CS_PN, 1);
    if(gpio_op_result<hwGPIO_OK)
    {
        return HX8357x_Map_GPIO_Error_Code(gpio_op_result);
    }

    return HX8357x_OK;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

HX8357x_OpResult HX8357x_Init(void)
{
    HX8357x_OpResult op_status;

    op_status = HX8357x_IO_Init();
    if(op_status<HX8357x_OK) { return op_status; }

    //Reset the display
    op_status = HX8357x_Reset();
    if(op_status<HX8357x_OK) { return op_status; }

    HX8357X_Init_Command* pHX8357x_Init_Cmds = NULL;
#ifdef DISPLAY_HX8357B
    pHX8357x_Init_Cmds = HX8357B_Init_Cmds;
#endif // DISPLAY_HX8357B
#ifdef DISPLAY_HX8357D
    pHX8357x_Init_Cmds = HX8357D_Init_Cmds;
#endif // DISPLAY_HX8357D

    if(pHX8357x_Init_Cmds!=NULL)
    {
        uint16_t cmd = 0;
        while (pHX8357x_Init_Cmds[cmd].databytes!=0xff)
        {
            op_status = HX8357x_IO_Write(pHX8357x_Init_Cmds[cmd].cmd, pHX8357x_Init_Cmds[cmd].data, pHX8357x_Init_Cmds[cmd].databytes&0x1F);
            if(op_status<HX8357x_OK) { break; }

            if (pHX8357x_Init_Cmds[cmd].databytes & 0x80) {
                NeonRTOS_Sleep(100);
            }

            cmd++;
        } 
    }

    return op_status;
}

HX8357x_OpResult HX8357x_Power_On()
{
    HX8357x_OpResult op_status;
    
    op_status = HX8357x_IO_Write(HX8357X_CMD_SLPOUT, NULL, 0);
    if(op_status<HX8357x_OK) { return op_status; }

    NeonRTOS_Sleep(120);
    
    op_status = HX8357x_IO_Write(HX8357X_CMD_DISPON, NULL, 0);
    if(op_status<HX8357x_OK) { return op_status; }
    
    NeonRTOS_Sleep(120);

    return HX8357x_OK;
}

HX8357x_OpResult HX8357x_Power_Off()
{
    HX8357x_OpResult op_status;
    
    op_status = HX8357x_IO_Write(HX8357X_CMD_DISPOFF, NULL, 0);
    if(op_status<HX8357x_OK) { return op_status; }
    
    NeonRTOS_Sleep(120);

    op_status = HX8357x_IO_Write(HX8357X_CMD_SLPIN, NULL, 0);
    if(op_status<HX8357x_OK) { return op_status; }

    NeonRTOS_Sleep(120);

    return HX8357x_OK;
}

HX8357x_OpResult HX8357x_SetWindow(int16_t x1, int16_t x2, int16_t y1, int16_t y2)
{
    HX8357x_OpResult op_status;

    uint8_t set_x_index[4] = {0};
    uint8_t set_y_index[4] = {0};
        
    set_x_index[0] = (x1 >> 8) & 0xFF;
    set_x_index[1] = x1 & 0xFF;
    set_x_index[2] = (x2 >> 8) & 0xFF;
    set_x_index[3] = x2 & 0xFF;

    set_y_index[0] = (y1 >> 8) & 0xFF;
    set_y_index[1] = y1 & 0xFF;
    set_y_index[2] = (y2 >> 8) & 0xFF;
    set_y_index[3] = y2 & 0xFF;

    /*Column addresses*/
    op_status = HX8357x_IO_Write(HX8357X_CMD_CASET, set_x_index, 4);
    if(op_status<HX8357x_OK) { return op_status; }
    
    /*Page addresses*/
    op_status = HX8357x_IO_Write(HX8357X_CMD_PASET, set_y_index, 4);
    if(op_status<HX8357x_OK) { return op_status; }
    
    return HX8357x_OK;
}

HX8357x_OpResult HX8357x_Draw(int16_t x1, int16_t x2, int16_t y1, int16_t y2, HX8357x_Color16_RGB565* data)
{
    uint32_t size = (abs(x2-x1) + 1) * (abs(y2-y1) + 1);

    HX8357x_OpResult op_status;

    op_status = HX8357x_SetWindow(x1, x2, y1, y2);
    if(op_status<HX8357x_OK) { return op_status; }
    
    /*Memory write*/
    op_status = HX8357x_Nenory_Write(data, size);
	if(op_status<HX8357x_OK) { return op_status; }
    
    return HX8357x_OK;
}

HX8357x_OpResult HX8357x_DrawPixel(int16_t x, int16_t y, HX8357x_Color16_RGB565* data)
{
      return HX8357x_Draw(x, x, y, y, data);
}