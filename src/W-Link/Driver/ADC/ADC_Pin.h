
#ifndef ADC_PIN_H
#define ADC_PIN_H

#ifdef STM32C0
#include "Pin/ADC_Pin_STM32C0.h"
#endif //STM32C0

#ifdef STM32F0
#include "Pin/ADC_Pin_STM32F0.h"
#endif //STM32F0

#ifdef STM32F1
#include "Pin/ADC_Pin_STM32F1.h"
#endif //STM32F1

#ifdef STM32F2
#include "Pin/ADC_Pin_STM32F2.h"
#endif //STM32F2

#ifdef STM32F3
#include "Pin/ADC_Pin_STM32F3.h"
#endif //STM32F3

#ifdef STM32F4
#include "Pin/ADC_Pin_STM32F4.h"
#endif //STM32F4

#ifdef STM32F7
#include "Pin/ADC_Pin_STM32F7.h"
#endif //STM32F7

#ifdef STM32G0
#include "Pin/ADC_Pin_STM32G0.h"
#endif //STM32G0

#ifdef STM32G4
#include "Pin/ADC_Pin_STM32G4.h"
#endif //STM32G4

#ifdef STM32U0
#include "Pin/ADC_Pin_STM32U0.h"
#endif //STM32U0

#ifdef STM32U5
#include "Pin/ADC_Pin_STM32U5.h"
#endif //STM32U5

#if defined(STM32L0) || defined(STM32L1) || defined(STM32L4) || defined(STM32L5)
#include "Pin/ADC_Pin_STM32Lx.h"
#endif //STM32L0 || STM32L1 || STM32L4 || STM32L5

#ifdef STM32H7
#include "Pin/ADC_Pin_STM32H7.h"
#endif //STM32H7

#endif //ADC_PIN_H