#ifndef PWM_PIN_STM32U5_H
#define PWM_PIN_STM32U5_H

#include "PWM_Pin_STM32_Def.h"

#include "PWM/Device/STM32/PWM_STM32_Base.h"

typedef enum {
    PWM_Pinset_DEFAULT = 0,
    PWM_Pinset_ALT1,
    PWM_Pinset_ALT2,
    PWM_Pinset_ALT3,
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
#ifndef CONFIG_PWM4_PINSET
#define CONFIG_PWM4_PINSET PWM_Pinset_DEFAULT
#endif
#ifndef CONFIG_PWM5_PINSET
#define CONFIG_PWM5_PINSET PWM_Pinset_DEFAULT
#endif
#ifndef CONFIG_PWM8_PINSET
#define CONFIG_PWM8_PINSET PWM_Pinset_DEFAULT
#endif
#ifndef CONFIG_PWM15_PINSET
#define CONFIG_PWM15_PINSET PWM_Pinset_DEFAULT
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
#if defined(TIM4_BASE)
    CONFIG_PWM4_PINSET,
#endif
#if defined(TIM5_BASE)
    CONFIG_PWM5_PINSET,
#endif
#if defined(TIM8_BASE)
    CONFIG_PWM8_PINSET,
#endif
#if defined(TIM15_BASE)
    CONFIG_PWM15_PINSET,
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
        { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_A8,  TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_A9,  TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_A10, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_4, hwTimer_Index_0, hwGPIO_Pin_A11, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_E9,  TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_E11, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_E13, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_4, hwTimer_Index_0, hwGPIO_Pin_E14, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_A7,  TIM_CHANNEL_1, 1 },
        { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_B0,  TIM_CHANNEL_2, 1 },
        { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_B1,  TIM_CHANNEL_3, 1 },
        { hwPWM_Channel_4, hwTimer_Index_0, hwGPIO_Pin_E15, TIM_CHANNEL_4, 1 },
    },
    {
        { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_B13, TIM_CHANNEL_1, 1 },
        { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_B14, TIM_CHANNEL_2, 1 },
        { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_B15, TIM_CHANNEL_3, 1 },
        { hwPWM_Channel_4, hwTimer_Index_0, hwGPIO_Pin_C5,  TIM_CHANNEL_4, 1 },
    },
},
#endif

#if defined(TIM2_BASE)
{
    {
        { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A0,  TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_6, hwTimer_Index_1, hwGPIO_Pin_A1,  TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_7, hwTimer_Index_1, hwGPIO_Pin_A2,  TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_8, hwTimer_Index_1, hwGPIO_Pin_A3,  TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A5,  TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_6, hwTimer_Index_1, hwGPIO_Pin_B3,  TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_7, hwTimer_Index_1, hwGPIO_Pin_B10, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_8, hwTimer_Index_1, hwGPIO_Pin_B11, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A15, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
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
        { hwPWM_Channel_9,  hwTimer_Index_2, hwGPIO_Pin_B4, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_10, hwTimer_Index_2, hwGPIO_Pin_B5, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_11, hwTimer_Index_2, hwGPIO_Pin_C8, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_12, hwTimer_Index_2, hwGPIO_Pin_C9, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_9,  hwTimer_Index_2, hwGPIO_Pin_C6, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_10, hwTimer_Index_2, hwGPIO_Pin_C7, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_11, hwTimer_Index_2, hwGPIO_Pin_E5, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_12, hwTimer_Index_2, hwGPIO_Pin_E6, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_9,  hwTimer_Index_2, hwGPIO_Pin_E3, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_10, hwTimer_Index_2, hwGPIO_Pin_E4, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
    },
},
#endif

#if defined(TIM4_BASE)
{
    {
        { hwPWM_Channel_13, hwTimer_Index_3, hwGPIO_Pin_B6, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_14, hwTimer_Index_3, hwGPIO_Pin_B7, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_15, hwTimer_Index_3, hwGPIO_Pin_B8, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_16, hwTimer_Index_3, hwGPIO_Pin_B9, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_13, hwTimer_Index_3, hwGPIO_Pin_D12, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_14, hwTimer_Index_3, hwGPIO_Pin_D13, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_15, hwTimer_Index_3, hwGPIO_Pin_D14, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_16, hwTimer_Index_3, hwGPIO_Pin_D15, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_3, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_3, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_3, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_3, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_3, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_3, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_3, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_3, hwGPIO_Pin_NC, 0, 0 },
    },
},
#endif

#if defined(TIM5_BASE)
{
    {
        { hwPWM_Channel_17, hwTimer_Index_4, hwGPIO_Pin_A0, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_18, hwTimer_Index_4, hwGPIO_Pin_A1, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_19, hwTimer_Index_4, hwGPIO_Pin_A2, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_20, hwTimer_Index_4, hwGPIO_Pin_A3, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_17, hwTimer_Index_4, hwGPIO_Pin_F6, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_18, hwTimer_Index_4, hwGPIO_Pin_F7, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_19, hwTimer_Index_4, hwGPIO_Pin_F8, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_20, hwTimer_Index_4, hwGPIO_Pin_F9, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
    },
},
#endif

#if defined(TIM8_BASE)
{
    {
        { hwPWM_Channel_21, hwTimer_Index_7, hwGPIO_Pin_C6, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_22, hwTimer_Index_7, hwGPIO_Pin_C7, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_23, hwTimer_Index_7, hwGPIO_Pin_C8, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_24, hwTimer_Index_7, hwGPIO_Pin_C9, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_21, hwTimer_Index_7, hwGPIO_Pin_I5, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_22, hwTimer_Index_7, hwGPIO_Pin_I6, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_23, hwTimer_Index_7, hwGPIO_Pin_I7, TIM_CHANNEL_3, 0 },
        { hwPWM_Channel_24, hwTimer_Index_7, hwGPIO_Pin_I2, TIM_CHANNEL_4, 0 },
    },
    {
        { hwPWM_Channel_21, hwTimer_Index_7, hwGPIO_Pin_A5,  TIM_CHANNEL_1, 1 },
        { hwPWM_Channel_22, hwTimer_Index_7, hwGPIO_Pin_B0,  TIM_CHANNEL_2, 1 },
        { hwPWM_Channel_23, hwTimer_Index_7, hwGPIO_Pin_B1,  TIM_CHANNEL_3, 1 },
        { hwPWM_Channel_24, hwTimer_Index_7, hwGPIO_Pin_B2,  TIM_CHANNEL_4, 1 },
    },
    {
        { hwPWM_Channel_21, hwTimer_Index_7, hwGPIO_Pin_H13, TIM_CHANNEL_1, 1 },
        { hwPWM_Channel_22, hwTimer_Index_7, hwGPIO_Pin_H14, TIM_CHANNEL_2, 1 },
        { hwPWM_Channel_23, hwTimer_Index_7, hwGPIO_Pin_H15, TIM_CHANNEL_3, 1 },
        { hwPWM_Channel_24, hwTimer_Index_7, hwGPIO_Pin_H12, TIM_CHANNEL_4, 1 },
    },
},
#endif

#if defined(TIM15_BASE)
{
    {
        { hwPWM_Channel_33, hwTimer_Index_14, hwGPIO_Pin_A2,  TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_34, hwTimer_Index_14, hwGPIO_Pin_A3,  TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_33, hwTimer_Index_14, hwGPIO_Pin_B14, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_34, hwTimer_Index_14, hwGPIO_Pin_B15, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_33, hwTimer_Index_14, hwGPIO_Pin_F9,  TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_34, hwTimer_Index_14, hwGPIO_Pin_F10, TIM_CHANNEL_2, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_34, hwTimer_Index_14, hwGPIO_Pin_G10, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
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
        { hwPWM_Channel_35, hwTimer_Index_15, hwGPIO_Pin_B8, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_35, hwTimer_Index_15, hwGPIO_Pin_E0, TIM_CHANNEL_1, 0 },
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
        { hwPWM_Channel_36, hwTimer_Index_16, hwGPIO_Pin_B9, TIM_CHANNEL_1, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
    },
    {
        { hwPWM_Channel_36, hwTimer_Index_16, hwGPIO_Pin_E1, TIM_CHANNEL_1, 0 },
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

#endif // PWM_PIN_STM32U5_H