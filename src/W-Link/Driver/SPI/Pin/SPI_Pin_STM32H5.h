#ifndef SPI_PIN_STM32H5_H
#define SPI_PIN_STM32H5_H

#include "SPI_Pin_STM32.h"

typedef enum {
    SPI_Pinset_DEFAULT = 0,
    SPI_Pinset_ALT,
    SPI_Pinset_ALT2,
    SPI_Pinset_ALT3,
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
#ifndef CONFIG_SPI4_PINSET
#define CONFIG_SPI4_PINSET SPI_Pinset_DEFAULT
#endif
#ifndef CONFIG_SPI5_PINSET
#define CONFIG_SPI5_PINSET SPI_Pinset_DEFAULT
#endif

const SPI_Pinset_t SPI_Index_Map_Alt[hwSPI_Index_MAX] = {
#if defined(SPI1_BASE)
    CONFIG_SPI0_PINSET,
#endif
#if defined(SPI2_BASE)
    CONFIG_SPI1_PINSET,
#endif
#if defined(SPI3_BASE)
    CONFIG_SPI2_PINSET,
#endif
#if defined(SPI4_BASE)
    CONFIG_SPI3_PINSET,
#endif
#if defined(SPI5_BASE)
    CONFIG_SPI4_PINSET,
#endif
#if defined(SPI6_BASE)
    CONFIG_SPI5_PINSET,
#endif
};

const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX][SPI_Pinset_MAX] =
{
#if defined(SPI1_BASE)
    {
        { hwGPIO_Pin_A7,  hwGPIO_Pin_A6,  hwGPIO_Pin_A5,  hwGPIO_Pin_A4  },
        { hwGPIO_Pin_B5,  hwGPIO_Pin_B4,  hwGPIO_Pin_B3,  hwGPIO_Pin_A15 },
        { hwGPIO_Pin_E15, hwGPIO_Pin_E14, hwGPIO_Pin_E13, hwGPIO_Pin_E12 },
        { hwGPIO_Pin_G4,  hwGPIO_Pin_G3,  hwGPIO_Pin_G2,  hwGPIO_Pin_G5  },
    },
#endif

#if defined(SPI2_BASE)
    {
        { hwGPIO_Pin_B15, hwGPIO_Pin_B14, hwGPIO_Pin_B13, hwGPIO_Pin_B12 },
        { hwGPIO_Pin_C3,  hwGPIO_Pin_C2,  hwGPIO_Pin_B10, hwGPIO_Pin_B9  },
        { hwGPIO_Pin_D4,  hwGPIO_Pin_D3,  hwGPIO_Pin_D1,  hwGPIO_Pin_D0  },
        { hwGPIO_Pin_I3,  hwGPIO_Pin_I2,  hwGPIO_Pin_I1,  hwGPIO_Pin_I0  },
    },
#endif

#if defined(SPI3_BASE)
    {
        { hwGPIO_Pin_B5,  hwGPIO_Pin_B4,  hwGPIO_Pin_B3,  hwGPIO_Pin_A15 },
        { hwGPIO_Pin_C12, hwGPIO_Pin_C11, hwGPIO_Pin_C10, hwGPIO_Pin_A4  },
        { hwGPIO_Pin_D6,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_G10, hwGPIO_Pin_G9, hwGPIO_Pin_G12 },
    },
#endif

#if defined(SPI4_BASE)
    {
        { hwGPIO_Pin_E6,  hwGPIO_Pin_E5,  hwGPIO_Pin_E2,  hwGPIO_Pin_E4  },
        { hwGPIO_Pin_E14, hwGPIO_Pin_E13, hwGPIO_Pin_E12, hwGPIO_Pin_E11 },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC  },
    },
#endif

#if defined(SPI5_BASE)
    {
        { hwGPIO_Pin_F9,  hwGPIO_Pin_F8,  hwGPIO_Pin_F7,  hwGPIO_Pin_F6  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC  },
    },
#endif

#if defined(SPI6_BASE)
    {
        { hwGPIO_Pin_G14, hwGPIO_Pin_G12, hwGPIO_Pin_G13, hwGPIO_Pin_G8  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC  },
        { hwGPIO_Pin_NC,  hwGPIO_Pin_NC,  hwGPIO_Pin_NC, hwGPIO_Pin_NC  },
    },
#endif
};

const SPI_AF_Map SPI_Pin_AF_Map[] =
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
#if defined(GPIOE)
    { hwSPI_Index_0, hwGPIO_Pin_E12, GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_E13, GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_E14, GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_E15, GPIO_AF5_SPI1 },
#endif
#if defined(GPIOG)
    { hwSPI_Index_0, hwGPIO_Pin_G2,  GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_G3,  GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_G4,  GPIO_AF5_SPI1 },
    { hwSPI_Index_0, hwGPIO_Pin_G5,  GPIO_AF5_SPI1 },
#endif
#endif

#if defined(SPI2_BASE)
    { hwSPI_Index_1, hwGPIO_Pin_B9,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B10, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B12, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B13, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B14, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_B15, GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_C2,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_C3,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_D0,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_D1,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_D3,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_D4,  GPIO_AF5_SPI2 },
#if defined(GPIOI)
    { hwSPI_Index_1, hwGPIO_Pin_I0,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_I1,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_I2,  GPIO_AF5_SPI2 },
    { hwSPI_Index_1, hwGPIO_Pin_I3,  GPIO_AF5_SPI2 },
#endif
#endif

#if defined(SPI3_BASE)
    { hwSPI_Index_2, hwGPIO_Pin_A4,  GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_A15, GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_B3,  GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_B4,  GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_B5,  GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_C10, GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_C11, GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_C12, GPIO_AF6_SPI3 },
#if defined(GPIOG)
    { hwSPI_Index_2, hwGPIO_Pin_G9,  GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_G10, GPIO_AF6_SPI3 },
    { hwSPI_Index_2, hwGPIO_Pin_G12, GPIO_AF6_SPI3 },
#endif
#if defined(GPIOD)
    { hwSPI_Index_2, hwGPIO_Pin_D6, GPIO_AF5_SPI3 },
#endif
#endif

#if defined(SPI4_BASE)
    { hwSPI_Index_3, hwGPIO_Pin_E2,  GPIO_AF5_SPI4 },
    { hwSPI_Index_3, hwGPIO_Pin_E4,  GPIO_AF5_SPI4 },
    { hwSPI_Index_3, hwGPIO_Pin_E5,  GPIO_AF5_SPI4 },
    { hwSPI_Index_3, hwGPIO_Pin_E6,  GPIO_AF5_SPI4 },
    { hwSPI_Index_3, hwGPIO_Pin_E11, GPIO_AF5_SPI4 },
    { hwSPI_Index_3, hwGPIO_Pin_E12, GPIO_AF5_SPI4 },
    { hwSPI_Index_3, hwGPIO_Pin_E13, GPIO_AF5_SPI4 },
    { hwSPI_Index_3, hwGPIO_Pin_E14, GPIO_AF5_SPI4 },
#endif

#if defined(SPI5_BASE)
    { hwSPI_Index_4, hwGPIO_Pin_F6, GPIO_AF5_SPI5 },
    { hwSPI_Index_4, hwGPIO_Pin_F7, GPIO_AF5_SPI5 },
    { hwSPI_Index_4, hwGPIO_Pin_F8, GPIO_AF5_SPI5 },
    { hwSPI_Index_4, hwGPIO_Pin_F9, GPIO_AF5_SPI5 },
#endif

#if defined(SPI6_BASE)
    { hwSPI_Index_5, hwGPIO_Pin_G8,  GPIO_AF5_SPI6 },
    { hwSPI_Index_5, hwGPIO_Pin_G12, GPIO_AF5_SPI6 },
    { hwSPI_Index_5, hwGPIO_Pin_G13, GPIO_AF5_SPI6 },
    { hwSPI_Index_5, hwGPIO_Pin_G14, GPIO_AF5_SPI6 },
#endif
};

#endif // SPI_PIN_STM32H5_H