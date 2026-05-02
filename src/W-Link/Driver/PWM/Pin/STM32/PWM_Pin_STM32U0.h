#ifndef PWM_PIN_STM32U0_H
#define PWM_PIN_STM32U0_H

#include "PWM_Pin_STM32_Def.h"

#include "PWM/Device/STM32/PWM_STM32_Base.h"

typedef enum {
    PWM_Pinset_DEFAULT = 0,
    PWM_Pinset_ALT1,
    PWM_Pinset_MAX
} PWM_Pinset_t;

/* ===== Config ===== */

#ifndef CONFIG_PWM1_PINSET
#define CONFIG_PWM1_PINSET PWM_Pinset_DEFAULT
#endif
#ifndef CONFIG_PWM2_PINSET
#define CONFIG_PWM2_PINSET PWM_Pinset_DEFAULT
#endif
#ifndef CONFIG_PWM3_PINSET
#define CONFIG_PWM3_PINSET PWM_Pinset_DEFAULT
#endif
#ifndef CONFIG_PWM14_PINSET
#define CONFIG_PWM14_PINSET PWM_Pinset_DEFAULT
#endif
#ifndef CONFIG_PWM16_PINSET
#define CONFIG_PWM16_PINSET PWM_Pinset_DEFAULT
#endif
#ifndef CONFIG_PWM17_PINSET
#define CONFIG_PWM17_PINSET PWM_Pinset_DEFAULT
#endif

/* ===== Index Map ===== */

static const PWM_Pinset_t PWM_Index_Map_Alt[hwPWM_Base_MAX] = {
#if defined(TIM1_BASE)
    CONFIG_PWM1_PINSET,
#endif
#if defined(TIM2_BASE)
    CONFIG_PWM2_PINSET,
#endif
#if defined(TIM3_BASE)
    CONFIG_PWM3_PINSET,
#endif
#if defined(TIM14_BASE)
    CONFIG_PWM14_PINSET,
#endif
#if defined(TIM16_BASE)
    CONFIG_PWM16_PINSET,
#endif
#if defined(TIM17_BASE)
    CONFIG_PWM17_PINSET,
#endif
};

/* ===== Pin Table ===== */

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Base_MAX][PWM_Pinset_MAX][4] =
{

#if defined(TIM1_BASE)
{
    {
        { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_A8, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_A9, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_A10, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_4, hwTimer_Index_0, hwGPIO_Pin_A11, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_0, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_0, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_0, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_0, hwGPIO_Pin_NC, 0, 0 },
    },
},
#endif

#if defined(TIM2_BASE)
{
    {
        { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A0, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_6, hwTimer_Index_1, hwGPIO_Pin_A1, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_7, hwTimer_Index_1, hwGPIO_Pin_A2, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_8, hwTimer_Index_1, hwGPIO_Pin_A3, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A5, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
    },
},
#endif

#if defined(TIM3_BASE)
{
    {
        { hwPWM_Channel_9,  hwTimer_Index_2, hwGPIO_Pin_A6, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_10, hwTimer_Index_2, hwGPIO_Pin_A7, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_11, hwTimer_Index_2, hwGPIO_Pin_B0, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_12, hwTimer_Index_2, hwGPIO_Pin_B1, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
    },
},
#endif

#if defined(TIM14_BASE)
{
    {
        { hwPWM_Channel_33, hwTimer_Index_13, hwGPIO_Pin_A7, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
    },
},
#endif

#if defined(TIM16_BASE)
{
    {
        { hwPWM_Channel_35, hwTimer_Index_15, hwGPIO_Pin_A6, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
    },
},
#endif

#if defined(TIM17_BASE)
{
    {
        { hwPWM_Channel_36, hwTimer_Index_16, hwGPIO_Pin_A7, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
    },
},
#endif

};

#endif // PWM_PIN_STM32U0_H