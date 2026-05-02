
#ifndef TIMER_STM32_H
#define TIMER_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "Timer/Timer.h"

#define TIMER_IRQ_NVIC_PRIORITY        5
#define TIMER_IRQ_NVIC_SUB_PRIORITY    0

#ifdef	__cplusplus
extern "C" {
#endif

extern TIM_HandleTypeDef g_timer[hwTimer_Index_MAX];
extern bool Timer_Init_Status[hwTimer_Index_MAX];

TIM_TypeDef *Timer_Map_Soc_Base(hwTimer_Index index);

extern TIM_HandleTypeDef g_timer[hwTimer_Index_MAX];

hwTimer_OpResult Timer_Instance_Init(hwTimer_Index index);
hwTimer_OpResult Timer_Instance_DeInit(hwTimer_Index index);

void Timer_NVIC_Enable(hwTimer_Index index);
void Timer_NVIC_Disable(hwTimer_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //TIMER_STM32_H