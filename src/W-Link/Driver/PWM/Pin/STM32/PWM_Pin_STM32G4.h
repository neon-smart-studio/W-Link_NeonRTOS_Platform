#ifndef PWM_PIN_STM32G4_H
#define PWM_PIN_STM32G4_H

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

#ifndef CONFIG_PWM3_PINSET
#define CONFIG_PWM3_PINSET PWM_Pinset_DEFAULT
#endif

#ifndef CONFIG_PWM4_PINSET
#define CONFIG_PWM4_PINSET PWM_Pinset_DEFAULT
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

#ifndef CONFIG_PWM20_PINSET
#define CONFIG_PWM20_PINSET PWM_Pinset_DEFAULT
#endif


/* ================= Pinset Mapping ================= */

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
#if defined(TIM20_BASE)
    CONFIG_PWM20_PINSET,
#endif
};


/* ================= Pin Table ================= */

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Base_MAX][PWM_Pinset_MAX][4] =
{

#if defined(TIM1_BASE)
    /* ===== TIM1 ===== */
    {
        {
            { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_A8,  TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_A9,  TIM_CHANNEL_2, 0 },
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
    /* ===== TIM4 (G4新增) ===== */
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


#if defined(TIM8_BASE)
    /* ===== TIM8 (高階PWM) ===== */
    {
        {
            { hwPWM_Channel_21, hwTimer_Index_7, hwGPIO_Pin_C6, TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_22, hwTimer_Index_7, hwGPIO_Pin_C7, TIM_CHANNEL_2, 0 },
            { hwPWM_Channel_23, hwTimer_Index_7, hwGPIO_Pin_C8, TIM_CHANNEL_3, 0 },
            { hwPWM_Channel_24, hwTimer_Index_7, hwGPIO_Pin_C9, TIM_CHANNEL_4, 0 },
        },
        {
            { hwPWM_Channel_MAX, hwTimer_Index_7, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_7, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_7, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_7, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif


#if defined(TIM20_BASE)
    /* ===== TIM20 (G4高階) ===== */
    {
        {
            { hwPWM_Channel_37, hwTimer_Index_19, hwGPIO_Pin_E2, TIM_CHANNEL_1, 0 },
            { hwPWM_Channel_38, hwTimer_Index_19, hwGPIO_Pin_E3, TIM_CHANNEL_2, 0 },
            { hwPWM_Channel_39, hwTimer_Index_19, hwGPIO_Pin_E4, TIM_CHANNEL_3, 0 },
            { hwPWM_Channel_40, hwTimer_Index_19, hwGPIO_Pin_E5, TIM_CHANNEL_4, 0 },
        },
        {
            { hwPWM_Channel_MAX, hwTimer_Index_19, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_19, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_19, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_19, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif

};


/* ================= AF Map ================= */

static const PWM_AF_Map PWM_Pin_AF_Map[] =
{
#if defined(TIM1_BASE)
    { hwTimer_Index_0, hwGPIO_Pin_A8,  GPIO_AF2_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A9,  GPIO_AF2_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A10, GPIO_AF2_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A11, GPIO_AF2_TIM1 },
#endif

#if defined(TIM2_BASE)
    { hwTimer_Index_1, hwGPIO_Pin_A0, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A1, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A2, GPIO_AF2_TIM2 },
    { hwTimer_Index_1, hwGPIO_Pin_A3, GPIO_AF2_TIM2 },
#endif

#if defined(TIM3_BASE)
    { hwTimer_Index_2, hwGPIO_Pin_A6, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_A7, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_B0, GPIO_AF2_TIM3 },
    { hwTimer_Index_2, hwGPIO_Pin_B1, GPIO_AF2_TIM3 },
#endif

#if defined(TIM4_BASE)
    { hwTimer_Index_3, hwGPIO_Pin_B6, GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_B7, GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_B8, GPIO_AF2_TIM4 },
    { hwTimer_Index_3, hwGPIO_Pin_B9, GPIO_AF2_TIM4 },
#endif

#if defined(TIM8_BASE)
    { hwTimer_Index_7, hwGPIO_Pin_C6, GPIO_AF4_TIM8 },
    { hwTimer_Index_7, hwGPIO_Pin_C7, GPIO_AF4_TIM8 },
#endif

#if defined(TIM20_BASE)
    { hwTimer_Index_19, hwGPIO_Pin_E2, GPIO_AF6_TIM20 },
#endif
};

#endif