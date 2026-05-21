
/**
  ******************************************************************************
  * @file           : NDef__type_uri.cpp
  * @brief          : NDEF TNEP (Tag NDEF Exchange Protocol record) types
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
#include "NDef_Type_Tnep.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_RTD_TNEP_SUPPORT

/*! TNEP defines */
#define NDEF_TNEP_SERVICE_NAME_URI_LENGTH_LENGTH            sizeof(uint8_t) /*!< Service Name URI length's length */

/*! TNEP Service Parameter defines */
#define NDEF_RTD_TNEP_SP_VERSION_OFFSET                     0U    /*!< TNEP Service Parameter TNEP Version offset                      */
#define NDEF_RTD_TNEP_SP_SERVICE_URI_LENGTH_OFFSET          1U    /*!< TNEP Service Parameter URI length offset                        */
#define NDEF_RTD_TNEP_SP_SERVICE_URI_OFFSET                 2U    /*!< TNEP Service Parameter URI offset                               */
#define NDEF_RTD_TNEP_SP_SERVICE_URI_LENGTH_MAX             255U  /*!< TNEP Service Parameter URI max length (bytes)                   */
#define NDEF_RTD_TNEP_SP_COMMUNICATION_MODE_OFFSET          2U    /*!< TNEP Service Parameter Communication Mode offset                */
#define NDEF_RTD_TNEP_SP_MIN_WAITING_TIME_OFFSET            3U    /*!< TNEP Service Parameter Minimum Waiting Time offset              */
#define NDEF_RTD_TNEP_SP_MAX_WT_EXTENSIONS_OFFSET           4U    /*!< TNEP Service Parameter Maximum Waiting Time Extensions offset   */
#define NDEF_RTD_TNEP_SP_MAX_MESSAGE_SIZE_OFFSET            5U    /*!< TNEP Service Parameter Maximum NDEF Message Size (bytes) offset */
#define NDEF_RTD_TNEP_SP_MINIMUM_LENGTH                     8U    /*!< TNEP Service Parameter minimum length (bytes)                   */

/*! TNEP Service Select defines */
#define NDEF_RTD_TNEP_SS_SERVICE_URI_LENGTH_OFFSET          0U    /*!< TNEP Service Select URI length offset                       */
#define NDEF_RTD_TNEP_SS_SERVICE_URI_OFFSET                 1U    /*!< TNEP Service Select URI offset                              */
#define NDEF_RTD_TNEP_SS_MINIMUM_LENGTH                     2U    /*!< TNEP Service Select minimum length (bytes)                  */

/*! TNEP Service Select defines */
#define NDEF_RTD_TNEP_STATUS_TYPE_OFFSET                    0U    /*!< TNEP Status Type offset                       */
#define NDEF_RTD_TNEP_STATUS_MINIMUM_LENGTH                 1U    /*!< TNEP Status minimum length (bytes)            */

/*! RTD TNEP Type strings */
static const uint8_t NDef_RtdTypeTnepServiceParameter[] = "Tp";              /*!< Tnep Service Parameter Record Type          */
static const uint8_t NDef_RtdTypeTnepServiceSelect[]    = "Ts";              /*!< Tnep Service Select Record Type             */
static const uint8_t NDef_RtdTypeTnepStatus[]           = "Te";              /*!< Tnep Status Record Type                     */

const NDef_Const_Buffer_8 bufRtdTypeTnepServiceParameter = { NDef_RtdTypeTnepServiceParameter, sizeof(NDef_RtdTypeTnepServiceParameter) - 1U }; /*!< TNEP Service Parameter Type Record buffer       */
const NDef_Const_Buffer_8 bufRtdTypeTnepServiceSelect    = { NDef_RtdTypeTnepServiceSelect,    sizeof(NDef_RtdTypeTnepServiceSelect) - 1U };    /*!< TNEP Service Select Type Record buffer       */
const NDef_Const_Buffer_8 bufRtdTypeTnepStatus           = { NDef_RtdTypeTnepStatus,           sizeof(NDef_RtdTypeTnepStatus) - 1U };           /*!< TNEP Status Type Record buffer       */

/*****************************************************************************/
static uint32_t NDef_RtdTnepServiceParameterGetPayloadLength(const NDef_Type *type)
{
  const NDef_Type_Rtd_TnepServiceParameter *rtdServiceParameter;

  if ((type == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_SERVICE_PARAMETER)) {
    return 0;
  }

  rtdServiceParameter = &type->data.tnepServiceParameter;

  return sizeof(rtdServiceParameter->tnepVersion)
         + NDEF_TNEP_SERVICE_NAME_URI_LENGTH_LENGTH
         + rtdServiceParameter->bufServiceNameUri.length
         + sizeof(rtdServiceParameter->communicationMode)
         + sizeof(rtdServiceParameter->minimumWaitingTime)
         + sizeof(rtdServiceParameter->maximumWaitingTimeExtensions)
         + sizeof(rtdServiceParameter->maximumNdefMessageSize);
}


/*****************************************************************************/
static const uint8_t *NDef_RtdTnepServiceParameterToPayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Rtd_TnepServiceParameter *rtdServiceParameter;

  if ((type    == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_SERVICE_PARAMETER)
      || (bufItem == NULL)) {
    return NULL;
  }

  rtdServiceParameter = &type->data.tnepServiceParameter;

  if (begin == true) {
    item = 0;
  }

  switch (item) {
    case 0:
      /* TNEP version byte */
      bufItem->buffer = &rtdServiceParameter->tnepVersion;
      bufItem->length = sizeof(rtdServiceParameter->tnepVersion);
      break;

    case 1:
      /* Service URI length byte */
      bufItem->buffer = (const uint8_t *) & (rtdServiceParameter->bufServiceNameUri.length);
      bufItem->length = NDEF_TNEP_SERVICE_NAME_URI_LENGTH_LENGTH;
      break;

    case 2:
      /* Service URI string */
      bufItem->buffer = rtdServiceParameter->bufServiceNameUri.buffer;
      bufItem->length = rtdServiceParameter->bufServiceNameUri.length;
      break;

    case 3:
      /* TNEP communication Mode byte */
      bufItem->buffer = &rtdServiceParameter->communicationMode;
      bufItem->length = sizeof(rtdServiceParameter->communicationMode);
      break;

    case 4:
      /* Minimum waiting time byte */
      bufItem->buffer = &rtdServiceParameter->minimumWaitingTime;
      bufItem->length = sizeof(rtdServiceParameter->minimumWaitingTime);
      break;

    case 5:
      /* Maximum waiting time extensions byte */
      bufItem->buffer = &rtdServiceParameter->maximumWaitingTimeExtensions;
      bufItem->length = sizeof(rtdServiceParameter->maximumWaitingTimeExtensions);
      break;

    case 6:
      /* Maximum NDEF message size */
      bufItem->buffer = rtdServiceParameter->maximumNdefMessageSize;
      bufItem->length = sizeof(rtdServiceParameter->maximumNdefMessageSize);
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
uint8_t NDef_RtdTnepServiceParameterComputeWtInt(float twait)
{
  return ceil(4 * ((log(twait) / 0.69314) + 1));
}

/*****************************************************************************/
float NDef_RtdTnepServiceParameterComputeTwait(uint8_t wtInt)
{
  return powf(2, (((float)wtInt / 4) - 1));
}

/*****************************************************************************/
NFC_OpResult NDef_RtdTnepServiceParameterInit(NDef_Type *type, uint8_t tnepVersion, const NDef_Const_Buffer *bufServiceUri, uint8_t comMode, uint8_t minWaitingTime, uint8_t maxExtensions, uint16_t maxMessageSize)
{
  NDef_Type_Rtd_TnepServiceParameter *rtdServiceParameter;

  if ((type == NULL)    || (bufServiceUri == NULL)
      || (bufServiceUri->buffer == NULL) || (bufServiceUri->length == 0U)
      || (bufServiceUri->length > (NDEF_RTD_TNEP_SP_SERVICE_URI_LENGTH_MAX))) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_TNEP_SERVICE_PARAMETER;
  type->getPayloadLength = NDef_RtdTnepServiceParameterGetPayloadLength;
  type->getPayloadItem   = NDef_RtdTnepServiceParameterToPayloadItem;
  type->typeToRecord     = NDef_RtdTnepServiceParameterToRecord;
  rtdServiceParameter    = &type->data.tnepServiceParameter;

  rtdServiceParameter->tnepVersion                  = tnepVersion;
  rtdServiceParameter->bufServiceNameUri.length     = bufServiceUri->length;
  rtdServiceParameter->bufServiceNameUri.buffer     = bufServiceUri->buffer;
  rtdServiceParameter->communicationMode            = comMode;
  rtdServiceParameter->minimumWaitingTime           = minWaitingTime;
  rtdServiceParameter->maximumWaitingTimeExtensions = maxExtensions;
  rtdServiceParameter->maximumNdefMessageSize[0]    = (uint8_t)(maxMessageSize >> 8U);
  rtdServiceParameter->maximumNdefMessageSize[1]    = (uint8_t)(maxMessageSize & 0xFFU);

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdTnepServiceParameter(const NDef_Type *type, uint8_t *tnepVersion, NDef_Const_Buffer *bufServiceUri, uint8_t *comMode, uint8_t *minWaitingTime, uint8_t *maxExtensions, uint16_t *maxMessageSize)
{
  const NDef_Type_Rtd_TnepServiceParameter *rtdServiceParameter;

  if ((type == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_SERVICE_PARAMETER)
      || (bufServiceUri  == NULL) || (tnepVersion   == NULL) || (comMode        == NULL)
      || (minWaitingTime == NULL) || (maxExtensions == NULL) || (maxMessageSize == NULL)) {
    return NFC_InvalidParameter;
  }

  rtdServiceParameter = &type->data.tnepServiceParameter;

  *tnepVersion          = rtdServiceParameter->tnepVersion;
  bufServiceUri->buffer = rtdServiceParameter->bufServiceNameUri.buffer;
  bufServiceUri->length = rtdServiceParameter->bufServiceNameUri.length;
  *comMode              = rtdServiceParameter->communicationMode;
  *minWaitingTime       = rtdServiceParameter->minimumWaitingTime;
  *maxExtensions        = rtdServiceParameter->maximumWaitingTimeExtensions;
  *maxMessageSize       = (((uint16_t)(rtdServiceParameter->maximumNdefMessageSize)[0] << 8) | (uint16_t)(rtdServiceParameter->maximumNdefMessageSize)[1]);

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdTnepServiceParameter(const NDef_Const_Buffer *bufTnepServiceParameter, NDef_Type *type)
{
  uint8_t tnepVersion;
  NDef_Const_Buffer bufServiceUri;
  uint8_t commMode;
  uint8_t minWaitingTime;
  uint8_t maxWaitingTimeExtensions;
  uint16_t maxNdefMessageSize;
  uint32_t maxNdefMessageSizeOffset;

  if ((bufTnepServiceParameter == NULL) || (bufTnepServiceParameter->buffer == NULL) || (bufTnepServiceParameter->length < (NDEF_RTD_TNEP_SP_MINIMUM_LENGTH)) ||
      (type                    == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Extract info from the payload */
  tnepVersion              = bufTnepServiceParameter->buffer[NDEF_RTD_TNEP_SP_VERSION_OFFSET];
  bufServiceUri.buffer     = &bufTnepServiceParameter->buffer[NDEF_RTD_TNEP_SP_SERVICE_URI_OFFSET];
  bufServiceUri.length     = bufTnepServiceParameter->buffer[NDEF_RTD_TNEP_SP_SERVICE_URI_LENGTH_OFFSET];
  commMode                 = bufTnepServiceParameter->buffer[NDEF_RTD_TNEP_SP_COMMUNICATION_MODE_OFFSET + bufServiceUri.length];
  minWaitingTime           = bufTnepServiceParameter->buffer[NDEF_RTD_TNEP_SP_MIN_WAITING_TIME_OFFSET + bufServiceUri.length];
  maxWaitingTimeExtensions = bufTnepServiceParameter->buffer[NDEF_RTD_TNEP_SP_MAX_WT_EXTENSIONS_OFFSET + bufServiceUri.length];
  maxNdefMessageSizeOffset = NDEF_RTD_TNEP_SP_MAX_MESSAGE_SIZE_OFFSET + bufServiceUri.length;
  maxNdefMessageSize       = (((uint16_t)(&bufTnepServiceParameter->buffer[maxNdefMessageSizeOffset])[0] << 8) | (uint16_t)(&bufTnepServiceParameter->buffer[maxNdefMessageSizeOffset])[1]);

  return NDef_RtdTnepServiceParameterInit(type, tnepVersion, &bufServiceUri, commMode, minWaitingTime, maxWaitingTimeExtensions, maxNdefMessageSize);
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdTnepServiceParameter(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *NDef_Data;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_Record_TypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeTnepServiceParameter)) { /* "Tp" */
    return NFC_ProtocolError;
  }

  NDef_Data = NDef_RecordGetNDefType(record);
  if ((NDef_Data != NULL) && (NDef_Data->id == NDEF_TYPE_ID_RTD_TNEP_SERVICE_PARAMETER)) {
    (void)memcpy(type, NDef_Data, sizeof(NDef_Type));
    return NFC_OK;
  }

  if (record->bufPayload.length < NDEF_RTD_TNEP_SP_MINIMUM_LENGTH) {
    return NFC_ProtocolError;
  }

  return NDef_PayloadToRtdTnepServiceParameter(&record->bufPayload, type);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdTnepServiceParameterToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type   == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_SERVICE_PARAMETER) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  /* "Tp" */
  (void)NDef_Record_SetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeTnepServiceParameter);

  (void)NDef_RecordSetNDefType(record, type);

  return NFC_OK;
}

/*****************************************************************************/
static uint32_t NDef_RtdTnepServiceSelectGetPayloadLength(const NDef_Type *type)
{
  const NDef_Type_Rtd_TnepServiceSelect *rtdServiceSelect;

  if ((type == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_SERVICE_SELECT)) {
    return 0;
  }

  rtdServiceSelect = &type->data.tnepServiceSelect;

  return NDEF_TNEP_SERVICE_NAME_URI_LENGTH_LENGTH + rtdServiceSelect->bufServiceNameUri.length;
}


/*****************************************************************************/
static const uint8_t *NDef_RtdTnepServiceSelectToPayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Rtd_TnepServiceSelect *rtdServiceSelect;

  if ((type    == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_SERVICE_SELECT)
      || (bufItem == NULL)) {
    return NULL;
  }

  rtdServiceSelect = &type->data.tnepServiceSelect;

  if (begin == true) {
    item = 0;
  }

  switch (item) {
    case 0:
      /* Service URI length byte */
      bufItem->buffer = (const uint8_t *)&rtdServiceSelect->bufServiceNameUri.length;
      bufItem->length = NDEF_TNEP_SERVICE_NAME_URI_LENGTH_LENGTH;
      break;

    case 1:
      /* Service URI string */
      bufItem->buffer = rtdServiceSelect->bufServiceNameUri.buffer;
      bufItem->length = rtdServiceSelect->bufServiceNameUri.length;
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
NFC_OpResult NDef_RtdTnepServiceSelectInit(NDef_Type *type, const NDef_Const_Buffer *bufServiceUri)
{
  NDef_Type_Rtd_TnepServiceSelect *rtdServiceSelect;

  if ((type == NULL)       || (bufServiceUri == NULL)
      || (bufServiceUri->buffer == NULL) || (bufServiceUri->length == 0U)
      || (bufServiceUri->length > (NDEF_RTD_TNEP_SP_SERVICE_URI_LENGTH_MAX))) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_TNEP_SERVICE_SELECT;
  type->getPayloadLength = NDef_RtdTnepServiceSelectGetPayloadLength;
  type->getPayloadItem   = NDef_RtdTnepServiceSelectToPayloadItem;
  type->typeToRecord     = NDef_RtdTnepServiceSelectToRecord;
  rtdServiceSelect       = &type->data.tnepServiceSelect;

  rtdServiceSelect->bufServiceNameUri.length = bufServiceUri->length;
  rtdServiceSelect->bufServiceNameUri.buffer = bufServiceUri->buffer;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdTnepServiceSelect(const NDef_Type *type, NDef_Const_Buffer *bufServiceUri)
{
  const NDef_Type_Rtd_TnepServiceSelect *rtdServiceSelect;

  if ((type          == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_SERVICE_SELECT) ||
      (bufServiceUri == NULL)) {
    return NFC_InvalidParameter;
  }

  rtdServiceSelect = &type->data.tnepServiceSelect;

  bufServiceUri->buffer  = rtdServiceSelect->bufServiceNameUri.buffer;
  bufServiceUri->length  = rtdServiceSelect->bufServiceNameUri.length;

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdTnepServiceSelect(const NDef_Const_Buffer *bufTnepServiceSelect, NDef_Type *type)
{
  NDef_Const_Buffer bufServiceUri;

  if ((bufTnepServiceSelect == NULL) || (bufTnepServiceSelect->buffer == NULL) || (bufTnepServiceSelect->length < (NDEF_RTD_TNEP_SS_MINIMUM_LENGTH)) ||
      (type                 == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Extract info from the payload */
  bufServiceUri.buffer         = &bufTnepServiceSelect->buffer[NDEF_RTD_TNEP_SS_SERVICE_URI_OFFSET];
  bufServiceUri.length         = bufTnepServiceSelect->buffer[NDEF_RTD_TNEP_SS_SERVICE_URI_LENGTH_OFFSET];

  return NDef_RtdTnepServiceSelectInit(type, &bufServiceUri);
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdTnepServiceSelect(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *NDef_Data;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_Record_TypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeTnepServiceSelect)) { /* "Ts" */
    return NFC_ProtocolError;
  }

  NDef_Data = NDef_RecordGetNDefType(record);
  if ((NDef_Data != NULL) && (NDef_Data->id == NDEF_TYPE_ID_RTD_TNEP_SERVICE_SELECT)) {
    (void)memcpy(type, NDef_Data, sizeof(NDef_Type));
    return NFC_OK;
  }

  if (record->bufPayload.length < NDEF_RTD_TNEP_SS_MINIMUM_LENGTH) {
    return NFC_ProtocolError;
  }

  return NDef_PayloadToRtdTnepServiceSelect(&record->bufPayload, type);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdTnepServiceSelectToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type   == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_SERVICE_SELECT) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  /* "Ts" */
  (void)NDef_Record_SetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeTnepServiceSelect);

  (void)NDef_RecordSetNDefType(record, type);

  return NFC_OK;
}

/*****************************************************************************/
static uint32_t NDef_RtdTnepStatusGetPayloadLength(const NDef_Type *type)
{
  if ((type == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_STATUS)) {
    return 0;
  }

  return sizeof(type->data.tnepStatus.statusType);
}


/*****************************************************************************/
static const uint8_t *NDef_RtdTnepStatusToPayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Rtd_TnepStatus *rtdStatus;

  if ((type == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_STATUS)
      || (bufItem == NULL)) {
    return NULL;
  }

  rtdStatus = &type->data.tnepStatus;

  if (begin == true) {
    item = 0;
  }

  switch (item) {
    case 0:
      /* Status type byte */
      bufItem->buffer = &rtdStatus->statusType;
      bufItem->length = sizeof(rtdStatus->statusType);
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
NFC_OpResult NDef_RtdTnepStatusInit(NDef_Type *type, uint8_t statusType)
{
  NDef_Type_Rtd_TnepStatus *rtdStatus;

  if (type == NULL) {
    return NFC_InvalidParameter;
  }

  type->id               = NDEF_TYPE_ID_RTD_TNEP_STATUS;
  type->getPayloadLength = NDef_RtdTnepStatusGetPayloadLength;
  type->getPayloadItem   = NDef_RtdTnepStatusToPayloadItem;
  type->typeToRecord     = NDef_RtdTnepStatusToRecord;
  rtdStatus              = &type->data.tnepStatus;

  rtdStatus->statusType = statusType;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdTnepStatus(const NDef_Type *type, uint8_t *statusType)
{
  const NDef_Type_Rtd_TnepStatus *rtdStatus;

  if ((type       == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_STATUS) ||
      (statusType == NULL)) {
    return NFC_InvalidParameter;
  }

  rtdStatus = &type->data.tnepStatus;

  *statusType = rtdStatus->statusType;

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdTnepStatus(const NDef_Const_Buffer *bufTnepStatus, NDef_Type *type)
{
  uint8_t statusType;

  if ((bufTnepStatus == NULL) || (bufTnepStatus->buffer == NULL) || (bufTnepStatus->length < (NDEF_RTD_TNEP_STATUS_MINIMUM_LENGTH)) ||
      (type          == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Extract info from the payload */
  statusType = bufTnepStatus->buffer[NDEF_RTD_TNEP_STATUS_TYPE_OFFSET];

  return NDef_RtdTnepStatusInit(type, statusType);
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdTnepStatus(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *NDef_Data;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_Record_TypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeTnepStatus)) { /* "Te" */
    return NFC_ProtocolError;
  }

  NDef_Data = NDef_RecordGetNDefType(record);
  if ((NDef_Data != NULL) && (NDef_Data->id == NDEF_TYPE_ID_RTD_TNEP_STATUS)) {
    (void)memcpy(type, NDef_Data, sizeof(NDef_Type));
    return NFC_OK;
  }

  if (record->bufPayload.length < NDEF_RTD_TNEP_STATUS_MINIMUM_LENGTH) {
    return NFC_ProtocolError;
  }

  return NDef_PayloadToRtdTnepStatus(&record->bufPayload, type);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdTnepStatusToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type   == NULL) || (type->id != NDEF_TYPE_ID_RTD_TNEP_STATUS) ||
      (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_Record_Reset(record);

  /* "Te" */
  (void)NDef_Record_SetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeTnepStatus);

  (void)NDef_RecordSetNDefType(record, type);

  return NFC_OK;
}

#endif
