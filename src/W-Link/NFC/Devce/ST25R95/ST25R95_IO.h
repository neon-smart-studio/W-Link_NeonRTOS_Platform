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

#ifndef ST25R95_IO_H
#define ST25R95_IO_H

#include "NeonRTOS.h"

#include "ST25R95_Def.h"

#include "NFC_Config.h"

#ifndef CONFIG_ST25R95_SPI_INDEX
#define ST25R95_SPI_INDEX                  hwSPI_Index_0
#else
#define ST25R95_SPI_INDEX                  CONFIG_ST25R95_SPI_INDEX
#endif

#ifndef CONFIG_ST25R95_SPI_CLOCK
#define ST25R95_SPI_CLOCK                  100000
#else
#define ST25R95_SPI_CLOCK                  CONFIG_ST25R95_SPI_CLOCK
#endif

#ifndef CONFIG_ST25R95_GPIO_IRQ_IN_PIN
#define ST25R95_GPIO_IRQ_IN_PIN            hwGPIO_Pin_A9
#else
#define ST25R95_GPIO_IRQ_IN_PIN            CONFIG_ST25R95_GPIO_IRQ_IN_PIN
#endif

#ifndef CONFIG_ST25R95_GPIO_IRQ_OUT_PIN
#define ST25R95_GPIO_IRQ_OUT_PIN           hwGPIO_Pin_A10
#else
#define ST25R95_GPIO_IRQ_OUT_PIN           CONFIG_ST25R95_GPIO_IRQ_OUT_PIN
#endif

#ifndef CONFIG_ST25R95_GPIO_CS_PIN
#define ST25R95_GPIO_CS_PIN                hwGPIO_Pin_B6
#else
#define ST25R95_GPIO_CS_PIN                CONFIG_ST25R95_GPIO_CS_PIN
#endif

#ifndef CONFIG_ST25R95_GPIO_INTERFACE_PIN
#define ST25R95_GPIO_INTERFACE_PIN         hwGPIO_Pin_C7
#else
#define ST25R95_GPIO_INTERFACE_PIN         CONFIG_ST25R95_GPIO_INTERFACE_PIN
#endif

/* See ST95HF DS §4.1.1 or CR95HF DS §4.2.1 */

#define ST25R95_CONTROL_SEND                             0x00 /*!< Send command to the ST25R95 */
#define ST25R95_CONTROL_RESET                            0x01 /*!< Reset the ST25R95           */
#define ST25R95_CONTROL_READ                             0x02 /*!< Read data from the ST25R95  */
#define ST25R95_CONTROL_POLL                             0x03 /*!< Poll the ST25R95            */

#define ST25R95_CONTROL_POLL_TIMEOUT                      100 /*!< Polling timeout             */
#define ST25R95_CONTROL_POLL_NO_TIMEOUT                     0 /*!< non blocking polling        */

/* See ST95HF DS §5.2 or CR95HF DS §5.2 */
#define ST25R95_COMMAND_IDN                              0x01 /*!< Requests short information about the ST25R95 and its revision.                           */
#define ST25R95_COMMAND_PROTOCOLSELECT                   0x02 /*!< Selects the RF communication protocol and specifies certain protocol-related parameters. */
#define ST25R95_COMMAND_POLLFIED                         0x03 /*!< Returns the current value of the FieldDet flag (used in Card Emulation mode).            */
#define ST25R95_COMMAND_SENDRECV                         0x04 /*!< Sends data using the previously selected protocol and receives the tag response.         */
#define ST25R95_COMMAND_LISTEN                           0x05 /*!< Listens for data using previously selected protocol (used in Card Emulation mode).       */
#define ST25R95_COMMAND_SEND                             0x06 /*!< Sends data using previously selected protocol (used in Card Emulation mode).             */
#define ST25R95_COMMAND_IDLE                             0x07 /*!< Switches the ST25R95 into a low consumption                                              */
#define ST25R95_COMMAND_RDREG                            0x08 /*!< Reads Wake-up event register or the Analog Register Configuration (ARC_B) register       */
#define ST25R95_COMMAND_WRREG                            0x09 /*!< Write register                                                                           */
#define ST25R95_COMMAND_BAUDRATE                         0x0A /*!< Sets the UART baud rate.                                                                 */
#define ST25R95_COMMAND_ACFILTER                         0x0D /*!< Enables or disables the anti-collision filter for ISO/IEC 14443 Type A protocol.         */
#define ST25R95_COMMAND_ECHO                             0x55 /*!< ST25R95 performs a serial interface ECHO command                                         */

#define ST25R95_SPI_DUMMY_BYTE                           0x00 /*!< Dummy byte when nothing to transmit on SPI                                               */

#define ST25R95_CMD_COMMAND_OFFSET                       0x00U /*!< CMD Offset. See CR95HF DS § 4.2.1                        */
#define ST25R95_CMD_RESULT_OFFSET                        0x00U /*!< Resp Code Offset. See CR95HF DS § 4.2.1                  */
#define ST25R95_CMD_LENGTH_OFFSET                        0x01U /*!< LEN Offset. See CR95HF DS § 4.2.1                        */
#define ST25R95_CMD_DATA_OFFSET                          0x02U /*!< DATA[0] Offset. See CR95HF DS § 4.2.1                    */

#define ST25R95_COMMUNICATION_BUFFER_SIZE                (528 + 2) /*!< Max received buffer size                            */
#define ST25R95_COMMUNICATION_UART_WDOGTIMER             (5000U)   /*!< Uart watchdog timer                                 */

#define ST25R95_FWT_MAX                                  0x40A8BC0 /*!< Max FWT supported: 5s */

#define ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Pos           (3U)                                                                                       /*!< SPI poll flag bit 3: Data can be read when set */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Msk           (0x1U << ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Pos)                                           /*!< Mask 0x08 */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_READ               ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Msk                                                     /*!< 0x08 */
#define ST25R95_POLL_DATA_CAN_BE_READ(Flags)             (((Flags) & ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Msk) == ST25R95_POLL_FLAG_DATA_CAN_BE_READ) /*!< SPI read poll flag test */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Pos           (2U)                                                                                       /*!< SPI poll flag bit 2: Data can be send when set */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Msk           (0x1U << ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Pos)                                           /*!< Mask 0x04 */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_SEND               ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Msk                                                     /*!< 0x04 */
#define ST25R95_POLL_DATA_CAN_BE_SEND(Flags)             (((Flags) & ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Msk) == ST25R95_POLL_FLAG_DATA_CAN_BE_SEND) /*!< SPI send poll flag test*/

#define ST25R95_IS_PROT_ISO15693_CRC_ERR(status)        (((status) & 0x02U) == 0x02U) /*!< Test for CRC flag in SendRcv response additional byte for ISO15693 protocol */
#define ST25R95_IS_PROT_ISO15693_COLLISION_ERR(status)  (((status) & 0x01U) == 0x01U) /*!< Test for Collision flag in SendRcv response additional byte for ISO15693 protocol*/

#define ST25R95_IS_PROT_ISO14443A_COLLISION_ERR(status) (((status) & 0x80U) == 0x80U) /*!< Test for Collision flag in SendRcv response additional byte for ISO14443A protocol */
#define ST25R95_IS_PROT_ISO14443A_CRC_ERR(status)       (((status) & 0x20U) == 0x20U) /*!< Test for CRC       flag in SendRcv response additional byte for ISO14443A protocol */
#define ST25R95_IS_PROT_ISO14443A_PARITY_ERR(status)    (((status) & 0x10U) == 0x10U) /*!< Test for Parity    flag in SendRcv response additional byte for ISO14443A protocol */

#define ST25R95_IS_PROT_ISO14443B_CRC_ERR(status)       (((status) & 0x02U) == 0x02U) /*!< Test for CRC       flag in SendRcv response additional byte for ISO14443B protocol */

#define ST25R95_IS_PROT_ISO18092_CRC_ERR(status)        (((status) & 0x02U) == 0x02U) /*!< Test for CRC       flag in SendRcv response additional byte for ISO18092  protocol */

#ifdef __cplusplus
extern "C" {
#endif

NFC_OpResult ST25R95_IO_SPI_Wait_Read(NeonRTOS_Time_t timeout);
NFC_OpResult ST25R95_IO_SPI_Wait_Send(void);
NFC_OpResult ST25R95_IO_SPI_Send_Command_Type_And_Len(uint8_t *cmd, uint8_t *resp, uint16_t respBuffLen);
NFC_OpResult ST25R95_IO_SPI_Command_Echo(void);
NFC_OpResult ST25R95_IO_SPI_Send_Transmit_Flag(ST25R95_Protocol protocol, uint8_t transmitFlag);
NFC_OpResult ST25R95_IO_SPI_Send_Data(uint8_t *buf, uint8_t bufLen, ST25R95_Protocol protocol, uint32_t flags);
NFC_OpResult ST25R95_IO_SPI_Complete_Rx(ST25R95_Protocol protocol, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen, uint32_t flags, uint8_t *additionalRespBytes);
NFC_OpResult ST25R95_IO_SPI_Idle(uint8_t dacDataL, uint8_t dacDataH, uint8_t WUPeriod);
NFC_OpResult ST25R95_IO_SPI_Get_Idle_Response(void);
NFC_OpResult ST25R95_IO_SPI_nIRQ_IN_Pulse(void);
NFC_OpResult ST25R95_IO_SPI_Kill_Idle(void);
NFC_OpResult ST25R95_IO_SPI_Reset_Chip(void);
NFC_OpResult ST25R95_IO_Set_BitRate(ST25R95_BitRate txBR, ST25R95_BitRate rxBR);
NFC_OpResult ST25R95_IO_Get_BitRate(ST25R95_BitRate* pTxBR, ST25R95_BitRate* pRxBR);
NFC_OpResult ST25R95_IO_Init(void);
NFC_OpResult ST25R95_IO_DeInit(void);


#ifdef __cplusplus
}
#endif

#endif /* ST25R95_IO_H */
