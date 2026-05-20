
/**
  ******************************************************************************
  * @file           : ndef_type_wpcwlc.h
  * @brief          : NDEF RTD Wireless Power Consortium WLC Record (WPCWLC) type header file
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

#ifndef NDEF_TYPE_WPCWLC_H
#define NDEF_TYPE_WPCWLC_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define NDEF_KI_APPLICATION_PROFILE           03U   /*!< Ki Application Profile */

#define NDEF_KI_V10_PAYLOAD_LENGTH            16U   /*!< Ki v1.0 Payload length */

#define NDEF_KI_APPLICATION_PROFILE_OFFSET  0x00U   /*!< Ki Application Profile offset             */
#define NDEF_KI_VERSION_OFFSET              0x01U   /*!< Ki Major|Minor Version Offset             */
#define NDEF_KI_ALIVE_FDT_OFFSET            0x02U   /*!< Ki Alive FDT Offset                       */
#define NDEF_KI_READ_ADDRESS_OFFSET         0x03U   /*!< Ki Read Data Buffer Start Address Offset  */
#define NDEF_KI_WRITE_ADDRESS_OFFSET        0x04U   /*!< Ki Write Data Buffer Start Address Offset */
#define NDEF_KI_READ_SIZE_OFFSET            0x05U   /*!< Ki Read Data Buffer Size Offset           */
#define NDEF_KI_WRITE_SIZE_OFFSET           0x06U   /*!< Ki Write Data Buffer Size Offset          */
#define NDEF_KI_READ_CMD_OFFSET             0x07U   /*!< Ki Read Command Code Offset               */
#define NDEF_KI_WRITE_CMD_OFFSET            0x08U   /*!< Ki Write Commande Code Offset             */
#define NDEF_KI_MAX_T_SLOT_FOD_OFFSET       0x09U   /*!< Ki Maximum T_SLOT FOD Offset              */
#define NDEF_KI_MIN_T_POWER_OFFSET          0x0AU   /*!< Ki Minimum T_POWER Offset                 */
#define NDEF_KI_T_SUSPEND_OFFSET            0x0BU   /*!< Ki T_SUSPEND Offset                       */
#define NDEF_KI_COMM_LAG_MAX_OFFSET         0x0CU   /*!< Ki T_COMM_LAG,MAX Offset                  */
#define NDEF_KI_WRITE_SEQ_LENGTH_OFFSET     0x0DU   /*!< Ki Write Sequence Length Offset           */
#define NDEF_KI_MIN_POWER_OFFSET            0x0EU   /*!< Ki Minimum Power Offset                   */
#define NDEF_KI_MAX_POWER_OFFSET            0x0FU   /*!< Ki Maximum Power Offset                   */

/*! RTD Record Type buffers */
extern const NDef_Const_Buffer_8 bufRtdTypeWpcWlc;        /*! WPCWLC (Wireless Power Consortium WLC) Record Type buffer */


/*! RTD Wireless Power Consortium WLC Record External Type */
typedef struct {
  NDef_Const_Buffer  bufPayload; /*!< WPCWLC payload */
} NDef_Type_Rtd_WpcWlc;

NFC_OpResult NDef_RtdWpcWlcInit(NDef_Type *wpcWlc, const NDef_Const_Buffer *bufPayload);
NFC_OpResult NDef_GetRtdWpcWlc(const NDef_Type *wpcWlc, NDef_Const_Buffer *bufWpcWlc);
NFC_OpResult NDef_RecordToRtdWpcWlc(const NDef_Record *record, NDef_Type *wpcWlc);
NFC_OpResult NDef_RtdWpcWlcToRecord(const NDef_Type *wpcWlc, NDef_Record *record);

#endif /* NDEF_TYPE_WPCWLC_H */
