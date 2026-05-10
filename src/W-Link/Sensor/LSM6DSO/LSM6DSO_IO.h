/*
This LSM6DSO driver is based on STMicroelectronics stm32-lsm6dso / LSM6DSO component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LSM6DSO_IO_H
#define LSM6DSO_IO_H

#include <stdint.h>
#include <stdbool.h>

#include "LSM6DSO_Def.h"

#include "Sensor_Config.h"

/* Defines -------------------------------------------------------------------*/

/*
 * LSM6DSO I2C 7-bit address
 *
 * SA0 = 0 -> 0x6A
 * SA0 = 1 -> 0x6B
 */
#define LSM6DSO_I2C_ADD_L                  0x6A
#define LSM6DSO_I2C_ADD_H                  0x6B

#define LSM6DSO_I2C_OP_TIMEOUT             500

#ifdef CONFIG_LSM6DSO_I2C_ADDR_L
#define LSM6DSO_I2C_ADDRESS                LSM6DSO_I2C_ADD_L
#endif

#ifdef CONFIG_LSM6DSO_I2C_ADDR_H
#define LSM6DSO_I2C_ADDRESS                LSM6DSO_I2C_ADD_H
#endif

#ifndef LSM6DSO_I2C_ADDRESS
#define LSM6DSO_I2C_ADDRESS                LSM6DSO_I2C_ADD_H
#endif

#ifndef CONFIG_LSM6DSO_I2C_INDEX
#define LSM6DSO_I2C_INDEX                  hwI2C_Index_0
#else
#define LSM6DSO_I2C_INDEX                  CONFIG_LSM6DSO_I2C_INDEX
#endif

#define LSM6DSO_IO_TX_BUF_SIZE             32

/* Typedefs ------------------------------------------------------------------*/

typedef void (*LSM6DSO_IO_Event_IRQ)(LSM6DSO_Interrupt_Index index);

/* Functions -----------------------------------------------------------------*/

LSM6DSO_OpStatus LSM6DSO_Register_Interrupt_Handler(LSM6DSO_IO_Event_IRQ irq_callback);
LSM6DSO_OpStatus LSM6DSO_UnRegister_Interrupt_Handler(void);

LSM6DSO_OpStatus LSM6DSO_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
);

LSM6DSO_OpStatus LSM6DSO_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
);

LSM6DSO_OpStatus LSM6DSO_IO_ReadByte(
    uint8_t RegisterAddr,
    uint8_t* value
);

LSM6DSO_OpStatus LSM6DSO_IO_WriteByte(
    uint8_t RegisterAddr,
    uint8_t value
);

#endif /* LSM6DSO_IO_H */