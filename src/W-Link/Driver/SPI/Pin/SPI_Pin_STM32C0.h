#ifndef SPI_PIN_STM32C0_H
#define SPI_PIN_STM32C0_H

#include "SPI_Pin_STM32.h"

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

static const SPI_Pinset_t SPI_Index_Map_Alt[hwSPI_Index_MAX] = {
#if defined(SPI1_BASE)
    CONFIG_SPI0_PINSET,
#endif
#if defined(SPI2_BASE)
    CONFIG_SPI1_PINSET,
#endif
};

static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX][SPI_Pinset_MAX] =
{
#if defined(SPI1_BASE)
    {
        /* DEFAULT: MOSI, MISO, SCK, NSS */
        { hwGPIO_Pin_A7, hwGPIO_Pin_A6, hwGPIO_Pin_A5, hwGPIO_Pin_A4 },

        /* ALT */
        { hwGPIO_Pin_B5, hwGPIO_Pin_B4, hwGPIO_Pin_B3, hwGPIO_Pin_A15 },
    },
#endif

#if defined(SPI2_BASE)
    {
        /* DEFAULT */
        { hwGPIO_Pin_B15, hwGPIO_Pin_B14, hwGPIO_Pin_B13, hwGPIO_Pin_B12 },

        /* ALT */
        { hwGPIO_Pin_C3, hwGPIO_Pin_C2, hwGPIO_Pin_B10, hwGPIO_Pin_B9 },
    },
#endif
};

static const SPI_AF_Map SPI_Pin_AF_Map[] =
{
#if defined(SPI1_BASE)
    /* SPI1 */
    { hwSPI_Index_0, hwGPIO_Pin_A4,  GPIO_AF0_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_A5,  GPIO_AF0_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_A6,  GPIO_AF0_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_A7,  GPIO_AF0_SPI1 },

#if defined(GPIO_AF0_SPI1)
    { hwSPI_Index_0, hwGPIO_Pin_A15, GPIO_AF0_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_B3,  GPIO_AF0_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_B4,  GPIO_AF0_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_B5,  GPIO_AF0_SPI1 },
#endif
#endif

#if defined(SPI2_BASE)
    /* SPI2 */
    { hwSPI_Index_1, hwGPIO_Pin_B12, GPIO_AF0_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B13, GPIO_AF0_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B14, GPIO_AF0_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B15, GPIO_AF0_SPI2 },

#if defined(GPIO_AF0_SPI2)
    { hwSPI_Index_1, hwGPIO_Pin_B9,  GPIO_AF0_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B10, GPIO_AF0_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_C2,  GPIO_AF0_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_C3,  GPIO_AF0_SPI2 },
#endif
#endif
};

#endif // SPI_PIN_STM32C0_H