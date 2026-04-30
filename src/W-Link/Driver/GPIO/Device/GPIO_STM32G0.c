#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#ifdef STM32G0

#include "GPIO/Device/GPIO_STM32.h"

typedef enum GPIO_EXTI_IRQn{
  GPIO_EXTI_IRQn_0_1,
  GPIO_EXTI_IRQn_2_3,
  GPIO_EXTI_IRQn_4_15,
  GPIO_EXTI_IRQn_MAX,
} GPIO_EXTI_IRQn;

static bool gpio_exti_enable_status[GPIO_EXTI_IRQn_MAX] = {false};

void GPIO_Enable_RCC_Clock(GPIO_TypeDef *base)
{
#if defined(GPIOA)
  if (base == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); }
#endif
#if defined(GPIOB)
  else if (base == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); }
#endif
#if defined(GPIOC)
  else if (base == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); }
#endif
#if defined(GPIOD)
  else if (base == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); }
#endif
#if defined(GPIOE)
  else if (base == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); }
#endif
#if defined(GPIOF)
  else if (base == GPIOF) { __HAL_RCC_GPIOF_CLK_ENABLE(); }
#endif
#if defined(GPIOG)
  else if (base == GPIOG) { __HAL_RCC_GPIOG_CLK_ENABLE(); }
#endif
#if defined(GPIOH)
  else if (base == GPIOH) { __HAL_RCC_GPIOH_CLK_ENABLE(); }
#endif
#if defined(GPIOI)
  else if (base == GPIOI) { __HAL_RCC_GPIOI_CLK_ENABLE(); }
#endif
}

void GPIO_Disable_RCC_Clock(GPIO_TypeDef *base)
{
#if defined(GPIOA)
  if (base == GPIOA) { __HAL_RCC_GPIOA_CLK_DISABLE(); }
#endif
#if defined(GPIOB)
  else if (base == GPIOB) { __HAL_RCC_GPIOB_CLK_DISABLE(); }
#endif
#if defined(GPIOC)
  else if (base == GPIOC) { __HAL_RCC_GPIOC_CLK_DISABLE(); }
#endif
#if defined(GPIOD)
  else if (base == GPIOD) { __HAL_RCC_GPIOD_CLK_DISABLE(); }
#endif
#if defined(GPIOE)
  else if (base == GPIOE) { __HAL_RCC_GPIOE_CLK_DISABLE(); }
#endif
#if defined(GPIOF)
  else if (base == GPIOF) { __HAL_RCC_GPIOF_CLK_DISABLE(); }
#endif
#if defined(GPIOG)
  else if (base == GPIOG) { __HAL_RCC_GPIOG_CLK_DISABLE(); }
#endif
#if defined(GPIOH)
  else if (base == GPIOH) { __HAL_RCC_GPIOH_CLK_DISABLE(); }
#endif
#if defined(GPIOI)
  else if (base == GPIOI) { __HAL_RCC_GPIOI_CLK_DISABLE(); }
#endif
}

void EXTI0_1_IRQHandler(void)
{
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_0);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_1);
}

void EXTI2_3_IRQHandler(void)
{
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_2);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_3);
}

void EXTI4_15_IRQHandler(void)
{
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_4);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_5);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_6);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_7);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_8);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_9);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_10);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_11);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_12);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_13);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_14);
    GPIO_EXTI_Dispatch(GPIO_EXTI_Line_15);
}

void GPIO_EXTI_NVIC_Init(GPIO_EXTI_Line exti_line_idx)
{
    if (exti_line_idx == GPIO_EXTI_Line_0 || exti_line_idx == GPIO_EXTI_Line_1)
    {
        if (!gpio_exti_enable_status[GPIO_EXTI_IRQn_0_1])
        {
            gpio_exti_enable_status[GPIO_EXTI_IRQn_0_1] = true;
            HAL_NVIC_SetPriority(EXTI0_1_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
        }
    }

    if (exti_line_idx == GPIO_EXTI_Line_2 || exti_line_idx == GPIO_EXTI_Line_3)
    {
        if (!gpio_exti_enable_status[GPIO_EXTI_IRQn_2_3])
        {
            gpio_exti_enable_status[GPIO_EXTI_IRQn_2_3] = true;
            HAL_NVIC_SetPriority(EXTI2_3_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
        }
    }

    if (exti_line_idx >= GPIO_EXTI_Line_4 && exti_line_idx <= GPIO_EXTI_Line_15)
    {
        if (!gpio_exti_enable_status[GPIO_EXTI_IRQn_4_15])
        {
            gpio_exti_enable_status[GPIO_EXTI_IRQn_4_15] = true;
            HAL_NVIC_SetPriority(EXTI4_15_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
        }
    }
}

void GPIO_EXTI_NVIC_DeInit(GPIO_EXTI_Line exti_line_idx, GPIO_EXTI_Desc *gpio_exti_desc)
{
    if (exti_line_idx == GPIO_EXTI_Line_0 || exti_line_idx == GPIO_EXTI_Line_1)
    {
        bool irq_0_1_used = false;
        for (GPIO_EXTI_Line idx = GPIO_EXTI_Line_0; idx <= GPIO_EXTI_Line_1; idx++)
        {
            if (gpio_exti_desc[idx].irq_pin != hwGPIO_Int_Pin_NC)
            {
                irq_0_1_used = true;
                break;
            }
        }

        if (gpio_exti_enable_status[GPIO_EXTI_IRQn_0_1] && !irq_0_1_used)
        {
            gpio_exti_enable_status[GPIO_EXTI_IRQn_0_1] = false;
            HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
        }
    }

    if (exti_line_idx == GPIO_EXTI_Line_2 || exti_line_idx == GPIO_EXTI_Line_3)
    {
        bool irq_2_3_used = false;
        for (GPIO_EXTI_Line idx = GPIO_EXTI_Line_2; idx <= GPIO_EXTI_Line_3; idx++)
        {
            if (gpio_exti_desc[idx].irq_pin != hwGPIO_Int_Pin_NC)
            {
                irq_2_3_used = true;
                break;
            }
        }

        if (gpio_exti_enable_status[GPIO_EXTI_IRQn_2_3] && !irq_2_3_used)
        {
            gpio_exti_enable_status[GPIO_EXTI_IRQn_2_3] = false;
            HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
        }
    }

    if (exti_line_idx >= GPIO_EXTI_Line_4 && exti_line_idx <= GPIO_EXTI_Line_15)
    {
        bool irq_4_15_used = false;
        for (GPIO_EXTI_Line idx = GPIO_EXTI_Line_4; idx <= GPIO_EXTI_Line_15; idx++)
        {
            if (gpio_exti_desc[idx].irq_pin != hwGPIO_Int_Pin_NC)
            {
                irq_4_15_used = true;
                break;
            }
        }

        if (gpio_exti_enable_status[GPIO_EXTI_IRQn_4_15] && !irq_4_15_used)
        {
            gpio_exti_enable_status[GPIO_EXTI_IRQn_4_15] = false;
            HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
        }
    }
}

#endif // STM32G0