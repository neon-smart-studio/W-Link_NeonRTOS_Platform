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

#include "ST25R95.h"

static uint8_t EchoCommand[1] = {ST25R95_COMMAND_ECHO};
static uint8_t Idle[] = {ST25R95_COMMAND_IDLE, 0x0E, 0x0A, 0x21, 0x00, 0x38, 0x01, 0x18, 0x00, 0x20, 0x60, 0x60, 0x74, 0x84, 0x3F, 0x00};

static ST25R95_BitRate ioCurrenTxBitRate = ST25R95_BitRate_KEEP;
static ST25R95_BitRate ioCurrenRxBitRate = ST25R95_BitRate_KEEP;

ST25R95_IO_SPIRxContext st25r95SPIRxCtx;

static NFC_OpResult NFC_ST25R95_Map_GPIO_Error_Code(hwGPIO_OpResult error_code)
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

static NFC_OpResult NFC_ST25R95_Map_SPI_Error_Code(hwSPI_OpResult error_code)
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

NFC_OpResult ST25R95_IO_SPI_Wait_Read(NeonRTOS_Time_t timeout)
{
    NeonRTOS_Time_t elapsed = 0;

    do 
    {
        hwGPIO_OpResult gpio_op_status;
        bool level;

        gpio_op_status = GPIO_Pin_Read(ST25R95_GPIO_IRQ_OUT_PIN, &level);
        if (gpio_op_status < hwGPIO_OK)
        {
            return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        if(level == 0)
        {
            return NFC_OK;
        }

        NeonRTOS_Sleep(1);
        elapsed++;
    }while(elapsed < timeout);

    return NFC_SlaveTimeout;
}

NFC_OpResult ST25R95_IO_SPI_Wait_Send(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;
    uint8_t response;

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_POLL);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    bool success = false;

    for (uint32_t i = 0; i < ST25R95_CONTROL_POLL_TIMEOUT; i++)
    {
        spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_POLL, &response);
        if (spi_op_status < hwSPI_OK) {
            GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
            return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
        }

        if (ST25R95_POLL_DATA_CAN_BE_SEND(response)) {
            success = true;
            break;
        }

        NeonRTOS_Sleep(1);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    if (!success) {
        return NFC_SlaveTimeout;
    }

    return NFC_OK;
}

NFC_OpResult ST25R95_IO_SPI_Send_Command_Type_And_Len(uint8_t *cmd, uint8_t *resp, uint16_t respBuffLen)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;
    uint32_t len;

    if (respBuffLen < 2) {
      return NFC_InvalidParameter;
    } 

    resp[ST25R95_CMD_RESULT_OFFSET] = ST25R95_ERRCODE_COMERROR;
    resp[ST25R95_CMD_LENGTH_OFFSET] = 0x00;

    /* 1 - Send the  command */
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
    if(spi_op_status<hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    spi_op_status = SPI_Master_Stream_Write(ST25R95_SPI_INDEX, cmd, cmd[ST25R95_CMD_LENGTH_OFFSET] + 2);
    if(spi_op_status<hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }
    
    NFC_OpResult op_status;

    /* 2 - Poll the ST25R95 until it is ready to transmit */
    op_status = ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_TIMEOUT);

    if (op_status == NFC_OK)
    {
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
        if(gpio_op_status<hwGPIO_OK)
        {
            return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_READ);
        if(op_status < NFC_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
            return op_status;
        }
        
        op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &resp[ST25R95_CMD_RESULT_OFFSET]);
        if(op_status < NFC_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
            return op_status;
        }
        
        op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, resp[ST25R95_CMD_RESULT_OFFSET], &resp[ST25R95_CMD_LENGTH_OFFSET]);
        if(op_status < NFC_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
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
                op_status = SPI_Master_Stream_Read(ST25R95_SPI_INDEX, &resp[ST25R95_CMD_DATA_OFFSET], len);
                if(op_status < NFC_OK)
                {
                    GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                    return op_status;
                }
            }
        }
        else
        {
            op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
            if(op_status < NFC_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                return op_status;
            }
            
            gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
            if(gpio_op_status<hwGPIO_OK)
            {
                return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
            }
    
            return NFC_MemoryError;
        }
        
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        if(gpio_op_status<hwGPIO_OK)
        {
            return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        return NFC_OK;
    }
    else
    {
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
        if(gpio_op_status<hwGPIO_OK)
        {
            return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
        if(op_status < NFC_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
            return op_status;
        }
            
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        if(gpio_op_status<hwGPIO_OK)
        {
            return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        return NFC_System;
    }
}

NFC_OpResult ST25R95_IO_SPI_Command_Echo(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    NFC_OpResult op_status;

    uint8_t respBuffer[ST25R95_ECHO_RESPONSE_BUFLEN];

    /* 0 - Poll the ST25R95 to make sure data can be send */
    /* Used only in cas of ECHO Command as this command is sent just after the ST25R95 reset */
    op_status = ST25R95_IO_SPI_Wait_Send();
    if (op_status < NFC_OK) {
      return op_status;
    }

    /* 1 - Send the echo command */
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, EchoCommand[0]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    /* 2 - Poll the ST25R95 until it is ready to transmit */
    op_status = ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_TIMEOUT);
    if (op_status < NFC_OK) {
      return op_status;
    }

    /* 3 - Read echo response */
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_READ);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &respBuffer[ST25R95_CMD_RESULT_OFFSET]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    /* Read 2 additional bytes. See  ST95HF DS §5.7 :
    * The ECHO command (0x55) allows exiting Listening mode.
    * In response to the ECHO command, the ST25R95 sends 0x55 + 0x8500 (error code of the Listening state cancelled by the MCU).
    */
   
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &respBuffer[1]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &respBuffer[2]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    if (respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_COMMAND_ECHO)
    {
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
        if(gpio_op_status<hwGPIO_OK)
        {
            return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }
        
        spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
        if(spi_op_status < hwSPI_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
            return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
        
        gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        if(gpio_op_status<hwGPIO_OK)
        {
            return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
        }

        return NFC_System;
    }
    
    return NFC_OK;
}

NFC_OpResult ST25R95_IO_SPI_Send_Transmit_Flag(ST25R95_Protocol protocol, uint8_t transmitFlag)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    if ((protocol == ST25R95_Protocol_ISO14443A) || (protocol == ST25R95_Protocol_CE_ISO14443A))
    {
        /* send transmission Flag */
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, transmitFlag);
        if(spi_op_status < hwSPI_OK)
        {
            return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    }
    
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }
        
    return NFC_OK;
}

NFC_OpResult ST25R95_IO_SPI_Send_Data(uint8_t *buf, uint8_t bufLen, ST25R95_Protocol protocol, uint32_t flags)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    uint8_t len;

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
    if(spi_op_status < hwSPI_OK)
    {
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    if (protocol == ST25R95_Protocol_CE_ISO14443A) {
        /* Card Emulation mode */
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
        if(spi_op_status < hwSPI_OK)
        {
            return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    }
    else
    {
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_COMMAND_SENDRECV);
        if(spi_op_status < hwSPI_OK)
        {
            return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    }

    /* add transmission Flag Len in case of 14443A */
    len = ((protocol == ST25R95_Protocol_ISO14443A) || (protocol == ST25R95_Protocol_CE_ISO14443A)) ? bufLen + 1 : bufLen;
    
    /* add SoD len in case of ISO14443A + NFCIP1 */
    len += ((protocol == ST25R95_Protocol_ISO14443A) && ((flags & ST25R95_TXRX_FLAGS_NFCIP1_ON) == ST25R95_TXRX_FLAGS_NFCIP1_ON)) ? 2 : 0;
    
    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, len);
    if(spi_op_status < hwSPI_OK)
    {
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    if ((protocol == ST25R95_Protocol_ISO14443A) && ((flags & ST25R95_TXRX_FLAGS_NFCIP1_ON) == ST25R95_TXRX_FLAGS_NFCIP1_ON))
    {
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, 0xF0U);
        if(spi_op_status < hwSPI_OK)
        {
            return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    
        /* DP 2.0 17.4.1.3 The SoD SHALL contain a length byte LEN at the position shown in Figure 43 with a value equal to n+1, where n indicates the number of bytes the payload consists of.*/
        spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, bufLen + 1);
        if(spi_op_status < hwSPI_OK)
        {
            return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    }

    spi_op_status = SPI_Master_Stream_Write(ST25R95_SPI_INDEX, buf, bufLen);
    if(spi_op_status < hwSPI_OK)
    {
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    return NFC_OK;
}

NFC_OpResult ST25R95_IO_SPI_PrepareRx(ST25R95_Protocol protocol, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen, uint32_t flags, uint8_t *additionalRespBytes)
{
    st25r95SPIRxCtx.protocol            = protocol;
    st25r95SPIRxCtx.rxBuf               = rxBuf;
    st25r95SPIRxCtx.rxBufLen            = rxBufLen;
    st25r95SPIRxCtx.rxRcvdLen           = rxRcvdLen;
    st25r95SPIRxCtx.rmvCRC              = ((flags & ST25R95_TXRX_FLAGS_CRC_RX_KEEP) != ST25R95_TXRX_FLAGS_CRC_RX_KEEP);
    st25r95SPIRxCtx.NFCIP1              = ((protocol == ST25R95_Protocol_ISO14443A) && ((flags & ST25R95_TXRX_FLAGS_NFCIP1_ON) == ST25R95_TXRX_FLAGS_NFCIP1_ON));
    st25r95SPIRxCtx.additionalRespBytes = additionalRespBytes;

    return NFC_OK;
}

NFC_OpResult ST25R95_IO_SPI_Complete_Rx()
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    NFC_OpResult op_status;

    uint8_t Result;
    uint8_t len;
    uint16_t rcvdLen;
    uint16_t additionalRespBytesNb = 1;

   uint8_t BufCRC[2];                   /*!< BufCRC                          */
   uint8_t NFCIP1_SoD[1];               /*!< NFCIP1_SoD                      */

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_READ);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &Result);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &len);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    /* compute len according to CR95HF DS § 4.4 */
    if ((Result & 0x8F) == 0x80) {
      len |= (((uint32_t)Result) & 0x60) << 3;
      Result &= 0x9F;
    }

    rcvdLen = 0;

    op_status = NFC_OK;
    switch (Result) {
      case ST25R95_ERRCODE_NONE:
      case ST25R95_ERRCODE_FRAMEOKADDITIONALINFO:
      case ST25R95_ERRCODE_RESULTSRESIDUAL:
        break;
      case ST25R95_ERRCODE_COMERROR:
        op_status = NFC_InternalError;
        break;
      case ST25R95_ERRCODE_FRAMEWAITTIMEOUT:
        op_status = NFC_FrameTimeout;
        break;
      case ST25R95_ERRCODE_OVERFLOW:
        op_status = NFC_Hw_OverRun;
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
        op_status = NFC_FramingError;
        break;
      case ST25R95_ERRCODE_62_CRC:
        op_status = NFC_CRC_Error;
        break;
      case ST25R95_ERRCODE_NOFIELD:
        op_status = NFC_LinkLoss;
        break;
      default:
        op_status = NFC_System;
        break;
    }

    if ((op_status != NFC_OK) && (len != 0)) {
      spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
      if(spi_op_status < hwSPI_OK)
      {
          GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
          return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
      }
      len = 0;
    }

    /* In ISO14443A 106kbps 2 additional bytes of collision information are provided */
    if ((st25r95SPIRxCtx.protocol == ST25R95_Protocol_ISO14443A) && (ioCurrenRxBitRate == ST25R95_BitRate_106)) {
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
            if(spi_op_status < hwSPI_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
            op_status = NFC_System;
            break;
        }

        len -= additionalRespBytesNb;

        if ((Result == ST25R95_ERRCODE_RESULTSRESIDUAL) && (st25r95SPIRxCtx.protocol == ST25R95_Protocol_ISO14443A)) {
          st25r95SPIRxCtx.rmvCRC = false;
        }

        if ((st25r95SPIRxCtx.rmvCRC) && (st25r95SPIRxCtx.protocol != ST25R95_Protocol_ISO18092)) {
          if (len < 2) {
            /* Flush ST25R95 fifo */
            spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
            if(spi_op_status < hwSPI_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
            additionalRespBytesNb = 0;
            op_status = NFC_System;
            break;
          }
          len -= 2;
        }

        if ((st25r95SPIRxCtx.NFCIP1) && (len >= 1)) {
            spi_op_status = SPI_Master_ReadByte(ST25R95_SPI_INDEX, NFCIP1_SoD);
            if(spi_op_status < hwSPI_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
            len -= 1;
        }

        if ((len > st25r95SPIRxCtx.rxBufLen) ||
            ((st25r95SPIRxCtx.protocol == ST25R95_Protocol_ISO18092) && ((len + 1U) > st25r95SPIRxCtx.rxBufLen)) || /* Need one extra byte room to prepend Len byte in rxBuf in case of Felica */
            ((!st25r95SPIRxCtx.rmvCRC) && (st25r95SPIRxCtx.protocol == ST25R95_Protocol_ISO18092) && ((len + 3U) > st25r95SPIRxCtx.rxBufLen)))
            {
                /* same + 2 extra bytes room to append CRC */
              /* Flush ST25R95 fifo */
              spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
              if(spi_op_status < hwSPI_OK)
              {
                  GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                  return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
              }
              additionalRespBytesNb = 0;
              op_status = NFC_MemoryError;
              break;
        }

        rcvdLen = len;
        if (len != 0) {
            if (st25r95SPIRxCtx.protocol == ST25R95_Protocol_ISO18092) {
                spi_op_status = SPI_Master_Stream_Read(ST25R95_SPI_INDEX, &st25r95SPIRxCtx.rxBuf[ST25R95_NFCF_LENGTH_LEN], len);
                if(spi_op_status < hwSPI_OK)
                {
                    GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                    return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
                }
                rcvdLen += ST25R95_NFCF_LENGTH_LEN;
                len += ST25R95_NFCF_LENGTH_LEN;
                st25r95SPIRxCtx.rxBuf[0] = (uint8_t)(rcvdLen & 0xFFU);
            } else {
                spi_op_status = SPI_Master_Stream_Read(ST25R95_SPI_INDEX, st25r95SPIRxCtx.rxBuf, len);
                if(spi_op_status < hwSPI_OK)
                {
                    GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                    return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
                }
            }
        }

        if ((st25r95SPIRxCtx.rmvCRC) && (st25r95SPIRxCtx.protocol != ST25R95_Protocol_ISO18092)) {
            spi_op_status = SPI_Master_Stream_Read(ST25R95_SPI_INDEX, BufCRC, 2);
            if(spi_op_status < hwSPI_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
        }

        spi_op_status = SPI_Master_Stream_Read(ST25R95_SPI_INDEX, st25r95SPIRxCtx.additionalRespBytes, additionalRespBytesNb);
        if(spi_op_status < hwSPI_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
            return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
        }

        /* check collision and CRC error */
        switch (st25r95SPIRxCtx.protocol) {
          case (ST25R95_Protocol_ISO15693):
            if (ST25R95_IS_PROT_ISO15693_COLLISION_ERR(st25r95SPIRxCtx.additionalRespBytes[0]))
            {
                op_status = NFC_RF_Collision;
            }
            else if (ST25R95_IS_PROT_ISO15693_CRC_ERR(st25r95SPIRxCtx.additionalRespBytes[0]))
            {
                op_status = NFC_CRC_Error;
            }
            break;
          case (ST25R95_Protocol_ISO14443A):
            if (Result == ST25R95_ERRCODE_RESULTSRESIDUAL)
            {
                uint8_t errno = ST25R95_ERR_INCOMPLETE_BYTE + ((st25r95SPIRxCtx.additionalRespBytes[0] & 0xFU) % 8U);

                switch(errno)
                {
                  case ST25R95_ERR_INCOMPLETE_BYTE:
                  case ST25R95_ERR_INCOMPLETE_BYTE_01:
                  case ST25R95_ERR_INCOMPLETE_BYTE_02:
                  case ST25R95_ERR_INCOMPLETE_BYTE_03:
                  case ST25R95_ERR_INCOMPLETE_BYTE_04:
                  case ST25R95_ERR_INCOMPLETE_BYTE_05:
                  case ST25R95_ERR_INCOMPLETE_BYTE_06:
                  case ST25R95_ERR_INCOMPLETE_BYTE_07:
                    op_status = NFC_ImcompleteByte;
                    break;
                }
            }
            else if (ST25R95_IS_PROT_ISO14443A_COLLISION_ERR(st25r95SPIRxCtx.additionalRespBytes[0]))
            {
                op_status = NFC_RF_Collision;
            }
            else if (ST25R95_IS_PROT_ISO14443A_PARITY_ERR(st25r95SPIRxCtx.additionalRespBytes[0]))
            {
                op_status = NFC_ParityError;   // 你的 enum 目前還沒有這個
            }
            else if (ST25R95_IS_PROT_ISO14443A_CRC_ERR(st25r95SPIRxCtx.additionalRespBytes[0]))
            {
                op_status = NFC_CRC_Error;
            }
            break;
          case (ST25R95_Protocol_ISO14443B):
            if (ST25R95_IS_PROT_ISO14443B_CRC_ERR(st25r95SPIRxCtx.additionalRespBytes[0])) {
              op_status = NFC_CRC_Error;
            }
            break;
          case (ST25R95_Protocol_ISO18092):
            if (ST25R95_IS_PROT_ISO18092_CRC_ERR(st25r95SPIRxCtx.additionalRespBytes[0])) {
              op_status = NFC_CRC_Error;
            }
            break;
          default:
            break;
        }
    } while (0);

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    if ((!st25r95SPIRxCtx.rmvCRC) && (st25r95SPIRxCtx.protocol == ST25R95_Protocol_ISO18092) && (rcvdLen == len)) {
      /* increase room for CRC*/
      st25r95SPIRxCtx.rxBuf[rcvdLen++] = 0x00;
      st25r95SPIRxCtx.rxBuf[rcvdLen++] = 0x00;
    }

    /* update *rxRcvdLen if not null pointer */
    if (st25r95SPIRxCtx.rxRcvdLen != NULL) {
      (*st25r95SPIRxCtx.rxRcvdLen) = rcvdLen;
    }
    
    return op_status;
}

NFC_OpResult ST25R95_IO_SPI_Idle(uint8_t dacDataL, uint8_t dacDataH, uint8_t WUPeriod)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    Idle[ST25R95_IDLE_WUPERIOD_OFFSET] = WUPeriod;
    Idle[ST25R95_IDLE_DACDATAL_OFFSET] = dacDataL;
    Idle[ST25R95_IDLE_DACDATAH_OFFSET] = dacDataH;
    
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_SEND);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_Stream_Write(ST25R95_SPI_INDEX, Idle, Idle[ST25R95_CMD_LENGTH_OFFSET] + 2);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    return NFC_OK;
}

NFC_OpResult ST25R95_IO_SPI_Get_Idle_Response(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    uint8_t respBuffer[ST25R95_IDLE_RESPONSE_BUFLEN];

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_READ);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, ST25R95_SPI_DUMMY_BYTE, &respBuffer[ST25R95_CMD_RESULT_OFFSET]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    spi_op_status = SPI_Master_TransferByte(ST25R95_SPI_INDEX, respBuffer[ST25R95_CMD_RESULT_OFFSET], &respBuffer[ST25R95_CMD_LENGTH_OFFSET]);
    if(spi_op_status < hwSPI_OK)
    {
        GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    if ((sizeof(respBuffer)) >= (respBuffer[ST25R95_CMD_LENGTH_OFFSET] + 2U))
    {
        if (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0) {
            spi_op_status = SPI_Master_Stream_Read(ST25R95_SPI_INDEX, &respBuffer[ST25R95_CMD_DATA_OFFSET], respBuffer[ST25R95_CMD_LENGTH_OFFSET]);
            if(spi_op_status < hwSPI_OK)
            {
                GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
                return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
            }
        }
    }
    else
    {
        spi_op_status = SPI_Master_DummyBytes(ST25R95_SPI_INDEX, ST25R95_COMMUNICATION_BUFFER_SIZE);
        if(spi_op_status < hwSPI_OK)
        {
            GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
            return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
        }
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    return NFC_OK;
}

NFC_OpResult ST25R95_IO_SPI_nIRQ_IN_Pulse(void)
{
    hwGPIO_OpResult gpio_op_status;

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_IN_PIN, 1);
    if (gpio_op_status < hwGPIO_OK) return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);

    NeonRTOS_Sleep(1);

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_IN_PIN, 0);
    if (gpio_op_status < hwGPIO_OK) return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);

    NeonRTOS_Sleep(1);

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_IN_PIN, 1);
    if (gpio_op_status < hwGPIO_OK) return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);

    NeonRTOS_Sleep(11);

    return NFC_OK;
}

NFC_OpResult ST25R95_IO_SPI_Kill_Idle(void)
{
    NFC_OpResult op_status;

    op_status = ST25R95_IO_SPI_nIRQ_IN_Pulse();
    if(op_status < NFC_OK)
    {
        return op_status;
    }

    /* Poll the ST25R95 until it is ready to transmit */
    op_status = ST25R95_IO_SPI_Wait_Read(ST25R95_CONTROL_POLL_TIMEOUT);
    if(op_status < NFC_OK)
    {
        return op_status;
    }

    return ST25R95_IO_SPI_Get_Idle_Response();
}

NFC_OpResult ST25R95_IO_SPI_Reset_Chip(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 0);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_WriteByte(ST25R95_SPI_INDEX, ST25R95_CONTROL_RESET);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }
    
    NeonRTOS_Sleep(1);

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    NeonRTOS_Sleep(3);

    return ST25R95_IO_SPI_nIRQ_IN_Pulse();
}

NFC_OpResult ST25R95_IO_Set_BitRate(ST25R95_BitRate txBR, ST25R95_BitRate rxBR)
{
    ioCurrenTxBitRate = txBR;
    ioCurrenRxBitRate = rxBR;

    return NFC_OK;
}

NFC_OpResult ST25R95_IO_Get_BitRate(ST25R95_BitRate* pTxBR, ST25R95_BitRate* pRxBR)
{
    if(pTxBR!=NULL)
    {
        *pTxBR = ioCurrenTxBitRate;
    }
    if(pRxBR!=NULL)
    {
        *pRxBR = ioCurrenRxBitRate;
    }

    return NFC_OK;
}

NFC_OpResult ST25R95_IO_Init(void)
{
    NFC_OpResult op_status;

    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;
    
    gpio_op_status = GPIO_Pin_Init(ST25R95_GPIO_CS_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_CS_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Init(ST25R95_GPIO_INTERFACE_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_INTERFACE_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Init(ST25R95_GPIO_IRQ_IN_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_Up);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Write(ST25R95_GPIO_IRQ_IN_PIN, 1);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Init(ST25R95_GPIO_IRQ_OUT_PIN, hwGPIO_Direction_Input, hwGPIO_Pull_Mode_None);
    if (gpio_op_status < hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    spi_op_status = SPI_Master_Init(ST25R95_SPI_INDEX, ST25R95_SPI_CLOCK, hwSPI_OpMode_Polarity0_Phase0, false);
    if(spi_op_status<hwSPI_OK)
    {
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    /* First perform the startup sequence */
    op_status = ST25R95_IO_SPI_nIRQ_IN_Pulse();
    if(op_status < NFC_OK)
    {
        return op_status;
    }
    
    /* Reset ST25R95 */
    op_status = ST25R95_IO_SPI_Reset_Chip();
    if(op_status < NFC_OK)
    {
        return op_status;
    }

    /* Initialize chip */
    uint32_t attempt = 5;

    /* If no answer from ECHO command, reset and retry again up to max attempt */
    while ((ST25R95_IO_SPI_Command_Echo() != NFC_OK) && (attempt != 0)) {
      attempt--;
      ST25R95_IO_SPI_Reset_Chip();
    }
    if (attempt == 0) {
      return NFC_System;
    }

    /* Check expected chip: ST25R95 */
    if (!ST25R95_CheckChipID()) {
      return NFC_Hw_Mismatch;
    }

    return NFC_OK;
}
NFC_OpResult ST25R95_IO_DeInit(void)
{
    NFC_OpResult op_status;
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    /* Reset ST25R95 */
    op_status = ST25R95_IO_SPI_Reset_Chip();
    if (op_status < NFC_OK)
    {
        return op_status;
    }

    spi_op_status = SPI_Master_DeInit(ST25R95_SPI_INDEX);
    if (spi_op_status < hwSPI_OK)
    {
        return NFC_ST25R95_Map_SPI_Error_Code(spi_op_status);
    }

    gpio_op_status = GPIO_Pin_DeInit(ST25R95_GPIO_IRQ_IN_PIN);
    if (gpio_op_status < hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_DeInit(ST25R95_GPIO_IRQ_OUT_PIN);
    if (gpio_op_status < hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_DeInit(ST25R95_GPIO_INTERFACE_PIN);
    if(gpio_op_status<hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_DeInit(ST25R95_GPIO_CS_PIN);
    if (gpio_op_status < hwGPIO_OK)
    {
        return NFC_ST25R95_Map_GPIO_Error_Code(gpio_op_status);
    }

    return NFC_OK;
}