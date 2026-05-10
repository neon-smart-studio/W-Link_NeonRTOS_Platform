/*
This LSM303AGR driver is based on STMicroelectronics stm32-lsm6dso / LSM303AGR component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LSM303AGR_DEF_H
#define LSM303AGR_DEF_H

typedef enum LSM303AGR_OpStatus_t
{
  LSM303AGR_OK = 0,
  LSM303AGR_NotInit = -1,
  LSM303AGR_InvalidParameter = -2,
  LSM303AGR_MemoryError = -3,
  LSM303AGR_MutexTimeout = -4,
  LSM303AGR_SlaveTimeout = -5,
  LSM303AGR_IO_Error = -6,
  LSM303AGR_Unsupport = -7,
} LSM303AGR_OpStatus;

typedef enum LSM303AGR_Interrupt_Index_t
{
	LSM303AGR_Interrupt_Index_1 = 0,
	LSM303AGR_Interrupt_Index_2,
	LSM303AGR_Interrupt_Index_MAX
}LSM303AGR_Interrupt_Index;

#endif //LSM303AGR_DEF_H