
/**
  ******************************************************************************
  * @file           : ndef_types.cpp
  * @brief          :NDEF RTD and MIME types
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

#include "NFC/NFC_Def.h"

/*! NDEF type table to associate a TNF, type and the recordToType function pointers */
typedef struct {
  uint8_t                 tnf;           /*!< TNF                */
  const NDef_Const_Buffer_8 *bufTypeString; /*!< Type String buffer */
  NFC_OpResult(*recordToType)(const NDef_Record *record, NDef_Type *type);  /*!< Pointer to read function  */
} NDef_TypeConverter;

/*****************************************************************************/
NFC_OpResult NDef_RecordToType(const NDef_Record *record, NDef_Type *type)
{
#if NDEF_TYPE_EMPTY_SUPPORT
  /*! Empty string */
  static const uint8_t    NDef_TypeEmpty[] = "";    /*!< Empty string */
  static NDef_Const_Buffer_8 bufTypeEmpty    = { NDef_TypeEmpty, sizeof(NDef_TypeEmpty) - 1U };
#endif

  /*! Array to match RTD strings with Well-known types, and converting functions */
  static const NDef_TypeConverter typeConverterTable[] = {
#if NDEF_TYPE_EMPTY_SUPPORT
    { NDEF_TNF_EMPTY,               &bufTypeEmpty,            NDef_Record_ToEmptyType        },
#endif
#if NDEF_TYPE_RTD_DEVICE_INFO_SUPPORT
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeDeviceInfo,    NDef_Record_ToRtdDeviceInfo    },
#endif
#if NDEF_TYPE_RTD_TEXT_SUPPORT
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeText,          NDef_Record_ToRtdText          },
#endif
#if NDEF_TYPE_RTD_URI_SUPPORT
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeUri,           NDef_Record_ToRtdUri           },
#endif
#if NDEF_TYPE_RTD_AAR_SUPPORT
    { NDEF_TNF_RTD_EXTERNAL_TYPE,   &bufRtdTypeAar,           NDef_Record_ToRtdAar           },
#endif
#if NDEF_TYPE_RTD_WLC_SUPPORT
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcCapability, NDef_Record_ToRtdWlcCapability },
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcStatusInfo, NDef_Record_ToRtdWlcStatusInfo },
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcPollInfo,   NDef_Record_ToRtdWlcPollInfo   },
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufTypeRtdWlcListenCtl,  NDef_Record_ToRtdWlcListenCtl  },
#endif
#if NDEF_TYPE_RTD_WPCWLC_SUPPORT
    { NDEF_TNF_RTD_EXTERNAL_TYPE,   &bufRtdTypeWpcWlc,        NDef_Record_ToRtdWpcWlc        },
#endif
#if NDEF_TYPE_RTD_TNEP_SUPPORT
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeTnepServiceParameter, NDef_Record_ToRtdTnepServiceParameter },
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeTnepServiceSelect,    NDef_Record_ToRtdTnepServiceSelect    },
    { NDEF_TNF_RTD_WELL_KNOWN_TYPE, &bufRtdTypeTnepStatus,           NDef_Record_ToRtdTnepStatus           },
#endif
#if NDEF_TYPE_BLUETOOTH_SUPPORT
    { NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothBrEdr,       NDef_Record_ToBluetooth        },
    { NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothLe,          NDef_Record_ToBluetooth        },
    { NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothSecureBrEdr, NDef_Record_ToBluetooth        },
    { NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothSecureLe,    NDef_Record_ToBluetooth        },
#endif
#if NDEF_TYPE_VCARD_SUPPORT
    { NDEF_TNF_MEDIA_TYPE,          &bufMediaTypeVCard,       NDef_Record_ToVCard            },
#endif
#if NDEF_TYPE_WIFI_SUPPORT
    { NDEF_TNF_MEDIA_TYPE,          &bufMediaTypeWifi,        NDef_Record_ToWifi             },
#endif
    /* Non-conditional field to avoid empty union when all types are disabled */
    { 0,                            NULL,                     NULL                         }
  };

  const NDef_Type *ndefData;

  if (type == NULL) {
    return NFC_InvalidParameter;
  }

  ndefData = NDef_RecordGetNDefType(record);
  if (ndefData != NULL) {
    /* Return the well-known type contained in the record */
    (void)memcpy(type, ndefData, sizeof(NDef_Type));
    return NFC_OK;
  }

  for (int32_t i = 0; i < (int32_t)(sizeof(typeConverterTable)/sizeof(typeConverterTable[0])); i++) {
    if (NDef_Record_TypeMatch(record, typeConverterTable[i].tnf, typeConverterTable[i].bufTypeString)) {
      /* Call the appropriate function to the matching type */
      if (typeConverterTable[i].recordToType != NULL) {
        return typeConverterTable[i].recordToType(record, type);
      }
    }
  }

#if NDEF_TYPE_FLAT_SUPPORT
  return NDef_Record_ToFlatPayloadType(record, type);
#else
  return NFC_Unsupport;
#endif
}


/*****************************************************************************/
NFC_OpResult NDef_TypeToRecord(const NDef_Type *type, NDef_Record *record)
{
  if (type == NULL) {
    return NFC_InvalidParameter;
  }

  if (type->typeToRecord != NULL) {
    return type->typeToRecord(type, record);
  }

  return NFC_Unsupport;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordSetNDefType(NDef_Record *record, const NDef_Type *type)
{
  uint32_t payloadLength;

  if ((record == NULL) ||
      (type                   == NULL)               ||
      (type->id               == NDEF_TYPE_ID_NONE)  ||
      (type->id                > NDEF_TYPE_ID_COUNT) ||
      (type->getPayloadLength == NULL)               ||
      (type->getPayloadItem   == NULL)               ||
      (type->typeToRecord     == NULL)) {
    return NFC_InvalidParameter;
  }

  record->ndeftype = type;

  /* Set Short Record bit accordingly */
  payloadLength = NDef_RecordGetPayloadLength(record);
  NDef_Header_SetValueSR(record, (payloadLength <= NDEF_SHORT_RECORD_LENGTH_MAX) ? 1 : 0);

  return NFC_OK;
}


/*****************************************************************************/
const NDef_Type *NDef_RecordGetNDefType(const NDef_Record *record)
{
  if (record == NULL) {
    return NULL;
  }

  /* Check whether it is a valid NDEF type */
  if ((record->ndeftype != NULL) &&
      (record->ndeftype->id               != NDEF_TYPE_ID_NONE)  &&
      (record->ndeftype->id                < NDEF_TYPE_ID_COUNT) &&
      (record->ndeftype->getPayloadItem   != NULL)               &&
      (record->ndeftype->getPayloadLength != NULL)               &&
      (record->ndeftype->typeToRecord     != NULL)) {
    return record->ndeftype;
  }

  return NULL;
}
