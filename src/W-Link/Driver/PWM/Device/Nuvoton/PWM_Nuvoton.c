#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "Timer/Timer.h"
#include "PWM/PWM.h"

#ifdef NUC472

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "PWM/Pin/Nuvoton/PWM_Pin_Nuvoton.h"

#include "PWM_Nuvoton_Base.h"

#include "PWM_Nuvoton.h"

#define PWM_DEFAULT_FREQ_HZ 1000UL

bool PWM_Base_Init_Status[hwPWM_Base_MAX] = {false};
bool PWM_Channel_Init_Status[hwPWM_Channel_MAX] = {false};

static bool PWM_Channel_OnOff_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_Inverse_Status[hwPWM_Channel_MAX] = {false};
static uint16_t PWM_Channel_Current_Duty[hwPWM_Channel_MAX] = {0};

static const PWM_Pin_Def *PWM_Find_PinDef(hwPWM_Channel channel_index)
{
    for(int i = 0; i < hwPWM_Base_MAX; i++)
    {
        PWM_Pinset_t pinset = PWM_Index_Map_Alt[i];

        for(int j = 0; j < 6; j++)
        {
            const PWM_Pin_Def *def = &PWM_Pin_Def_Table[i][pinset][j];

            if(def->channel == channel_index)
                return def;
        }
    }

    return NULL;
}

hwPWM_OpResult PWM_Channel_Init(hwPWM_Channel channel_index, bool inverse)
{
    if(channel_index >= hwPWM_Channel_MAX)
        return hwPWM_InvalidParameter;

    if(PWM_Channel_Init_Status[channel_index])
        return hwPWM_OK;

    const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);

    if(!def || def->pin == hwGPIO_Pin_NC)
        return hwPWM_InvalidParameter;

    hwPWM_Base_Index base_index = def->base;

    if(base_index >= hwPWM_Base_MAX)
        return hwPWM_InvalidParameter;

    GPIO_T *pwm_soc_base = GPIO_Map_Soc_Base(def->pin);
    uint16_t pwm_soc_pin = GPIO_Map_Soc_Pin(def->pin);

    if(pwm_soc_base == NULL || pwm_soc_pin == 0)
        return hwPWM_InvalidParameter;

    PWM_GPIO_ConfigAF(def);

    PWM_Clock_Enable(base_index, channel_index);

    PWM_Channel_Inverse_Status[channel_index] = inverse;
    PWM_Channel_Current_Duty[channel_index] = 0;
    PWM_Channel_OnOff_Status[channel_index] = false;
    PWM_Channel_Init_Status[channel_index] = true;
    PWM_Base_Init_Status[base_index] = true;

    gpio_pin_init_status[def->pin] = true;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Channel_DeInit(hwPWM_Channel channel_index)
{
    if(channel_index >= hwPWM_Channel_MAX)
        return hwPWM_InvalidParameter;

    if(!PWM_Channel_Init_Status[channel_index])
        return hwPWM_OK;

    const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);

    if(!def || def->pin == hwGPIO_Pin_NC)
        return hwPWM_InvalidParameter;

    hwPWM_Base_Index base_index = def->base;

    if(base_index >= hwPWM_Base_MAX)
        return hwPWM_InvalidParameter;

    PWM_T *pwm = PWM_Map_Soc_Base(base_index);

    if(pwm == NULL)
        return hwPWM_InvalidParameter;

    uint32_t ch_mask = PWM_Channel_To_Mask(channel_index);

    PWM_DisableOutput(pwm, ch_mask);
    PWM_ForceStop(pwm, ch_mask);

    PWM_GPIO_DeConfigAF(def);

    PWM_Channel_Init_Status[channel_index] = false;
    PWM_Channel_OnOff_Status[channel_index] = false;
    PWM_Channel_Inverse_Status[channel_index] = false;
    PWM_Channel_Current_Duty[channel_index] = 0;

    gpio_pin_init_status[def->pin] = false;

    PWM_Clock_Disable(base_index, channel_index);

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On(hwPWM_Channel channel_index)
{
    if(channel_index >= hwPWM_Channel_MAX)
        return hwPWM_InvalidParameter;

    if(!PWM_Channel_Init_Status[channel_index])
        return hwPWM_NotInit;

    const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);

    if(!def || def->pin == hwGPIO_Pin_NC)
        return hwPWM_InvalidParameter;

    PWM_T *pwm = PWM_Map_Soc_Base(def->base);

    if(pwm == NULL)
        return hwPWM_InvalidParameter;

    uint32_t ch_mask = PWM_Channel_To_Mask(channel_index);

    if(PWM_Channel_Inverse_Status[channel_index])
    {
        PWM_ConfigOutputChannel(pwm, def->channel, PWM_DEFAULT_FREQ_HZ, PWM_MAX_DUTY - PWM_Channel_Current_Duty[channel_index]);
    }
    else
    {
        PWM_ConfigOutputChannel(pwm, def->channel, PWM_DEFAULT_FREQ_HZ, PWM_Channel_Current_Duty[channel_index]);
    }

    PWM_EnableOutput(pwm, ch_mask);

    PWM_Start(pwm, ch_mask);

    PWM_Channel_OnOff_Status[channel_index] = true;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On_And_Set_Duty(hwPWM_Channel channel_index, uint16_t duty)
{
    if(channel_index >= hwPWM_Channel_MAX)
        return hwPWM_InvalidParameter;

    if(duty > PWM_MAX_DUTY)
        return hwPWM_InvalidParameter;

    if(!PWM_Channel_Init_Status[channel_index])
        return hwPWM_NotInit;

    const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);

    if(!def || def->pin == hwGPIO_Pin_NC)
        return hwPWM_InvalidParameter;

    PWM_T *pwm = PWM_Map_Soc_Base(def->base);
    
    if(pwm == NULL)
        return hwPWM_InvalidParameter;

    uint32_t ch_mask = PWM_Channel_To_Mask(channel_index);

    if(PWM_Channel_Inverse_Status[channel_index])
    {
        PWM_ConfigOutputChannel(pwm, def->channel, PWM_DEFAULT_FREQ_HZ, PWM_MAX_DUTY - duty);
    }
    else
    {
        PWM_ConfigOutputChannel(pwm, def->channel, PWM_DEFAULT_FREQ_HZ, duty);
    }

    PWM_EnableOutput(pwm, ch_mask);

    PWM_Start(pwm, ch_mask);

    PWM_Channel_OnOff_Status[channel_index] = true;
    PWM_Channel_Current_Duty[channel_index] = duty;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_Off(hwPWM_Channel channel_index)
{
    if(channel_index >= hwPWM_Channel_MAX)
        return hwPWM_InvalidParameter;

    if(!PWM_Channel_Init_Status[channel_index])
        return hwPWM_OK;

    const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);

    if(!def || def->pin == hwGPIO_Pin_NC)
        return hwPWM_InvalidParameter;

    PWM_T *pwm = PWM_Map_Soc_Base(def->base);
    
    if(pwm == NULL)
        return hwPWM_InvalidParameter;

    uint32_t ch_mask = PWM_Channel_To_Mask(channel_index);

    PWM_DisableOutput(pwm, ch_mask);
    PWM_ForceStop(pwm, ch_mask);

    PWM_Channel_OnOff_Status[channel_index] = false;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Set_Duty(hwPWM_Channel channel_index, uint16_t duty)
{
    if(channel_index >= hwPWM_Channel_MAX)
        return hwPWM_InvalidParameter;

    if(duty > PWM_MAX_DUTY)
        return hwPWM_InvalidParameter;

    if(!PWM_Channel_Init_Status[channel_index])
        return hwPWM_NotInit;

    if(!PWM_Channel_OnOff_Status[channel_index])
        return hwPWM_NotTurnOn;

    const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);

    if(!def || def->pin == hwGPIO_Pin_NC)
        return hwPWM_InvalidParameter;

    PWM_T *pwm = PWM_Map_Soc_Base(def->base);
    
    if(pwm == NULL)
        return hwPWM_InvalidParameter;

    if(PWM_Channel_Inverse_Status[channel_index])
    {
        PWM_ConfigOutputChannel(pwm, def->channel, PWM_DEFAULT_FREQ_HZ, PWM_MAX_DUTY - duty);
    }
    else
    {
        PWM_ConfigOutputChannel(pwm, def->channel, PWM_DEFAULT_FREQ_HZ, duty);
    }

    PWM_Channel_Current_Duty[channel_index] = duty;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Step_Duty(hwPWM_Channel channel_index,
                             uint16_t step_duty,
                             hwPWM_Step_Direction direction)
{
    if(channel_index >= hwPWM_Channel_MAX)
        return hwPWM_InvalidParameter;

    if(direction >= hwPWM_Step_Dir_MAX)
        return hwPWM_InvalidParameter;

    if(!PWM_Channel_Init_Status[channel_index])
        return hwPWM_NotInit;

    if(!PWM_Channel_OnOff_Status[channel_index])
        return hwPWM_NotTurnOn;

    const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);

    if(!def || def->pin == hwGPIO_Pin_NC)
        return hwPWM_InvalidParameter;

    PWM_T *pwm = PWM_Map_Soc_Base(def->base);
    
    if(pwm == NULL)
        return hwPWM_InvalidParameter;

    uint16_t current_duty = PWM_Channel_Current_Duty[channel_index];

    switch(direction)
    {
        case hwPWM_Step_Dir_Up:
            if((PWM_MAX_DUTY - current_duty) < step_duty)
                current_duty = PWM_MAX_DUTY;
            else
                current_duty += step_duty;
            break;

        case hwPWM_Step_Dir_Down:
            if((current_duty - PWM_MIN_DUTY) < step_duty)
                current_duty = PWM_MIN_DUTY;
            else
                current_duty -= step_duty;
            break;
    }

    if(PWM_Channel_Inverse_Status[channel_index])
    {
        PWM_ConfigOutputChannel(pwm, def->channel, PWM_DEFAULT_FREQ_HZ, PWM_MAX_DUTY - current_duty);
    }
    else
    {
        PWM_ConfigOutputChannel(pwm, def->channel, PWM_DEFAULT_FREQ_HZ, current_duty);
    }

    PWM_Channel_Current_Duty[channel_index] = current_duty;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_OnOff_Status(hwPWM_Channel channel_index, bool *onoff_status)
{
    if(channel_index >= hwPWM_Channel_MAX || onoff_status == NULL)
        return hwPWM_InvalidParameter;

    if(!PWM_Channel_Init_Status[channel_index])
        return hwPWM_OK;

    *onoff_status = PWM_Channel_OnOff_Status[channel_index];

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_Current_Duty(hwPWM_Channel channel_index, uint16_t *current_duty)
{
    if(channel_index >= hwPWM_Channel_MAX || current_duty == NULL)
        return hwPWM_InvalidParameter;

    if(!PWM_Channel_Init_Status[channel_index])
        return hwPWM_OK;

    *current_duty = PWM_Channel_Current_Duty[channel_index];

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_Current_OnOff_Duty(hwPWM_Channel channel_index,
                                                  bool *onoff_status,
                                                  uint16_t *current_duty)
{
    if(channel_index >= hwPWM_Channel_MAX ||
       onoff_status == NULL ||
       current_duty == NULL)
    {
        return hwPWM_InvalidParameter;
    }

    if(!PWM_Channel_Init_Status[channel_index])
        return hwPWM_OK;

    *onoff_status = PWM_Channel_OnOff_Status[channel_index];
    *current_duty = PWM_Channel_Current_Duty[channel_index];

    return hwPWM_OK;
}

bool PWM_is_Init(hwPWM_Channel channel_index)
{
    if(channel_index >= hwPWM_Channel_MAX)
        return false;

    return PWM_Channel_Init_Status[channel_index];
}

#endif /* NUC472 */