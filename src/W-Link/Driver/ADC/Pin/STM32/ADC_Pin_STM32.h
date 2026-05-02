
#ifndef ADC_PIN_STM32_H
#define ADC_PIN_STM32_H

#include "soc.h"

#ifdef STM32C0
#include "ADC_Pin_STM32C0.h"
#endif //STM32C0

#ifdef STM32F0
#include "ADC_Pin_STM32F0.h"
#endif //STM32F0

#ifdef STM32F1
#include "ADC_Pin_STM32F1.h"
#endif //STM32F1

#ifdef STM32F2
#include "ADC_Pin_STM32F2.h"
#endif //STM32F2

#ifdef STM32F3
#include "ADC_Pin_STM32F3.h"
#endif //STM32F3

#ifdef STM32F4
#include "ADC_Pin_STM32F4.h"
#endif //STM32F4

#ifdef STM32F7
#include "ADC_Pin_STM32F7.h"
#endif //STM32F7

#ifdef STM32G0
#include "ADC_Pin_STM32G0.h"
#endif //STM32G0

#ifdef STM32G4
#include "ADC_Pin_STM32G4.h"
#endif //STM32G4

#ifdef STM32H5
#include "ADC_Pin_STM32H5.h"
#endif //STM32H5

#ifdef STM32H7
#include "ADC_Pin_STM32H7.h"
#endif //STM32H7

#ifdef STM32H7RS
#include "ADC_Pin_STM32H7RS.h"
#endif //STM32H7RS

#ifdef STM32L0
#include "ADC_Pin_STM32L0.h"
#endif //STM32L0

#ifdef STM32L1
#include "ADC_Pin_STM32L1.h"
#endif //STM32L1

#ifdef STM32L4
#include "ADC_Pin_STM32L4.h"
#endif //STM32L4

#ifdef STM32L5
#include "ADC_Pin_STM32L5.h"
#endif //STM32L5

#ifdef STM32U0
#include "ADC_Pin_STM32U0.h"
#endif //STM32U0

#ifdef STM32U5
#include "ADC_Pin_STM32U5.h"
#endif //STM32U5

#ifdef STM32WB
#include "ADC_Pin_STM32WB.h"
#endif //STM32WB

#ifdef STM32WBA
#include "ADC_Pin_STM32WBA.h"
#endif //STM32WBA

#ifdef STM32WL
#include "ADC_Pin_STM32WL.h"
#endif //STM32WL

#endif //ADC_PIN_STM32_H