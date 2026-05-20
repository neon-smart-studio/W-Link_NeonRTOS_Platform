
/**
  ******************************************************************************
  * @file           : ndef_type_deviceinfo.cpp
  * @brief          : NDEF RTD Device Information type
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
#include "NDef_Type_DeviceInfo.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_RTD_DEVICE_INFO_SUPPORT

/*! Device Information payload defines */
#define NDEF_RTD_DEVICE_INFO_PAYLOAD_MIN     (2U * (sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t))) /*!< Device Information minimum length (2 required TLV structures) */
#define NDEF_RTD_DEVICE_INFO_PAYLOAD_MAX     ((4U * (sizeof(uint8_t) + sizeof(uint8_t) + 255U)) + (sizeof(uint8_t) + sizeof(uint8_t) + 16U)) /*!< Device Information maximum length */
#define NDEF_RTD_DEVICE_INFO_TLV_LENGTH_MIN  (sizeof(uint8_t) + sizeof(uint8_t))  /*!< Device Information minimum TLV length */

/*! RTD Device Information Type string */
static const uint8_t ndefRtdTypeDeviceInfo[]     = "Di";              /*!< Device Information Record Type {0x44, 0x69} */

const NDef_Const_Buffer_8 bufRtdTypeDeviceInfo      = { ndefRtdTypeDeviceInfo, sizeof(ndefRtdTypeDeviceInfo) - 1U }; /*!< Device Information Record Type buffer */

/*****************************************************************************/
static uint32_t NDef_RtdDeviceInfoPayloadGetLength(const NDef_Type *devInfo)
{
  const NDef_Type_Rtd_DeviceInfo *rtdDevInfo;
  uint32_t payloadLength = 0;

  if ((devInfo == NULL) || (devInfo->id != NDEF_TYPE_ID_RTD_DEVICE_INFO)) {
    return 0;
  }

  rtdDevInfo = &devInfo->data.deviceInfo;

  for (uint32_t i = 0; i < NDEF_DEVICE_INFO_TYPE_COUNT; i++) {
    if (rtdDevInfo->devInfo[i].length != 0U) {
      /* sizeof(type) + sizeof(length) + actual length */
      payloadLength += sizeof(rtdDevInfo->devInfo[i].type) + sizeof(rtdDevInfo->devInfo[i].length) + (uint32_t)rtdDevInfo->devInfo[i].length;
    }
  }

  return payloadLength;
}


/*****************************************************************************/
static const uint8_t *NDef_RtdDeviceInfoToPayloadItem(const NDef_Type *devInfo, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  const NDef_Type_Rtd_DeviceInfo *rtdDevInfo;
  uint32_t index;

  if ((devInfo == NULL) || (devInfo->id != NDEF_TYPE_ID_RTD_DEVICE_INFO) ||
      (bufItem == NULL)) {
    return NULL;
  }

  rtdDevInfo = &devInfo->data.deviceInfo;

  if (begin == true) {
    item = 0;
  }

  bufItem->buffer = NULL;
  bufItem->length = 0;

  index = item / 3U;

  /* Stop streaming on first empty entry */
  if (rtdDevInfo->devInfo[index].length > 0U) {
    switch (item % 3U) {
      case 0:
        bufItem->buffer = &rtdDevInfo->devInfo[index].type;
        bufItem->length = sizeof(rtdDevInfo->devInfo[index].type);
        break;
      case 1:
        bufItem->buffer = &rtdDevInfo->devInfo[index].length;
        bufItem->length = sizeof(rtdDevInfo->devInfo[index].length);
        break;
      case 2:
        bufItem->buffer = rtdDevInfo->devInfo[index].buffer;
        bufItem->length = rtdDevInfo->devInfo[index].length;
        break;
      default:
        bufItem->buffer = NULL;
        bufItem->length = 0;
        break;
    }
  }

  /* Move to next item for next call */
  item++;

  return bufItem->buffer;
}


/*****************************************************************************/
NFC_OpResult NDef_RtdDeviceInfoInit(NDef_Type *devInfo, const NDef_DeviceInfoEntry *devInfoData, uint8_t devInfoDataCount)
{
  NDef_Type_Rtd_DeviceInfo *rtdDevInfo;
  uint8_t  count;
  uint8_t  manufacturerNameIndex;
  uint8_t  modelNameIndex;

  if ((devInfo     == NULL)    ||
      (devInfoData == NULL)    || (devInfoData->length == 0U) ||
      (devInfoDataCount == 0U) || (devInfoDataCount > NDEF_DEVICE_INFO_TYPE_COUNT)) {
    return NFC_InvalidParameter;
  }

  devInfo->id               = NDEF_TYPE_ID_RTD_DEVICE_INFO;
  devInfo->getPayloadLength = NDef_RtdDeviceInfoPayloadGetLength;
  devInfo->getPayloadItem   = NDef_RtdDeviceInfoToPayloadItem;
  devInfo->typeToRecord     = NDef_RtdDeviceInfoToRecord;
  rtdDevInfo                = &devInfo->data.deviceInfo;

  /* Clear the Device Information structure before parsing */
  for (uint32_t i = 0; i < NDEF_DEVICE_INFO_TYPE_COUNT; i++) {
    rtdDevInfo->devInfo[i].type   = 0;
    rtdDevInfo->devInfo[i].length = 0;
    rtdDevInfo->devInfo[i].buffer = NULL;
  }

  /* Read Type, Length and Value fields */
  /* Not checking multiple occurrences of a given field, use the last one */
  count = 0;
  manufacturerNameIndex = 0;
  modelNameIndex = 0;

  while (count < devInfoDataCount) {
    uint8_t type   = devInfoData[count].type;
    uint8_t length = devInfoData[count].length;
    if ((type == NDEF_DEVICE_INFO_UUID) && (length != NDEF_UUID_LENGTH)) {
      return NFC_ProtocolError;
    }
    if ((type > NDEF_DEVICE_INFO_TYPE_COUNT) || (length == 0U)) {
      return NFC_ProtocolError;
    }
    if (type == NDEF_DEVICE_INFO_MANUFACTURER_NAME) {
      manufacturerNameIndex = count;
    } else {
      if (type == NDEF_DEVICE_INFO_MODEL_NAME) {
        modelNameIndex = count;
      }
    }

    rtdDevInfo->devInfo[count].type   = type;
    rtdDevInfo->devInfo[count].length = length;
    rtdDevInfo->devInfo[count].buffer = devInfoData[count].buffer;
    count++;
  }

  /* Check that both required fields are there */
  if ((manufacturerNameIndex != modelNameIndex) &&
      (rtdDevInfo->devInfo[manufacturerNameIndex].buffer != NULL) &&
      (rtdDevInfo->devInfo[modelNameIndex].buffer        != NULL)) {
    return NFC_OK;
  } else {
    return NFC_InvalidParameter;
  }
}


/*****************************************************************************/
NFC_OpResult NDef_GetRtdDeviceInfo(const NDef_Type *devInfo, NDef_Type_Rtd_DeviceInfo *devInfoData)
{
  const NDef_Type_Rtd_DeviceInfo *rtdDevInfo;

  if ((devInfo     == NULL) || (devInfo->id != NDEF_TYPE_ID_RTD_DEVICE_INFO) ||
      (devInfoData == NULL)) {
    return NFC_InvalidParameter;
  }

  rtdDevInfo = &devInfo->data.deviceInfo;

  for (uint32_t i = 0; i < NDEF_DEVICE_INFO_TYPE_COUNT; i++) {
    devInfoData->devInfo[i].type   = rtdDevInfo->devInfo[i].type;
    devInfoData->devInfo[i].length = rtdDevInfo->devInfo[i].length;
    devInfoData->devInfo[i].buffer = rtdDevInfo->devInfo[i].buffer;
  }

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult NDef_PayloadToRtdDeviceInfo(const NDef_Const_Buffer *bufDevInfo, NDef_Type *devInfo)
{
  NDef_Type_Rtd_DeviceInfo *rtdDevInfo;
  uint32_t offset;
  uint8_t  count;
  uint8_t  manufacturerNameIndex;
  uint8_t  modelNameIndex;

  if ((bufDevInfo == NULL) || (bufDevInfo->buffer == NULL) ||
      (devInfo    == NULL)) {
    return NFC_InvalidParameter;
  }

  devInfo->id               = NDEF_TYPE_ID_RTD_DEVICE_INFO;
  devInfo->getPayloadLength = NDef_RtdDeviceInfoPayloadGetLength;
  devInfo->getPayloadItem   = NDef_RtdDeviceInfoToPayloadItem;
  devInfo->typeToRecord     = NDef_RtdDeviceInfoToRecord;
  rtdDevInfo                = &devInfo->data.deviceInfo;

  if ((bufDevInfo->length < NDEF_RTD_DEVICE_INFO_PAYLOAD_MIN) ||
      (bufDevInfo->length > NDEF_RTD_DEVICE_INFO_PAYLOAD_MAX)) {
    return NFC_ProtocolError;
  }

  /* Extract device information from the buffer */

  /* Clear the Device Information structure before parsing */
  for (uint32_t i = 0; i < NDEF_DEVICE_INFO_TYPE_COUNT; i++) {
    rtdDevInfo->devInfo[i].type   = 0;
    rtdDevInfo->devInfo[i].length = 0;
    rtdDevInfo->devInfo[i].buffer = NULL;
  }

  /* Read Type, Length and Value fields */
  /* Not checking multiple occurrences of a given field, use the last one */
  offset = 0;
  count = 0;
  manufacturerNameIndex = 0;
  modelNameIndex = 0;

  while (((offset + NDEF_RTD_DEVICE_INFO_TLV_LENGTH_MIN) < bufDevInfo->length)
         && (count < NDEF_DEVICE_INFO_TYPE_COUNT)) {
    uint8_t type   =  bufDevInfo->buffer[offset];
    uint8_t length =  bufDevInfo->buffer[offset + 1U];
    if ((type == NDEF_DEVICE_INFO_UUID) && (length != NDEF_UUID_LENGTH)) {
      return NFC_ProtocolError;
    }
    if ((type > NDEF_DEVICE_INFO_TYPE_COUNT) || (length == 0U)) {
      return NFC_ProtocolError;
    }
    if (type == NDEF_DEVICE_INFO_MANUFACTURER_NAME) {
      manufacturerNameIndex = count;
    } else {
      if (type == NDEF_DEVICE_INFO_MODEL_NAME) {
        modelNameIndex = count;
      }
    }

    rtdDevInfo->devInfo[count].type   = type;
    rtdDevInfo->devInfo[count].length = length;
    rtdDevInfo->devInfo[count].buffer = &bufDevInfo->buffer[offset + 2U];
    count++;

    /* Next entry */
    offset += sizeof(uint8_t) + sizeof(uint8_t) + (uint32_t)length;
  }

  /* Check both required fields are there */
  if ((manufacturerNameIndex != modelNameIndex) &&
      (rtdDevInfo->devInfo[manufacturerNameIndex].buffer != NULL) &&
      (rtdDevInfo->devInfo[modelNameIndex].buffer        != NULL)) {
    return NFC_OK;
  } else {
    return NFC_InvalidParameter;
  }
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToRtdDeviceInfo(const NDef_Record *record, NDef_Type *devInfo)
{
  const NDef_Type *type;

  if ((record == NULL) || (devInfo == NULL)) {
    return NFC_InvalidParameter;
  }

  if (! NDef_RecordTypeMatch(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeDeviceInfo)) { /* "Di" */
    return NFC_ProtocolError;
  }

  type = NDef_RecordGetNdefType(record);
  if ((type != NULL) && (type->id == NDEF_TYPE_ID_RTD_DEVICE_INFO)) {
    (void)memcpy(devInfo, type, sizeof(NDef_Type));
    return NFC_OK;
  }

  return ndefPayloadToRtdDeviceInfo(&record->bufPayload, devInfo);
}


/*****************************************************************************/
NFC_OpResult NDef_RtdDeviceInfoToRecord(const NDef_Type *devInfo, NDef_Record *record)
{
  if ((devInfo == NULL) || (devInfo->id != NDEF_TYPE_ID_RTD_DEVICE_INFO) ||
      (record  == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_RecordReset(record);

  /* "Di" */
  (void)NDef_RecordSetType(record, NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeDeviceInfo);

  if (NDef_RecordSetNdefType(record, devInfo) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
