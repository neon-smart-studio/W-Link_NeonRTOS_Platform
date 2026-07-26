
#ifndef GPIO_PIN_TIMSP432P_H
#define GPIO_PIN_TIMSP432P_H

#include <stdint.h>

#include "soc.h"

#ifdef DEVICE_TIMSP432P

typedef enum hwGPIO_Pin_t
{
  hwGPIO_Pin_NC = -1,

  // A = P1 + P2
  hwGPIO_Pin_A0 = 0,   // P1.0
  hwGPIO_Pin_A1,       // P1.1
  hwGPIO_Pin_A2,       // P1.2
  hwGPIO_Pin_A3,       // P1.3
  hwGPIO_Pin_A4,       // P1.4
  hwGPIO_Pin_A5,       // P1.5
  hwGPIO_Pin_A6,       // P1.6
  hwGPIO_Pin_A7,       // P1.7
  hwGPIO_Pin_A8,       // P2.0
  hwGPIO_Pin_A9,       // P2.1
  hwGPIO_Pin_A10,      // P2.2
  hwGPIO_Pin_A11,      // P2.3
  hwGPIO_Pin_A12,      // P2.4
  hwGPIO_Pin_A13,      // P2.5
  hwGPIO_Pin_A14,      // P2.6
  hwGPIO_Pin_A15,      // P2.7

  // B = P3 + P4
  hwGPIO_Pin_B0,       // P3.0
  hwGPIO_Pin_B1,
  hwGPIO_Pin_B2,
  hwGPIO_Pin_B3,
  hwGPIO_Pin_B4,
  hwGPIO_Pin_B5,
  hwGPIO_Pin_B6,
  hwGPIO_Pin_B7,
  hwGPIO_Pin_B8,       // P4.0
  hwGPIO_Pin_B9,
  hwGPIO_Pin_B10,
  hwGPIO_Pin_B11,
  hwGPIO_Pin_B12,
  hwGPIO_Pin_B13,
  hwGPIO_Pin_B14,
  hwGPIO_Pin_B15,

  // C = P5 + P6
  hwGPIO_Pin_C0,       // P5.0
  hwGPIO_Pin_C1,
  hwGPIO_Pin_C2,
  hwGPIO_Pin_C3,
  hwGPIO_Pin_C4,
  hwGPIO_Pin_C5,
  hwGPIO_Pin_C6,
  hwGPIO_Pin_C7,
  hwGPIO_Pin_C8,       // P6.0
  hwGPIO_Pin_C9,
  hwGPIO_Pin_C10,
  hwGPIO_Pin_C11,
  hwGPIO_Pin_C12,
  hwGPIO_Pin_C13,
  hwGPIO_Pin_C14,
  hwGPIO_Pin_C15,

  // D = P7 + P8
  hwGPIO_Pin_D0,       // P7.0
  hwGPIO_Pin_D1,
  hwGPIO_Pin_D2,
  hwGPIO_Pin_D3,
  hwGPIO_Pin_D4,
  hwGPIO_Pin_D5,
  hwGPIO_Pin_D6,
  hwGPIO_Pin_D7,
  hwGPIO_Pin_D8,       // P8.0
  hwGPIO_Pin_D9,
  hwGPIO_Pin_D10,
  hwGPIO_Pin_D11,
  hwGPIO_Pin_D12,
  hwGPIO_Pin_D13,
  hwGPIO_Pin_D14,
  hwGPIO_Pin_D15,

  // E = P9 + P10
  hwGPIO_Pin_E0,       // P9.0
  hwGPIO_Pin_E1,
  hwGPIO_Pin_E2,
  hwGPIO_Pin_E3,
  hwGPIO_Pin_E4,
  hwGPIO_Pin_E5,
  hwGPIO_Pin_E6,
  hwGPIO_Pin_E7,
  hwGPIO_Pin_E8,       // P10.0
  hwGPIO_Pin_E9,
  hwGPIO_Pin_E10,
  hwGPIO_Pin_E11,
  hwGPIO_Pin_E12,
  hwGPIO_Pin_E13,
  hwGPIO_Pin_E14,
  hwGPIO_Pin_E15,

  // PJ
  hwGPIO_Pin_J0,       // PJ.0
  hwGPIO_Pin_J1,
  hwGPIO_Pin_J2,
  hwGPIO_Pin_J3,

  hwGPIO_Pin_MAX
} hwGPIO_Pin;

typedef enum hwGPIO_Int_Pin_t
{
  hwGPIO_Int_Pin_NC = -1,

  // A = P1 + P2
  hwGPIO_Int_Pin_A0 = 0,   // P1.0
  hwGPIO_Int_Pin_A1,       // P1.1
  hwGPIO_Int_Pin_A2,       // P1.2
  hwGPIO_Int_Pin_A3,       // P1.3
  hwGPIO_Int_Pin_A4,       // P1.4
  hwGPIO_Int_Pin_A5,       // P1.5
  hwGPIO_Int_Pin_A6,       // P1.6
  hwGPIO_Int_Pin_A7,       // P1.7
  hwGPIO_Int_Pin_A8,       // P2.0
  hwGPIO_Int_Pin_A9,       // P2.1
  hwGPIO_Int_Pin_A10,      // P2.2
  hwGPIO_Int_Pin_A11,      // P2.3
  hwGPIO_Int_Pin_A12,      // P2.4
  hwGPIO_Int_Pin_A13,      // P2.5
  hwGPIO_Int_Pin_A14,      // P2.6
  hwGPIO_Int_Pin_A15,      // P2.7

  // B = P3 + P4
  hwGPIO_Int_Pin_B0,       // P3.0
  hwGPIO_Int_Pin_B1,
  hwGPIO_Int_Pin_B2,
  hwGPIO_Int_Pin_B3,
  hwGPIO_Int_Pin_B4,
  hwGPIO_Int_Pin_B5,
  hwGPIO_Int_Pin_B6,
  hwGPIO_Int_Pin_B7,
  hwGPIO_Int_Pin_B8,       // P4.0
  hwGPIO_Int_Pin_B9,
  hwGPIO_Int_Pin_B10,
  hwGPIO_Int_Pin_B11,
  hwGPIO_Int_Pin_B12,
  hwGPIO_Int_Pin_B13,
  hwGPIO_Int_Pin_B14,
  hwGPIO_Int_Pin_B15,

  // C = P5 + P6
  hwGPIO_Int_Pin_C0,       // P5.0
  hwGPIO_Int_Pin_C1,
  hwGPIO_Int_Pin_C2,
  hwGPIO_Int_Pin_C3,
  hwGPIO_Int_Pin_C4,
  hwGPIO_Int_Pin_C5,
  hwGPIO_Int_Pin_C6,
  hwGPIO_Int_Pin_C7,
  hwGPIO_Int_Pin_C8,       // P6.0
  hwGPIO_Int_Pin_C9,
  hwGPIO_Int_Pin_C10,
  hwGPIO_Int_Pin_C11,
  hwGPIO_Int_Pin_C12,
  hwGPIO_Int_Pin_C13,
  hwGPIO_Int_Pin_C14,
  hwGPIO_Int_Pin_C15,

  // D = P7 + P8
  hwGPIO_Int_Pin_D0,       // P7.0
  hwGPIO_Int_Pin_D1,
  hwGPIO_Int_Pin_D2,
  hwGPIO_Int_Pin_D3,
  hwGPIO_Int_Pin_D4,
  hwGPIO_Int_Pin_D5,
  hwGPIO_Int_Pin_D6,
  hwGPIO_Int_Pin_D7,
  hwGPIO_Int_Pin_D8,       // P8.0
  hwGPIO_Int_Pin_D9,
  hwGPIO_Int_Pin_D10,
  hwGPIO_Int_Pin_D11,
  hwGPIO_Int_Pin_D12,
  hwGPIO_Int_Pin_D13,
  hwGPIO_Int_Pin_D14,
  hwGPIO_Int_Pin_D15,

  // E = P9 + P10
  hwGPIO_Int_Pin_E0,       // P9.0
  hwGPIO_Int_Pin_E1,
  hwGPIO_Int_Pin_E2,
  hwGPIO_Int_Pin_E3,
  hwGPIO_Int_Pin_E4,
  hwGPIO_Int_Pin_E5,
  hwGPIO_Int_Pin_E6,
  hwGPIO_Int_Pin_E7,
  hwGPIO_Int_Pin_E8,       // P10.0
  hwGPIO_Int_Pin_E9,
  hwGPIO_Int_Pin_E10,
  hwGPIO_Int_Pin_E11,
  hwGPIO_Int_Pin_E12,
  hwGPIO_Int_Pin_E13,
  hwGPIO_Int_Pin_E14,
  hwGPIO_Int_Pin_E15,

  // PJ
  hwGPIO_Int_Pin_J0,       // PJ.0
  hwGPIO_Int_Pin_J1,
  hwGPIO_Int_Pin_J2,
  hwGPIO_Int_Pin_J3,

  hwGPIO_Int_Pin_MAX
} hwGPIO_Int_Pin;

uint8_t GPIO_Map_Soc_Port_Base(hwGPIO_Pin pin);
uint8_t GPIO_Map_Soc_Int_Port_Base(hwGPIO_Int_Pin pin);
uint16_t GPIO_Map_Soc_Pin_Mask(hwGPIO_Pin pin);
uint16_t GPIO_Map_Soc_Int_Pin_Mask(hwGPIO_Int_Pin pin);
hwGPIO_Int_Pin GPIO_Map_Int_Pin_By_Mask(uint8_t portBase, uint16_t intMask);

#endif // DEVICE_TIMSP432P

#endif // GPIO_PIN_TIMSP432P_H
