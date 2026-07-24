
#ifndef CAN_PIN_H
#define CAN_PIN_H

#include "soc.h"

#include "Driver_Config.h"

#ifdef DEVICE_NUVOTON
#include "Nuvoton/CAN_Pin_Nuvoton.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/CAN_Pin_STM32.h"
#endif

#ifdef DEVICE_TITIVAC
#include "TITivaC/CAN_Pin_TITivaC.h"
#endif

#ifdef DEVICE_TIMSP432E
#include "TIMSP432E/CAN_Pin_TIMSP432E.h"
#endif

#ifdef DEVICE_TIMSPM0
#include "TIMSPM0/CAN_Pin_TIMSPM0.h"
#endif

#endif //CAN_PIN_H