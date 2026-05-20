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

#ifndef RFAL_NFCA_H
#define RFAL_NFCA_H

#include "RFal_RF.h"
#include "RFal_T1T.h"

#include "NFC/NFC_Def.h"

#define RFAL_NFCA_CASCADE_1_UID_LEN                           4U    /*!< UID length of cascade level 1 only tag                            */
#define RFAL_NFCA_CASCADE_2_UID_LEN                           7U    /*!< UID length of cascade level 2 only tag                            */
#define RFAL_NFCA_CASCADE_3_UID_LEN                           10U   /*!< UID length of cascade level 3 only tag                            */

#define RFAL_NFCA_SENS_RES_PLATFORM_MASK                      0x0FU /*!< SENS_RES (ATQA) platform configuration mask  Digital 1.1 Table 10 */
#define RFAL_NFCA_SENS_RES_PLATFORM_T1T                       0x0CU /*!< SENS_RES (ATQA) T1T platform configuration  Digital 1.1 Table 10  */

#define RFAL_NFCA_SEL_RES_CONF_MASK                           0x60U /*!< SEL_RES (SAK) platform configuration mask  Digital 1.1 Table 19   */
#define RFAL_NFCA_SEL_RES_CONF_T2T                            0x00U /*!< SEL_RES (SAK) T2T configuration  Digital 1.1 Table 19             */
#define RFAL_NFCA_SEL_RES_CONF_T4T                            0x20U /*!< SEL_RES (SAK) T4T configuration  Digital 1.1 Table 19             */
#define RFAL_NFCA_SEL_RES_CONF_NFCDEP                         0x40U /*!< SEL_RES (SAK) NFC-DEP configuration  Digital 1.1 Table 19         */
#define RFAL_NFCA_SEL_RES_CONF_T4T_NFCDEP                     0x60U /*!< SEL_RES (SAK) T4T and NFC-DEP configuration  Digital 1.1 Table 19 */


/*! NFC-A minimum FDT(listen) = ((n * 128 + (84)) / fc) with n_min = 9      Digital 1.1  6.10.1
 *                            = (1236)/fc
 * Relax with 3etu: (3*128)/fc as with multiple NFC-A cards, response may take longer (JCOP cards)
 *                            = (1236 + 384)/fc = 1620 / fc                                      */
#define RFAL_NFCA_FDTMIN          1620U

/*! Checks if device is a T1T given its SENS_RES */
#define RFal_NFCA_IsSensResT1T( sensRes )          ((((RFal_NFCA_SensRes*)(sensRes))->platformInfo & RFAL_NFCA_SENS_RES_PLATFORM_MASK) == RFAL_NFCA_SENS_RES_PLATFORM_T1T )

/*! Checks if device is a T2T given its SENS_RES */
#define RFal_NFCA_IsSelResT2T( selRes )            ((((RFal_NFCA_SelRes*)(selRes))->sak & RFAL_NFCA_SEL_RES_CONF_MASK) == RFAL_NFCA_SEL_RES_CONF_T2T )

/*! Checks if device is a T4T given its SENS_RES */
#define RFal_NFCA_IsSelResT4T( selRes )            ((((RFal_NFCA_SelRes*)(selRes))->sak & RFAL_NFCA_SEL_RES_CONF_MASK) == RFAL_NFCA_SEL_RES_CONF_T4T )

/*! Checks if device supports NFC-DEP protocol given its SENS_RES */
#define RFal_NFCA_IsSelResNFCDEP( selRes )         ((((RFal_NFCA_SelRes*)(selRes))->sak & RFAL_NFCA_SEL_RES_CONF_MASK) == RFAL_NFCA_SEL_RES_CONF_NFCDEP )

/*! Checks if device supports ISO-DEP and NFC-DEP protocol given its SENS_RES */
#define RFal_NFCA_IsSelResT4TNFCDEP( selRes )      ((((RFal_NFCA_SelRes*)(selRes))->sak & RFAL_NFCA_SEL_RES_CONF_MASK) == RFAL_NFCA_SEL_RES_CONF_T4T_NFCDEP )

/*! Checks if a NFC-A listener device supports multiple protocols (ISO-DEP and NFC-DEP) */
#define RFal_NFCA_LisDevIsMultiProto( lisDev )     (((RFal_NFCA_ListenDevice*)(lisDev))->type == RFAL_NFCA_T4T_NFCDEP )

/*! NFC-A Listen device types */
typedef enum {
  RFAL_NFCA_T1T        = 0x01,                                  /* Device configured for T1T  Digital 1.1 Table 9                               */
  RFAL_NFCA_T2T        = 0x00,                                  /* Device configured for T2T  Digital 1.1 Table 19                              */
  RFAL_NFCA_T4T        = 0x20,                                  /* Device configured for T4T  Digital 1.1 Table 19                              */
  RFAL_NFCA_NFCDEP     = 0x40,                                  /* Device configured for NFC-DEP  Digital 1.1 Table 19                          */
  RFAL_NFCA_T4T_NFCDEP = 0x60                                   /* Device configured for NFC-DEP and T4T  Digital 1.1 Table 19                  */
} RFal_NFCA_ListenDeviceType;


/*! SENS_RES (ATQA) format  Digital 1.1  6.6.3 & Table 7 */
typedef struct {
  uint8_t      anticollisionInfo;                               /*!< SENS_RES Anticollision Information                                         */
  uint8_t      platformInfo;                                    /*!< SENS_RES Platform Information                                              */
} RFal_NFCA_SensRes;


/*! SDD_REQ (Anticollision) format   Digital 1.1  6.7.1 & Table 11 */
typedef struct {
  uint8_t      selCmd;                                          /*!< SDD_REQ SEL_CMD: cascade Level                                             */
  uint8_t      selPar;                                          /*!< SDD_REQ SEL_PAR: Byte Count[4b] | Bit Count[4b] (NVB: Number of Valid Bits)*/
} RFal_NFCA_SddReq;


/*! SDD_RES (UID CLn) format   Digital 1.1  6.7.2 & Table 15 */
typedef struct {
  uint8_t      nfcid1[RFAL_NFCA_CASCADE_1_UID_LEN];             /*!< NFCID1 cascade level NFCID                                                 */
  uint8_t      bcc;                                             /*!< BCC Exclusive-OR over first 4 bytes of SDD_RES                             */
} RFal_NFCA_SddRes;


/*! SEL_REQ (Select) format   Digital 1.1  6.8.1 & Table 17 */
typedef struct {
  uint8_t      selCmd;                                          /*!< SDD_REQ SEL_CMD: cascade Level                                             */
  uint8_t      selPar;                                          /*!< SDD_REQ SEL_PAR: Byte Count[4b] | Bit Count[4b] (NVB: Number of Valid Bits)*/
  uint8_t      nfcid1[RFAL_NFCA_CASCADE_1_UID_LEN];             /*!< NFCID1 data                                                                */
  uint8_t      bcc;                                             /*!< Checksum calculated as exclusive-OR over the 4 bytes of NFCID1 CLn         */
} RFal_NFCA_SelReq;


/*! SEL_RES (SAK) format   Digital 1.1  6.8.2 & Table 19 */
typedef struct {
  uint8_t      sak;                                             /*!< Select Acknowledge                                                         */
} RFal_NFCA_SelRes;


/*! NFC-A listener device (PICC) struct  */
typedef struct {
  RFal_NFCA_ListenDeviceType type;                                /*!< NFC-A Listen device type                                                   */
  RFal_NFCA_SensRes          sensRes;                             /*!< SENS_RES (ATQA)                                                            */
  RFal_NFCA_SelRes           selRes;                              /*!< SEL_RES  (SAK)                                                             */
  uint8_t                  nfcId1Len;                           /*!< NFCID1 Length                                                              */
  uint8_t                  nfcId1[RFAL_NFCA_CASCADE_3_UID_LEN]; /*!< NFCID1   (UID)                                                             */
  RFal_T1T_RidRes            ridRes;                              /*!< RID_RES                                                                    */
  bool                     isSleep;                             /*!< Device sleeping flag                                                       */
} RFal_NFCA_ListenDevice;

#ifdef __cplusplus
extern "C" {
#endif

NFC_OpResult RFal_NFCA_PollerInit(void);
NFC_OpResult RFal_NFCA_PollerCheckPresence(RFal_14443AShortFrameCmd cmd, RFal_NFCA_SensRes *sensRes);
NFC_OpResult RFal_NFCA_PollerSelect(const uint8_t *nfcid1, uint8_t nfcidLen, RFal_NFCA_SelRes *selRes);
NFC_OpResult RFal_NFCA_PollerStartSelect(const uint8_t *nfcid1, uint8_t nfcidLen, RFal_NFCA_SelRes *selRes);
NFC_OpResult RFal_NFCA_PollerGetSelectStatus(void);
NFC_OpResult RFal_NFCA_PollerSleep(void);
NFC_OpResult RFal_NFCA_PollerStartSleep(void);
NFC_OpResult RFal_NFCA_PollerGetSleepStatus(void);
bool RFal_NFCA_ListenerIsSleepReq(const uint8_t *buf, uint16_t bufLen);
NFC_OpResult RFal_NFCA_PollerTechnologyDetection(RFal_ComplianceMode compMode, RFal_NFCA_SensRes *sensRes);
NFC_OpResult RFal_NFCA_PollerStartTechnologyDetection(RFal_ComplianceMode compMode, RFal_NFCA_SensRes *sensRes);
NFC_OpResult RFal_NFCA_PollerSingleCollisionResolution(uint8_t devLimit, bool *collPending, RFal_NFCA_SelRes *selRes, uint8_t *nfcId1, uint8_t *nfcId1Len);
NFC_OpResult RFal_NFCA_PollerFullCollisionResolution(RFal_ComplianceMode compMode, uint8_t devLimit, RFal_NFCA_ListenDevice *nfcaDevList, uint8_t *devCnt);
bool RFal_NFCA_ListenerIsSleepReq(const uint8_t *buf, uint16_t bufLen);

NFC_OpResult RFal_NFCA_PollerGetTechnologyDetectionStatus(void);
NFC_OpResult RFal_NFCA_PollerStartFullCollisionResolution(RFal_ComplianceMode compMode, uint8_t devLimit, RFal_NFCA_ListenDevice *nfcaDevList, uint8_t *devCnt);
NFC_OpResult RFal_NFCA_PollerGetFullCollisionResolutionStatus(void);
NFC_OpResult RFal_NFCB_PollerGetCollisionResolutionStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_NFCA_H */
