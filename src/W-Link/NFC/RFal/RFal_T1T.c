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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "RFal_RF.h"
#include "RFal_NFC.h"
#include "RFal_T1T.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#define RFAL_T1T_DRD_READ           (1236U*2U) /*!< DRD for Reads with n=9         => 1236/fc  ~= 91 us   T1T 1.2  4.4.2 */
#define RFAL_T1T_DRD_WRITE          36052U     /*!< DRD for Write with n=281       => 36052/fc ~= 2659 us T1T 1.2  4.4.2 */
#define RFAL_T1T_DRD_WRITE_E        70996U     /*!< DRD for Write/Erase with n=554 => 70996/fc ~= 5236 us T1T 1.2  4.4.2 */

#define RFAL_T1T_RID_RES_HR0_VAL    0x10U      /*!< HR0 indicating NDEF support  Digital 2.0 (Candidate) 11.6.2.1        */
#define RFAL_T1T_RID_RES_HR0_MASK   0xF0U      /*!< HR0 most significant nibble mask                                     */

/*! NFC-A T1T (Topaz) RID_REQ  Digital 1.1  10.6.1 & Table 49 */
typedef struct {
  uint8_t cmd;                               /*!< T1T cmd: RID              */
  uint8_t add;                               /*!< ADD: undefined value      */
  uint8_t data;                              /*!< DATA: undefined value     */
  uint8_t uid[RFAL_T1T_UID_LEN];             /*!< UID-echo: undefined value */
} RFal_T1T_RidReq;


/*! NFC-A T1T (Topaz) RALL_REQ   T1T 1.2  Table 4 */
typedef struct {
  uint8_t cmd;                               /*!< T1T cmd: RALL             */
  uint8_t add1;                              /*!< ADD: 0x00                 */
  uint8_t add0;                              /*!< ADD: 0x00                 */
  uint8_t uid[RFAL_T1T_UID_LEN];             /*!< UID                       */
} RFal_T1T_RallReq;


/*! NFC-A T1T (Topaz) WRITE_REQ   T1T 1.2  Table 4 */
typedef struct {
  uint8_t cmd;                               /*!< T1T cmd: RALL             */
  uint8_t add;                               /*!< ADD                       */
  uint8_t data;                              /*!< DAT                       */
  uint8_t uid[RFAL_T1T_UID_LEN];             /*!< UID                       */
} RFal_T1T_WriteReq;


/*! NFC-A T1T (Topaz) WRITE_RES   T1T 1.2  Table 4 */
typedef struct {
  uint8_t add;                               /*!< ADD                       */
  uint8_t data;                              /*!< DAT                       */
} RFal_T1T_WriteRes;

NFC_OpResult RFal_T1T_PollerInit(void)
{
  NFC_OpResult ret;

  ret = RFal_SetMode(RFAL_MODE_POLL_NFCA_T1T, RFAL_BR_106, RFAL_BR_106);
  if(ret < NFC_OK)
  {
      return ret;
  }

  RFal_SetErrorHandling(ERRORHANDLING_NONE);

  RFal_SetGT(RFAL_GT_NONE);                            /* T1T should only be initialized after NFC-A mode, therefore the GT has been fulfilled */
  RFal_SetFDTListen(RFAL_FDT_LISTEN_NFCA_POLLER);      /* T1T uses NFC-A FDT Listen with n=9   Digital 1.1  10.7.2                             */
  RFal_SetFDTPoll(RFAL_FDT_POLL_NFCA_T1T_POLLER);

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_T1T_PollerRid(RFal_T1T_RidRes *ridRes)
{
  NFC_OpResult     ret;
  RFal_T1T_RidReq  ridReq;
  uint16_t       rcvdLen;

  if (ridRes == NULL) {
    return NFC_InvalidParameter;
  }

  /* Compute RID command and set Undefined Values to 0x00    Digital 1.1 10.6.1 */
  memset(&ridReq, 0x00, sizeof(RFal_T1T_RidReq));
  ridReq.cmd = (uint8_t)RFAL_T1T_CMD_RID;

  ret = RFal_TransceiveBlockingTxRx((uint8_t *)&ridReq, sizeof(RFal_T1T_RidReq), (uint8_t *)ridRes, sizeof(RFal_T1T_RidRes), &rcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_T1T_DRD_READ);
  if(ret < NFC_OK)
  {
      return ret;
  }

  /* Check expected RID response length and the HR0   Digital 2.0 (Candidate) 11.6.2.1 */
  if ((rcvdLen != sizeof(RFal_T1T_RidRes)) || ((ridRes->hr0 & RFAL_T1T_RID_RES_HR0_MASK) != RFAL_T1T_RID_RES_HR0_VAL)) {
    return NFC_ProtocolError;
  }

  return NFC_OK;
}


/*******************************************************************************/
NFC_OpResult RFal_T1T_PollerRall(const uint8_t *uid, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen)
{
  RFal_T1T_RallReq rallReq;

  if ((rxBuf == NULL) || (uid == NULL) || (rxRcvdLen == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Compute RALL command and set Add to 0x00 */
  memset(&rallReq, 0x00, sizeof(RFal_T1T_RallReq));
  rallReq.cmd = (uint8_t)RFAL_T1T_CMD_RALL;
  memcpy(rallReq.uid, uid, RFAL_T1T_UID_LEN);

  return RFal_TransceiveBlockingTxRx((uint8_t *)&rallReq, sizeof(RFal_T1T_RallReq), (uint8_t *)rxBuf, rxBufLen, rxRcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_T1T_DRD_READ);
}


/*******************************************************************************/
NFC_OpResult RFal_T1T_PollerWrite(const uint8_t *uid, uint8_t address, uint8_t data)
{
  RFal_T1T_WriteReq writeReq;
  RFal_T1T_WriteRes writeRes;
  uint16_t        rxRcvdLen;
  NFC_OpResult      err;

  if (uid == NULL) {
    return NFC_InvalidParameter;
  }

  writeReq.cmd  = (uint8_t)RFAL_T1T_CMD_WRITE_E;
  writeReq.add  = address;
  writeReq.data = data;
  memcpy(writeReq.uid, uid, RFAL_T1T_UID_LEN);

  err = RFal_TransceiveBlockingTxRx((uint8_t *)&writeReq, sizeof(RFal_T1T_WriteReq), (uint8_t *)&writeRes, sizeof(RFal_T1T_WriteRes), &rxRcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_T1T_DRD_WRITE_E);

  if (err == NFC_OK) {
    if ((writeReq.add != writeRes.add) || (writeReq.data != writeRes.data) || (rxRcvdLen != sizeof(RFal_T1T_WriteRes))) {
      return NFC_ProtocolError;
    }
  }
  return err;
}
