#ifndef GPIO_STM32_H
#define GPIO_STM32_H

#include <stdbool.h>
#include <stdint.h>

#include "soc.h"
#include "GPIO/GPIO.h"

#define GPIO_EXTI_NVIC_PRIORITY      5
#define GPIO_EXTI_NVIC_SUB_PRIORITY  0

typedef enum {
    GPIO_EXTI_Line_0,
    GPIO_EXTI_Line_1,
    GPIO_EXTI_Line_2,
    GPIO_EXTI_Line_3,
    GPIO_EXTI_Line_4,
    GPIO_EXTI_Line_5,
    GPIO_EXTI_Line_6,
    GPIO_EXTI_Line_7,
    GPIO_EXTI_Line_8,
    GPIO_EXTI_Line_9,
    GPIO_EXTI_Line_10,
    GPIO_EXTI_Line_11,
    GPIO_EXTI_Line_12,
    GPIO_EXTI_Line_13,
    GPIO_EXTI_Line_14,
    GPIO_EXTI_Line_15,
    GPIO_EXTI_Line_MAX,
} GPIO_EXTI_Line;

typedef struct {
    hwGPIO_Int_Pin irq_pin;
    hwGPIO_Interrupt_Mode mode;
    EXTI_HandleTypeDef hexti;
} GPIO_EXTI_Desc;

extern bool gpio_pin_init_status[hwGPIO_Pin_MAX];

void GPIO_Enable_RCC_Clock(GPIO_TypeDef *base);
void GPIO_Disable_RCC_Clock(GPIO_TypeDef *base);

GPIO_EXTI_Line GPIO_Map_EXTI_Line_Index(uint32_t exti_line);

void GPIO_EXTI_NVIC_Init(GPIO_EXTI_Line line);
void GPIO_EXTI_NVIC_DeInit(GPIO_EXTI_Line line, GPIO_EXTI_Desc *desc);

void GPIO_EXTI_Dispatch(GPIO_EXTI_Line line);

#endif