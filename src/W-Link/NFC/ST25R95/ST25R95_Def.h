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

#ifndef ST25R95_DEF_H
#define ST25R95_DEF_H

typedef enum {
    ST25R95_OK = 0,
    ST25R95_NotInit = -1,
    ST25R95_InvalidParameter = -2,
    ST25R95_MemoryError = -3,
    ST25R95_MutexTimeout = -4,
    ST25R95_SlaveTimeout = -5,
    ST25R95_IO_Error = -6,
    ST25R95_Hw_Mismatch = -7,
    ST25R95_Hw_OverRun = -8,
    ST25R95_System = -9,
    ST25R95_WrongState = -10,
    ST25R95_CRC_Error = -11,
    ST25R95_ParityError = -12,
    ST25R95_CalibrateError = -13,
    ST25R95_FrameTimeout = -14,
    ST25R95_FramingError = -15,
    ST25R95_RF_Collision = -16,
    ST25R95_LinkLose = -17,
    ST25R95_InternalError = -18,
    ST25R95_ImcompleteByte_0 = -19,
    ST25R95_ImcompleteByte_1 = -19,
    ST25R95_ImcompleteByte_2 = -19,
    ST25R95_ImcompleteByte_3 = -19,
    ST25R95_ImcompleteByte_4 = -19,
    ST25R95_ImcompleteByte_5 = -19,
    ST25R95_ImcompleteByte_6 = -19,
    ST25R95_ImcompleteByte_7 = -19,
    ST25R95_Unsupport = -19
} ST25R95_OpResult;

typedef enum{
    ST25R95_Protocol_FieldOff       = 0x00U, /*!< Field OFF */
    ST25R95_Protocol_ISO15693       = 0x01U, /*!< ISO15693 Reader */
    ST25R95_Protocol_ISO14443A      = 0x02U, /*!< ISO14443A Reader */
    ST25R95_Protocol_ISO14443B      = 0x03U, /*!< ISO14443B Reader */
    ST25R95_Protocol_ISO18092       = 0x04U, /*!< ISO18092 Reader */
    ST25R95_Protocol_CE_ISO14443A   = 0x05U,  /*!< ISO14443A Card Emulation */
    ST25R95_Protocol_MAX
} ST25R95_Protocol;

/*! ST25R95 modes    */
typedef enum {
  ST25R95_Mode_None                   = 0,    /*!< No mode selected/defined                                         */
  ST25R95_Mode_Poll_NFCA              = 1,    /*!< Mode to perform as NFCA (ISO14443A) Poller (PCD)                 */
  ST25R95_Mode_Poll_NFCA_T1T          = 2,    /*!< Mode to perform as NFCA T1T (Topaz) Poller (PCD)                 */
  ST25R95_Mode_Poll_NFCB              = 3,    /*!< Mode to perform as NFCB (ISO14443B) Poller (PCD)                 */
  ST25R95_Mode_Poll_B_PRIME           = 4,    /*!< Mode to perform as B' Calypso (Innovatron) (PCD)                 */
  ST25R95_Mode_Poll_B_CTS             = 5,    /*!< Mode to perform as CTS Poller (PCD)                              */
  ST25R95_Mode_Poll_NFCF              = 6,    /*!< Mode to perform as NFCF (FeliCa) Poller (PCD)                    */
  ST25R95_Mode_Poll_NFCV              = 7,    /*!< Mode to perform as NFCV (ISO15963) Poller (PCD)                  */
  ST25R95_Mode_Poll_PICOPASS          = 8,    /*!< Mode to perform as PicoPass / iClass Poller (PCD)                */
  ST25R95_Mode_Poll_Active_P2P        = 9,    /*!< Mode to perform as Active P2P (ISO18092) Initiator               */
  ST25R95_Mode_Listen_NFCA            = 10,   /*!< Mode to perform as NFCA (ISO14443A) Listener (PICC)              */
  ST25R95_Mode_Listen_NFCB            = 11,   /*!< Mode to perform as NFCA (ISO14443B) Listener (PICC)              */
  ST25R95_Mode_Listen_NFCF            = 12,   /*!< Mode to perform as NFCA (ISO15963) Listener (PICC)               */
  ST25R95_Mode_Listen_Active_P2P      = 13    /*!< Mode to perform as Active P2P (ISO18092) Target                  */
} ST25R95_Mode;

/*! RFAL Listen Mode States */
typedef enum {
  ST25R95_LM_State_Not_Init              = 0x00,     /*!< Not Initialized state                       */
  ST25R95_LM_State_Power_Off             = 0x01,     /*!< Power Off state                             */
  ST25R95_LM_State_Idle                  = 0x02,     /*!< Idle state  Activity 1.1  5.2               */
  ST25R95_LM_State_Ready_A               = 0x03,     /*!< Ready A state  Activity 1.1  5.3 5.4 & 5.5  */
  ST25R95_LM_State_Ready_B               = 0x04,     /*!< Ready B state  Activity 1.1  5.11 5.12      */
  ST25R95_LM_State_Ready_F               = 0x05,     /*!< Ready F state  Activity 1.1  5.15           */
  ST25R95_LM_State_Active_A              = 0x06,     /*!< Active A state  Activity 1.1  5.6           */
  ST25R95_LM_State_CardEmu_4A            = 0x07,     /*!< Card Emulation 4A state  Activity 1.1  5.10 */
  ST25R95_LM_State_CardEmu_4B            = 0x08,     /*!< Card Emulation 4B state  Activity 1.1  5.14 */
  ST25R95_LM_State_CardEmu_3             = 0x09,     /*!< Card Emulation 3 state  Activity 1.1  5.18  */
  ST25R95_LM_State_Target_A              = 0x0A,     /*!< Target A state  Activity 1.1  5.9           */
  ST25R95_LM_State_Target_F              = 0x0B,     /*!< Target F state  Activity 1.1  5.17          */
  ST25R95_LM_State_Sleep_A               = 0x0C,     /*!< Sleep A state  Activity 1.1  5.7            */
  ST25R95_LM_State_Sleep_B               = 0x0D,     /*!< Sleep B state  Activity 1.1  5.13           */
  ST25R95_LM_State_Ready_Ax              = 0x0E,     /*!< Ready A* state  Activity 1.1  5.3 5.4 & 5.5 */
  ST25R95_LM_State_Active_Ax             = 0x0F,     /*!< Active A* state  Activity 1.1  5.6          */
  ST25R95_LM_State_Sleep_AF              = 0x10,     /*!< Sleep AF state  Activity 1.1  5.19          */
} ST25R95_LmState;

/*! ST25R95 Bit rates    */
typedef enum {
  ST25R95_BitRate_106                      = 0,    /*!< Bit Rate 106 kbit/s (fc/128)                                      */
  ST25R95_BitRate_212                      = 1,    /*!< Bit Rate 212 kbit/s (fc/64)                                      */
  ST25R95_BitRate_424                      = 2,    /*!< Bit Rate 424 kbit/s (fc/32)                                      */
  ST25R95_BitRate_848                      = 3,    /*!< Bit Rate 848 kbit/s (fc/16)                                      */
  ST25R95_BitRate_1695                     = 4,    /*!< Bit Rate 1695 kbit/s (fc/8)                                      */
  ST25R95_BitRate_3390                     = 5,    /*!< Bit Rate 3390 kbit/s (fc/4)                                      */
  ST25R95_BitRate_6780                     = 6,    /*!< Bit Rate 6780 kbit/s (fc/2)                                      */
  ST25R95_BitRate_13560                    = 7,    /*!< Bit Rate 13560 kbit/s (fc)                                       */
  ST25R95_BitRate_211p88                   = 0xE9, /*!< Bit Rate 211,88 kbit/s (fc/64) Fast Mode VICC->VCD               */
  ST25R95_BitRate_105p94                   = 0xEA, /*!< Bit Rate 105,94 kbit/s (fc/128) Fast Mode VICC->VCD              */
  ST25R95_BitRate_52p97                    = 0xEB, /*!< Bit Rate 52.97 kbit/s (fc/256) Fast Mode VICC->VCD               */
  ST25R95_BitRate_26p48                    = 0xEC, /*!< Bit Rate 26,48 kbit/s (fc/512) NFCV VICC->VCD & VCD->VICC 1of4   */
  ST25R95_BitRate_1p66                     = 0xED, /*!< Bit Rate 1,66 kbit/s (fc/8192) NFCV VCD->VICC 1of256             */
  ST25R95_BitRate_KEEP                     = 0xFF  /*!< Value indicating to keep the same previous bit rate              */
} ST25R95_BitRate;

/*! ST25R95 main states flags    */
typedef enum {
  ST25R95_State_Idle                  = 0,
  ST25R95_State_Init                  = 1,
  ST25R95_State_Mode_Set              = 2,

  ST25R95_State_TXRX                  = 3,
  ST25R95_State_LM                    = 4,
  ST25R95_State_WUM                   = 5

} ST25R95_State;

typedef enum {
  ST25R95_Transceive_State_Idle             = 0,
  ST25R95_Transceive_State_Init             = 1,
  ST25R95_Transceive_State_Start            = 2,

  ST25R95_Transceive_State_TX_Idle          = 11,
  ST25R95_Transceive_State_TX_Wait_GT       = 12,
  ST25R95_Transceive_State_TX_Wait_FDT      = 13,
  ST25R95_Transceive_State_TX_Prepare_TX    = 14,
  ST25R95_Transceive_State_TX_Transmit      = 15,
  ST25R95_Transceive_State_TX_Wait_WL       = 16,
  ST25R95_Transceive_State_TX_Reload_FIFO   = 17,
  ST25R95_Transceive_State_TX_Wait_TXE      = 18,
  ST25R95_Transceive_State_TX_Done          = 19,
  ST25R95_Transceive_State_TX_Fail          = 20,

  ST25R95_Transceive_State_RX_Idle          = 81,
  ST25R95_Transceive_State_RX_Wait_EON      = 82,
  ST25R95_Transceive_State_RX_Wait_RXS      = 83,
  ST25R95_Transceive_State_RX_Wait_RXE      = 84,
  ST25R95_Transceive_State_RX_Read_FIFO     = 85,
  ST25R95_Transceive_State_RX_ERR_Check     = 86,
  ST25R95_Transceive_State_RX_Read_Data     = 87,
  ST25R95_Transceive_State_RX_Wait_EOF      = 88,
  ST25R95_Transceive_State_RX_Done          = 89,
  ST25R95_Transceive_State_RX_Fail          = 90,

} ST25R95_TransceiveState;

typedef enum {
  ST25R95_WUM_State_Not_Init              = 0x00,     /*!< Not Initialized state                       */
  ST25R95_WUM_State_Initializing          = 0x01,     /*!< Wake-Up mode is starting                    */
  ST25R95_WUM_State_Enabled               = 0x02,     /*!< Wake-Up mode is enabled                     */
  ST25R95_WUM_State_Enabled_Woke          = 0x03,     /*!< Wake-Up mode enabled and has received IRQ(s)*/
} ST25R95_WumState;


#endif // ST25R95_DEF_H