#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "Timer/Timer.h"

#ifdef DEVICE_NUVOTON

#include "Timer_Nuvoton.h"

bool Timer_Init_Status[hwTimer_Index_MAX] = {false};
static bool Timer_IsPeriodic[hwTimer_Index_MAX] = {false};
static onTimerEventHandler Timer_Expired_Handler[hwTimer_Index_MAX] = {NULL};

void Timer_PeriodElapsedCallback(hwTimer_Index index)
{
    if (Timer_Expired_Handler[index] != NULL) {
        Timer_Expired_Handler[index](index);
    }

    if (!Timer_IsPeriodic[index]) {
        Timer_Instance_Stop(index);
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

    return Timer_Instance_Start(index, duration_us);
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

    return Timer_Instance_Start(index, duration_us);
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

    hwTimer_OpResult op_status;

    op_status = Timer_Instance_Stop(index);
    if(op_status < hwTimer_OK)
    {
        return op_status;
    }

    op_status = Timer_Instance_Start(index, duration_us);
    if(op_status < hwTimer_OK)
    {
        return op_status;
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

    return Timer_Instance_Stop(index);
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

    return Timer_Instance_Read_Ticks(index, ticks);
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

#endif // DEVICE_NUVOTON