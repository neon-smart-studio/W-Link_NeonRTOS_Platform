#ifndef ADC_PIN_STM32H7RS_H
#define ADC_PIN_STM32H7RS_H

#include "ADC_Pin_STM32.h"

static const ADC_Channel_Def ADC_Channel_Def_Table[hwADC_Channel_Index_MAX] = {
#if defined(ADC1_BASE)
    /* ===== ADC1 / ADC12 common pins ===== */
    { hwGPIO_Pin_A0,  hwADC_Instance_1 }, /* ADC12_INP0  */
    { hwGPIO_Pin_A1,  hwADC_Instance_1 }, /* ADC12_INP1  */
    { hwGPIO_Pin_A6,  hwADC_Instance_1 }, /* ADC12_INP3  */
    { hwGPIO_Pin_C4,  hwADC_Instance_1 }, /* ADC12_INP4  */
    { hwGPIO_Pin_B1,  hwADC_Instance_1 }, /* ADC12_INP5  */
    { hwGPIO_Pin_A7,  hwADC_Instance_1 }, /* ADC12_INP7  */
    { hwGPIO_Pin_C5,  hwADC_Instance_1 }, /* ADC12_INP8  */
    { hwGPIO_Pin_B0,  hwADC_Instance_1 }, /* ADC12_INP9  */
    { hwGPIO_Pin_C0,  hwADC_Instance_1 }, /* ADC12_INP10 */
    { hwGPIO_Pin_C1,  hwADC_Instance_1 }, /* ADC12_INP11 */
    { hwGPIO_Pin_C2,  hwADC_Instance_1 }, /* ADC12_INP12 */
    { hwGPIO_Pin_C3,  hwADC_Instance_1 }, /* ADC12_INP13 */
    { hwGPIO_Pin_A2,  hwADC_Instance_1 }, /* ADC12_INP14 */
    { hwGPIO_Pin_A3,  hwADC_Instance_1 }, /* ADC12_INP15 */

    /* ADC1 only */
    { hwGPIO_Pin_F11, hwADC_Instance_1 }, /* ADC1_INP2  */
    { hwGPIO_Pin_F12, hwADC_Instance_1 }, /* ADC1_INP6  */
    { hwGPIO_Pin_A4,  hwADC_Instance_1 }, /* ADC1_INP18 */
#endif

#if defined(ADC2_BASE)
    /* ADC2 only */
    { hwGPIO_Pin_F13, hwADC_Instance_2 }, /* ADC2_INP2  */
    { hwGPIO_Pin_F14, hwADC_Instance_2 }, /* ADC2_INP6  */
    { hwGPIO_Pin_A5,  hwADC_Instance_2 }, /* ADC2_INP18 */
#endif
};

#endif // ADC_PIN_STM32H7RS_H