/*
This LSM6DSL driver is based on STMicroelectronics stm32-lsm6dso / LSM6DSL component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LSM6DSL_DEF_H
#define LSM6DSL_DEF_H

typedef enum LSM6DSL_OpStatus_t
{
  LSM6DSL_OK = 0,
  LSM6DSL_NotInit = -1,
  LSM6DSL_InvalidParameter = -2,
  LSM6DSL_MemoryError = -3,
  LSM6DSL_MutexTimeout = -4,
  LSM6DSL_SlaveTimeout = -5,
  LSM6DSL_IO_Error = -6,
  LSM6DSL_Unsupport = -7,
} LSM6DSL_OpStatus;

typedef enum LSM6DSL_Interrupt_Index_t
{
	LSM6DSL_Interrupt_Index_1 = 0,
	LSM6DSL_Interrupt_Index_2,
	LSM6DSL_Interrupt_Index_MAX
}LSM6DSL_Interrupt_Index;

#endif //LSM6DSL_DEF_H