#ifndef I2C_PIN_STM32L5_H
#define I2C_PIN_STM32L5_H

#include "I2C_Pin_STM32.h"

typedef enum {
    I2C_Pinset_DEFAULT = 0,
    I2C_Pinset_ALT,
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

static const I2C_Pinset_t I2C_Index_Map_Alt[hwI2C_Index_MAX] = {
#if defined(I2C1_BASE)
    CONFIG_I2C0_PINSET,
#endif
#if defined(I2C2_BASE)
    CONFIG_I2C1_PINSET,
#endif
#if defined(I2C3_BASE)
    CONFIG_I2C2_PINSET,
#endif
};

static const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX][I2C_Pinset_MAX] =
{
#if defined(I2C1_BASE)
    {
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7  },
        { hwGPIO_Pin_B8,  hwGPIO_Pin_B9  },
    },
#endif

#if defined(I2C2_BASE)
    {
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11 },
        { hwGPIO_Pin_F1,  hwGPIO_Pin_F0  },
    },
#endif

#if defined(I2C3_BASE)
    {
        { hwGPIO_Pin_C0,  hwGPIO_Pin_C1  },
        { hwGPIO_Pin_A7,  hwGPIO_Pin_B4  },
    },
#endif
};

static const I2C_AF_Map I2C_Pin_AF_Map[] =
{
#if defined(I2C1_BASE)
    { hwI2C_Index_0, hwGPIO_Pin_B6,  GPIO_AF4_I2C1 },
    { hwI2C_Index_0, hwGPIO_Pin_B7,  GPIO_AF4_I2C1 },
    { hwI2C_Index_0, hwGPIO_Pin_B8,  GPIO_AF4_I2C1 },
    { hwI2C_Index_0, hwGPIO_Pin_B9,  GPIO_AF4_I2C1 },
#endif

#if defined(I2C2_BASE)
    { hwI2C_Index_1, hwGPIO_Pin_B10, GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_B11, GPIO_AF4_I2C2 },
#if defined(GPIOF)
    { hwI2C_Index_1, hwGPIO_Pin_F1,  GPIO_AF4_I2C2 },
    { hwI2C_Index_1, hwGPIO_Pin_F0,  GPIO_AF4_I2C2 },
#endif
#endif

#if defined(I2C3_BASE)
    { hwI2C_Index_2, hwGPIO_Pin_C0,  GPIO_AF4_I2C3 },
    { hwI2C_Index_2, hwGPIO_Pin_C1,  GPIO_AF4_I2C3 },
    { hwI2C_Index_2, hwGPIO_Pin_A7,  GPIO_AF4_I2C3 },
    { hwI2C_Index_2, hwGPIO_Pin_B4,  GPIO_AF4_I2C3 },
#endif
};

#endif