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

#ifndef RFAL_T1T_H
#define RFAL_T1T_H

#include "RFal_RF.h"

#include "NFC/NFC_Def.h"

#define RFAL_T1T_UID_LEN               4   /*!< T1T UID length of cascade level 1 only tag  */
#define RFAL_T1T_HR_LENGTH             2   /*!< T1T HR(Header ROM) length                   */

#define RFAL_T1T_HR0_NDEF_MASK      0xF0   /*!< T1T HR0 NDEF capability mask  T1T 1.2 2.2.2 */
#define RFAL_T1T_HR0_NDEF_SUPPORT   0x10   /*!< T1T HR0 NDEF capable value    T1T 1.2 2.2.2 */

/*! NFC-A T1T (Topaz) command set */
typedef enum {
  RFAL_T1T_CMD_RID      = 0x78,          /*!< T1T Read UID                                */
  RFAL_T1T_CMD_RALL     = 0x00,          /*!< T1T Read All                                */
  RFAL_T1T_CMD_READ     = 0x01,          /*!< T1T Read                                    */
  RFAL_T1T_CMD_WRITE_E  = 0x53,          /*!< T1T Write with erase (single byte)          */
  RFAL_T1T_CMD_WRITE_NE = 0x1A           /*!< T1T Write with no erase (single byte)       */
} RFal_T1T_cmds;

/*! NFC-A T1T (Topaz) RID_RES  Digital 1.1  10.6.2 & Table 50 */
typedef struct {
  uint8_t hr0;                           /*!< T1T Header ROM: HR0                         */
  uint8_t hr1;                           /*!< T1T Header ROM: HR1                         */
  uint8_t uid[RFAL_T1T_UID_LEN];         /*!< T1T UID                                     */
} RFal_T1T_RidRes;

#ifdef __cplusplus
extern "C" {
#endif

NFC_OpResult RFal_T1T_PollerInit(void);
NFC_OpResult RFal_T1T_PollerRid(RFal_T1T_RidRes *ridRes);
NFC_OpResult RFal_T1T_PollerRall(const uint8_t *uid, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen);
NFC_OpResult RFal_T1T_PollerWrite(const uint8_t *uid, uint8_t address, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_T1T_H */