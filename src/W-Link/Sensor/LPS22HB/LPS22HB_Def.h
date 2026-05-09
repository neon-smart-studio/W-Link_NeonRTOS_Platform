/*
This LPS22HB driver is based on STMicroelectronics stm32-lsm6dso / LPS22HB component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LPS22HB_DEF_H
#define LPS22HB_DEF_H

typedef enum LPS22HB_IO_OpStatus_t
{
  LPS22HB_OK = 0,
  LPS22HB_NotInit = -1,
  LPS22HB_InvalidParameter = -2,
  LPS22HB_MemoryError = -3,
  LPS22HB_MutexTimeout = -4,
  LPS22HB_SlaveTimeout = -5,
  LPS22HB_IO_Error = -6,
  LPS22HB_Unsupport = -7,
} LPS22HB_OpStatus;

typedef enum LPS22HB_Interrupt_Index_t
{
	LPS22HB_Interrupt_Index_1 = 0,
	LPS22HB_Interrupt_Index_2,
	LPS22HB_Interrupt_Index_MAX
}LPS22HB_Interrupt_Index;

#endif //LPS22HB_DEF_H