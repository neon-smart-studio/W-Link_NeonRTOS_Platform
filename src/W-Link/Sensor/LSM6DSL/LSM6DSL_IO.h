/*
This LSM6DSL driver is based on STMicroelectronics stm32-lsm6dso / LSM6DSL component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LSM6DSL_IO_H
#define LSM6DSL_IO_H

#include <stdint.h>
#include <stdbool.h>

#include "LSM6DSL_Def.h"

#include "Sensor_Config.h"

/* Defines -------------------------------------------------------------------*/

/*
 * LSM6DSL I2C 7-bit address
 *
 * SA0 = 0 -> 0x6A
 * SA0 = 1 -> 0x6B
 */
#define LSM6DSL_I2C_ADD_L                  0xD4
#define LSM6DSL_I2C_ADD_H                  0xD6

#define LSM6DSL_I2C_OP_TIMEOUT             500

#ifdef CONFIG_LSM6DSL_I2C_ADDR_L
#define LSM6DSL_I2C_ADDRESS                LSM6DSL_I2C_ADD_L
#endif

#ifdef CONFIG_LSM6DSL_I2C_ADDR_H
#define LSM6DSL_I2C_ADDRESS                LSM6DSL_I2C_ADD_H
#endif

#ifndef LSM6DSL_I2C_ADDRESS
#define LSM6DSL_I2C_ADDRESS                LSM6DSL_I2C_ADD_H
#endif

#ifndef CONFIG_LSM6DSL_I2C_INDEX
#define LSM6DSL_I2C_INDEX                  hwI2C_Index_0
#else
#define LSM6DSL_I2C_INDEX                  CONFIG_LSM6DSL_I2C_INDEX
#endif

#define LSM6DSL_IO_TX_BUF_SIZE             32

/* Typedefs ------------------------------------------------------------------*/

typedef void (*LSM6DSL_IO_Event_IRQ)(LSM6DSL_Interrupt_Index index);

/* Functions -----------------------------------------------------------------*/

LSM6DSL_OpStatus LSM6DSL_Register_Interrupt_Handler(LSM6DSL_IO_Event_IRQ irq_callback);
LSM6DSL_OpStatus LSM6DSL_UnRegister_Interrupt_Handler(void);

LSM6DSL_OpStatus LSM6DSL_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
);

LSM6DSL_OpStatus LSM6DSL_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
);

LSM6DSL_OpStatus LSM6DSL_IO_ReadByte(
    uint8_t RegisterAddr,
    uint8_t* value
);

LSM6DSL_OpStatus LSM6DSL_IO_WriteByte(
    uint8_t RegisterAddr,
    uint8_t value
);

#endif /* LSM6DSL_IO_H */