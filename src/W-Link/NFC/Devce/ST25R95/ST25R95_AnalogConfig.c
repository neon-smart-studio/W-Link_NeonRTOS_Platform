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

#include <stddef.h>

#include "ST25R95_Def.h"

#include "ST25R95_AnalogConfig.h"
#include "ST25R95_AnalogConfigTable.h"

#define ST25R95_TEST_REG         0x0080U      /*!< Test Register indicator  */

ST25R95_AnalogConfigMgmt gRfalAnalogConfigMgmt;  /*!< Analog Configuration LUT management */

static ST25R95_AnalogConfigNum ST25R95_AnalogConfigSearch(ST25R95_AnalogConfigId configId, uint16_t *configOffset)
{
  ST25R95_AnalogConfigId foundConfigId;
  ST25R95_AnalogConfigId configIdMaskVal;
  const uint8_t *configTbl;
  const uint8_t *currentConfigTbl;
  uint16_t i;

  currentConfigTbl = gRfalAnalogConfigMgmt.currentAnalogConfigTbl;
  configIdMaskVal  = ((ST25R95_ANALOG_CONFIG_POLL_LISTEN_MODE_MASK | ST25R95_ANALOG_CONFIG_BITRATE_MASK)
                      | ((ST25R95_ANALOG_CONFIG_TECH_CHIP == ST25R95_ANALOG_CONFIG_ID_GET_TECH(configId)) ? (ST25R95_ANALOG_CONFIG_TECH_MASK | ST25R95_ANALOG_CONFIG_CHIP_SPECIFIC_MASK) : configId)
                      | ((ST25R95_ANALOG_CONFIG_NO_DIRECTION == ST25R95_ANALOG_CONFIG_ID_GET_DIRECTION(configId)) ? ST25R95_ANALOG_CONFIG_DIRECTION_MASK : configId)
                     );


  /* When specific ConfigIDs are to be used, override search mask */
  if ((ST25R95_ANALOG_CONFIG_ID_GET_DIRECTION(configId) == ST25R95_ANALOG_CONFIG_DPO)) {
    configIdMaskVal = (ST25R95_ANALOG_CONFIG_POLL_LISTEN_MODE_MASK | ST25R95_ANALOG_CONFIG_TECH_MASK | ST25R95_ANALOG_CONFIG_BITRATE_MASK | ST25R95_ANALOG_CONFIG_DIRECTION_MASK);
  }


  i = *configOffset;
  while (i < gRfalAnalogConfigMgmt.configTblSize) {
    configTbl = &currentConfigTbl[i];
    foundConfigId = GETU16(configTbl);
    if (configId == (foundConfigId & configIdMaskVal)) {
      *configOffset = (uint16_t)(i + sizeof(ST25R95_AnalogConfigId) + sizeof(ST25R95_AnalogConfigNum));
      return configTbl[sizeof(ST25R95_AnalogConfigId)];
    }

    /* If Config Id does not match, increment to next Configuration Id */
    i += (uint16_t)(sizeof(ST25R95_AnalogConfigId) + sizeof(ST25R95_AnalogConfigNum)
                    + (configTbl[sizeof(ST25R95_AnalogConfigId)] * sizeof(ST25R95_AnalogConfigRegAddrMaskVal))
                   );
  } /* for */

  return ST25R95_ANALOG_CONFIG_LUT_NOT_FOUND;
}

NFC_OpResult ST25R95_AnalogConfig_Init(void)
{
  /* Use default Analog configuration settings in Flash by default. */
  gRfalAnalogConfigMgmt.currentAnalogConfigTbl = (const uint8_t *)&ST25R95_AnalogConfigDefaultSettings;
  gRfalAnalogConfigMgmt.configTblSize          = sizeof(ST25R95_AnalogConfigDefaultSettings);

  return NFC_OK;
}

NFC_OpResult ST25R95_AnalogConfig_List_Read_Raw(uint8_t *tblBuf, uint16_t tblBufLen, uint16_t *configTblSize)
{
  /* Check if the the current table will fit into the given buffer */
  if (tblBufLen < gRfalAnalogConfigMgmt.configTblSize) {
    return NFC_MemoryError;
  }

  /* Check for invalid parameters */
  if (configTblSize == NULL) {
    return NFC_InvalidParameter;
  }

  /* Copy the whole Table to the given buffer */
  if (gRfalAnalogConfigMgmt.configTblSize > 0U) {                    /* MISRA 21.18 */
    memcpy(tblBuf, gRfalAnalogConfigMgmt.currentAnalogConfigTbl, gRfalAnalogConfigMgmt.configTblSize);
  }
  *configTblSize = gRfalAnalogConfigMgmt.configTblSize;

  return NFC_OK;
}

NFC_OpResult ST25R95_AnalogConfig_List_Read(ST25R95_AnalogConfigOffset *configOffset, uint8_t *more, ST25R95_AnalogConfig *config, ST25R95_AnalogConfigNum numConfig)
{
  uint16_t configSize;
  ST25R95_AnalogConfigOffset offset = *configOffset;
  ST25R95_AnalogConfigNum numConfigSet;

  /* Check if the number of register-mask-value settings for the respective Configuration ID will fit into the buffer passed in. */
  if (gRfalAnalogConfigMgmt.currentAnalogConfigTbl[offset + sizeof(ST25R95_AnalogConfigId)] > numConfig) {
    return NFC_MemoryError;
  }

  /* Get the number of Configuration set */
  numConfigSet = gRfalAnalogConfigMgmt.currentAnalogConfigTbl[offset + sizeof(ST25R95_AnalogConfigId)];

  /* Pass Configuration Register-Mask-Value sets */
  configSize = (sizeof(ST25R95_AnalogConfigId) + sizeof(ST25R95_AnalogConfigNum) + (uint16_t)(numConfigSet * sizeof(ST25R95_AnalogConfigRegAddrMaskVal)));
  memcpy((uint8_t *) config
            , &gRfalAnalogConfigMgmt.currentAnalogConfigTbl[offset]
            , configSize
           );
  *configOffset = offset + configSize;

  /* Check if it is the last Analog Configuration in the Table.*/
  *more = (uint8_t)((*configOffset >= gRfalAnalogConfigMgmt.configTblSize) ? ST25R95_ANALOG_CONFIG_UPDATE_LAST
                    : ST25R95_ANALOG_CONFIG_UPDATE_MORE);

  return NFC_OK;
} /* ST25R95_AnalogConfigListRead() */


NFC_OpResult ST25R95_AnalogConfig_Set(ST25R95_AnalogConfigId configId)
{
  ST25R95_AnalogConfigOffset configOffset = 0;
  ST25R95_AnalogConfigNum numConfigSet;
  ST25R95_AnalogConfigRegAddrMaskVal *configTbl;
  ST25R95_AnalogConfigNum i;
  NFC_OpResult op_status;

  /* Search LUT for the specific Configuration ID. */
  while (true) {
    numConfigSet = ST25R95_AnalogConfigSearch(configId, &configOffset);
    if (ST25R95_ANALOG_CONFIG_LUT_NOT_FOUND == numConfigSet) {
      break;
    }

    configTbl = (ST25R95_AnalogConfigRegAddrMaskVal *)((uint32_t)gRfalAnalogConfigMgmt.currentAnalogConfigTbl + (uint32_t)configOffset);
    /* Increment the offset to the next index to search from. */
    configOffset += (uint16_t)(numConfigSet * sizeof(ST25R95_AnalogConfigRegAddrMaskVal));

    if ((gRfalAnalogConfigMgmt.configTblSize + 1U) < configOffset) {
      /* Error check make sure that the we do not access outside the configuration Table Size */
      return NFC_MemoryError;
    }

    for (i = 0; i < numConfigSet; i++) {
      if ((GETU16(configTbl[i].addr) & ST25R95_TEST_REG) == 0U)
      {
        op_status = ST25R95_ChipChangeRegBits(GETU16(configTbl[i].addr), configTbl[i].mask, configTbl[i].val);
        if(op_status < NFC_OK)
        {
          return op_status;
        }
      }
      else
      {
        return NFC_Unsupport;
      }
    }

  } /* while(found Analog Config Id) */

  return op_status;
}

