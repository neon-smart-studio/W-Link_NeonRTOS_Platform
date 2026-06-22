#ifndef GPIO_TI_H
#define GPIO_TI_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "GPIO/GPIO.h"

extern bool gpio_pin_init_status[hwGPIO_Pin_MAX];

void GPIO_Int_Handler(hwGPIO_Int_Pin irq_pin);

void GPIO_NVIC_Init(hwGPIO_Int_Pin irq_pin);
void GPIO_NVIC_DeInit(hwGPIO_Int_Pin irq_pin);

#endif //GPIO_TI_H