#ifndef I2C_PIN_STM32H5_H
#define I2C_PIN_STM32H5_H

#include "I2C_Pin_STM32.h"

typedef enum {
    I2C_Pinset_DEFAULT = 0,
    I2C_Pinset_ALT,
    I2C_Pinset_ALT2,
    I2C_Pinset_ALT3,
    I2C_Pinset_MAX
} I2C_Pinset_t;

#ifndef CONFIG_I2C0_PINSET
#define CONFIG_I2C0_PINSET I2C_Pinset_DEFAULT
#endif

#ifndef CONFIG_I2C1_PINSET
#define CONFIG_I2C1_PINSET I2C_Pinset_DEFAULT
#endif

#ifndef CONFIG_I2C2_PINSET
#define CONFIG_I2C2_PINSET I2C_Pinset_DEFAULT
#endif

#ifndef CONFIG_I2C3_PINSET
#define CONFIG_I2C3_PINSET I2C_Pinset_DEFAULT
#endif

const I2C_Pinset_t I2C_Index_Map_Alt[hwI2C_Index_MAX] = {
#if defined(I2C1_BASE)
    CONFIG_I2C0_PINSET,
#endif
#if defined(I2C2_BASE)
    CONFIG_I2C1_PINSET,
#endif
#if defined(I2C3_BASE)
    CONFIG_I2C2_PINSET,
#endif
#if defined(I2C4_BASE)
    CONFIG_I2C3_PINSET,
#endif
};

const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX][I2C_Pinset_MAX] =
{
#if defined(I2C1_BASE)
    {
        { hwGPIO_Pin_B6, hwGPIO_Pin_B7  }, // SCL, SDA
        { hwGPIO_Pin_B8, hwGPIO_Pin_B9  },
        { hwGPIO_Pin_B6, hwGPIO_Pin_B9  },
        { hwGPIO_Pin_B8, hwGPIO_Pin_B7  },
    },
#endif

#if defined(I2C2_BASE)
    {
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11 },
        { hwGPIO_Pin_B13, hwGPIO_Pin_B14 },
        { hwGPIO_Pin_F1,  hwGPIO_Pin_F0  },
        { hwGPIO_Pin_H4,  hwGPIO_Pin_H5  },
    },
#endif

#if defined(I2C3_BASE)
    {
        { hwGPIO_Pin_C0, hwGPIO_Pin_C1 },
        { hwGPIO_Pin_A7, hwGPIO_Pin_B4 },
        { hwGPIO_Pin_G7, hwGPIO_Pin_G8 },
        { hwGPIO_Pin_H7, hwGPIO_Pin_H8 },
    },
#endif

#if defined(I2C4_BASE)
    {
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7  },
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11 },
        { hwGPIO_Pin_D12, hwGPIO_Pin_D13 },
        { hwGPIO_Pin_F14, hwGPIO_Pin_F15 },
    },
#endif

};

const I2C_AF_Map I2C_Pin_AF_Map[] =
{
#if defined(I2C1_BASE)
    { hwI2C_Index_0, hwGPIO_Pin_B6, GPIO_AF4_I2C1 },
    { hwI2C_Index_0, hwGPIO_Pin_B7, GPIO_AF4_I2C1 },
    { hwI2C_Index_0, hwGPIO_Pin_B8, GPIO_AF4_I2C1 },
    { hwI2C_Index_0, hwGPIO_Pin_B9, GPIO_AF4_I2C1 },
#endif

#if defined(I2C2_BASE)
    { hwI2C_Index_1, hwGPIO_Pin_B10, GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_B11, GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_B13, GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_B14, GPIO_AF4_I2C2 },

#if defined(GPIOF)
    { hwI2C_Index_1, hwGPIO_Pin_F1,  GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_F0,  GPIO_AF4_I2C2 },
#endif

#if defined(GPIOH)
    { hwI2C_Index_1, hwGPIO_Pin_H4,  GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_H5,  GPIO_AF4_I2C2 },
#endif
#endif

#if defined(I2C3_BASE)
    { hwI2C_Index_2, hwGPIO_Pin_A7, GPIO_AF4_I2C3 },
    { hwI2C_Index_2, hwGPIO_Pin_B4, GPIO_AF4_I2C3 },
    { hwI2C_Index_2, hwGPIO_Pin_C0, GPIO_AF4_I2C3 },
    { hwI2C_Index_2, hwGPIO_Pin_C1, GPIO_AF4_I2C3 },

#if defined(GPIOG)
    { hwI2C_Index_2, hwGPIO_Pin_G7, GPIO_AF4_I2C3 },
    { hwI2C_Index_2, hwGPIO_Pin_G8, GPIO_AF4_I2C3 },
#endif

#if defined(GPIOH)
    { hwI2C_Index_2, hwGPIO_Pin_H7, GPIO_AF4_I2C3 },
    { hwI2C_Index_2, hwGPIO_Pin_H8, GPIO_AF4_I2C3 },
#endif
#endif

#if defined(I2C4_BASE)
    { hwI2C_Index_3, hwGPIO_Pin_B6,  GPIO_AF4_I2C4  },
    { hwI2C_Index_3, hwGPIO_Pin_B7,  GPIO_AF4_I2C4  },
    { hwI2C_Index_3, hwGPIO_Pin_B10, GPIO_AF4_I2C4 },
    { hwI2C_Index_3, hwGPIO_Pin_B11, GPIO_AF4_I2C4 },
    { hwI2C_Index_3, hwGPIO_Pin_D12, GPIO_AF4_I2C4 },
    { hwI2C_Index_3, hwGPIO_Pin_D13, GPIO_AF4_I2C4 },

#if defined(GPIOF)
    { hwI2C_Index_3, hwGPIO_Pin_F14, GPIO_AF4_I2C4 },
    { hwI2C_Index_3, hwGPIO_Pin_F15, GPIO_AF4_I2C4 },
#endif
#endif
};

#endif // I2C_PIN_STM32H5_H