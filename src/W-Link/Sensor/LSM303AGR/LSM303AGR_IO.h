/*
This LSM303AGR driver is based on STMicroelectronics stm32-LSM303AGR / LSM303AGR component driver.
Original copyright: STMicroelectronics.
Original license: BSD-3-Clause.

Modifications:
- Ported IO layer to W-Link / NeonRTOS I2C interface.
- Adapted configuration macros and operation status mapping.
- Removed STM32 BSP object/context dependency.
*/

#ifndef LSM303AGR_IO_H
#define LSM303AGR_IO_H

#include <stdint.h>
#include <stdbool.h>

#include "LSM303AGR_Def.h"

#include "Sensor_Config.h"

/* Defines -------------------------------------------------------------------*/

/*
 * LSM303AGR I2C 7-bit address
 *
 * SA0 = 0 -> 0x6A
 * SA0 = 1 -> 0x6B
 */
/************** I2C Address *****************/

#define LSM303AGR_ACC_I2C_ADDRESS            0x32
#define LSM303AGR_MAG_I2C_ADDRESS            0x3C

#define LSM303AGR_I2C_OP_TIMEOUT             500

#ifndef CONFIG_LSM303AGR_I2C_INDEX
#define LSM303AGR_I2C_INDEX                  hwI2C_Index_0
#else
#define LSM303AGR_I2C_INDEX                  CONFIG_LSM303AGR_I2C_INDEX
#endif

#define LSM303AGR_IO_TX_BUF_SIZE             32

LSM303AGR_OpStatus LSM303AGR_ACC_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
);

LSM303AGR_OpStatus LSM303AGR_ACC_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
);

LSM303AGR_OpStatus LSM303AGR_ACC_IO_ReadByte(
    uint8_t RegisterAddr,
    uint8_t* value
);

LSM303AGR_OpStatus LSM303AGR_ACC_IO_WriteByte(
    uint8_t RegisterAddr,
    uint8_t value
);

LSM303AGR_OpStatus LSM303AGR_MAG_IO_Read(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToRead
);

LSM303AGR_OpStatus LSM303AGR_MAG_IO_Write(
    uint8_t RegisterAddr,
    uint8_t* pBuffer,
    uint16_t NumByteToWrite
);

LSM303AGR_OpStatus LSM303AGR_MAG_IO_ReadByte(
    uint8_t RegisterAddr,
    uint8_t* value
);

LSM303AGR_OpStatus LSM303AGR_MAG_IO_WriteByte(
    uint8_t RegisterAddr,
    uint8_t value
);

#endif /* LSM303AGR_IO_H */