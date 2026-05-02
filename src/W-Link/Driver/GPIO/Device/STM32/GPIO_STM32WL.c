#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#ifdef STM32WL

#include "GPIO_STM32.h"

typedef enum GPIO_EXTI_IRQn{
  GPIO_EXTI_IRQn_0,
  GPIO_EXTI_IRQn_1,
  GPIO_EXTI_IRQn_2,
  GPIO_EXTI_IRQn_3,
  GPIO_EXTI_IRQn_4,
  GPIO_EXTI_IRQn_5_9,
  GPIO_EXTI_IRQn_10_15,
  GPIO_EXTI_IRQn_MAX,
}GPIO_EXTI_IRQn;

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

void EXTI0_IRQHandler(void) { GPIO_EXTI_Dispatch(GPIO_EXTI_Line_0); }
void EXTI1_IRQHandler(void) { GPIO_EXTI_Dispatch(GPIO_EXTI_Line_1); }
void EXTI2_IRQHandler(void) { GPIO_EXTI_Dispatch(GPIO_EXTI_Line_2); }
void EXTI3_IRQHandler(void) { GPIO_EXTI_Dispatch(GPIO_EXTI_Line_3); }
void EXTI4_IRQHandler(void) { GPIO_EXTI_Dispatch(GPIO_EXTI_Line_4); }
void EXTI9_5_IRQHandler(void)
{
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_5);
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_6);
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_7);
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_8);
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_9);
}
void EXTI15_10_IRQHandler(void)
{
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_10);
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_11);
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_12);
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_13);
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_14);
  GPIO_EXTI_Dispatch(GPIO_EXTI_Line_15);
}

void GPIO_EXTI_NVIC_Init(GPIO_EXTI_Line exti_line_idx)
{
    if(exti_line_idx==GPIO_EXTI_Line_0)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_0]==false)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_0] = true;
        HAL_NVIC_SetPriority(EXTI0_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
        HAL_NVIC_EnableIRQ(EXTI0_IRQn);
      }
    }

    if(exti_line_idx==GPIO_EXTI_Line_1)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_1]==false)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_1] = true;
        HAL_NVIC_SetPriority(EXTI1_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
        HAL_NVIC_EnableIRQ(EXTI1_IRQn);
      }
    }

    if(exti_line_idx==GPIO_EXTI_Line_2)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_2]==false)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_2] = true;
        HAL_NVIC_SetPriority(EXTI2_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
        HAL_NVIC_EnableIRQ(EXTI2_IRQn);
      }
    }

    if(exti_line_idx==GPIO_EXTI_Line_3)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_3]==false)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_3] = true;
        HAL_NVIC_SetPriority(EXTI3_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
        HAL_NVIC_EnableIRQ(EXTI3_IRQn);
      }
    }

    if(exti_line_idx==GPIO_EXTI_Line_4)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_4]==false)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_4] = true;
        HAL_NVIC_SetPriority(EXTI4_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
        HAL_NVIC_EnableIRQ(EXTI4_IRQn);
      }
    }

    if(exti_line_idx>=GPIO_EXTI_Line_5 && exti_line_idx<=GPIO_EXTI_Line_9)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_5_9]==false)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_5_9] = true;
        HAL_NVIC_SetPriority(EXTI9_5_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
      }
    }

    if(exti_line_idx>=GPIO_EXTI_Line_10 && exti_line_idx<=GPIO_EXTI_Line_15)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_10_15]==false)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_10_15] = true;
        HAL_NVIC_SetPriority(EXTI15_10_IRQn, GPIO_EXTI_NVIC_PRIORITY, GPIO_EXTI_NVIC_SUB_PRIORITY);
        HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
      }
    }
}

void GPIO_EXTI_NVIC_DeInit(GPIO_EXTI_Line exti_line_idx, GPIO_EXTI_Desc *gpio_exti_desc)
{
    if(exti_line_idx==GPIO_EXTI_Line_0)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_0]==true)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_0] = false;
        HAL_NVIC_DisableIRQ(EXTI0_IRQn);
      }
    }

    if(exti_line_idx==GPIO_EXTI_Line_1)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_1]==true)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_1] = false;
        HAL_NVIC_DisableIRQ(EXTI1_IRQn);
      }
    }

    if(exti_line_idx==GPIO_EXTI_Line_2)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_2]==true)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_2] = false;
        HAL_NVIC_DisableIRQ(EXTI2_IRQn);
      }
    }

    if(exti_line_idx==GPIO_EXTI_Line_3)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_3]==true)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_3] = false;
        HAL_NVIC_DisableIRQ(EXTI3_IRQn);
      }
    }

    if(exti_line_idx==GPIO_EXTI_Line_4)
    {
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_4]==true)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_4] = false;
        HAL_NVIC_DisableIRQ(EXTI4_IRQn);
      }
    }

    if(exti_line_idx>=GPIO_EXTI_Line_5 && exti_line_idx<=GPIO_EXTI_Line_9)
    {
      bool irq_5_9_used = false;
      for(GPIO_EXTI_Line idx = GPIO_EXTI_Line_5; idx<=GPIO_EXTI_Line_9; idx++)
      {
        if(gpio_exti_desc[idx].irq_pin!=hwGPIO_Int_Pin_NC)
        {
          irq_5_9_used = true;
        }
      }
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_5_9]==true && irq_5_9_used==false)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_5_9] = false;
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
      }
    }

    if(exti_line_idx>=GPIO_EXTI_Line_10 && exti_line_idx<=GPIO_EXTI_Line_15)
    {
      bool irq_10_15_used = false;
      for(GPIO_EXTI_Line idx = GPIO_EXTI_Line_10; idx<=GPIO_EXTI_Line_15; idx++)
      {
        if(gpio_exti_desc[idx].irq_pin!=hwGPIO_Int_Pin_NC)
        {
          irq_10_15_used = true;
        }
      }
      if(gpio_exti_enable_status[GPIO_EXTI_IRQn_10_15]==true && irq_10_15_used==false)
      {
        gpio_exti_enable_status[GPIO_EXTI_IRQn_10_15] = false;
        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
      }
    }
}

#endif // STM32WL