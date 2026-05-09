/*
This HTS221 driver is based on STMicroelectronics stm32-lsm6dso / HTS221 component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef HTS221_DEF_H
#define HTS221_DEF_H

typedef enum HTS221_IO_OpStatus_t
{
  HTS221_OK = 0,
  HTS221_NotInit = -1,
  HTS221_InvalidParameter = -2,
  HTS221_MemoryError = -3,
  HTS221_MutexTimeout = -4,
  HTS221_SlaveTimeout = -5,
  HTS221_IO_Error = -6,
  HTS221_Unsupport = -7,
} HTS221_OpStatus;

#endif //HTS221_DEF_H