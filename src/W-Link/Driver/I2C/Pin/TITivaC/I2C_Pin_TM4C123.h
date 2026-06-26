#ifndef I2C_PIN_TM4C123_H
#define I2C_PIN_TM4C123_H

#include "I2C_Pin_TITivaC_Def.h"

static const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX] =
{
#if defined(I2C0_BASE)
    { hwGPIO_Pin_B2, hwGPIO_Pin_B3 },   // I2C0
#endif

#if defined(I2C1_BASE)
    { hwGPIO_Pin_A6, hwGPIO_Pin_A7 },   // I2C1
#endif

#if defined(I2C2_BASE)
    { hwGPIO_Pin_E4, hwGPIO_Pin_E5 },   // I2C2
#endif

#if defined(I2C3_BASE)
    { hwGPIO_Pin_D0, hwGPIO_Pin_D1 },   // I2C3
#endif
};

#endif // I2C_PIN_TM4C123_H