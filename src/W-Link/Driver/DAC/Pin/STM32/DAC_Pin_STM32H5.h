#ifndef DAC_PIN_STM32H5_H
#define DAC_PIN_STM32H5_H

#include "DAC_Pin_STM32_Def.h"

static const DAC_Channel_Def DAC_Channel_Def_Table[hwDAC_Channel_Index_MAX] = {
#if defined(DAC_CHANNEL_1)
    /* ===== DAC / DAC1 ===== */
    { hwGPIO_Pin_A4, hwDAC_Instance_1 },   // DAC_OUT1 / DAC1_OUT1

#if defined(DAC_CHANNEL_2)
    { hwGPIO_Pin_A5, hwDAC_Instance_1 },   // DAC_OUT2 / DAC1_OUT2
#endif
#endif
};

#endif // DAC_PIN_STM32H5_H