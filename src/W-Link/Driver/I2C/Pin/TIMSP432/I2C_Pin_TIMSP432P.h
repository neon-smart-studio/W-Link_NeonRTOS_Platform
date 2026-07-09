#ifndef I2C_PIN_TIMSP432P_H
#define I2C_PIN_TIMSP432P_H

#include "I2C_Pin_TIMSP432_Def.h"

static const I2C_Pin_Def I2C_Pin_Def_Table[hwI2C_Index_MAX] =
{
#if defined(EUSCI_B0_BASE)
    { hwGPIO_Pin_B2, hwGPIO_Pin_B3 },   // EUSCI_B0 I2C
#endif

#if defined(EUSCI_B1_BASE)
    { hwGPIO_Pin_C6, hwGPIO_Pin_C7 },   // EUSCI_B1 I2C
#endif

#if defined(EUSCI_B2_BASE)
    { hwGPIO_Pin_D6, hwGPIO_Pin_D7 },   // EUSCI_B2 I2C
#endif

#if defined(EUSCI_B3_BASE)
    { hwGPIO_Pin_J3, hwGPIO_Pin_J2 },   // EUSCI_B3 I2C
#endif
};

#endif // I2C_PIN_TIMSP432P_H