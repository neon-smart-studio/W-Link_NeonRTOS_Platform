#ifndef PWM_PIN_STM32L1_H
#define PWM_PIN_STM32L1_H

#include "PWM_Pin_STM32_Def.h"

#include "PWM/Device/STM32/PWM_STM32_Base.h"

typedef enum {
    PWM_Pinset_DEFAULT = 0,
    PWM_Pinset_ALT1,
    PWM_Pinset_MAX
} PWM_Pinset_t;

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

/* ================= Pinset Mapping ================= */

static const PWM_Pinset_t PWM_Index_Map_Alt[hwPWM_Base_MAX] = {
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
};

/* ================= Pin Table ================= */

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Base_MAX][PWM_Pinset_MAX][4] =
{

#if defined(TIM2_BASE)
    /* ===== TIM2 ===== */
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
    /* ===== TIM3 ===== */
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
    },
#endif

#if defined(TIM4_BASE)
    /* ===== TIM4 ===== */
    {
        {
            { hwPWM_Channel_13, hwTimer_Index_3, hwGPIO_Pin_B6, TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_14, hwTimer_Index_3, hwGPIO_Pin_B7, TIM_CHANNEL_2, 0 },
            { hwPWM_Channel_15, hwTimer_Index_3, hwGPIO_Pin_B8, TIM_CHANNEL_3, 0 },
            { hwPWM_Channel_16, hwTimer_Index_3, hwGPIO_Pin_B9, TIM_CHANNEL_4, 0 },
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
    /* ===== TIM5 ===== */
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
    },
#endif

};

/* ================= AF Map ================= */

static const PWM_AF_Map PWM_Pin_AF_Map[] =
{
#if defined(TIM2_BASE) && defined(GPIO_AF1_TIM2)
    { hwTimer_Index_1, hwGPIO_Pin_A0, GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A1, GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A2, GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A3, GPIO_AF1_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A5, GPIO_AF1_TIM2 },
#elif defined(TIM2_BASE) && defined(GPIO_AF2_TIM2)
    { hwTimer_Index_1, hwGPIO_Pin_A0, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A1, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A2, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A3, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A5, GPIO_AF2_TIM2 },
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
    { hwTimer_Index_3, hwGPIO_Pin_B6, GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_B7, GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_B8, GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_B9, GPIO_AF2_TIM4 },
#endif

#if defined(TIM5_BASE)
    { hwTimer_Index_4, hwGPIO_Pin_A0, GPIO_AF2_TIM5 },
    { hwTimer_Index_4, hwGPIO_Pin_A1, GPIO_AF2_TIM5 },
    { hwTimer_Index_4, hwGPIO_Pin_A2, GPIO_AF2_TIM5 },
    { hwTimer_Index_4, hwGPIO_Pin_A3, GPIO_AF2_TIM5 },
#endif
};

#endif /* PWM_PIN_STM32L1_H */