
#ifndef QSPI_MASTER_INDEX_H
#define QSPI_MASTER_INDEX_H

#include "soc.h"

#ifdef DEVICE_STM32
typedef enum hwQSPI_Index_t
{
#if defined (QUADSPI)
  hwQSPI_Index_0 = 0,
#endif
  hwQSPI_Index_MAX,
}hwQSPI_Index;
#endif // DEVICE_STM32

#endif //QSPI_MASTER_INDEX_H