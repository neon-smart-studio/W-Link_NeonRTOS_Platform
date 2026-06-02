#ifndef PWM_PIN_STM32WL_H
#define PWM_PIN_STM32WL_H

#include "PWM_Pin_STM32_Def.h"

#include "PWM/Device/STM32/PWM_STM32_Base.h"

typedef enum {
    PWM_Pinset_DEFAULT = 0,
    PWM_Pinset_ALT1,
    PWM_Pinset_MAX
} PWM_Pinset_t;

#ifndef CONFIG_PWM1_PINSET
#define CONFIG_PWM1_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM2_PINSET
#define CONFIG_PWM2_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM4_PINSET
#define CONFIG_PWM4_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM5_PINSET
#define CONFIG_PWM5_PINSET PWM_Pinset_DEFAULT
#endif

static const PWM_Pinset_t PWM_Index_Map_Alt[hwPWM_Base_MAX] = {
#if defined(TIM1_BASE)
    CONFIG_PWM1_PINSET,
#endif
#if defined(TIM2_BASE)
    CONFIG_PWM2_PINSET,
#endif
#if defined(TIM16_BASE)
    CONFIG_PWM4_PINSET,
#endif
#if defined(TIM17_BASE)
    CONFIG_PWM5_PINSET,
#endif
};

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Base_MAX][PWM_Pinset_MAX][4] =
{
#if defined(TIM1_BASE)
    {
        {
            { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_A8,  1, 0 },
            { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_A9,  2, 0 },
            { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_A10, 3, 0 },
            { hwPWM_Channel_4, hwTimer_Index_0, hwGPIO_Pin_A11, 4, 0 },
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
            { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A0, 1, 0 },
            { hwPWM_Channel_6, hwTimer_Index_1, hwGPIO_Pin_A1, 2, 0 },
            { hwPWM_Channel_7, hwTimer_Index_1, hwGPIO_Pin_A2, 3, 0 },
            { hwPWM_Channel_8, hwTimer_Index_1, hwGPIO_Pin_A3, 4, 0 },
        },
        {
            { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A5, 1, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_1, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif

#if defined(TIM16_BASE)
    {
        {
            { hwPWM_Channel_35, hwTimer_Index_15, hwGPIO_Pin_A6, 1, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_35, hwTimer_Index_15, hwGPIO_Pin_B8, 1, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif

#if defined(TIM17_BASE)
    {
        {
            { hwPWM_Channel_36, hwTimer_Index_16, hwGPIO_Pin_A7, 1, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_36, hwTimer_Index_16, hwGPIO_Pin_B9, 1, 0 },
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
    { hwTimer_Index_0, hwGPIO_Pin_A8,  GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A9,  GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A10, GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A11, GPIO_AF1_TIM1 },
#endif

#if defined(TIM2_BASE)
    { hwTimer_Index_1, hwGPIO_Pin_A0, GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A1, GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A2, GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A3, GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A5, GPIO_AF1_TIM2 },
#endif

#if defined(TIM16_BASE)
    { hwTimer_Index_15, hwGPIO_Pin_A6, GPIO_AF14_TIM16 },
    { hwTimer_Index_15, hwGPIO_Pin_B8, GPIO_AF14_TIM16 },
#endif

#if defined(TIM17_BASE)
    { hwTimer_Index_16, hwGPIO_Pin_A7, GPIO_AF14_TIM17 },
    { hwTimer_Index_16, hwGPIO_Pin_B9, GPIO_AF14_TIM17 },
#endif
};

#endif // PWM_PIN_STM32WL_H