
#ifndef I2C_PIN_H
#define I2C_PIN_H

#include "soc.h"

#ifdef DEVICE_NUVOTON
#include "Nuvoton/I2C_Pin_Nuvoton.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/I2C_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/I2C_Pin_RP2.h"
#endif

#ifdef DEVICE_TITIVAC
#include "TITivaC/I2C_Pin_TITivaC.h"
#endif

#ifdef DEVICE_TIMSP432P
#include "TIMSP432/I2C_Pin_TIMSP432P.h"
#endif

#ifdef DEVICE_TIMSP432E
#include "TIMSP432/I2C_Pin_TIMSP432E.h"
#endif

#ifdef DEVICE_TIMSPM0
#include "TIMSPM0/I2C_Pin_TIMSPM0.h"
#endif

#endif //I2C_PIN_H