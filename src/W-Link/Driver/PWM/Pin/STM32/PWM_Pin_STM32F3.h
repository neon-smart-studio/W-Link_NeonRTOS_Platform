#ifndef PWM_PIN_STM32F3_H
#define PWM_PIN_STM32F3_H

#include "PWM_Pin_STM32_Def.h"

#include "PWM/Device/STM32/PWM_STM32_Base.h"

typedef enum {
    PWM_Pinset_DEFAULT = 0,
    PWM_Pinset_ALT1,
    PWM_Pinset_ALT2,
    PWM_Pinset_MAX
} PWM_Pinset_t;

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

#ifndef CONFIG_PWM9_PINSET
#define CONFIG_PWM9_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM10_PINSET
#define CONFIG_PWM10_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM11_PINSET
#define CONFIG_PWM11_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM12_PINSET
#define CONFIG_PWM12_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM13_PINSET
#define CONFIG_PWM13_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM14_PINSET
#define CONFIG_PWM14_PINSET PWM_Pinset_DEFAULT
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
#if defined(TIM9_BASE)
    CONFIG_PWM9_PINSET,
#endif
#if defined(TIM10_BASE)
    CONFIG_PWM10_PINSET,
#endif
#if defined(TIM11_BASE)
    CONFIG_PWM11_PINSET,
#endif
#if defined(TIM12_BASE)
    CONFIG_PWM12_PINSET,
#endif
#if defined(TIM13_BASE)
    CONFIG_PWM13_PINSET,
#endif
#if defined(TIM14_BASE)
    CONFIG_PWM14_PINSET,
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
            { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_B13, TIM_CHANNEL_1, 1 },
            { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_B14, TIM_CHANNEL_2, 1 },
            { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_B15, TIM_CHANNEL_3, 1 },
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
            { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A5,  TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_6, hwTimer_Index_1, hwGPIO_Pin_B3,  TIM_CHANNEL_2, 0 },
            { hwPWM_Channel_7, hwTimer_Index_1, hwGPIO_Pin_B10, TIM_CHANNEL_3, 0 },
            { hwPWM_Channel_8, hwTimer_Index_1, hwGPIO_Pin_B11, TIM_CHANNEL_4, 0 },
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
            { hwPWM_Channel_9,  hwTimer_Index_2, hwGPIO_Pin_C6, TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_10, hwTimer_Index_2, hwGPIO_Pin_C7, TIM_CHANNEL_2, 0 },
            { hwPWM_Channel_11, hwTimer_Index_2, hwGPIO_Pin_C8, TIM_CHANNEL_3, 0 },
            { hwPWM_Channel_12, hwTimer_Index_2, hwGPIO_Pin_C9, TIM_CHANNEL_4, 0 },
        },
        {
            { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
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
            { hwPWM_Channel_21, hwTimer_Index_7, hwGPIO_Pin_A5, TIM_CHANNEL_1, 1 },
            { hwPWM_Channel_22, hwTimer_Index_7, hwGPIO_Pin_B0, TIM_CHANNEL_2, 1 },
            { hwPWM_Channel_23, hwTimer_Index_7, hwGPIO_Pin_B1, TIM_CHANNEL_3, 1 },
            { hwPWM_Channel_MAX, hwTimer_Index_7, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_MAX, hwTimer_Index_7, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_7, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_7, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_7, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif
#if defined(TIM15_BASE)
    {
        {
            { hwPWM_Channel_33, hwTimer_Index_14, hwGPIO_Pin_A2, TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_34, hwTimer_Index_14, hwGPIO_Pin_A3, TIM_CHANNEL_2, 0 },
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
            { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
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
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif
};

static const PWM_AF_Map PWM_Pin_AF_Map[] =
{
#if defined(TIM1_BASE)
    { hwTimer_Index_0, hwGPIO_Pin_A8,  GPIO_AF6_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A9,  GPIO_AF6_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A10, GPIO_AF6_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A11, GPIO_AF6_TIM1 },
#endif

#if defined(TIM1_BASE)
    { hwTimer_Index_0, hwGPIO_Pin_E9,  GPIO_AF2_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_E11, GPIO_AF2_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_E13, GPIO_AF2_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_E14, GPIO_AF2_TIM1 },
#endif

#if defined(TIM1_BASE)
    { hwTimer_Index_0, hwGPIO_Pin_B13, GPIO_AF6_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_B14, GPIO_AF6_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_B15, GPIO_AF4_TIM1 },
#endif

#if defined(TIM2_BASE)
    { hwTimer_Index_1, hwGPIO_Pin_A0,  GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A1,  GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A2,  GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A3,  GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A5,  GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_B3,  GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_B10, GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_B11, GPIO_AF1_TIM2 },
#endif

#if defined(TIM3_BASE)
    { hwTimer_Index_2, hwGPIO_Pin_A6, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_A7, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_B0, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_B1, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C6, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C7, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C8, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C9, GPIO_AF2_TIM3 },
#endif

#if defined(TIM4_BASE)
    { hwTimer_Index_3, hwGPIO_Pin_B6,  GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_B7,  GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_B8,  GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_B9,  GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_D12, GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_D13, GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_D14, GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_D15, GPIO_AF2_TIM4 },
#endif

#if defined(TIM5_BASE)
    { hwTimer_Index_4, hwGPIO_Pin_A0, GPIO_AF2_TIM5 },
    { hwTimer_Index_4, hwGPIO_Pin_A1, GPIO_AF2_TIM5 },
    { hwTimer_Index_4, hwGPIO_Pin_A2, GPIO_AF2_TIM5 },
    { hwTimer_Index_4, hwGPIO_Pin_A3, GPIO_AF2_TIM5 },
#endif

#if defined(TIM8_BASE)
    { hwTimer_Index_7, hwGPIO_Pin_C6, GPIO_AF4_TIM8 },
    { hwTimer_Index_7, hwGPIO_Pin_C7, GPIO_AF4_TIM8 },
    { hwTimer_Index_7, hwGPIO_Pin_C8, GPIO_AF4_TIM8 },
    { hwTimer_Index_7, hwGPIO_Pin_C9, GPIO_AF4_TIM8 },
#endif

#if defined(TIM8_BASE)
    { hwTimer_Index_7, hwGPIO_Pin_A5, GPIO_AF5_TIM8 },
    { hwTimer_Index_7, hwGPIO_Pin_B0, GPIO_AF5_TIM8 },
    { hwTimer_Index_7, hwGPIO_Pin_B1, GPIO_AF5_TIM8 },
#endif

#if defined(TIM15_BASE)
    { hwTimer_Index_14, hwGPIO_Pin_A2,  GPIO_AF9_TIM15 },
    { hwTimer_Index_14, hwGPIO_Pin_A3,  GPIO_AF9_TIM15 },
    { hwTimer_Index_14, hwGPIO_Pin_B14, GPIO_AF1_TIM15 },
    { hwTimer_Index_14, hwGPIO_Pin_B15, GPIO_AF2_TIM15 },
#endif

#if defined(TIM16_BASE)
    { hwTimer_Index_15, hwGPIO_Pin_A6, GPIO_AF1_TIM16 },
    { hwTimer_Index_15, hwGPIO_Pin_B8, GPIO_AF1_TIM16 },
#endif

#if defined(TIM17_BASE)
    { hwTimer_Index_16, hwGPIO_Pin_A7, GPIO_AF1_TIM17 },
    { hwTimer_Index_16, hwGPIO_Pin_B9, GPIO_AF1_TIM17 },
#endif
};

#endif // PWM_PIN_STM32F3_H