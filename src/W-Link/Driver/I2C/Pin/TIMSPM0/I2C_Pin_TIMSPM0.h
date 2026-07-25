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

#if defined(I2C1_BASE)
    /*
     * L130x/L134x do not have GPIOB, so their I2C1 uses PA4/PA3.
     * Other traditional-I2C MSPM0 families use PB2/PB3.
     */
#if defined(MSPM0L130x) || defined(MSPM0L134x)
    { hwGPIO_Pin_A4, hwGPIO_Pin_A3 }, /* I2C1: SCL, SDA */
#else
    { hwGPIO_Pin_B2, hwGPIO_Pin_B3 }, /* I2C1: SCL, SDA */
#endif
#endif

#if defined(I2C2_BASE)
    /*
     * I2C2 exists on G151x/G351x/G352x and L122x/L222x.
     */
    { hwGPIO_Pin_B6, hwGPIO_Pin_B7 }, /* I2C2: SCL, SDA */
#endif
};

#endif // I2C_PIN_TIMSPM0_H