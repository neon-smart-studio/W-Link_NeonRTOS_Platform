#ifndef I2C_PIN_TIMSP432R_H
#define I2C_PIN_TIMSP432R_H

#include "I2C_Pin_TIMSP432_Def.h"

static const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX] =
{
#if defined(I2C0_BASE)
    { hwGPIO_Pin_B2, hwGPIO_Pin_B3 },   // I2C0
#endif

#if defined(I2C1_BASE)
    { hwGPIO_Pin_G0, hwGPIO_Pin_G1 },   // I2C1
#endif

#if defined(I2C2_BASE)
    { hwGPIO_Pin_N5, hwGPIO_Pin_N4 },   // I2C2
#endif

#if defined(I2C3_BASE)
    { hwGPIO_Pin_K4, hwGPIO_Pin_K5 },   // I2C3
#endif

#if defined(I2C4_BASE)
    { hwGPIO_Pin_K6, hwGPIO_Pin_K7 },   // I2C4
#endif

#if defined(I2C5_BASE)
    { hwGPIO_Pin_B0, hwGPIO_Pin_B1 },   // I2C5
#endif

#if defined(I2C6_BASE)
    { hwGPIO_Pin_A6, hwGPIO_Pin_A7 },   // I2C6
#endif

#if defined(I2C7_BASE)
    { hwGPIO_Pin_D0, hwGPIO_Pin_D1 },   // I2C7
#endif

#if defined(I2C8_BASE)
    { hwGPIO_Pin_D2, hwGPIO_Pin_D3 },   // I2C8
#endif

#if defined(I2C9_BASE)
    { hwGPIO_Pin_A0, hwGPIO_Pin_A1 },   // I2C9
#endif
};

#endif // I2C_PIN_TIMSP432R_H