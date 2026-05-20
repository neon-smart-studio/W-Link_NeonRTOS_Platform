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

#ifndef RFAL_T4T_H
#define RFAL_T4T_H

#include "RFal_RF.h"
#include "RFal_ISO_Dep.h"

#include "NFC/NFC_Def.h"

#define RFAL_T4T_MAX_CAPDU_PROLOGUE_LEN                          4U                          /*!< Command-APDU prologue length (CLA INS P1 P2)                    */
#define RFAL_T4T_LE_LEN                                          1U                          /*!< Le Expected Response Length (short field coding)                */
#define RFAL_T4T_LC_LEN                                          1U                          /*!< Lc Data field length  (short field coding)                      */
#define RFAL_T4T_MAX_RAPDU_SW1SW2_LEN                            2U                          /*!< SW1 SW2 length                                                  */
#define RFAL_T4T_CLA                                          0x00U                          /*!< Class byte (contains 00h because secure message are not used)   */

#define RFAL_T4T_ISO7816_P1_SELECT_BY_DF_NAME                 0x04U                          /*!< P1 value for Select by name                                     */
#define RFAL_T4T_ISO7816_P1_SELECT_BY_FILEID                  0x00U                          /*!< P1 value for Select by file identifier                          */
#define RFAL_T4T_ISO7816_P2_SELECT_FIRST_OR_ONLY_OCCURENCE    0x00U                          /*!<      b2b1 P2 value for First or only occurrence                  */
#define RFAL_T4T_ISO7816_P2_SELECT_RETURN_FCI_TEMPLATE        0x00U                          /*!< b4b3      P2 value for Return FCI template                      */
#define RFAL_T4T_ISO7816_P2_SELECT_NO_RESPONSE_DATA           0x0CU                          /*!< b4b3      P2 value for No response data                         */

#define RFAL_T4T_ISO7816_STATUS_COMPLETE                      0x9000U                        /*!< Command completed \ Normal processing - No further qualification*/

/*! NFC-A T4T Command-APDU structure */
typedef struct {
  uint8_t                  CLA;                              /*!< Class byte                                         */
  uint8_t                  INS;                              /*!< Instruction byte                                   */
  uint8_t                  P1;                               /*!< Parameter byte 1                                   */
  uint8_t                  P2;                               /*!< Parameter byte 2                                   */
  uint8_t                  Lc;                               /*!< Data field length                                  */
  bool                     LcFlag;                           /*!< Lc flag (append Lc when true)                      */
  uint8_t                  Le;                               /*!< Expected Response Length                           */
  bool                     LeFlag;                           /*!< Le flag (append Le when true)                      */

  RFal_ISO_Dep_ApduBufFormat  *cApduBuf;                        /*!< Command-APDU buffer  (Tx)                          */
  uint16_t                 *cApduLen;                        /*!< Command-APDU Length                                */
} RFal_T4T_CApduParam;

/*! NFC-A T4T Response-APDU structure */
typedef struct {
  RFal_ISO_Dep_ApduBufFormat  *rApduBuf;                        /*!< Response-APDU buffer (Rx)                          */
  uint16_t                 rcvdLen;                          /*!< Full response length                               */
  uint16_t                 rApduBodyLen;                     /*!< Response body length                               */
  uint16_t                 statusWord;                       /*!< R-APDU Status Word SW1|SW2                         */
} RFal_T4T_RApduParam;



/*! NFC-A T4T command set    T4T 1.0 & ISO7816-4 2013 Table 4 */
typedef enum {
  RFAL_T4T_INS_SELECT           = 0xA4U,                     /*!< T4T Select                                         */
  RFAL_T4T_INS_READBINARY       = 0xB0U,                     /*!< T4T ReadBinary                                     */
  RFAL_T4T_INS_UPDATEBINARY     = 0xD6U,                     /*!< T4T UpdateBinay                                    */
  RFAL_T4T_INS_READBINARY_ODO   = 0xB1U,                     /*!< T4T ReadBinary using ODO                           */
  RFAL_T4T_INS_UPDATEBINARY_ODO = 0xD7U                      /*!< T4T UpdateBinay using ODO                          */
} RFal_T4T_Cmds;

#ifdef __cplusplus
extern "C" {
#endif

NFC_OpResult RFal_T4T_PollerComposeCAPDU(const RFal_T4T_CApduParam *apduParam);
NFC_OpResult RFal_T4T_PollerParseRAPDU(RFal_T4T_RApduParam *apduParam);
NFC_OpResult RFal_T4T_PollerComposeSelectAppl(RFal_ISO_Dep_ApduBufFormat *cApduBuf, const uint8_t *aid, uint8_t aidLen, uint16_t *cApduLen);
NFC_OpResult RFal_T4T_PollerComposeSelectFile(RFal_ISO_Dep_ApduBufFormat *cApduBuf, const uint8_t *fid, uint8_t fidLen, uint16_t *cApduLen);
NFC_OpResult RFal_T4T_PollerComposeSelectFileV1Mapping(RFal_ISO_Dep_ApduBufFormat *cApduBuf, const uint8_t *fid, uint8_t fidLen, uint16_t *cApduLen);
NFC_OpResult RFal_T4T_PollerComposeReadData(RFal_ISO_Dep_ApduBufFormat *cApduBuf, uint16_t offset, uint8_t expLen, uint16_t *cApduLen);
NFC_OpResult RFal_T4T_PollerComposeReadDataODO(RFal_ISO_Dep_ApduBufFormat *cApduBuf, uint32_t offset, uint8_t expLen, uint16_t *cApduLen);
NFC_OpResult RFal_T4T_PollerComposeWriteData(RFal_ISO_Dep_ApduBufFormat *cApduBuf, uint16_t offset, const uint8_t *data, uint8_t dataLen, uint16_t *cApduLen);
NFC_OpResult RFal_T4T_PollerComposeWriteDataODO(RFal_ISO_Dep_ApduBufFormat *cApduBuf, uint32_t offset, const uint8_t *data, uint8_t dataLen, uint16_t *cApduLen);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_T4T_H */
