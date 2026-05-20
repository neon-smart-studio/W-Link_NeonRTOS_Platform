
/**
  ******************************************************************************
  * @file           : NDef__type_tnep.h
  * @brief          : NDEF TNEP (Tag NDEF Exchange Protocol record) types header file
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

#ifndef NDEF_TYPES_TNEP_H
#define NDEF_TYPES_TNEP_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

/*! RTD TNEP defines */
#define TNEP_VERSION_V1_0                       0x10U   /*!< TNEP version                              */
#define TNEP_COMMUNICATION_MODE_SINGLE_RESPONSE 0x00U   /*!< Single Response Communication mode        */
#define TNEP_COMMUNICATION_MODE_SPECIFIC        0xFEU   /*!< Specific Communication mode               */
#define TNEP_STATUS_TYPE_SUCCESS                0U      /*!< Status type Success                       */
#define TNEP_STATUS_TYPE_PROTOCOL_ERROR         1U      /*!< Status type Protocol Error                */

/*! RTD TNEP Service Parameter */
typedef struct {
  NDef_Const_Buffer bufServiceNameUri;            /*!< Service Name URI string buffer            */
  uint8_t         tnepVersion;                  /*!< TNEP version                              */
  uint8_t         communicationMode;            /*!< TNEP communication mode                   */
  uint8_t         minimumWaitingTime;           /*!< Minimum Waiting Time WT_INT               */
  uint8_t         maximumWaitingTimeExtensions; /*!< Maximum number of waiting time extensions */
  uint8_t         maximumNdefMessageSize[2];    /*!< Maximum NDEF message size (Big Endian)    */
} NDef_Type_Rtd_TnepServiceParameter;


/*! RTD TNEP Service Select */
typedef struct {
  NDef_Const_Buffer bufServiceNameUri;            /*!< Service Name URI string buffer         */
} NDef_Type_Rtd_TnepServiceSelect;


/*! RTD TNEP Status */
typedef struct {
  uint8_t         statusType;                   /*!< Status type */
} NDef_Type_Rtd_TnepStatus;


/*! RTD TNEP Record Type buffers */
extern const NDef_Const_Buffer_8 bufRtdTypeTnepServiceParameter; /*! TNEP Service Parameter buffer  */
extern const NDef_Const_Buffer_8 bufRtdTypeTnepServiceSelect;    /*! TNEP Service Select buffer     */
extern const NDef_Const_Buffer_8 bufRtdTypeTnepStatus;           /*! TNEP Status buffer             */

uint8_t NDef_RtdTnepServiceParameterComputeWtInt(float twait);
float NDef_RtdTnepServiceParameterComputeTwait(uint8_t wtInt);
NFC_OpResult NDef_RtdTnepServiceParameterInit(NDef_Type *type, uint8_t tnepVersion, const NDef_Const_Buffer *bufServiceUri, uint8_t comMode, uint8_t minWaitingTime, uint8_t maxExtensions, uint16_t maxMessageSize);
NFC_OpResult NDef_GetRtdTnepServiceParameter(const NDef_Type *type, uint8_t *tnepVersion, NDef_Const_Buffer *bufServiceUri, uint8_t *comMode, uint8_t *minWaitingTime, uint8_t *maxExtensions, uint16_t *maxMessageSize);
NFC_OpResult NDef_RecordToRtdTnepServiceParameter(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_RtdTnepServiceParameterToRecord(const NDef_Type *type, NDef_Record *record);
NFC_OpResult NDef_RtdTnepServiceSelectInit(NDef_Type *type, const NDef_Const_Buffer *bufServiceUri);
NFC_OpResult NDef_GetRtdTnepServiceSelect(const NDef_Type *type, NDef_Const_Buffer *bufServiceUri);
NFC_OpResult NDef_RecordToRtdTnepServiceSelect(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_RtdTnepServiceSelectToRecord(const NDef_Type *type, NDef_Record *record);
NFC_OpResult NDef_RtdTnepStatusInit(NDef_Type *type, uint8_t statusType);
NFC_OpResult NDef_GetRtdTnepStatus(const NDef_Type *type, uint8_t *statusType);
NFC_OpResult NDef_RecordToRtdTnepStatus(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_RtdTnepStatusToRecord(const NDef_Type *type, NDef_Record *record);

#endif /* NDEF_TYPES_TNEP_H */
