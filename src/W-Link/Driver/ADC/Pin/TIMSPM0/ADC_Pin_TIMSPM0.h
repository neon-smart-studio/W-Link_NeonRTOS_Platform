#ifndef ADC_PIN_TIMSPM0_H
#define ADC_PIN_TIMSPM0_H

#include "ADC_Pin_TIMSPM0_Def.h"

static const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] =
{
/* MSPM0C1103 / MSPM0C1104 */
#if defined(MSPM0C110x)
    { hwGPIO_Pin_A27, hwADC_Instance_1, 0 }, /* CH0 */
    { hwGPIO_Pin_A26, hwADC_Instance_1, 1 }, /* CH1 */
    { hwGPIO_Pin_A25, hwADC_Instance_1, 2 }, /* CH2 */
    { hwGPIO_Pin_A24, hwADC_Instance_1, 3 }, /* CH3 */
    { hwGPIO_Pin_A22, hwADC_Instance_1, 4 }, /* CH4 */
    { hwGPIO_Pin_A28, hwADC_Instance_1, 5 }, /* CH5 */
    { hwGPIO_Pin_A20, hwADC_Instance_1, 6 }, /* CH6 */
    { hwGPIO_Pin_A18, hwADC_Instance_1, 7 }, /* CH7 */
    { hwGPIO_Pin_A16, hwADC_Instance_1, 8 }, /* CH8 */
    { hwGPIO_Pin_A17, hwADC_Instance_1, 9 }, /* CH9 */
#endif

/* MSPM0C1105 / MSPM0C1106 and MSPM0H321x */
#if defined(MSPM0C1105) || defined(MSPM0C1106) || defined(MSPM0H321x)
    { hwGPIO_Pin_A27, hwADC_Instance_1, 0 }, /* CH0 */
    { hwGPIO_Pin_A26, hwADC_Instance_1, 1 }, /* CH1 */
    { hwGPIO_Pin_A25, hwADC_Instance_1, 2 }, /* CH2 */
    { hwGPIO_Pin_A24, hwADC_Instance_1, 3 }, /* CH3 */
    { hwGPIO_Pin_A20, hwADC_Instance_1, 4 }, /* CH4 */
    { hwGPIO_Pin_B24, hwADC_Instance_1, 5 }, /* CH5 */
    { hwGPIO_Pin_B20, hwADC_Instance_1, 6 }, /* CH6 */
    { hwGPIO_Pin_A22, hwADC_Instance_1, 7 }, /* CH7 */
    { hwGPIO_Pin_A21, hwADC_Instance_1, 8 }, /* CH8 */
    { hwGPIO_Pin_B19, hwADC_Instance_1, 9 }, /* CH9 */
    { hwGPIO_Pin_B18, hwADC_Instance_1, 10 }, /* CH10 */
    { hwGPIO_Pin_B17, hwADC_Instance_1, 11 }, /* CH11 */
    { hwGPIO_Pin_A18, hwADC_Instance_1, 12 }, /* CH12 */
    { hwGPIO_Pin_A17, hwADC_Instance_1, 13 }, /* CH13 */
    { hwGPIO_Pin_A16, hwADC_Instance_1, 14 }, /* CH14 */
    { hwGPIO_Pin_A15, hwADC_Instance_1, 15 }, /* CH15 */
    { hwGPIO_Pin_A14, hwADC_Instance_1, 16 }, /* CH16 */
    { hwGPIO_Pin_A13, hwADC_Instance_1, 17 }, /* CH17 */
    { hwGPIO_Pin_A12, hwADC_Instance_1, 18 }, /* CH18 */
    { hwGPIO_Pin_B16, hwADC_Instance_1, 19 }, /* CH19 */
    { hwGPIO_Pin_B15, hwADC_Instance_1, 20 }, /* CH20 */
    { hwGPIO_Pin_B14, hwADC_Instance_1, 21 }, /* CH21 */
    { hwGPIO_Pin_A19, hwADC_Instance_1, 22 }, /* CH22 */
    { hwGPIO_Pin_B7, hwADC_Instance_1, 23 }, /* CH23 */
    { hwGPIO_Pin_B6, hwADC_Instance_1, 24 }, /* CH24 */
    { hwGPIO_Pin_A11, hwADC_Instance_1, 25 }, /* CH25 */
    { hwGPIO_Pin_A23, hwADC_Instance_1, 26 }, /* CH26 */
#endif

/*
 * MSPM0G110x / G150x / G350x
 *
 * These families have ADC0 and ADC1.  ADC0_CH8 can select ADC1_CH7
 * internally and ADC1_CH8 can select ADC0_CH7 internally; those are not
 * separate external pins and therefore remain NC here.
 */
#if defined(MSPM0G110x) || defined(MSPM0G150x) || defined(MSPM0G350x)
    { hwGPIO_Pin_A27, hwADC_Instance_1, 0 }, /* CH0 */
    { hwGPIO_Pin_A26, hwADC_Instance_1, 1 }, /* CH1 */
    { hwGPIO_Pin_A25, hwADC_Instance_1, 2 }, /* CH2 */
    { hwGPIO_Pin_A24, hwADC_Instance_1, 3 }, /* CH3 */
    { hwGPIO_Pin_B25, hwADC_Instance_1, 4 }, /* CH4 */
    { hwGPIO_Pin_B24, hwADC_Instance_1, 5 }, /* CH5 */
    { hwGPIO_Pin_B20, hwADC_Instance_1, 6 }, /* CH6 */
    { hwGPIO_Pin_A22, hwADC_Instance_1, 7 }, /* CH7 */
    { hwGPIO_Pin_A14, hwADC_Instance_1, 12 }, /* CH12 */
    { hwGPIO_Pin_A15, hwADC_Instance_2, 0 }, /* CH0 */
    { hwGPIO_Pin_A16, hwADC_Instance_2, 1 }, /* CH1 */
    { hwGPIO_Pin_A17, hwADC_Instance_2, 2 }, /* CH2 */
    { hwGPIO_Pin_A18, hwADC_Instance_2, 3 }, /* CH3 */
    { hwGPIO_Pin_B17, hwADC_Instance_2, 4 }, /* CH4 */
    { hwGPIO_Pin_B18, hwADC_Instance_2, 5 }, /* CH5 */
    { hwGPIO_Pin_B19, hwADC_Instance_2, 6 }, /* CH6 */
    { hwGPIO_Pin_A21, hwADC_Instance_2, 7 }, /* CH7 */
#endif

/*
 * MSPM0G310x has smaller packages than G110x/G150x/G350x.  PB17..PB20,
 * PB24, and PB25 ADC inputs are not bonded out on this family.
 */
#if defined(MSPM0G310x)
    { hwGPIO_Pin_A27, hwADC_Instance_1, 0 }, /* CH0 */
    { hwGPIO_Pin_A26, hwADC_Instance_1, 1 }, /* CH1 */
    { hwGPIO_Pin_A25, hwADC_Instance_1, 2 }, /* CH2 */
    { hwGPIO_Pin_A24, hwADC_Instance_1, 3 }, /* CH3 */
    { hwGPIO_Pin_A22, hwADC_Instance_1, 7 }, /* CH7 */
    { hwGPIO_Pin_A14, hwADC_Instance_1, 12 }, /* CH12 */
    { hwGPIO_Pin_A15, hwADC_Instance_2, 0 }, /* CH0 */
    { hwGPIO_Pin_A16, hwADC_Instance_2, 1 }, /* CH1 */
    { hwGPIO_Pin_A17, hwADC_Instance_2, 2 }, /* CH2 */
    { hwGPIO_Pin_A18, hwADC_Instance_2, 3 }, /* CH3 */
    { hwGPIO_Pin_A21, hwADC_Instance_2, 7 }, /* CH7 */
#endif

/*
 * Extended dual-ADC G families:
 * G120x/G121x/G320x/G321x/G151x/G351x/G352x
 */
#if defined(MSPM0G120x) || defined(MSPM0G121x) || \
      defined(MSPM0G320x) || defined(MSPM0G321x) || \
      defined(MSPM0G151x) || defined(MSPM0G351x) || \
      defined(MSPM0G352x)
    { hwGPIO_Pin_A27, hwADC_Instance_1, 0 }, /* CH0 */
    { hwGPIO_Pin_A26, hwADC_Instance_1, 1 }, /* CH1 */
    { hwGPIO_Pin_A25, hwADC_Instance_1, 2 }, /* CH2 */
    { hwGPIO_Pin_A24, hwADC_Instance_1, 3 }, /* CH3 */
    { hwGPIO_Pin_B25, hwADC_Instance_1, 4 }, /* CH4 */
    { hwGPIO_Pin_B24, hwADC_Instance_1, 5 }, /* CH5 */
    { hwGPIO_Pin_B20, hwADC_Instance_1, 6 }, /* CH6 */
    { hwGPIO_Pin_A22, hwADC_Instance_1, 7 }, /* CH7 */
    { hwGPIO_Pin_A12, hwADC_Instance_1, 8 }, /* CH8 */
    { hwGPIO_Pin_A13, hwADC_Instance_1, 9 }, /* CH9 */
    { hwGPIO_Pin_A14, hwADC_Instance_1, 12 }, /* CH12 */
    { hwGPIO_Pin_A19, hwADC_Instance_1, 13 }, /* CH13 */
    { hwGPIO_Pin_A20, hwADC_Instance_1, 14 }, /* CH14 */
    { hwGPIO_Pin_A15, hwADC_Instance_2, 0 }, /* CH0 */
    { hwGPIO_Pin_A16, hwADC_Instance_2, 1 }, /* CH1 */
    { hwGPIO_Pin_A17, hwADC_Instance_2, 2 }, /* CH2 */
    { hwGPIO_Pin_A18, hwADC_Instance_2, 3 }, /* CH3 */
    { hwGPIO_Pin_B17, hwADC_Instance_2, 4 }, /* CH4 */
    { hwGPIO_Pin_B18, hwADC_Instance_2, 5 }, /* CH5 */
    { hwGPIO_Pin_B19, hwADC_Instance_2, 6 }, /* CH6 */
    { hwGPIO_Pin_A21, hwADC_Instance_2, 7 }, /* CH7 */
    { hwGPIO_Pin_B21, hwADC_Instance_2, 8 }, /* CH8 */
    { hwGPIO_Pin_B22, hwADC_Instance_2, 10 }, /* CH10 */
    { hwGPIO_Pin_B23, hwADC_Instance_2, 11 }, /* CH11 */
    { hwGPIO_Pin_A23, hwADC_Instance_2, 12 }, /* CH12 */
    { hwGPIO_Pin_B26, hwADC_Instance_2, 13 }, /* CH13 */
    { hwGPIO_Pin_B27, hwADC_Instance_2, 14 }, /* CH14 */
#endif

/* MSPM0G511x / MSPM0G518x */
#if defined(MSPM0G511x) || defined(MSPM0G518x)
    { hwGPIO_Pin_A20, hwADC_Instance_1, 0 }, /* CH0 */
    { hwGPIO_Pin_A19, hwADC_Instance_1, 1 }, /* CH1 */
    { hwGPIO_Pin_A25, hwADC_Instance_1, 2 }, /* CH2 */
    { hwGPIO_Pin_A24, hwADC_Instance_1, 3 }, /* CH3 */
    { hwGPIO_Pin_B25, hwADC_Instance_1, 4 }, /* CH4 */
    { hwGPIO_Pin_B24, hwADC_Instance_1, 5 }, /* CH5 */
    { hwGPIO_Pin_B20, hwADC_Instance_1, 6 }, /* CH6 */
    { hwGPIO_Pin_A22, hwADC_Instance_1, 7 }, /* CH7 */
    { hwGPIO_Pin_A21, hwADC_Instance_1, 8 }, /* CH8 */
    { hwGPIO_Pin_B19, hwADC_Instance_1, 9 }, /* CH9 */
    { hwGPIO_Pin_B18, hwADC_Instance_1, 10 }, /* CH10 */
    { hwGPIO_Pin_B17, hwADC_Instance_1, 11 }, /* CH11 */
    { hwGPIO_Pin_A18, hwADC_Instance_1, 12 }, /* CH12 */
    { hwGPIO_Pin_A17, hwADC_Instance_1, 13 }, /* CH13 */
    { hwGPIO_Pin_A16, hwADC_Instance_1, 14 }, /* CH14 */
    { hwGPIO_Pin_A15, hwADC_Instance_1, 15 }, /* CH15 */
    { hwGPIO_Pin_A14, hwADC_Instance_1, 16 }, /* CH16 */
    { hwGPIO_Pin_A13, hwADC_Instance_1, 17 }, /* CH17 */
    { hwGPIO_Pin_A12, hwADC_Instance_1, 18 }, /* CH18 */
    { hwGPIO_Pin_B16, hwADC_Instance_1, 19 }, /* CH19 */
    { hwGPIO_Pin_B15, hwADC_Instance_1, 20 }, /* CH20 */
    { hwGPIO_Pin_B14, hwADC_Instance_1, 21 }, /* CH21 */
    { hwGPIO_Pin_B23, hwADC_Instance_1, 22 }, /* CH22 */
    { hwGPIO_Pin_B26, hwADC_Instance_1, 23 }, /* CH23 */
    { hwGPIO_Pin_B22, hwADC_Instance_1, 24 }, /* CH24 */
    { hwGPIO_Pin_B21, hwADC_Instance_1, 25 }, /* CH25 */
#endif

/* MSPM0L110x / MSPM0L130x / MSPM0L134x */
#if defined(MSPM0L110x) || defined(MSPM0L130x) || defined(MSPM0L134x)
    { hwGPIO_Pin_A27, hwADC_Instance_1, 0 }, /* CH0 */
    { hwGPIO_Pin_A26, hwADC_Instance_1, 1 }, /* CH1 */
    { hwGPIO_Pin_A25, hwADC_Instance_1, 2 }, /* CH2 */
    { hwGPIO_Pin_A24, hwADC_Instance_1, 3 }, /* CH3 */
    { hwGPIO_Pin_A22, hwADC_Instance_1, 4 }, /* CH4 */
    { hwGPIO_Pin_A21, hwADC_Instance_1, 5 }, /* CH5 */
    { hwGPIO_Pin_A20, hwADC_Instance_1, 6 }, /* CH6 */
    { hwGPIO_Pin_A18, hwADC_Instance_1, 7 }, /* CH7 */
    { hwGPIO_Pin_A16, hwADC_Instance_1, 8 }, /* CH8 */
    { hwGPIO_Pin_A15, hwADC_Instance_1, 9 }, /* CH9 */
#endif

/* MSPM0L111x */
#if defined(MSPM0L111x)
    { hwGPIO_Pin_A27, hwADC_Instance_1, 0 }, /* CH0 */
    { hwGPIO_Pin_A26, hwADC_Instance_1, 1 }, /* CH1 */
    { hwGPIO_Pin_A25, hwADC_Instance_1, 2, 2 }, /* CH2 */
    { hwGPIO_Pin_A24, hwADC_Instance_1, 3 }, /* CH3 */
    { hwGPIO_Pin_A18, hwADC_Instance_1, 4 }, /* CH4 */
    { hwGPIO_Pin_B19, hwADC_Instance_1, 5 }, /* CH5 */
    { hwGPIO_Pin_B20, hwADC_Instance_1, 6 }, /* CH6 */
    { hwGPIO_Pin_A22, hwADC_Instance_1, 7 }, /* CH7 */
    { hwGPIO_Pin_A12, hwADC_Instance_1, 8 }, /* CH8 */
    { hwGPIO_Pin_A13, hwADC_Instance_1, 9 }, /* CH9 */
    { hwGPIO_Pin_A14, hwADC_Instance_1, 12 }, /* CH12 */
    { hwGPIO_Pin_A16, hwADC_Instance_1, 13 }, /* CH13 */
    { hwGPIO_Pin_A17, hwADC_Instance_1, 14 }, /* CH14 */
#endif

/* MSPM0L112x / L122x / L211x / L222x */
#if defined(MSPM0L112x) || defined(MSPM0L122x) || defined(MSPM0L211x) || defined(MSPM0L222x)
    { hwGPIO_Pin_A27, hwADC_Instance_1, 0 }, /* CH0 */
    { hwGPIO_Pin_A26, hwADC_Instance_1, 1 }, /* CH1 */
    { hwGPIO_Pin_A25, hwADC_Instance_1, 2 }, /* CH2 */
    { hwGPIO_Pin_A24, hwADC_Instance_1, 3 }, /* CH3 */
    { hwGPIO_Pin_B25, hwADC_Instance_1, 4 }, /* CH4 */
    { hwGPIO_Pin_B24, hwADC_Instance_1, 5 }, /* CH5 */
    { hwGPIO_Pin_B20, hwADC_Instance_1, 6 }, /* CH6 */
    { hwGPIO_Pin_A22, hwADC_Instance_1, 7 }, /* CH7 */
    { hwGPIO_Pin_A21, hwADC_Instance_1, 8 }, /* CH8 */
    { hwGPIO_Pin_B19, hwADC_Instance_1, 9 }, /* CH9 */
    { hwGPIO_Pin_B18, hwADC_Instance_1, 10 }, /* CH10 */
    { hwGPIO_Pin_B17, hwADC_Instance_1, 11 }, /* CH11 */
    { hwGPIO_Pin_A18, hwADC_Instance_1, 12 }, /* CH12 */
    { hwGPIO_Pin_A17, hwADC_Instance_1, 13 }, /* CH13 */
    { hwGPIO_Pin_A16, hwADC_Instance_1, 14 }, /* CH14 */
    { hwGPIO_Pin_A15, hwADC_Instance_1, 15 }, /* CH15 */
    { hwGPIO_Pin_A14, hwADC_Instance_1, 16 }, /* CH16 */
    { hwGPIO_Pin_A13, hwADC_Instance_1, 17 }, /* CH17 */
    { hwGPIO_Pin_A12, hwADC_Instance_1, 18 }, /* CH18 */
    { hwGPIO_Pin_B16, hwADC_Instance_1, 19 }, /* CH19 */
    { hwGPIO_Pin_B15, hwADC_Instance_1, 20 }, /* CH20 */
    { hwGPIO_Pin_B14, hwADC_Instance_1, 21 }, /* CH21 */
    { hwGPIO_Pin_B27, hwADC_Instance_1, 22 }, /* CH22 */
    { hwGPIO_Pin_B26, hwADC_Instance_1, 23 }, /* CH23 */
    { hwGPIO_Pin_B22, hwADC_Instance_1, 24 }, /* CH24 */
    { hwGPIO_Pin_B21, hwADC_Instance_1, 25 }, /* CH25 */
#endif
};


#endif //ADC_PIN_TIMSPM0_H