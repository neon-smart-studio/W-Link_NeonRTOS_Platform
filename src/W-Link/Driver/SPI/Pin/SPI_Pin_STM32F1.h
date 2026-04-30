
#ifndef SPI_PIN_STM32F1_H
#define SPI_PIN_STM32F1_H

#include "SPI_Pin_STM32.h"

typedef enum {
    SPI_Pinset_DEFAULT = 0,
    SPI_Pinset_ALT,
    SPI_Pinset_MAX
} SPI_Pinset_t;

#ifndef CONFIG_SPI0_PINSET
#define CONFIG_SPI0_PINSET SPI_Pinset_DEFAULT
#endif //CONFIG_SPI0_PINSET

#ifndef CONFIG_SPI1_PINSET
#define CONFIG_SPI1_PINSET SPI_Pinset_DEFAULT
#endif //CONFIG_SPI1_PINSET

static const SPI_Pinset_t SPI_Index_Map_Alt[hwSPI_Index_MAX] = {
#if defined(SPI1_BASE)
    CONFIG_SPI0_PINSET,
#endif
#if defined(SPI2_BASE)
    CONFIG_SPI1_PINSET
#endif
};

static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX][SPI_Pinset_MAX] =
{
#if defined(SPI1_BASE)
    /* ================= SPI1 (SPI0) ================= */
    {
        /* DEFAULT */
        { hwGPIO_Pin_A7, hwGPIO_Pin_A6, hwGPIO_Pin_A5, hwGPIO_Pin_A4 },

        /* ALT */
        { hwGPIO_Pin_B5, hwGPIO_Pin_B4, hwGPIO_Pin_B3, hwGPIO_Pin_A15 },
    },
#endif
#if defined(SPI2_BASE)
    /* ================= SPI2 (SPI1) ================= */
    {
        /* DEFAULT */
        { hwGPIO_Pin_B15, hwGPIO_Pin_B14, hwGPIO_Pin_B13, hwGPIO_Pin_B12 },

        /* ALT */
        { hwGPIO_Pin_C3,  hwGPIO_Pin_C2,  hwGPIO_Pin_B10, hwGPIO_Pin_B9  },
    },
#endif
};

#endif //SPI_PIN_STM32F1_H