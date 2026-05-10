/*
This LSM6DSO driver is based on STMicroelectronics stm32-lsm6dso / LSM6DSO component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LSM6DSO_DEF_H
#define LSM6DSO_DEF_H

typedef enum LSM6DSO_OpStatus_t
{
  LSM6DSO_OK = 0,
  LSM6DSO_NotInit = -1,
  LSM6DSO_InvalidParameter = -2,
  LSM6DSO_MemoryError = -3,
  LSM6DSO_MutexTimeout = -4,
  LSM6DSO_SlaveTimeout = -5,
  LSM6DSO_IO_Error = -6,
  LSM6DSO_Unsupport = -7,
} LSM6DSO_OpStatus;

typedef enum LSM6DSO_Interrupt_Index_t
{
	LSM6DSO_Interrupt_Index_1 = 0,
	LSM6DSO_Interrupt_Index_2,
	LSM6DSO_Interrupt_Index_MAX
}LSM6DSO_Interrupt_Index;

#endif //LSM6DSO_DEF_H