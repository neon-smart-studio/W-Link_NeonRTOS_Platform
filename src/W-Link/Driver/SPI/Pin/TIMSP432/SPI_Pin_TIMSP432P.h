
#ifndef SPI_PIN_DEF_TM4C123_H
#define SPI_PIN_DEF_TM4C123_H

#include "SPI_Pin_TIMSP432_Def.h"

static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX] =
{
    /* ================= eUSCI_B0 ================= */
    {
        hwGPIO_Pin_A6,      // MOSI  P1.6
        hwGPIO_Pin_A7,      // MISO  P1.7
        hwGPIO_Pin_A5,      // SCLK  P1.5
        hwGPIO_Pin_A4       // CS    P1.4
    },

    /* ================= eUSCI_B1 ================= */
    {
        hwGPIO_Pin_C14,      // MOSI  P6.6
        hwGPIO_Pin_C13,      // MISO  P6.5
        hwGPIO_Pin_C12,      // SCLK  P6.4
        hwGPIO_Pin_C15       // CS    P6.7
    },

    /* ================= eUSCI_B2 ================= */
    {
        hwGPIO_Pin_B6,       // MOSI  P3.6
        hwGPIO_Pin_B7,       // MISO  P3.7
        hwGPIO_Pin_B5,       // SCLK  P3.5
        hwGPIO_Pin_B4        // CS    P3.4
    },

    /* ================= eUSCI_B3 ================= */
    {
        hwGPIO_Pin_E11,      // MOSI  P10.3
        hwGPIO_Pin_E10,      // MISO  P10.2
        hwGPIO_Pin_E9,       // SCLK  P10.1
        hwGPIO_Pin_E8        // CS    P10.0
    }
};

#endif //SPI_PIN_DEF_TM4C123_H