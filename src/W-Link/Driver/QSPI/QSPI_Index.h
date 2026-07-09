
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

#ifdef DEVICE_NUVOTON
typedef enum hwQSPI_Index_t
{
  hwQSPI_Index_MAX,
}hwQSPI_Index;
#endif // DEVICE_NUVOTON

#ifdef DEVICE_TITIVAC
typedef enum hwQSPI_Index_t
{
  hwQSPI_Index_0 = 0,
  hwQSPI_Index_1,
  hwQSPI_Index_2,
  hwQSPI_Index_3,
  hwQSPI_Index_MAX,
}hwQSPI_Index;
#endif // DEVICE_TITIVAC

#ifdef DEVICE_TIMSP432P
typedef enum {
    hwQSPI_Index_MAX
} hwQSPI_Index;
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
typedef enum {
    hwQSPI_Index_MAX
} hwQSPI_Index;
#endif // DEVICE_TIMSP432E

#endif //QSPI_MASTER_INDEX_H