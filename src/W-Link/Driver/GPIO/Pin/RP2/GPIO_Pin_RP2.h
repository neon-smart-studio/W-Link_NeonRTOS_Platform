
#ifndef GPIO_PIN_RP2_H
#define GPIO_PIN_RP2_H

#include <stdint.h>

typedef enum hwGPIO_Pin_t
{
  hwGPIO_Pin_NC = -1,
  hwGPIO_Pin_0 = 0,
  hwGPIO_Pin_1,
  hwGPIO_Pin_2,
  hwGPIO_Pin_3,
  hwGPIO_Pin_4,
  hwGPIO_Pin_5,
  hwGPIO_Pin_6,
  hwGPIO_Pin_7,
  hwGPIO_Pin_8,
  hwGPIO_Pin_9,
  hwGPIO_Pin_10,
  hwGPIO_Pin_11,
  hwGPIO_Pin_12,
  hwGPIO_Pin_13,
  hwGPIO_Pin_14,
  hwGPIO_Pin_15,
  hwGPIO_Pin_16,
  hwGPIO_Pin_17,
  hwGPIO_Pin_18,
  hwGPIO_Pin_19,
  hwGPIO_Pin_20,
  hwGPIO_Pin_21,
  hwGPIO_Pin_22,
  hwGPIO_Pin_23,
  hwGPIO_Pin_24,
  hwGPIO_Pin_25,
  hwGPIO_Pin_26,
  hwGPIO_Pin_27,
  hwGPIO_Pin_28,
  hwGPIO_Pin_29,
  hwGPIO_Pin_MAX
} hwGPIO_Pin;

typedef enum hwGPIO_Int_Pin_t
{
  hwGPIO_Int_Pin_NC = -1,
  hwGPIO_Int_Pin_0 = 0,
  hwGPIO_Int_Pin_1,
  hwGPIO_Int_Pin_2,
  hwGPIO_Int_Pin_3,
  hwGPIO_Int_Pin_4,
  hwGPIO_Int_Pin_5,
  hwGPIO_Int_Pin_6,
  hwGPIO_Int_Pin_7,
  hwGPIO_Int_Pin_8,
  hwGPIO_Int_Pin_9,
  hwGPIO_Int_Pin_10,
  hwGPIO_Int_Pin_11,
  hwGPIO_Int_Pin_12,
  hwGPIO_Int_Pin_13,
  hwGPIO_Int_Pin_14,
  hwGPIO_Int_Pin_15,
  hwGPIO_Int_Pin_16,
  hwGPIO_Int_Pin_17,
  hwGPIO_Int_Pin_18,
  hwGPIO_Int_Pin_19,
  hwGPIO_Int_Pin_20,
  hwGPIO_Int_Pin_21,
  hwGPIO_Int_Pin_22,
  hwGPIO_Int_Pin_23,
  hwGPIO_Int_Pin_24,
  hwGPIO_Int_Pin_25,
  hwGPIO_Int_Pin_26,
  hwGPIO_Int_Pin_27,
  hwGPIO_Int_Pin_28,
  hwGPIO_Int_Pin_29,
  hwGPIO_Int_Pin_MAX
} hwGPIO_Int_Pin;

#endif //GPIO_PIN_RP2_H