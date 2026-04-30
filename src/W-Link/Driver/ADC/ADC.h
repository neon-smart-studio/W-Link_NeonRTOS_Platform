
#ifndef ADC_H
#define ADC_H

#include "soc.h"  

#include "ADC_Def.h"

#include "ADC_Channel.h"

#ifdef	__cplusplus
extern "C" {
#endif

hwADC_OpStatus ADC_Channel_Init(hwADC_Channel_Index channel);
hwADC_OpStatus ADC_Channel_DeInit(hwADC_Channel_Index channel);
hwADC_OpStatus ADC_Read_MiniVolt(hwADC_Channel_Index channel, float* readMv);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //ADC_H