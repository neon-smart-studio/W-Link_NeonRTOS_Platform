
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "PWM/PWM.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TITIVAC

#include "GPIO/Device/TITivaC/GPIO_TITivaC.h"

#include "PWM/Pin/TITivaC/PWM_Pin_TITivaC.h"

#include "PWM_TITivaC_Base.h"

#include "PWM_TITivaC.h"

#define PWM_HZ           1000

static bool PWM_Channel_Init_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_OnOff_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_Inverse_Status[hwPWM_Channel_MAX] = {false};
static uint16_t PWM_Channel_Current_Duty[hwPWM_Channel_MAX] = {0};

static bool PWM_Base_Init_Status[hwPWM_Base_MAX] = {false};

hwPWM_OpResult PWM_Channel_Init(hwPWM_Channel channel_index, bool inverse_PWM)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==true){return hwPWM_OK;}
        
        hwGPIO_Pin pwm_pin = PWM_Pin_Def_Table[channel_index].pin;

        uint32_t pwmPortBase = GPIO_Map_Soc_Port_Base(pwm_pin);
        uint32_t pwmPinMask = GPIO_Map_Soc_Pin_Mask(pwm_pin);

        uint32_t pwmPinCfg = PWM_Map_Pin_Cfg(channel_index, pwm_pin);
        uint32_t pwmGen = PWM_Map_Gen(channel_index);
        uint32_t pwmOut = PWM_Map_Out(channel_index);
        uint32_t pwmOutMask = PWM_Map_Out_Mask(channel_index);
        
        if(pwmPortBase==0 || pwmPinMask==0 || pwmPinCfg==0 || pwmGen==0 || pwmOut==0 || pwmOutMask==0)
        {
                return hwGPIO_InvalidParameter;
        }

        GPIO_Enable_Port_Clock(pwmPortBase);

        MAP_GPIOPinConfigure(pwmPinCfg);
        MAP_GPIOPinTypePWM(pwmPortBase, pwmPinMask);

        if(PWM_Base_Init_Status[hwPWM_Base_0]==false)
        {
                PWM_Base_Init_Status[hwPWM_Base_0] = true;

                MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
                while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) {} 
                
                MAP_PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_8);
        }

        //
        // Configure PWM2 to count up/down without synchronization.
        //
        MAP_PWMGenConfigure(PWM0_BASE, pwmGen, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
        
        //
        // Set the PWM period to 1000Hz.  To calculate the appropriate parameter
        // use the following equation: N = (1 / f) * PWMClk.  Where N is the
        // function parameter, f is the desired frequency, and PWMClk is the
        // PWM clock frequency based on the system clock.
        //
        MAP_PWMGenPeriodSet(PWM0_BASE, pwmGen, (g_sys_clock_hz / 8 / PWM_HZ));

        if(inverse_PWM==true)
        {
                MAP_PWMPulseWidthSet(PWM0_BASE, pwmOut, MAP_PWMGenPeriodGet(PWM0_BASE, pwmGen));
        }
        else
        {
                MAP_PWMPulseWidthSet(PWM0_BASE, pwmOut, 0);
        }

        if(PWM_Channel_Inverse_Status[channel_index]==true)
        {
                MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
        }

        MAP_PWMOutputState(PWM0_BASE, pwmOutMask, true);

        MAP_PWMGenEnable(PWM0_BASE, pwmGen);

        PWM_Channel_Init_Status[channel_index] = true;
        PWM_Channel_OnOff_Status[channel_index] = false;
	PWM_Channel_Inverse_Status[channel_index] = inverse_PWM;
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Channel_DeInit(hwPWM_Channel channel_index)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false){return hwPWM_OK;}
        
        hwGPIO_Pin pwm_pin = PWM_Pin_Def_Table[channel_index].pin;

        uint32_t pwmPortBase = GPIO_Map_Soc_Port_Base(pwm_pin);
        uint32_t pwmPinMask = GPIO_Map_Soc_Pin_Mask(pwm_pin);

        uint32_t pwmGen = PWM_Map_Gen(channel_index);
        
        if(pwmPortBase==0 || pwmPinMask==0 || pwmGen==0)
        {
                return hwGPIO_InvalidParameter;
        }

        MAP_PWMGenDisable(PWM0_BASE, pwmGen);

        bool turnOffSysCtrl = true;
        for(hwPWM_Channel i = 0; i<hwPWM_Channel_MAX; i++)
        {
                if(i==channel_index){continue;}

                if(PWM_Channel_Init_Status[i])
                {
                        turnOffSysCtrl = false;
                }
        }

        if(turnOffSysCtrl)
        {
                PWM_Base_Init_Status[hwPWM_Base_0] = false;

                MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_PWM0);
        }

        MAP_GPIOPinTypeGPIOInput(pwmPortBase, pwmPinMask);

        PWM_Channel_Init_Status[channel_index] = false;
        PWM_Channel_OnOff_Status[channel_index] = false;
	PWM_Channel_Inverse_Status[channel_index] = false;
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On(hwPWM_Channel channel_index)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false){return hwPWM_NotInit;}
        
        uint32_t pwmGen = PWM_Map_Gen(channel_index);
        uint32_t pwmOut = PWM_Map_Out(channel_index);
        uint32_t pwmOutMask = PWM_Map_Out_Mask(channel_index);
        
        if(pwmGen==0 || pwmOut==0 || pwmOutMask==0)
        {
                return hwGPIO_InvalidParameter;
        }

        if(PWM_Channel_Current_Duty[channel_index]>PWM_MAX_DUTY)
        {
                PWM_Channel_Current_Duty[channel_index] = PWM_MAX_DUTY;
        }
 
        float pwm_duty_float = ((float)PWM_Channel_Current_Duty[channel_index]/PWM_MAX_DUTY);

        MAP_PWMPulseWidthSet(PWM0_BASE, pwmOut, MAP_PWMGenPeriodGet(PWM0_BASE, pwmGen)*pwm_duty_float);

        if(PWM_Channel_Current_Duty[channel_index]==PWM_MAX_DUTY)
        {
                if(PWM_Channel_Inverse_Status[channel_index]==true)
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, false);
                }
                else
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
                }
        }
        else
        {
                if(PWM_Channel_Inverse_Status[channel_index]==true)
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
                }
                else
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, false);
                } 
        }

        PWM_Channel_OnOff_Status[channel_index] = true;
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On_And_Set_Duty(hwPWM_Channel channel_index, uint16_t duty)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        if(duty>PWM_MAX_DUTY){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false){return hwPWM_NotInit;}
        
        uint32_t pwmGen = PWM_Map_Gen(channel_index);
        uint32_t pwmOut = PWM_Map_Out(channel_index);
        uint32_t pwmOutMask = PWM_Map_Out_Mask(channel_index);
        
        if(pwmGen==0 || pwmOut==0 || pwmOutMask==0)
        {
                return hwGPIO_InvalidParameter;
        }

        float pwm_duty_float = ((float)duty/PWM_MAX_DUTY);
				
        MAP_PWMPulseWidthSet(PWM0_BASE, pwmOut, MAP_PWMGenPeriodGet(PWM0_BASE, pwmGen)*pwm_duty_float);

        PWM_Channel_OnOff_Status[channel_index] = true;
        PWM_Channel_Current_Duty[channel_index] = duty;
                
        if(PWM_Channel_Current_Duty[channel_index]>PWM_MAX_DUTY)
        {
                PWM_Channel_Current_Duty[channel_index] = PWM_MAX_DUTY;
        }
        
        if(PWM_Channel_Current_Duty[channel_index]==PWM_MAX_DUTY)
        {
                if(PWM_Channel_Inverse_Status[channel_index]==true)
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, false);
                }
                else
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
                }
        }
        else
        {
                if(PWM_Channel_Inverse_Status[channel_index]==true)
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
                }
                else
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, false);
                } 
        }

        return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_Off(hwPWM_Channel channel_index)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false){return hwPWM_NotInit;}
        
        uint32_t pwmOut = PWM_Map_Out(channel_index);
        uint32_t pwmOutMask = PWM_Map_Out_Mask(channel_index);
        
        if(pwmOut==0 || pwmOutMask==0)
        {
                return hwGPIO_InvalidParameter;
        }

        MAP_PWMPulseWidthSet(PWM0_BASE, pwmOut, 0);

        if(PWM_Channel_Current_Duty[channel_index]==PWM_MAX_DUTY)
        {
                if(PWM_Channel_Inverse_Status[channel_index]==true)
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
                }
                else
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, false);
                }
        }

        PWM_Channel_OnOff_Status[channel_index] = false;
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Set_Duty(hwPWM_Channel channel_index, uint16_t duty)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        if(duty>PWM_MAX_DUTY){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false){return hwPWM_NotInit;}
        
        if(PWM_Channel_OnOff_Status[channel_index]==false){return hwPWM_NotTurnOn;}
        
        uint32_t pwmGen = PWM_Map_Gen(channel_index);
        uint32_t pwmOut = PWM_Map_Out(channel_index);
        uint32_t pwmOutMask = PWM_Map_Out_Mask(channel_index);
        
        if(pwmGen==0 || pwmOut==0 || pwmOutMask==0)
        {
                return hwGPIO_InvalidParameter;
        }

        float pwm_duty_float = ((float)duty/PWM_MAX_DUTY);

        MAP_PWMPulseWidthSet(PWM0_BASE, pwmOut, MAP_PWMGenPeriodGet(PWM0_BASE, pwmGen)*pwm_duty_float);

        if(duty==PWM_MAX_DUTY)
        {
                if(PWM_Channel_Inverse_Status[channel_index]==true)
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, false);
                }
                else
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
                }
        }
        else
        {
                if(PWM_Channel_Inverse_Status[channel_index]==true)
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
                }
                else
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, false);
                } 
        }

        PWM_Channel_Current_Duty[channel_index] = duty;
        
        return hwPWM_OK;
}

hwPWM_OpResult PWM_Step_Duty(hwPWM_Channel channel_index, uint16_t step_duty, hwPWM_Step_Direction direction)
{
        if(channel_index>=hwPWM_Channel_MAX){return hwPWM_InvalidParameter;}
        if(direction>=hwPWM_Step_Dir_MAX){return hwPWM_InvalidParameter;}
        
        if(PWM_Channel_Init_Status[channel_index]==false){return hwPWM_NotInit;}
        
        if(PWM_Channel_OnOff_Status[channel_index]==false){return hwPWM_NotTurnOn;}
        
        uint32_t pwmGen = PWM_Map_Gen(channel_index);
        uint32_t pwmOut = PWM_Map_Out(channel_index);
        uint32_t pwmOutMask = PWM_Map_Out_Mask(channel_index);
        
        if(pwmGen==0 || pwmOut==0 || pwmOutMask==0)
        {
                return hwGPIO_InvalidParameter;
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
        
        float pwm_duty_float = ((float)current_duty/PWM_MAX_DUTY);
                                            
        MAP_PWMPulseWidthSet(PWM0_BASE, pwmOut, MAP_PWMGenPeriodGet(PWM0_BASE, pwmGen)*pwm_duty_float);

        if(current_duty==PWM_MAX_DUTY)
        {
                if(PWM_Channel_Inverse_Status[channel_index]==true)
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, false);
                }
                else
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
                }
        }
        else
        {
                if(PWM_Channel_Inverse_Status[channel_index]==true)
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, true);
                }
                else
                {
                        MAP_PWMOutputInvert(PWM0_BASE, pwmOutMask, false);
                } 
        }

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

#endif //DEVICE_TM4C1294
