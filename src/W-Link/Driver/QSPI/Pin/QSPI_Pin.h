
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

#ifdef DEVICE_TM4C1294
#include "TI/QSPI_Pin_TM4C1294.h"
#endif

#endif //QSPI_MASTER_PIN_H