#ifndef PWM_PIN_STM32L0_H
#define PWM_PIN_STM32L0_H

#include "PWM_Pin_STM32.h"

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

#ifndef CONFIG_PWM21_PINSET
#define CONFIG_PWM21_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM22_PINSET
#define CONFIG_PWM22_PINSET PWM_Pinset_DEFAULT
#endif

/* ================= Pinset Mapping ================= */

const PWM_Pinset_t PWM_Index_Map_Alt[hwPWM_Base_MAX] = {
#if defined(TIM2_BASE)
    CONFIG_PWM2_PINSET,
#endif
#if defined(TIM3_BASE)
    CONFIG_PWM3_PINSET,
#endif
#if defined(TIM21_BASE)
    CONFIG_PWM21_PINSET,
#endif
#if defined(TIM22_BASE)
    CONFIG_PWM22_PINSET,
#endif
};

/* ================= Pin Table ================= */

const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Base_MAX][PWM_Pinset_MAX][4] =
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
            { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_2, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif

#if defined(TIM21_BASE)
    /* ===== TIM21 ===== */
    {
        {
            { hwPWM_Channel_41, hwTimer_Index_20, hwGPIO_Pin_A2,  TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_42, hwTimer_Index_20, hwGPIO_Pin_A3,  TIM_CHANNEL_2, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_20, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_20, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_41, hwTimer_Index_20, hwGPIO_Pin_B13, TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_42, hwTimer_Index_20, hwGPIO_Pin_B14, TIM_CHANNEL_2, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_20, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_20, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif

#if defined(TIM22_BASE)
    /* ===== TIM22 ===== */
    {
        {
            { hwPWM_Channel_43, hwTimer_Index_21, hwGPIO_Pin_A6, TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_44, hwTimer_Index_21, hwGPIO_Pin_A7, TIM_CHANNEL_2, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_21, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_21, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_43, hwTimer_Index_21, hwGPIO_Pin_B4, TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_44, hwTimer_Index_21, hwGPIO_Pin_B5, TIM_CHANNEL_2, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_21, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_21, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif

};

/* ================= AF Map ================= */

const PWM_AF_Map PWM_Pin_AF_Map[] =
{
#if defined(TIM2_BASE) && defined(GPIO_AF2_TIM2)
    { hwTimer_Index_1, hwGPIO_Pin_A0, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A1, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A2, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A3, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A5, GPIO_AF2_TIM2 },
#endif

#if defined(TIM3_BASE) && defined(GPIO_AF2_TIM3)
    { hwTimer_Index_2, hwGPIO_Pin_A6, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_A7, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_B0, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_B1, GPIO_AF2_TIM3 },
#endif

#if defined(TIM21_BASE) && defined(GPIO_AF0_TIM21)
    { hwTimer_Index_20, hwGPIO_Pin_A2,  GPIO_AF0_TIM21 },
    { hwTimer_Index_20, hwGPIO_Pin_A3,  GPIO_AF0_TIM21 },
    { hwTimer_Index_20, hwGPIO_Pin_B13, GPIO_AF0_TIM21 },
    { hwTimer_Index_20, hwGPIO_Pin_B14, GPIO_AF0_TIM21 },
#endif

#if defined(TIM22_BASE) && defined(GPIO_AF0_TIM22)
    { hwTimer_Index_21, hwGPIO_Pin_A6, GPIO_AF0_TIM22 },
    { hwTimer_Index_21, hwGPIO_Pin_A7, GPIO_AF0_TIM22 },
    { hwTimer_Index_21, hwGPIO_Pin_B4, GPIO_AF0_TIM22 },
    { hwTimer_Index_21, hwGPIO_Pin_B5, GPIO_AF0_TIM22 },
#endif
};

#endif /* PWM_PIN_STM32L0_H */