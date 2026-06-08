
#ifndef SPI_MASTER_INDEX_H
#define SPI_MASTER_INDEX_H

#include "soc.h"

#ifdef DEVICE_NUVOTON
typedef enum hwSPI_Index_t
{
#if defined (SPI0_BASE)
  hwSPI_Index_0 = 0,
#endif
#if defined (SPI1_BASE)
  hwSPI_Index_1,
#endif
#if defined (SPI2_BASE)
  hwSPI_Index_2,
#endif
#if defined (SPI3_BASE)
  hwSPI_Index_3,
#endif
  hwSPI_Index_MAX,
}hwSPI_Index;
#endif // DEVICE_NUVOTON

#ifdef DEVICE_STM32
typedef enum hwSPI_Index_t
{
#if defined (SPI1_BASE)
  hwSPI_Index_0 = 0,
#endif
#if defined (SPI2_BASE)
  hwSPI_Index_1,
#endif
#if defined (SPI3_BASE)
  hwSPI_Index_2,
#endif
#if defined (SPI4_BASE)
  hwSPI_Index_3,
#endif
#if defined (SPI5_BASE)
  hwSPI_Index_4,
#endif
#if defined (SPI6_BASE)
  hwSPI_Index_5,
#endif
  hwSPI_Index_MAX,
}hwSPI_Index;
#endif // DEVICE_STM32

#ifdef DEVICE_RP2
typedef enum hwSPI_Index_t
{
  hwSPI_Index_0 = 0,
  hwSPI_Index_1,
  hwSPI_Index_MAX,
}hwSPI_Index;
#endif // DEVICE_RP2

#endif //SPI_MASTER_INDEX_H