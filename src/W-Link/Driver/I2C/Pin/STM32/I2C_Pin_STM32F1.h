
#ifndef I2C_PIN_STM32F1_H
#define I2C_PIN_STM32F1_H

#include "I2C_Pin_STM32_Def.h"

typedef enum {
    I2C_Pinset_DEFAULT = 0,
    I2C_Pinset_ALT,
    I2C_Pinset_MAX
} I2C_Pinset_t;

#ifndef CONFIG_I2C0_PINSET
#define CONFIG_I2C0_PINSET I2C_Pinset_DEFAULT
#endif //CONFIG_I2C0_PINSET

#ifndef CONFIG_I2C1_PINSET
#define CONFIG_I2C1_PINSET I2C_Pinset_DEFAULT
#endif //CONFIG_I2C1_PINSET

static const I2C_Pinset_t I2C_Index_Map_Alt[hwI2C_Index_MAX] = {
#if defined (I2C1_BASE)
    CONFIG_I2C0_PINSET,
#endif
#if defined (I2C2_BASE)
    CONFIG_I2C1_PINSET
#endif
};

static const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX][I2C_Pinset_MAX] =
{
#if defined (I2C1_BASE)
    /* ================= I2C1 (I2C0) ================= */
    {
        { hwGPIO_Pin_B6, hwGPIO_Pin_B7 },  // DEFAULT
        { hwGPIO_Pin_B8, hwGPIO_Pin_B9 },  // ALT
    },
#endif
#if defined (I2C2_BASE)
    /* ================= I2C2 (I2C1) ================= */
    {
        /* DEFAULT */
        { hwGPIO_Pin_B10, hwGPIO_Pin_B11 },

        /* ALT */
        { hwGPIO_Pin_F1,  hwGPIO_Pin_F0  },
    },
#endif
};

#endif //I2C_PIN_STM32F1_H