
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "PWM/PWM.h"

#if defined(TM4C1294)

#include "PWM/Pin/PWM_Pin.h"

uint32_t PWM_Map_Gen(hwPWM_Channel ch)
{
        switch(ch)
        {
                case hwPWM_Channel_1: return PWM_GEN_0;
                case hwPWM_Channel_2: return PWM_GEN_0;
                case hwPWM_Channel_3: return PWM_GEN_1;
                case hwPWM_Channel_4: return PWM_GEN_1;
                case hwPWM_Channel_5: return PWM_GEN_2;
                case hwPWM_Channel_6: return PWM_GEN_2;
                case hwPWM_Channel_7: return PWM_GEN_3;
                case hwPWM_Channel_8: return PWM_GEN_3;
        }

        return 0;
}

uint32_t PWM_Map_Gen_Mask(hwPWM_Channel ch)
{
        switch(ch)
        {
                case hwPWM_Channel_1: return PWM_GEN_0_BIT;
                case hwPWM_Channel_2: return PWM_GEN_0_BIT;
                case hwPWM_Channel_3: return PWM_GEN_1_BIT;
                case hwPWM_Channel_4: return PWM_GEN_1_BIT;
                case hwPWM_Channel_5: return PWM_GEN_2_BIT;
                case hwPWM_Channel_6: return PWM_GEN_2_BIT;
                case hwPWM_Channel_7: return PWM_GEN_3_BIT;
                case hwPWM_Channel_8: return PWM_GEN_3_BIT;
        }

        return 0;
}

uint32_t PWM_Map_Out(hwPWM_Channel ch)
{
        switch(ch)
        {
                case hwPWM_Channel_1: return PWM_OUT_0;
                case hwPWM_Channel_2: return PWM_OUT_1;
                case hwPWM_Channel_3: return PWM_OUT_2;
                case hwPWM_Channel_4: return PWM_OUT_3;
                case hwPWM_Channel_5: return PWM_OUT_4;
                case hwPWM_Channel_6: return PWM_OUT_5;
                case hwPWM_Channel_7: return PWM_OUT_6;
                case hwPWM_Channel_8: return PWM_OUT_7;
        }

        return 0;
}

uint32_t PWM_Map_Out_Mask(hwPWM_Channel ch)
{
        switch(ch)
        {
                case hwPWM_Channel_1: return PWM_OUT_0_BIT;
                case hwPWM_Channel_2: return PWM_OUT_1_BIT;
                case hwPWM_Channel_3: return PWM_OUT_2_BIT;
                case hwPWM_Channel_4: return PWM_OUT_3_BIT;
                case hwPWM_Channel_5: return PWM_OUT_4_BIT;
                case hwPWM_Channel_6: return PWM_OUT_5_BIT;
                case hwPWM_Channel_7: return PWM_OUT_6_BIT;
                case hwPWM_Channel_8: return PWM_OUT_7_BIT;
        }

        return 0;
}

uint32_t PWM_Map_Pin_Cfg(hwPWM_Channel ch, hwGPIO_Pin pin)
{
        switch(ch)
        {
                case hwPWM_Channel_1:
                        if (pin == hwGPIO_Pin_F0) return GPIO_PF0_M0PWM0;
                        break;
                case hwPWM_Channel_2:
                        if (pin == hwGPIO_Pin_F1) return GPIO_PF1_M0PWM1;
                        break;
                case hwPWM_Channel_3:
                        if (pin == hwGPIO_Pin_F2) return GPIO_PF2_M0PWM2;
                        break;
                case hwPWM_Channel_4:
                        if (pin == hwGPIO_Pin_F3) return GPIO_PF3_M0PWM3;
                        break;
                case hwPWM_Channel_5:
                        if (pin == hwGPIO_Pin_G0) return GPIO_PG0_M0PWM4;
                        break;
                case hwPWM_Channel_6:
                        if (pin == hwGPIO_Pin_G1) return GPIO_PG1_M0PWM5;
                        break;
                case hwPWM_Channel_7:
                        if (pin == hwGPIO_Pin_K4) return GPIO_PK4_M0PWM6;
                        break;
                case hwPWM_Channel_8:
                        if (pin == hwGPIO_Pin_K5) return GPIO_PK5_M0PWM7;
                        break;
        }

        return 0;
}

#endif //TM4C1294
