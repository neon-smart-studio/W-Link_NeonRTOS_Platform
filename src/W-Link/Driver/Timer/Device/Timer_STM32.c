#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef DEVICE_STM32

static bool Timer_Init_Status[hwTimer_Index_MAX] = {false};
static bool Timer_IsPeriodic[hwTimer_Index_MAX] = {false};
static onTimerEventHandler Timer_Expired_Handler[hwTimer_Index_MAX] = {NULL};

TIM_HandleTypeDef g_timer[hwTimer_Index_MAX];

hwTimer_Index Timer_IndexFromHandle(TIM_HandleTypeDef *htimer)
{
    for (int i = 0; i < hwTimer_Index_MAX; i++) {
        if (&g_timer[i] == htimer) {
            return (hwTimer_Index)i;
        }
    }
    return hwTimer_Index_MAX;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    hwTimer_Index idx = Timer_IndexFromHandle(htim);

    if (idx >= hwTimer_Index_MAX) {
        return;
    }

    if (Timer_Expired_Handler[idx] != NULL) {
        Timer_Expired_Handler[idx](idx);
    }

    if (!Timer_IsPeriodic[idx]) {
        HAL_TIM_Base_Stop_IT(htim);
    }
}

hwTimer_OpResult Timer_Init(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX) {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index]) {
        return hwTimer_OK;
    }

    hwTimer_OpResult result = Timer_Instance_Init(index);
    if (result != hwTimer_OK) {
        return result;
    }

    Timer_NVIC_Enable(index);

    Timer_Init_Status[index] = true;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_DeInit(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index] == false)
    {
        return hwTimer_OK;
    }

    Timer_Init_Status[index] = false;

    Timer_NVIC_Disable(index);

    Timer_Instance_DeInit(index);

    Timer_Expired_Handler[index] = NULL;
    Timer_IsPeriodic[index] = false;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Start_OneShout(hwTimer_Index index, uint32_t duration_us, onTimerEventHandler timer_exp_cb)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index] == false)
    {
        return hwTimer_NotInit;
    }

    if (duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index] = false;

    HAL_TIM_Base_Stop_IT(&g_timer[index]);

    g_timer[index].Init.Period = duration_us - 1U;
    if (HAL_TIM_Base_Init(&g_timer[index]) != HAL_OK)
    {
        return hwTimer_HwError;
    }

    if (HAL_TIM_Base_Start_IT(&g_timer[index]) != HAL_OK)
    {
        return hwTimer_HwError;
    }

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Start_Period(hwTimer_Index index, uint32_t duration_us, onTimerEventHandler timer_exp_cb)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index] == false)
    {
        return hwTimer_NotInit;
    }

    if (duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    Timer_Expired_Handler[index] = timer_exp_cb;
    Timer_IsPeriodic[index] = true;

    HAL_TIM_Base_Stop_IT(&g_timer[index]);

    g_timer[index].Init.Period = duration_us - 1U;
    if (HAL_TIM_Base_Init(&g_timer[index]) != HAL_OK)
    {
        return hwTimer_HwError;
    }

    if (HAL_TIM_Base_Start_IT(&g_timer[index]) != HAL_OK)
    {
        return hwTimer_HwError;
    }

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Reload(hwTimer_Index index, uint32_t duration_us)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index] == false)
    {
        return hwTimer_NotInit;
    }

    if (duration_us == 0)
    {
        return hwTimer_InvalidParameter;
    }

    HAL_TIM_Base_Stop_IT(&g_timer[index]);

    g_timer[index].Init.Period = duration_us - 1U;
    if (HAL_TIM_Base_Init(&g_timer[index]) != HAL_OK)
    {
        return hwTimer_HwError;
    }

    if (HAL_TIM_Base_Start_IT(&g_timer[index]) != HAL_OK)
    {
        return hwTimer_HwError;
    }

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Stop(hwTimer_Index index)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index] == false)
    {
        return hwTimer_NotInit;
    }

    HAL_TIM_Base_Stop_IT(&g_timer[index]);

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_Ticks(hwTimer_Index index, uint32_t* ticks)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (ticks == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index] == false)
    {
        return hwTimer_NotInit;
    }

    *ticks = g_timer[index].Instance->CNT;

    return hwTimer_OK;
}

hwTimer_OpResult Timer_Read_uSec(hwTimer_Index index, uint32_t* uSec)
{
    if (index >= hwTimer_Index_MAX)
    {
        return hwTimer_InvalidParameter;
    }

    if (uSec == NULL)
    {
        return hwTimer_InvalidParameter;
    }

    if (Timer_Init_Status[index] == false)
    {
        return hwTimer_NotInit;
    }

    return Timer_Read_Ticks(index, uSec);
}

bool Timer_is_Init(hwTimer_Index hw_index)
{
    if (hw_index >= hwTimer_Index_MAX)
    {
        return false;
    }

    return Timer_Init_Status[hw_index];
}

#endif // DEVICE_STM32