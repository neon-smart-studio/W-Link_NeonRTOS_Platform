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

#ifndef RFAL_NFC_H
#define RFAL_NFC_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include <stdbool.h>
#include <stdint.h>

#include "RFal_RF.h"
#include "RFal_ISO_Dep.h"
#include "RFal_NFCA.h"
#include "RFal_NFCB.h"
#include "RFal_NFCF.h"
#include "RFal_NFCV.h"
#include "RFal_ST25TB.h"
#include "RFal_NFC_Dep.h"
#include "RFal_T4T.h"

#include "NFC/NFC_Def.h"

#define RFAL_FEATURE_NFC_DEP_BLOCK_MAX_LEN     254U       /*!< NFC-DEP Block/Payload length. Allowed values: 64, 128, 192, 254           */
#define RFAL_FEATURE_NFC_RF_BUF_LEN            258U       /*!< RF buffer length used by RFAL NFC layer                                   */

#define RFAL_FEATURE_NFC_DEP_PDU_MAX_LEN       512U       /*!< NFC-DEP PDU max length.                                                   */

#define RFAL_NFC_MAX_DEVICES          5U    /* Max number of devices supported */

#define RFAL_NFC_TECH_NONE               0x0000U  /*!< No technology                     */
#define RFAL_NFC_POLL_TECH_A             0x0001U  /*!< Poll NFC-A technology Flag        */
#define RFAL_NFC_POLL_TECH_B             0x0002U  /*!< Poll NFC-B technology Flag        */
#define RFAL_NFC_POLL_TECH_F             0x0004U  /*!< Poll NFC-F technology Flag        */
#define RFAL_NFC_POLL_TECH_V             0x0008U  /*!< Poll NFC-V technology Flag        */
#define RFAL_NFC_POLL_TECH_AP2P          0x0010U  /*!< Poll AP2P technology Flag         */
#define RFAL_NFC_POLL_TECH_ST25TB        0x0020U  /*!< Poll ST25TB technology Flag       */
#define RFAL_NFC_POLL_TECH_PROP          0x0040U  /*!< Poll Proprietary technology Flag  */
#define RFAL_NFC_LISTEN_TECH_A           0x1000U  /*!< Listen NFC-A technology Flag      */
#define RFAL_NFC_LISTEN_TECH_B           0x2000U  /*!< Listen NFC-B technology Flag      */
#define RFAL_NFC_LISTEN_TECH_F           0x4000U  /*!< Listen NFC-F technology Flag      */
#define RFAL_NFC_LISTEN_TECH_AP2P        0x8000U  /*!< Listen AP2P technology Flag       */

/*! Checks if a device is currently activated */
#define RFal_NFC_IsDevActivated( st )        ( ((st)>= RFAL_NFC_STATE_ACTIVATED) && ((st)<RFAL_NFC_STATE_DEACTIVATION) )

/*! Checks if a device is in discovery */
#define RFal_NFC_IsInDiscovery( st )         ( ((st)>= RFAL_NFC_STATE_START_DISCOVERY) && ((st)<RFAL_NFC_STATE_ACTIVATED) )

/*! Checks if remote device is in Poll mode */
#define RFal_NFC_IsRemDevPoller( tp )    ( ((tp)>= RFAL_NFC_POLL_TYPE_NFCA) && ((tp)<=RFAL_NFC_POLL_TYPE_AP2P ) )

/*! Checks if remote device is in Listen mode */
#define RFal_NFC_IsRemDevListener( tp )  ( ((int16_t)(tp)>= (int16_t)RFAL_NFC_LISTEN_TYPE_NFCA) && ((tp)<=RFAL_NFC_LISTEN_TYPE_AP2P) )

/*! Sets the discover parameters to its default values */
#define RFal_NFC_DefaultDiscParams(dp)                       \
  if ((dp) != NULL)                                        \
  {                                                        \
    ST_MEMSET((dp), 0x00, sizeof(RFal_NFC_DiscoverParam)); \
    ((dp))->compMode = RFAL_COMPLIANCE_MODE_NFC;           \
    ((dp))->devLimit = 1U;                                 \
    ((dp))->nfcfBR = RFAL_BR_212;                          \
    ((dp))->ap2pBR = RFAL_BR_424;                          \
    ((dp))->maxBR = RFAL_BR_KEEP;                          \
    ((dp))->isoDepFS = RFAL_ISODEP_FSXI_256;               \
    ((dp))->nfcDepLR = RFAL_NFCDEP_LR_254;                 \
    ((dp))->GBLen = 0U;                                    \
    ((dp))->p2pNfcaPrio = false;                           \
    ((dp))->wakeupEnabled = false;                         \
    ((dp))->wakeupConfigDefault = true;                    \
    ((dp))->wakeupPollBefore = false;                      \
    ((dp))->wakeupNPolls = 1U;                             \
    ((dp))->totalDuration = 1000U;                         \
    ((dp))->techs2Find = RFAL_NFC_TECH_NONE;               \
    ((dp))->techs2Bail = RFAL_NFC_TECH_NONE;               \
  }

/*! Main state                                                                       */
typedef enum {
  RFAL_NFC_STATE_NOTINIT                  =  0,   /*!< Not Initialized state       */
  RFAL_NFC_STATE_IDLE                     =  1,   /*!< Initialize state            */
  RFAL_NFC_STATE_START_DISCOVERY          =  2,   /*!< Start Discovery loop state  */
  RFAL_NFC_STATE_WAKEUP_MODE              =  3,   /*!< Wake-Up state               */
  RFAL_NFC_STATE_POLL_TECHDETECT          =  10,  /*!< Technology Detection state  */
  RFAL_NFC_STATE_POLL_COLAVOIDANCE        =  11,  /*!< Collision Avoidance state   */
  RFAL_NFC_STATE_POLL_SELECT              =  12,  /*!< Wait for Selection state    */
  RFAL_NFC_STATE_POLL_ACTIVATION          =  13,  /*!< Activation state            */
  RFAL_NFC_STATE_LISTEN_TECHDETECT        =  20,  /*!< Listen Tech Detect          */
  RFAL_NFC_STATE_LISTEN_COLAVOIDANCE      =  21,  /*!< Listen Collision Avoidance  */
  RFAL_NFC_STATE_LISTEN_ACTIVATION        =  22,  /*!< Listen Activation state     */
  RFAL_NFC_STATE_LISTEN_SLEEP             =  23,  /*!< Listen Sleep state          */
  RFAL_NFC_STATE_ACTIVATED                =  30,  /*!< Activated state             */
  RFAL_NFC_STATE_DATAEXCHANGE             =  31,  /*!< Data Exchange Start state   */
  RFAL_NFC_STATE_DATAEXCHANGE_DONE        =  33,  /*!< Data Exchange terminated    */
  RFAL_NFC_STATE_DEACTIVATION             =  34   /*!< Deactivation state          */
} RFal_NFC_State;


/*! Device type                                                                       */
typedef enum {
  RFAL_NFC_LISTEN_TYPE_NFCA               =  0,   /*!< NFC-A Listener device type  */
  RFAL_NFC_LISTEN_TYPE_NFCB               =  1,   /*!< NFC-B Listener device type  */
  RFAL_NFC_LISTEN_TYPE_NFCF               =  2,   /*!< NFC-F Listener device type  */
  RFAL_NFC_LISTEN_TYPE_NFCV               =  3,   /*!< NFC-V Listener device type  */
  RFAL_NFC_LISTEN_TYPE_ST25TB             =  4,   /*!< ST25TB Listener device type */
  RFAL_NFC_LISTEN_TYPE_AP2P               =  5,   /*!< AP2P Listener device type   */
  RFAL_NFC_LISTEN_TYPE_PROP               =  6,   /*!< Proprietary Listen dev type */
  RFAL_NFC_POLL_TYPE_NFCA                 =  10,  /*!< NFC-A Poller device type    */
  RFAL_NFC_POLL_TYPE_NFCB                 =  11,  /*!< NFC-B Poller device type    */
  RFAL_NFC_POLL_TYPE_NFCF                 =  12,  /*!< NFC-F Poller device type    */
  RFAL_NFC_POLL_TYPE_NFCV                 =  13,  /*!< NFC-V Poller device type    */
  RFAL_NFC_POLL_TYPE_AP2P                 =  15   /*!< AP2P Poller device type     */
} RFal_NFC_DevType;


/*! Device interface                                                                 */
typedef enum {
  RFAL_NFC_INTERFACE_RF                   = 0,    /*!< RF Frame interface          */
  RFAL_NFC_INTERFACE_ISODEP               = 1,    /*!< ISO-DEP interface           */
  RFAL_NFC_INTERFACE_NFCDEP               = 2     /*!< NFC-DEP interface           */
} RFal_NFC_RfInterface;

/*! Deactivation type                                                                     */
typedef enum {
  RFAL_NFC_DEACTIVATE_IDLE                = 0,    /*!< Deactivate and go to IDLE        */
  RFAL_NFC_DEACTIVATE_SLEEP               = 1,    /*!< Deactivate and go to SELECT      */
  RFAL_NFC_DEACTIVATE_DISCOVERY           = 2     /*!< Deactivate and restart DISCOVERY */
} RFal_NFC_DeactivateType;

/*! Device struct containing all its details                                          */
typedef struct {
  RFal_NFC_DevType type;                            /*!< Device's type                */
  union {                             /*  PRQA S 0750 # MISRA 19.2 - Members of the union will not be used concurrently, only one technology at a time */
    RFal_NFCA_ListenDevice   nfca;                /*!< NFC-A Listen Device instance */
    RFal_NFCB_ListenDevice   nfcb;                /*!< NFC-B Listen Device instance */
    RFal_NFCF_ListenDevice   nfcf;                /*!< NFC-F Listen Device instance */
    RFal_NFCV_ListenDevice   nfcv;                /*!< NFC-V Listen Device instance */
    RFal_ST25TB_ListenDevice st25tb;              /*!< ST25TB Listen Device instance*/
  } dev;                                          /*!< Device's instance            */

  uint8_t                    *nfcid;              /*!< Device's NFCID               */
  uint8_t                    nfcidLen;            /*!< Device's NFCID length        */
  RFal_NFC_RfInterface         rfInterface;         /*!< Device's interface           */

  union {                             /*  PRQA S 0750 # MISRA 19.2 - Members of the union will not be used concurrently, only one protocol at a time */
    RFal_ISO_Dep_Device       isoDep;              /*!< ISO-DEP instance             */
    RFal_NFC_Dep_Device       nfcDep;              /*!< NFC-DEP instance             */
  } proto;                                        /*!< Device's protocol            */
} RFal_NFC_Device;

/*! Callbacks for Proprietary|Other Technology      Activity 2.1   &   EMVCo 3.0  9.2 */
typedef NFC_OpResult(* RFal_NFC_PropCallback)(void);


/*! Struct that holds the Proprietary NFC callbacks                                                                                  */
typedef struct {
  RFal_NFC_PropCallback    RFal_NFC_pPollerInitialize;                    /*!< Prorietary NFC Initialization callback                  */
  RFal_NFC_PropCallback    RFal_NFC_pPollerTechnologyDetection;           /*!< Prorietary NFC Technoly Detection callback              */
  RFal_NFC_PropCallback    RFal_NFC_pPollerStartCollisionResolution;      /*!< Prorietary NFC Start Collision Resolution callback      */
  RFal_NFC_PropCallback    RFal_NFC_pPollerGetCollisionResolutionStatus;  /*!< Prorietary NFC Get Collision Resolution status callback */
  RFal_NFC_PropCallback    RFal_NFC_pStartActivation;                     /*!< Prorietary NFC Start Activation callback                */
  RFal_NFC_PropCallback    RFal_NFC_pGetActivationStatus;                 /*!< Prorietary NFC Get Activation status callback           */
} RFal_NFC_PropCallbacks;

/*! Discovery parameters                                                                                           */
typedef struct {
  RFal_ComplianceMode compMode;                        /*!< Compliance mode to be used                            */
  uint16_t           techs2Find;                      /*!< Technologies to search for                            */
  uint16_t           techs2Bail;                       /*!< Bail-out after certain NFC technologies                            */
  uint16_t               totalDuration;                    /*!< Duration of a whole Poll + Listen cycle        NCI 2.1 Table 46    */
  uint8_t                devLimit;                         /*!< Max number of devices                      Activity 2.1  Table 11  */
  RFal_BitRate            maxBR;                            /*!< Max Bit rate to be used                        NCI 2.1  Table 28   */

  RFal_BitRate            nfcfBR;                           /*!< Bit rate to poll for NFC-F                     NCI 2.1  Table 27   */
  uint8_t                nfcid3[RFAL_NFCDEP_NFCID3_LEN];   /*!< NFCID3 to be used on the ATR_REQ/ATR_RES                           */
  uint8_t                GB[RFAL_NFCDEP_GB_MAX_LEN];       /*!< General bytes to be used on the ATR-REQ        NCI 2.1  Table 29   */
  uint8_t                GBLen;                            /*!< Length of the General Bytes                    NCI 2.1  Table 29   */
  RFal_BitRate            ap2pBR;                           /*!< Bit rate to poll for AP2P                      NCI 2.1  Table 31   */
  bool                   p2pNfcaPrio;                      /*!< NFC-A P2P (true) or ISO14443-4/T4T (false) priority                */
  RFal_NFC_PropCallbacks   propNfc;                          /*!< Proprietary Technology callbacks                                    */


  RFal_ISO_Dep_FSxI         isoDepFS;                         /*!< ISO-DEP Poller announced maximum frame size   Digital 2.2 Table 60 */
  uint8_t                nfcDepLR;                         /*!< NFC-DEP Poller & Listener maximum frame size  Digital 2.2 Table 90 */

  RFal_LmConfPA           lmConfigPA;                       /*!< Configuration for Passive Listen mode NFC-A                        */
  RFal_LmConfPF           lmConfigPF;                       /*!< Configuration for Passive Listen mode NFC-A                        */

  void (*notifyCb)(RFal_NFC_State st);                       /*!< Callback to Notify upper layer                                     */

  bool                   wakeupEnabled;                    /*!< Enable Wake-Up mode before polling                                 */
  bool                   wakeupConfigDefault;              /*!< Wake-Up mode default configuration                                 */
  RFal_WakeUpConfig       wakeupConfig;                     /*!< Wake-Up mode configuration                                         */
  bool                   wakeupPollBefore;                 /*!< Flag to Poll wakeupNPolls times before entering Wake-up            */
  uint16_t               wakeupNPolls;                     /*!< Number of polling cycles before|after entering Wake-up             */
} RFal_NFC_DiscoverParam;


/*! Buffer union, only one interface is used at a time                                                             */
typedef union { /*  PRQA S 0750 # MISRA 19.2 - Members of the union will not be used concurrently, only one interface at a time */
  uint8_t                  rfBuf[RFAL_FEATURE_NFC_RF_BUF_LEN]; /*!< RF buffer                                    */
  RFal_ISO_Dep_ApduBufFormat  isoDepBuf;                          /*!< ISO-DEP buffer format (with header/prologue) */
  RFal_NFC_Dep_PduBufFormat   nfcDepBuf;                          /*!< NFC-DEP buffer format (with header/prologue) */
} RFal_NFC_Buffer;

/*! Buffer union, only one interface is used at a time                                                        */
typedef union { /*PRQA S 0750 # MISRA 19.2 - Members of the union will not be used concurrently, only one interface at a time */
  RFal_ISO_Dep_BufFormat   isoDepBuf;                  /*!< ISO-DEP buffer format (with header/prologue)       */
  RFal_NFC_Dep_BufFormat   nfcDepBuf;                  /*!< NFC-DEP buffer format (with header/prologue)       */
} RFal_NFC_TmpBuffer;


/*! RFAL NFC instance                                                                                */
typedef struct {
  RFal_NFC_State            state;              /*!< Main state                                      */
  uint16_t                techsFound;         /*!< Technologies found bitmask                      */
  uint16_t                techs2do;           /*!< Technologies still to be performed              */
  uint16_t                techDctCnt;         /*!< Technologies detection counter (before WU)      */
  RFal_BitRate             ap2pBR;             /*!< Bit rate to poll for AP2P                       */
  uint8_t                 selDevIdx;          /*!< Selected device index                           */
  RFal_NFC_Device           *activeDev;         /*!< Active device pointer                           */
  RFal_NFC_DiscoverParam    disc;               /*!< Discovery parameters                            */
  RFal_NFC_Device           devList[RFAL_NFC_MAX_DEVICES];   /*!< Location of device list            */
  uint8_t                 devCnt;             /*!< Devices found counter                           */
  uint32_t                discTmr;            /*!< Discovery Total duration timer                  */
  NFC_OpResult              dataExErr;          /*!< Last Data Exchange error                        */
  bool                    discRestart;        /* Restart discover after deactivation flag        */
  RFal_NFC_DeactivateType   deactType;          /*!< Deactivation type                               */
  bool                    isRxChaining;       /*!< Flag indicating Other device is chaining        */
  uint32_t                lmMask;             /*!< Listen Mode mask                                */
  bool                    isFieldOn;          /*!< Flag indicating Fieldon for Passive Poll        */
  bool                    isTechInit;         /*!< Flag indicating technology has been set         */
  bool                    isOperOngoing;      /*!< Flag indicating operation is ongoing            */
  bool                    isDeactivating;     /*!< Flag indicating deactivation is ongoing         */

  RFal_NFCA_SensRes         sensRes;            /*!< SENS_RES during card detection and activation   */
  RFal_NFCB_SensbRes        sensbRes;           /*!< SENSB_RES during card detection and activation  */
  uint8_t                 sensbResLen;        /*!< SENSB_RES length                                */

  RFal_NFC_Buffer           txBuf;              /*!< Tx buffer for Data Exchange                     */
  RFal_NFC_Buffer           rxBuf;              /*!< Rx buffer for Data Exchange                     */
  uint16_t                rxLen;              /*!< Length of received data on Data Exchange        */

  RFal_NFC_TmpBuffer        tmpBuf;             /*!< Tmp buffer for Data Exchange                    */

} RFal_NFC;

#ifdef __cplusplus
extern "C" {
#endif

void RFal_NFC_Worker(void);
NFC_OpResult RFal_NFC_Init(void);
NFC_OpResult RFal_NFC_Discover(const RFal_NFC_DiscoverParam *disParams);
RFal_NFC_State RFal_NFC_GetState(void);
NFC_OpResult RFal_NFC_GetDevicesFound(RFal_NFC_Device **devList, uint8_t *devCnt);
NFC_OpResult RFal_NFC_GetActiveDevice(RFal_NFC_Device **dev);
NFC_OpResult RFal_NFC_Select(uint8_t devIdx);
NFC_OpResult RFal_NFC_DataExchangeStart(uint8_t *txData, uint16_t txDataLen, uint8_t **rxData, uint16_t **rvdLen, uint32_t fwt);
NFC_OpResult RFal_NFC_DataExchangeGetStatus(void);
NFC_OpResult RFal_NFC_Deactivate(RFal_NFC_DeactivateType deactType);

NFC_OpResult RFal_NFC_PollTechDetection(void);
NFC_OpResult RFal_NFC_PollCollResolution(void);
NFC_OpResult RFal_NFC_PollActivation(uint8_t devIt);
NFC_OpResult RFal_NFC_ListenActivation(void);
NFC_OpResult RFal_NFC_Dep_Activate(RFal_NFC_Device *device, RFal_NFC_Dep_CommMode commMode, const uint8_t *atrReq, uint16_t atrReqLen);
NFC_OpResult RFal_NFC_DeActivation(void);

uint32_t timerCalculateTimer(uint16_t time);
bool timerIsExpired(uint32_t timer);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_NFC_H */
