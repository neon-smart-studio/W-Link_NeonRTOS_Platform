
/**
  ******************************************************************************
  * @file           : ndef_type_wlc.h
  * @brief          : NDEF WLC (Wireless Charging) types header file
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



#ifndef NDEF_TYPE_WLC_H
#define NDEF_TYPE_WLC_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

#define NDEF_WLC_STATUSINFO_CONTROLBYTE1_BATTERY_LEVEL_MASK        0x01U /*!< WLC Status and Info Control byte 1: Battery Level mask */
#define NDEF_WLC_STATUSINFO_CONTROLBYTE1_RECEIVE_POWER_MASK        0x02U /*!< WLC Status and Info Control byte 1: Receive Power mask */
#define NDEF_WLC_STATUSINFO_CONTROLBYTE1_RECEIVE_VOLTAGE_MASK      0x04U /*!< WLC Status and Info Control byte 1: Receive Voltage mask */
#define NDEF_WLC_STATUSINFO_CONTROLBYTE1_RECEIVE_CURRENT_MASK      0x08U /*!< WLC Status and Info Control byte 1: Receive Current mask */
#define NDEF_WLC_STATUSINFO_CONTROLBYTE1_TEMPERATURE_BATTERY_MASK  0x10U /*!< WLC Status and Info Control byte 1: Temperature Battery mask */
#define NDEF_WLC_STATUSINFO_CONTROLBYTE1_TEMPERATURE_WLCL_MASK     0x20U /*!< WLC Status and Info Control byte 1: Temperature WLCL mask */
#define NDEF_WLC_STATUSINFO_CONTROLBYTE1_RFU_MASK                  0x40U /*!< WLC Status and Info Control byte 1: RFU mask */
#define NDEF_WLC_STATUSINFO_CONTROLBYTE1_CONTROL_BYTE_2_MASK       0x80U /*!< WLC Status and Info Control byte 1: Control byte 2 mask */

/*! Structure to store WLC Capability */
typedef struct {
  uint8_t wlcProtocolVersion;     /*!< WLC Protocol Version */
  uint8_t wlcConfigModeReq;       /*!< WLC Config: MODE_REQ */
  uint8_t wlcConfigWaitTimeRetry; /*!< WLC Config: WaitTimeRetry */
  uint8_t wlcConfigNegoWait;      /*!< WLC Config: NEGO_WAIT */
  uint8_t wlcConfigRdConf;        /*!< WLC Config: RD_CONF */
  uint8_t capWtIntRfu;            /*!< Cap Wt Int RFU */
  uint8_t capWtInt;               /*!< Cap Wt Int */
  uint8_t ndefRdWt;               /*!< NDEF Rd Wt */
  uint8_t ndefWriteToInt;         /*!< NDEF Write To Int */
  uint8_t ndefWriteWtInt;         /*!< NDEF Write Wt Int */
} NDef_Type_Rtd_WlcCapability;


/*! Structure to store WLC Status and Info */
typedef struct {
  uint8_t controlByte1;       /*!< Control byte 1 */
  uint8_t batteryLevel;       /*!< Battery level */
  uint8_t receivePower;       /*!< Receive power */
  uint8_t receiveVoltage;     /*!< Receive voltage */
  uint8_t receiveCurrent;     /*!< Receive current */
  uint8_t temperatureBattery; /*!< Battery temperature */
  uint8_t temperatureWlcl;    /*!< WLCL temperature */
  uint8_t rfu;                /*!< RFU */
  uint8_t controlByte2;       /*!< Control byte 2 */
} NDef_Type_Rtd_WlcStatusInfo;


/*! Structure to store WLC Poll Information */
typedef struct {
  uint8_t pTx;               /*!< P Tx, Transmit Power Level */
  uint8_t wlcPCap;           /*!< WLC_P Capability */
  uint8_t powerClass;        /*!< Power Class */
  uint8_t totPowerSteps;     /*!< Tot Power Steps */
  uint8_t curPowerStep;      /*!< Current Power Step */
  uint8_t nextMinStepInc;    /*!< Next Min Step Inc */
  uint8_t nextMinStepDec;    /*!< Next Min Step Dec */
} NDef_Type_Rtd_WlcPollInfo;


/*! Structure to store WLC Listen Control */
typedef struct {
  uint8_t statusInfoErrorFlag;     /*!< Status information: ERROR_FLG */
  uint8_t statusInfoBatteryStatus; /*!< Status information: BATTERY_STATUS */
  uint8_t statusInfoCnt;           /*!< Status information: CNT */
  uint8_t wptConfigWptReq;         /*!< WPT Config: WPT_REQ */
  uint8_t wptConfigWptDuration;    /*!< WPT Config: WPT_DURATION */
  uint8_t wptConfigInfoReq;        /*!< WPT Config: INFO_REQ */
  uint8_t powerAdjReq;             /*!< Power Adjust Req */
  uint8_t batteryLevel;            /*!< Battery level */
  uint8_t drvInfoFlag;             /*!< Drv info: DRV_FLAG */
  uint8_t drvInfoInt;              /*!< Drv info: DRV_INT */
  uint8_t holdOffWtInt;            /*!< Hold off Wt Int */
  uint8_t errorInfoError;          /*!< [Error info]* if ERROR_FLG set: WLC_INFO_ERROR */
  uint8_t errorInfoTemperature;    /*!< [Error info]* if ERROR_FLG set: TEMPERATURE_ERROR */
} NDef_Type_Rtd_WlcListenCtl;


/*! WLC Record Type buffers */
extern const NDef_Const_Buffer_8 bufTypeRtdWlcCapability; /*!< WLC Capability Type Record buffer             */
extern const NDef_Const_Buffer_8 bufTypeRtdWlcStatusInfo; /*!< WLC Status and Information Type Record buffer */
extern const NDef_Const_Buffer_8 bufTypeRtdWlcPollInfo;   /*!< WLC Poll Information Type Record buffer       */
extern const NDef_Const_Buffer_8 bufTypeRtdWlcListenCtl;  /*!< WLC Listen Control Type Record buffer         */


/*! WLC MODE_REQ */
typedef enum {
  NDEF_RTD_WLC_CAPABILITY_MODE_STATIC,
  NDEF_RTD_WLC_CAPABILITY_MODE_NEGOTIATED,
  NDEF_RTD_WLC_CAPABILITY_MODE_BATTERY_FULL,
  NDEF_RTD_WLC_CAPABILITY_MODE_RFU
} NDef_RtdWlcReqMode;

NFC_OpResult NDef_RtdWlcCapabilityInit(NDef_Type *type, const NDef_Type_Rtd_WlcCapability *wlcCapability);
NFC_OpResult NDef_GetRtdWlcCapability(const NDef_Type *type, NDef_Type_Rtd_WlcCapability *wlcCapability);
NFC_OpResult NDef_RecordToRtdWlcCapability(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_RtdWlcCapabilityToRecord(const NDef_Type *type, NDef_Record *record);
NFC_OpResult NDef_RtdWlcStatusInfoInit(NDef_Type *type, const NDef_Type_Rtd_WlcStatusInfo *wlcStatusInfo);
NFC_OpResult NDef_GetRtdWlcStatusInfo(const NDef_Type *type, NDef_Type_Rtd_WlcStatusInfo *wlcStatusInfo);
NFC_OpResult NDef_RecordToRtdWlcStatusInfo(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_RtdWlcStatusInfoToRecord(const NDef_Type *type, NDef_Record *record);
NFC_OpResult NDef_RtdWlcPollInfoInit(NDef_Type *type, const NDef_Type_Rtd_WlcPollInfo *wlcPollInfo);
NFC_OpResult NDef_GetRtdWlcPollInfo(const NDef_Type *type, NDef_Type_Rtd_WlcPollInfo *wlcPollInfo);
NFC_OpResult NDef_RecordToRtdWlcPollInfo(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_RtdWlcPollInfoToRecord(const NDef_Type *type, NDef_Record *record);
NFC_OpResult NDef_RtdWlcListenCtlInit(NDef_Type *type, const NDef_Type_Rtd_WlcListenCtl *wlcListenCtl);
NFC_OpResult NDef_GetRtdWlcListenCtl(const NDef_Type *type, NDef_Type_Rtd_WlcListenCtl *wlcListenCtl);
NFC_OpResult NDef_RecordToRtdWlcListenCtl(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_RtdWlcListenCtlToRecord(const NDef_Type *type, NDef_Record *record);

#endif
