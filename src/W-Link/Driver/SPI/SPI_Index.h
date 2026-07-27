
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

#ifdef DEVICE_TITIVAC
typedef enum hwSPI_Index_t
{
  hwSPI_Index_0 = 0,
  hwSPI_Index_1,
  hwSPI_Index_2,
  hwSPI_Index_3,
  hwSPI_Index_MAX,
}hwSPI_Index;
#endif // DEVICE_TITIVAC

#ifdef DEVICE_TIMSP432P
typedef enum hwSPI_Index_t
{
  hwSPI_Index_0 = 0,
  hwSPI_Index_1,
  hwSPI_Index_2,
  hwSPI_Index_3,
  hwSPI_Index_MAX,
}hwSPI_Index;
#endif // DEVICE_TIMSP432P

#ifdef DEVICE_TIMSP432E
typedef enum hwSPI_Index_t
{
  hwSPI_Index_0 = 0,
  hwSPI_Index_1,
  hwSPI_Index_2,
  hwSPI_Index_3,
  hwSPI_Index_MAX,
}hwSPI_Index;
#endif // DEVICE_TIMSP432E

#ifdef DEVICE_TIMSPM0
typedef enum hwSPI_Index_t
{
#if defined(SPI0_BASE) || defined(UC0_SPI_BASE)
    hwSPI_Index_0 = 0,       // SPI0 / UC0 SPI
#endif

#if defined(SPI1_BASE) || defined(UC1_SPI_BASE)
    hwSPI_Index_1,           // SPI1 / UC1 SPI
#endif

#if defined(SPI2_BASE) || defined(UC2_SPI_BASE)
    hwSPI_Index_2,           // SPI2 / UC2 SPI
#endif

#if defined(SPI3_BASE) || defined(UC3_SPI_BASE)
    hwSPI_Index_3,           // SPI3 / UC3 SPI
#endif

#if defined(SPI4_BASE) || defined(UC4_SPI_BASE)
    hwSPI_Index_4,           // SPI4 / UC4 SPI
#endif

#if defined(SPI5_BASE) || defined(UC5_SPI_BASE)
    hwSPI_Index_5,           // SPI5 / UC5 SPI
#endif

#if defined(SPI6_BASE) || defined(UC6_SPI_BASE)
    hwSPI_Index_6,           // SPI6 / UC6 SPI
#endif

#if defined(SPI7_BASE) || defined(UC7_SPI_BASE)
    hwSPI_Index_7,           // SPI7 / UC7 SPI
#endif

#if defined(SPI8_BASE) || defined(UC8_SPI_BASE)
    hwSPI_Index_8,           // SPI8 / UC8 SPI
#endif

#if defined(SPI9_BASE) || defined(UC9_SPI_BASE)
    hwSPI_Index_9,           // SPI9 / UC9 SPI
#endif

    hwSPI_Index_MAX

} hwSPI_Index;
#endif // DEVICE_TIMSPM0

#endif //SPI_MASTER_INDEX_H