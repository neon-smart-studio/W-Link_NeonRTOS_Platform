#ifndef GPIO_TIMSP432E_H
#define GPIO_TIMSP432E_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "GPIO/GPIO.h"

extern bool gpio_pin_init_status[hwGPIO_Pin_MAX];

void GPIO_Enable_Port_Clock(uint32_t base);
void GPIO_Disable_Port_Clock(uint32_t base);

#endif //GPIO_TIMSP432E_H