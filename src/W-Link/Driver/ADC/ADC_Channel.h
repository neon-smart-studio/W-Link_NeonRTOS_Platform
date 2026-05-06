
#ifndef ADC_CHANNEL_H
#define ADC_CHANNEL_H

#ifdef DEVICE_STM32
typedef enum hwADC_Channel_Index_t
{
#ifdef ADC_CHANNEL_0
  hwADC_Channel_Index_0,
#endif
#ifdef ADC_CHANNEL_1
  hwADC_Channel_Index_1,
#endif
#ifdef ADC_CHANNEL_2
  hwADC_Channel_Index_2,
#endif
#ifdef ADC_CHANNEL_3
  hwADC_Channel_Index_3,
#endif
#ifdef ADC_CHANNEL_4
  hwADC_Channel_Index_4,
#endif
#ifdef ADC_CHANNEL_5
  hwADC_Channel_Index_5,
#endif
#ifdef ADC_CHANNEL_6
  hwADC_Channel_Index_6,
#endif
#ifdef ADC_CHANNEL_7
  hwADC_Channel_Index_7,
#endif
#ifdef ADC_CHANNEL_8
  hwADC_Channel_Index_8,
#endif
#ifdef ADC_CHANNEL_9
  hwADC_Channel_Index_9,
#endif
#ifdef ADC_CHANNEL_10
  hwADC_Channel_Index_10,
#endif
#ifdef ADC_CHANNEL_11
  hwADC_Channel_Index_11,
#endif
#ifdef ADC_CHANNEL_12
  hwADC_Channel_Index_12,
#endif
#ifdef ADC_CHANNEL_13
  hwADC_Channel_Index_13,
#endif
#ifdef ADC_CHANNEL_14
  hwADC_Channel_Index_14,
#endif
#ifdef ADC_CHANNEL_15
  hwADC_Channel_Index_15,
#endif
#ifdef ADC_CHANNEL_16
  hwADC_Channel_Index_16,
#endif
#ifdef ADC_CHANNEL_17
  hwADC_Channel_Index_17,
#endif
#ifdef ADC_CHANNEL_18
  hwADC_Channel_Index_18,
#endif
#ifdef ADC_CHANNEL_19
  hwADC_Channel_Index_19,
#endif
#ifdef ADC_CHANNEL_20
  hwADC_Channel_Index_20,
#endif
#ifdef ADC_CHANNEL_21
  hwADC_Channel_Index_21,
#endif
#ifdef ADC_CHANNEL_22
  hwADC_Channel_Index_22,
#endif
#ifdef ADC_CHANNEL_23
  hwADC_Channel_Index_23,
#endif
#ifdef ADC_CHANNEL_24
  hwADC_Channel_Index_24,
#endif
#ifdef ADC_CHANNEL_25
  hwADC_Channel_Index_25,
#endif
#ifdef ADC_CHANNEL_26
  hwADC_Channel_Index_26,
#endif
#ifdef ADC_CHANNEL_27
  hwADC_Channel_Index_27,
#endif
#ifdef ADC_CHANNEL_28
  hwADC_Channel_Index_28,
#endif
#ifdef ADC_CHANNEL_29
  hwADC_Channel_Index_29,
#endif
#ifdef ADC_CHANNEL_30
  hwADC_Channel_Index_30,
#endif
#ifdef ADC_CHANNEL_31
  hwADC_Channel_Index_31,
#endif
  hwADC_Channel_Index_MAX,
}hwADC_Channel_Index;
#endif // DEVICE_STM32

#ifdef DEVICE_RP2
typedef enum hwADC_Channel_Index_t
{
  hwADC_Channel_Index_0,
  hwADC_Channel_Index_1,
  hwADC_Channel_Index_2,
  hwADC_Channel_Index_3,
  hwADC_Channel_Index_MAX,
}hwADC_Channel_Index;
#endif // DEVICE_RP2

#endif //ADC_CHANNEL_H