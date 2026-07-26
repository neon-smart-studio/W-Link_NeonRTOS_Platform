
#ifndef TIMER_INDEX_H
#define TIMER_INDEX_H

#include "soc.h"

#ifdef DEVICE_NUVOTON
typedef enum hwTimer_Index_t
{
#if defined (TIMER0_BASE)
  hwTimer_Index_0 = 0,
#endif
#if defined (TIMER1_BASE)
  hwTimer_Index_1,
#endif
#if defined (TIMER2_BASE)
  hwTimer_Index_2,
#endif
#if defined (TIMER3_BASE)
  hwTimer_Index_3,
#endif
  hwTimer_Index_MAX,
}hwTimer_Index;
#endif // DEVICE_NUVOTON

#ifdef DEVICE_STM32
typedef enum hwTimer_Index_t
{
#if defined (TIM1_BASE)
  hwTimer_Index_0 = 0,
#endif
#if defined (TIM2_BASE)
  hwTimer_Index_1,
#endif
#if defined (TIM3_BASE)
  hwTimer_Index_2,
#endif
#if defined (TIM4_BASE)
  hwTimer_Index_3,
#endif
#if defined (TIM5_BASE)
  hwTimer_Index_4,
#endif
#if defined (TIM6_BASE)
  hwTimer_Index_5,
#endif
#if defined (TIM7_BASE)
  hwTimer_Index_6,
#endif
#if defined (TIM8_BASE)
  hwTimer_Index_7,
#endif
#if defined (TIM9_BASE)
  hwTimer_Index_8,
#endif
#if defined (TIM10_BASE)
  hwTimer_Index_9,
#endif
#if defined (TIM11_BASE)
  hwTimer_Index_10,
#endif
#if defined (TIM12_BASE)
  hwTimer_Index_11,
#endif
#if defined (TIM13_BASE)
  hwTimer_Index_12,
#endif
#if defined (TIM14_BASE)
  hwTimer_Index_13,
#endif
#if defined (TIM15_BASE)
  hwTimer_Index_14,
#endif
#if defined (TIM16_BASE)
  hwTimer_Index_15,
#endif
#if defined (TIM17_BASE)
  hwTimer_Index_16,
#endif
#if defined (TIM18_BASE)
  hwTimer_Index_17,
#endif
#if defined (TIM19_BASE)
  hwTimer_Index_18,
#endif
#if defined (TIM20_BASE)
  hwTimer_Index_19,
#endif
#if defined(TIM21_BASE)
  hwTimer_Index_20,
#endif
#if defined(TIM22_BASE)
  hwTimer_Index_21,
#endif
#if defined(TIM23_BASE)
  hwTimer_Index_22,
#endif
#if defined(TIM24_BASE)
  hwTimer_Index_23,
#endif
  hwTimer_Index_MAX,
}hwTimer_Index;
#endif // DEVICE_STM32

#ifdef DEVICE_RP2
typedef enum hwTimer_Index_t
{
    hwTimer_Index_0 = 0,
    hwTimer_Index_1,
    hwTimer_Index_2,
    hwTimer_Index_3,
#if defined(RP2350)
    hwTimer_Index_4,
    hwTimer_Index_5,
    hwTimer_Index_6,
    hwTimer_Index_7,
#endif
  hwTimer_Index_MAX,
}hwTimer_Index;
#endif // DEVICE_RP2

#ifdef DEVICE_TITIVAC
typedef enum hwTimer_Index_t
{
    hwTimer_Index_0 = 0,
    hwTimer_Index_1,
    hwTimer_Index_2,
    hwTimer_Index_3,
    hwTimer_Index_4,
    hwTimer_Index_5,
    hwTimer_Index_6,
    hwTimer_Index_7,
  hwTimer_Index_MAX,
}hwTimer_Index;
#endif // DEVICE_TITIVAC

#ifdef DEVICE_TIMSP432P
typedef enum {
    hwTimer_Index_0 = 0,   // TIMER32_0
    hwTimer_Index_1,       // TIMER32_1
    hwTimer_Index_MAX
} hwTimer_Index;
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
typedef enum {
    hwTimer_Index_0 = 0,
    hwTimer_Index_1,
    hwTimer_Index_2,
    hwTimer_Index_3,
    hwTimer_Index_4,
    hwTimer_Index_5,
    hwTimer_Index_6,
    hwTimer_Index_7,
    hwTimer_Index_MAX
} hwTimer_Index;
#endif // DEVICE_TIMSP432E

#ifdef DEVICE_TIMSPM0

typedef enum {
#if defined(TIMA0_BASE)
    hwTimer_Index_0 = 0,
#endif
#if defined(TIMA1_BASE)
    hwTimer_Index_1,
#endif
#if defined(TIMG0_BASE)
    hwTimer_Index_2,
#endif
#if defined(TIMG1_BASE)
    hwTimer_Index_3,
#endif
#if defined(TIMG2_BASE)
    hwTimer_Index_4,
#endif
#if defined(TIMG4_BASE)
    hwTimer_Index_5,
#endif
#if defined(TIMG5_BASE)
    hwTimer_Index_6,
#endif
#if defined(TIMG6_BASE)
    hwTimer_Index_7,
#endif
#if defined(TIMG7_BASE)
    hwTimer_Index_8,
#endif
#if defined(TIMG8_BASE)
    hwTimer_Index_9,
#endif
#if defined(TIMG9_BASE)
    hwTimer_Index_10,
#endif
#if defined(TIMG12_BASE)
    hwTimer_Index_11,
#endif
#if defined(TIMG14_BASE)
    hwTimer_Index_12,
#endif

    hwTimer_Index_MAX
} hwTimer_Index;

#endif /* DEVICE_TIMSPM0 */

#endif //TIMER_INDEX_H