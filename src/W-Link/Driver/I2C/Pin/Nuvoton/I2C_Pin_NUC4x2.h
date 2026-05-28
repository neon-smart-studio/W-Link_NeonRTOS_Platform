#ifndef I2C_PIN_NUC4X2_H
#define I2C_PIN_NUC4X2_H

#include "I2C_Pin_Nuvoton_Def.h"

typedef enum {
    I2C_Pinset_DEFAULT = 0,
    I2C_Pinset_ALT,
    I2C_Pinset_ALT2,
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

#ifndef CONFIG_I2C4_PINSET
#define CONFIG_I2C4_PINSET I2C_Pinset_DEFAULT
#endif

static const I2C_Pinset_t I2C_Index_Map_Alt[hwI2C_Index_MAX] = {
#if defined(I2C0)
    CONFIG_I2C0_PINSET,
#endif
#if defined(I2C1)
    CONFIG_I2C1_PINSET,
#endif
#if defined(I2C2)
    CONFIG_I2C2_PINSET,
#endif
#if defined(I2C3)
    CONFIG_I2C3_PINSET,
#endif
#if defined(I2C4)
    CONFIG_I2C4_PINSET,
#endif
};

static const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX][I2C_Pinset_MAX] =
{
#if defined(I2C0)
    /* ================= I2C0 ================= */
    {
        /* DEFAULT: SCL, SDA */
        { hwGPIO_Pin_A10, hwGPIO_Pin_A11 },

        /* ALT: SCL, SDA */
        { hwGPIO_Pin_B4,  hwGPIO_Pin_B5  },

        /* ALT2: SCL, SDA */
        { hwGPIO_Pin_D14, hwGPIO_Pin_D15 },
    },
#endif

#if defined(I2C1)
    /* ================= I2C1 ================= */
    {
        /* DEFAULT: SCL, SDA */
        { hwGPIO_Pin_A12, hwGPIO_Pin_A13 },

        /* ALT: SCL, SDA */
        { hwGPIO_Pin_B6,  hwGPIO_Pin_B7  },

        /* ALT2: SCL, SDA */
        { hwGPIO_Pin_E0,  hwGPIO_Pin_E1  },
    },
#endif

#if defined(I2C2)
    /* ================= I2C2 ================= */
    {
        /* DEFAULT: SCL, SDA */
        { hwGPIO_Pin_A14, hwGPIO_Pin_A15 },

        /* ALT: SCL, SDA */
        { hwGPIO_Pin_B8,  hwGPIO_Pin_B9  },

        /* ALT2: SCL, SDA */
        { hwGPIO_Pin_E2,  hwGPIO_Pin_E3  },
    },
#endif

#if defined(I2C3)
    /* ================= I2C3 ================= */
    {
        /* DEFAULT: SCL, SDA */
        { hwGPIO_Pin_C0, hwGPIO_Pin_C1 },

        /* ALT: SCL, SDA */
        { hwGPIO_Pin_D0, hwGPIO_Pin_D1 },

        /* ALT2: SCL, SDA */
        { hwGPIO_Pin_E4, hwGPIO_Pin_E5 },
    },
#endif

#if defined(I2C4)
    /* ================= I2C4 ================= */
    {
        /* DEFAULT: SCL, SDA */
        { hwGPIO_Pin_C2, hwGPIO_Pin_C3 },

        /* ALT: SCL, SDA */
        { hwGPIO_Pin_D2, hwGPIO_Pin_D3 },

        /* ALT2: SCL, SDA */
        { hwGPIO_Pin_E6, hwGPIO_Pin_E7 },
    },
#endif
};

#endif /* I2C_PIN_NUC4X2_H */