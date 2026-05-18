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

#ifndef ST25R95_H
#define ST25R95_H

#include <stdbool.h>
#include <stdint.h>

#include "ST25R95_Def.h"

#include "ST25R95_IO.h"

#define ST25R95_PROTOCOLSELECT_BR_OFFSET                (3U) /*!< Bit Rate offset in ProtocolSelect Command */

#ifdef __cplusplus
extern "C" {
#endif

ST25R95_OpResult ST25R95_Field_On(ST25R95_Protocol protocol);
ST25R95_OpResult ST25R95_Field_Off(void);
ST25R95_OpResult ST25R95_Set_BitRate(ST25R95_Protocol protocol, ST25R95_BitRate txBR, ST25R95_BitRate rxBR);
ST25R95_OpResult ST25R95_Set_FWT(ST25R95_Protocol protocol, uint32_t fwt);
ST25R95_OpResult ST25R95_Set_SlotCounter(ST25R95_FeliCaPollSlots slots);
ST25R95_OpResult ST25R95_Protocol_Select(ST25R95_Protocol protocol);
ST25R95_OpResult ST25R95_Calibrate_Tag_Detector(uint8_t* pCalibrate);
ST25R95_OpResult ST25R95_Read_Reg(uint16_t reg, uint8_t *value);
ST25R95_OpResult ST25R95_Write_Reg(ST25R95_Protocol protocol, uint16_t reg, uint8_t value);
bool ST25R95_CheckChipID(void);

#ifdef __cplusplus
}
#endif

#endif /* ST25R95_H */
