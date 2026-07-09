
#ifndef SPI_MASTER_PIN_H
#define SPI_MASTER_PIN_H

#include "soc.h"

#include "Driver_Config.h"

#ifdef DEVICE_NUVOTON
#include "Nuvoton/SPI_Pin_Nuvoton.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/SPI_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/SPI_Pin_RP2.h"
#endif

#ifdef DEVICE_TITIVAC
#include "TITivaC/SPI_Pin_TITivaC.h"
#endif

#ifdef DEVICE_TIMSP432P
#include "TIMSP432/SPI_Pin_TIMSP432P.h"
#endif

#ifdef DEVICE_TIMSP432E
#include "TIMSP432/SPI_Pin_TIMSP432E.h"
#endif

#endif //SPI_MASTER_PIN_H