
#ifndef SPI_MASTER_PIN_H
#define SPI_MASTER_PIN_H

#include "soc.h"

#ifdef DEVICE_STM32
#include "STM32/SPI_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/SPI_Pin_RP2.h"
#endif

#ifdef DEVICE_TM4C1294
#include "TI/SPI_Pin_TM4C1294.h"
#endif

#endif //SPI_MASTER_PIN_H