
/**
  ******************************************************************************
  * @file           : NDef__type_wlc.cpp
  * @brief          : NDEF WLC (Wireless Charging) types
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
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
#include <string.h>

#include "NDef_Record.h"
#include "NDef_Types.h"
#include "NDef_Type_WLC.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_RTD_WLC_SUPPORT

#define NDEF_TYPE_RTD_WLC_PAYLOAD_LENGTH                    6U   /*!< WLC Capability/Poll Info/Listen Control Records Payload Length */

#define NDEF_TYPE_RTD_WLC_STATUS_INFO_MIN_PAYLOAD_LENGTH    1U   /*!< WLC Status Info Record Payload min length */
#define NDEF_TYPE_RTD_WLC_STATUS_INFO_MAX_PAYLOAD_LENGTH    9U   /*!< WLC Status Info Record Payload max length */


/* WLC Capability */
#define NDEF_WLC_CAPABILITY_PROTOCOL_VERSION_OFFSET         0U   /*!< WLC Capability Protocol Version Offset */
#define NDEF_WLC_CAPABILITY_CONFIG_OFFSET                   1U   /*!< WLC Capability Config Offset */
#define NDEF_WLC_CAPABILITY_WT_INT_OFFSET                   2U   /*!< WLC Capability WT INT Offset */
#define NDEF_WLC_CAPABILITY_NDEF_RD_WT_OFFSET               3U   /*!< WLC Capability NDEF RD WT Offset */
#define NDEF_WLC_CAPABILITY_NDEF_WR_TO_INT_OFFSET           4U   /*!< WLC Capability NDEF WR TO INT Offset */
#define NDEF_WLC_CAPABILITY_NDEF_WR_WT_INT_OFFSET           5U   /*!< WLC Capability NDEF WR WT INT Offset */

/* WLC Config: Protocol Version */
//#define NDEF_WLC_CAPABILITY_PROTOCOL_MAJOR_VERSION_SHIFT    4U  /*!< WLC Capability Protocol Major Version bit shift */
//#define NDEF_WLC_CAPABILITY_PROTOCOL_VERSION_MASK         0xFU  /*!< WLC Capability Protocol Major Version mask */

/* WLC Config: Mode Req */
#define NDEF_WLC_CAPABILITY_CONFIG_MODE_REQ_SHIFT           6U /*! WLC Capability Config Req Mode bit shift */
#define NDEF_WLC_CAPABILITY_CONFIG_MODE_REQ_MASK            3U /*! WLC Capability Config Req Mode mask */

/* WLC Config: Nego Wait Retry */
#define NDEF_WLC_CAPABILITY_CONFIG_WAIT_TIME_RETRY_SHIFT    2U /*! WLC Capability config Nego Wait Retry bit shift */
#define NDEF_WLC_CAPABILITY_CONFIG_WAIT_TIME_RETRY_MASK   0xFU /*! WLC Capability config Nego Wait Retry mask */

/* WLC Config: Nego Wait Retry flag */
#define NDEF_WLC_CAPABILITY_CONFIG_NEGO_WAIT_SHIFT          1U /*! WLC Capability config Nego Wait bit shift */
#define NDEF_WLC_CAPABILITY_CONFIG_NEGO_WAIT_MASK           1U /*! WLC Capability config Nego Wait mask */

/* WLC Config: Rd Conf flag */
#define NDEF_WLC_CAPABILITY_CONFIG_RD_CONF_SHIFT            0U /*! WLC Capability config Rd Conf bit shift */
#define NDEF_WLC_CAPABILITY_CONFIG_RD_CONF_MASK             1U /*! WLC Capability config Rd Conf mask */

/* WLC Config: CapWtInt RFU */
#define NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_RFU_SHIFT     5U /*! WLC Capability config CapWtInt RFU bit shift */
#define NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_RFU_MASK    0x7U /*! WLC Capability config CapWtInt RFU mask */

/* WLC Config: CapWtInt */
#define NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_SHIFT         0U /*! WLC Capability config CapWtInt bit shift */
#define NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_MASK       0x1FU /*! WLC Capability config CapWtInt mask */

//#define NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_MAX        0x13U /*! WLC Capability config CapWtInt max value */
//#define NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_MASK       0x1FU /*! WLC Capability config CapWtInt mask */

//#define NDEF_WLC_CAPABILITY_CONFIG_RD_CONF_MASK             1U /*! WLC Capability config Rd Conf mask */

/* WLC Status and Information */
#define NDEF_WLC_STATUSINFO_CONTROL_BYTE_1_OFFSET           0U /*! WLC Status and Info Control byte 1 offset */

/* WLC Poll Info */
#define NDEF_WLC_POLL_INFO_PTX_OFFSET                       0U   /*!< WLC Poll Info P Tx Offset */
#define NDEF_WLC_POLL_INFO_WLC_P_CAPABILITY_OFFSET          1U   /*!< WLC Poll Info WLC-P Capability Offset */
#define NDEF_WLC_POLL_INFO_POWER_CLASS_OFFSET               1U   /*!< WLC Poll Info Power Class Offset */
#define NDEF_WLC_POLL_INFO_TOT_POWER_STEPS_OFFSET           2U   /*!< WLC Poll Info Total Power Steps Offset */
#define NDEF_WLC_POLL_INFO_CUR_POWER_STEP_OFFSET            3U   /*!< WLC Poll Info Current Power Step Offset */
#define NDEF_WLC_POLL_INFO_NEXT_MIN_STEP_INC_OFFSET         4U   /*!< WLC Poll Info Next Min Step Inc Offset */
#define NDEF_WLC_POLL_INFO_NEXT_MIN_STEP_DEC_OFFSET         5U   /*!< WLC Poll Info Next Min Step Dec Offset */

#define NDEF_WLC_POLL_INFO_WLC_P_CAPABILITY_SHIFT           4U   /*!< WLC Poll Info WLC-P Capability bit shift */
#define NDEF_WLC_POLL_INFO_WLC_P_CAPABILITY_MASK          0xFU   /*!< WLC Poll Info WLC-P Capability mask */

/* WLC Listen Ctl */
#define NDEF_WLC_LISTEN_CTL_STATUS_INFO_OFFSET              0U   /*!< WLC Listen Control Status Info Offset */
#define NDEF_WLC_LISTEN_CTL_WPT_CONFIG_OFFSET               1U   /*!< WLC Listen Control WPT Config Offset */
#define NDEF_WLC_LISTEN_CTL_POWER_ADJ_REQ_OFFSET            2U   /*!< WLC Listen Control Power Adjust Request Offset */
#define NDEF_WLC_LISTEN_CTL_BATTERY_LEVEL_OFFSET            3U   /*!< WLC Listen Control Battery Level Offset */
#define NDEF_WLC_LISTEN_CTL_DRV_INFO_OFFSET                 4U   /*!< WLC Listen Control Drv Info Offset */
#define NDEF_WLC_LISTEN_CTL_HOLD_OFF_WT_INT_OFFSET          5U   /*!< WLC Listen Control Hold Off WT INT Offset */
#define NDEF_WLC_LISTEN_CTL_ERROR_INFO_OFFSET               5U   /*!< WLC Listen Control Error Info Offset */

/* WLC Listen Ctl: Status Info */
#define NDEF_WLC_LISTEN_CTL_STATUS_INFO_ERROR_SHIFT          7U /*! WLC Listen Control Status Error Flag bit shift */
#define NDEF_WLC_LISTEN_CTL_STATUS_INFO_ERROR_MASK           1U /*! WLC Listen Control Status Error Flag mask */

/* WLC Listen Ctl: Battery Status */
#define NDEF_WLC_LISTEN_CTL_STATUS_INFO_BATTERY_SHIFT       3U /*! WLC Listen Control Battery Status bit shift */
#define NDEF_WLC_LISTEN_CTL_STATUS_INFO_BATTERY_MASK        3U /*! WLC Listen Control Battery Status mask */

/* WLC Listen Ctl: Status Counter */
#define NDEF_WLC_LISTEN_CTL_STATUS_INFO_COUNTER_SHIFT       0U /*! WLC Listen Control Status Counter bit shift */
#define NDEF_WLC_LISTEN_CTL_STATUS_INFO_COUNTER_MASK      0x7U /*! WLC Listen Control Status Counter Status mask */

/* WLC Listen Ctl: WPT Config Req */
#define NDEF_WLC_LISTEN_CTL_WPT_CONFIG_REQ_SHIFT            6U /*! WLC Listen Control WPT Config Req bit shift */
#define NDEF_WLC_LISTEN_CTL_WPT_CONFIG_REQ_MASK             3U /*! WLC Listen Control WPT Config Req mask */

/* WLC Listen Ctl: WPT Config Duration */
#define NDEF_WLC_LISTEN_CTL_WPT_CONFIG_DURATION_SHIFT       1U /*! WLC Listen Control WPT Config Duration bit shift */
#define NDEF_WLC_LISTEN_CTL_WPT_CONFIG_DURATION_MASK     0x1FU /*! WLC Listen Control WPT Config Duration mask */

/* WLC Listen Ctl: WPT Config Info Request */
#define NDEF_WLC_LISTEN_CTL_WPT_CONFIG_INFO_REQ_SHIFT       0U /*! WLC Listen Control WPT Config Info Request bit shift */
#define NDEF_WLC_LISTEN_CTL_WPT_CONFIG_INFO_REQ_MASK        1U /*! WLC Listen Control WPT Config Info Request mask */

/* WLC Listen Ctl: DRV Info Flag */
#define NDEF_WLC_LISTEN_CTL_DRV_INFO_FLAG_SHIFT             6U /*! WLC Listen Control Drv Info Flag bit shift */
#define NDEF_WLC_LISTEN_CTL_DRV_INFO_FLAG_MASK              3U /*! WLC Listen Control Drv Info Flag mask */

/* WLC Listen Ctl: DRV Info Int */
#define NDEF_WLC_LISTEN_CTL_DRV_INFO_INT_SHIFT              0U /*! WLC Listen Control Drv Info Int bit shift */
#define NDEF_WLC_LISTEN_CTL_DRV_INFO_INT_MASK            0x3FU /*! WLC Listen Control Drv Info Int mask */

/* WLC Listen Ctl: Error Info/WLC Protocol Error */
#define NDEF_WLC_LISTEN_CTL_ERROR_INFO_PROTOCOL_SHIFT       1U /*! WLC Listen Control Error Info Protocol bit shift */
#define NDEF_WLC_LISTEN_CTL_ERROR_INFO_PROTOCOL_MASK        1U /*! WLC Listen Control Error Info Protocol mask */

/* WLC Listen Ctl: Error Info/Temperature */
#define NDEF_WLC_LISTEN_CTL_ERROR_INFO_TEMPERATURE_SHIFT    0U /*! WLC Listen Control Error Info Temperature bit shift */
#define NDEF_WLC_LISTEN_CTL_ERROR_INFO_TEMPERATURE_MASK     1U /*! WLC Listen Control Error Info Temperature mask */

/*! RTD WLC Type strings */
static const uint8_t NDef_RtdTypeWlcCapability[] = "WLCCAP";  /*!< WLC Capability Record Type             */
static const uint8_t NDef_RtdTypeWlcStatusInfo[] = "WLCSTAI"; /*!< WLC Status and Information Record Type */
static const uint8_t NDef_RtdTypeWlcPollInfo[]   = "WLCINF";  /*!< WLC Poll Info Record Type              */
static const uint8_t NDef_RtdTypeWlcListenCtl[]  = "WLCCTL";  /*!< WLC Listen Control Record Type         */

const NDef_Const_Buffer_8 bufTypeRtdWlcCapability = { NDef_RtdTypeWlcCapability, sizeof(NDef_RtdTypeWlcCapability) - 1U };    /*!< WLC Capabiilty Type Record buffer       */
const NDef_Const_Buffer_8 bufTypeRtdWlcStatusInfo = { NDef_RtdTypeWlcStatusInfo, sizeof(NDef_RtdTypeWlcStatusInfo) - 1U };    /*!< WLC Capabiilty Type Record buffer       */
const NDef_Const_Buffer_8 bufTypeRtdWlcPollInfo   = { NDef_RtdTypeWlcPollInfo,   sizeof(NDef_RtdTypeWlcPollInfo) - 1U };      /*!< WLC Poll Information Type Record buffer */
const NDef_Const_Buffer_8 bufTypeRtdWlcListenCtl  = { NDef_RtdTypeWlcListenCtl,  sizeof(NDef_RtdTypeWlcListenCtl) - 1U };     /*!< WLC Listen Control Type Record buffer   */

/*****************************************************************************/
static uint32_t NDef_RtdWlcCapabilityGetPayloadLength(const NDef_Type *type)
{
  if ((type == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCCAP)) {
    return 0;
  }

  return NDEF_TYPE_RTD_WLC_PAYLOAD_LENGTH;
}


/*****************************************************************************/
static const uint8_t *NDef_RtdWlcCapabilityGetPayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  static uint8_t temp = 0;
  const NDef_Type_Rtd_WlcCapability *NDef_Data;

  if ((type    == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCCAP) ||
      (bufItem == NULL)) {
    return NULL;
  }

  NDef_Data = &type->data.wlcCapability;

  if (begin == true) {
    item = 0;
  }

  /* Prepare bufItem->length: Each field is 1 byte, except when complete */
  bufItem->length = sizeof(uint8_t);

  switch (item) {
    case 0: /* WLC Protocol Version */
      bufItem->buffer = &NDef_Data->wlcProtocolVersion;
      break;
    case 1: { /* WLC Config */
        temp =
          ((NDef_Data->wlcConfigModeReq       & NDEF_WLC_CAPABILITY_CONFIG_MODE_REQ_MASK)        << NDEF_WLC_CAPABILITY_CONFIG_MODE_REQ_SHIFT)
          | ((NDef_Data->wlcConfigWaitTimeRetry & NDEF_WLC_CAPABILITY_CONFIG_WAIT_TIME_RETRY_MASK) << NDEF_WLC_CAPABILITY_CONFIG_WAIT_TIME_RETRY_SHIFT)
          | ((NDef_Data->wlcConfigNegoWait      & NDEF_WLC_CAPABILITY_CONFIG_NEGO_WAIT_MASK)       << NDEF_WLC_CAPABILITY_CONFIG_NEGO_WAIT_SHIFT)
          | ((NDef_Data->wlcConfigRdConf        & NDEF_WLC_CAPABILITY_CONFIG_RD_CONF_MASK)         << NDEF_WLC_CAPABILITY_CONFIG_RD_CONF_SHIFT);

        bufItem->buffer = &temp;
        break;
      }
    case 2: /* Cap Wt Int */
      temp =
        ((NDef_Data->capWtIntRfu & NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_RFU_MASK) << NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_RFU_SHIFT)
        | ((NDef_Data->capWtInt    & NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_MASK)     << NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_SHIFT);

      bufItem->buffer = &temp;
      break;
    case 3: /* NDEF Rd Wt */
      bufItem->buffer = &NDef_Data->ndefRdWt;
      break;
    case 4: /* NDEF Write To Int */
      bufItem->buffer = &NDef_Data->ndefWriteToInt;
      break;
    case 5: /* NDEF Write Wt Int */
      bufItem->buffer = &NDef_Data->ndefWriteWtInt;
      break;
    default:
      bufItem->buffer = NULL;
      bufItem->length = 0;
      break;
  }

  /* Move to next item for next call */
  item++;

  return bufItem->buffer;
}


/*****************************************************************************/
NFC_OpResult NDef_RtdWlcCapabilityInit(NDef_Type *type, const NDef_Type_Rtd_WlcCapability *param)
{
  NDef_Type_Rtd_WlcCapability *NDef_Data;

  if ((type == NULL) || (param == NULL)) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_WLCCAP;
  type->getPayloadLength = NDef_RtdWlcCapabilityGetPayloadLength;
  type->getPayloadItem   = NDef_RtdWlcCapabilityGetPayloadItem;
  type->typeToRecord     = NDef_RtdWlcCapabilityToRecord;
  NDef_Data               = &type->data.wlcCapability;

  (void)memcpy(NDef_Data, param, sizeof(NDef_Type_Rtd_WlcCapability));

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdWlcCapability(const NDef_Type *type, NDef_Type_Rtd_WlcCapability *param)
{
  const NDef_Type_Rtd_WlcCapability *NDef_Data;

  if ((type  == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCCAP) ||
      (param == NULL)) {
    return NFC_InvalidParameter;
  }

  NDef_Data = &type->data.wlcCapability;

  (void)memcpy(param, NDef_Data, sizeof(NDef_Type_Rtd_WlcCapability));

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdWlcCapability(const NDef_Const_Buffer *bufPayload, NDef_Type *type)
{
  NDef_Type_Rtd_WlcCapability *NDef_Data;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (type       == NULL)) {
    return NFC_InvalidParameter;
  }

  if (bufPayload->length != NDEF_TYPE_RTD_WLC_PAYLOAD_LENGTH) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_WLCCAP;
  type->getPayloadLength = NDef_RtdWlcCapabilityGetPayloadLength;
  type->getPayloadItem   = NDef_RtdWlcCapabilityGetPayloadItem;
  type->typeToRecord     = NDef_RtdWlcCapabilityToRecord;
  NDef_Data               = &type->data.wlcCapability;

  NDef_Data->wlcProtocolVersion     = bufPayload->buffer[NDEF_WLC_CAPABILITY_PROTOCOL_VERSION_OFFSET];

  uint8_t config = bufPayload->buffer[NDEF_WLC_CAPABILITY_CONFIG_OFFSET];
  NDef_Data->wlcConfigModeReq       = (config >> NDEF_WLC_CAPABILITY_CONFIG_MODE_REQ_SHIFT)        & NDEF_WLC_CAPABILITY_CONFIG_MODE_REQ_MASK;
  NDef_Data->wlcConfigWaitTimeRetry = (config >> NDEF_WLC_CAPABILITY_CONFIG_WAIT_TIME_RETRY_SHIFT) & NDEF_WLC_CAPABILITY_CONFIG_WAIT_TIME_RETRY_MASK;
  NDef_Data->wlcConfigNegoWait      = (config >> NDEF_WLC_CAPABILITY_CONFIG_NEGO_WAIT_SHIFT)       & NDEF_WLC_CAPABILITY_CONFIG_NEGO_WAIT_MASK;
  NDef_Data->wlcConfigRdConf        = (config >> NDEF_WLC_CAPABILITY_CONFIG_RD_CONF_SHIFT)         & NDEF_WLC_CAPABILITY_CONFIG_RD_CONF_MASK;

  uint8_t capWtInt = bufPayload->buffer[NDEF_WLC_CAPABILITY_WT_INT_OFFSET];
  NDef_Data->capWtIntRfu            = (capWtInt >> NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_RFU_SHIFT) & NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_RFU_MASK;
  NDef_Data->capWtInt               = (capWtInt >> NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_SHIFT)     & NDEF_WLC_CAPABILITY_CONFIG_CAP_WT_INT_MASK;
  NDef_Data->ndefRdWt               = bufPayload->buffer[NDEF_WLC_CAPABILITY_NDEF_RD_WT_OFFSET];
  NDef_Data->ndefWriteToInt         = bufPayload->buffer[NDEF_WLC_CAPABILITY_NDEF_WR_TO_INT_OFFSET];
  NDef_Data->ndefWriteWtInt         = bufPayload->buffer[NDEF_WLC_CAPABILITY_NDEF_WR_WT_INT_OFFSET];

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdWlcCapability(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *NDef_Data;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  /* NDEF TNF and String type */
  if (! NDef_Record_TypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcCapability)) {
    return NFC_ProtocolError;
  }

  NDef_Data = NDef_RecordGetNDefType(record);
  if ((NDef_Data != NULL) && (NDef_Data->id == NDEF_TYPE_ID_RTD_WLCCAP)) {
    (void)memcpy(type, NDef_Data, sizeof(NDef_Type));
    return NFC_OK;
  }

  return NDef_PayloadToRtdWlcCapability(&record->bufPayload, type);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdWlcCapabilityToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type   == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCCAP) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  /* String type */
  (void)NDef_Record_SetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcCapability);

  if (NDef_RecordSetNDefType(record, type) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

/*****************************************************************************/
static uint32_t NDef_RtdWlcStatusInfoGetPayloadLength(const NDef_Type *type)
{
  uint32_t length;

  if ((type == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCSTAI)) {
    return 0;
  }

  const NDef_Type_Rtd_WlcStatusInfo *NDef_Data = &type->data.wlcStatusInfo;

  length = sizeof(uint8_t); /* Control byte 1 */

  /* Count the number of bits set in the Control byte 1 to get the number of following bytes */
  for (uint32_t i = 0; i < 8U; i++) {
    if ((NDef_Data->controlByte1 & (1U << i)) != 0U) {
      length += sizeof(uint8_t);
    }
  }

  return length;
}


/*****************************************************************************/
static const uint8_t *NDef_RtdWlcStatusInfoGetPayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Rtd_WlcStatusInfo *NDef_Data;

  if ((type    == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCSTAI) ||
      (bufItem == NULL)) {
    return NULL;
  }

  NDef_Data = &type->data.wlcStatusInfo;

  if (begin == true) {
    item = 0;
  }

  /* Prepare bufItem->length: Each field is 1 byte, except when complete */
  bufItem->length = sizeof(uint8_t);

  switch (item) {
    case 0: /* Control Byte 1 */
      bufItem->buffer = &NDef_Data->controlByte1;
      item++;
      break;
    case 1: /* Battery Level */
      if ((NDef_Data->controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_BATTERY_LEVEL_MASK) != 0U) {
        bufItem->buffer = &NDef_Data->batteryLevel;
        item = 2;
        break;
      }
    /* fall through */
    case 2: /* Receive Power */
      if ((NDef_Data->controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_RECEIVE_POWER_MASK) != 0U) {
        bufItem->buffer = &NDef_Data->receivePower;
        item = 3;
        break;
      }
    /* fall through */
    case 3: /* Receive Voltage */
      if ((NDef_Data->controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_RECEIVE_VOLTAGE_MASK) != 0U) {
        bufItem->buffer = &NDef_Data->receiveVoltage;
        item = 4;
        break;
      }
    /* fall through */
    case 4: /* Receive Current */
      if ((NDef_Data->controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_RECEIVE_CURRENT_MASK) != 0U) {
        bufItem->buffer = &NDef_Data->receiveCurrent;
        item = 5;
        break;
      }
    /* fall through */
    case 5: /* Receive Current */
      if ((NDef_Data->controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_TEMPERATURE_BATTERY_MASK) != 0U) {
        bufItem->buffer = &NDef_Data->temperatureBattery;
        item = 6;
        break;
      }
    /* fall through */
    case 6: /* Receive Current */
      if ((NDef_Data->controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_TEMPERATURE_WLCL_MASK) != 0U) {
        bufItem->buffer = &NDef_Data->temperatureWlcl;
        item = 7;
        break;
      }
    /* fall through */
    case 7: /* Receive Current */
      if ((NDef_Data->controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_RFU_MASK) != 0U) {
        bufItem->buffer = &NDef_Data->rfu;
        item = 8;
        break;
      }
    /* fall through */
    case 8: /* Receive Current */
      if ((NDef_Data->controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_CONTROL_BYTE_2_MASK) != 0U) {
        bufItem->buffer = &NDef_Data->controlByte2;
        item = 9;
        break;
      }
    /* fall through */
    default:
      bufItem->buffer = NULL;
      bufItem->length = 0;
      break;
  }

  return bufItem->buffer;
}


/*****************************************************************************/
NFC_OpResult NDef_RtdWlcStatusInfoInit(NDef_Type *type, const NDef_Type_Rtd_WlcStatusInfo *param)
{
  NDef_Type_Rtd_WlcStatusInfo *NDef_Data;

  if ((type == NULL) || (param == NULL)) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_WLCSTAI;
  type->getPayloadLength = NDef_RtdWlcStatusInfoGetPayloadLength;
  type->getPayloadItem   = NDef_RtdWlcStatusInfoGetPayloadItem;
  type->typeToRecord     = NDef_RtdWlcStatusInfoToRecord;
  NDef_Data               = &type->data.wlcStatusInfo;

  (void)memcpy(NDef_Data, param, sizeof(NDef_Type_Rtd_WlcStatusInfo));

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdWlcStatusInfo(const NDef_Type *type, NDef_Type_Rtd_WlcStatusInfo *param)
{
  const NDef_Type_Rtd_WlcStatusInfo *NDef_Data;

  if ((type  == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCSTAI) ||
      (param == NULL)) {
    return NFC_InvalidParameter;
  }

  NDef_Data = &type->data.wlcStatusInfo;

  (void)memcpy(param, NDef_Data, sizeof(NDef_Type_Rtd_WlcStatusInfo));

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdWlcStatusInfo(const NDef_Const_Buffer *bufPayload, NDef_Type *type)
{
  NDef_Type_Rtd_WlcStatusInfo *NDef_Data;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (type       == NULL)) {
    return NFC_InvalidParameter;
  }

  if ((bufPayload->length < NDEF_TYPE_RTD_WLC_STATUS_INFO_MIN_PAYLOAD_LENGTH) ||
      (bufPayload->length > NDEF_TYPE_RTD_WLC_STATUS_INFO_MAX_PAYLOAD_LENGTH)) {
    return NFC_ProtocolError;
  }

  type->id               = NDEF_TYPE_ID_RTD_WLCSTAI;
  type->getPayloadLength = NDef_RtdWlcStatusInfoGetPayloadLength;
  type->getPayloadItem   = NDef_RtdWlcStatusInfoGetPayloadItem;
  type->typeToRecord     = NDef_RtdWlcStatusInfoToRecord;
  NDef_Data               = &type->data.wlcStatusInfo;

  uint32_t offset = NDEF_WLC_STATUSINFO_CONTROL_BYTE_1_OFFSET;

  uint8_t controlByte1 = bufPayload->buffer[offset];
  offset++;

  /* Initialize each field */
  NDef_Data->controlByte1       = controlByte1;
  NDef_Data->batteryLevel       = 0;
  NDef_Data->receivePower       = 0;
  NDef_Data->receiveVoltage     = 0;
  NDef_Data->receiveCurrent     = 0;
  NDef_Data->temperatureBattery = 0;
  NDef_Data->temperatureWlcl    = 0;
  NDef_Data->rfu                = 0;
  NDef_Data->controlByte2       = 0;

  /* Check each bit in Control byte 1 and read the appropriate byte */
  if ((controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_BATTERY_LEVEL_MASK) != 0U) {
    NDef_Data->batteryLevel = bufPayload->buffer[offset];
    offset++;
  }

  if ((controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_RECEIVE_POWER_MASK) != 0U) {
    NDef_Data->receivePower = bufPayload->buffer[offset];
    offset++;
  }

  if ((controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_RECEIVE_VOLTAGE_MASK) != 0U) {
    NDef_Data->receiveVoltage = bufPayload->buffer[offset];
    offset++;
  }

  if ((controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_RECEIVE_CURRENT_MASK) != 0U) {
    NDef_Data->receiveCurrent = bufPayload->buffer[offset];
    offset++;
  }

  if ((controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_TEMPERATURE_BATTERY_MASK) != 0U) {
    NDef_Data->temperatureBattery = bufPayload->buffer[offset];
    offset++;
  }

  if ((controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_TEMPERATURE_WLCL_MASK) != 0U) {
    NDef_Data->temperatureWlcl = bufPayload->buffer[offset];
    offset++;
  }

  if ((controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_RFU_MASK) != 0U) {
    NDef_Data->rfu = bufPayload->buffer[offset];
    offset++;
  }

  if ((controlByte1 & NDEF_WLC_STATUSINFO_CONTROLBYTE1_CONTROL_BYTE_2_MASK) != 0U) {
    NDef_Data->controlByte2 = bufPayload->buffer[offset];
    /*offset++;*/
  }

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdWlcStatusInfo(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *NDef_Data;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  /* NDEF TNF and String type */
  if (! NDef_Record_TypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcStatusInfo)) {
    return NFC_ProtocolError;
  }

  NDef_Data = NDef_RecordGetNDefType(record);
  if ((NDef_Data != NULL) && (NDef_Data->id == NDEF_TYPE_ID_RTD_WLCSTAI)) {
    (void)memcpy(type, NDef_Data, sizeof(NDef_Type));
    return NFC_OK;
  }

  return NDef_PayloadToRtdWlcStatusInfo(&record->bufPayload, type);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdWlcStatusInfoToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type   == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCSTAI) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  /* String type */
  (void)NDef_Record_SetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcStatusInfo);

  if (NDef_RecordSetNDefType(record, type) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}


/*****************************************************************************/
/*
 * WLC Poll Information
 */
/*****************************************************************************/


/*****************************************************************************/
static uint32_t NDef_RtdWlcPollInfoGetPayloadLength(const NDef_Type *type)
{
  if ((type == NULL) || ((type)->id != NDEF_TYPE_ID_RTD_WLCINFO)) {
    return 0;
  }

  return NDEF_TYPE_RTD_WLC_PAYLOAD_LENGTH;
}


/*****************************************************************************/
static const uint8_t *NDef_RtdWlcPollInfoGetPayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Rtd_WlcPollInfo *NDef_Data;

  if ((type    == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCINFO) ||
      (bufItem == NULL)) {
    return NULL;
  }

  NDef_Data = &type->data.wlcPollInfo;

  if (begin == true) {
    item = 0;
  }

  /* Prepare bufItem->length: Each field is 1 byte, except when complete */
  bufItem->length = sizeof(uint8_t);

  switch (item) {
    case 0: /* P Tx */
      bufItem->buffer = &NDef_Data->pTx;
      break;
    case 1: { /* WLC-P Capability | Power class */
        static uint8_t temp;
        temp =
          ((NDef_Data->wlcPCap   << NDEF_WLC_POLL_INFO_WLC_P_CAPABILITY_SHIFT)
           | ((NDef_Data->powerClass & NDEF_WLC_POLL_INFO_WLC_P_CAPABILITY_MASK)));

        bufItem->buffer = &temp;
        break;
      }
    case 2: /* Total Power Steps */
      bufItem->buffer = &NDef_Data->totPowerSteps;
      break;
    case 3: /* Current Power Steps */
      bufItem->buffer = &NDef_Data->curPowerStep;
      break;
    case 4: /* Next min step increment */
      bufItem->buffer = &NDef_Data->nextMinStepInc;
      break;
    case 5: /* Next min step decrement */
      bufItem->buffer = &NDef_Data->nextMinStepDec;
      break;
    default:
      bufItem->buffer = NULL;
      bufItem->length = 0;
      break;
  }

  /* Move to next item for next call */
  item++;

  return bufItem->buffer;
}


/*****************************************************************************/
NFC_OpResult NDef_RtdWlcPollInfoInit(NDef_Type *type, const NDef_Type_Rtd_WlcPollInfo *param)
{
  NDef_Type_Rtd_WlcPollInfo *NDef_Data;

  if ((type == NULL) || (param == NULL)) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_WLCINFO;
  type->getPayloadLength = NDef_RtdWlcPollInfoGetPayloadLength;
  type->getPayloadItem   = NDef_RtdWlcPollInfoGetPayloadItem;
  type->typeToRecord     = NDef_RtdWlcPollInfoToRecord;
  NDef_Data               = &type->data.wlcPollInfo;

  (void)memcpy(NDef_Data, param, sizeof(NDef_Type_Rtd_WlcPollInfo));

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdWlcPollInfo(const NDef_Type *type, NDef_Type_Rtd_WlcPollInfo *param)
{
  const NDef_Type_Rtd_WlcPollInfo *NDef_Data;

  if ((type  == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCINFO) ||
      (param == NULL)) {
    return NFC_InvalidParameter;
  }

  NDef_Data = &type->data.wlcPollInfo;

  (void)memcpy(param, NDef_Data, sizeof(NDef_Type_Rtd_WlcPollInfo));

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdWlcPollInfo(const NDef_Const_Buffer *bufPayload, NDef_Type *type)
{
  NDef_Type_Rtd_WlcPollInfo *NDef_Data;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (type       == NULL)) {
    return NFC_InvalidParameter;
  }

  if (bufPayload->length != NDEF_TYPE_RTD_WLC_PAYLOAD_LENGTH) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_WLCINFO;
  type->getPayloadLength = NDef_RtdWlcPollInfoGetPayloadLength;
  type->getPayloadItem   = NDef_RtdWlcPollInfoGetPayloadItem;
  type->typeToRecord     = NDef_RtdWlcPollInfoToRecord;
  NDef_Data               = &type->data.wlcPollInfo;

  NDef_Data->pTx            = bufPayload->buffer[NDEF_WLC_POLL_INFO_PTX_OFFSET];
  NDef_Data->wlcPCap        = bufPayload->buffer[NDEF_WLC_POLL_INFO_WLC_P_CAPABILITY_OFFSET] >> NDEF_WLC_POLL_INFO_WLC_P_CAPABILITY_SHIFT;
  NDef_Data->powerClass     = bufPayload->buffer[NDEF_WLC_POLL_INFO_POWER_CLASS_OFFSET]       & NDEF_WLC_POLL_INFO_WLC_P_CAPABILITY_MASK;
  NDef_Data->totPowerSteps  = bufPayload->buffer[NDEF_WLC_POLL_INFO_TOT_POWER_STEPS_OFFSET];
  NDef_Data->curPowerStep   = bufPayload->buffer[NDEF_WLC_POLL_INFO_CUR_POWER_STEP_OFFSET];
  NDef_Data->nextMinStepInc = bufPayload->buffer[NDEF_WLC_POLL_INFO_NEXT_MIN_STEP_INC_OFFSET];
  NDef_Data->nextMinStepDec = bufPayload->buffer[NDEF_WLC_POLL_INFO_NEXT_MIN_STEP_DEC_OFFSET];

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdWlcPollInfo(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *NDef_Data;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  /* NDEF TNF and String type */
  if (! NDef_Record_TypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcPollInfo)) {
    return NFC_ProtocolError;
  }

  NDef_Data = NDef_RecordGetNDefType(record);
  if ((NDef_Data != NULL) && (NDef_Data->id == NDEF_TYPE_ID_RTD_WLCINFO)) {
    (void)memcpy(type, NDef_Data, sizeof(NDef_Type));
    return NFC_OK;
  }

  return NDef_PayloadToRtdWlcPollInfo(&record->bufPayload, type);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdWlcPollInfoToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type   == NULL) || ((type)->id != NDEF_TYPE_ID_RTD_WLCINFO) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  /* String type */
  (void)NDef_Record_SetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcPollInfo);

  if (NDef_RecordSetNDefType(record, type) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}


/*****************************************************************************/
/*
 * WLC Listen Control
 */
/*****************************************************************************/


/*****************************************************************************/
static uint32_t NDef_RtdWlcListenCtlGetPayloadLength(const NDef_Type *type)
{
  const NDef_Type_Rtd_WlcListenCtl *NDef_Data;
  uint32_t payloadLength;

  if ((type == NULL) || ((type)->id != NDEF_TYPE_ID_RTD_WLCCTL)) {
    return 0;
  }

  NDef_Data = &type->data.wlcListenCtl;

  payloadLength  = NDEF_TYPE_RTD_WLC_PAYLOAD_LENGTH;
  /* Check for optional last ERROR_INFO byte */
  payloadLength += ((NDef_Data->statusInfoErrorFlag != 0U) ? 1U : 0U);

  return payloadLength;
}


/*****************************************************************************/
static const uint8_t *NDef_RtdWlcListenCtlGetPayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  static uint8_t temp = 0;
  const NDef_Type_Rtd_WlcListenCtl *NDef_Data;

  if ((type    == NULL) || ((type)->id != NDEF_TYPE_ID_RTD_WLCCTL) ||
      (bufItem == NULL)) {
    return NULL;
  }

  NDef_Data = &type->data.wlcListenCtl;

  if (begin == true) {
    item = 0;
  }

  /* Prepare bufItem->length: Each field is 1 byte, except when complete */
  bufItem->length = sizeof(uint8_t);

  switch (item) {
    case 0: { /* Status Info */
        temp =
          ((NDef_Data->statusInfoErrorFlag     & NDEF_WLC_LISTEN_CTL_STATUS_INFO_ERROR_MASK)   << NDEF_WLC_LISTEN_CTL_STATUS_INFO_ERROR_SHIFT)
          | ((NDef_Data->statusInfoBatteryStatus & NDEF_WLC_LISTEN_CTL_STATUS_INFO_BATTERY_MASK) << NDEF_WLC_LISTEN_CTL_STATUS_INFO_BATTERY_SHIFT)
          | ((NDef_Data->statusInfoCnt           & NDEF_WLC_LISTEN_CTL_STATUS_INFO_COUNTER_MASK) << NDEF_WLC_LISTEN_CTL_STATUS_INFO_COUNTER_SHIFT);

        bufItem->buffer = &temp;
        break;
      }
    case 1: /* WPT Config */
      temp =
        ((NDef_Data->wptConfigWptReq      & NDEF_WLC_LISTEN_CTL_WPT_CONFIG_REQ_MASK)      << NDEF_WLC_LISTEN_CTL_WPT_CONFIG_REQ_SHIFT)
        | ((NDef_Data->wptConfigWptDuration & NDEF_WLC_LISTEN_CTL_WPT_CONFIG_DURATION_MASK) << NDEF_WLC_LISTEN_CTL_WPT_CONFIG_DURATION_SHIFT)
        | ((NDef_Data->wptConfigInfoReq     & NDEF_WLC_LISTEN_CTL_WPT_CONFIG_INFO_REQ_MASK) << NDEF_WLC_LISTEN_CTL_WPT_CONFIG_INFO_REQ_SHIFT);

      bufItem->buffer = &temp;
      break;
    case 2: /* Power Adjust Request */
      bufItem->buffer = &NDef_Data->powerAdjReq;
      break;
    case 3: /* Battery Level */
      bufItem->buffer = &NDef_Data->batteryLevel;
      break;
    case 4: { /* Drv Info */
        temp =
          ((NDef_Data->drvInfoFlag & NDEF_WLC_LISTEN_CTL_DRV_INFO_FLAG_MASK) << NDEF_WLC_LISTEN_CTL_DRV_INFO_FLAG_SHIFT)
          | ((NDef_Data->drvInfoInt  & NDEF_WLC_LISTEN_CTL_DRV_INFO_INT_MASK)  << NDEF_WLC_LISTEN_CTL_DRV_INFO_INT_SHIFT);

        bufItem->buffer = &temp;
        break;
      }
    case 5: /* Hold Off Wt Int */
      bufItem->buffer = &NDef_Data->holdOffWtInt;
      break;
    case 6: /* Optional last ERROR_INFO byte */
      if (NDef_Data->statusInfoErrorFlag != 0U) {
        /* Send the ERROR_INFO byte */
        temp =
          ((NDef_Data->errorInfoError       & NDEF_WLC_LISTEN_CTL_ERROR_INFO_PROTOCOL_MASK)    << NDEF_WLC_LISTEN_CTL_ERROR_INFO_PROTOCOL_SHIFT)
          | ((NDef_Data->errorInfoTemperature & NDEF_WLC_LISTEN_CTL_ERROR_INFO_TEMPERATURE_MASK) << NDEF_WLC_LISTEN_CTL_ERROR_INFO_TEMPERATURE_SHIFT);

        bufItem->buffer = &temp;
        break;
      }
    /* fall through if no ERR_INFO byte to send */
    default:
      bufItem->buffer = NULL;
      bufItem->length = 0;
      break;
  }

  /* Move to next item for next call */
  item++;

  return bufItem->buffer;
}


/*****************************************************************************/
NFC_OpResult NDef_RtdWlcListenCtlInit(NDef_Type *type, const NDef_Type_Rtd_WlcListenCtl *param)
{
  NDef_Type_Rtd_WlcListenCtl *NDef_Data;

  if ((type == NULL) || (param == NULL)) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_WLCCTL;
  type->getPayloadLength = NDef_RtdWlcListenCtlGetPayloadLength;
  type->getPayloadItem   = NDef_RtdWlcListenCtlGetPayloadItem;
  type->typeToRecord     = NDef_RtdWlcListenCtlToRecord;
  NDef_Data               = &type->data.wlcListenCtl;

  (void)memcpy(NDef_Data, param, sizeof(NDef_Type_Rtd_WlcListenCtl));

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdWlcListenCtl(const NDef_Type *type, NDef_Type_Rtd_WlcListenCtl *param)
{
  const NDef_Type_Rtd_WlcListenCtl *NDef_Data;

  if ((type  == NULL) || (type->id != NDEF_TYPE_ID_RTD_WLCCTL) ||
      (param == NULL)) {
    return NFC_InvalidParameter;
  }

  NDef_Data = &type->data.wlcListenCtl;

  (void)memcpy(param, NDef_Data, sizeof(NDef_Type_Rtd_WlcListenCtl));

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdWlcListenCtl(const NDef_Const_Buffer *bufPayload, NDef_Type *type)
{
  NDef_Type_Rtd_WlcListenCtl *NDef_Data;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (type       == NULL)) {
    return NFC_InvalidParameter;
  }

  if ((bufPayload->length !=  NDEF_TYPE_RTD_WLC_PAYLOAD_LENGTH) &&
      (bufPayload->length != (NDEF_TYPE_RTD_WLC_PAYLOAD_LENGTH + 1U))) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_WLCCTL;
  type->getPayloadLength = NDef_RtdWlcListenCtlGetPayloadLength;
  type->getPayloadItem   = NDef_RtdWlcListenCtlGetPayloadItem;
  type->typeToRecord     = NDef_RtdWlcListenCtlToRecord;
  NDef_Data               = &type->data.wlcListenCtl;

  uint8_t status = bufPayload->buffer[NDEF_WLC_LISTEN_CTL_STATUS_INFO_OFFSET];
  NDef_Data->statusInfoErrorFlag     = (status >> NDEF_WLC_LISTEN_CTL_STATUS_INFO_ERROR_SHIFT)   & NDEF_WLC_LISTEN_CTL_STATUS_INFO_ERROR_MASK;
  NDef_Data->statusInfoBatteryStatus = (status >> NDEF_WLC_LISTEN_CTL_STATUS_INFO_BATTERY_SHIFT) & NDEF_WLC_LISTEN_CTL_STATUS_INFO_BATTERY_MASK;
  NDef_Data->statusInfoCnt           = (status >> NDEF_WLC_LISTEN_CTL_STATUS_INFO_COUNTER_SHIFT) & NDEF_WLC_LISTEN_CTL_STATUS_INFO_COUNTER_MASK;

  uint8_t config = bufPayload->buffer[NDEF_WLC_LISTEN_CTL_WPT_CONFIG_OFFSET];
  NDef_Data->wptConfigWptReq         = (config >> NDEF_WLC_LISTEN_CTL_WPT_CONFIG_REQ_SHIFT)      & NDEF_WLC_LISTEN_CTL_WPT_CONFIG_REQ_MASK;
  NDef_Data->wptConfigWptDuration    = (config >> NDEF_WLC_LISTEN_CTL_WPT_CONFIG_DURATION_SHIFT) & NDEF_WLC_LISTEN_CTL_WPT_CONFIG_DURATION_MASK;
  NDef_Data->wptConfigInfoReq        = (config >> NDEF_WLC_LISTEN_CTL_WPT_CONFIG_INFO_REQ_SHIFT) & NDEF_WLC_LISTEN_CTL_WPT_CONFIG_INFO_REQ_MASK;

  NDef_Data->powerAdjReq             = bufPayload->buffer[NDEF_WLC_LISTEN_CTL_POWER_ADJ_REQ_OFFSET];
  NDef_Data->batteryLevel            = bufPayload->buffer[NDEF_WLC_LISTEN_CTL_BATTERY_LEVEL_OFFSET];

  uint8_t drvInfo = bufPayload->buffer[NDEF_WLC_LISTEN_CTL_DRV_INFO_OFFSET];
  NDef_Data->drvInfoFlag             = (drvInfo >> NDEF_WLC_LISTEN_CTL_DRV_INFO_FLAG_SHIFT) & NDEF_WLC_LISTEN_CTL_DRV_INFO_FLAG_MASK;
  NDef_Data->drvInfoInt              = (drvInfo >> NDEF_WLC_LISTEN_CTL_DRV_INFO_INT_SHIFT)  & NDEF_WLC_LISTEN_CTL_DRV_INFO_INT_MASK;

  NDef_Data->holdOffWtInt            = bufPayload->buffer[NDEF_WLC_LISTEN_CTL_HOLD_OFF_WT_INT_OFFSET];

  uint8_t error = 0;
  if (bufPayload->length == (NDEF_TYPE_RTD_WLC_PAYLOAD_LENGTH + 1U)) {
    error = bufPayload->buffer[NDEF_WLC_LISTEN_CTL_ERROR_INFO_OFFSET];
  }
  NDef_Data->errorInfoError          = (error >> NDEF_WLC_LISTEN_CTL_ERROR_INFO_PROTOCOL_SHIFT)    & NDEF_WLC_LISTEN_CTL_ERROR_INFO_PROTOCOL_MASK;
  NDef_Data->errorInfoTemperature    = (error >> NDEF_WLC_LISTEN_CTL_ERROR_INFO_TEMPERATURE_SHIFT) & NDEF_WLC_LISTEN_CTL_ERROR_INFO_TEMPERATURE_MASK;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdWlcListenCtl(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *NDef_Data;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  /* NDEF TNF and String type */
  if (! NDef_Record_TypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcListenCtl)) {
    return NFC_ProtocolError;
  }

  NDef_Data = NDef_RecordGetNDefType(record);
  if ((NDef_Data != NULL) && (NDef_Data->id == NDEF_TYPE_ID_RTD_WLCCTL)) {
    (void)memcpy(type, NDef_Data, sizeof(NDef_Type));
    return NFC_OK;
  }

  return NDef_PayloadToRtdWlcListenCtl(&record->bufPayload, type);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdWlcListenCtlToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type   == NULL) || ((type)->id != NDEF_TYPE_ID_RTD_WLCCTL) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  /* String type */
  (void)NDef_Record_SetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcListenCtl);

  if (NDef_RecordSetNDefType(record, type) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
