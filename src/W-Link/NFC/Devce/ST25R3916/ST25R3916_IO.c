/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2021 STMicroelectronics</center></h2>
  *
  * Licensed under ST MIX MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/mix_myliberty
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/
/******************************************************************************
 * This file contains code derived from or based on software provided by
 * STMicroelectronics.
 *
 * Original source:
 * STMicroelectronics X-CUBE / BSP / Middleware component
 *
 * Modifications:
 * Copyright (c) 2026 Neon Smart Studio
 * Author: Neon / Neona
 *
 * Licensed under:
 * - Original ST license: ST MIX MYLIBERTY SOFTWARE LICENSE AGREEMENT
 * - Additional modifications may be licensed separately where applicable.
 *
 * The original ST copyright and license notice are preserved below.
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"
#include "I2C/I2C_Master.h"

#include "NeonRTOS.h"

#include "ST25R3916_Def.h"

#include "ST25R3916_IO.h"

#define ST25R3916_OPTIMIZE              true                           /*!< Optimization switch: false always write value to register      */
#define ST25R3916_I2C_ADDR              (0xA0U >> 1)                   /*!< ST25R3916's default I2C 8bit address                                */
#define ST25R3916_REG_LEN               1U                             /*!< Byte length of a ST25R3916 register                            */

#define ST25R3916_WRITE_MODE            (0U << 6)                      /*!< ST25R3916 Operation Mode: Write                                */
#define ST25R3916_READ_MODE             (1U << 6)                      /*!< ST25R3916 Operation Mode: Read                                 */
#define ST25R3916_CMD_MODE              (3U << 6)                      /*!< ST25R3916 Operation Mode: Direct Command                       */
#define ST25R3916_FIFO_LOAD             (0x80U)                        /*!< ST25R3916 Operation Mode: FIFO Load                            */
#define ST25R3916_FIFO_READ             (0x9FU)                        /*!< ST25R3916 Operation Mode: FIFO Read                            */
#define ST25R3916_PT_A_CONFIG_LOAD      (0xA0U)                        /*!< ST25R3916 Operation Mode: Passive Target Memory A-Config Load  */
#define ST25R3916_PT_F_CONFIG_LOAD      (0xA8U)                        /*!< ST25R3916 Operation Mode: Passive Target Memory F-Config Load  */
#define ST25R3916_PT_TSN_DATA_LOAD      (0xACU)                        /*!< ST25R3916 Operation Mode: Passive Target Memory TSN Load       */
#define ST25R3916_PT_MEM_READ           (0xBFU)                        /*!< ST25R3916 Operation Mode: Passive Target Memory Read           */

#define ST25R3916_CMD_LEN               (1U)                           /*!< ST25R3916 CMD length                                           */
#define ST25R3916_BUF_LEN               (ST25R3916_CMD_LEN+ST25R3916_FIFO_DEPTH) /*!< ST25R3916 communication buffer: CMD + FIFO length    */

/*! Length of the interrupt registers       */
#define ST25R3916_INT_REGS_LEN          ( (ST25R3916_REG_IRQ_TARGET - ST25R3916_REG_IRQ_MAIN) + 1U )

/*! Holds current and previous interrupt callback pointer as well as current Interrupt status and mask */
typedef struct {
  void (*prevCallback)(void);      /*!< call back function for ST25R3916 interrupt          */
  void (*callback)(void);          /*!< call back function for ST25R3916 interrupt          */
  uint32_t  status;                /*!< latest interrupt status                             */
  uint32_t  mask;                  /*!< Interrupt mask. Negative mask = ST25R3916 mask regs */
} ST25R3916_Interrupt;

static ST25R3916_Interrupt st25r3916interrupt;

static NFC_OpResult NFC_ST25R3916_Map_GPIO_Error_Code(hwGPIO_OpResult error_code)
{
    switch (error_code)
    {
        case hwGPIO_OK:
            return NFC_OK;

        case hwGPIO_InvalidParameter:
            return NFC_InvalidParameter;

        case hwGPIO_PinConflict:
            return NFC_IO_Error;

        case hwGPIO_HW_Error:
            return NFC_IO_Error;

        case hwGPIO_Unsupport:
            return NFC_Unsupport;

        default:
            return NFC_IO_Error;
    }
}

#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
static NFC_OpResult NFC_ST25R3916_Map_SPI_Error_Code(hwSPI_OpResult error_code)
{
    switch (error_code)
    {
        case hwSPI_OK:
            return NFC_OK;

        case hwSPI_NotInit:
            return NFC_NotInit;

        case hwSPI_InvalidParameter:
            return NFC_InvalidParameter;

        case hwSPI_MemoryError:
            return NFC_MemoryError;

        case hwSPI_MutexTimeout:
            return NFC_MutexTimeout;

        case hwSPI_SlaveTimeout:
            return NFC_SlaveTimeout;

        default:
            return NFC_IO_Error;
    }
}
#endif

#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
static NFC_OpResult NFC_ST25R3916_Map_I2C_Error_Code(hwI2C_OpResult error_code)
{
    switch (error_code)
    {
        case hwI2C_OK:
            return NFC_OK;

        case hwI2C_NotInit:
            return NFC_NotInit;

        case hwI2C_InvalidParameter:
            return NFC_InvalidParameter;

        case hwI2C_MemoryError:
            return NFC_MemoryError;

        case hwI2C_MutexTimeout:
            return NFC_MutexTimeout;

        case hwI2C_SlaveTimeout:
            return NFC_SlaveTimeout;

        case hwI2C_BusError:
        default:
            return NFC_IO_Error;
    }
}
#endif

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ReadRegister(uint8_t reg, uint8_t *val)
{
  return ST25R3916_IO_ReadMultipleRegisters(reg, val, ST25R3916_REG_LEN);
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ReadMultipleRegisters(uint8_t reg, uint8_t *values, uint8_t length)
{
  if (length > 0U) {
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
      hwGPIO_OpResult gpio_op_status;
      hwSPI_OpResult spi_op_status;

      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }

      /* If is a space-B register send a direct command first */
      if ((reg & ST25R3916_SPACE_B) != 0U) {
          spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ST25R3916_CMD_SPACE_B_ACCESS);
          if(spi_op_status<hwSPI_OK)
          {
              return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
          }
      }

      spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ((reg & ~ST25R3916_SPACE_B) | ST25R3916_READ_MODE));
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
      spi_op_status = SPI_Master_Stream_Read(ST25R3916_SPI_INDEX, values, length);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
          
      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
      hwI2C_OpResult i2c_op_status;

      uint8_t cmd_buf[2];
      uint8_t cmd_len = 0;

      /* If is a space-B register send a direct command first */
      if ((reg & ST25R3916_SPACE_B) != 0U) {
        cmd_buf[cmd_len] = ST25R3916_CMD_SPACE_B_ACCESS;
        cmd_len++;
      }

      cmd_buf[cmd_len] = (reg & ~ST25R3916_SPACE_B) | ST25R3916_READ_MODE;
      cmd_len++;

      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, false, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
      
      i2c_op_status = I2C_Master_Read(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, values, length, true, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
#endif
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_WriteRegister(uint8_t reg, uint8_t val)
{
  uint8_t value = val;               /* MISRA 17.8: use intermediate variable */
  return ST25R3916_IO_WriteMultipleRegisters(reg, &value, ST25R3916_REG_LEN);
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_WriteMultipleRegisters(uint8_t reg, const uint8_t *values, uint8_t length)
{
  if (length > 0U) {
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
      hwGPIO_OpResult gpio_op_status;
      hwSPI_OpResult spi_op_status;

      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }

      /* If is a space-B register send a direct command first */
      if ((reg & ST25R3916_SPACE_B) != 0U) {
          spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ST25R3916_CMD_SPACE_B_ACCESS);
          if(spi_op_status<hwSPI_OK)
          {
              return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
          }
      }

      spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ((reg & ~ST25R3916_SPACE_B) | ST25R3916_WRITE_MODE));
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
      spi_op_status = SPI_Master_Stream_Write(ST25R3916_SPI_INDEX, values, length);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
          
      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
      hwI2C_OpResult i2c_op_status;

      uint8_t cmd_buf[2];
      uint8_t cmd_len = 0;

      /* If is a space-B register send a direct command first */
      if ((reg & ST25R3916_SPACE_B) != 0U) {
        cmd_buf[cmd_len] = ST25R3916_CMD_SPACE_B_ACCESS;
        cmd_len++;
      }

      cmd_buf[cmd_len] = (reg & ~ST25R3916_SPACE_B) | ST25R3916_WRITE_MODE;
      cmd_len++;

      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, false, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
      
      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, values, length, true, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
#endif
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_WriteFifo(const uint8_t *values, uint16_t length)
{
  if (length > ST25R3916_FIFO_DEPTH) {
    return NFC_InvalidParameter;
  }

  if (length > 0U) {
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
      hwGPIO_OpResult gpio_op_status;
      hwSPI_OpResult spi_op_status;

      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
    
      spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ST25R3916_FIFO_LOAD);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
      spi_op_status = SPI_Master_Stream_Write(ST25R3916_SPI_INDEX, values, length);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
          
      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
      hwI2C_OpResult i2c_op_status;

      uint8_t cmd_buf[1];
      uint8_t cmd_len = 0;

      cmd_buf[cmd_len] = ST25R3916_FIFO_LOAD;
      cmd_len++;

      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, false, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
      
      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, values, length, true, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
#endif
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ReadFifo(uint8_t *buf, uint16_t length)
{
  if (length > 0U) {
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
      hwGPIO_OpResult gpio_op_status;
      hwSPI_OpResult spi_op_status;

      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
    
      spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ST25R3916_FIFO_READ);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
      spi_op_status = SPI_Master_Stream_Read(ST25R3916_SPI_INDEX, buf, length);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
          
      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
      hwI2C_OpResult i2c_op_status;

      uint8_t cmd_buf[1];
      uint8_t cmd_len = 0;

      cmd_buf[cmd_len] = ST25R3916_FIFO_READ;
      cmd_len++;

      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, false, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
      
      i2c_op_status = I2C_Master_Read(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, buf, length, true, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
#endif
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_WritePTMem(const uint8_t *values, uint16_t length)
{
  if (length > ST25R3916_PTM_LEN) {
    return NFC_InvalidParameter;
  }

  if (length > 0U) {
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
      hwGPIO_OpResult gpio_op_status;
      hwSPI_OpResult spi_op_status;

      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
    
      spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ST25R3916_PT_A_CONFIG_LOAD);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
      spi_op_status = SPI_Master_Stream_Write(ST25R3916_SPI_INDEX, values, length);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
          
      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
      hwI2C_OpResult i2c_op_status;

      uint8_t cmd_buf[1];
      uint8_t cmd_len = 0;

      cmd_buf[cmd_len] = ST25R3916_PT_A_CONFIG_LOAD;
      cmd_len++;

      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, false, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
      
      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, values, length, true, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
#endif
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ReadPTMem(uint8_t *values, uint16_t length)
{
  uint8_t tmp[ST25R3916_REG_LEN + ST25R3916_PTM_LEN];  /* local buffer to handle prepended byte on I2C and SPI */

  if (length > 0U) {
      if (length > ST25R3916_PTM_LEN) {
        return NFC_InvalidParameter;
      }

#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
      hwGPIO_OpResult gpio_op_status;
      hwSPI_OpResult spi_op_status;

      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
    
      spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ST25R3916_PT_MEM_READ);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
      spi_op_status = SPI_Master_Stream_Read(ST25R3916_SPI_INDEX, tmp, (ST25R3916_REG_LEN + length));
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
          
      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
      hwI2C_OpResult i2c_op_status;

      uint8_t cmd_buf[1];
      uint8_t cmd_len = 0;

      cmd_buf[cmd_len] = ST25R3916_PT_MEM_READ;
      cmd_len++;

      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, false, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
      
      i2c_op_status = I2C_Master_Read(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, tmp, (uint8_t)(ST25R3916_REG_LEN + length), true, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
#endif

    /* Copy PTMem content without prepended byte */
    memcpy(values, (tmp + ST25R3916_REG_LEN), length);
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_WritePTMemF(const uint8_t *values, uint16_t length)
{
  if (length > (ST25R3916_PTM_F_LEN + ST25R3916_PTM_TSN_LEN)) {
    return NFC_InvalidParameter;
  }

  if (length > 0U) {
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
      hwGPIO_OpResult gpio_op_status;
      hwSPI_OpResult spi_op_status;

      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
    
      spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ST25R3916_PT_F_CONFIG_LOAD);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
      spi_op_status = SPI_Master_Stream_Write(ST25R3916_SPI_INDEX, values, length);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
          
      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
      hwI2C_OpResult i2c_op_status;

      uint8_t cmd_buf[1];
      uint8_t cmd_len = 0;

      cmd_buf[cmd_len] = ST25R3916_PT_F_CONFIG_LOAD;
      cmd_len++;

      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, false, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
      
      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, values, length, true, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
#endif
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_WritePTMemTSN(const uint8_t *values, uint16_t length)
{
  if (length > ST25R3916_PTM_TSN_LEN) {
    return NFC_InvalidParameter;
  }

  if (length > 0U) {
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
      hwGPIO_OpResult gpio_op_status;
      hwSPI_OpResult spi_op_status;

      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
    
      spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ST25R3916_PT_TSN_DATA_LOAD);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
      spi_op_status = SPI_Master_Stream_Write(ST25R3916_SPI_INDEX, values, length);
      if(spi_op_status<hwSPI_OK)
      {
          return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
      }
          
      gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
      if(gpio_op_status<hwGPIO_OK)
      {
          return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
      }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
      hwI2C_OpResult i2c_op_status;

      uint8_t cmd_buf[1];
      uint8_t cmd_len = 0;

      cmd_buf[cmd_len] = ST25R3916_PT_TSN_DATA_LOAD;
      cmd_len++;

      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, false, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
      
      i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, values, length, true, ST25R3916_OP_TIMEOUT);
      if(i2c_op_status<hwI2C_OK)
      {
          I2C_Master_Reset(ST25R3916_I2C_INDEX);
          return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
      }
#endif
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ExecuteCommand(uint8_t cmd)
{
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }
  
    spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, cmd | ST25R3916_CMD_MODE);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
    }
        
    gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
    hwI2C_OpResult i2c_op_status;

    uint8_t cmd_buf[1];
    uint8_t cmd_len = 0;

    cmd_buf[cmd_len] = cmd | ST25R3916_CMD_MODE;
    cmd_len++;

    i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, true, ST25R3916_OP_TIMEOUT);
    if(i2c_op_status<hwI2C_OK)
    {
        I2C_Master_Reset(ST25R3916_I2C_INDEX);
        return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
    }
#endif

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ReadTestRegister(uint8_t reg, uint8_t *val)
{
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }
  
    spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, reg | ST25R3916_READ_MODE);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
    }
    spi_op_status = SPI_Master_Stream_Read(ST25R3916_SPI_INDEX, val, ST25R3916_REG_LEN);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
    }
        
    gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
    hwI2C_OpResult i2c_op_status;

    uint8_t cmd_buf[1];
    uint8_t cmd_len = 0;

    cmd_buf[cmd_len] = reg | ST25R3916_READ_MODE;
    cmd_len++;

    i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, false, ST25R3916_OP_TIMEOUT);
    if(i2c_op_status<hwI2C_OK)
    {
        I2C_Master_Reset(ST25R3916_I2C_INDEX);
        return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
    }
    
    i2c_op_status = I2C_Master_Read(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, val, ST25R3916_REG_LEN, true, ST25R3916_OP_TIMEOUT);
    if(i2c_op_status<hwI2C_OK)
    {
        I2C_Master_Reset(ST25R3916_I2C_INDEX);
        return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
    }
#endif

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_WriteTestRegister(uint8_t reg, uint8_t val)
{
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }
  
    spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, ST25R3916_CMD_TEST_ACCESS);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
    }
    spi_op_status = SPI_Master_WriteByte(ST25R3916_SPI_INDEX, reg | ST25R3916_WRITE_MODE);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
    }
    spi_op_status = SPI_Master_Stream_Write(ST25R3916_SPI_INDEX, &val, ST25R3916_REG_LEN);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
    }
        
    gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }
#endif
#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
    hwI2C_OpResult i2c_op_status;

    uint8_t cmd_buf[2];
    uint8_t cmd_len = 0;

    cmd_buf[cmd_len] = ST25R3916_CMD_TEST_ACCESS;
    cmd_len++;

    cmd_buf[cmd_len] = reg | ST25R3916_READ_MODE;
    cmd_len++;

    cmd_buf[cmd_len] = val;
    cmd_len++;

    i2c_op_status = I2C_Master_Write(ST25R3916_I2C_INDEX, ST25R3916_I2C_ADDR, cmd_buf, cmd_len, true, ST25R3916_OP_TIMEOUT);
    if(i2c_op_status<hwI2C_OK)
    {
        I2C_Master_Reset(ST25R3916_I2C_INDEX);
        return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
    }
#endif

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ClearRegisterBits(uint8_t reg, uint8_t clear_mask)
{
  NFC_OpResult ret;
  uint8_t    rdVal;

  /* Read current reg value */
  ret = ST25R3916_IO_ReadRegister(reg, &rdVal);
  if(ret < NFC_OK)
  {
      return ret;
  }

  /* Only perform a Write if value to be written is different */
  if (ST25R3916_OPTIMIZE && (rdVal == (uint8_t)(rdVal & ~clear_mask))) {
    return NFC_OK;
  }

  /* Write new reg value */
  return ST25R3916_IO_WriteRegister(reg, (uint8_t)(rdVal & ~clear_mask));
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_SetRegisterBits(uint8_t reg, uint8_t set_mask)
{
  NFC_OpResult ret;
  uint8_t    rdVal;

  /* Read current reg value */
  ret = ST25R3916_IO_ReadRegister(reg, &rdVal);
  if(ret < NFC_OK)
  {
      return ret;
  }

  /* Only perform a Write if the value to be written is different */
  if (ST25R3916_OPTIMIZE && (rdVal == (rdVal | set_mask))) {
    return NFC_OK;
  }

  /* Write new reg value */
  return ST25R3916_IO_WriteRegister(reg, (rdVal | set_mask));
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ModifyRegister(uint8_t reg, uint8_t clr_mask, uint8_t set_mask)
{
  NFC_OpResult ret;
  uint8_t    rdVal;
  uint8_t    wrVal;

  /* Read current reg value */
  ret = ST25R3916_IO_ReadRegister(reg, &rdVal);
  if(ret < NFC_OK)
  {
      return ret;
  }

  /* Compute new value */
  wrVal  = (uint8_t)(rdVal & ~clr_mask);
  wrVal |= set_mask;

  /* Only perform a Write if the value to be written is different */
  if (ST25R3916_OPTIMIZE && (rdVal == wrVal)) {
    return NFC_OK;
  }

  /* Write new reg value */
  return ST25R3916_IO_WriteRegister(reg, wrVal);
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ChangeRegisterBits(uint8_t reg, uint8_t valueMask, uint8_t value)
{
  return ST25R3916_IO_ModifyRegister(reg, valueMask, (valueMask & value));
}

/*******************************************************************************/
NFC_OpResult ST25R3916_IO_ChangeTestRegisterBits(uint8_t reg, uint8_t valueMask, uint8_t value)
{
  NFC_OpResult ret;
  uint8_t    rdVal;
  uint8_t    wrVal;

  /* Read current reg value */
  ret = ST25R3916_IO_ReadTestRegister(reg, &rdVal);
  if(ret < NFC_OK)
  {
      return ret;
  }

  /* Compute new value */
  wrVal  = (uint8_t)(rdVal & ~valueMask);
  wrVal |= (uint8_t)(value & valueMask);

  /* Only perform a Write if the value to be written is different */
  if (ST25R3916_OPTIMIZE && (rdVal == wrVal)) {
    return NFC_OK;
  }

  /* Write new reg value */
  return ST25R3916_IO_WriteTestRegister(reg, wrVal);
}

/*******************************************************************************/
bool ST25R3916_IO_CheckReg(uint8_t reg, uint8_t mask, uint8_t val)
{
  uint8_t regVal;

  regVal = 0;
  ST25R3916_IO_ReadRegister(reg, &regVal);

  return ((regVal & mask) == val);
}

/*******************************************************************************/
bool ST25R3916_IO_IsRegValid(uint8_t reg)
{
  if (!(((int16_t)reg >= (int16_t)ST25R3916_REG_IO_CONF1) && (reg <= (ST25R3916_SPACE_B | ST25R3916_REG_IC_IDENTITY)))) {
    return false;
  }
  return true;
}

/*******************************************************************************/
void ST25R3916_IO_CheckForReceivedInterrupts(void)
{
  uint8_t  iregs[ST25R3916_INT_REGS_LEN];
  uint32_t irqStatus;

  /* Initialize iregs */
  irqStatus = ST25R3916_IRQ_MASK_NONE;
  memset(iregs, (int32_t)(ST25R3916_IRQ_MASK_ALL & 0xFFU), ST25R3916_INT_REGS_LEN);

    bool level;

    /* In case the IRQ is Edge (not Level) triggered read IRQs until done */

   do{
    GPIO_Interrupt_Pin_Read(ST25R3916_GPIO_IRQ_PIN, &level);

    ST25R3916_IO_ReadMultipleRegisters(ST25R3916_REG_IRQ_MAIN, iregs, ST25R3916_INT_REGS_LEN);

    irqStatus |= (uint32_t)iregs[0];
    irqStatus |= (uint32_t)iregs[1] << 8;
    irqStatus |= (uint32_t)iregs[2] << 16;
    irqStatus |= (uint32_t)iregs[3] << 24;
  }while (level == 1);

  /* Forward all interrupts, even masked ones to application */
  st25r3916interrupt.status |= irqStatus;
}

/*******************************************************************************/
void ST25R3916_IO_ModifyInterrupts(uint32_t clr_mask, uint32_t set_mask)
{
  uint8_t  i;
  uint32_t old_mask;
  uint32_t new_mask;

  old_mask = st25r3916interrupt.mask;
  new_mask = ((~old_mask & set_mask) | (old_mask & clr_mask));
  st25r3916interrupt.mask &= ~clr_mask;
  st25r3916interrupt.mask |= set_mask;

  for (i = 0; i < ST25R3916_INT_REGS_LEN; i++) {
    if (((new_mask >> (8U * i)) & 0xFFU) == 0U) {
      continue;
    }

    ST25R3916_IO_WriteRegister(ST25R3916_REG_IRQ_MASK_MAIN + i, (uint8_t)((st25r3916interrupt.mask >> (8U * i)) & 0xFFU));
  }
  return;
}

/*******************************************************************************/
uint32_t ST25R3916_IO_WaitForInterruptsTimed(uint32_t mask, uint16_t tmo)
{
  uint32_t tmrDelay;
  uint32_t status;

  tmrDelay = timerCalculateTimer(tmo);

  /* Run until specific interrupt has happen or the timer has expired */
  do {
    status = (st25r3916interrupt.status & mask);
  } while ((!timerIsExpired(tmrDelay) || (tmo == 0U)) && (status == 0U));

  status = st25r3916interrupt.status & mask;

  st25r3916interrupt.status &= ~status;

  return status;
}


/*******************************************************************************/
uint32_t ST25R3916_IO_GetInterrupt(uint32_t mask)
{
  uint32_t irqs;

  irqs = (st25r3916interrupt.status & mask);
  if (irqs != ST25R3916_IRQ_MASK_NONE) {
    st25r3916interrupt.status &= ~irqs;
  }

  return irqs;
}

/*******************************************************************************/
void ST25R3916_IO_ClearAndEnableInterrupts(uint32_t mask)
{
  ST25R3916_IO_GetInterrupt(mask);
  ST25R3916_IO_EnableInterrupts(mask);
}

/*******************************************************************************/
void ST25R3916_IO_EnableInterrupts(uint32_t mask)
{
  ST25R3916_IO_ModifyInterrupts(mask, 0);
}

/*******************************************************************************/
void ST25R3916_IO_DisableInterrupts(uint32_t mask)
{
  ST25R3916_IO_ModifyInterrupts(0, mask);
}

/*******************************************************************************/
void ST25R3916_IO_ClearInterrupts(void)
{
  uint8_t iregs[ST25R3916_INT_REGS_LEN];

  ST25R3916_IO_ReadMultipleRegisters(ST25R3916_REG_IRQ_MAIN, iregs, ST25R3916_INT_REGS_LEN);

  st25r3916interrupt.status = ST25R3916_IRQ_MASK_NONE;

  return;
}

/*******************************************************************************/
void ST25R3916_IO_IRQCallbackSet(void (*cb)(void))
{
  st25r3916interrupt.prevCallback = st25r3916interrupt.callback;
  st25r3916interrupt.callback     = cb;
}

/*******************************************************************************/
void ST25R3916_IO_IRQCallbackRestore(void)
{
  st25r3916interrupt.callback     = st25r3916interrupt.prevCallback;
  st25r3916interrupt.prevCallback = NULL;
}

void ST25R3916_IO_Interrupt_PendingFunctionCall(void *p1, uint32_t p2)
{
  ST25R3916_IO_CheckForReceivedInterrupts();

  // Check if callback is set and run it
  if (NULL != st25r3916interrupt.callback) {
    st25r3916interrupt.callback();
  }
}

/*******************************************************************************/
void ST25R3916_IO_Interrupt_Handler(hwGPIO_Int_Pin pin, hwGPIO_Interrupt_Action action)
{
  NeonRTOS_PendingFunctionCall(ST25R3916_IO_Interrupt_PendingFunctionCall, NULL, 0);
}

NFC_OpResult ST25R3916_IO_Init()
{
    hwGPIO_OpResult gpio_op_status;

    st25r3916interrupt.callback     = NULL;
    st25r3916interrupt.prevCallback = NULL;
    st25r3916interrupt.status       = ST25R3916_IRQ_MASK_NONE;
    st25r3916interrupt.mask         = ST25R3916_IRQ_MASK_NONE;
    
    gpio_op_status = GPIO_Interrupt_Init(ST25R3916_GPIO_IRQ_PIN, hwGPIO_Interrupt_Mode_Rising_Edge);
    if (gpio_op_status < hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Register_Interrupt_Handler(ST25R3916_GPIO_IRQ_PIN, ST25R3916_IO_Interrupt_Handler);
    if (gpio_op_status < hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_Enable(ST25R3916_GPIO_IRQ_PIN);
    if (gpio_op_status < hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }

#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
    hwSPI_OpResult spi_op_status;

    gpio_op_status = GPIO_Pin_Init(ST25R3916_GPIO_CS_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R3916_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_Init(ST25R3916_SPI_INDEX, ST25R3916_SPI_CLOCK, hwSPI_OpMode_Polarity0_Phase1, false);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
    }
#endif

#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
    hwI2C_OpResult i2c_op_status;

    i2c_op_status = I2C_Master_Init(ST25R3916_I2C_INDEX, ST25R3916_I2C_SPEED_MODE);
    if(i2c_op_status<hwI2C_OK)
    {
        return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
    }
#endif

    return NFC_OK;
}

NFC_OpResult ST25R3916_IO_DeInit()
{
    hwGPIO_OpResult gpio_op_status;

#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_SPI
    hwSPI_OpResult spi_op_status;

    spi_op_status = SPI_Master_DeInit(ST25R3916_SPI_INDEX);
    if (spi_op_status < hwSPI_OK)
    {
        return NFC_ST25R3916_Map_SPI_Error_Code(spi_op_status);
    }

    gpio_op_status = GPIO_Pin_DeInit(ST25R3916_GPIO_CS_PIN);
    if (gpio_op_status < hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }
#endif

#if ST25R3916_IO_INTERFACE==ST25R3916_IO_INTERFACE_I2C
    hwI2C_OpResult i2c_op_status;

    i2c_op_status = I2C_Master_DeInit(ST25R3916_I2C_INDEX);
    if(i2c_op_status<hwI2C_OK)
    {
        return NFC_ST25R3916_Map_I2C_Error_Code(i2c_op_status);
    }
#endif

    gpio_op_status = GPIO_Interrupt_DeInit(ST25R3916_GPIO_IRQ_PIN);
    if (gpio_op_status < hwGPIO_OK)
    {
        return NFC_ST25R3916_Map_GPIO_Error_Code(gpio_op_status);
    }

    return NFC_OK;
}
