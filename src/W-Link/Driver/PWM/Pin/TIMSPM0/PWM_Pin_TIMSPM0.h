
#ifndef PWM_PIN_TIMSPM0_H
#define PWM_PIN_TIMSPM0_H

#include "PWM_Pin_TIMSPM0_Def.h"

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Channel_MAX] =
{
#if defined(MSPM0C110x)
    { hwPWM_Channel_1,  hwTimer_Index_0,  hwGPIO_Pin_A28, 0 },
    { hwPWM_Channel_2,  hwTimer_Index_0,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_3,  hwTimer_Index_0,  hwGPIO_Pin_A17, 2 },
    { hwPWM_Channel_4,  hwTimer_Index_0,  hwGPIO_Pin_A18, 3 },
    { hwPWM_Channel_5,  hwTimer_Index_12, hwGPIO_Pin_A16, 0 },
    { hwPWM_Channel_6,  hwTimer_Index_12, hwGPIO_Pin_A23, 1 },
    { hwPWM_Channel_7,  hwTimer_Index_12, hwGPIO_Pin_A24, 2 },
    { hwPWM_Channel_8,  hwTimer_Index_12, hwGPIO_Pin_A25, 3 },
#endif

#if defined(MSPM0C1105) || defined(MSPM0C1106)
    { hwPWM_Channel_1,  hwTimer_Index_0,  hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwTimer_Index_0,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_3,  hwTimer_Index_0,  hwGPIO_Pin_A3,  2 },
    { hwPWM_Channel_4,  hwTimer_Index_0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwTimer_Index_3,  hwGPIO_Pin_A7,  0 },
    { hwPWM_Channel_6,  hwTimer_Index_3,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwTimer_Index_4,  hwGPIO_Pin_A9,  0 },
    { hwPWM_Channel_8,  hwTimer_Index_4,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_9,  hwTimer_Index_12, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_10, hwTimer_Index_12, hwGPIO_Pin_A1,  1 },
    { hwPWM_Channel_11, hwTimer_Index_12, hwGPIO_Pin_A16, 2 },
    { hwPWM_Channel_12, hwTimer_Index_12, hwGPIO_Pin_A13, 3 },
#endif

#if defined(MSPM0H321x)
    { hwPWM_Channel_1,  hwTimer_Index_0,  hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwTimer_Index_0,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_3,  hwTimer_Index_0,  hwGPIO_Pin_A3,  2 },
    { hwPWM_Channel_4,  hwTimer_Index_0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwTimer_Index_3,  hwGPIO_Pin_A7,  0 },
    { hwPWM_Channel_6,  hwTimer_Index_3,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwTimer_Index_4,  hwGPIO_Pin_A9,  0 },
    { hwPWM_Channel_8,  hwTimer_Index_4,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_9,  hwTimer_Index_9,  hwGPIO_Pin_A17, 0 },
    { hwPWM_Channel_10, hwTimer_Index_9,  hwGPIO_Pin_A0,  1 },
    { hwPWM_Channel_11, hwTimer_Index_12, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_12, hwTimer_Index_12, hwGPIO_Pin_A1,  1 },
    { hwPWM_Channel_13, hwTimer_Index_12, hwGPIO_Pin_A16, 2 },
    { hwPWM_Channel_14, hwTimer_Index_12, hwGPIO_Pin_A13, 3 },
#endif

#if defined(MSPM0G110x) || defined(MSPM0G150x) || \
      defined(MSPM0G310x) || defined(MSPM0G350x)
    { hwPWM_Channel_1,  hwTimer_Index_0,  hwGPIO_Pin_A8,  0 },
    { hwPWM_Channel_2,  hwTimer_Index_0,  hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwTimer_Index_0,  hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwTimer_Index_0,  hwGPIO_Pin_A4,  3 },
    { hwPWM_Channel_5,  hwTimer_Index_1,  hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_6,  hwTimer_Index_1,  hwGPIO_Pin_A11, 1 },
    { hwPWM_Channel_7,  hwTimer_Index_2,  hwGPIO_Pin_A12, 0 },
    { hwPWM_Channel_8,  hwTimer_Index_2,  hwGPIO_Pin_A13, 1 },
    { hwPWM_Channel_9,  hwTimer_Index_7,  hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_10, hwTimer_Index_7,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_11, hwTimer_Index_8,  hwGPIO_Pin_A17, 0 },
    { hwPWM_Channel_12, hwTimer_Index_8,  hwGPIO_Pin_A2,  1 },
    { hwPWM_Channel_13, hwTimer_Index_9,  hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_14, hwTimer_Index_9,  hwGPIO_Pin_A0,  1 },
    { hwPWM_Channel_15, hwTimer_Index_11, hwGPIO_Pin_A14, 0 },
    { hwPWM_Channel_16, hwTimer_Index_11, hwGPIO_Pin_A25, 1 },
#endif

#if defined(MSPM0G120x) || defined(MSPM0G121x) || \
      defined(MSPM0G320x) || defined(MSPM0G321x)
    { hwPWM_Channel_1,  hwTimer_Index_0, hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwTimer_Index_0, hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwTimer_Index_0, hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwTimer_Index_0, hwGPIO_Pin_A4,  3 },
    { hwPWM_Channel_5,  hwTimer_Index_1, hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_6,  hwTimer_Index_1, hwGPIO_Pin_A11, 1 },
    { hwPWM_Channel_7,  hwTimer_Index_2, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_8,  hwTimer_Index_2, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_9,  hwTimer_Index_3, hwGPIO_Pin_A23, 0 },
    { hwPWM_Channel_10, hwTimer_Index_3, hwGPIO_Pin_A25, 1 },
    { hwPWM_Channel_11, hwTimer_Index_9, hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_12, hwTimer_Index_9, hwGPIO_Pin_A0,  1 },
#endif

#if defined(MSPM0G151x) || defined(MSPM0G351x) || defined(MSPM0G352x)
    { hwPWM_Channel_1,  hwTimer_Index_0,  hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwTimer_Index_0,  hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwTimer_Index_0,  hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwTimer_Index_0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwTimer_Index_1,  hwGPIO_Pin_A15, 0 },
    { hwPWM_Channel_6,  hwTimer_Index_1,  hwGPIO_Pin_A16, 1 },
    { hwPWM_Channel_7,  hwTimer_Index_2,  hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_8,  hwTimer_Index_2,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_9,  hwTimer_Index_7,  hwGPIO_Pin_A21, 0 },
    { hwPWM_Channel_10, hwTimer_Index_7,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_11, hwTimer_Index_8,  hwGPIO_Pin_A17, 0 },
    { hwPWM_Channel_12, hwTimer_Index_8,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_13, hwTimer_Index_9,  hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_14, hwTimer_Index_9,  hwGPIO_Pin_A0,  1 },
    { hwPWM_Channel_15, hwTimer_Index_10, hwGPIO_Pin_B7,  0 },
    { hwPWM_Channel_16, hwTimer_Index_10, hwGPIO_Pin_B9,  1 },
    { hwPWM_Channel_17, hwTimer_Index_11, hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_18, hwTimer_Index_11, hwGPIO_Pin_A11, 1 },
    { hwPWM_Channel_19, hwTimer_Index_12, hwGPIO_Pin_A29, 0 },
    { hwPWM_Channel_20, hwTimer_Index_12, hwGPIO_Pin_A30, 1 },
    { hwPWM_Channel_21, hwTimer_Index_12, hwGPIO_Pin_A8,  2 },
    { hwPWM_Channel_22, hwTimer_Index_12, hwGPIO_Pin_A9,  3 },
#endif

#if defined(MSPM0G511x) || defined(MSPM0G518x)
    { hwPWM_Channel_1,  hwTimer_Index_0, hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwTimer_Index_0, hwGPIO_Pin_A1,  1 },
    { hwPWM_Channel_3,  hwTimer_Index_0, hwGPIO_Pin_A3,  2 },
    { hwPWM_Channel_4,  hwTimer_Index_0, hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwTimer_Index_2, hwGPIO_Pin_A0,  0 },
    { hwPWM_Channel_6,  hwTimer_Index_2, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwTimer_Index_7, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_8,  hwTimer_Index_7, hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_9,  hwTimer_Index_8, hwGPIO_Pin_A14, 0 },
    { hwPWM_Channel_10, hwTimer_Index_8, hwGPIO_Pin_A4,  1 },
#endif

#if defined(MSPM0L110x) || defined(MSPM0L130x) || defined(MSPM0L134x)
    { hwPWM_Channel_1, hwTimer_Index_2, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_2, hwTimer_Index_2, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_3, hwTimer_Index_3, hwGPIO_Pin_A0,  0 },
    { hwPWM_Channel_4, hwTimer_Index_3, hwGPIO_Pin_A1,  1 },
    { hwPWM_Channel_5, hwTimer_Index_4, hwGPIO_Pin_A3,  0 },
    { hwPWM_Channel_6, hwTimer_Index_4, hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_7, hwTimer_Index_5, hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_8, hwTimer_Index_5, hwGPIO_Pin_A11, 1 },
#endif

#if defined(MSPM0L111x)
    { hwPWM_Channel_1,  hwTimer_Index_0, hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwTimer_Index_0, hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwTimer_Index_0, hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwTimer_Index_0, hwGPIO_Pin_A4,  3 },
    { hwPWM_Channel_5,  hwTimer_Index_2, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_6,  hwTimer_Index_2, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwTimer_Index_3, hwGPIO_Pin_A15, 0 },
    { hwPWM_Channel_8,  hwTimer_Index_3, hwGPIO_Pin_A16, 1 },
    { hwPWM_Channel_9,  hwTimer_Index_9, hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_10, hwTimer_Index_9, hwGPIO_Pin_A0,  1 },
#endif

#if defined(MSPM0L112x) || defined(MSPM0L211x)
    { hwPWM_Channel_1,  hwTimer_Index_0,  hwGPIO_Pin_A0,  0 },
    { hwPWM_Channel_2,  hwTimer_Index_0,  hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwTimer_Index_0,  hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwTimer_Index_0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwTimer_Index_3,  hwGPIO_Pin_A21, 0 },
    { hwPWM_Channel_6,  hwTimer_Index_3,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_7,  hwTimer_Index_4,  hwGPIO_Pin_A9,  0 },
    { hwPWM_Channel_8,  hwTimer_Index_4,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_9,  hwTimer_Index_12, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_10, hwTimer_Index_12, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_11, hwTimer_Index_12, hwGPIO_Pin_A1,  2 },
    { hwPWM_Channel_12, hwTimer_Index_12, hwGPIO_Pin_A2,  3 },
#endif

#if defined(MSPM0L122x) || defined(MSPM0L222x)
    { hwPWM_Channel_1,  hwTimer_Index_0,  hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwTimer_Index_0,  hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwTimer_Index_0,  hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwTimer_Index_0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwTimer_Index_2,  hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_6,  hwTimer_Index_2,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwTimer_Index_5,  hwGPIO_Pin_A21, 0 },
    { hwPWM_Channel_8,  hwTimer_Index_5,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_9,  hwTimer_Index_6,  hwGPIO_Pin_A9,  0 },
    { hwPWM_Channel_10, hwTimer_Index_6,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_11, hwTimer_Index_9,  hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_12, hwTimer_Index_9,  hwGPIO_Pin_A0,  1 },
    { hwPWM_Channel_13, hwTimer_Index_11, hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_14, hwTimer_Index_11, hwGPIO_Pin_A11, 1 },
#endif
};

#endif //PWM_PIN_TIMSPM0_H