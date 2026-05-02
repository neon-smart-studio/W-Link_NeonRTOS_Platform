#ifndef DAC_PIN_STM32G4_H
#define DAC_PIN_STM32G4_H

#include "DAC_Pin_STM32_Def.h"

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

#if defined(DAC2_BASE)
    /* ===== DAC2 ===== */
    { hwGPIO_Pin_A6, hwDAC_Instance_2 },   // DAC2_OUT1
#endif

#if defined(DAC3_BASE)
    /* ===== DAC3 ===== */
    { hwGPIO_Pin_B1, hwDAC_Instance_3 },   // DAC3_OUT1
#endif

#if defined(DAC4_BASE)
    /* ===== DAC4 ===== */
    { hwGPIO_Pin_B12, hwDAC_Instance_4 },  // DAC4_OUT1
#endif
};

#endif // DAC_PIN_STM32G4_H