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

#ifndef RFAL_T2T_H
#define RFAL_T2T_H

#include "RFal_RF.h"

#include "NFC/NFC_Def.h"

#define RFAL_T2T_BLOCK_LEN            4U                          /*!< T2T block length           */
#define RFAL_T2T_READ_DATA_LEN        (4U * RFAL_T2T_BLOCK_LEN)   /*!< T2T READ data length       */
#define RFAL_T2T_WRITE_DATA_LEN       RFAL_T2T_BLOCK_LEN          /*!< T2T WRITE data length      */

#ifdef __cplusplus
extern "C" {
#endif

NFC_OpResult RFal_T2T_PollerRead(uint8_t blockNum, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvLen);
NFC_OpResult RFal_T2T_PollerWrite(uint8_t blockNum, const uint8_t *wrData);
NFC_OpResult RFal_T2T_PollerSectorSelect(uint8_t sectorNum);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_T2T_H */
