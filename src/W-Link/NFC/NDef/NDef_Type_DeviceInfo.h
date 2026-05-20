
/**
  ******************************************************************************
  * @file           : ndef_type_deviceinfo.h
  * @brief          : NDEF RTD Device Information type header file
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

#ifndef NDEF_TYPE_RTD_DEVICE_INFO_H
#define NDEF_TYPE_RTD_DEVICE_INFO_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

/*! Device Information defines */
#define NDEF_UUID_LENGTH                16U    /*!< Device Information UUID length */

/*! RTD Device Information Record Type buffer */
extern const NDef_Const_Buffer_8 bufRtdTypeDeviceInfo; /*! Device Information Record Type buffer               */


/*! RTD Device Information types */
#define NDEF_DEVICE_INFO_MANUFACTURER_NAME     0U /*!< Manufacturer name                      */
#define NDEF_DEVICE_INFO_MODEL_NAME            1U /*!< Model name                             */
#define NDEF_DEVICE_INFO_DEVICE_UNIQUE_NAME    2U /*!< Device Unique Name aka "Friendly Name" */
#define NDEF_DEVICE_INFO_UUID                  3U /*!< UUID                                   */
#define NDEF_DEVICE_INFO_FIRMWARE_VERSION      4U /*!< Firmware Version                       */
#define NDEF_DEVICE_INFO_TYPE_COUNT    5U /*!< Maximum Device Information types */


/*! RTD Device Information Entry */
typedef struct {
  uint8_t        type;      /*!< Device Information Type              */
  uint8_t        length;    /*!< Device Information length            */
  const uint8_t *buffer;    /*!< Device Information pointer to buffer */
} NDef_DeviceInfoEntry;


/*! RTD Type Device Information */
typedef struct {
  NDef_DeviceInfoEntry devInfo[NDEF_DEVICE_INFO_TYPE_COUNT]; /*!< Device Information entries */
} NDef_Type_Rtd_DeviceInfo;

NFC_OpResult NDef_RtdDeviceInfoInit(NDef_Type *devInfo, const NDef_DeviceInfoEntry *devInfoData, uint8_t devInfoDataCount);
NFC_OpResult NDef_GetRtdDeviceInfo(const NDef_Type *devInfo, NDef_Type_Rtd_DeviceInfo *devInfoData);
NFC_OpResult NDef_RecordToRtdDeviceInfo(const NDef_Record *record, NDef_Type *devInfo);
NFC_OpResult NDef_RtdDeviceInfoToRecord(const NDef_Type *devInfo, NDef_Record *record);

#endif /* NDEF_TYPE_RTD_DEVICE_INFO_H */

/**
  * @}
  *
  */
