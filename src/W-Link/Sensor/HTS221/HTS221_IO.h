/*
This HTS221 driver is based on STMicroelectronics stm32-lsm6dso / HTS221 component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef HTS221_IO_H
#define HTS221_IO_H

#include <stdint.h>
#include <stdbool.h>

#include "HTS221_Def.h"

#include "Sensor_Config.h"

/* Defines -------------------------------------------------------------------*/

/*
 * HTS221 I2C 7-bit address
 *
 * SA0 = 0 -> 0x6A
 * SA0 = 1 -> 0x6B
 */
#define HTS221_I2C_ADDRESS               0xBE

#define HTS221_I2C_OP_TIMEOUT             500

#ifndef CONFIG_HTS221_I2C_INDEX
#define HTS221_I2C_INDEX                  hwI2C_Index_0
#else
#define HTS221_I2C_INDEX                  CONFIG_HTS221_I2C_INDEX
#endif

#define HTS221_IO_TX_BUF_SIZE             32

/* Typedefs ------------------------------------------------------------------*/

typedef void (*HTS221_IO_Event_IRQ)(HTS221_Interrupt_Index index);

/* Functions -----------------------------------------------------------------*/

HTS221_OpStatus HTS221_IO_Init(void);
HTS221_OpStatus HTS221_IO_DeInit(void);

HTS221_OpStatus HTS221_Register_Interrupt_Handler(HTS221_IO_Event_IRQ irq_callback);
HTS221_OpStatus HTS221_UnRegister_Interrupt_Handler(void);

HTS221_OpStatus HTS221_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
);

HTS221_OpStatus HTS221_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
);

HTS221_OpStatus HTS221_IO_ReadByte(
    uint8_t RegisterAddr,
    uint8_t* value
);

HTS221_OpStatus HTS221_IO_WriteByte(
    uint8_t RegisterAddr,
    uint8_t value
);

#endif /* HTS221_IO_H */