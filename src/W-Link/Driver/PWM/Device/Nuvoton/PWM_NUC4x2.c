#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "Timer/Timer.h"
#include "PWM/PWM.h"

#ifdef NUC472

#include "PWM_Nuvoton.h"

#include "PWM_Nuvoton_Base.h"

PWM_T *PWM_Map_Soc_Base(hwPWM_Base_Index pwm_base)
{
    switch(pwm_base)
    {
#if defined(PWM0_BASE)
        case hwPWM_Base_1:
            return PWM0;
#endif

#if defined(PWM1_BASE)
        case hwPWM_Base_2:
            return PWM1;
#endif

        default:
            return NULL;
    }
}

void PWM_GPIO_ConfigAF(const PWM_Pin_Def *def)
{
    if(def == NULL || def->pin == hwGPIO_Pin_NC)
        return;

    SYS_UnlockReg();

    /*
     * 這裡先用 raw MFP value = 0x2。
     * 如果 NUC472_442.h 有官方 macro，例如 xxx_PWM0_CH0，
     * 建議之後改成官方 macro。
     */

    switch(def->pin)
    {
        case hwGPIO_Pin_A12:
            SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA12MFP_Msk) |
                             (0x2UL << SYS_GPA_MFPH_PA12MFP_Pos);
            break;

        case hwGPIO_Pin_A13:
            SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA13MFP_Msk) |
                             (0x2UL << SYS_GPA_MFPH_PA13MFP_Pos);
            break;

        case hwGPIO_Pin_A14:
            SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA14MFP_Msk) |
                             (0x2UL << SYS_GPA_MFPH_PA14MFP_Pos);
            break;

        case hwGPIO_Pin_A15:
            SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA15MFP_Msk) |
                             (0x2UL << SYS_GPA_MFPH_PA15MFP_Pos);
            break;

        case hwGPIO_Pin_B0:
            SYS->GPB_MFPL = (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB0MFP_Msk) |
                             (0x2UL << SYS_GPB_MFPL_PB0MFP_Pos);
            break;

        case hwGPIO_Pin_B1:
            SYS->GPB_MFPL = (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB1MFP_Msk) |
                             (0x2UL << SYS_GPB_MFPL_PB1MFP_Pos);
            break;

        case hwGPIO_Pin_B8:
            SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB8MFP_Msk) |
                             (0x2UL << SYS_GPB_MFPH_PB8MFP_Pos);
            break;

        case hwGPIO_Pin_B9:
            SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB9MFP_Msk) |
                             (0x2UL << SYS_GPB_MFPH_PB9MFP_Pos);
            break;

        case hwGPIO_Pin_B10:
            SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB10MFP_Msk) |
                             (0x2UL << SYS_GPB_MFPH_PB10MFP_Pos);
            break;

        case hwGPIO_Pin_B11:
            SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB11MFP_Msk) |
                             (0x2UL << SYS_GPB_MFPH_PB11MFP_Pos);
            break;

        case hwGPIO_Pin_B12:
            SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB12MFP_Msk) |
                             (0x2UL << SYS_GPB_MFPH_PB12MFP_Pos);
            break;

        case hwGPIO_Pin_B13:
            SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB13MFP_Msk) |
                             (0x2UL << SYS_GPB_MFPH_PB13MFP_Pos);
            break;

        case hwGPIO_Pin_C0:
            SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC0MFP_Msk) |
                             (0x2UL << SYS_GPC_MFPL_PC0MFP_Pos);
            break;

        case hwGPIO_Pin_C1:
            SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC1MFP_Msk) |
                             (0x2UL << SYS_GPC_MFPL_PC1MFP_Pos);
            break;

        case hwGPIO_Pin_C2:
            SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC2MFP_Msk) |
                             (0x2UL << SYS_GPC_MFPL_PC2MFP_Pos);
            break;

        case hwGPIO_Pin_C3:
            SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC3MFP_Msk) |
                             (0x2UL << SYS_GPC_MFPL_PC3MFP_Pos);
            break;

        case hwGPIO_Pin_C4:
            SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC4MFP_Msk) |
                             (0x2UL << SYS_GPC_MFPL_PC4MFP_Pos);
            break;

        case hwGPIO_Pin_C5:
            SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC5MFP_Msk) |
                             (0x2UL << SYS_GPC_MFPL_PC5MFP_Pos);
            break;

        case hwGPIO_Pin_D0:
            SYS->GPD_MFPL = (SYS->GPD_MFPL & ~SYS_GPD_MFPL_PD0MFP_Msk) |
                             (0x2UL << SYS_GPD_MFPL_PD0MFP_Pos);
            break;

        case hwGPIO_Pin_D1:
            SYS->GPD_MFPL = (SYS->GPD_MFPL & ~SYS_GPD_MFPL_PD1MFP_Msk) |
                             (0x2UL << SYS_GPD_MFPL_PD1MFP_Pos);
            break;

        case hwGPIO_Pin_D2:
            SYS->GPD_MFPL = (SYS->GPD_MFPL & ~SYS_GPD_MFPL_PD2MFP_Msk) |
                             (0x2UL << SYS_GPD_MFPL_PD2MFP_Pos);
            break;

        case hwGPIO_Pin_D3:
            SYS->GPD_MFPL = (SYS->GPD_MFPL & ~SYS_GPD_MFPL_PD3MFP_Msk) |
                             (0x2UL << SYS_GPD_MFPL_PD3MFP_Pos);
            break;

        case hwGPIO_Pin_D4:
            SYS->GPD_MFPL = (SYS->GPD_MFPL & ~SYS_GPD_MFPL_PD4MFP_Msk) |
                             (0x2UL << SYS_GPD_MFPL_PD4MFP_Pos);
            break;

        case hwGPIO_Pin_D5:
            SYS->GPD_MFPL = (SYS->GPD_MFPL & ~SYS_GPD_MFPL_PD5MFP_Msk) |
                             (0x2UL << SYS_GPD_MFPL_PD5MFP_Pos);
            break;

        default:
            break;
    }

    GPIO_T *port = GPIO_Map_Soc_Base(def->pin);
    uint16_t pin = GPIO_Map_Soc_Pin(def->pin);

    if(port != NULL && pin != 0)
        GPIO_SetMode(port, pin, GPIO_MODE_OUTPUT);

    SYS_LockReg();
}

void PWM_GPIO_DeConfigAF(const PWM_Pin_Def *def)
{
    if(def == NULL || def->pin == hwGPIO_Pin_NC)
        return;

    SYS_UnlockReg();

    switch(def->pin)
    {
        case hwGPIO_Pin_A12: SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA12MFP_Msk; break;
        case hwGPIO_Pin_A13: SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA13MFP_Msk; break;
        case hwGPIO_Pin_A14: SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA14MFP_Msk; break;
        case hwGPIO_Pin_A15: SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA15MFP_Msk; break;

        case hwGPIO_Pin_B0:  SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB0MFP_Msk;  break;
        case hwGPIO_Pin_B1:  SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB1MFP_Msk;  break;
        case hwGPIO_Pin_B8:  SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB8MFP_Msk;  break;
        case hwGPIO_Pin_B9:  SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB9MFP_Msk;  break;
        case hwGPIO_Pin_B10: SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB10MFP_Msk; break;
        case hwGPIO_Pin_B11: SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB11MFP_Msk; break;
        case hwGPIO_Pin_B12: SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB12MFP_Msk; break;
        case hwGPIO_Pin_B13: SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB13MFP_Msk; break;

        case hwGPIO_Pin_C0:  SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC0MFP_Msk;  break;
        case hwGPIO_Pin_C1:  SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC1MFP_Msk;  break;
        case hwGPIO_Pin_C2:  SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC2MFP_Msk;  break;
        case hwGPIO_Pin_C3:  SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC3MFP_Msk;  break;
        case hwGPIO_Pin_C4:  SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC4MFP_Msk;  break;
        case hwGPIO_Pin_C5:  SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC5MFP_Msk;  break;

        case hwGPIO_Pin_D0:  SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD0MFP_Msk;  break;
        case hwGPIO_Pin_D1:  SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD1MFP_Msk;  break;
        case hwGPIO_Pin_D2:  SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD2MFP_Msk;  break;
        case hwGPIO_Pin_D3:  SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD3MFP_Msk;  break;
        case hwGPIO_Pin_D4:  SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD4MFP_Msk;  break;
        case hwGPIO_Pin_D5:  SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD5MFP_Msk;  break;

        default:
            break;
    }

    GPIO_T *port = GPIO_Map_Soc_Base(def->pin);
    uint16_t pin = GPIO_Map_Soc_Pin(def->pin);

    if(port != NULL && pin != 0)
        GPIO_SetMode(port, pin, GPIO_MODE_INPUT);

    SYS_LockReg();
}

void PWM_Clock_Enable(hwPWM_Base_Index pwm_base, hwPWM_Channel channel_index)
{
        if(pwm_base >= hwPWM_Base_MAX)
        {
                return;
        }
                
        if(channel_index >= hwPWM_Channel_MAX)
        {
                return;
        }
                
        switch(pwm_base)
        {
#if defined(PWM0_BASE)
                case hwPWM_Base_1:
                        switch(channel_index)
                        {
                                case hwPWM_Channel_1:
                                case hwPWM_Channel_2:
                                        CLK_EnableModuleClock(PWM0CH01_MODULE);
                                        break;

                                case hwPWM_Channel_3:
                                case hwPWM_Channel_4:
                                        CLK_EnableModuleClock(PWM0CH23_MODULE);
                                        break;

                                case hwPWM_Channel_5:
                                case hwPWM_Channel_6:
                                        CLK_EnableModuleClock(PWM0CH45_MODULE);
                                        break;

                                default:
                                        break;
                        }
                        break;
#endif

#if defined(PWM1_BASE)
                case hwPWM_Base_2:
                        switch(channel_index)
                        {
                                case hwPWM_Channel_7:
                                case hwPWM_Channel_8:
                                        CLK_EnableModuleClock(PWM1CH01_MODULE);
                                        break;

                                case hwPWM_Channel_9:
                                case hwPWM_Channel_10:
                                        CLK_EnableModuleClock(PWM1CH23_MODULE);
                                        break;

                                case hwPWM_Channel_11:
                                case hwPWM_Channel_12:
                                        CLK_EnableModuleClock(PWM1CH45_MODULE);
                                        break;
                        }
                        break;
#endif
        }
}

void PWM_Clock_Disable(hwPWM_Base_Index pwm_base, hwPWM_Channel channel_index)
{
        if(pwm_base >= hwPWM_Base_MAX)
        {
                return;
        }
                
        if(channel_index >= hwPWM_Channel_MAX)
        {
                return;
        }
                
        bool all_off;

        switch(pwm_base)
        {
#if defined(PWM0_BASE)
                case hwPWM_Base_1:
                        switch(channel_index)
                        {
                                case hwPWM_Channel_1:
                                case hwPWM_Channel_2:
                                        if(!PWM_Channel_Init_Status[hwPWM_Channel_1] &&
                                           !PWM_Channel_Init_Status[hwPWM_Channel_2])
                                        {
                                            all_off = true;
                                        }
                                        else
                                        {
                                            all_off = false;
                                        }
                                        if(all_off)
                                        {
                                                CLK_DisableModuleClock(PWM0CH01_MODULE);
                                        }
                                        break;

                                case hwPWM_Channel_3:
                                case hwPWM_Channel_4:
                                        if(!PWM_Channel_Init_Status[hwPWM_Channel_3] &&
                                           !PWM_Channel_Init_Status[hwPWM_Channel_4])
                                        {
                                            all_off = true;
                                        }
                                        else
                                        {
                                            all_off = false;
                                        }
                                        if(all_off)
                                        {
                                                CLK_DisableModuleClock(PWM0CH23_MODULE);
                                        }
                                        break;

                                case hwPWM_Channel_5:
                                case hwPWM_Channel_6:
                                        if(!PWM_Channel_Init_Status[hwPWM_Channel_5] &&
                                           !PWM_Channel_Init_Status[hwPWM_Channel_6])
                                        {
                                            all_off = true;
                                        }
                                        else
                                        {
                                            all_off = false;
                                        }
                                        if(all_off)
                                        {
                                                CLK_DisableModuleClock(PWM0CH45_MODULE);
                                        }
                                        break;

                                default:
                                        break;
                        }
                        break;
#endif

#if defined(PWM1_BASE)
                case hwPWM_Base_2:
                        switch(channel_index)
                        {
                                case hwPWM_Channel_7:
                                case hwPWM_Channel_8:
                                        if(!PWM_Channel_Init_Status[hwPWM_Channel_7] &&
                                           !PWM_Channel_Init_Status[hwPWM_Channel_8])
                                        {
                                            all_off = true;
                                        }
                                        else
                                        {
                                            all_off = false;
                                        }
                                        if(all_off)
                                        {
                                                CLK_DisableModuleClock(PWM1CH01_MODULE);
                                        }
                                        break;

                                case hwPWM_Channel_9:
                                case hwPWM_Channel_10:
                                        if(!PWM_Channel_Init_Status[hwPWM_Channel_9] &&
                                           !PWM_Channel_Init_Status[hwPWM_Channel_10])
                                        {
                                            all_off = true;
                                        }
                                        else
                                        {
                                            all_off = false;
                                        }
                                        if(all_off)
                                        {
                                                CLK_DisableModuleClock(PWM1CH23_MODULE);
                                        }
                                        break;

                                case hwPWM_Channel_11:
                                case hwPWM_Channel_12:
                                        if(!PWM_Channel_Init_Status[hwPWM_Channel_11] &&
                                           !PWM_Channel_Init_Status[hwPWM_Channel_12])
                                        {
                                            all_off = true;
                                        }
                                        else
                                        {
                                            all_off = false;
                                        }
                                        if(all_off)
                                        {
                                                CLK_DisableModuleClock(PWM1CH45_MODULE);
                                        }
                                        break;
                        }
                        break;
#endif
        }
}

uint32_t PWM_Channel_To_Mask(hwPWM_Channel channel_index)
{
    switch(channel_index)
    {
        case hwPWM_Channel_1:
        case hwPWM_Channel_7:
            return PWM_CH_0_MASK;

        case hwPWM_Channel_2:
        case hwPWM_Channel_8:
            return PWM_CH_1_MASK;

        case hwPWM_Channel_3:
        case hwPWM_Channel_9:
            return PWM_CH_2_MASK;

        case hwPWM_Channel_4:
        case hwPWM_Channel_10:
            return PWM_CH_3_MASK;

        case hwPWM_Channel_5:
        case hwPWM_Channel_11:
            return PWM_CH_4_MASK;

        case hwPWM_Channel_6:
        case hwPWM_Channel_12:
            return PWM_CH_5_MASK;

        default:
            return 0;
    }
}

#endif /* NUC472 */