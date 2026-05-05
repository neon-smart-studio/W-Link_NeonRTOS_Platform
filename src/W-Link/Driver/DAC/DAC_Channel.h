
#ifndef DAC_CHANNEL_H
#define DAC_CHANNEL_H

#ifdef DEVICE_STM32
typedef enum hwDAC_Channel_Index_t
{
#if defined(DAC1_BASE) || defined(DAC_BASE)
#if defined(DAC_CHANNEL_1)
  hwDAC_Channel_Index_0 = 0,
#endif
#if defined(DAC_CHANNEL_2)
  hwDAC_Channel_Index_1,
#endif
#endif
#if defined(DAC2_BASE)
  hwDAC_Channel_Index_2,
#endif
#if defined(DAC3_BASE)
  hwDAC_Channel_Index_3,
#endif
#if defined(DAC4_BASE)
  hwDAC_Channel_Index_4,
#endif
  hwDAC_Channel_Index_MAX,
}hwDAC_Channel_Index;
#endif

#ifdef DEVICE_RPI
typedef enum hwDAC_Channel_Index_t
{
  hwDAC_Channel_Index_MAX = 0,
}hwDAC_Channel_Index;
#endif

#endif //DAC_CHANNEL_H