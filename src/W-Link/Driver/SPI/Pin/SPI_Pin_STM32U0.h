#ifndef SPI_PIN_STM32U0_H
#define SPI_PIN_STM32U0_H

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

#ifndef CONFIG_SPI2_PINSET
#define CONFIG_SPI2_PINSET SPI_Pinset_DEFAULT
#endif

static const SPI_Pinset_t SPI_Index_Map_Alt[hwSPI_Index_MAX] = {
#if defined(SPI1_BASE)
    CONFIG_SPI0_PINSET,
#endif
#if defined(SPI2_BASE)
    CONFIG_SPI1_PINSET,
#endif
#if defined(SPI3_BASE)
    CONFIG_SPI2_PINSET,
#endif
};

static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX][SPI_Pinset_MAX] =
{
#if defined(SPI1_BASE)
    {
        { hwGPIO_Pin_A7, hwGPIO_Pin_A6, hwGPIO_Pin_A5, hwGPIO_Pin_A4 },
        { hwGPIO_Pin_B5, hwGPIO_Pin_B4, hwGPIO_Pin_B3, hwGPIO_Pin_A15 },
    },
#endif

#if defined(SPI2_BASE)
    {
        { hwGPIO_Pin_B15, hwGPIO_Pin_B14, hwGPIO_Pin_B13, hwGPIO_Pin_B12 },
        { hwGPIO_Pin_C3,  hwGPIO_Pin_C2,  hwGPIO_Pin_B10, hwGPIO_Pin_B9  },
    },
#endif

#if defined(SPI3_BASE)
    {
        { hwGPIO_Pin_B5,  hwGPIO_Pin_B4,  hwGPIO_Pin_B3,  hwGPIO_Pin_A15 },
        { hwGPIO_Pin_C12, hwGPIO_Pin_C11, hwGPIO_Pin_C10, hwGPIO_Pin_NC  },
    },
#endif
};

static const SPI_AF_Map SPI_Pin_AF_Map[] =
{
#if defined(SPI1_BASE)
    { hwSPI_Index_0, hwGPIO_Pin_A4,  GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_A5,  GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_A6,  GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_A7,  GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_A15, GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_B3,  GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_B4,  GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_B5,  GPIO_AF5_SPI1 },
#endif

#if defined(SPI2_BASE)
    { hwSPI_Index_1, hwGPIO_Pin_B12, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B13, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B14, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B15, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B9,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B10, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_C2,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_C3,  GPIO_AF5_SPI2 },
#endif

#if defined(SPI3_BASE)
    { hwSPI_Index_2, hwGPIO_Pin_A15, GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_B3,  GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_B4,  GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_B5,  GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_C10, GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_C11, GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_C12, GPIO_AF6_SPI3 },
#endif
};

#endif // SPI_PIN_STM32U0_H