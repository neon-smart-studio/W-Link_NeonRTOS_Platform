/*
This LIS3MDL driver is based on STMicroelectronics stm32-LIS3MDL / LIS3MDL component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LIS3MDL_IO_H
#define LIS3MDL_IO_H

#include <stdint.h>
#include <stdbool.h>

#include "LIS3MDL_Def.h"

#include "Sensor_Config.h"

/*
 * LIS3MDL I2C 7-bit address
 *
 * SA0 = 0 -> 0x6A
 * SA0 = 1 -> 0x6B
 */
#define LIS3MDL_MAG_I2C_ADDR_L   0x38    // SAD[1] = 0
#define LIS3MDL_MAG_I2C_ADDR_H   0x3C    // SAD[1] = 1

#define LIS3MDL_I2C_OP_TIMEOUT   500

#ifdef CONFIG_LIS3MDL_I2C_ADDR_L
#define LIS3MDL_I2C_ADDRESS                LIS3MDL_MAG_I2C_ADDR_L
#endif

#ifdef CONFIG_LIS3MDL_I2C_ADDR_H
#define LIS3MDL_I2C_ADDRESS                LIS3MDL_MAG_I2C_ADDR_H
#endif

#ifndef LIS3MDL_I2C_ADDRESS
#define LIS3MDL_I2C_ADDRESS                LIS3MDL_MAG_I2C_ADDR_H
#endif

#ifndef CONFIG_LIS3MDL_I2C_INDEX
#define LIS3MDL_I2C_INDEX                  hwI2C_Index_0
#else
#define LIS3MDL_I2C_INDEX                  CONFIG_LIS3MDL_I2C_INDEX
#endif

#define LIS3MDL_IO_TX_BUF_SIZE             32

LIS3MDL_OpResult LIS3MDL_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
);

LIS3MDL_OpResult LIS3MDL_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
);

LIS3MDL_OpResult LIS3MDL_IO_ReadByte(
    uint8_t RegisterAddr,
    uint8_t* value
);

LIS3MDL_OpResult LIS3MDL_IO_WriteByte(
    uint8_t RegisterAddr,
    uint8_t value
);

#endif /* LIS3MDL_IO_H */