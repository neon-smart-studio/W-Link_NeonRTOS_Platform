
/**
  ******************************************************************************
  * @file           : ndef_type_aar.h
  * @brief          : NDEF RTD Android Application Record (AAR) type header file
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

#ifndef NDEF_TYPE_AAR_H
#define NDEF_TYPE_AAR_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

/*! RTD Record Type buffers */
extern const NDef_Const_Buffer_8 bufRtdTypeAar;        /*! AAR (Android Application Record) Record Type buffer */

/*! RTD Android Application Record External Type */
typedef struct {
  NDef_Const_Buffer_8 bufType;    /*!< AAR type    */
  NDef_Const_Buffer  bufPayload; /*!< AAR payload */
} NDef_Type_Rtd_AAR;

NFC_OpResult ndefRtdAarInit(NDef_Type *aar, const NDef_Const_Buffer *bufPayload);
NFC_OpResult ndefGetRtdAar(const NDef_Type *aar, NDef_Const_Buffer *bufAarString);
NFC_OpResult NDefRecordToRtdAar(const NDef_Record *record, NDef_Type *aar);
NFC_OpResult ndefRtdAarToRecord(const NDef_Type *aar, NDef_Record *record);

#endif /* NDEF_TYPE_AAR_H */
