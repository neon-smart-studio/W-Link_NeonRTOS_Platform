#ifndef I2C_PIN_TIMSPM0_H
#define I2C_PIN_TIMSPM0_H

#include "I2C_Pin_TIMSPM0_Def.h"

static const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX] =
{
#if defined(I2C0_BASE)
    /*
     * Valid on:
     * C110x, C1105/C1106, G110x/G150x/G310x/G350x,
     * G151x/G351x/G352x, H321x, L110x/L111x,
     * L122x/L222x, L130x/L134x.
     *
     * PA1: I2C0_SCL
     * PA0: I2C0_SDA
     */
    { hwGPIO_Pin_A1, hwGPIO_Pin_A0 },
#endif
#if defined(UC0_I2CC_BASE)

    /*
     * MSPM0G511x/G518x UC0
     * PA1: UC0_SCL_RX
     * PA0: UC0_SDA_TX
     */
    [hwI2C_Index_0] =
    {
        hwGPIO_Pin_A1,
        hwGPIO_Pin_A0
    },

#endif

#if defined(I2C1_BASE)
    /*
     * L130x/L134x do not have GPIOB, so their I2C1 uses PA4/PA3.
     * Other traditional-I2C MSPM0 families use PB2/PB3.
     */
#if defined(MSPM0L130x) || defined(MSPM0L134x)
    { hwGPIO_Pin_A4, hwGPIO_Pin_A3 }, /* I2C1: SCL, SDA */
#elif defined(MSPM0C1105) || defined(MSPM0C1106) || defined(MSPM0H321x)
    { hwGPIO_Pin_B2, hwGPIO_Pin_B3 }, /* I2C1: SCL, SDA */
#else
    { hwGPIO_Pin_B2, hwGPIO_Pin_B3 }, /* I2C1: SCL, SDA */
#endif
#endif
#if defined(UC1_I2CC_BASE)

    /*
     * MSPM0G511x/G518x UC1
     * PA9 : UC1_SCL_RX
     * PA10: UC1_SDA_TX
     */
    [hwI2C_Index_1] =
    {
        hwGPIO_Pin_A9,
        hwGPIO_Pin_A10
    },

#endif

#if defined(I2C2_BASE)
    /*
     * I2C2 exists on G151x/G351x/G352x and L122x/L222x.
     */
#if defined(MSPM0L122x) || defined(MSPM0L222x)
    { hwGPIO_Pin_B6, hwGPIO_Pin_B7 }, /* I2C2: SCL, SDA */
#else
    { hwGPIO_Pin_B6, hwGPIO_Pin_B7 }, /* I2C2: SCL, SDA */
#endif
#endif

#if defined(UC5_I2CC_BASE)

    /*
     * MSPM0L111x UC5
     * PA1: UC5_SCL
     * PA0: UC5_SDA
     */
    [hwI2C_Index_5] =
    {
        hwGPIO_Pin_A4,
        hwGPIO_Pin_A3
    },

#endif

#if defined(UC6_I2CC_BASE)

    /*
     * MSPM0L112x/L211x UC6
     * PA1: UC6_SCL
     * PA0: UC6_SDA
     */
    [hwI2C_Index_6] =
    {
        hwGPIO_Pin_A11,
        hwGPIO_Pin_A10
    },

#endif
};

#endif // I2C_PIN_TIMSPM0_H