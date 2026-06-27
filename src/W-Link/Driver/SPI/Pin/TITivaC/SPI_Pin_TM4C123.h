
#ifndef SPI_PIN_DEF_TM4C123_H
#define SPI_PIN_DEF_TM4C123_H

#include "SPI_Pin_TITivaC_Def.h"

#if defined(TM4C123)

static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX] =
{
    /* SPI0 (SSI0) */
    {
        hwGPIO_Pin_A5,    // MOSI
        hwGPIO_Pin_A4,    // MISO
        hwGPIO_Pin_A2,    // SCLK
        hwGPIO_Pin_A3     // CS/FSS
    },

    /* SPI1 (SSI1) */
    {
        hwGPIO_Pin_F1,    // MOSI
        hwGPIO_Pin_F0,    // MISO
        hwGPIO_Pin_F2,    // SCLK
        hwGPIO_Pin_F3     // CS/FSS
    },

    /* SPI2 (SSI2) */
    {
        hwGPIO_Pin_B7,    // MOSI
        hwGPIO_Pin_B6,    // MISO
        hwGPIO_Pin_B4,    // SCLK
        hwGPIO_Pin_B5     // CS/FSS
    },

    /* SPI3 (SSI3) */
    {
        hwGPIO_Pin_D3,    // MOSI
        hwGPIO_Pin_D2,    // MISO
        hwGPIO_Pin_D0,    // SCLK
        hwGPIO_Pin_D1     // CS/FSS
    }
};

#endif //TM4C123

#endif //SPI_PIN_DEF_TM4C123_H