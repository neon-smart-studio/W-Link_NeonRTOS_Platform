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

#ifndef ST25R95_COM_H
#define ST25R95_COM_H

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

#ifndef CONFIG_ST25R95_GPIO_IRQ_CS_PIN
#define ST25R95_GPIO_IRQ_CS_PIN            hwGPIO_Int_Pin_E4
#else
#define ST25R95_GPIO_IRQ_CS_PIN            CONFIG_ST25R95_GPIO_IRQ_CS_PIN
#endif

#ifndef CONFIG_ST25R95_GPIO_IRQ_IN_PIN
#define ST25R95_GPIO_IRQ_IN_PIN            hwGPIO_Int_Pin_E4
#else
#define ST25R95_GPIO_IRQ_IN_PIN            CONFIG_ST25R95_GPIO_IRQ_IN_PIN
#endif

#ifndef CONFIG_ST25R95_GPIO_IRQ_OUT_PIN
#define ST25R95_GPIO_IRQ_OUT_PIN           hwGPIO_Int_Pin_E4
#else
#define ST25R95_GPIO_IRQ_OUT_PIN           CONFIG_ST25R95_GPIO_IRQ_OUT_PIN
#endif

#ifndef CONFIG_ST25R95_GPIO_INTERFACE_PIN
#define ST25R95_GPIO_INTERFACE_PIN         hwGPIO_Pin_E2
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

#define ST25R95_ERRCODE_NONE                             0x00 /*!< no error occurred */
#define ST25R95_ERRCODE_FRAMEOKADDITIONALINFO            0x80 /*!< Frame correctly received (additionally see CRC/Parity information) */
#define ST25R95_ERRCODE_INVALIDCMDLENGHT                 0x82 /*!< Invalid command length */
#define ST25R95_ERRCODE_INVALIDPROTOCOL                  0x83 /*!< Invalid protocol */
#define ST25R95_ERRCODE_COMERROR                         0x86 /*!< Hardware communication error */
#define ST25R95_ERRCODE_FRAMEWAITTIMEOUT                 0x87 /*!< Frame wait time out (no valid reception) */
#define ST25R95_ERRCODE_INVALIDSOF                       0x88 /*!< Invalid SOF */
#define ST25R95_ERRCODE_OVERFLOW                         0x89 /*!< Too many bytes received and data still arriving */
#define ST25R95_ERRCODE_FRAMING                          0x8A /*!< if start bit = 1 or stop bit = 0 */
#define ST25R95_ERRCODE_EGT                              0x8B /*!< EGT time out */
#define ST25R95_ERRCODE_FIELDLENGTH                      0x8C /*!< Valid for ISO/IEC 18092, if Length <3 */
#define ST25R95_ERRCODE_CRC                              0x8D /*!< CRC error, Valid only for ISO/IEC 18092 */
#define ST25R95_ERRCODE_RECEPTIONLOST                    0x8E /*!< When reception is lost without EOF received (or subcarrier was lost) */
#define ST25R95_ERRCODE_NOFIELD                          0x8F /*!< When Listen command detects the absence of external field */
#define ST25R95_ERRCODE_RESULTSRESIDUAL                  0x90 /*!< Residual bits in last byte. Useful for ACK/NAK reception of ISO/IEC 14443 Type A. */
#define ST25R95_ERRCODE_61_SOF                           0x61 /*!< SOF error during the EMD process */
#define ST25R95_ERRCODE_62_CRC                           0x62 /*!< CRC error during the EMD process */
#define ST25R95_ERRCODE_63_SOF_HIGH                      0x63 /*!< SOF error in ISO14443B occurs during high part (duration of 2 to 3 Elementary Unit Time, ETU) */
#define ST25R95_ERRCODE_65_SOF_LOW                       0x65 /*!< SOF error in ISO14443B occurs during low part (duration of 10 to 11 Elementary Unit Time, ETU) */
#define ST25R95_ERRCODE_66_EGT                           0x66 /*!< Extra Guard Time (EGT) error in ISO14443B */
#define ST25R95_ERRCODE_67_TR1TOOLONG                    0x67 /*!< TR1 set by card too long in case of protocol ISO14443B */
#define ST25R95_ERRCODE_68_TR1TOOSHORT                   0x68 /*!< TTR1 set by card too short in case of protocol ISO14443B */

#define ST25R95_REG_ARC_B                                0x6801 /*!< ARC_B register address */
#define ST25R95_REG_ACC_A                                0x6804 /*!< ACC_A register address */
#define ST25R95_REG_TIMERW                               0x3A00 /*!< TIMER register address */

#define ST25R95_IDN_RESPONSE_BUFLEN                              (15 + 2) /*!< IDN response buffer len */
#define ST25R95_PROTOCOLSELECT_RESPONSE_BUFLEN                   (0  + 2) /*!< ProtocolSelect response buffer len */
#define ST25R95_POLLFIELD_RESPONSE_BUFLEN                        (1  + 2) /*!< PollField response buffer len */
#define ST25R95_LISTEN_RESPONSE_BUFLEN                           (0  + 2) /*!< Listen response buffer len */
#define ST25R95_RDREG_RESPONSE_BUFLEN                            (1  + 2) /*!< ReadReg response buffer len */
#define ST25R95_WRREG_RESPONSE_BUFLEN                            (0  + 2) /*!< WriteRead response buffer len */
#define ST25R95_ACFILTER_RESPONSE_BUFLEN                         (1  + 2) /*!< ACFilter response buffer len */
#define ST25R95_IDLE_RESPONSE_BUFLEN                             (1  + 2) /*!< Idle response buffer len */
#define ST25R95_ECHO_RESPONSE_BUFLEN                             (3)      /*!< Echo response buffer len */
#define ST25R95_SEND_RESPONSE_BUFLEN                             (0  + 2) /*!< Send response buffer len */

#define ST25R95_ACSTATE_IDLE                             0x00U /*!< AC Filter state: Idle */
#define ST25R95_ACSTATE_READYA                           0x01U /*!< AC Filter state: ReadyA */
#define ST25R95_ACSTATE_ACTIVE                           0x04U /*!< AC Filter state: Active */
#define ST25R95_ACSTATE_HALT                             0x80U /*!< AC Filter state: Halt */
#define ST25R95_ACSTATE_READYAX                          0x81U /*!< AC Filter state: ReadyA* */
#define ST25R95_ACSTATE_ACTIVEX                          0x84U /*!< AC Filter state: Active* */

#define ST25R95_IS_PROT_ISO15693_CRC_ERR(status)        (((status) & 0x02U) == 0x02U) /*!< Test for CRC flag in SendRcv response additional byte for ISO15693 protocol */
#define ST25R95_IS_PROT_ISO15693_COLLISION_ERR(status)  (((status) & 0x01U) == 0x01U) /*!< Test for Collision flag in SendRcv response additional byte for ISO15693 protocol*/

#define ST25R95_IS_PROT_ISO14443A_COLLISION_ERR(status) (((status) & 0x80U) == 0x80U) /*!< Test for Collision flag in SendRcv response additional byte for ISO14443A protocol */
#define ST25R95_IS_PROT_ISO14443A_CRC_ERR(status)       (((status) & 0x20U) == 0x20U) /*!< Test for CRC       flag in SendRcv response additional byte for ISO14443A protocol */
#define ST25R95_IS_PROT_ISO14443A_PARITY_ERR(status)    (((status) & 0x10U) == 0x10U) /*!< Test for Parity    flag in SendRcv response additional byte for ISO14443A protocol */

#define ST25R95_IS_PROT_ISO14443B_CRC_ERR(status)       (((status) & 0x02U) == 0x02U) /*!< Test for CRC       flag in SendRcv response additional byte for ISO14443B protocol */

#define ST25R95_IS_PROT_ISO18092_CRC_ERR(status)        (((status) & 0x02U) == 0x02U) /*!< Test for CRC       flag in SendRcv response additional byte for ISO18092  protocol */

#define ST25R95_PROTOCOLSELECT_BR_OFFSET                (3U) /*!< Bit Rate offset in ProtocolSelect Command */

#define ST25R95_IDLE_WUPERIOD_OFFSET                    (0x09U) /*!< WUPeriod offset in Idle Command  */
#define ST25R95_IDLE_DACDATAL_OFFSET                    (0x0CU) /*!< DacDataL offset in Idle Command  */
#define ST25R95_IDLE_DACDATAH_OFFSET                    (0x0DU) /*!< DacDataH offset in Idle Command  */
#define ST25R95_DACDATA_MAX                             (0xFCU) /*!< DacData max value (6 bits MSB)   */
#define ST25R95_IDLE_WKUP_TIMEOUT                       (0x01U) /*!< Idle wakeup source: timeout      */
#define ST25R95_IDLE_WKUP_TAGDETECT                     (0x02U) /*!< Idle wakeup source: Tag Detected */

#define ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Pos           (3U)                                                                                       /*!< SPI poll flag bit 3: Data can be read when set */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Msk           (0x1U << ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Pos)                                           /*!< Mask 0x08 */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_READ               ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Msk                                                     /*!< 0x08 */
#define ST25R95_POLL_DATA_CAN_BE_READ(Flags)             (((Flags) & ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Msk) == ST25R95_POLL_FLAG_DATA_CAN_BE_READ) /*!< SPI read poll flag test */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Pos           (2U)                                                                                       /*!< SPI poll flag bit 2: Data can be send when set */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Msk           (0x1U << ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Pos)                                           /*!< Mask 0x04 */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_SEND               ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Msk                                                     /*!< 0x04 */
#define ST25R95_POLL_DATA_CAN_BE_SEND(Flags)             (((Flags) & ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Msk) == ST25R95_POLL_FLAG_DATA_CAN_BE_SEND) /*!< SPI send poll flag test*/


/*! ST25R95 transceive flags */
#define ST25R95_TXRX_FLAGS_CRC_TX_AUTO       (0U << 0) /*!< CRC will be generated automatic upon transmission                                     */
#define ST25R95_TXRX_FLAGS_CRC_TX_MANUAL     (1U << 0) /*!< CRC was calculated manually, included in txBuffer                                     */
#define ST25R95_TXRX_FLAGS_CRC_RX_KEEP       (1U << 1) /*!< Upon Reception keep the CRC in rxBuffer (reflected on rcvd length)                    */
#define ST25R95_TXRX_FLAGS_CRC_RX_REMV       (0U << 1) /*!< Remove the CRC from rxBuffer                                                          */
#define ST25R95_TXRX_FLAGS_NFCIP1_ON         (1U << 2) /*!< Enable NFCIP1 mode: Add SB(F0) and LEN bytes during Tx and skip SB(F0) byte during Rx */
#define ST25R95_TXRX_FLAGS_NFCIP1_OFF        (0U << 2) /*!< Disable NFCIP1 mode: do not append protocol bytes while Tx nor skip while Rx          */
#define ST25R95_TXRX_FLAGS_AGC_OFF           (1U << 3) /*!< Disable Automatic Gain Control, improving multiple devices collision detection. \b DEPRECATED: flag is deprecated, usage of Anticollision APIs based on Analog Config table with ST25R95_ANALOG_CONFIG_ANTICOL settings */
#define ST25R95_TXRX_FLAGS_AGC_ON            (0U << 3) /*!< Enable Automatic Gain Control, improving single device reception                \b DEPRECATED: flag is deprecated, usage of Anticollision APIs based on Analog Config table with ST25R95_ANALOG_CONFIG_ANTICOL settings */
#define ST25R95_TXRX_FLAGS_PAR_RX_KEEP       (1U << 4) /*!< Disable Parity check and keep the Parity and CRC bits in the received buffer          */
#define ST25R95_TXRX_FLAGS_PAR_RX_REMV       (0U << 4) /*!< Enable Parity check and remove the parity bits from the received buffer               */
#define ST25R95_TXRX_FLAGS_PAR_TX_NONE       (1U << 5) /*!< Disable automatic Parity generation (ISO14443A) and use the one provided in the buffer*/
#define ST25R95_TXRX_FLAGS_PAR_TX_AUTO       (0U << 5) /*!< Enable automatic Parity generation (ISO14443A)                                        */
#define ST25R95_TXRX_FLAGS_NFCV_FLAG_MANUAL  (1U << 6) /*!< Disable automatic adaption of flag byte (ISO15693) according to current comm params   */
#define ST25R95_TXRX_FLAGS_NFCV_FLAG_AUTO    (0U << 6) /*!< Enable automatic adaption of flag byte (ISO115693) according to current comm params   */
#define ST25R95_TXRX_FLAGS_CRC_RX_MANUAL     (1U << 7) /*!< Disable automatic CRC check                                                           */
#define ST25R95_TXRX_FLAGS_CRC_RX_AUTO       (0U << 7) /*!< Enable automatic CRC check                                                            */

#define ST25R95_ERR_INCOMPLETE_BYTE                (40U) /*!< Incomplete byte rcvd         */
#define ST25R95_ERR_INCOMPLETE_BYTE_01             (41U) /*!< Incomplete byte rcvd - 1 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_02             (42U) /*!< Incomplete byte rcvd - 2 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_03             (43U) /*!< Incomplete byte rcvd - 3 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_04             (44U) /*!< Incomplete byte rcvd - 4 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_05             (45U) /*!< Incomplete byte rcvd - 5 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_06             (46U) /*!< Incomplete byte rcvd - 6 bit */
#define ST25R95_ERR_INCOMPLETE_BYTE_07             (47U) /*!< Incomplete byte rcvd - 7 bit */

#define ST25R95_NFCF_NFCID2_LEN                    8U       /*!< NFCID2 (FeliCa IDm) length                        */
#define ST25R95_NFCF_SENSF_RES_LEN_MIN             16U      /*!< SENSF_RES minimum length                          */
#define ST25R95_NFCF_SENSF_RES_LEN_MAX             18U      /*!< SENSF_RES maximum length                          */
#define ST25R95_NFCF_SENSF_RES_PAD0_LEN            2U       /*!< SENSF_RES PAD0 length                             */
#define ST25R95_NFCF_SENSF_RES_PAD1_LEN            3U       /*!< SENSF_RES PAD1 length                             */
#define ST25R95_NFCF_SENSF_RES_RD_LEN              2U       /*!< SENSF_RES Request Data length                     */
#define ST25R95_NFCF_SENSF_RES_BYTE1               1U       /*!< SENSF_RES first byte value                        */
#define ST25R95_NFCF_SENSF_SC_LEN                  2U       /*!< Felica SENSF_REQ System Code length               */
#define ST25R95_NFCF_SENSF_PARAMS_SC1_POS          0U       /*!< System Code byte1 position in the SENSF_REQ       */
#define ST25R95_NFCF_SENSF_PARAMS_SC2_POS          1U       /*!< System Code byte2 position in the SENSF_REQ       */
#define ST25R95_NFCF_SENSF_PARAMS_RC_POS           2U       /*!< Request Code position in the SENSF_REQ            */
#define ST25R95_NFCF_SENSF_PARAMS_TSN_POS          3U       /*!< Time Slot Number position in the SENSF_REQ        */
#define ST25R95_NFCF_POLL_MAXCARDS                 16U      /*!< Max number slots/cards 16                         */

#define ST25R95_NFCF_CMD_POS                        0U      /*!< Command/Response code length                      */
#define ST25R95_NFCF_CMD_LEN                        1U      /*!< Command/Response code length                      */
#define ST25R95_NFCF_LENGTH_LEN                     1U      /*!< LEN field length                                  */
#define ST25R95_NFCF_HEADER_LEN                     (ST25R95_NFCF_LENGTH_LEN + ST25R95_NFCF_CMD_LEN) /*!< Header length  */

#define ST25R95_NFCF_NOS_LEN                        1U      /*!< Number of Services length                         */
#define ST25R95_NFCF_NOB_LEN                        1U      /*!< Number of Blocks length                           */

#define ST25R95_NFCF_SENSF_NFCID2_BYTE1_POS         0U      /*!< NFCID2 byte1 position                             */
#define ST25R95_NFCF_SENSF_NFCID2_BYTE2_POS         1U      /*!< NFCID2 byte2 position                             */

#define ST25R95_NFCF_SENSF_NFCID2_PROT_TYPE_LEN     2U      /*!< NFCID2 length for byte 1 and byte 2 indicating NFC-DEP or T3T support */
#define ST25R95_NFCF_SENSF_NFCID2_BYTE1_NFCDEP      0x01U   /*!< NFCID2 byte1 NFC-DEP support            Digital 1.0 Table 44 */
#define ST25R95_NFCF_SENSF_NFCID2_BYTE2_NFCDEP      0xFEU   /*!< NFCID2 byte2 NFC-DEP support            Digital 1.0 Table 44 */

#define ST25R95_NFCF_SYSTEMCODE                     0xFFFFU /*!< SENSF_RES Default System Code            Digital 2.3 8.6.1.5 */
#define ST25R95_NFCF_SYSTEMCODE_LEN                 2U      /*!< SENSF_RES System Code length             Digital 2.3 8.6.1   */

#define ST25R95_NFCF_BLOCK_LEN                      16U     /*!< NFCF T3T Block size                        T3T 1.0  4.1      */
#define ST25R95_NFCF_CHECKUPDATE_RES_ST1_POS        9U      /*!< Check|Update Res Status Flag 1 position    T3T 1.0  Table 8  */
#define ST25R95_NFCF_CHECKUPDATE_RES_ST2_POS        10U     /*!< Check|Update Res Status Flag 2 position    T3T 1.0  Table 8  */
#define ST25R95_NFCF_CHECKUPDATE_RES_NOB_POS        11U     /*!< Check|Update Res Number of Blocks position T3T 1.0  Table 8  */

#define ST25R95_NFCF_STATUS_FLAG_SUCCESS            0x00U   /*!< Check response Number of Blocks position   T3T 1.0  Table 11 */
#define ST25R95_NFCF_STATUS_FLAG_ERROR              0xFFU   /*!< Check response Number of Blocks position   T3T 1.0  Table 11 */

#define ST25R95_NFCF_BLOCKLISTELEM_MAX_LEN          3U      /*!< Block List Element max Length (3 bytes)        T3T 1.0 5.6.1 */
#define ST25R95_NFCF_BLOCKLISTELEM_LEN_BIT          0x80U   /*!< Block List Element Length bit (2|3 bytes)      T3T 1.0 5.6.1 */

#define ST25R95_NFCF_SERVICECODE_RDONLY           0x000BU   /*!< NDEF Service Code as Read-Only                 T3T 1.0 7.2.1 */
#define ST25R95_NFCF_SERVICECODE_RDWR             0x0009U   /*!< NDEF Service Code as Read and Write            T3T 1.0 7.2.1 */

#define ST25R95_NFCF_TEST_LB_CMD0                   0xD8U /*!< T3T loopback CMD0                 ETSI TS 102 695-1  5.6.4.4.2 */
#define ST25R95_NFCF_TEST_LB_CMD1                   0x00U /*!< T3T loopback CMD1                 ETSI TS 102 695-1  5.6.4.4.2 */

#ifdef __cplusplus
extern "C" {
#endif

ST25R95_OpResult ST25R95_IO_SPI_Wait_Read(NeonRTOS_Time_t timeout);
ST25R95_OpResult ST25R95_IO_SPI_Wait_Send(void);
ST25R95_OpResult ST25R95_IO_SPI_Send_Command_Type_And_Len(uint8_t *cmd, uint8_t *resp, uint16_t respBuffLen);
ST25R95_OpResult ST25R95_IO_SPI_Command_Echo(void);
ST25R95_OpResult ST25R95_IO_SPI_Send_Transmit_Flag(ST25R95_Protocol protocol, uint8_t transmitFlag);
ST25R95_OpResult ST25R95_IO_SPI_Send_Data(uint8_t *buf, uint8_t bufLen, ST25R95_Protocol protocol, uint32_t flags);
ST25R95_OpResult ST25R95_IO_SPI_Complete_Rx(ST25R95_Protocol protocol, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen, uint32_t flags, uint8_t *additionalRespBytes);
ST25R95_OpResult ST25R95_IO_SPI_Idle(uint8_t dacDataL, uint8_t dacDataH, uint8_t WUPeriod);
ST25R95_OpResult ST25R95_IO_SPI_Get_Idle_Response(void);
ST25R95_OpResult ST25R95_IO_SPI_nIRQ_IN_Pulse(void);
ST25R95_OpResult ST25R95_IO_SPI_Kill_Idle(void);
ST25R95_OpResult ST25R95_IO_SPI_Reset_Chip(void);
ST25R95_OpResult ST25R95_IO_FieldOn(ST25R95_Protocol protocol);
ST25R95_OpResult ST25R95_IO_FieldOff(void);
ST25R95_OpResult ST25R95_IO_SetBitRate(ST25R95_Protocol protocol, ST25R95_BitRate txBR, ST25R95_BitRate rxBR);
ST25R95_OpResult ST25R95_IO_SetFWT(ST25R95_Protocol protocol, uint32_t fwt);
ST25R95_OpResult ST25R95_IO_SetSlotCounter(uint8_t slots);
ST25R95_OpResult ST25R95_IO_Protocol_Select(ST25R95_Protocol protocol);
ST25R95_OpResult ST25R95_IO_CalibrateTagDetector(uint8_t* pCalibrate);
ST25R95_OpResult ST25R95_IO_ReadReg(uint16_t reg, uint8_t *value);
ST25R95_OpResult ST25R95_IO_WriteReg(uint8_t protocol, uint16_t reg, uint8_t value);
ST25R95_OpResult ST25R95_IO_Init(void);
ST25R95_OpResult ST25R95_IO_DeInit(void);
bool ST25R95_IO_CheckChipID(void);


#ifdef __cplusplus
}
#endif

#endif /* ST25R95_COM_H */
