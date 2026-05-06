#ifndef PWM_PIN_RP2_H
#define PWM_PIN_RP2_H

#include "PWM_Pin_RP2_Def.h"

#include "PWM/Device/RP2/PWM_RP2_Base.h"

#ifdef DEVICE_RP2

typedef enum {
    PWM_Pinset_DEFAULT = 0,
    PWM_Pinset_MAX
} PWM_Pinset_t;

#ifndef CONFIG_PWM0_PINSET
#define CONFIG_PWM0_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM1_PINSET
#define CONFIG_PWM1_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM2_PINSET
#define CONFIG_PWM2_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM3_PINSET
#define CONFIG_PWM3_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM4_PINSET
#define CONFIG_PWM4_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM5_PINSET
#define CONFIG_PWM5_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM6_PINSET
#define CONFIG_PWM6_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM7_PINSET
#define CONFIG_PWM7_PINSET PWM_Pinset_DEFAULT
#endif

static const PWM_Pinset_t PWM_Index_Map_Alt[hwPWM_Base_MAX] = {
    CONFIG_PWM0_PINSET,
    CONFIG_PWM1_PINSET,
    CONFIG_PWM2_PINSET,
    CONFIG_PWM3_PINSET,
    CONFIG_PWM4_PINSET,
    CONFIG_PWM5_PINSET,
    CONFIG_PWM6_PINSET,
    CONFIG_PWM7_PINSET,
};

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Base_MAX][PWM_Pinset_MAX][2] =
{
    {
        {
            { hwPWM_Channel_0, hwPWM_Base_0, hwGPIO_Pin_0, 0, 0 },
            { hwPWM_Channel_1, hwPWM_Base_0, hwGPIO_Pin_1, 0, 1 },
        },
    },
    {
        {
            { hwPWM_Channel_2, hwPWM_Base_1, hwGPIO_Pin_2, 1, 0 },
            { hwPWM_Channel_3, hwPWM_Base_1, hwGPIO_Pin_3, 1, 1 },
        },
    },
    {
        {
            { hwPWM_Channel_4, hwPWM_Base_2, hwGPIO_Pin_4, 2, 0 },
            { hwPWM_Channel_5, hwPWM_Base_2, hwGPIO_Pin_5, 2, 1 },
        },
    },
    {
        {
            { hwPWM_Channel_6, hwPWM_Base_3, hwGPIO_Pin_6, 3, 0 },
            { hwPWM_Channel_7, hwPWM_Base_3, hwGPIO_Pin_7, 3, 1 },
        },
    },
    {
        {
            { hwPWM_Channel_8, hwPWM_Base_4, hwGPIO_Pin_8, 4, 0 },
            { hwPWM_Channel_9, hwPWM_Base_4, hwGPIO_Pin_9, 4, 1 },
        },
    },
    {
        {
            { hwPWM_Channel_10, hwPWM_Base_5, hwGPIO_Pin_10, 5, 0 },
            { hwPWM_Channel_11, hwPWM_Base_5, hwGPIO_Pin_11, 5, 1 },
        },
    },
    {
        {
            { hwPWM_Channel_12, hwPWM_Base_6, hwGPIO_Pin_12, 6, 0 },
            { hwPWM_Channel_13, hwPWM_Base_6, hwGPIO_Pin_13, 6, 1 },
        },
    },
    {
        {
            { hwPWM_Channel_14, hwPWM_Base_7, hwGPIO_Pin_14, 7, 0 },
            { hwPWM_Channel_15, hwPWM_Base_7, hwGPIO_Pin_15, 7, 1 },
        },
    },
};

#endif // DEVICE_RP2

#endif // PWM_PIN_RP2_H