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

#include "NeonRTOS.h"

#include "ST25R95_Def.h"

#include "ST25R95_IO.h"

static uint8_t ST25R95_CommandIDN[] = {ST25R95_COMMAND_IDN, 0x00};

static uint8_t ProtocolSelectCommandFieldOff[]     = {0x02, 0x02, 0x00, 0x00};
static uint8_t ProtocolSelectCommandISO15693[]     = {0x02, 0x02, 0x01, 0x0D};
static uint8_t ProtocolSelectCommandISO14443A[]    = {0x02, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t ProtocolSelectCommandISO14443B[]    = {0x02, 0x05, 0x03, 0x01, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t ProtocolSelectCommandISO18092[]     = {0x02, 0x05, 0x04, 0x51, 0x1F, 0x06, 0x00, 0x00}; /* WA: keep len=5 & do not use DD */
static uint8_t ProtocolSelectCommandCEISO14443A[]  = {0x02, 0x02, 0x12, 0x0A};

static uint8_t *ProtocolSelectCommands[6] = {
  ProtocolSelectCommandFieldOff,
  ProtocolSelectCommandISO15693,
  ProtocolSelectCommandISO14443A,
  ProtocolSelectCommandISO14443B,
  ProtocolSelectCommandISO18092,
  ProtocolSelectCommandCEISO14443A,
};

static uint8_t WrRegAnalogRegConfigISO15693[]   = {0x09, 0x04, 0x68, 0x01, 0x01, 0x53};
static uint8_t WrRegAnalogRegConfigISO14443A[]  = {0x09, 0x04, 0x68, 0x01, 0x01, 0xD3};
static uint8_t WrRegAnalogRegConfigISO14443B[]  = {0x09, 0x04, 0x68, 0x01, 0x01, 0x30};
static uint8_t WrRegAnalogRegConfigISO18092[]   = {0x09, 0x04, 0x68, 0x01, 0x01, 0x50};
static uint8_t WrRegAnalogRegConfigCEISO1443A[] = {0x09, 0x04, 0x68, 0x01, 0x04, 0x27};
static uint8_t *WrRegAnalogRegConfigs[6] = {
  NULL,
  WrRegAnalogRegConfigISO15693,
  WrRegAnalogRegConfigISO14443A,
  WrRegAnalogRegConfigISO14443B,
  WrRegAnalogRegConfigISO18092,
  WrRegAnalogRegConfigCEISO1443A
};

static uint8_t WrRegEnableAutoDetectFilter[] = {0x09, 0x04, 0x0A, 0x01, 0x02, 0xA1};
static uint8_t WrRegTimerWindowValue[]       = {0x09, 0x04, 0x3A, 0x00, 0x58, 0x04};

static uint8_t Calibrate[] = {ST25R95_COMMAND_IDLE, 0x0E, 0x03, 0xA1, 0x00, 0xB8, 0x01, 0x18, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x3F, 0x01};
static uint8_t WrRegAnalogRegConfigIndex[]  = {0x09, 0x03, 0x68, 0x00, 0x01};
static uint8_t RdRegAnalogRegConfig[]       = {0x08, 0x03, 0x69, 0x01, 0x00};

static uint8_t EchoCommand[1] = {ST25R95_COMMAND_ECHO};
static uint8_t Idle[] = {ST25R95_COMMAND_IDLE, 0x0E, 0x0A, 0x21, 0x00, 0x38, 0x01, 0x18, 0x00, 0x20, 0x60, 0x60, 0x74, 0x84, 0x3F, 0x00};

static NeonRTOS_SyncObj_t ST25R95_IRQ_Out_SyncHandle;

static ST25R95_BitRate ioCurrenTxBitRate = ST25R95_BitRate_KEEP;
static ST25R95_BitRate ioCurrenRxBitRate = ST25R95_BitRate_KEEP;

static ST25R95_OpResult ST25R95_Map_GPIO_Error_Code(hwGPIO_OpResult error_code)
{
    switch (error_code)
    {
        case hwGPIO_OK:
            return ST25R95_OK;

        case hwGPIO_InvalidParameter:
            return ST25R95_InvalidParameter;

        case hwGPIO_PinConflict:
            return ST25R95_IO_Error;

        case hwGPIO_HW_Error:
            return ST25R95_IO_Error;

        case hwGPIO_Unsupport:
            return ST25R95_Unsupport;

        default:
            return ST25R95_IO_Error;
    }
}

static ST25R95_OpResult ST25R95_Map_I2C_Error_Code(hwSPI_OpResult error_code)
{
    switch (error_code)
    {
        case hwSPI_OK:
            return ST25R95_OK;

        case hwSPI_NotInit:
            return ST25R95_NotInit;

        case hwSPI_InvalidParameter:
            return ST25R95_InvalidParameter;

        case hwSPI_MemoryError:
            return ST25R95_MemoryError;

        case hwSPI_MutexTimeout:
            return ST25R95_MutexTimeout;

        case hwSPI_SlaveTimeout:
            return ST25R95_SlaveTimeout;

        default:
            return ST25R95_IO_Error;
    }
}

void ST25R95_IO_Interrupt_Handler(hwGPIO_Int_Pin pin, hwGPIO_Interrupt_Action action)
{
    NeonRTOS_SyncObjSignalFromISR(&ST25R95_IRQ_Out_SyncHandle);
}

ST25R95_OpResult ST25R95_IO_SPI_Wait_Read(NeonRTOS_Time_t timeout)
{
    if(NeonRTOS_SyncObjWait(&ST25R95_IRQ_Out_SyncHandle, timeout)!=NeonRTOS_OK)
    {
      return ST25R95_SlaveTimeout;
    }

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_SPI_Wait_Send(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;
    uint8_t response;

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_POLL);
    if(spi_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    spi_op_status = SPI_Master_TransferByteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_POLL, &response);
    if(spi_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    if (!ST25R95_POLL_DATA_CAN_BE_SEND(response)) {
        return ST25R95_SlaveTimeout;
    }

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_SPI_Send_Command_Type_And_Len(uint8_t *cmd, uint8_t *resp, uint16_t respBuffLen)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;
    uint32_t len;

    if (respBuffLen < 2) {
      return ST25R95_InvalidParameter;
    } 

    resp[ST25R95_CMD_RESULT_OFFSET] = ST25R95_ERRCODE_COMERROR;
    resp[ST25R95_CMD_LENGTH_OFFSET] = 0x00;

    /* 1 - Send the  command */
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
    if(spi_op_status<hwGPIO_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    spi_op_status = SPI_Master_Stream_Write(ST25R95_SPI_INDEX, cmd, cmd[ST25R95_CMD_LENGTH_OFFSET] + 2);
    if(spi_op_status<hwGPIO_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }
    
    ST25R95_OpResult op_status;

    /* 2 - Poll the ST25R95 until it is ready to transmit */
    op_status = ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_TIMEOUT);

    if (op_status == ST25R95_OK)
    {
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
        if(gpio_op_status<hwGPIO_OK)
        {
            return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_READ);
        if(op_status < ST25R95_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
            return op_status;
        }
        
        op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &resp[ST25R95_CMD_RESULT_OFFSET]);
        if(op_status < ST25R95_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
            return op_status;
        }
        
        op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, resp[ST25R95_CMD_RESULT_OFFSET], &resp[ST25R95_CMD_LENGTH_OFFSET]);
        if(op_status < ST25R95_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
            return op_status;
        }
        
        len = resp[ST25R95_CMD_LENGTH_OFFSET];

        /* compute len according to CR95HF DS § 4.4 */
        if ((resp[ST25R95_CMD_RESULT_OFFSET] & 0x8F) == 0x80) {
            len |= (((uint32_t)resp[ST25R95_CMD_RESULT_OFFSET]) & 0x60U) << 3U;
        }

        /* read the len-bytes frame */
        if (respBuffLen >= (len + 2))
        {
            if (len != 0)
            {
                op_status = SPI_Master_Stream_Write(ST25R95_SPI_INDEX, &resp[ST25R95_CMD_DATA_OFFSET], len);
                if(op_status < ST25R95_OK)
                {
                    GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
                    return op_status;
                }
            }
        }
        else
        {
            op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
            if(op_status < ST25R95_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
                return op_status;
            }
            
            gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
            if(gpio_op_status<hwGPIO_OK)
            {
                return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
            }
    
            return ST25R95_MemoryError;
        }
        
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        if(gpio_op_status<hwGPIO_OK)
        {
            return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        return ST25R95_OK;
    }
    else
    {
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
        if(gpio_op_status<hwGPIO_OK)
        {
            return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
        if(op_status < ST25R95_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
            return op_status;
        }
            
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        if(gpio_op_status<hwGPIO_OK)
        {
            return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        return ST25R95_System;
    }
}

ST25R95_OpResult ST25R95_IO_SPI_Command_Echo(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    ST25R95_OpResult op_status;

    uint8_t respBuffer[ST25R95_ECHO_RESPONSE_BUFLEN];

    /* 0 - Poll the ST25R95 to make sure data can be send */
    /* Used only in cas of ECHO Command as this command is sent just after the ST25R95 reset */
    op_status = ST25R95_IO_SPI_Wait_Send();
    if (op_status < ST25R95_OK) {
      return op_status;
    }

    /* 1 - Send the echo command */
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, EchoCommand[0]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    /* 2 - Poll the ST25R95 until it is ready to transmit */
    op_status = ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_TIMEOUT);
    if (op_status < ST25R95_OK) {
      return op_status;
    }

    /* 3 - Read echo response */
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_READ);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &respBuffer[ST25R95_CMD_RESULT_OFFSET]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    /* Read 2 additional bytes. See  ST95HF DS §5.7 :
    * The ECHO command (0x55) allows exiting Listening mode.
    * In response to the ECHO command, the ST25R95 sends 0x55 + 0x8500 (error code of the Listening state cancelled by the MCU).
    */
   
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &respBuffer[1]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &respBuffer[2]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    if (respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_COMMAND_ECHO)
    {
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
        if(gpio_op_status<hwGPIO_OK)
        {
            return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }
        
        spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
        if(spi_op_status < hwSPI_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
            return ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
        
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        if(gpio_op_status<hwGPIO_OK)
        {
            return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        return ST25R95_System;
    }
    
    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_SPI_Send_Transmit_Flag(ST25R95_Protocol protocol, uint8_t transmitFlag)
{
    if ((protocol == ST25R95_Protocol_ISO14443A) || (protocol == ST25R95_Protocol_CE_ISO14443A))
    {
        hwGPIO_OpResult gpio_op_status;
        hwSPI_OpResult spi_op_status;

        /* send transmission Flag */
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, transmitFlag);
        if(spi_op_status < hwSPI_OK)
        {
            return ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
        
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        if(gpio_op_status<hwGPIO_OK)
        {
            return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        return ST25R95_OK;
    }
    else
    {
        return ST25R95_InvalidParameter;
    }
}

ST25R95_OpResult ST25R95_IO_SPI_Send_Data(uint8_t *buf, uint8_t bufLen, ST25R95_Protocol protocol, uint32_t flags)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    uint8_t len;

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
    if(spi_op_status < hwSPI_OK)
    {
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    if (protocol == ST25R95_Protocol_CE_ISO14443A) {
        /* Card Emulation mode */
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
        if(spi_op_status < hwSPI_OK)
        {
            return ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    }
    else
    {
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_COMMAND_SENDRECV);
        if(spi_op_status < hwSPI_OK)
        {
            return ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    }

    /* add transmission Flag Len in case of 14443A */
    len = ((protocol == ST25R95_Protocol_ISO14443A) || (protocol == ST25R95_Protocol_CE_ISO14443A)) ? bufLen + 1 : bufLen;
    
    /* add SoD len in case of ISO14443A + NFCIP1 */
    len += ((protocol == ST25R95_Protocol_ISO14443A) && ((flags & ST25R95_TXRX_FLAGS_NFCIP1_ON) == ST25R95_TXRX_FLAGS_NFCIP1_ON)) ? 2 : 0;
    
    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, len);
    if(spi_op_status < hwSPI_OK)
    {
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    if ((protocol == ST25R95_Protocol_ISO14443A) && ((flags & ST25R95_TXRX_FLAGS_NFCIP1_ON) == ST25R95_TXRX_FLAGS_NFCIP1_ON))
    {
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, 0xF0U);
        if(spi_op_status < hwSPI_OK)
        {
            return ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    
        /* DP 2.0 17.4.1.3 The SoD SHALL contain a length byte LEN at the position shown in Figure 43 with a value equal to n+1, where n indicates the number of bytes the payload consists of.*/
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, bufLen + 1);
        if(spi_op_status < hwSPI_OK)
        {
            return ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    }

    spi_op_status = SPI_Master_Stream_Write(ST25R95_SPI_INDEX, buf, bufLen);
    if(spi_op_status < hwSPI_OK)
    {
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_SPI_Complete_Rx(ST25R95_Protocol protocol, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen, uint32_t flags, uint8_t *additionalRespBytes)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    ST25R95_OpResult op_status;

    uint8_t Result;
    uint16_t len;
    uint16_t rcvdLen;
    uint16_t additionalRespBytesNb = 1;

   bool rmvCRC = ((flags & ST25R95_TXRX_FLAGS_CRC_RX_KEEP) != ST25R95_TXRX_FLAGS_CRC_RX_KEEP);
   bool NFCIP1 = ((protocol == ST25R95_Protocol_ISO14443A) && ((flags & ST25R95_TXRX_FLAGS_NFCIP1_ON) == ST25R95_TXRX_FLAGS_NFCIP1_ON));

   uint8_t BufCRC[2];                   /*!< BufCRC                          */
   uint8_t NFCIP1_SoD[1];               /*!< NFCIP1_SoD                      */

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_READ);
    if(spi_op_status < ST25R95_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &Result);
    if(spi_op_status < ST25R95_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &len);
    if(spi_op_status < ST25R95_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    /* compute len according to CR95HF DS § 4.4 */
    if ((Result & 0x8F) == 0x80) {
      len |= (((uint32_t)Result) & 0x60) << 3;
      Result &= 0x9F;
    }

    rcvdLen = 0;

    op_status = ST25R95_OK;
    switch (Result) {
      case ST25R95_ERRCODE_NONE:
      case ST25R95_ERRCODE_FRAMEOKADDITIONALINFO:
      case ST25R95_ERRCODE_RESULTSRESIDUAL:
        break;
      case ST25R95_ERRCODE_COMERROR:
        op_status = ST25R95_InternalError;
        break;
      case ST25R95_ERRCODE_FRAMEWAITTIMEOUT:
        op_status = ST25R95_FrameTimeout;
        break;
      case ST25R95_ERRCODE_OVERFLOW:
        op_status = ST25R95_Hw_OverRun;
        break;
      case ST25R95_ERRCODE_INVALIDSOF:
      case ST25R95_ERRCODE_RECEPTIONLOST:
      case ST25R95_ERRCODE_FRAMING:
      case ST25R95_ERRCODE_EGT:
      case ST25R95_ERRCODE_61_SOF:
      case ST25R95_ERRCODE_63_SOF_HIGH:
      case ST25R95_ERRCODE_65_SOF_LOW:
      case ST25R95_ERRCODE_66_EGT:
      case ST25R95_ERRCODE_67_TR1TOOLONG:
      case ST25R95_ERRCODE_68_TR1TOOSHORT:
        op_status = ST25R95_FramingError;
        break;
      case ST25R95_ERRCODE_62_CRC:
        op_status = ST25R95_CRC_Error;
        break;
      case ST25R95_ERRCODE_NOFIELD:
        op_status = ST25R95_LinkLose;
        break;
      default:
        op_status = ST25R95_System;
        break;
    }

    if ((op_status != ST25R95_OK) && (len != 0)) {
      spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
      if(spi_op_status < ST25R95_OK)
      {
          GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
          return ST25R95_Map_SPI_Error_Code(spi_op_status);
      }
      len = 0;
    }

    /* In ISO14443A 106kbps 2 additional bytes of collision information are provided */
    if ((protocol == ST25R95_Protocol_ISO14443A) && (ioCurrenRxBitRate == ST25R95_BitRate_106)) {
      additionalRespBytesNb += 2;
    }

    /* read the frame */
    do {
        if (len == 0) {
          additionalRespBytesNb = 0;
          break;
        }

        if (len < additionalRespBytesNb) {
          /* Flush ST25R95 fifo */
            spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
            if(spi_op_status < ST25R95_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
                return ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
            op_status = ST25R95_System;
            break;
        }

        len -= additionalRespBytesNb;

        if ((Result == ST25R95_ERRCODE_RESULTSRESIDUAL) && (protocol == ST25R95_Protocol_ISO14443A)) {
          rmvCRC = false;
        }

        if ((rmvCRC) && (protocol != ST25R95_Protocol_ISO18092)) {
          if (len < 2) {
            /* Flush ST25R95 fifo */
            spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
            if(spi_op_status < ST25R95_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
                return ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
            additionalRespBytesNb = 0;
            op_status = ST25R95_System;
            break;
          }
          len -= 2;
        }

        if ((NFCIP1) && (len >= 1)) {
            spi_op_status = SPI_Master_ReadByte(ST25R95_SPI_INDEX, NFCIP1_SoD);
            if(spi_op_status < ST25R95_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
                return ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
            len -= 1;
        }

        if ((len > rxBufLen) ||
            ((protocol == ST25R95_Protocol_ISO18092) && ((len + 1U) > rxBufLen)) || /* Need one extra byte room to prepend Len byte in rxBuf in case of Felica */
            ((!rmvCRC) && (protocol == ST25R95_Protocol_ISO18092) && ((len + 3U) > rxBufLen)))
            {
                /* same + 2 extra bytes room to append CRC */
              /* Flush ST25R95 fifo */
              spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
              if(spi_op_status < ST25R95_OK)
              {
                  GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
                  return ST25R95_Map_SPI_Error_Code(spi_op_status);
              }
              additionalRespBytesNb = 0;
              op_status = ST25R95_MemoryError;
              break;
        }

        rcvdLen = len;

        if (len != 0) {
            if (protocol == ST25R95_Protocol_ISO18092) {
                ST25R95_IO_SPIRxTx(NULL, &rxBuf[ST25R95_NFCF_LENGTH_LEN], len);
                rcvdLen += ST25R95_NFCF_LENGTH_LEN;
                len += ST25R95_NFCF_LENGTH_LEN;
                rxBuf[0] = (uint8_t)(rcvdLen & 0xFFU);
            } else {
                ST25R95_IO_SPIRxTx(NULL, rxBuf, len);
            }
        }

        if ((rmvCRC) && (protocol != ST25R95_Protocol_ISO18092)) {
            spi_op_status = SPI_Master_Stream_Read(ST25R95_SPI_INDEX, BufCRC, 2);
            if(spi_op_status < ST25R95_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
                return ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
        }

        spi_op_status = SPI_Master_Stream_Read(ST25R95_SPI_INDEX, additionalRespBytes, additionalRespBytesNb);
        if(spi_op_status < ST25R95_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
            return ST25R95_Map_SPI_Error_Code(spi_op_status);
        }

        /* check collision and CRC error */
        switch (protocol) {
          case (ST25R95_Protocol_ISO15693):
            if (ST25R95_IS_PROT_ISO15693_COLLISION_ERR(additionalRespBytes[0]))
            {
                op_status = ST25R95_RF_Collision;
            }
            else if (ST25R95_IS_PROT_ISO15693_CRC_ERR(additionalRespBytes[0]))
            {
                op_status = ST25R95_CRC_Error;
            }
            break;
          case (ST25R95_Protocol_ISO14443A):
            if (Result == ST25R95_ERRCODE_RESULTSRESIDUAL)
            {
                uint8_t errno = ST25R95_ERR_INCOMPLETE_BYTE + ((additionalRespBytes[0] & 0xFU) % 8U);

                switch(errno)
                {
                  case ST25R95_ERR_INCOMPLETE_BYTE:
                    op_status = ST25R95_ImcompleteByte_0;
                    break;
                  case ST25R95_ERR_INCOMPLETE_BYTE_01:
                    op_status = ST25R95_ImcompleteByte_1;
                    break;
                  case ST25R95_ERR_INCOMPLETE_BYTE_02:
                    op_status = ST25R95_ImcompleteByte_2;
                    break;
                  case ST25R95_ERR_INCOMPLETE_BYTE_03:
                    op_status = ST25R95_ImcompleteByte_3;
                    break;
                  case ST25R95_ERR_INCOMPLETE_BYTE_04:
                    op_status = ST25R95_ImcompleteByte_4;
                    break;
                  case ST25R95_ERR_INCOMPLETE_BYTE_05:
                    op_status = ST25R95_ImcompleteByte_5;
                    break;
                  case ST25R95_ERR_INCOMPLETE_BYTE_06:
                    op_status = ST25R95_ImcompleteByte_6;
                    break;
                  case ST25R95_ERR_INCOMPLETE_BYTE_07:
                    op_status = ST25R95_ImcompleteByte_7;
                    break;
                }
            }
            else if (ST25R95_IS_PROT_ISO14443A_COLLISION_ERR(additionalRespBytes[0]))
            {
                op_status = ST25R95_RF_Collision;
            }
            else if (ST25R95_IS_PROT_ISO14443A_PARITY_ERR(additionalRespBytes[0]))
            {
                op_status = ST25R95_ParityError;   // 你的 enum 目前還沒有這個
            }
            else if (ST25R95_IS_PROT_ISO14443A_CRC_ERR(additionalRespBytes[0]))
            {
                op_status = ST25R95_CRC_Error;
            }
            break;
          case (ST25R95_Protocol_ISO14443B):
            if (ST25R95_IS_PROT_ISO14443B_CRC_ERR(additionalRespBytes[0])) {
              op_status = ST25R95_CRC_Error;
            }
            break;
          case (ST25R95_Protocol_ISO18092):
            if (ST25R95_IS_PROT_ISO18092_CRC_ERR(additionalRespBytes[0])) {
              op_status = ST25R95_CRC_Error;
            }
            break;
          default:
            break;
        }
    } while (0);

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    if ((!rmvCRC) && (protocol == ST25R95_Protocol_ISO18092) && (rcvdLen == len)) {
      /* increase room for CRC*/
      rxBuf[rcvdLen++] = 0x00;
      rxBuf[rcvdLen++] = 0x00;
    }

    /* update *rxRcvdLen if not null pointer */
    if (rxRcvdLen != NULL) {
      (*rxRcvdLen) = rcvdLen;
    }
    
    return op_status;
}

ST25R95_OpResult ST25R95_IO_SPI_Idle(uint8_t dacDataL, uint8_t dacDataH, uint8_t WUPeriod)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    Idle[ST25R95_IDLE_WUPERIOD_OFFSET] = WUPeriod;
    Idle[ST25R95_IDLE_DACDATAL_OFFSET] = dacDataL;
    Idle[ST25R95_IDLE_DACDATAH_OFFSET] = dacDataH;
    
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_Stream_Write(ST25R95_SPI_INDEX, Idle, Idle[ST25R95_CMD_LENGTH_OFFSET] + 2);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_SPI_Get_Idle_Response(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    uint8_t respBuffer[ST25R95_IDLE_RESPONSE_BUFLEN];

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_READ);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &respBuffer[ST25R95_CMD_RESULT_OFFSET]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, respBuffer[ST25R95_CMD_RESULT_OFFSET], &respBuffer[ST25R95_CMD_LENGTH_OFFSET]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    if ((sizeof(respBuffer)) >= (respBuffer[ST25R95_CMD_LENGTH_OFFSET] + 2U))
    {
        if (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0) {
            spi_op_status = SPI_Master_Stream_Read(ST25R95_SPI_INDEX, &respBuffer[ST25R95_CMD_DATA_OFFSET], respBuffer[ST25R95_CMD_LENGTH_OFFSET]);
            if(spi_op_status < hwSPI_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
                return ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
        }
    }
    else
    {
        spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
        if(spi_op_status < hwSPI_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
            return ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_SPI_nIRQ_IN_Pulse(void)
{
    hwGPIO_OpResult gpio_op_status;

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_IN_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    // wait t0
    NeonRTOS_Sleep(1);

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_IN_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    // wait t1
    NeonRTOS_Sleep(1);

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_IN_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    // wait t3: seems more than 10ms needed
    NeonRTOS_Sleep(11);

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_SPI_Kill_Idle(void)
{
    ST25R95_OpResult op_status;

    op_status = ST25R95_IO_SPI_nIRQ_IN_Pulse();
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    /* Poll the ST25R95 until it is ready to transmit */
    op_status = ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_TIMEOUT);
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    return ST25R95_IO_SPI_Get_Idle_Response();
}

ST25R95_OpResult ST25R95_IO_SPI_Reset_Chip(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_RESET);
    if(spi_op_status<hwSPI_OK)
    {
        return ST25R95_Map_SPI_Error_Code(gpio_op_status);
    }
    
    NeonRTOS_Sleep(1);

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    NeonRTOS_Sleep(3);

    return ST25R95_IO_SPI_nIRQ_IN_Pulse();
}

ST25R95_OpResult ST25R95_IO_FieldOn(ST25R95_Protocol protocol)
{
  if (protocol == ST25R95_Protocol_FieldOff) {
    protocol = ST25R95_Protocol_ISO15693;
  }
  return (ST25R95_IO_ProtocolSelect(protocol));
}

ST25R95_OpResult ST25R95_IO_FieldOff(void)
{
  return (ST25R95_IO_ProtocolSelect(ST25R95_Protocol_FieldOff));
}

ST25R95_OpResult ST25R95_IO_SetBitRate(ST25R95_Protocol protocol, ST25R95_BitRate txBR, ST25R95_BitRate rxBR)
{
    uint8_t *conf;
    if ((protocol == ST25R95_Protocol_FieldOff) || (protocol > ST25R95_Protocol_MAX))
    {
      return ST25R95_InvalidParameter;
    }
    
    conf = &ProtocolSelectCommands[protocol][ST25R95_PROTOCOLSELECT_BR_OFFSET];
    *conf &= 0x0F;

    switch (protocol) {
      case (ST25R95_Protocol_ISO15693):
        switch (rxBR) {
          case (ST25R95_BitRate_26p48):
            break;
          case (ST25R95_BitRate_52p97):
            *conf |= 0x10;
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      case (ST25R95_Protocol_ISO14443A):
        switch (txBR) {
          case (ST25R95_BitRate_106):
            break;
          case (ST25R95_BitRate_212):
            *conf |= 0x40;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x80;
            break;
          default:
            return ST25R95_Unsupport;
        }
        switch (rxBR) {
          case (ST25R95_BitRate_106):
            break;
          case (ST25R95_BitRate_212):
            *conf |= 0x10;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x20;
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      case (ST25R95_Protocol_ISO14443B):
        switch (txBR) {
          case (ST25R95_BitRate_106):
            break;
          case (ST25R95_BitRate_212):
            *conf |= 0x40;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x80;
            break;
          case (ST25R95_BitRate_848):
            *conf |= 0xC0;
            break;
          default:
            return ST25R95_Unsupport;
        }
        switch (rxBR) {
          case (ST25R95_BitRate_106):
            break;
          case (ST25R95_BitRate_212):
            *conf |= 0x10;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x20;
            break;
          case (ST25R95_BitRate_848):
            *conf |= 0x30;
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      case (ST25R95_Protocol_ISO18092):
        switch (txBR) {
          case (ST25R95_BitRate_212):
            *conf |= 0x40;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x80;
            break;
          default:
            return ST25R95_Unsupport;
        }
        switch (rxBR) {
          case (ST25R95_BitRate_212):
            *conf |= 0x10;
            break;
          case (ST25R95_BitRate_424):
            *conf |= 0x20;
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      case (ST25R95_Protocol_CE_ISO14443A):
        switch (txBR) {
          case (ST25R95_BitRate_106):
            break;
          default:
            return ST25R95_Unsupport;
        }
        switch (rxBR) {
          case (ST25R95_BitRate_106):
            break;
          default:
            return ST25R95_Unsupport;
        }
        break;
      default:
        return ST25R95_Unsupport;
    }

    ioCurrenTxBitRate = txBR;
    ioCurrenRxBitRate = rxBR;

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_SetFWT(ST25R95_Protocol protocol, uint32_t fwt)
{
    uint8_t PP;
    uint32_t MM;
    uint32_t DD;
    uint32_t FWT;

    FWT = (fwt < ST25R95_FWT_MAX) ? fwt : ST25R95_FWT_MAX;     /* Limit the FWT to the max supported */
    fwt = FWT;
    PP = 0;

    if (protocol == ST25R95_Protocol_ISO18092) {
      /* Workaround for ST25R95_Protocol_ISO18092:
      * DD parameters seems to overwritten by MM by the ROM code.
      * So this parameter should not be used (i.e ProtocolSelect Len should be 5)
      */
      DD = 0; /* Should not be used in protocolSelect */
      while (FWT > ((128U + 1U) * (128U) * 32U)) {
        PP++;
        FWT /= 2U;
      }
      MM = FWT / (128U * 32U);
    } else {
      while (FWT > ((128 + 1) * (255) * 32)) {
        PP++;
        FWT /= 2;
      }
      do {
        if (FWT > ((64 + 1) * (255) * 32)) {
          MM = 128UL;
          break;
        }
        if (FWT > ((32 + 1) * (255) * 32)) {
          MM = 64UL;
          break;
        }
        if (FWT > ((16 + 1) * (255) * 32)) {
          MM = 32UL;
          break;
        }
        if (FWT > ((8 + 1) * (255) * 32)) {
          MM = 16UL;
          break;
        }
        if (FWT > ((4 + 1) * (255) * 32)) {
          MM = 8UL;
          break;
        }
        if (FWT > ((2 + 1) * (255) * 32)) {
          MM = 4UL;
          break;
        }
        if (FWT > ((1 + 1) * (255) * 32)) {
          MM = 2UL;
          break;
        }
        if (FWT > ((0 + 1) * (255) * 32)) {
          MM = 1UL;
          break;
        }
        MM = 0UL;
      } while (0);

      DD = (((FWT + 31UL) / 32UL) + MM) / (MM + 1UL);
      DD = (DD > 128) ? DD - 128UL : 0;
    }

    switch (protocol) {
      case ST25R95_Protocol_ISO14443A:
        if (
          (ProtocolSelectCommandISO14443A[4] != PP) ||
          (ProtocolSelectCommandISO14443A[5] != MM) ||
          (ProtocolSelectCommandISO14443A[6] != DD)) {
          ProtocolSelectCommandISO14443A[4] = PP;
          ProtocolSelectCommandISO14443A[5] = (uint8_t)MM;
          ProtocolSelectCommandISO14443A[6] = (uint8_t)DD;

          return (ST25R95_IO_ProtocolSelect(protocol));
        }
        break;

      case ST25R95_Protocol_ISO14443B:
        if (
          (ProtocolSelectCommandISO14443B[4] != PP) ||
          (ProtocolSelectCommandISO14443B[5] != MM) ||
          (ProtocolSelectCommandISO14443B[6] != DD)) {
          ProtocolSelectCommandISO14443B[4] = PP;
          ProtocolSelectCommandISO14443B[5] = (uint8_t)MM;
          ProtocolSelectCommandISO14443B[6] = (uint8_t)DD;

          return (ST25R95_IO_ProtocolSelect(protocol));
        }
        break;

      case ST25R95_Protocol_ISO18092:
        if (
          (ProtocolSelectCommandISO18092[5] != PP) ||
          (ProtocolSelectCommandISO18092[6] != MM) ||
          (ProtocolSelectCommandISO18092[7] != DD)) {
          ProtocolSelectCommandISO18092[5] = PP;
          ProtocolSelectCommandISO18092[6] = (uint8_t)MM;
          ProtocolSelectCommandISO18092[7] = (uint8_t)DD;

          return (ST25R95_IO_ProtocolSelect(protocol));
        }
        break;

      default:
        break;
    }
    return ST25R95_OK;
}

bool ST25R95_IO_CheckChipID(void)
{
    bool ret = false;
    uint8_t respBuffer[ST25R95_IDN_RESPONSE_BUFLEN];

    if (ST25R95_IO_SPI_Send_Command_Type_And_Len(ST25R95_CommandIDN, respBuffer, ST25R95_IDN_RESPONSE_BUFLEN) == ST25R95_OK)
    {
      if (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0)
      {
        ret = (strcmp((const char *)&respBuffer[ST25R95_CMD_DATA_OFFSET], "NFC FS2JAST4") == 0);
      }
    }

    return (ret);
}

ST25R95_OpResult ST25R95_IO_SetSlotCounter(uint8_t slots)
{
    if ((ProtocolSelectCommandISO18092[4] & 0xF) != slots)
    {
      ProtocolSelectCommandISO18092[4] &= 0xF0;
      ProtocolSelectCommandISO18092[4] |= (slots & 0xF);
      return (ST25R95_IO_ProtocolSelect(ST25R95_Protocol_ISO18092));
    }

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_Protocol_Select(ST25R95_Protocol protocol)
{
    ST25R95_OpResult op_status;

    uint8_t protocolResp[ST25R95_PROTOCOLSELECT_RESPONSE_BUFLEN];
    uint8_t wrregResp[ST25R95_WRREG_RESPONSE_BUFLEN];

    if (protocol > ST25R95_Protocol_CE_ISO14443A)
    {
        return ST25R95_InvalidParameter;
    }

    if (ProtocolSelectCommands[protocol] == NULL)
    {
        return ST25R95_InvalidParameter;
    }

    op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(ProtocolSelectCommands[protocol], protocolResp, ST25R95_PROTOCOLSELECT_RESPONSE_BUFLEN);
    if ((op_status == ST25R95_OK) && (protocolResp[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE)) {
      return ST25R95_InvalidParameter;
    }

    /* Adjust ARC_B or ACC_A register */
    if ((protocol != ST25R95_Protocol_FieldOff)) {
      op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegAnalogRegConfigs[protocol], wrregResp, ST25R95_WRREG_RESPONSE_BUFLEN);
      if(op_status < ST25R95_OK)
      {
          return op_status;
      }
    }

    if (protocol == ST25R95_Protocol_ISO18092) {
      op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegEnableAutoDetectFilter, wrregResp, ST25R95_WRREG_RESPONSE_BUFLEN);
      if(op_status < ST25R95_OK)
      {
          return op_status;
      }
    }
    if (protocol == ST25R95_Protocol_ISO14443A) {
      op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegTimerWindowValue, wrregResp, ST25R95_WRREG_RESPONSE_BUFLEN);
      if(op_status < ST25R95_OK)
      {
          return op_status;
      }
    }
    
    return (ST25R95_OK);
}

ST25R95_OpResult ST25R95_IO_CalibrateTagDetector(uint8_t* pCalibrate)
{
    ST25R95_OpResult op_status;

    const uint8_t steps[6] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x4U};
    uint8_t       respBuffer[ST25R95_IDLE_RESPONSE_BUFLEN];
    uint8_t       i;

    /* 8 steps dichotomy implementation as per AN3433 */

    /* Check that wake up detection is tag detect (0x02) when DacDataH is Min Dac value 0x00 */
    Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] = 0x00U;

    op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(Calibrate, respBuffer, ST25R95_IDLE_RESPONSE_BUFLEN);
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    if ((respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE) || (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0x01) || (respBuffer[ST25R95_CMD_DATA_OFFSET] != ST25R95_IDLE_WKUP_TAGDETECT)) {
        *pCalibrate = (0xFFU);
        return ST25R95_CalibrateError;
    }

    /* Check that wake up detection is timeout (0x01) when DacDataH is Max Dac value 0xFC */
    Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] = ST25R95_DACDATA_MAX;

    op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(Calibrate, respBuffer, ST25R95_IDLE_RESPONSE_BUFLEN);
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    if ((respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE) || (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0x01) || (respBuffer[ST25R95_CMD_DATA_OFFSET] != ST25R95_IDLE_WKUP_TIMEOUT)) {
        *pCalibrate = (0xFFU);
        return ST25R95_CalibrateError;
    }

    for (i = 0; i < 6; i++)
    {
        switch (respBuffer[ST25R95_CMD_DATA_OFFSET])
        {
          case ST25R95_IDLE_WKUP_TIMEOUT:
            Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] -= steps[i];
            break;
          case ST25R95_IDLE_WKUP_TAGDETECT:
            Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] += steps[i];
            break;
          default:
            return ST25R95_System;
            /*NOTREACHED*/
            break;
        }

        respBuffer[ST25R95_CMD_DATA_OFFSET] = 0x00U;

        op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(Calibrate, respBuffer, ST25R95_IDLE_RESPONSE_BUFLEN);
        if(op_status < ST25R95_OK)
        {
            return op_status;
        }
    }

    if (respBuffer[2U] == ST25R95_IDLE_WKUP_TIMEOUT) {
      Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] -= 0x04U;
    }

    *pCalibrate = (Calibrate[ST25R95_IDLE_DACDATAH_OFFSET]);

    return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_ReadReg(uint16_t reg, uint8_t *value)
{
  ST25R95_OpResult op_status;

  uint8_t respBuffer[ST25R95_RDREG_RESPONSE_BUFLEN];

  switch (reg) {
    case (ST25R95_REG_ARC_B):
      WrRegAnalogRegConfigIndex[4U] = 0x01U;
      break;

    case (ST25R95_REG_ACC_A):
      WrRegAnalogRegConfigIndex[4U] = 0x04U;
      break;

    default:
      return ST25R95_InvalidParameter;
  }

  op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegAnalogRegConfigIndex, respBuffer, ST25R95_RDREG_RESPONSE_BUFLEN);
  if(op_status < ST25R95_OK)
  {
      return op_status;
  }

  if (respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE)
  {
      return ST25R95_InvalidParameter;
  }

  op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(RdRegAnalogRegConfig, respBuffer, ST25R95_RDREG_RESPONSE_BUFLEN);
  if(op_status < ST25R95_OK)
  {
      return op_status;
  }

  if (respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE)
  {
    return ST25R95_InvalidParameter;
  }

  *value = respBuffer[2];

  return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_WriteReg(uint8_t protocol, uint16_t reg, uint8_t value)
{
  ST25R95_OpResult op_status;

  uint8_t respBuffer[ST25R95_WRREG_RESPONSE_BUFLEN];

  switch (reg) {
    case (ST25R95_REG_ARC_B):
      if ((protocol == ST25R95_Protocol_ISO15693)  ||
          (protocol == ST25R95_Protocol_ISO14443A) ||
          (protocol == ST25R95_Protocol_ISO14443B) ||
          (protocol == ST25R95_Protocol_ISO18092))
          {
              WrRegAnalogRegConfigs[protocol][5] = value;

              op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegAnalogRegConfigs[protocol], respBuffer, ST25R95_WRREG_RESPONSE_BUFLEN);
              if(op_status < ST25R95_OK)
              {
                  return op_status;
              }
              
              if(respBuffer[ST25R95_CMD_RESULT_OFFSET] != 0)
              {
                  return ST25R95_InvalidParameter;
              }
          }
          else
          {
              return ST25R95_WrongState;
          }
      break;

    case (ST25R95_REG_ACC_A):
      if (protocol == ST25R95_Protocol_CE_ISO14443A)
      {
          WrRegAnalogRegConfigs[protocol][5] = value;

          op_status = ST25R95_IO_SPI_Send_Command_Type_And_Len(WrRegAnalogRegConfigs[protocol], respBuffer, ST25R95_WRREG_RESPONSE_BUFLEN);
          
          if(op_status < ST25R95_OK)
          {
              return op_status;
          }

          if(respBuffer[ST25R95_CMD_RESULT_OFFSET] != 0)
          {
              return ST25R95_InvalidParameter;
          }
      }
      else
      {
          return ST25R95_WrongState;
      }
      break;
    default:
      return ST25R95_InvalidParameter;
  }

  return ST25R95_OK;
}

ST25R95_OpResult ST25R95_IO_Init(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    gpio_op_status = GPIO_Pin_Init(ST25R95_GPIO_IRQ_CS_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_None);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Init(ST25R95_GPIO_IRQ_IN_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_None);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_IN_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_Init(ST25R95_GPIO_IRQ_OUT_PIN, hwGPIO_Interrupt_Mode_Falling_Edge);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }
    
    gpio_op_status = GPIO_Register_Interrupt_Handler(ST25R95_GPIO_IRQ_OUT_PIN, ST25R95_IO_Interrupt_Handler);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_Enable(ST25R95_GPIO_IRQ_OUT_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Init(ST25R95_GPIO_INTERFACE_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_INTERFACE_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_Init(ST25R95_SPI_INDEX, ST25R95_SPI_CLOCK, hwSPI_OpMode_Polarity0_Phase0, false);
    if(spi_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    ST25R95_OpResult op_status;

    op_status = ST25R95_AnalogConfig_Init(); /* Initialize Analog Configs */
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    /* First perform the startup sequence */
    op_status = ST25R95_IO_SPI_nIRQ_IN_Pulse();
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    /* Reset ST25R95 */
    op_status = ST25R95_IO_SPI_Reset_Chip();
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    /* Initialize chip */
    uint32_t attempt = 5;

    /* If no answer from ECHO command, reset and retry again up to max attempt */
    while ((ST25R95_IO_SPI_Command_Echo() != ST25R95_OK) && (attempt != 0)) {
      attempt--;
      ST25R95_IO_SPI_Reset_Chip();
    }
    if (attempt == 0) {
      return ST25R95_System;
    }

    /* Check expected chip: ST25R95 */
    if (!ST25R95_CheckChipID()) {
      return ST25R95_Hw_Mismatch;
    }
}

ST25R95_OpResult ST25R95_IO_DeInit(void)
{
    ST25R95_OpResult op_status;

    /* Reset ST25R95 */
    op_status = ST25R95_IO_SPI_Reset_Chip();
    if(op_status < ST25R95_OK)
    {
        return op_status;
    }

    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    spi_op_status = SPI_Master_DeInit(ST25R95_SPI_INDEX);
    if(spi_op_status<hwGPIO_OK)
    {
        return ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    gpio_op_status = GPIO_DeInit(ST25R95_GPIO_IRQ_OUT_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }
    
    gpio_op_status = GPIO_UnRegister_Interrupt_Handler(ST25R95_GPIO_IRQ_OUT_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_Disable(ST25R95_GPIO_IRQ_OUT_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_DeInit(ST25R95_GPIO_IRQ_OUT_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_DeInit(ST25R95_GPIO_IRQ_IN_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }
    
    gpio_op_status = GPIO_DeInit(ST25R95_GPIO_IRQ_CS_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return M24SR_Map_GPIO_Error_Code(gpio_op_status);
    }
    
    return ST25R95_OK;
}
