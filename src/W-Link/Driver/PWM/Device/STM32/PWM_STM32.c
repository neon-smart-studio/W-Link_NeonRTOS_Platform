
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Timer/Timer.h"

#include "PWM/PWM.h"

#ifdef DEVICE_STM32

#include "GPIO/Device/STM32/GPIO_STM32.h"

#include "PWM/Pin/STM32/PWM_Pin_STM32.h"

#include "Timer/Device/STM32/Timer_STM32.h"

#include "PWM_STM32_Base.h"

#include "PWM_STM32.h"

#define PWM_PERIOD_US    1000

bool PWM_Channel_Init_Status[hwPWM_Channel_MAX] = {false};

static bool PWM_Channel_OnOff_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_Inverse_Status[hwPWM_Channel_MAX] = {false};
static uint16_t PWM_Channel_Current_Duty[hwPWM_Channel_MAX] = {0};

#ifndef STM32F1
static uint8_t STM32_PWM_GetAF(hwTimer_Index timer, hwGPIO_Pin pin)
{
    for (size_t i = 0; i < sizeof(PWM_Pin_AF_Map)/sizeof(PWM_Pin_AF_Map[0]); i++) {
        if (PWM_Pin_AF_Map[i].timer == timer &&
            PWM_Pin_AF_Map[i].pin == pin) {
            return PWM_Pin_AF_Map[i].af;
        }
    }
    return 0;
}
#endif

const PWM_Pin_Def* PWM_Find_PinDef(hwPWM_Channel channel_index)
{
    for (int i = 0; i < hwPWM_Base_MAX; i++) {
        PWM_Pinset_t pinset = PWM_Index_Map_Alt[i];

        for (int j = 0; j < 4; j++) {
            const PWM_Pin_Def *def = &PWM_Pin_Def_Table[i][pinset][j];

            if (def->channel == channel_index)
                return def;
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

        if(Timer_is_Init(def->timer))
        {
                return hwPWM_HwError;
        }

#ifndef STM32F1
        uint8_t af = STM32_PWM_GetAF(def->timer, def->pin);
        if (af == 0)
        {
                return hwPWM_InvalidParameter;
        }
#endif

        GPIO_TypeDef * pwm_soc_base = GPIO_Map_Soc_Base(def->pin);
        uint16_t pwm_soc_pin = GPIO_Map_Soc_Pin(def->pin);
        if (pwm_soc_base == NULL || pwm_soc_pin == 0)
        {
                return hwPWM_InvalidParameter;
        }

        GPIO_Enable_RCC_Clock(pwm_soc_base);

        PWM_Clock_Enable(def->timer);

        GPIO_InitTypeDef gpio = {0};
        gpio.Mode      = GPIO_MODE_AF_PP;
        gpio.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
        gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
        gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
        gpio.Alternate = af;
#endif
        gpio.Pin = pwm_soc_pin;

        HAL_GPIO_Init(pwm_soc_base, &gpio);

        /* HAL init */
        if (HAL_TIM_PWM_Init(&g_timer[def->timer]) != HAL_OK)
        {
                HAL_GPIO_DeInit(pwm_soc_base, pwm_soc_pin);
                return hwPWM_HwError;
        }

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

        if(Timer_is_Init(def->timer))
        {
                return hwPWM_HwError;
        }

        GPIO_TypeDef * pwm_soc_base = GPIO_Map_Soc_Base(def->pin);
        uint16_t pwm_soc_pin = GPIO_Map_Soc_Pin(def->pin);
        if (pwm_soc_base == NULL || pwm_soc_pin == 0)
        {
                return hwPWM_InvalidParameter;
        }

        HAL_TIM_PWM_Stop(&g_timer[def->timer], def->chx);
	
        if (HAL_TIM_PWM_DeInit(&g_timer[def->timer]) != HAL_OK)
        {
                return hwPWM_HwError;
        }

        HAL_GPIO_DeInit(pwm_soc_base, pwm_soc_pin);

        PWM_Channel_Init_Status[channel_index] = false;
        PWM_Channel_OnOff_Status[channel_index] = false;
        PWM_Channel_Inverse_Status[channel_index] = false;
        
        PWM_Clock_Disable(def->timer);

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

        /* Timer 必須已初始化 */
        if (!Timer_is_Init(def->timer))
        {
                return hwPWM_NotInit;
        }

        /* duty clamp */
        uint16_t duty = PWM_Channel_Current_Duty[channel_index];
        if (duty > PWM_MAX_DUTY)
        {
                duty = PWM_MAX_DUTY;
                PWM_Channel_Current_Duty[channel_index] = duty;
        }

        /* 計算 CCR */
        uint32_t period = g_timer[def->timer].Init.Period;
        uint32_t pulse;

        if (PWM_Channel_Inverse_Status[channel_index])
        {
                pulse = period - ((uint32_t)duty * period) / PWM_MAX_DUTY;
        }
        else
        {
                pulse = ((uint32_t)duty * period) / PWM_MAX_DUTY;
        }

        TIM_OC_InitTypeDef oc = {0};
        oc.OCMode     = TIM_OCMODE_PWM1;
        oc.Pulse      = pulse;
        oc.OCPolarity = TIM_OCPOLARITY_HIGH;
        oc.OCFastMode = TIM_OCFAST_DISABLE;

        if (HAL_TIM_PWM_ConfigChannel(&g_timer[def->timer], &oc, def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

        if (HAL_TIM_PWM_Start(&g_timer[def->timer], def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

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

        /* Timer 必須已初始化 */
        if (!Timer_is_Init(def->timer))
        {
                return hwPWM_NotInit;
        }

        /* 計算 CCR */
        uint32_t period = g_timer[def->timer].Init.Period;
        uint32_t pulse;

        if (PWM_Channel_Inverse_Status[channel_index])
        {
                pulse = period - ((uint32_t)duty * period) / PWM_MAX_DUTY;
        }
        else
        {
                pulse = ((uint32_t)duty * period) / PWM_MAX_DUTY;
        }
	
        TIM_OC_InitTypeDef oc = {0};
        oc.OCMode     = TIM_OCMODE_PWM1;
        oc.Pulse      = pulse;
        oc.OCPolarity = TIM_OCPOLARITY_HIGH;
        oc.OCFastMode = TIM_OCFAST_DISABLE;

        if (HAL_TIM_PWM_ConfigChannel(&g_timer[def->timer], &oc, def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

        if (HAL_TIM_PWM_Start(&g_timer[def->timer], def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

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

        /* Timer 必須已初始化 */
        if (!Timer_is_Init(def->timer))
        {
                return hwPWM_NotInit;
        }

        /* 計算 CCR */
        uint32_t period = g_timer[def->timer].Init.Period;
        uint32_t pulse;

        if (PWM_Channel_Inverse_Status[channel_index])
        {
                pulse = period;
        }
        else
        {
                pulse = 0;
        }
	
        TIM_OC_InitTypeDef oc = {0};
        oc.OCMode     = TIM_OCMODE_PWM1;
        oc.Pulse      = pulse;
        oc.OCPolarity = TIM_OCPOLARITY_HIGH;
        oc.OCFastMode = TIM_OCFAST_DISABLE;

        if (HAL_TIM_PWM_ConfigChannel(&g_timer[def->timer], &oc, def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

        if (HAL_TIM_PWM_Start(&g_timer[def->timer], def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

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

        /* Timer 必須已初始化 */
        if (!Timer_is_Init(def->timer))
        {
                return hwPWM_NotInit;
        }

        /* 計算 CCR */
        uint32_t period = g_timer[def->timer].Init.Period;
        uint32_t pulse;

        if (PWM_Channel_Inverse_Status[channel_index])
        {
                pulse = period - ((uint32_t)duty * period) / PWM_MAX_DUTY;
        }
        else
        {
                pulse = ((uint32_t)duty * period) / PWM_MAX_DUTY;
        }
	
        TIM_OC_InitTypeDef oc = {0};
        oc.OCMode     = TIM_OCMODE_PWM1;
        oc.Pulse      = pulse;
        oc.OCPolarity = TIM_OCPOLARITY_HIGH;
        oc.OCFastMode = TIM_OCFAST_DISABLE;

        if (HAL_TIM_PWM_ConfigChannel(&g_timer[def->timer], &oc, def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

        if (HAL_TIM_PWM_Start(&g_timer[def->timer], def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

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
        
        const PWM_Pin_Def *def = PWM_Find_PinDef(channel_index);
        if (!def || def->pin == hwGPIO_Pin_NC)
        {
                return hwPWM_InvalidParameter;
        }

        /* Timer 必須已初始化 */
        if (!Timer_is_Init(def->timer))
        {
                return hwPWM_NotInit;
        }

        uint16_t current_duty = PWM_Channel_Current_Duty[channel_index];
        
        switch(direction)
        {
        case hwPWM_Step_Dir_Up:
                current_duty += step_duty;
                if(current_duty>PWM_MAX_DUTY){current_duty = PWM_MAX_DUTY;}
                break;
        case hwPWM_Step_Dir_Down:
                if((current_duty-PWM_MIN_DUTY)<step_duty){
                        current_duty = PWM_MIN_DUTY;
                }
                else{
                        current_duty -= step_duty;
                }
                break;
        }
        
        /* 計算 CCR */
        uint32_t period = g_timer[def->timer].Init.Period;
        uint32_t pulse;

        if (PWM_Channel_Inverse_Status[channel_index])
        {
                pulse = period - ((uint32_t)current_duty * period) / PWM_MAX_DUTY;
        }
        else
        {
                pulse = ((uint32_t)current_duty * period) / PWM_MAX_DUTY;
        }
	
        TIM_OC_InitTypeDef oc = {0};
        oc.OCMode     = TIM_OCMODE_PWM1;
        oc.Pulse      = pulse;
        oc.OCPolarity = TIM_OCPOLARITY_HIGH;
        oc.OCFastMode = TIM_OCFAST_DISABLE;

        if (HAL_TIM_PWM_ConfigChannel(&g_timer[def->timer], &oc, def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

        if (HAL_TIM_PWM_Start(&g_timer[def->timer], def->chx) != HAL_OK)
        {
                return hwPWM_HwError;
        }

        PWM_Channel_Current_Duty[channel_index] = current_duty;
        
        return hwPWM_OK;
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

#endif // DEVICE_STM32
