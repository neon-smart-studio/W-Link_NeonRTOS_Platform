#ifndef I2C_PIN_RP2_H
#define I2C_PIN_RP2_H

#include "I2C_Pin_RP2_Def.h"

typedef enum {
    I2C_Pinset_DEFAULT = 0,
    I2C_Pinset_ALT,
    I2C_Pinset_ALT2,
    I2C_Pinset_ALT3,
    I2C_Pinset_ALT4,
    I2C_Pinset_ALT5,
    I2C_Pinset_MAX
} I2C_Pinset_t;

#ifndef CONFIG_I2C0_PINSET
#define CONFIG_I2C0_PINSET I2C_Pinset_DEFAULT
#endif

#ifndef CONFIG_I2C1_PINSET
#define CONFIG_I2C1_PINSET I2C_Pinset_DEFAULT
#endif

static const I2C_Pinset_t I2C_Index_Map_Alt[hwI2C_Index_MAX] = {
    CONFIG_I2C0_PINSET,
    CONFIG_I2C1_PINSET,
};

static const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX][I2C_Pinset_MAX] =
{
    /* ================= I2C1 (I2C0) ================= */
    {
        /* DEFAULT: SCL, SDA */
        { hwGPIO_Pin_5, hwGPIO_Pin_4 },

        /* ALT: SCL, SDA */
        { hwGPIO_Pin_1, hwGPIO_Pin_0 },
        /* ALT1: SCL, SDA */
        { hwGPIO_Pin_9, hwGPIO_Pin_8 },
        /* ALT2: SCL, SDA */
        { hwGPIO_Pin_13, hwGPIO_Pin_12 },
        /* ALT3: SCL, SDA */
        { hwGPIO_Pin_17, hwGPIO_Pin_16 },
        /* ALT4: SCL, SDA */
        { hwGPIO_Pin_21, hwGPIO_Pin_20 },
    },

    /* ================= I2C2 (I2C1) ================= */
    {
        /* DEFAULT: SCL, SDA */
        { hwGPIO_Pin_3, hwGPIO_Pin_2 },

        /* ALT: SCL, SDA */
        { hwGPIO_Pin_7, hwGPIO_Pin_6 },
        /* ALT1: SCL, SDA */
        { hwGPIO_Pin_11, hwGPIO_Pin_10 },
        /* ALT2: SCL, SDA */
        { hwGPIO_Pin_15, hwGPIO_Pin_14 },
        /* ALT3: SCL, SDA */
        { hwGPIO_Pin_19, hwGPIO_Pin_18 },
        /* ALT4: SCL, SDA */
        { hwGPIO_Pin_27, hwGPIO_Pin_26 },
    },
};

#endif // I2C_PIN_RP2_H