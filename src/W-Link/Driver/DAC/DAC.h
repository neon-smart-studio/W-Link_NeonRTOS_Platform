
#ifndef DAC_H
#define DAC_H

#include "soc.h"  

#include "DAC_Def.h"

#include "DAC_Channel.h"

hwDAC_OpStatus DAC_Channel_Init(hwDAC_Channel_Index channel);
hwDAC_OpStatus DAC_Channel_DeInit(hwDAC_Channel_Index channel);
hwDAC_OpStatus DAC_Read_MiniVolt(hwDAC_Channel_Index channel, float* readMv);

#endif //DAC_H