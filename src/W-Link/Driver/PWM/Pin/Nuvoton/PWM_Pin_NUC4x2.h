#ifndef PWM_PIN_NUC4X2_H
#define PWM_PIN_NUC4X2_H

#include "PWM_Pin_Nuvoton_Def.h"

#include "PWM/Device/Nuvoton/PWM_Nuvoton_Base.h"

typedef enum {
    PWM_Pinset_DEFAULT = 0,
    PWM_Pinset_ALT1,
    PWM_Pinset_MAX
} PWM_Pinset_t;

#ifndef CONFIG_PWM0_PINSET
#define CONFIG_PWM0_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM1_PINSET
#define CONFIG_PWM1_PINSET PWM_Pinset_DEFAULT
#endif

static const PWM_Pinset_t PWM_Index_Map_Alt[hwPWM_Base_MAX] = {
#if defined(PWM0_BASE)
    CONFIG_PWM0_PINSET,
#endif
#if defined(PWM1_BASE)
    CONFIG_PWM1_PINSET,
#endif
};

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Base_MAX][PWM_Pinset_MAX][6] =
{
#if defined(PWM0_BASE)
    {
        /* DEFAULT */
        {
            { hwPWM_Channel_1, hwPWM_Base_1, hwGPIO_Pin_A12, 0 },
            { hwPWM_Channel_2, hwPWM_Base_1, hwGPIO_Pin_A13, 1 },
            { hwPWM_Channel_3, hwPWM_Base_1, hwGPIO_Pin_A14, 2 },
            { hwPWM_Channel_4, hwPWM_Base_1, hwGPIO_Pin_A15, 3 },
            { hwPWM_Channel_5, hwPWM_Base_1, hwGPIO_Pin_B0,  4 },
            { hwPWM_Channel_6, hwPWM_Base_1, hwGPIO_Pin_B1,  5 },
        },

        /* ALT1 */
        {
            { hwPWM_Channel_1, hwPWM_Base_1, hwGPIO_Pin_C0, 0 },
            { hwPWM_Channel_2, hwPWM_Base_1, hwGPIO_Pin_C1, 1 },
            { hwPWM_Channel_3, hwPWM_Base_1, hwGPIO_Pin_C2, 2 },
            { hwPWM_Channel_4, hwPWM_Base_1, hwGPIO_Pin_C3, 3 },
            { hwPWM_Channel_5, hwPWM_Base_1, hwGPIO_Pin_C4, 4 },
            { hwPWM_Channel_6, hwPWM_Base_1, hwGPIO_Pin_C5, 5 },
        },
    },
#endif

#if defined(PWM1_BASE)
    {
        /* DEFAULT */
        {
            { hwPWM_Channel_7, hwPWM_Base_2, hwGPIO_Pin_B8,  0 },
            { hwPWM_Channel_8, hwPWM_Base_2, hwGPIO_Pin_B9,  1 },
            { hwPWM_Channel_9, hwPWM_Base_2, hwGPIO_Pin_B10, 2 },
            { hwPWM_Channel_10, hwPWM_Base_2, hwGPIO_Pin_B11, 3 },
            { hwPWM_Channel_11, hwPWM_Base_2, hwGPIO_Pin_B12, 4 },
            { hwPWM_Channel_12, hwPWM_Base_2, hwGPIO_Pin_B13, 5 },
        },

        /* ALT1 */
        {
            { hwPWM_Channel_7, hwPWM_Base_2, hwGPIO_Pin_D0, 0 },
            { hwPWM_Channel_8, hwPWM_Base_2, hwGPIO_Pin_D1, 1 },
            { hwPWM_Channel_9, hwPWM_Base_2, hwGPIO_Pin_D2, 2 },
            { hwPWM_Channel_10, hwPWM_Base_2, hwGPIO_Pin_D3, 3 },
            { hwPWM_Channel_11, hwPWM_Base_2, hwGPIO_Pin_D4, 4 },
            { hwPWM_Channel_12, hwPWM_Base_2, hwGPIO_Pin_D5, 5 },
        },
    },
#endif
};

#endif /* PWM_PIN_NUC4X2_H */