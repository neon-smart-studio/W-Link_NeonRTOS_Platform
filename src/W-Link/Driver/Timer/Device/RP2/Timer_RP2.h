
#ifndef TIMER_STM32_H
#define TIMER_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "Timer/Timer.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern bool Timer_Init_Status[hwTimer_Index_MAX];

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //TIMER_STM32_H