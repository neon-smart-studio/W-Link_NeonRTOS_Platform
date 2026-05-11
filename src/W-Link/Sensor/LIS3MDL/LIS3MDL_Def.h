/*
This LIS3MDL driver is based on STMicroelectronics stm32-LIS3MDL / LIS3MDL component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LIS3MDL_DEF_H
#define LIS3MDL_DEF_H

typedef enum LIS3MDL_OpStatus_t
{
  LIS3MDL_OK = 0,
  LIS3MDL_NotInit = -1,
  LIS3MDL_InvalidParameter = -2,
  LIS3MDL_MemoryError = -3,
  LIS3MDL_MutexTimeout = -4,
  LIS3MDL_SlaveTimeout = -5,
  LIS3MDL_IO_Error = -6,
  LIS3MDL_Unsupport = -7,
} LIS3MDL_OpStatus;

typedef enum LIS3MDL_Interrupt_Index_t
{
	LIS3MDL_Interrupt_Index_1 = 0,
	LIS3MDL_Interrupt_Index_2,
	LIS3MDL_Interrupt_Index_MAX
}LIS3MDL_Interrupt_Index;

#endif //LIS3MDL_DEF_H