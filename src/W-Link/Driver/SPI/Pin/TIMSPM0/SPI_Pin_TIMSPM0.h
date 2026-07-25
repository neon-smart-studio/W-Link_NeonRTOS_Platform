
#ifndef SPI_PIN_TIMSPM0_H
#define SPI_PIN_TIMSPM0_H

#include "SPI_Pin_TIMSPM0_Def.h"
static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX] =
{
    /* ================= SPI0 / UC Primary ================= */

#if defined(SPI0_BASE)
#if defined(MSPM0C110x) || defined(MSPM0S003Fx)

    /* MSPM0C1103/C1104、MSPS003F3/F4：SPI0 */
    {
        hwGPIO_Pin_A18,     // MOSI  PA18  SPI0_PICO
        hwGPIO_Pin_A4,      // MISO  PA4   SPI0_POCI
        hwGPIO_Pin_A6,      // SCLK  PA6   SPI0_SCLK
        hwGPIO_Pin_A2       // CS    PA2   SPI0_CS0
    },

#elif defined(MSPM0G511x) || defined(MSPM0G518x)

    /* MSPM0G511x/G518x：UC2 */
    {
        hwGPIO_Pin_A5,      // MOSI  PA5  UC2_PICO
        hwGPIO_Pin_A4,      // MISO  PA4  UC2_POCI
        hwGPIO_Pin_A6,      // SCLK  PA6  UC2_SCLK
        hwGPIO_Pin_A8       // CS    PA8  UC2_CS0
    },

#elif defined(MSPM0C031Cx) || defined(MSPM0G031Cx) || \
      defined(MSPM0C1105) || defined(MSPM0C1106) || \
      defined(MSPM0G110x) || \
      defined(MSPM0G120x) || defined(MSPM0G121x) || \
      defined(MSPM0G150x) || defined(MSPM0G151x) || \
      defined(MSPM0G310x) || \
      defined(MSPM0G320x) || defined(MSPM0G321x) || \
      defined(MSPM0G350x) || defined(MSPM0G351x) || \
      defined(MSPM0G352x) || \
      defined(MSPM0H321x) || \
      defined(MSPM0L110x) || defined(MSPM0L111x) || \
      defined(MSPM0L112x) || defined(MSPM0L122x) || \
      defined(MSPM0L130x) || defined(MSPM0L134x) || \
      defined(MSPM0L211x) || defined(MSPM0L222x)

    /*
     * 固定 SPI 型號：SPI0
     * G120/G121/G320/G321：UC2
     * L112/L211：UC4
     */
    {
        hwGPIO_Pin_A5,      // MOSI  PA5  PICO
        hwGPIO_Pin_A4,      // MISO  PA4  POCI
        hwGPIO_Pin_A6,      // SCLK  PA6  SCLK
        hwGPIO_Pin_A2       // CS    PA2  CS0
    },
#endif
#endif


    /* ================= SPI1 / UC Secondary ================= */

#if defined(SPI1_BASE)
#if defined(MSPM0G120x) || defined(MSPM0G121x) || \
    defined(MSPM0G320x) || defined(MSPM0G321x) || \
    defined(MSPM0G511x) || defined(MSPM0G518x) || \
    defined(MSPM0L112x) || defined(MSPM0L211x)

    /*
     * 固定 SPI 型號：SPI1
     * G120/G121/G320/G321：UC4
     * G511/G518：UC3
     * L112/L211：UC8
     */
    {
        hwGPIO_Pin_B8,      // MOSI  PB8  PICO
        hwGPIO_Pin_B7,      // MISO  PB7  POCI
        hwGPIO_Pin_B9,      // SCLK  PB9  SCLK
        hwGPIO_Pin_B6       // CS    PB6  CS0
    },

#endif
#endif


    /* ================= SPI2 ================= */

#if defined(SPI2_BASE)
    /*
     * MSPM0G151x
     * MSPM0G351x
     * MSPM0G352x
     */
    {
        hwGPIO_Pin_B4,      // MOSI  PB4   SPI2_PICO
        hwGPIO_Pin_B5,      // MISO  PB5   SPI2_POCI
        hwGPIO_Pin_A10,     // SCLK  PA10  SPI2_SCLK
        hwGPIO_Pin_A4       // CS    PA4   SPI2_CS0
    },
#endif
};

#endif //SPI_PIN_TIMSPM0_H