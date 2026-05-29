#ifndef SPI_PIN_NUC4X2_H
#define SPI_PIN_NUC4X2_H

#include "SPI_Pin_Nuvoton.h"

typedef enum {
    SPI_Pinset_DEFAULT = 0,
    SPI_Pinset_ALT,
    SPI_Pinset_MAX
} SPI_Pinset_t;

#ifndef CONFIG_SPI0_PINSET
#define CONFIG_SPI0_PINSET SPI_Pinset_DEFAULT
#endif

#ifndef CONFIG_SPI1_PINSET
#define CONFIG_SPI1_PINSET SPI_Pinset_DEFAULT
#endif

#ifndef CONFIG_SPI2_PINSET
#define CONFIG_SPI2_PINSET SPI_Pinset_DEFAULT
#endif

#ifndef CONFIG_SPI3_PINSET
#define CONFIG_SPI3_PINSET SPI_Pinset_DEFAULT
#endif

static const SPI_Pinset_t SPI_Index_Map_Alt[hwSPI_Index_MAX] = {
#if defined(SPI0_BASE)
    CONFIG_SPI0_PINSET,
#endif
#if defined(SPI1_BASE)
    CONFIG_SPI1_PINSET,
#endif
#if defined(SPI2_BASE)
    CONFIG_SPI2_PINSET,
#endif
#if defined(SPI3_BASE)
    CONFIG_SPI3_PINSET,
#endif
};

static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX][SPI_Pinset_MAX] =
{
#if defined(SPI0_BASE)
    {
        /* DEFAULT: MOSI, MISO, SCK, NSS */
        { hwGPIO_Pin_A0, hwGPIO_Pin_A1, hwGPIO_Pin_A2, hwGPIO_Pin_A3 },

        /* ALT */
        { hwGPIO_Pin_B0, hwGPIO_Pin_B1, hwGPIO_Pin_B2, hwGPIO_Pin_B3 },
    },
#endif

#if defined(SPI1_BASE)
    {
        /* DEFAULT: MOSI, MISO, SCK, NSS */
        { hwGPIO_Pin_A4, hwGPIO_Pin_A5, hwGPIO_Pin_A6, hwGPIO_Pin_A7 },

        /* ALT */
        { hwGPIO_Pin_B4, hwGPIO_Pin_B5, hwGPIO_Pin_B6, hwGPIO_Pin_B7 },
    },
#endif

#if defined(SPI2_BASE)
    {
        /* DEFAULT: MOSI, MISO, SCK, NSS */
        { hwGPIO_Pin_A8, hwGPIO_Pin_A9, hwGPIO_Pin_A10, hwGPIO_Pin_A11 },

        /* ALT */
        { hwGPIO_Pin_B8, hwGPIO_Pin_B9, hwGPIO_Pin_B10, hwGPIO_Pin_B11 },
    },
#endif

#if defined(SPI3_BASE)
    {
        /* DEFAULT: MOSI, MISO, SCK, NSS */
        { hwGPIO_Pin_C0, hwGPIO_Pin_C1, hwGPIO_Pin_C2, hwGPIO_Pin_C3 },

        /* ALT */
        { hwGPIO_Pin_D0, hwGPIO_Pin_D1, hwGPIO_Pin_D2, hwGPIO_Pin_D3 },
    },
#endif
};

#endif // SPI_PIN_NUC4X2_H