
#ifndef QSPI_MASTER_PIN_H
#define QSPI_MASTER_PIN_H

#include "soc.h"

#include "Driver_Config.h"

#ifdef DEVICE_NUVOTON
#include "Nuvoton/QSPI_Pin_Nuvoton.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/QSPI_Pin_STM32.h"
#endif

#ifdef DEVICE_TITIVAC
#include "TITIVAC/QSPI_Pin_TITivaC.h"
#endif

#ifdef DEVICE_TIMSP432E
#include "TIMSP432/QSPI_Pin_TIMSP432E.h"
#endif

#endif //QSPI_MASTER_PIN_H