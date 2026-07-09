
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "PWM/PWM.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSP432P

#include "PWM/Pin/PWM_Pin.h"

#define PWM_HZ           1000

static bool PWM_Channel_Init_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_OnOff_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_Inverse_Status[hwPWM_Channel_MAX] = {false};
static uint16_t PWM_Channel_Current_Duty[hwPWM_Channel_MAX] = {0};

static bool PWM_Base_Init_Status[hwPWM_Base_MAX] = {false};

static uint32_t PWM_Get_Period(void)
{
    return (g_sys_clock_hz / PWM_HZ);
}

static uint32_t PWM_Duty_To_Compare(uint16_t duty)
{
    if(duty > PWM_MAX_DUTY)
    {
        duty = PWM_MAX_DUTY;
    }

    return (PWM_Get_Period() * duty) / PWM_MAX_DUTY;
}

hwPWM_OpResult PWM_Channel_Init(hwPWM_Channel channel_index, bool inverse_PWM)
{
    if(channel_index >= hwPWM_Channel_MAX)
    {
        return hwPWM_InvalidParameter;
    }

    if(PWM_Channel_Init_Status[channel_index] == true)
    {
        return hwPWM_OK;
    }

    hwGPIO_Pin pwm_pin = PWM_Pin_Def_Table[channel_index].pin;

    uint32_t pwm_port = GPIO_Map_Soc_Port_Base(pwm_pin);
    uint32_t pwm_mask = GPIO_Map_Soc_Pin_Mask(pwm_pin);

    if (pwm_port == 0 || pwm_mask == 0)
    {
        return hwPWM_InvalidParameter;
    }

    uint32_t timer_base = PWM_Map_Timer_Base(channel_index);
    uint_fast16_t compare_register = PWM_Map_Timer_Compare_Register(channel_index);
    uint_fast16_t output_mode = inverse_PWM ? TIMER_A_OUTPUTMODE_RESET_SET : TIMER_A_OUTPUTMODE_SET_RESET;

    if(timer_base == 0 || compare_register == 0)
    {
        return hwPWM_InvalidParameter;
    }

    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(pwm_port, pwm_mask, GPIO_PRIMARY_MODULE_FUNCTION);

    Timer_A_PWMConfig pwmConfig =
    {
        .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
        .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
        .timerPeriod = PWM_Get_Period(),
        .compareRegister = compare_register,
        .compareOutputMode = output_mode,
        .dutyCycle = 0
    };

    MAP_Timer_A_generatePWM(timer_base, &pwmConfig);

    PWM_Base_Init_Status[PWM_Pin_Def_Table[channel_index].base] = true;

    PWM_Channel_Init_Status[channel_index] = true;
    PWM_Channel_OnOff_Status[channel_index] = false;
    PWM_Channel_Inverse_Status[channel_index] = inverse_PWM;
    PWM_Channel_Current_Duty[channel_index] = 0;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Channel_DeInit(hwPWM_Channel channel_index)
{
    if(channel_index >= hwPWM_Channel_MAX)
    {
        return hwPWM_InvalidParameter;
    }

    if(PWM_Channel_Init_Status[channel_index] == false)
    {
        return hwPWM_OK;
    }

    hwGPIO_Pin pwm_pin = PWM_Pin_Def_Table[channel_index].pin;

    uint32_t pwm_port = GPIO_Map_Soc_Port_Base(pwm_pin);
    uint32_t pwm_mask = GPIO_Map_Soc_Pin_Mask(pwm_pin);

    if (pwm_port == 0 || pwm_mask == 0)
    {
        return hwPWM_InvalidParameter;
    }

    uint32_t timer_base = PWM_Map_Timer_Base(channel_index);

    if(timer_base == 0)
    {
        return hwPWM_InvalidParameter;
    }

    MAP_Timer_A_stopTimer(timer_base);

    MAP_GPIO_setAsInputPin(pwm_port, pwm_mask);

    PWM_Channel_Init_Status[channel_index] = false;
    PWM_Channel_OnOff_Status[channel_index] = false;
    PWM_Channel_Inverse_Status[channel_index] = false;
    PWM_Channel_Current_Duty[channel_index] = 0;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On(hwPWM_Channel channel_index)
{
    if(channel_index >= hwPWM_Channel_MAX)
    {
        return hwPWM_InvalidParameter;
    }

    if(PWM_Channel_Init_Status[channel_index] == false)
    {
        return hwPWM_NotInit;
    }

    uint32_t timer_base = PWM_Map_Timer_Base(channel_index);
    uint_fast16_t compare_register = PWM_Map_Timer_Compare_Register(channel_index);

    if(timer_base == 0 || compare_register == 0)
    {
        return hwPWM_InvalidParameter;
    }

    MAP_Timer_A_setCompareValue(
        timer_base,
        compare_register,
        PWM_Duty_To_Compare(PWM_Channel_Current_Duty[channel_index])
    );

    PWM_Channel_OnOff_Status[channel_index] = true;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On_And_Set_Duty(hwPWM_Channel channel_index, uint16_t duty)
{
    if(channel_index >= hwPWM_Channel_MAX)
    {
        return hwPWM_InvalidParameter;
    }

    if(duty > PWM_MAX_DUTY)
    {
        return hwPWM_InvalidParameter;
    }

    if(PWM_Channel_Init_Status[channel_index] == false)
    {
        return hwPWM_NotInit;
    }

    uint32_t timer_base = PWM_Map_Timer_Base(channel_index);
    uint_fast16_t compare_register = PWM_Map_Timer_Compare_Register(channel_index);

    if(timer_base == 0 || compare_register == 0)
    {
        return hwPWM_InvalidParameter;
    }

    MAP_Timer_A_setCompareValue(
        timer_base,
        compare_register,
        PWM_Duty_To_Compare(duty)
    );

    PWM_Channel_Current_Duty[channel_index] = duty;
    PWM_Channel_OnOff_Status[channel_index] = true;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_Off(hwPWM_Channel channel_index)
{
    if(channel_index >= hwPWM_Channel_MAX)
    {
        return hwPWM_InvalidParameter;
    }

    if(PWM_Channel_Init_Status[channel_index] == false)
    {
        return hwPWM_NotInit;
    }

    uint32_t timer_base = PWM_Map_Timer_Base(channel_index);
    uint_fast16_t compare_register = PWM_Map_Timer_Compare_Register(channel_index);

    if(timer_base == 0 || compare_register == 0)
    {
        return hwPWM_InvalidParameter;
    }

    MAP_Timer_A_setCompareValue(timer_base, compare_register, 0);

    PWM_Channel_OnOff_Status[channel_index] = false;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Set_Duty(hwPWM_Channel channel_index, uint16_t duty)
{
    if(channel_index >= hwPWM_Channel_MAX)
    {
        return hwPWM_InvalidParameter;
    }

    if(duty > PWM_MAX_DUTY)
    {
        return hwPWM_InvalidParameter;
    }

    if(PWM_Channel_Init_Status[channel_index] == false)
    {
        return hwPWM_NotInit;
    }

    if(PWM_Channel_OnOff_Status[channel_index] == false)
    {
        return hwPWM_NotTurnOn;
    }

    uint32_t timer_base = PWM_Map_Timer_Base(channel_index);
    uint_fast16_t compare_register = PWM_Map_Timer_Compare_Register(channel_index);

    if(timer_base == 0 || compare_register == 0)
    {
        return hwPWM_InvalidParameter;
    }

    MAP_Timer_A_setCompareValue(
        timer_base,
        compare_register,
        PWM_Duty_To_Compare(duty)
    );

    PWM_Channel_Current_Duty[channel_index] = duty;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Step_Duty(hwPWM_Channel channel_index, uint16_t step_duty, hwPWM_Step_Direction direction)
{
    if(channel_index >= hwPWM_Channel_MAX)
    {
        return hwPWM_InvalidParameter;
    }

    if(direction >= hwPWM_Step_Dir_MAX)
    {
        return hwPWM_InvalidParameter;
    }

    if(PWM_Channel_Init_Status[channel_index] == false)
    {
        return hwPWM_NotInit;
    }

    if(PWM_Channel_OnOff_Status[channel_index] == false)
    {
        return hwPWM_NotTurnOn;
    }

    uint32_t timer_base = PWM_Map_Timer_Base(channel_index);
    uint_fast16_t compare_register = PWM_Map_Timer_Compare_Register(channel_index);

    if(timer_base == 0 || compare_register == 0)
    {
        return hwPWM_InvalidParameter;
    }

    uint16_t current_duty = PWM_Channel_Current_Duty[channel_index];

    switch(direction)
    {
        case hwPWM_Step_Dir_Up:
            if((PWM_MAX_DUTY - current_duty) < step_duty)
            {
                current_duty = PWM_MAX_DUTY;
            }
            else
            {
                current_duty += step_duty;
            }
            break;

        case hwPWM_Step_Dir_Down:
            if((current_duty - PWM_MIN_DUTY) < step_duty)
            {
                current_duty = PWM_MIN_DUTY;
            }
            else
            {
                current_duty -= step_duty;
            }
            break;
    }

    MAP_Timer_A_setCompareValue(
        timer_base,
        compare_register,
        PWM_Duty_To_Compare(current_duty)
    );

    PWM_Channel_Current_Duty[channel_index] = current_duty;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_OnOff_Status(hwPWM_Channel channel_index, bool* onoff_status)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        
        if(onoff_status==NULL){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false){return hwPWM_NotInit;}
        
        *onoff_status = PWM_Channel_OnOff_Status[channel_index];
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_Current_Duty(hwPWM_Channel channel_index, uint16_t* current_duty)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        
        if(current_duty==NULL){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false){return hwPWM_NotInit;}
        
        *current_duty = PWM_Channel_Current_Duty[channel_index];
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_Current_OnOff_Duty(hwPWM_Channel channel_index, bool* onoff_status, uint16_t* current_duty)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        
        if(onoff_status==NULL || current_duty==NULL){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false){return hwPWM_NotInit;}
        
        *onoff_status = PWM_Channel_OnOff_Status[channel_index];
        *current_duty = PWM_Channel_Current_Duty[channel_index];
        
        return hwPWM_OK;
}

bool PWM_is_Init(hwPWM_Channel channel_index)
{
    if(channel_index>=hwPWM_Channel_MAX)
    {
      return false;
    }
    
    return PWM_Channel_Init_Status[channel_index];
}

#endif // DEVICE_TIMSP432P