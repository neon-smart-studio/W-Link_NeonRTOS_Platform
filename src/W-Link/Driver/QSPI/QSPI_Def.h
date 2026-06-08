
#ifndef QSPI_DEF_H
#define QSPI_DEF_H

typedef enum hwQSPI_OpMode_t
{
  hwQSPI_OpMode_Polarity0_Phase0 = 0,
  hwQSPI_OpMode_Polarity1_Phase1 = 1,
  hwQSPI_OpMode_MAX = 2,
}hwQSPI_OpMode;

typedef enum hwQSPI_OpResult_t
{
  hwQSPI_OK = 0,
  hwQSPI_NotInit = -1,
  hwQSPI_InvalidParameter = -2,
  hwQSPI_HwError = -3,
  hwQSPI_MemoryError = -4,
  hwQSPI_MutexTimeout = -5,
  hwQSPI_SlaveTimeout = -6,
  hwQSPI_Unsupport = -7,
}hwQSPI_OpResult;

#endif //QSPI_DEF_H