
#ifndef QSPI_PIN_TIMSP432E_H
#define QSPI_PIN_TIMSP432E_H

#include "QSPI_Pin_TIMSP432_Def.h"

static const SPI_Pin_Def QSPI_Pin_Def_Table[hwSPI_Index_MAX] =
{
    /* SSI0 */
    {
        hwGPIO_Pin_A4,    // XDAT0
        hwGPIO_Pin_A5,    // XDAT1
        hwGPIO_Pin_A6,    // XDAT2
        hwGPIO_Pin_A7,    // XDAT3
        hwGPIO_Pin_A2,    // SCLK
        hwGPIO_Pin_A3     // FSS
    },

    /* SSI1 */
    {
        hwGPIO_Pin_E4,    // XDAT0
        hwGPIO_Pin_E5,    // XDAT1
        hwGPIO_Pin_D4,    // XDAT2
        hwGPIO_Pin_D5,    // XDAT3
        hwGPIO_Pin_B5,    // SCLK
        hwGPIO_Pin_B4     // FSS
    },

    /* SSI2 */
    {
        hwGPIO_Pin_D1,    // XDAT0
        hwGPIO_Pin_D0,    // XDAT1
        hwGPIO_Pin_D7,    // XDAT2
        hwGPIO_Pin_D6,    // XDAT3
        hwGPIO_Pin_D3,    // SCLK
        hwGPIO_Pin_D2     // FSS
    },

    /* SSI3 */
    {
        hwGPIO_Pin_Q2,    // XDAT0
        hwGPIO_Pin_Q3,    // XDAT1
        hwGPIO_Pin_F4,    // XDAT2
        hwGPIO_Pin_P1,    // XDAT3
        hwGPIO_Pin_Q0,    // SCLK
        hwGPIO_Pin_Q1     // FSS
    }
};

#endif //QSPI_PIN_TIMSP432E_H