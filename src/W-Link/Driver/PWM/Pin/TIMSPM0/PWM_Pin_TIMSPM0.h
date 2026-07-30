
#ifndef PWM_PIN_TIMSPM0_H
#define PWM_PIN_TIMSPM0_H

#include "PWM_Pin_TIMSPM0_Def.h"

static const PWM_Pin_Def PWM_Pin_Def_Table[hwPWM_Channel_MAX] =
{
#if defined(MSPM0C110x)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A28, 0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A17, 2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A18, 3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMG14, hwGPIO_Pin_A16, 0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMG14, hwGPIO_Pin_A23, 1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG14, hwGPIO_Pin_A24, 2 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG14, hwGPIO_Pin_A25, 3 },
#endif

#if defined(MSPM0C1105) || defined(MSPM0C1106)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A3,  2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMG1,  hwGPIO_Pin_A7,  0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMG1,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG2,  hwGPIO_Pin_A9,  0 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG2,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_9, hwPWM_Base_TIMG14, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_10, hwPWM_Base_TIMG14, hwGPIO_Pin_A1,  1 },
    { hwPWM_Channel_11, hwPWM_Base_TIMG14, hwGPIO_Pin_A16, 2 },
    { hwPWM_Channel_12, hwPWM_Base_TIMG14, hwGPIO_Pin_A13, 3 },
#endif

#if defined(MSPM0H321x)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A3,  2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMG1,  hwGPIO_Pin_A7,  0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMG1,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG2,  hwGPIO_Pin_A9,  0 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG2,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_9,  hwPWM_Base_TIMG8,  hwGPIO_Pin_A17, 0 },
    { hwPWM_Channel_10, hwPWM_Base_TIMG8,  hwGPIO_Pin_A0,  1 },
    { hwPWM_Channel_11, hwPWM_Base_TIMG14, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_12, hwPWM_Base_TIMG14, hwGPIO_Pin_A1,  1 },
    { hwPWM_Channel_13, hwPWM_Base_TIMG14, hwGPIO_Pin_A16, 2 },
    { hwPWM_Channel_14, hwPWM_Base_TIMG14, hwGPIO_Pin_A13, 3 },
#endif

#if defined(MSPM0G110x) || defined(MSPM0G150x) || \
      defined(MSPM0G310x) || defined(MSPM0G350x)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A8,  0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A4,  3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMA1,  hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMA1,  hwGPIO_Pin_A11, 1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG0,  hwGPIO_Pin_A12, 0 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG0,  hwGPIO_Pin_A13, 1 },
    { hwPWM_Channel_9,  hwPWM_Base_TIMG6,  hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_10, hwPWM_Base_TIMG6,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_11, hwPWM_Base_TIMG7,  hwGPIO_Pin_A17, 0 },
    { hwPWM_Channel_12, hwPWM_Base_TIMG7,  hwGPIO_Pin_A2,  1 },
    { hwPWM_Channel_13, hwPWM_Base_TIMG8,  hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_14, hwPWM_Base_TIMG8,  hwGPIO_Pin_A0,  1 },
    { hwPWM_Channel_15, hwPWM_Base_TIMG12, hwGPIO_Pin_A14, 0 },
    { hwPWM_Channel_16, hwPWM_Base_TIMG12, hwGPIO_Pin_A25, 1 },
#endif

#if defined(MSPM0G120x) || defined(MSPM0G121x) || \
      defined(MSPM0G320x) || defined(MSPM0G321x)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0, hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0, hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0, hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0, hwGPIO_Pin_A4,  3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMA1, hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMA1, hwGPIO_Pin_A11, 1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG0, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG0, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_9,  hwPWM_Base_TIMG1, hwGPIO_Pin_A23, 0 },
    { hwPWM_Channel_10, hwPWM_Base_TIMG1, hwGPIO_Pin_A25, 1 },
    { hwPWM_Channel_11, hwPWM_Base_TIMG8, hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_12, hwPWM_Base_TIMG8, hwGPIO_Pin_A0,  1 },
#endif

#if defined(MSPM0G151x) || defined(MSPM0G351x) || defined(MSPM0G352x)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMA1,  hwGPIO_Pin_A15, 0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMA1,  hwGPIO_Pin_A16, 1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG0,  hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG0,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_9,  hwPWM_Base_TIMG6,  hwGPIO_Pin_A21, 0 },
    { hwPWM_Channel_10, hwPWM_Base_TIMG6,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_11, hwPWM_Base_TIMG7,  hwGPIO_Pin_A17, 0 },
    { hwPWM_Channel_12, hwPWM_Base_TIMG7,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_13, hwPWM_Base_TIMG8,  hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_14, hwPWM_Base_TIMG8,  hwGPIO_Pin_A0,  1 },
    { hwPWM_Channel_15, hwPWM_Base_TIMG9,  hwGPIO_Pin_B7,  0 },
    { hwPWM_Channel_16, hwPWM_Base_TIMG9,  hwGPIO_Pin_B9,  1 },
    { hwPWM_Channel_17, hwPWM_Base_TIMG12, hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_18, hwPWM_Base_TIMG12, hwGPIO_Pin_A11, 1 },
    { hwPWM_Channel_19, hwPWM_Base_TIMG14, hwGPIO_Pin_A29, 0 },
    { hwPWM_Channel_20, hwPWM_Base_TIMG14, hwGPIO_Pin_A30, 1 },
    { hwPWM_Channel_21, hwPWM_Base_TIMG14, hwGPIO_Pin_A8,  2 },
    { hwPWM_Channel_22, hwPWM_Base_TIMG14, hwGPIO_Pin_A9,  3 },
#endif

#if defined(MSPM0G511x) || defined(MSPM0G518x)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0, hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0, hwGPIO_Pin_A1,  1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0, hwGPIO_Pin_A3,  2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0, hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMG0, hwGPIO_Pin_A0,  0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMG0, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG6, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG6, hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_9,  hwPWM_Base_TIMG7, hwGPIO_Pin_A14, 0 },
    { hwPWM_Channel_10, hwPWM_Base_TIMG7, hwGPIO_Pin_A4,  1 },
#endif

#if defined(MSPM0L110x) || defined(MSPM0L130x) || defined(MSPM0L134x)
    { hwPWM_Channel_1, hwPWM_Base_TIMG0, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_2, hwPWM_Base_TIMG0, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_3, hwPWM_Base_TIMG1, hwGPIO_Pin_A0,  0 },
    { hwPWM_Channel_4, hwPWM_Base_TIMG1, hwGPIO_Pin_A1,  1 },
    { hwPWM_Channel_5, hwPWM_Base_TIMG2, hwGPIO_Pin_A3,  0 },
    { hwPWM_Channel_6, hwPWM_Base_TIMG2, hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_7, hwPWM_Base_TIMG4, hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_8, hwPWM_Base_TIMG4, hwGPIO_Pin_A11, 1 },
#endif

#if defined(MSPM0L111x)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0, hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0, hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0, hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0, hwGPIO_Pin_A4,  3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMG0, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMG0, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG1, hwGPIO_Pin_A15, 0 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG1, hwGPIO_Pin_A16, 1 },
    { hwPWM_Channel_9,  hwPWM_Base_TIMG8, hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_10, hwPWM_Base_TIMG8, hwGPIO_Pin_A0,  1 },
#endif

#if defined(MSPM0L112x) || defined(MSPM0L211x)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A0,  0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMG1,  hwGPIO_Pin_A21, 0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMG1,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG2,  hwGPIO_Pin_A9,  0 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG2,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_9,  hwPWM_Base_TIMG14, hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_10, hwPWM_Base_TIMG14, hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_11, hwPWM_Base_TIMG14, hwGPIO_Pin_A1,  2 },
    { hwPWM_Channel_12, hwPWM_Base_TIMG14, hwGPIO_Pin_A2,  3 },
#endif

#if defined(MSPM0L122x) || defined(MSPM0L222x)
    { hwPWM_Channel_1,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A2,  0 },
    { hwPWM_Channel_2,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A3,  1 },
    { hwPWM_Channel_3,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A7,  2 },
    { hwPWM_Channel_4,  hwPWM_Base_TIMA0,  hwGPIO_Pin_A12, 3 },
    { hwPWM_Channel_5,  hwPWM_Base_TIMG0,  hwGPIO_Pin_A5,  0 },
    { hwPWM_Channel_6,  hwPWM_Base_TIMG0,  hwGPIO_Pin_A6,  1 },
    { hwPWM_Channel_7,  hwPWM_Base_TIMG4,  hwGPIO_Pin_A21, 0 },
    { hwPWM_Channel_8,  hwPWM_Base_TIMG4,  hwGPIO_Pin_A22, 1 },
    { hwPWM_Channel_9,  hwPWM_Base_TIMG5,  hwGPIO_Pin_A9,  0 },
    { hwPWM_Channel_10, hwPWM_Base_TIMG5,  hwGPIO_Pin_A4,  1 },
    { hwPWM_Channel_11, hwPWM_Base_TIMG8,  hwGPIO_Pin_A1,  0 },
    { hwPWM_Channel_12, hwPWM_Base_TIMG8,  hwGPIO_Pin_A0,  1 },
    { hwPWM_Channel_13, hwPWM_Base_TIMG12, hwGPIO_Pin_A10, 0 },
    { hwPWM_Channel_14, hwPWM_Base_TIMG12, hwGPIO_Pin_A11, 1 },
#endif
};

#endif //PWM_PIN_TIMSPM0_H