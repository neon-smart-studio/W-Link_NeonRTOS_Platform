#ifndef DAC_PIN_TIMSPM0_H
#define DAC_PIN_TIMSPM0_H

#include "DAC_Pin_TIMSPM0_Def.h"

static const DAC_Channel_Def DAC_Channel_Def_Table[hwDAC_Channel_Index_MAX] =
{
#if defined(DAC0_BASE)
    { hwGPIO_Pin_A15, hwDAC_Instance_1 }  /* DAC0_OUT */
#endif
};

#endif // DAC_PIN_TIMSPM0_H