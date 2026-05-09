/*
This LPS22HB driver is based on STMicroelectronics stm32-lsm6dso / LPS22HB component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LPS22HB_IO_H
#define LPS22HB_IO_H

#include <stdint.h>
#include <stdbool.h>

#include "LPS22HB_Def.h"

#include "Sensor_Config.h"

/*
 * LPS22HB I2C 7-bit address
 *
 * SA0 = 0 -> 0x6A
 * SA0 = 1 -> 0x6B
 */
#define LPS22HB_I2C_ADDRESS               0xBE

#define LPS22HB_I2C_OP_TIMEOUT             500

#ifndef CONFIG_LPS22HB_I2C_INDEX
#define LPS22HB_I2C_INDEX                  hwI2C_Index_0
#else
#define LPS22HB_I2C_INDEX                  CONFIG_LPS22HB_I2C_INDEX
#endif

#define LPS22HB_IO_TX_BUF_SIZE             32

typedef void (*LPS22HB_IO_Event_IRQ)(LPS22HB_Interrupt_Index index);

LPS22HB_OpStatus LPS22HB_IO_Init(void);
LPS22HB_OpStatus LPS22HB_IO_DeInit(void);

LPS22HB_OpStatus LPS22HB_Register_Interrupt_Handler(LPS22HB_IO_Event_IRQ irq_callback);
LPS22HB_OpStatus LPS22HB_UnRegister_Interrupt_Handler(void);

LPS22HB_OpStatus LPS22HB_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
);

LPS22HB_OpStatus LPS22HB_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
);

LPS22HB_OpStatus LPS22HB_IO_ReadByte(
    uint8_t RegisterAddr,
    uint8_t* value
);

LPS22HB_OpStatus LPS22HB_IO_WriteByte(
    uint8_t RegisterAddr,
    uint8_t value
);

#endif /* LPS22HB_IO_H */