
#ifndef PWM_Pin_STM32H7RS
#define PWM_Pin_STM32H7RS

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
#endif //CONFIG_PWM1_PINSET

#ifndef CONFIG_PWM2_PINSET
#define CONFIG_PWM2_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM2_PINSET

#ifndef CONFIG_PWM3_PINSET
#define CONFIG_PWM3_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM3_PINSET

#ifndef CONFIG_PWM4_PINSET
#define CONFIG_PWM4_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM4_PINSET

#ifndef CONFIG_PWM5_PINSET
#define CONFIG_PWM5_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM5_PINSET

#ifndef CONFIG_PWM12_PINSET
#define CONFIG_PWM12_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM12_PINSET

#ifndef CONFIG_PWM13_PINSET
#define CONFIG_PWM13_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM13_PINSET

#ifndef CONFIG_PWM14_PINSET
#define CONFIG_PWM14_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM14_PINSET

#ifndef CONFIG_PWM15_PINSET
#define CONFIG_PWM15_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM15_PINSET

#ifndef CONFIG_PWM16_PINSET
#define CONFIG_PWM16_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM16_PINSET

#ifndef CONFIG_PWM17_PINSET
#define CONFIG_PWM17_PINSET PWM_Pinset_DEFAULT
#endif //CONFIG_PWM17_PINSET

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
    /* ================= TIM1 ================= */
    {
        {
            { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_A8,  1, 0 },
            { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_A9,  2, 0 },
            { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_A10, 3, 0 },
            { hwPWM_Channel_4, hwTimer_Index_0, hwGPIO_Pin_A11, 4, 0 },
        },
        {
            { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_E9,  1, 0 },
            { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_E11, 2, 0 },
            { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_E13, 3, 0 },
            { hwPWM_Channel_4, hwTimer_Index_0, hwGPIO_Pin_E14, 4, 0 },
        },
        {
            { hwPWM_Channel_1, hwTimer_Index_0, hwGPIO_Pin_B13, 1, 1 },
            { hwPWM_Channel_2, hwTimer_Index_0, hwGPIO_Pin_B14, 2, 1 },
            { hwPWM_Channel_3, hwTimer_Index_0, hwGPIO_Pin_B15, 3, 1 },
            { hwPWM_Channel_MAX, hwTimer_Index_0, hwGPIO_Pin_NC,  0, 0 },
        },
    },
#endif
#if defined(TIM2_BASE)
    /* ================= TIM2 ================= */
    {
        {
            { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A0, 1, 0 },
            { hwPWM_Channel_6, hwTimer_Index_1, hwGPIO_Pin_A1, 2, 0 },
            { hwPWM_Channel_7, hwTimer_Index_1, hwGPIO_Pin_A2, 3, 0 },
            { hwPWM_Channel_8, hwTimer_Index_1, hwGPIO_Pin_A3, 4, 0 },
        },
        {
            { hwPWM_Channel_5, hwTimer_Index_1, hwGPIO_Pin_A5,  1, 0 },
            { hwPWM_Channel_6, hwTimer_Index_1, hwGPIO_Pin_B3,  2, 0 },
            { hwPWM_Channel_7, hwTimer_Index_1, hwGPIO_Pin_B10, 3, 0 },
            { hwPWM_Channel_8, hwTimer_Index_1, hwGPIO_Pin_B11, 4, 0 },
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
    /* ================= TIM3 ================= */
    {
        {
            { hwPWM_Channel_9,  hwTimer_Index_2, hwGPIO_Pin_A6, 1, 0 },
            { hwPWM_Channel_10, hwTimer_Index_2, hwGPIO_Pin_A7, 2, 0 },
            { hwPWM_Channel_11, hwTimer_Index_2, hwGPIO_Pin_B0, 3, 0 },
            { hwPWM_Channel_12, hwTimer_Index_2, hwGPIO_Pin_B1, 4, 0 },
        },
        {
            { hwPWM_Channel_9,  hwTimer_Index_2, hwGPIO_Pin_C6, 1, 0 },
            { hwPWM_Channel_10, hwTimer_Index_2, hwGPIO_Pin_C7, 2, 0 },
            { hwPWM_Channel_11, hwTimer_Index_2, hwGPIO_Pin_C8, 3, 0 },
            { hwPWM_Channel_12, hwTimer_Index_2, hwGPIO_Pin_C9, 4, 0 },
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
    /* ================= TIM4 ================= */
    {
        {
            { hwPWM_Channel_13, hwTimer_Index_3, hwGPIO_Pin_B6, 1, 0 },
            { hwPWM_Channel_14, hwTimer_Index_3, hwGPIO_Pin_B7, 2, 0 },
            { hwPWM_Channel_15, hwTimer_Index_3, hwGPIO_Pin_B8, 3, 0 },
            { hwPWM_Channel_16, hwTimer_Index_3, hwGPIO_Pin_B9, 4, 0 },
        },
        {
            { hwPWM_Channel_13, hwTimer_Index_3, hwGPIO_Pin_D12, 1, 0 },
            { hwPWM_Channel_14, hwTimer_Index_3, hwGPIO_Pin_D13, 2, 0 },
            { hwPWM_Channel_15, hwTimer_Index_3, hwGPIO_Pin_D14, 3, 0 },
            { hwPWM_Channel_16, hwTimer_Index_3, hwGPIO_Pin_D15, 4, 0 },
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
    /* ================= TIM5 ================= */
    {
        {
            { hwPWM_Channel_17, hwTimer_Index_4, hwGPIO_Pin_A0, 1, 0 },
            { hwPWM_Channel_18, hwTimer_Index_4, hwGPIO_Pin_A1, 2, 0 },
            { hwPWM_Channel_19, hwTimer_Index_4, hwGPIO_Pin_A2, 3, 0 },
            { hwPWM_Channel_20, hwTimer_Index_4, hwGPIO_Pin_A3, 4, 0 },
        },
        {
            { hwPWM_Channel_17, hwTimer_Index_4, hwGPIO_Pin_D12, 1, 0 },
            { hwPWM_Channel_18, hwTimer_Index_4, hwGPIO_Pin_D13, 2, 0 },
            { hwPWM_Channel_19, hwTimer_Index_4, hwGPIO_Pin_D14, 3, 0 },
            { hwPWM_Channel_20, hwTimer_Index_4, hwGPIO_Pin_D15, 4, 0 },
        },
        {
            { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_4, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif
#if defined(TIM12_BASE)
    /* ================= TIM12 ================= */
    {
        {
            { hwPWM_Channel_29,  hwTimer_Index_11, hwGPIO_Pin_B14, 1, 0 },
            { hwPWM_Channel_30,  hwTimer_Index_11, hwGPIO_Pin_B15, 2, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC,  0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC,  0, 0 },
        },
        {
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_11, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif
#if defined(TIM13_BASE)
    /* ================= TIM13 ================= */
    {
        {
            { hwPWM_Channel_31,  hwTimer_Index_12, hwGPIO_Pin_F8, 1, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_12, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif
#if defined(TIM14_BASE)
    /* ================= TIM14 ================= */
    {
        {
            { hwPWM_Channel_32,  hwTimer_Index_13, hwGPIO_Pin_F9, 1, 0 },
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
        {
            { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_13, hwGPIO_Pin_NC, 0, 0 },
        },
    },
#endif
#if defined(TIM15_BASE)
    /* ================= TIM15 ================= */
    {
        {
            { hwPWM_Channel_33, hwTimer_Index_14, hwGPIO_Pin_A2,  1, 0 },
            { hwPWM_Channel_34, hwTimer_Index_14, hwGPIO_Pin_A3,  2, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_14, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_33, hwTimer_Index_14, hwGPIO_Pin_B14, 1, 0 },
            { hwPWM_Channel_34, hwTimer_Index_14, hwGPIO_Pin_B15, 2, 0 },
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
    /* ================= TIM16 ================= */
    {
        {
            { hwPWM_Channel_35, hwTimer_Index_15, hwGPIO_Pin_A6,  1, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_15, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_35, hwTimer_Index_15, hwGPIO_Pin_B8,  1, 0 },
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
    /* ================= TIM17 ================= */
    {
        {
            { hwPWM_Channel_36, hwTimer_Index_16, hwGPIO_Pin_A7,  1, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
            { hwPWM_Channel_MAX, hwTimer_Index_16, hwGPIO_Pin_NC, 0, 0 },
        },
        {
            { hwPWM_Channel_36, hwTimer_Index_16, hwGPIO_Pin_B9,  1, 0 },
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
    /* TIM1 */
    { hwTimer_Index_0, hwGPIO_Pin_A8,  GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A9,  GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A10, GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_A11, GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_E9,  GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_E11, GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_E13, GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_E14, GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_B13, GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_B14, GPIO_AF1_TIM1 },
    { hwTimer_Index_0, hwGPIO_Pin_B15, GPIO_AF1_TIM1 },
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

#if defined(TIM12_BASE)
    { hwTimer_Index_11, hwGPIO_Pin_B14, GPIO_AF2_TIM12 },
    { hwTimer_Index_11, hwGPIO_Pin_B15, GPIO_AF2_TIM12 },
#endif

#if defined(TIM15_BASE)
    { hwTimer_Index_14, hwGPIO_Pin_A2,  GPIO_AF4_TIM15 },
    { hwTimer_Index_14, hwGPIO_Pin_A3,  GPIO_AF4_TIM15 },
    { hwTimer_Index_14, hwGPIO_Pin_B14, GPIO_AF4_TIM15 },
    { hwTimer_Index_14, hwGPIO_Pin_B15, GPIO_AF4_TIM15 },
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

#endif //PWM_Pin_STM32H7RS