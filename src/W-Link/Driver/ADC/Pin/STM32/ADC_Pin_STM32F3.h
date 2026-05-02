#ifndef ADC_PIN_STM32F3_H
#define ADC_PIN_STM32F3_H

#include "ADC_Pin_STM32_Def.h"

const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
#if defined(ADC1_BASE)
    /* ===== ADC1 ===== */
    { hwGPIO_Pin_A0, hwADC_Instance_1 },  /* ADC1_IN1  */
    { hwGPIO_Pin_A1, hwADC_Instance_1 },  /* ADC1_IN2  */
    { hwGPIO_Pin_A2, hwADC_Instance_1 },  /* ADC1_IN3  */
    { hwGPIO_Pin_A3, hwADC_Instance_1 },  /* ADC1_IN4  */
    { hwGPIO_Pin_F4, hwADC_Instance_1 },  /* ADC1_IN5  */
    { hwGPIO_Pin_C0, hwADC_Instance_1 },  /* ADC1_IN6  */
    { hwGPIO_Pin_C1, hwADC_Instance_1 },  /* ADC1_IN7  */
    { hwGPIO_Pin_C2, hwADC_Instance_1 },  /* ADC1_IN8  */
    { hwGPIO_Pin_C3, hwADC_Instance_1 },  /* ADC1_IN9  */
    { hwGPIO_Pin_F2, hwADC_Instance_1 },  /* ADC1_IN10 */
    { hwGPIO_Pin_B0, hwADC_Instance_1 },  /* ADC1_IN11 */
    { hwGPIO_Pin_B1, hwADC_Instance_1 },  /* ADC1_IN12 */
    { hwGPIO_Pin_B13, hwADC_Instance_1 }, /* ADC1_IN13 */
    { hwGPIO_Pin_B14, hwADC_Instance_1 }, /* ADC1_IN14 */
    { hwGPIO_Pin_B15, hwADC_Instance_1 }, /* ADC1_IN15 */
#endif
#if defined(ADC2_BASE)
    /* ===== ADC2 ===== */
    { hwGPIO_Pin_A4, hwADC_Instance_2 },  /* ADC2_IN1  */
    { hwGPIO_Pin_A5, hwADC_Instance_2 },  /* ADC2_IN2  */
    { hwGPIO_Pin_A6, hwADC_Instance_2 },  /* ADC2_IN3  */
    { hwGPIO_Pin_A7, hwADC_Instance_2 },  /* ADC2_IN4  */
    { hwGPIO_Pin_C4, hwADC_Instance_2 },  /* ADC2_IN5  */
    { hwGPIO_Pin_C0, hwADC_Instance_2 },  /* ADC2_IN6  */
    { hwGPIO_Pin_C1, hwADC_Instance_2 },  /* ADC2_IN7  */
    { hwGPIO_Pin_C2, hwADC_Instance_2 },  /* ADC2_IN8  */
    { hwGPIO_Pin_C3, hwADC_Instance_2 },  /* ADC2_IN9  */
    { hwGPIO_Pin_F2, hwADC_Instance_2 },  /* ADC2_IN10 */
    { hwGPIO_Pin_C5, hwADC_Instance_2 },  /* ADC2_IN11 */
    { hwGPIO_Pin_B2, hwADC_Instance_2 },  /* ADC2_IN12 */
    { hwGPIO_Pin_B11, hwADC_Instance_2 }, /* ADC2_IN14 */
    { hwGPIO_Pin_B12, hwADC_Instance_2 }, /* ADC2_IN15 */
#endif
#if defined(ADC3_BASE)
    /* ===== ADC3 ===== */
    { hwGPIO_Pin_B1, hwADC_Instance_3 },  /* ADC3_IN1  */
    { hwGPIO_Pin_E9, hwADC_Instance_3 },  /* ADC3_IN2  */
    { hwGPIO_Pin_E13, hwADC_Instance_3 }, /* ADC3_IN3  */
    { hwGPIO_Pin_E14, hwADC_Instance_3 }, /* ADC3_IN4  */
    { hwGPIO_Pin_B13, hwADC_Instance_3 }, /* ADC3_IN5  */
    { hwGPIO_Pin_E8, hwADC_Instance_3 },  /* ADC3_IN6  */
    { hwGPIO_Pin_D10, hwADC_Instance_3 }, /* ADC3_IN7  */
    { hwGPIO_Pin_D11, hwADC_Instance_3 }, /* ADC3_IN8  */
    { hwGPIO_Pin_D12, hwADC_Instance_3 }, /* ADC3_IN9  */
    { hwGPIO_Pin_D13, hwADC_Instance_3 }, /* ADC3_IN10 */
    { hwGPIO_Pin_D14, hwADC_Instance_3 }, /* ADC3_IN11 */
    { hwGPIO_Pin_B0, hwADC_Instance_3 },  /* ADC3_IN12 */
#endif
};

#endif // ADC_PIN_STM32F3_H