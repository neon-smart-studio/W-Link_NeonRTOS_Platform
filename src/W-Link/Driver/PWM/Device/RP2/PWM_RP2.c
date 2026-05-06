
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "PWM/PWM.h"

#ifdef DEVICE_RP2

#include "GPIO/Device/RP2/GPIO_RP2.h"

#include "PWM/Pin/RP2/PWM_Pin_RP2.h"

#include "PWM_RP2_Base.h"

#include "PWM_RP2.h"

#define PWM_PERIOD_US    1000
#define PWM_RP2_DEFAULT_FREQ_HZ 1000

bool PWM_Channel_Init_Status[hwPWM_Channel_MAX] = {false};

static bool PWM_Channel_OnOff_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_Inverse_Status[hwPWM_Channel_MAX] = {false};
static uint16_t PWM_Channel_Current_Duty[hwPWM_Channel_MAX] = {0};
static bool PWM_Slice_Init_Status[hwPWM_Channel_MAX/2];

static const PWM_Pin_Def* PWM_Find_PinDef(hwPWM_Channel channel_index)
{
    for (int i = 0; i < hwPWM_Base_MAX; i++) {
        PWM_Pinset_t pinset = PWM_Index_Map_Alt[i];

        for (int j = 0; j < 2; j++) {
            const PWM_Pin_Def *def = &PWM_Pin_Def_Table[i][pinset][j];

            if (def->channel == channel_index) {
                return def;
            }
        }
    }

    return NULL;
}

hwPWM_OpResult PWM_Channel_Init(hwPWM_Channel channel_index, bool inverse)
{
        if (channel_index >= hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }

        if (PWM_Channel_Init_Status[channel_index])
        {
                return hwPWM_OK;
        }

        const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
        if (!def || def->pin == hwGPIO_Pin_NC)
        {
                return hwPWM_InvalidParameter;
        }

        gpio_set_function(def->pin, GPIO_FUNC_PWM);

        uint slice = pwm_gpio_to_slice_num(def->pin);
        uint ch    = pwm_gpio_to_channel(def->pin);

        if (!PWM_Slice_Init_Status[slice]) {
                pwm_config cfg = pwm_get_default_config();

                uint32_t sys_clk = clock_get_hz(clk_sys);
                float clkdiv = (float)sys_clk / ((float)PWM_RP2_DEFAULT_FREQ_HZ * (float)PWM_MAX_DUTY);

                if (clkdiv < 1.0f) {
                        clkdiv = 1.0f;
                }

                pwm_config_set_clkdiv(&cfg, clkdiv);
                pwm_config_set_wrap(&cfg, PWM_MAX_DUTY);
                
                pwm_init(slice, &cfg, false);
                
                PWM_Slice_Init_Status[slice] = true;
        }

        uint16_t level = 0;

        if (inverse) {
                level = PWM_MAX_DUTY;
        }
        else
        {
                level = 0;
        }

        pwm_set_chan_level(slice, ch, level);
        pwm_set_enabled(slice, false);

        PWM_Channel_Init_Status[channel_index]    = true;
        PWM_Channel_Inverse_Status[channel_index] = inverse;
        PWM_Channel_Current_Duty[channel_index]   = 0;

        gpio_pin_init_status[def->pin] = true;

        return hwPWM_OK;
}

hwPWM_OpResult PWM_Channel_DeInit(hwPWM_Channel channel_index)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }
        
        if(PWM_Channel_Init_Status[channel_index]==false)
        {
                return hwPWM_OK;
        }
        
        const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
        if (!def || def->pin == hwGPIO_Pin_NC)
        {
                return hwPWM_InvalidParameter;
        }

        uint slice = pwm_gpio_to_slice_num(def->pin);

        bool all_off = true;

        for (int i = 0; i < hwPWM_Channel_MAX; i++)
        {
                if (!PWM_Channel_Init_Status[i]) {
                        continue;
                }

                const PWM_Pin_Def *other = PWM_Find_PinDef(i);

                if (!other || other->pin == hwGPIO_Pin_NC) {
                        continue;
                }

                if (pwm_gpio_to_slice_num(other->pin) == slice)
                {
                        all_off = false;
                        break;
                }
        }

        if (all_off)
        {
                pwm_set_enabled(slice, false);
                PWM_Slice_Init_Status[slice] = false;
        }
        
        gpio_deinit(def->pin);

        PWM_Channel_Init_Status[channel_index] = false;
        PWM_Channel_OnOff_Status[channel_index] = false;
        PWM_Channel_Inverse_Status[channel_index] = false;
        PWM_Channel_Current_Duty[channel_index] = 0;

        gpio_pin_init_status[def->pin] = false;

        return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On(hwPWM_Channel channel_index)
{
        if (channel_index >= hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }

        /* 必須先 Init */
        if (PWM_Channel_Init_Status[channel_index] == false)
        {
                return hwPWM_NotInit;
        }

        const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
        if (!def || def->pin == hwGPIO_Pin_NC)
        {
                return hwPWM_InvalidParameter;
        }

        uint slice = pwm_gpio_to_slice_num(def->pin);
        uint ch    = pwm_gpio_to_channel(def->pin);

        uint16_t duty = PWM_Channel_Current_Duty[channel_index];
        
        uint16_t level = 0;

        if (PWM_Channel_Inverse_Status[channel_index]) {
                level = PWM_MAX_DUTY - duty;
        }
        else
        {
                level = duty;
        }

        pwm_set_chan_level(slice, ch, level);
        pwm_set_enabled(slice, true);

        PWM_Channel_OnOff_Status[channel_index] = true;

        return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On_And_Set_Duty(hwPWM_Channel channel_index, uint16_t duty)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }
        
        if(duty>PWM_MAX_DUTY)
        {
                return hwPWM_InvalidParameter;
        }

        if(PWM_Channel_Init_Status[channel_index]==false)
        {
                return hwPWM_OK;
        }
        
        const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
        if (!def || def->pin == hwGPIO_Pin_NC)
        {
                return hwPWM_InvalidParameter;
        }

        uint slice = pwm_gpio_to_slice_num(def->pin);
        uint ch    = pwm_gpio_to_channel(def->pin);

        uint16_t level = 0;

        if (PWM_Channel_Inverse_Status[channel_index]) {
                level = PWM_MAX_DUTY - duty;
        }
        else
        {
                level = duty;
        }

        pwm_set_chan_level(slice, ch, level);
        pwm_set_enabled(slice, true);

        PWM_Channel_OnOff_Status[channel_index] = true;
        PWM_Channel_Current_Duty[channel_index] = duty;
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_Off(hwPWM_Channel channel_index)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }
        
        if(PWM_Channel_Init_Status[channel_index]==false)
        {
                return hwPWM_OK;
        }
        
        const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
        if (!def || def->pin == hwGPIO_Pin_NC)
        {
                return hwPWM_InvalidParameter;
        }

        uint slice = pwm_gpio_to_slice_num(def->pin);
        uint ch    = pwm_gpio_to_channel(def->pin);

        uint16_t level = PWM_Channel_Inverse_Status[channel_index] ? PWM_MAX_DUTY : 0;

        pwm_set_chan_level(slice, ch, level);

        PWM_Channel_OnOff_Status[channel_index] = false;
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Set_Duty(hwPWM_Channel channel_index, uint16_t duty)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }

        if(duty>PWM_MAX_DUTY)
        {
                return hwPWM_InvalidParameter;
        }
        
        if(PWM_Channel_Init_Status[channel_index]==false)
        {
                return hwPWM_OK;
        }
        
        if(PWM_Channel_OnOff_Status[channel_index]==false){return hwPWM_NotTurnOn;}
        
        const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
        if (!def || def->pin == hwGPIO_Pin_NC)
        {
                return hwPWM_InvalidParameter;
        }

        uint slice = pwm_gpio_to_slice_num(def->pin);
        uint ch    = pwm_gpio_to_channel(def->pin);

        uint16_t level = 0;

        if (PWM_Channel_Inverse_Status[channel_index]) {
                level = PWM_MAX_DUTY - duty;
        }
        else
        {
                level = duty;
        }

        pwm_set_chan_level(slice, ch, level);

        PWM_Channel_Current_Duty[channel_index] = duty;
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Step_Duty(hwPWM_Channel channel_index, uint16_t step_duty, hwPWM_Step_Direction direction)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }
        if(direction>=hwPWM_Step_Dir_MAX){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false)
        {
                return hwPWM_OK;
        }
        
        if(PWM_Channel_OnOff_Status[channel_index]==false){return hwPWM_NotTurnOn;}
        
        uint16_t current_duty = PWM_Channel_Current_Duty[channel_index];

        switch (direction) {
                case hwPWM_Step_Dir_Up:
                if ((PWM_MAX_DUTY - current_duty) < step_duty) {
                        current_duty = PWM_MAX_DUTY;
                } else {
                        current_duty += step_duty;
                }
                break;

                case hwPWM_Step_Dir_Down:
                if ((current_duty - PWM_MIN_DUTY) < step_duty) {
                        current_duty = PWM_MIN_DUTY;
                } else {
                        current_duty -= step_duty;
                }
                break;

                default:
                return hwPWM_InvalidParameter;
        }

        return PWM_Set_Duty(channel_index, current_duty);
}

hwPWM_OpResult PWM_Get_Channel_OnOff_Status(hwPWM_Channel channel_index, bool* onoff_status)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }
        
        if(onoff_status==NULL){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false)
        {
                return hwPWM_OK;
        }
        
        *onoff_status = PWM_Channel_OnOff_Status[channel_index];
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_Current_Duty(hwPWM_Channel channel_index, uint16_t* current_duty)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }
        
        if(current_duty==NULL){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false)
        {
                return hwPWM_OK;
        }
        
        *current_duty = PWM_Channel_Current_Duty[channel_index];
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_Current_OnOff_Duty(hwPWM_Channel channel_index, bool* onoff_status, uint16_t* current_duty)
{
        if(channel_index>=hwPWM_Channel_MAX)
        {
                return hwPWM_InvalidParameter;
        }
        
        if(onoff_status==NULL || current_duty==NULL){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false)
        {
                return hwPWM_OK;
        }
        
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

#endif // DEVICE_RP2
