#ifndef PWM_PIN_STM32F0_H
#define PWM_PIN_STM32F0_H

#include "PWM_Pin_STM32.h"

typedef enum {
    PWM_Pinset_DEFAULT = 0,
    PWM_Pinset_ALT1,
    PWM_Pinset_MAX
} PWM_Pinset_t;

#ifndef CONFIG_PWM1_PINSET
#define CONFIG_PWM1_PINSET PWM_Pinset_DEFAULT
#endif // CONFIG_PWM1_PINSET

#ifndef CONFIG_PWM2_PINSET
#define CONFIG_PWM2_PINSET PWM_Pinset_DEFAULT
#endif // CONFIG_PWM2_PINSET

#ifndef CONFIG_PWM3_PINSET
#define CONFIG_PWM3_PINSET PWM_Pinset_DEFAULT
#endif // CONFIG_PWM3_PINSET

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
};

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Base_MAX][PWM_Pinset_MAX][4] =
{
#if defined(TIM1_BASE)
    /* ================= TIM1 ================= */
    {
        /* DEFAULT */
        {
            { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_A8,  1, 0 },
            { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_A9,  2, 0 },
            { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_A10, 3, 0 },
            { hwPWM_Channel_4, hwTimer_Index_0, hwGPIO_Pin_A11, 4, 0 },
        },
        /* ALT1 */
        {
            { hwPWM_Channel_MAX, hwTimer_Index_0, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_0, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_0, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_0, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif

#if defined(TIM2_BASE)
    /* ================= TIM2 ================= */
    {
        /* DEFAULT */
        {
            { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A0, 1, 0 },
            { hwPWM_Channel_6, hwTimer_Index_1, hwGPIO_Pin_A1, 2, 0 },
            { hwPWM_Channel_7, hwTimer_Index_1, hwGPIO_Pin_A2, 3, 0 },
            { hwPWM_Channel_8, hwTimer_Index_1, hwGPIO_Pin_A3, 4, 0 },
        },
        /* ALT1 */
        {
            { hwPWM_Channel_5,   hwTimer_Index_1, hwGPIO_Pin_A5,  1, 0 },
            { hwPWM_Channel_6,   hwTimer_Index_1, hwGPIO_Pin_NC,  0, 0 },
            { hwPWM_Channel_7,   hwTimer_Index_1, hwGPIO_Pin_NC,  0, 0 },
            { hwPWM_Channel_8,   hwTimer_Index_1, hwGPIO_Pin_NC,  0, 0 },
        },
    },
#endif

#if defined(TIM3_BASE)
    /* ================= TIM3 ================= */
    {
        /* DEFAULT */
        {
            { hwPWM_Channel_9,  hwTimer_Index_2, hwGPIO_Pin_A6, 1, 0 },
            { hwPWM_Channel_10, hwTimer_Index_2, hwGPIO_Pin_A7, 2, 0 },
            { hwPWM_Channel_11, hwTimer_Index_2, hwGPIO_Pin_B0, 3, 0 },
            { hwPWM_Channel_12, hwTimer_Index_2, hwGPIO_Pin_B1, 4, 0 },
        },
        /* ALT1 */
        {
            { hwPWM_Channel_9,  hwTimer_Index_2, hwGPIO_Pin_C6, 1, 0 },
            { hwPWM_Channel_10, hwTimer_Index_2, hwGPIO_Pin_C7, 2, 0 },
            { hwPWM_Channel_11, hwTimer_Index_2, hwGPIO_Pin_C8, 3, 0 },
            { hwPWM_Channel_12, hwTimer_Index_2, hwGPIO_Pin_C9, 4, 0 },
        },
    },
#endif
};

static const PWM_AF_Map PWM_Pin_AF_Map[] =
{
#if defined(TIM1_BASE)
    /* TIM1 */
    { hwTimer_Index_0, hwGPIO_Pin_A8,  GPIO_AF2_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A9,  GPIO_AF2_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A10, GPIO_AF2_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A11, GPIO_AF2_TIM1 },
#endif

#if defined(TIM2_BASE)
    /* TIM2 */
    { hwTimer_Index_1, hwGPIO_Pin_A0,  GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A1,  GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A2,  GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A3,  GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A5,  GPIO_AF2_TIM2 },
#endif

#if defined(TIM3_BASE)
    /* TIM3 */
    { hwTimer_Index_2, hwGPIO_Pin_A6,  GPIO_AF1_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_A7,  GPIO_AF1_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_B0,  GPIO_AF1_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_B1,  GPIO_AF1_TIM3 },
#if defined (STM32F030x8)
    { hwTimer_Index_2, hwGPIO_Pin_C6,  GPIO_AF0_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C7,  GPIO_AF0_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C8,  GPIO_AF0_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C9,  GPIO_AF0_TIM3 },
#else
    { hwTimer_Index_2, hwGPIO_Pin_C6,  GPIO_AF1_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C7,  GPIO_AF1_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C8,  GPIO_AF1_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_C9,  GPIO_AF1_TIM3 },
#endif
#endif
};

#endif // PWM_PIN_STM32F0_H