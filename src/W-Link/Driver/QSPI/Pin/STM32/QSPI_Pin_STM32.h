
#ifndef QSPI_PIN_STM32_H
#define QSPI_PIN_STM32_H

#include "QSPI_Pin_STM32_Def.h"

#ifdef STM32F4
#include "QSPI_Pin_STM32F4.h"
#endif //STM32F4

#ifdef STM32F7
#include "QSPI_Pin_STM32F7.h"
#endif //STM32F7

#ifdef STM32G4
#include "QSPI_Pin_STM32G4.h"
#endif //STM32G4

#ifdef STM32H7
#include "QSPI_Pin_STM32H7.h"
#endif //STM32H7

#ifdef STM32L4
#include "QSPI_Pin_STM32L4.h"
#endif // STM32L4

#ifdef STM32WB
#include "QSPI_Pin_STM32WB.h"
#endif //STM32WB

#endif //QSPI_PIN_STM32_H