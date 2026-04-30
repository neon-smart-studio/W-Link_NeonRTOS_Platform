#ifndef DAC_PIN_STM32L0_H
#define DAC_PIN_STM32L0_H

#include "DAC_Pin_STM32.h"

static const DAC_Channel_Def DAC_Channel_Def_Table[hwDAC_Channel_Index_MAX] = {
#if defined(DAC1_BASE)
#if defined(DAC_CHANNEL_1)
    /* ===== DAC / DAC1 ===== */
    { hwGPIO_Pin_A4, hwDAC_Instance_1 },   // DAC_OUT1 / DAC1_OUT1
#endif
#if defined(DAC_CHANNEL_2)
    { hwGPIO_Pin_A5, hwDAC_Instance_1 },   // DAC_OUT2 / DAC1_OUT2
#endif
#endif
};

#endif // DAC_PIN_STM32L0_H