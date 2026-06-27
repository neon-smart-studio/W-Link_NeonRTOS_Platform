
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "PWM/PWM.h"

#if defined(TM4C1294)

#include "PWM/Pin/PWM_Pin.h"

#define PWM_HZ           1000

static bool PWM_Channel_Init_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_OnOff_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_Inverse_Status[hwPWM_Channel_MAX] = {false};
static uint16_t PWM_Channel_Current_Duty[hwPWM_Channel_MAX] = {0};

typedef enum
{
	hwPWM_Base_0 = 0,
	hwPWM_Base_MAX
}hwPWM_Base;

const uint32_t Map_Soc_PWM_Pin_Cfg[hwPWM_Channel_MAX] = 
{
    GPIO_PF0_M0PWM0,
    GPIO_PF1_M0PWM1,
    GPIO_PF2_M0PWM2,
    GPIO_PF3_M0PWM3,
    GPIO_PG0_M0PWM4,
    GPIO_PG1_M0PWM5,
    GPIO_PK4_M0PWM6,
    GPIO_PK5_M0PWM7
};

const uint32_t Map_Soc_PWM_Gen[hwPWM_Channel_MAX] = 
{
    PWM_GEN_0,
    PWM_GEN_0,
    PWM_GEN_1,
    PWM_GEN_1,
    PWM_GEN_2,
    PWM_GEN_2,
    PWM_GEN_3,
    PWM_GEN_3
};

const uint32_t Map_Soc_PWM_Gen_Mask[hwPWM_Channel_MAX] = 
{
    PWM_GEN_0_BIT,
    PWM_GEN_0_BIT,
    PWM_GEN_1_BIT,
    PWM_GEN_1_BIT,
    PWM_GEN_2_BIT,
    PWM_GEN_2_BIT,
    PWM_GEN_3_BIT,
    PWM_GEN_3_BIT
};

const uint32_t Map_Soc_PWM_Out[hwPWM_Channel_MAX] = 
{
    PWM_OUT_0,
    PWM_OUT_1,
    PWM_OUT_2,
    PWM_OUT_3,
    PWM_OUT_4,
    PWM_OUT_5,
    PWM_OUT_6,
    PWM_OUT_7
};

const uint32_t Map_Soc_PWM_Out_Mask[hwPWM_Channel_MAX] = 
{
    PWM_OUT_0_BIT,
    PWM_OUT_1_BIT,
    PWM_OUT_2_BIT,
    PWM_OUT_3_BIT,
    PWM_OUT_4_BIT,
    PWM_OUT_5_BIT,
    PWM_OUT_6_BIT,
    PWM_OUT_7_BIT
};

void PWM_Map_Pin_Cfg(hwPWM_Channel ch, hwGPIO_Pin pin)
{
        switch(ch)
        {
                case hwPWM_Channel_1:
                        break;
                case hwPWM_Channel_2:
                        break;
                case hwPWM_Channel_3:
                        break;
                case hwPWM_Channel_4:
                        break;
                case hwPWM_Channel_5:
                        break;
                case hwPWM_Channel_6:
                        break;
                case hwPWM_Channel_7:
                        break;
                case hwPWM_Channel_8:
                        break;
        }
}

#endif //TM4C1294
