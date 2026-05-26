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
#include <string.h>

#include "ST25R95_Def.h"

#include "RFal_ST25R95_AnalogConfig.h"
#include "RFal_ST25R95_AnalogConfigTable.h"

#include "NFC_Config.h"

#ifdef CONFIG_NFC_READER_DEVICE_ST25R95

#define RFal_TEST_REG         0x0080U      /*!< Test Register indicator  */

RFal_AnalogConfigMgmt gRfalAnalogConfigMgmt;  /*!< Analog Configuration LUT management */

static RFal_AnalogConfigNum RFal_AnalogConfigSearch(RFal_AnalogConfigId configId, uint16_t *configOffset)
{
  RFal_AnalogConfigId foundConfigId;
  RFal_AnalogConfigId configIdMaskVal;
  const uint8_t *configTbl;
  const uint8_t *currentConfigTbl;
  uint16_t i;

  currentConfigTbl = gRfalAnalogConfigMgmt.currentAnalogConfigTbl;
  configIdMaskVal  = ((RFAL_ST25R95_ANALOG_CONFIG_POLL_LISTEN_MODE_MASK | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_MASK)
                      | ((RFAL_ST25R95_ANALOG_CONFIG_TECH_CHIP == RFAL_ST25R95_ANALOG_CONFIG_ID_GET_TECH(configId)) ? (RFAL_ST25R95_ANALOG_CONFIG_TECH_MASK | RFAL_ST25R95_ANALOG_CONFIG_CHIP_SPECIFIC_MASK) : configId)
                      | ((RFAL_ST25R95_ANALOG_CONFIG_NO_DIRECTION == RFAL_ST25R95_ANALOG_CONFIG_ID_GET_DIRECTION(configId)) ? RFAL_ST25R95_ANALOG_CONFIG_DIRECTION_MASK : configId)
                     );


  /* When specific ConfigIDs are to be used, override search mask */
  if ((RFAL_ST25R95_ANALOG_CONFIG_ID_GET_DIRECTION(configId) == RFAL_ST25R95_ANALOG_CONFIG_DPO)) {
    configIdMaskVal = (RFAL_ST25R95_ANALOG_CONFIG_POLL_LISTEN_MODE_MASK | RFAL_ST25R95_ANALOG_CONFIG_TECH_MASK | RFAL_ST25R95_ANALOG_CONFIG_BITRATE_MASK | RFAL_ST25R95_ANALOG_CONFIG_DIRECTION_MASK);
  }


  i = *configOffset;
  while (i < gRfalAnalogConfigMgmt.configTblSize) {
    configTbl = &currentConfigTbl[i];
    foundConfigId = (((uint16_t)(configTbl)[0] << 8) | (uint16_t)(configTbl)[1]);
    if (configId == (foundConfigId & configIdMaskVal)) {
      *configOffset = (uint16_t)(i + sizeof(RFal_AnalogConfigId) + sizeof(RFal_AnalogConfigNum));
      return configTbl[sizeof(RFal_AnalogConfigId)];
    }

    /* If Config Id does not match, increment to next Configuration Id */
    i += (uint16_t)(sizeof(RFal_AnalogConfigId) + sizeof(RFal_AnalogConfigNum)
                    + (configTbl[sizeof(RFal_AnalogConfigId)] * sizeof(RFal_AnalogConfigRegAddrMaskVal))
                   );
  } /* for */

  return RFAL_ST25R95_ANALOG_CONFIG_LUT_NOT_FOUND;
}

NFC_OpResult RFal_AnalogConfig_Init(void)
{
  /* Use default Analog configuration settings in Flash by default. */
  gRfalAnalogConfigMgmt.currentAnalogConfigTbl = (const uint8_t *)&RFal_AnalogConfigDefaultSettings;
  gRfalAnalogConfigMgmt.configTblSize          = sizeof(RFal_AnalogConfigDefaultSettings);

  return NFC_OK;
}

NFC_OpResult RFal_AnalogConfig_List_Read_Raw(uint8_t *tblBuf, uint16_t tblBufLen, uint16_t *configTblSize)
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

NFC_OpResult RFal_AnalogConfig_List_Read(RFal_AnalogConfigOffset *configOffset, uint8_t *more, RFal_AnalogConfig *config, RFal_AnalogConfigNum numConfig)
{
  uint16_t configSize;
  RFal_AnalogConfigOffset offset = *configOffset;
  RFal_AnalogConfigNum numConfigSet;

  /* Check if the number of register-mask-value settings for the respective Configuration ID will fit into the buffer passed in. */
  if (gRfalAnalogConfigMgmt.currentAnalogConfigTbl[offset + sizeof(RFal_AnalogConfigId)] > numConfig) {
    return NFC_MemoryError;
  }

  /* Get the number of Configuration set */
  numConfigSet = gRfalAnalogConfigMgmt.currentAnalogConfigTbl[offset + sizeof(RFal_AnalogConfigId)];

  /* Pass Configuration Register-Mask-Value sets */
  configSize = (sizeof(RFal_AnalogConfigId) + sizeof(RFal_AnalogConfigNum) + (uint16_t)(numConfigSet * sizeof(RFal_AnalogConfigRegAddrMaskVal)));
  memcpy((uint8_t *) config
            , &gRfalAnalogConfigMgmt.currentAnalogConfigTbl[offset]
            , configSize
           );
  *configOffset = offset + configSize;

  /* Check if it is the last Analog Configuration in the Table.*/
  *more = (uint8_t)((*configOffset >= gRfalAnalogConfigMgmt.configTblSize) ? RFAL_ST25R95_ANALOG_CONFIG_UPDATE_LAST
                    : RFAL_ST25R95_ANALOG_CONFIG_UPDATE_MORE);

  return NFC_OK;
} /* RFal_AnalogConfigListRead() */


NFC_OpResult RFal_AnalogConfig_Set(RFal_AnalogConfigId configId)
{
  RFal_AnalogConfigOffset configOffset = 0;
  RFal_AnalogConfigNum numConfigSet;
  RFal_AnalogConfigRegAddrMaskVal *configTbl;
  RFal_AnalogConfigNum i;
  NFC_OpResult op_status;

  /* Search LUT for the specific Configuration ID. */
  while (true) {
    numConfigSet = RFal_AnalogConfigSearch(configId, &configOffset);
    if (RFAL_ST25R95_ANALOG_CONFIG_LUT_NOT_FOUND == numConfigSet) {
      break;
    }

    configTbl = (RFal_AnalogConfigRegAddrMaskVal *)((uint32_t)gRfalAnalogConfigMgmt.currentAnalogConfigTbl + (uint32_t)configOffset);
    /* Increment the offset to the next index to search from. */
    configOffset += (uint16_t)(numConfigSet * sizeof(RFal_AnalogConfigRegAddrMaskVal));

    if ((gRfalAnalogConfigMgmt.configTblSize + 1U) < configOffset) {
      /* Error check make sure that the we do not access outside the configuration Table Size */
      return NFC_MemoryError;
    }

    for (i = 0; i < numConfigSet; i++) {
      if (((((uint16_t)(configTbl[i].addr)[0] << 8) | (uint16_t)(configTbl[i].addr)[1]) & RFal_TEST_REG) == 0U)
      {
        op_status = RFal_ChipChangeRegBits((((uint16_t)(configTbl[i].addr)[0] << 8) | (uint16_t)(configTbl[i].addr)[1]), configTbl[i].mask, configTbl[i].val);
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

  return NFC_OK;
}

#endif //CONFIG_NFC_READER_DEVICE_ST25R95
