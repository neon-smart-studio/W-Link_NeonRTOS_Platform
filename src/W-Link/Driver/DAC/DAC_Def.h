
#ifndef DAC_DEF_H
#define DAC_DEF_H

typedef enum hwDAC_OpResult_t
{
  hwDAC_OK = 0,
  hwDAC_NotInit = -1,
  hwDAC_InvalidParameter = -2,
  hwDAC_HwError = -3,
  hwDAC_MemoryError = -4,
  hwDAC_MutexTimeout = -5,
  hwDAC_HW_Error = -6,
  hwDAC_Unsupport = -7
} hwDAC_OpResult;

#endif //DAC_DEF_H