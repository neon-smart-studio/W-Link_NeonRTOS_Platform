
#ifndef SPI_PIN_TIMSP432E_H
#define SPI_PIN_TIMSP432E_H

#include "SPI_Pin_TIMSP432_Def.h"

static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX] =
{
    /* SSI0 */
    {
        hwGPIO_Pin_A5,    // MOSI / SSI0XDAT1
        hwGPIO_Pin_A4,    // MISO / SSI0XDAT0
        hwGPIO_Pin_A2,    // SCLK / SSI0CLK
        hwGPIO_Pin_A3     // FSS  / SSI0FSS
    },

    /* SSI1 */
    {
        hwGPIO_Pin_E5,    // MOSI / SSI1XDAT1
        hwGPIO_Pin_E4,    // MISO / SSI1XDAT0
        hwGPIO_Pin_B5,    // SCLK / SSI1CLK
        hwGPIO_Pin_B4     // FSS  / SSI1FSS
    },

    /* SSI2 */
    {
        hwGPIO_Pin_D0,    // MOSI / SSI2XDAT1
        hwGPIO_Pin_D1,    // MISO / SSI2XDAT0
        hwGPIO_Pin_D3,    // SCLK / SSI2CLK
        hwGPIO_Pin_D2     // FSS  / SSI2FSS
    },

    /* SSI3 */
    {
        hwGPIO_Pin_Q3,    // MOSI / SSI3XDAT1
        hwGPIO_Pin_Q2,    // MISO / SSI3XDAT0
        hwGPIO_Pin_Q0,    // SCLK / SSI3CLK
        hwGPIO_Pin_Q1     // FSS  / SSI3FSS
    }
};

#endif //SPI_PIN_TIMSP432E_H