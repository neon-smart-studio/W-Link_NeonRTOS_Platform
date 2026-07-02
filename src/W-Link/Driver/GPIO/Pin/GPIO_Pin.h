
#ifndef GPIO_PIN_H
#define GPIO_PIN_H

#include "soc.h"

#include "Driver_Config.h"

#ifdef DEVICE_NUVOTON
#include "Nuvoton/GPIO_Pin_Nuvoton.h"
#endif

#ifdef DEVICE_STM32
#include "STM32/GPIO_Pin_STM32.h"
#endif

#ifdef DEVICE_RP2
#include "RP2/GPIO_Pin_RP2.h"
#endif

#ifdef DEVICE_TITIVAC
#include "TITivaC/GPIO_Pin_TITivaC.h"
#endif

#ifdef DEVICE_TICC3200
#include "TICC3200/GPIO_Pin_TICC3200.h"
#endif // DEVICE_TICC3200

#endif //GPIO_PIN_H