
#ifndef SPI_PIN_DEF_TM4C1294_H
#define SPI_PIN_DEF_TM4C1294_H

#include "SPI_Pin_TITivaC_Def.h"

static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX] =
{
    /* SSI0 */
    {
        hwGPIO_Pin_A5,    // MOSI (XDAT1)
        hwGPIO_Pin_A4,    // MISO (XDAT0)
        hwGPIO_Pin_A2,    // SCLK (CLK)
        hwGPIO_Pin_A3     // FSS
    },

    /* SSI1 */
    {
        hwGPIO_Pin_E5,    // MOSI
        hwGPIO_Pin_E4,    // MISO
        hwGPIO_Pin_B5,    // SCLK
        hwGPIO_Pin_B4     // FSS
    },

    /* SSI2 */
    {
        hwGPIO_Pin_D0,    // MOSI
        hwGPIO_Pin_D1,    // MISO
        hwGPIO_Pin_D3,    // SCLK
        hwGPIO_Pin_D2     // FSS
    },

    /* SSI3 */
    {
        hwGPIO_Pin_Q3,    // MOSI
        hwGPIO_Pin_Q2,    // MISO
        hwGPIO_Pin_Q0,    // SCLK
        hwGPIO_Pin_Q1     // FSS
    }
};

#endif //SPI_PIN_DEF_TM4C1294_H