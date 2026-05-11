/*
This M24SR driver is based on STMicroelectronics stm32-M24SR / M24SR component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef M24SR_DEF_H
#define M24SR_DEF_H

typedef enum M24SR_OpStatus_t
{
  M24SR_OK = 0,
  M24SR_NotInit = -1,
  M24SR_InvalidParameter = -2,
  M24SR_MemoryError = -3,
  M24SR_MutexTimeout = -4,
  M24SR_SlaveTimeout = -5,
  M24SR_IO_Error = -6,
  M24SR_CRC_Error = -7,
  M24SR_Unsupport = -8,
} M24SR_OpStatus;

#endif //M24SR_DEF_H