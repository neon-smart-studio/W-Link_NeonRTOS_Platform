
#ifndef PWM_INDEX_H
#define PWM_INDEX_H

#ifdef DEVICE_STM32
typedef enum
{
#if defined(TIM1_BASE)
	hwPWM_Channel_1 = 0,
	hwPWM_Channel_2,
	hwPWM_Channel_3,
	hwPWM_Channel_4,
#endif
#if defined(TIM2_BASE)
	hwPWM_Channel_5,
	hwPWM_Channel_6,
	hwPWM_Channel_7,
	hwPWM_Channel_8,
#endif
#if defined(TIM3_BASE)
	hwPWM_Channel_9,
	hwPWM_Channel_10,
	hwPWM_Channel_11,
	hwPWM_Channel_12,
#endif
#if defined(TIM4_BASE)
	hwPWM_Channel_13,
	hwPWM_Channel_14,
	hwPWM_Channel_15,
	hwPWM_Channel_16,
#endif
#if defined(TIM5_BASE)
	hwPWM_Channel_17,
	hwPWM_Channel_18,
	hwPWM_Channel_19,
	hwPWM_Channel_20,
#endif
#if defined(TIM8_BASE)
	hwPWM_Channel_21,
	hwPWM_Channel_22,
	hwPWM_Channel_23,
	hwPWM_Channel_24,
#endif
#if defined(TIM9_BASE)
	hwPWM_Channel_25,
	hwPWM_Channel_26,
#endif
#if defined(TIM10_BASE)
	hwPWM_Channel_27,
#endif
#if defined(TIM11_BASE)
	hwPWM_Channel_28,
#endif
#if defined(TIM12_BASE)
	hwPWM_Channel_29,
	hwPWM_Channel_30,
#endif
#if defined(TIM13_BASE)
	hwPWM_Channel_31,
#endif
#if defined(TIM14_BASE)
	hwPWM_Channel_32,
#endif
#if defined(TIM15_BASE)
	hwPWM_Channel_33,
	hwPWM_Channel_34,
#endif
#if defined(TIM16_BASE)
	hwPWM_Channel_35,
#endif
#if defined(TIM17_BASE)
	hwPWM_Channel_36,
#endif
#if defined(TIM20_BASE)
	hwPWM_Channel_37,
	hwPWM_Channel_38,
	hwPWM_Channel_39,
	hwPWM_Channel_40,
#endif
#if defined(TIM21_BASE)
	hwPWM_Channel_41,
	hwPWM_Channel_42,
#endif
#if defined(TIM22_BASE)
	hwPWM_Channel_43,
	hwPWM_Channel_44,
#endif
#if defined(TIM23_BASE)
	hwPWM_Channel_45,
	hwPWM_Channel_46,
	hwPWM_Channel_47,
	hwPWM_Channel_48,
#endif
#if defined(TIM24_BASE)
	hwPWM_Channel_49,
	hwPWM_Channel_50,
	hwPWM_Channel_51,
	hwPWM_Channel_52,
#endif
	hwPWM_Channel_MAX
}hwPWM_Channel;
#endif //DEVICE_STM32

#ifdef DEVICE_RPI
typedef enum hwPWM_Channel_t
{
	hwPWM_Channel_MAX
}hwPWM_Channel;
#endif

#ifdef DEVICE_TM4C1294
#undef DEFAULT_PWM_INDEX
typedef enum
{
	hwPWM_Channel_1 = 0,
	hwPWM_Channel_2,
	hwPWM_Channel_3,
	hwPWM_Channel_4,
	hwPWM_Channel_5,
	hwPWM_Channel_6,
	hwPWM_Channel_7,
	hwPWM_Channel_8,
  	hwPWM_Channel_MAX,
}hwPWM_Channel;
#endif //DEVICE_TM4C1294

#endif //PWM_INDEX_H