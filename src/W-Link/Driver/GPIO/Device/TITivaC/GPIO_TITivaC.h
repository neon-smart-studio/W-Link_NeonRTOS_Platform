#ifndef GPIO_TITIVAC_H
#define GPIO_TITIVAC_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "GPIO/GPIO.h"

extern bool gpio_pin_init_status[hwGPIO_Pin_MAX];

void GPIO_Enable_Port_Clock(uint32_t base);
void GPIO_Disable_Port_Clock(uint32_t base);

void GPIO_Int_Handler(hwGPIO_Int_Pin irq_pin);

void GPIO_NVIC_Init(hwGPIO_Int_Pin irq_pin);
void GPIO_NVIC_DeInit(hwGPIO_Int_Pin irq_pin);

#endif //GPIO_TITIVAC_H