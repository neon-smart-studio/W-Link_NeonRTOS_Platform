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

#include "RFal_NFC.h"
#include "RFal.h"

#include "NFC/NFC_Def.h"

#include "NeonRTOS.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
#define RFAL_NFC_MAX_DEVICES          5U    /*!< Max number of devices supported */
#define RFAL_NFC_T_FIELD_OFF          5U    /*!< tFIELD_OFF minimal duration  Activity 2.2  Table 26 */
/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/


#define RFal_NFC_NfcNotify( st )                        if( gNfcDev.disc.notifyCb != NULL )  gNfcDev.disc.notifyCb( st )
#define RFal_NFC_pCbPollerInitialize()                   ((gNfcDev.disc.propNfc.RFal_NFC_pPollerInitialize != NULL) ? gNfcDev.disc.propNfc.RFal_NFC_pPollerInitialize() : NFC_Unsupport )
#define RFal_NFC_pCbPollerTechnologyDetection()          ((gNfcDev.disc.propNfc.RFal_NFC_pPollerTechnologyDetection != NULL) ? gNfcDev.disc.propNfc.RFal_NFC_pPollerTechnologyDetection() : NFC_SlaveTimeout )
#define RFal_NFC_pCbPollerStartCollisionResolution()     ((gNfcDev.disc.propNfc.RFal_NFC_pPollerStartCollisionResolution != NULL) ? gNfcDev.disc.propNfc.RFal_NFC_pPollerStartCollisionResolution() : NFC_Unsupport )
#define RFal_NFC_pCbPollerGetCollisionResolutionStatus() ((gNfcDev.disc.propNfc.RFal_NFC_pPollerGetCollisionResolutionStatus != NULL) ? gNfcDev.disc.propNfc.RFal_NFC_pPollerGetCollisionResolutionStatus() : NFC_Unsupport )
#define RFal_NFC_pCbStartActivation()                    ((gNfcDev.disc.propNfc.RFal_NFC_pStartActivation != NULL) ? gNfcDev.disc.propNfc.RFal_NFC_pStartActivation() : NFC_Unsupport )
#define RFal_NFC_pCbGetActivationStatus()                ((gNfcDev.disc.propNfc.RFal_NFC_pGetActivationStatus != NULL) ? gNfcDev.disc.propNfc.RFal_NFC_pGetActivationStatus() : NFC_Unsupport )

#define RFal_NFC_HasPollerTechs()                        ((gNfcDev.disc.techs2Find & (RFAL_NFC_POLL_TECH_A | RFAL_NFC_POLL_TECH_B | RFAL_NFC_POLL_TECH_F | RFAL_NFC_POLL_TECH_V |  \
                                                                                   RFAL_NFC_POLL_TECH_AP2P | RFAL_NFC_POLL_TECH_ST25TB | RFAL_NFC_POLL_TECH_PROP)) != 0U)

static RFal_NFC gNfcDev;

/*******************************************************************************/
NFC_OpResult RFal_NFC_Init(void)
{
  NFC_OpResult err;

  gNfcDev.state = RFAL_NFC_STATE_NOTINIT;

  //RFal_RfDev->RFal_AnalogConfigInitialize();//
  err = RFal_Init();   /* Initialize RFAL */
  if(err < NFC_OK)
  {
    return err;
  }

  memset(&gNfcDev, 0x00, sizeof(gNfcDev));

  gNfcDev.state = RFAL_NFC_STATE_IDLE;         /* Go to initialized */
  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFC_Discover(const RFal_NFC_DiscoverParam *disParams)
{
  /* Check if initialization has been performed */
  if (gNfcDev.state != RFAL_NFC_STATE_IDLE) {
    return NFC_WrongState;
  }

  /* Check valid parameters */
  if ((disParams == NULL) || (disParams->devLimit > RFAL_NFC_MAX_DEVICES) || (disParams->devLimit == 0U)                                              ||
      ((disParams->maxBR > RFAL_BR_1695) && (disParams->maxBR != RFAL_BR_KEEP))                                                                      ||
      (((disParams->techs2Find & RFAL_NFC_POLL_TECH_F) != 0U)     && (disParams->nfcfBR != RFAL_BR_212) && (disParams->nfcfBR != RFAL_BR_424))        ||
      ((((disParams->techs2Find & RFAL_NFC_POLL_TECH_AP2P) != 0U) && (disParams->ap2pBR > RFAL_BR_424)) || (disParams->GBLen > RFAL_NFCDEP_GB_MAX_LEN))) {
    return NFC_InvalidParameter;
  }
  /*
  if ((((disParams->techs2Find & RFAL_NFC_POLL_TECH_A) != 0U)      ) ||
      (((disParams->techs2Find & RFAL_NFC_POLL_TECH_B) != 0U)      ) ||
      (((disParams->techs2Find & RFAL_NFC_POLL_TECH_F) != 0U)      ) ||
      (((disParams->techs2Find & RFAL_NFC_POLL_TECH_V) != 0U)      ) ||
      (((disParams->techs2Find & RFAL_NFC_POLL_TECH_ST25TB) != 0U) ) ||
      (((disParams->techs2Find & RFAL_NFC_POLL_TECH_AP2P) != 0U)   ) ||
      (((disParams->techs2Find & RFAL_NFC_LISTEN_TECH_A) != 0U)    ) ||
      (((disParams->techs2Find & RFAL_NFC_LISTEN_TECH_B) != 0U)    ) ||
      (((disParams->techs2Find & RFAL_NFC_LISTEN_TECH_F) != 0U)    ) ||
      (((disParams->techs2Find & RFAL_NFC_LISTEN_TECH_AP2P) != 0U) )) {
    return NFC_Unsupport;   //  PRQA S  2880 # MISRA 2.1 - Unreachable code due to configuration option being set/unset
  }

  if (((disParams->techs2Find & RFAL_NFC_POLL_TECH_A) != 0U)      ||
      ((disParams->techs2Find & RFAL_NFC_POLL_TECH_B) != 0U)      ||
      ((disParams->techs2Find & RFAL_NFC_POLL_TECH_F) != 0U)      ||
      ((disParams->techs2Find & RFAL_NFC_POLL_TECH_V) != 0U)      ||
      ((disParams->techs2Find & RFAL_NFC_POLL_TECH_ST25TB) != 0U) ||
      ((disParams->techs2Find & RFAL_NFC_POLL_TECH_AP2P) != 0U)   ||
      ((disParams->techs2Find & RFAL_NFC_LISTEN_TECH_A) != 0U)    ||
      ((disParams->techs2Find & RFAL_NFC_LISTEN_TECH_B) != 0U)    ||
      ((disParams->techs2Find & RFAL_NFC_LISTEN_TECH_F) != 0U)    ||
      ((disParams->techs2Find & RFAL_NFC_LISTEN_TECH_AP2P) != 0U)) {
    return NFC_Unsupport;   //  PRQA S  2880 # MISRA 2.1 - Unreachable code due to configuration option being set/unset
  }
  */

  /* Initialize context for discovery */
  gNfcDev.activeDev       = NULL;
  gNfcDev.techsFound      = RFAL_NFC_TECH_NONE;
  gNfcDev.techDctCnt      = 0;
  gNfcDev.devCnt          = 0;
  //gNfcDev.discRestart     = true;
  //gNfcDev.techDctCnt      = 0;
  gNfcDev.deactType       = RFAL_NFC_DEACTIVATE_DISCOVERY;
  gNfcDev.isTechInit      = false;
  gNfcDev.isFieldOn       = false;
  gNfcDev.isDeactivating  = false;
  gNfcDev.disc            = *disParams;


  /* Calculate Listen Mask */
  gNfcDev.lmMask  = 0U;
  gNfcDev.lmMask |= (((gNfcDev.disc.techs2Find & RFAL_NFC_LISTEN_TECH_A) != 0U) ? RFAL_LM_MASK_NFCA : 0U);
  gNfcDev.lmMask |= (((gNfcDev.disc.techs2Find & RFAL_NFC_LISTEN_TECH_B) != 0U) ? RFAL_LM_MASK_NFCB : 0U);
  gNfcDev.lmMask |= (((gNfcDev.disc.techs2Find & RFAL_NFC_LISTEN_TECH_F) != 0U) ? RFAL_LM_MASK_NFCF : 0U);
  gNfcDev.lmMask |= (((gNfcDev.disc.techs2Find & RFAL_NFC_LISTEN_TECH_AP2P) != 0U) ? RFAL_LM_MASK_ACTIVE_P2P : 0U);

  gNfcDev.state = RFAL_NFC_STATE_START_DISCOVERY;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFC_Deactivate(RFal_NFC_DeactivateType deactType)
{
  /* Check for valid state */
  if ((gNfcDev.state <= RFAL_NFC_STATE_IDLE) || ((deactType == RFAL_NFC_DEACTIVATE_SLEEP) && ((gNfcDev.state < RFAL_NFC_STATE_ACTIVATED) || (gNfcDev.activeDev == NULL)))) {
    return NFC_WrongState;
  }
  /* Check valid parameters for the deactivation types */
  if (((deactType == RFAL_NFC_DEACTIVATE_SLEEP) && RFal_NFC_IsRemDevPoller(gNfcDev.activeDev->type))       ||
      ((deactType == RFAL_NFC_DEACTIVATE_DISCOVERY)  && (gNfcDev.disc.techs2Find == RFAL_NFC_TECH_NONE))) {
    return NFC_InvalidParameter;
  }

  gNfcDev.deactType = deactType;
  /* Check if Discovery is to continue afterwards or back to Select */
  if ((deactType == RFAL_NFC_DEACTIVATE_DISCOVERY) || (deactType == RFAL_NFC_DEACTIVATE_SLEEP)) {
    /* If so let the state machine continue*/
    gNfcDev.state       = RFAL_NFC_STATE_DEACTIVATION;
  } else {
    /* Otherwise deactivate immediately and go to IDLE */
    RFal_NFC_DeActivation();
    gNfcDev.state = RFAL_NFC_STATE_IDLE;
  }

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFC_Select(uint8_t devIdx)
{
  /* Check for valid state */
  if (gNfcDev.state != RFAL_NFC_STATE_POLL_SELECT) {
    return NFC_WrongState;
  }

  gNfcDev.isTechInit = false;
  gNfcDev.selDevIdx = devIdx;
  gNfcDev.state     = RFAL_NFC_STATE_POLL_ACTIVATION;

  return NFC_OK;
}

/*******************************************************************************/
RFal_NFC_State RFal_NFC_GetState(void)
{
  return gNfcDev.state;
}

/*******************************************************************************/
NFC_OpResult RFal_NFC_GetDevicesFound(RFal_NFC_Device **devList, uint8_t *devCnt)
{
  /* Check for valid state */
  if (gNfcDev.state < RFAL_NFC_STATE_POLL_SELECT) {
    return NFC_WrongState;
  }

  /* Check valid parameters */
  if ((devList == NULL) || (devCnt == NULL)) {
    return NFC_InvalidParameter;
  }

  *devCnt  = gNfcDev.devCnt;
  *devList = gNfcDev.devList;

  return NFC_OK;
}

/*******************************************************************************/
NFC_OpResult RFal_NFC_GetActiveDevice(RFal_NFC_Device **dev)
{
  /* Check for valid state */
  if (gNfcDev.state < RFAL_NFC_STATE_ACTIVATED) {
    return NFC_WrongState;
  }

  /* Check valid parameter */
  if (dev == NULL) {
    return NFC_InvalidParameter;
  }

  /* Check for valid state */
  if ((gNfcDev.devCnt == 0U) || (gNfcDev.activeDev == NULL)) {
    return NFC_RequestError;
  }

  *dev = gNfcDev.activeDev;
  return NFC_OK;
}

/*******************************************************************************/
void RFal_NFC_Worker(void)
{
  NFC_OpResult err;

  RFal_Worker();                                                                     /* Execute RFAL process  */

  switch (gNfcDev.state) {
    /*******************************************************************************/
    case RFAL_NFC_STATE_NOTINIT:
    case RFAL_NFC_STATE_IDLE:
      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_START_DISCOVERY:

      /* Initialize context for discovery cycle */
      gNfcDev.devCnt      = 0;
      gNfcDev.selDevIdx   = 0;
      memset(gNfcDev.devList, 0x00, sizeof(gNfcDev.devList));
      gNfcDev.techsFound  = RFAL_NFC_TECH_NONE;
      gNfcDev.techs2do    = gNfcDev.disc.techs2Find;
      gNfcDev.state       = RFAL_NFC_STATE_POLL_TECHDETECT;
      gNfcDev.isDeactivating = false;

      /* Start total duration timer */
      gNfcDev.discTmr = (uint32_t)timerCalculateTimer(gNfcDev.disc.totalDuration);

      /* Check if Low power Wake-Up is to be performed */
      if (gNfcDev.disc.wakeupEnabled && (((gNfcDev.techDctCnt == 0U) && (gNfcDev.disc.wakeupPollBefore == false)) || (gNfcDev.techDctCnt >= gNfcDev.disc.wakeupNPolls))) {
        /* Initialize Low power Wake-up mode and wait */
        err = RFal_WakeUpModeStart((gNfcDev.disc.wakeupConfigDefault ? NULL : &gNfcDev.disc.wakeupConfig));
        if (err == NFC_OK) {
          gNfcDev.state = RFAL_NFC_STATE_WAKEUP_MODE;
          RFal_NFC_NfcNotify(gNfcDev.state);                                  /* Notify caller that WU was started */
        }
      }
      gNfcDev.techDctCnt++;

      RFal_NFC_NfcNotify(gNfcDev.state);
      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_WAKEUP_MODE:

      /* Check if the Wake-up mode has woke */
      if (RFal_WakeUpModeHasWoke()) {
        RFal_WakeUpModeStop();                                                 /* Disable Wake-up mode           */
        gNfcDev.state = RFAL_NFC_STATE_POLL_TECHDETECT;                       /* Go to Technology detection     */

        gNfcDev.techDctCnt = 1;                                               /* Tech Detect counter (1 woke)   */

        /* (Re)Start total duration timer upon waking up */
        gNfcDev.discTmr = (uint32_t)timerCalculateTimer(gNfcDev.disc.totalDuration);
        RFal_NFC_NfcNotify(gNfcDev.state);                                      /* Notify caller that WU has woke */
      }

      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_POLL_TECHDETECT:

      /* Start total duration timer */
      //gNfcDev.discTmr = (uint32_t)timerCalculateTimer(gNfcDev.disc.totalDuration);

      err = RFal_NFC_PollTechDetection();                                       /* Perform Technology Detection                         */



      if (err != NFC_Busy) {                                                    /* Wait until all technologies are performed            */
        if ((err != NFC_OK) || (gNfcDev.techsFound == RFAL_NFC_TECH_NONE)) { /* Check if any error occurred or no techs were found   */

          RFal_FieldOff();
          gNfcDev.isFieldOn = false;
          gNfcDev.state = RFAL_NFC_STATE_LISTEN_TECHDETECT;                 /* Nothing found as poller, go to listener */
          break;
        }

        gNfcDev.techs2do = gNfcDev.techsFound;                                /* Store the found technologies for collision resolution */
        gNfcDev.state    = RFAL_NFC_STATE_POLL_COLAVOIDANCE;                  /* One or more devices found, go to Collision Avoidance  */
      }
      break;


    /*******************************************************************************/
    case RFAL_NFC_STATE_POLL_COLAVOIDANCE:

      err = RFal_NFC_PollCollResolution();                                        /* Resolve any eventual collision                       */
      if (err != NFC_Busy) {                                                    /* Wait until all technologies are performed            */
        if ((err != NFC_OK) || (gNfcDev.devCnt == 0U)) {                    /* Check if any error occurred or no devices were found */
          gNfcDev.deactType = RFAL_NFC_DEACTIVATE_DISCOVERY;
          gNfcDev.state = RFAL_NFC_STATE_DEACTIVATION;
          break;                                                            /* Unable to retrieve any device, restart loop          */
        }

        /* Check if more than one device has been found */
        if (gNfcDev.devCnt > 1U) {
          /* If more than one device was found inform upper layer to choose which one to activate */
          if (gNfcDev.disc.notifyCb != NULL) {
            gNfcDev.state = RFAL_NFC_STATE_POLL_SELECT;
            gNfcDev.disc.notifyCb(gNfcDev.state);
            break;
          }
        }

        /* If only one device or no callback has been set, activate the first device found */
        gNfcDev.selDevIdx = 0U;
        gNfcDev.state = RFAL_NFC_STATE_POLL_ACTIVATION;
      }
      break;


    /*******************************************************************************/
    case RFAL_NFC_STATE_POLL_ACTIVATION:

      err = RFal_NFC_PollActivation(gNfcDev.selDevIdx);
      if (err != NFC_Busy) {                                                    /* Wait until all Activation is complete */
        if (err != NFC_OK) {                                                /* Check if activation has failed        */
          /* Check if more than one device has been found */
          if ((gNfcDev.devCnt > 1U) && (gNfcDev.disc.notifyCb != NULL)) {
            gNfcDev.state = RFAL_NFC_STATE_POLL_SELECT;
            RFal_NFC_NfcNotify(gNfcDev.state);
            break;
          }

          gNfcDev.deactType = RFAL_NFC_DEACTIVATE_DISCOVERY;                /* Ensure deactivation, not Sleep        */
          gNfcDev.state     = RFAL_NFC_STATE_DEACTIVATION;                  /* If Activation failed, restart loop    */
          break;
        }

        gNfcDev.state = RFAL_NFC_STATE_ACTIVATED;                                 /* Device has been properly activated */
        RFal_NFC_NfcNotify(gNfcDev.state);                                          /* Inform upper layer that a device has been activated */
      }
      break;


    /*******************************************************************************/
    case RFAL_NFC_STATE_DATAEXCHANGE:

      RFal_NFC_DataExchangeGetStatus();                                           /* Run the internal state machine */

      if (gNfcDev.dataExErr != NFC_Busy) {                                      /* If Dataexchange has terminated */
        gNfcDev.state = RFAL_NFC_STATE_DATAEXCHANGE_DONE;                     /* Go to done state               */
        RFal_NFC_NfcNotify(gNfcDev.state);                                      /* And notify caller              */
      }
      if (gNfcDev.dataExErr == NFC_SleepRequest) {                                 /* Check if Listen mode has to go to Sleep */
        gNfcDev.state = RFAL_NFC_STATE_LISTEN_SLEEP;                          /* Go to Listen Sleep state       */
        RFal_NFC_NfcNotify(gNfcDev.state);                                      /* And notify caller              */
      }
      break;


    /*******************************************************************************/
    case RFAL_NFC_STATE_DEACTIVATION:

      err = RFal_NFC_DeActivation();                                              /* Deactivate current device */
      if (err != NFC_Busy) {
        if (gNfcDev.deactType == RFAL_NFC_DEACTIVATE_SLEEP) {
          gNfcDev.state = RFAL_NFC_STATE_POLL_SELECT;
        } else {
          gNfcDev.state = ((gNfcDev.deactType == RFAL_NFC_DEACTIVATE_DISCOVERY) ? RFAL_NFC_STATE_START_DISCOVERY : RFAL_NFC_STATE_IDLE);
        }

        RFal_NFC_NfcNotify(gNfcDev.state);                                          /* Notify caller             */
      }
      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_LISTEN_TECHDETECT:

      if (timerIsExpired(gNfcDev.discTmr)) {
        RFal_ListenStop();
        //RFal_FieldOff();
        gNfcDev.isFieldOn = false;

        gNfcDev.state = RFAL_NFC_STATE_START_DISCOVERY;                       /* Restart the discovery loop */
        RFal_NFC_NfcNotify(gNfcDev.state);                                      /* Notify caller             */
        break;
      }

      if (gNfcDev.lmMask != 0U) {                                               /* Check if configured to perform Listen mode */
        err = RFal_ListenStart(gNfcDev.lmMask, &gNfcDev.disc.lmConfigPA, NULL, &gNfcDev.disc.lmConfigPF, (uint8_t *)&gNfcDev.rxBuf.rfBuf, (uint16_t)RFal_ConvBytesToBits(sizeof(gNfcDev.rxBuf.rfBuf)), &gNfcDev.rxLen);
        if (err == NFC_OK) {
          gNfcDev.state = RFAL_NFC_STATE_LISTEN_COLAVOIDANCE;               /* Wait for listen mode to be activated */
        }
      }
      break;

    /*******************************************************************************/
    case RFAL_NFC_STATE_LISTEN_COLAVOIDANCE:

      if (timerIsExpired(gNfcDev.discTmr)) {                            /* Check if the total duration has been reached */
        RFal_ListenStop();
        gNfcDev.state = RFAL_NFC_STATE_START_DISCOVERY;                       /* Restart the discovery loop */
        RFal_NFC_NfcNotify(gNfcDev.state);                                      /* Notify caller             */
        break;
      }

      /* Check for external field */
      if ((RFal_ListenGetState(NULL, NULL)) >= RFAL_LM_STATE_IDLE) {
        gNfcDev.state = RFAL_NFC_STATE_LISTEN_ACTIVATION;                     /* Wait for listen mode to be activated */
      }
      break;


    /*******************************************************************************/
    case RFAL_NFC_STATE_LISTEN_ACTIVATION:
    case RFAL_NFC_STATE_LISTEN_SLEEP:


      err = RFal_NFC_ListenActivation();
      if (err != NFC_Busy) {
        if (err == NFC_OK) {
          gNfcDev.activeDev = gNfcDev.devList;                              /* Assign the active device to be used further on */
          gNfcDev.devCnt++;

          gNfcDev.state = RFAL_NFC_STATE_ACTIVATED;                         /* Device has been properly activated */
          RFal_NFC_NfcNotify(gNfcDev.state);                                  /* Inform upper layer that a device has been activated */
        } else if ((!timerIsExpired(gNfcDev.discTmr)) && (err == NFC_LinkLoss) && (gNfcDev.state == RFAL_NFC_STATE_LISTEN_ACTIVATION)) {
          break;                                                            /* Field|Link broken during activation, keep in Listen the remaining total duration */
        } else {
          RFal_ListenStop();
          gNfcDev.state = RFAL_NFC_STATE_START_DISCOVERY;                   /* Restart the discovery loop */
          RFal_NFC_NfcNotify(gNfcDev.state);                                  /* Notify caller             */
        }
      }
      break;
    /*******************************************************************************/
    case RFAL_NFC_STATE_ACTIVATED:
    case RFAL_NFC_STATE_POLL_SELECT:
    case RFAL_NFC_STATE_DATAEXCHANGE_DONE:
    default:
      return;
  }
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_DataExchangeStart(uint8_t *txData, uint16_t txDataLen, uint8_t **rxData, uint16_t **rvdLen, uint32_t fwt)
{
  NFC_OpResult            err;
  RFal_TransceiveContext ctx;

  /*******************************************************************************/
  /* The Data Exchange is divided in two different moments, the trigger/Start of *
   *  the transfer followed by the check until its completion                    */
  if ((gNfcDev.state >= RFAL_NFC_STATE_ACTIVATED) && (gNfcDev.activeDev != NULL)) {

    /*******************************************************************************/
    /* In Listen mode is the Poller that initiates the communicatation             */
    /* Assign output parameters and RFal_NFC_DataExchangeGetStatus will return       */
    /* incoming data from Poller/Initiator                                         */
    if ((gNfcDev.state == RFAL_NFC_STATE_ACTIVATED) && RFal_NFC_IsRemDevPoller(gNfcDev.activeDev->type)) {
      if (txDataLen > 0U) {
        return NFC_WrongState;
      }

      *rvdLen = (uint16_t *)&gNfcDev.rxLen;
      *rxData = (uint8_t *)((gNfcDev.activeDev->rfInterface == RFAL_NFC_INTERFACE_ISODEP) ? gNfcDev.rxBuf.isoDepBuf.apdu :
                            ((gNfcDev.activeDev->rfInterface == RFAL_NFC_INTERFACE_NFCDEP) ? gNfcDev.rxBuf.nfcDepBuf.pdu  : gNfcDev.rxBuf.rfBuf));
      return NFC_OK;
    }


    /*******************************************************************************/
    switch (gNfcDev.activeDev->rfInterface) {                                     /* Check which RF interface shall be used/has been activated */
      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_RF:

        RFAL_CreateByteFlagsTxRxContext(ctx, (uint8_t *)txData, txDataLen, gNfcDev.rxBuf.rfBuf, sizeof(gNfcDev.rxBuf.rfBuf), &gNfcDev.rxLen, RFAL_TXRX_FLAGS_DEFAULT, fwt);
        ctx.txBufLen = txDataLen;    /* RF interface uses number of bits */

        *rxData = (uint8_t *)gNfcDev.rxBuf.rfBuf;
        *rvdLen = (uint16_t *)&gNfcDev.rxLen;
        err = RFal_StartTransceive(&ctx);
        break;

      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_ISODEP: {
          RFal_ISO_Dep_ApduTxRxParam isoDepTxRx;

          if (txDataLen > sizeof(gNfcDev.txBuf.isoDepBuf.apdu)) {
            return NFC_MemoryError;
          }

          if (txDataLen > 0U) {
            memcpy((uint8_t *)gNfcDev.txBuf.isoDepBuf.apdu, txData, txDataLen);
          }

          isoDepTxRx.DID       = RFAL_ISODEP_NO_DID;
          isoDepTxRx.ourFSx    = RFAL_ISODEP_FSX_KEEP;
          isoDepTxRx.FSx       = gNfcDev.activeDev->proto.isoDep.info.FSx;
          isoDepTxRx.dFWT      = gNfcDev.activeDev->proto.isoDep.info.dFWT;
          isoDepTxRx.FWT       = gNfcDev.activeDev->proto.isoDep.info.FWT;
          isoDepTxRx.txBuf     = &gNfcDev.txBuf.isoDepBuf;
          isoDepTxRx.txBufLen  = txDataLen;
          isoDepTxRx.rxBuf     = &gNfcDev.rxBuf.isoDepBuf;
          isoDepTxRx.rxLen     = &gNfcDev.rxLen;
          isoDepTxRx.tmpBuf    = &gNfcDev.tmpBuf.isoDepBuf;
          *rxData              = (uint8_t *)gNfcDev.rxBuf.isoDepBuf.apdu;
          *rvdLen              = (uint16_t *)&gNfcDev.rxLen;

          /*******************************************************************************/
          /* Trigger a RFAL ISO-DEP Transceive                                           */
          err = RFal_ISO_Dep_StartApduTransceive(isoDepTxRx);
          break;
        }
        
      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_NFCDEP: {
          RFal_NFC_Dep_PduTxRxParam nfcDepTxRx;

          if (txDataLen > sizeof(gNfcDev.txBuf.nfcDepBuf.pdu)) {
            return NFC_MemoryError;
          }
          if (txDataLen > 0U) {
            memcpy((uint8_t *)gNfcDev.txBuf.nfcDepBuf.pdu, txData, txDataLen);
          }

          nfcDepTxRx.DID       = RFAL_NFCDEP_DID_KEEP;
          nfcDepTxRx.FSx       = RFal_NFC_IsRemDevListener(gNfcDev.activeDev->type) ?
                                 RFal_NFC_Dep_LR2FS((uint8_t)RFal_NFC_Dep_PP2LR(gNfcDev.activeDev->proto.nfcDep.activation.Target.ATR_RES.PPt)) :
                                 RFal_NFC_Dep_LR2FS((uint8_t)RFal_NFC_Dep_PP2LR(gNfcDev.activeDev->proto.nfcDep.activation.Initiator.ATR_REQ.PPi));
          nfcDepTxRx.dFWT      = gNfcDev.activeDev->proto.nfcDep.info.dFWT;
          nfcDepTxRx.FWT       = gNfcDev.activeDev->proto.nfcDep.info.FWT;
          nfcDepTxRx.txBuf     = &gNfcDev.txBuf.nfcDepBuf;
          nfcDepTxRx.txBufLen  = txDataLen;
          nfcDepTxRx.rxBuf     = &gNfcDev.rxBuf.nfcDepBuf;
          nfcDepTxRx.rxLen     = &gNfcDev.rxLen;
          nfcDepTxRx.tmpBuf    = &gNfcDev.tmpBuf.nfcDepBuf;
          *rxData                  = (uint8_t *)gNfcDev.rxBuf.nfcDepBuf.pdu;
          *rvdLen                 = (uint16_t *)&gNfcDev.rxLen;

          /*******************************************************************************/
          /* Trigger a RFAL NFC-DEP Transceive                                           */
          err = RFal_NFC_Dep_StartPduTransceive(nfcDepTxRx);
          break;
        }

      /*******************************************************************************/
      default:
        err = NFC_InvalidParameter;
        break;
    }

    /* If a transceive has successfully started flag Data Exchange as ongoing */
    if (err == NFC_OK) {
      gNfcDev.dataExErr = NFC_Busy;
      gNfcDev.state     = RFAL_NFC_STATE_DATAEXCHANGE;
    }

    return err;
  }

  return NFC_WrongState;
}


/*******************************************************************************/
NFC_OpResult RFal_NFC_DataExchangeGetStatus(void)
{
  /*******************************************************************************/
  /* Check if it's the first frame received in Listen mode */
  if (gNfcDev.state == RFAL_NFC_STATE_ACTIVATED) {
    /* Continue data exchange as normal */
    gNfcDev.dataExErr = NFC_Busy;
    gNfcDev.state     = RFAL_NFC_STATE_DATAEXCHANGE;

    /* Check if we performing in T3T CE */
    if ((gNfcDev.activeDev->type == RFAL_NFC_POLL_TYPE_NFCF) && (gNfcDev.activeDev->rfInterface == RFAL_NFC_INTERFACE_RF)) {
      /* The first frame has been retrieved by RFal_ListenMode, flag data immediately                  */
      /* Can only call RFal_GetTransceiveStatus() after starting a transceive with RFal_StartTransceive */
      gNfcDev.dataExErr = NFC_OK;
    }
  }


  /*******************************************************************************/
  /* Check if we are in we have been placed to sleep, and return last error     */
  if (gNfcDev.state == RFAL_NFC_STATE_LISTEN_SLEEP) {
    return gNfcDev.dataExErr;                                /* NFC_SleepRequest */
  }


  /*******************************************************************************/
  /* Check if Data exchange has been started */
  if ((gNfcDev.state != RFAL_NFC_STATE_DATAEXCHANGE) && (gNfcDev.state != RFAL_NFC_STATE_DATAEXCHANGE_DONE)) {
    return NFC_WrongState;
  }

  /* Check if Data exchange is still ongoing */
  if (gNfcDev.dataExErr == NFC_Busy) {
    switch (gNfcDev.activeDev->rfInterface) {
      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_RF:
        gNfcDev.dataExErr = RFal_GetTransceiveStatus();
        break;

      /*******************************************************************************/
      case RFAL_NFC_INTERFACE_ISODEP:
        gNfcDev.dataExErr = RFal_ISO_Dep_GetApduTransceiveStatus();
        break;
        /*******************************************************************************/
      case RFAL_NFC_INTERFACE_NFCDEP:
        gNfcDev.dataExErr = RFal_NFC_Dep_GetPduTransceiveStatus();
        break;
      /*******************************************************************************/
      default:
        gNfcDev.dataExErr = NFC_InvalidParameter;
        break;
    }

    /*******************************************************************************/
    /* If a Sleep request has been received (Listen Mode) go to sleep immediately  */
    if (gNfcDev.dataExErr == NFC_SleepRequest) {
      gNfcDev.dataExErr = RFal_ListenSleepStart(RFAL_LM_STATE_SLEEP_A, gNfcDev.rxBuf.rfBuf, sizeof(gNfcDev.rxBuf.rfBuf), &gNfcDev.rxLen);
      if(gNfcDev.dataExErr < NFC_OK)
      {
        return gNfcDev.dataExErr;
      }

      /* If set Sleep was successful keep restore the Sleep request signal */
      gNfcDev.dataExErr = NFC_SleepRequest;
    }

  }

  return gNfcDev.dataExErr;
}

NFC_OpResult RFal_NFC_PollTechDetection(void)
{
  NFC_OpResult err;

  err = NFC_OK;

  /* Suppress warning when specific RFAL features have been disabled */


  /*******************************************************************************/
  /* AP2P Technology Detection                                                   */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_AP2P) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_AP2P) != 0U)) {
    if (!gNfcDev.isTechInit) {
      err = RFal_SetMode(RFAL_MODE_POLL_ACTIVE_P2P, gNfcDev.disc.ap2pBR, gNfcDev.disc.ap2pBR);
      if(err < NFC_OK)
      {
        return err;
      }
      RFal_SetErrorHandling(ERRORHANDLING_NONE);
      RFal_SetFDTListen(RFAL_FDT_LISTEN_AP2P_POLLER);
      RFal_SetFDTPoll(RFAL_FDT_POLL_AP2P_POLLER);
      RFal_SetGT(RFAL_GT_AP2P_ADJUSTED);
      err = RFal_FieldOnAndStartGT();                                       /* Turns the Field On and starts GT timer */
      if(err < NFC_OK)
      {
        return err;
      }
      gNfcDev.isTechInit = true;
    }

    if (RFal_IsGTExpired()) {                                                             /* Wait until Guard Time is fulfilled */
      gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_AP2P;

      err = RFal_NFC_Dep_Activate(gNfcDev.devList, RFAL_NFCDEP_COMM_ACTIVE, NULL, 0);  /* Poll for NFC-A devices */
      if (err == NFC_OK) {
        gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_AP2P;

        gNfcDev.devList->type        = RFAL_NFC_LISTEN_TYPE_AP2P;
        gNfcDev.devList->rfInterface = RFAL_NFC_INTERFACE_NFCDEP;
        gNfcDev.devCnt++;

        return NFC_OK;
      }
      gNfcDev.isTechInit = false;
      RFal_FieldOff();
    }
    return NFC_Busy;
  }

  /*******************************************************************************/
  /* Turn Field On if Passive Poll technologies are enabled                      */
  /*******************************************************************************/
  if ((!gNfcDev.isFieldOn) && ((gNfcDev.disc.techs2Find & (RFAL_NFC_POLL_TECH_A | RFAL_NFC_POLL_TECH_B | RFAL_NFC_POLL_TECH_F | RFAL_NFC_POLL_TECH_V | RFAL_NFC_POLL_TECH_ST25TB | RFAL_NFC_POLL_TECH_PROP)) != 0U)) {
    err = RFal_FieldOnAndStartGT();                                /* Turns the Field On  */
    if(err < NFC_OK)
    {
      return err;
    }
    gNfcDev.isFieldOn = true;
    return NFC_Busy;
  }

  /*******************************************************************************/
  /* Passive NFC-A Technology Detection                                          */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_A) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_A) != 0U)) {
    if (!gNfcDev.isTechInit) {
      err = RFal_NFCA_PollerInit();                         /* Initialize RFAL for NFC-A */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                            /* As field is already On only starts GT timer */
      if(err < NFC_OK)
      {
        return err;
      }
      gNfcDev.isTechInit    = true;
      gNfcDev.isOperOngoing = false;                                             /* No operation currently ongoing  */
    }

    if (RFal_IsGTExpired()) {                                                       /* Wait until Guard Time is fulfilled */
      if (!gNfcDev.isOperOngoing) {
        RFal_NFCA_PollerStartTechnologyDetection(gNfcDev.disc.compMode, &gNfcDev.sensRes);  /* Poll for NFC-A devices */

        gNfcDev.isOperOngoing = true;
        return NFC_Busy;
      }

      err = RFal_NFCA_PollerGetTechnologyDetectionStatus();
      if (err != NFC_Busy) {
        if (err == NFC_OK) {
          gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_A;
        }

        gNfcDev.isTechInit = false;
        gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_A;
      }

      /* Check if bail-out after NFC-A     Activity 2.1  9.2.3.21 */
      if (((gNfcDev.disc.techs2Bail & RFAL_NFC_POLL_TECH_A) != 0U) && (gNfcDev.techsFound != 0U)) {
        return NFC_OK;
      }
    }
    return NFC_Busy;
  }


  /*******************************************************************************/
  /* Passive NFC-B Technology Detection                                          */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_B) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_B) != 0U)) {

    if (!gNfcDev.isTechInit) {
      err = RFal_NFCB_PollerInit();                        /* Initialize RFAL for NFC-B */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                           /* As field is already On only starts GT timer */
      if(err < NFC_OK)
      {
        return err;
      }
      gNfcDev.isTechInit    = true;
      gNfcDev.isOperOngoing = false;                                            /* No operation currently ongoing  */
    }

    if (RFal_IsGTExpired()) {                                                      /* Wait until Guard Time is fulfilled */

      if (!gNfcDev.isOperOngoing) {
        RFal_NFCB_PollerStartTechnologyDetection(gNfcDev.disc.compMode, &gNfcDev.sensbRes, &gNfcDev.sensbResLen);  /* Poll for NFC-B devices */

        gNfcDev.isOperOngoing = true;
        return NFC_Busy;
      }

      err = RFal_NFCB_PollerGetTechnologyDetectionStatus();
      if (err != NFC_Busy) {
        if (err == NFC_OK) {
          gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_B;
        }

        gNfcDev.isTechInit = false;
        gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_B;
      }

      /* Check if bail-out after NFC-B     Activity 2.1  9.2.3.26 */
      if (((gNfcDev.disc.techs2Bail & RFAL_NFC_POLL_TECH_B) != 0U) && (gNfcDev.techsFound != 0U)) {
        return NFC_OK;
      }
    }

    return NFC_Busy;
  }

  /*******************************************************************************/
  /* Passive NFC-F Technology Detection                                          */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_F) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_F) != 0U)) {

    if (!gNfcDev.isTechInit) {
      err = RFal_NFCF_PollerInit(gNfcDev.disc.nfcfBR);    /* Initialize RFAL for NFC-F */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                          /* As field is already On only starts GT timer */
      if(err < NFC_OK)
      {
        return err;
      }

      gNfcDev.isTechInit    = true;
      gNfcDev.isOperOngoing = false;                                           /* No operation currently ongoing  */
    }

    if (RFal_IsGTExpired()) {                                                     /* Wait until Guard Time is fulfilled */

      if (!gNfcDev.isOperOngoing) {
        RFal_NFCF_PollerStartCheckPresence();

        gNfcDev.isOperOngoing = true;
        return NFC_Busy;
      }

      err = RFal_NFCF_PollerGetCheckPresenceStatus();                            /* Poll for NFC-F devices */
      if (err != NFC_Busy) {
        if (err == NFC_OK) {
          gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_F;
        }

        gNfcDev.isTechInit = false;
        gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_F;
      }

      /* Check if bail-out after NFC-F     Activity 2.1  9.2.3.31 */
      if (((gNfcDev.disc.techs2Bail & RFAL_NFC_POLL_TECH_F) != 0U) && (gNfcDev.techsFound != 0U)) {
        return NFC_OK;
      }
    }

    return NFC_Busy;
  }


  /*******************************************************************************/
  /* Passive NFC-V Technology Detection                                          */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_V) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_V) != 0U)) {

    RFal_NFCV_InventoryRes invRes;

    if (!gNfcDev.isTechInit) {
      err = RFal_NFCV_PollerInit();                        /* Initialize RFAL for NFC-V */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                           /* As field is already On only starts GT timer */
      if(err < NFC_OK)
      {
        return err;
      }
      gNfcDev.isTechInit = true;
    }

    if (RFal_IsGTExpired()) {                                                      /* Wait until Guard Time is fulfilled */
      err = RFal_NFCV_PollerCheckPresence(&invRes);                               /* Poll for NFC-V devices */
      if (err == NFC_OK) {
        gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_V;
      }

      gNfcDev.isTechInit = false;
      gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_V;
    }

    return NFC_Busy;
  }


  /*******************************************************************************/
  /* Passive Proprietary Technology ST25TB                                       */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_ST25TB) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_ST25TB) != 0U)) {

    if (!gNfcDev.isTechInit) {
      err = RFal_ST25TB_PollerInit();                      /* Initialize RFAL for NFC-V */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                           /* As field is already On only starts GT timer */
      if(err < NFC_OK)
      {
        return err;
      }
      gNfcDev.isTechInit = true;
    }

    if (RFal_IsGTExpired()) {                                                      /* Wait until Guard Time is fulfilled */
      err = RFal_ST25TB_PollerCheckPresence(NULL);                                /* Poll for ST25TB devices */
      if (err == NFC_OK) {
        gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_ST25TB;
      }

      gNfcDev.isTechInit = false;
      gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_ST25TB;
    }

    return NFC_Busy;
  }

  /*******************************************************************************/
  /* Passive Proprietary Technology                                              */
  /*******************************************************************************/
  if (((gNfcDev.disc.techs2Find & RFAL_NFC_POLL_TECH_PROP) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_PROP) != 0U)) {
    if (!gNfcDev.isTechInit) {
      err = RFal_NFC_pCbPollerInitialize();                      /* Initialize RFAL for Proprietary NFC */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                           /* As field may already be On only starts GT timer */
      if(err < NFC_OK)
      {
        return err;
      }
      gNfcDev.isTechInit = true;
    }

    if (RFal_IsGTExpired()) {                                                      /* Wait until Guard Time is fulfilled */
      err = RFal_NFC_pCbPollerTechnologyDetection();                              /* Poll for devices */
      if (err == NFC_OK) {
        gNfcDev.techsFound |= RFAL_NFC_POLL_TECH_PROP;
      }

      gNfcDev.isTechInit = false;
      gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_PROP;
    }

    return NFC_Busy;
  }

  return NFC_OK;
}

NFC_OpResult RFal_NFC_PollCollResolution(void)
{
  uint8_t    i;
  static uint8_t    devCnt;
  NFC_OpResult err;

  err    = NFC_OK;
  //devCnt = 0;
  i      = 0;

  /* Check if device limit has been reached */
  if (gNfcDev.devCnt >= gNfcDev.disc.devLimit) {
    return NFC_OK;
  }

  /*******************************************************************************/
  /* NFC-A Collision Resolution                                                  */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_A) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_A) != 0U)) {  /* If a NFC-A device was found/detected, perform Collision Resolution */
    static RFal_NFCA_ListenDevice nfcaDevList[RFAL_NFC_MAX_DEVICES];

    if (!gNfcDev.isTechInit) {
      err = RFal_NFCA_PollerInit();                         /* Initialize RFAL for NFC-A */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                            /* Turns the Field On and starts GT timer */
      if(err < NFC_OK)
      {
        return err;
      }

      gNfcDev.isTechInit    = true;                                              /* Technology has been initialized */
      gNfcDev.isOperOngoing = false;                                             /* No operation currently ongoing  */
    }

    if (!(RFal_IsGTExpired())) {
      return NFC_Busy;
    }

    if (!gNfcDev.isOperOngoing) {
      err = RFal_NFCA_PollerStartFullCollisionResolution(gNfcDev.disc.compMode, (gNfcDev.disc.devLimit - gNfcDev.devCnt), nfcaDevList, &devCnt);
      if(err < NFC_OK)
      {
        return err;
      }

      gNfcDev.isOperOngoing = true;
      return NFC_Busy;
    }

    err = RFal_NFCA_PollerGetFullCollisionResolutionStatus();
    if (err != NFC_Busy) {
      gNfcDev.isTechInit = false;
      gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_A;

      if ((err == NFC_OK) && (devCnt != 0U)) {
        for (i = 0; i < devCnt; i++) {                                            /* Copy devices found form local Nfca list into global device list */
          gNfcDev.devList[gNfcDev.devCnt].type     = RFAL_NFC_LISTEN_TYPE_NFCA;
          gNfcDev.devList[gNfcDev.devCnt].dev.nfca = nfcaDevList[i];
          gNfcDev.devCnt++;
        }
      }
    }

    return NFC_Busy;
  }

  /*******************************************************************************/
  /* NFC-B Collision Resolution                                                  */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_B) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_B) != 0U)) {  /* If a NFC-B device was found/detected, perform Collision Resolution */
    static RFal_NFCB_ListenDevice nfcbDevList[RFAL_NFC_MAX_DEVICES];

    if (!gNfcDev.isTechInit) {
      err = RFal_NFCB_PollerInit();                         /* Initialize RFAL for NFC-B */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                            /* Ensure GT again as other technologies have also been polled */
      if(err < NFC_OK)
      {
        return err;
      }

      gNfcDev.isTechInit    = true;
      gNfcDev.isOperOngoing = false;                                             /* No operation currently ongoing  */
    }

    if (!(RFal_IsGTExpired())) {
      return NFC_Busy;
    }

    if (!gNfcDev.isOperOngoing) {
      err = RFal_NFCB_PollerStartCollisionResolution(gNfcDev.disc.compMode, (gNfcDev.disc.devLimit - gNfcDev.devCnt), nfcbDevList, &devCnt);
      if(err < NFC_OK)
      {
        return err;
      }

      gNfcDev.isOperOngoing = true;
      return NFC_Busy;
    }


    err = RFal_NFCB_PollerGetCollisionResolutionStatus();
    if (err != NFC_Busy) {
      gNfcDev.isTechInit = false;
      gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_B;

      if ((err == NFC_OK) && (devCnt != 0U)) {
        for (i = 0; i < devCnt; i++) {                                            /* Copy devices found form local Nfcb list into global device list */
          gNfcDev.devList[gNfcDev.devCnt].type     = RFAL_NFC_LISTEN_TYPE_NFCB;
          gNfcDev.devList[gNfcDev.devCnt].dev.nfcb = nfcbDevList[i];
          gNfcDev.devCnt++;
        }
      }
    }

    return NFC_Busy;
  }

  /*******************************************************************************/
  /* NFC-F Collision Resolution                                                  */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_F) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_F) != 0U)) { /* If a NFC-F device was found/detected, perform Collision Resolution */
    static RFal_NFCF_ListenDevice nfcfDevList[RFAL_NFC_MAX_DEVICES];

    if (!gNfcDev.isTechInit) {
      err = RFal_NFCF_PollerInit(gNfcDev.disc.nfcfBR);      /* Initialize RFAL for NFC-F */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                            /* Ensure GT again as other technologies have also been polled */
      if(err < NFC_OK)
      {
        return err;
      }

      gNfcDev.isTechInit    = true;
      gNfcDev.isOperOngoing = false;                                             /* No operation currently ongoing  */
    }

    if (!(RFal_IsGTExpired())) {
      return NFC_Busy;
    }

    if (!gNfcDev.isOperOngoing) {
      err = RFal_NFCF_PollerStartCollisionResolution(gNfcDev.disc.compMode, (gNfcDev.disc.devLimit - gNfcDev.devCnt), nfcfDevList, &devCnt);
      if(err < NFC_OK)
      {
        return err;
      }

      gNfcDev.isOperOngoing = true;
      return NFC_Busy;
    }

    err = RFal_NFCF_PollerGetCollisionResolutionStatus();
    if (err != NFC_Busy) {
      gNfcDev.isTechInit = false;
      gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_F;

      if ((err == NFC_OK) && (devCnt != 0U)) {
        for (i = 0; i < devCnt; i++) {                                         /* Copy devices found form local Nfcf list into global device list */
          gNfcDev.devList[gNfcDev.devCnt].type     = RFAL_NFC_LISTEN_TYPE_NFCF;
          gNfcDev.devList[gNfcDev.devCnt].dev.nfcf = nfcfDevList[i];
          gNfcDev.devCnt++;
        }
      }
    }

    return NFC_Busy;
  }

  /*******************************************************************************/
  /* NFC-V Collision Resolution                                                  */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_V) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_V) != 0U)) { /* If a NFC-V device was found/detected, perform Collision Resolution */
    RFal_NFCV_ListenDevice nfcvDevList[RFAL_NFC_MAX_DEVICES];

    if (!gNfcDev.isTechInit) {
      err = RFal_NFCV_PollerInit();                        /* Initialize RFAL for NFC-V */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                           /* Ensure GT again as other technologies have also been polled */
      if(err < NFC_OK)
      {
        return err;
      }
      gNfcDev.isTechInit = true;
    }

    if (!(RFal_IsGTExpired())) {
      return NFC_Busy;
    }

    devCnt             = 0;
    gNfcDev.isTechInit = false;
    gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_V;

    err = RFal_NFCV_PollerCollisionResolution(RFAL_COMPLIANCE_MODE_NFC, (gNfcDev.disc.devLimit - gNfcDev.devCnt), nfcvDevList, &devCnt);
    if ((err == NFC_OK) && (devCnt != 0U)) {
      for (i = 0; i < devCnt; i++) {                                            /* Copy devices found form local Nfcf list into global device list */
        gNfcDev.devList[gNfcDev.devCnt].type     = RFAL_NFC_LISTEN_TYPE_NFCV;
        gNfcDev.devList[gNfcDev.devCnt].dev.nfcv = nfcvDevList[i];
        gNfcDev.devCnt++;
      }
    }

    return NFC_Busy;
  }

  /*******************************************************************************/
  /* ST25TB Collision Resolution                                                 */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_ST25TB) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_ST25TB) != 0U)) { /* If a ST25TB device was found/detected, perform Collision Resolution */
    RFal_ST25TB_ListenDevice st25tbDevList[RFAL_NFC_MAX_DEVICES];

    if (!gNfcDev.isTechInit) {
      err = RFal_ST25TB_PollerInit();                      /* Initialize RFAL for ST25TB */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                           /* Ensure GT again as other technologies have also been polled */
      if(err < NFC_OK)
      {
        return err;
      }
      gNfcDev.isTechInit = true;
    }

    if (!(RFal_IsGTExpired())) {
      return NFC_Busy;
    }

    devCnt             = 0;
    gNfcDev.isTechInit = false;
    gNfcDev.techs2do &= ~RFAL_NFC_POLL_TECH_ST25TB;

    err = RFal_ST25TB_PollerCollisionResolution((gNfcDev.disc.devLimit - gNfcDev.devCnt), st25tbDevList, &devCnt);
    if ((err == NFC_OK) && (devCnt != 0U)) {
      for (i = 0; i < devCnt; i++) {                                            /* Copy devices found form local Nfcf list into global device list */
        gNfcDev.devList[gNfcDev.devCnt].type       = RFAL_NFC_LISTEN_TYPE_ST25TB;
        gNfcDev.devList[gNfcDev.devCnt].dev.st25tb = st25tbDevList[i];
        gNfcDev.devCnt++;
      }
    }

    return NFC_Busy;
  }

  /*******************************************************************************/
  /* Proprietary NFC Collision Resolution                                        */
  /*******************************************************************************/
  if (((gNfcDev.techsFound & RFAL_NFC_POLL_TECH_PROP) != 0U) && ((gNfcDev.techs2do & RFAL_NFC_POLL_TECH_PROP) != 0U)) {  /* If a device was found/detected, perform Collision Resolution */
    if (!gNfcDev.isTechInit) {
      err = RFal_NFC_pCbPollerInitialize();                       /* Initialize RFAL for Proprietary NFC */
      if(err < NFC_OK)
      {
        return err;
      }
      err = RFal_FieldOnAndStartGT();                            /* Turns the Field On and starts GT timer */
      if(err < NFC_OK)
      {
        return err;
      }

      gNfcDev.isTechInit    = true;                                              /* Technology has been initialized */
      gNfcDev.isOperOngoing = false;                                             /* No operation currently ongoing  */
    }

    if (!(RFal_IsGTExpired())) {
      return NFC_Busy;
    }

    if (!gNfcDev.isOperOngoing) {
      err = RFal_NFC_pCbPollerStartCollisionResolution();
      if(err < NFC_OK)
      {
        return err;
      }

      gNfcDev.isOperOngoing = true;
      return NFC_Busy;
    }

    err = RFal_NFC_pCbPollerGetCollisionResolutionStatus();
    if (err != NFC_Busy) {
      gNfcDev.isTechInit = false;
      gNfcDev.techs2do  &= ~RFAL_NFC_POLL_TECH_PROP;

      if (err == NFC_OK) {
        gNfcDev.devCnt = 1;                                                   /* Device list held by caller */
        gNfcDev.devList[0].type = RFAL_NFC_LISTEN_TYPE_PROP;
      }
    }
    return NFC_Busy;
  }

  return NFC_OK;                                                                  /* All technologies have been performed */
}

NFC_OpResult RFal_NFC_PollActivation(uint8_t devIt)
{
  NFC_OpResult err;
  uint8_t                     devIdx;
  RFal_NFCA_ListenDeviceType    nfcaType;

  err = NFC_OK;
  devIdx   = 0;
  nfcaType = RFAL_NFCA_T1T;

  if (devIt > gNfcDev.devCnt) {
    return NFC_WrongState;
  }

  switch (gNfcDev.devList[devIt].type) {
      /*******************************************************************************/
      /* AP2P Activation                                                             */
      /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_AP2P:
      /* Activation has already been performed (ATR_REQ) */

      gNfcDev.devList[devIt].nfcid     = gNfcDev.devList[devIt].proto.nfcDep.activation.Target.ATR_RES.NFCID3;
      gNfcDev.devList[devIt].nfcidLen  = RFAL_NFCDEP_NFCID3_LEN;
      break;

      /*******************************************************************************/
      /* Passive NFC-A Activation                                                    */
      /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_NFCA:

      if (!gNfcDev.isTechInit) {
        RFal_NFCA_PollerInit();
        gNfcDev.isTechInit = true;
        gNfcDev.isOperOngoing = false;
        return NFC_Busy;
      }

      if (gNfcDev.devList[devIt].dev.nfca.isSleep) { /* Check if desired device is in Sleep */
        if (!gNfcDev.isOperOngoing) {
          /* Wake up all cards  */
          err = RFal_NFCA_PollerCheckPresence(RFAL_14443A_SHORTFRAME_CMD_WUPA, &gNfcDev.sensRes);
          if(err < NFC_OK)
          {
            return err;
          }

          /* Select specific device */
          err = RFal_NFCA_PollerStartSelect(gNfcDev.devList[devIt].dev.nfca.nfcId1, gNfcDev.devList[devIt].dev.nfca.nfcId1Len, &gNfcDev.devList[devIt].dev.nfca.selRes);
          if(err < NFC_OK)
          {
            return err;
          }

          gNfcDev.isOperOngoing = true;
        } else {
          err = RFal_NFCA_PollerGetSelectStatus();
          if(err < NFC_OK)
          {
            return err;
          }

          /* In case multiple NFC-A devices are present, when activating/waking a device that
            is sleeping (not the last one) will make the active one to go back to IDLE.
            Marking it as in sleep (Activity 2.2  9.4.4 Optional Symbol 2) will ensure that
            gets correctly activated afterwards                                              */
          for (devIdx = 0; devIdx < gNfcDev.devCnt; devIdx++) {
            if (gNfcDev.devList[devIdx].type == RFAL_NFC_LISTEN_TYPE_NFCA) {
              gNfcDev.devList[devIdx].dev.nfca.isSleep = true;
            }
          }

          gNfcDev.devList[devIt].dev.nfca.isSleep = false;
          gNfcDev.isOperOngoing = false;
        }
        return NFC_Busy;
      }

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfca.nfcId1;
      gNfcDev.devList[devIt].nfcidLen = gNfcDev.devList[devIt].dev.nfca.nfcId1Len;

      /* If device supports multiple technologies assign protocol requested */
      nfcaType = gNfcDev.devList[devIt].dev.nfca.type;
      if (nfcaType == RFAL_NFCA_T4T_NFCDEP) {
        nfcaType = ((gNfcDev.disc.p2pNfcaPrio) ? RFAL_NFCA_NFCDEP : RFAL_NFCA_T4T);
      }

      /*******************************************************************************/
      /* Perform protocol specific activation                                        */
      switch (nfcaType) {
        /*******************************************************************************/
        case RFAL_NFCA_T1T:

          /* No further activation needed for T1T (RID already performed) */

          gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfca.ridRes.uid;
          gNfcDev.devList[devIt].nfcidLen = RFAL_T1T_UID_LEN;

          gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;
          break;

        case RFAL_NFCA_T2T:

          /* No further activation needed for a T2T */

          gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;
          break;


        /*******************************************************************************/
        case RFAL_NFCA_T4T:                                                   /* Device supports ISO-DEP */

          if (!gNfcDev.isOperOngoing) {
            /* Perform ISO-DEP (ISO14443-4) activation: RATS and PPS if supported */
            RFal_ISO_Dep_InitWithParams(gNfcDev.disc.compMode, RFAL_ISODEP_MAX_R_RETRYS, RFAL_ISODEP_MAX_WTX_NACK_RETRYS, RFAL_ISODEP_MAX_WTX_RETRYS, RFAL_ISODEP_MAX_DSL_RETRYS, RFAL_ISODEP_MAX_I_RETRYS, RFAL_ISODEP_RATS_RETRIES);
            err = RFal_ISO_Dep_PollAStartActivation(gNfcDev.disc.isoDepFS, RFAL_ISODEP_NO_DID, gNfcDev.disc.maxBR, &gNfcDev.devList[devIt].proto.isoDep);
            if(err < NFC_OK)
            {
              return err;
            }

            gNfcDev.isOperOngoing = true;
            return NFC_Busy;
          }

          err = RFal_ISO_Dep_PollAGetActivationStatus();
          if (err != NFC_OK) {
            return err;
          }

          gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_ISODEP; /* NFC-A T4T device activated */
          //gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF; /* No ISO-DEP supported activate using RF interface */
          break;

        /*******************************************************************************/
        case RFAL_NFCA_NFCDEP:                                                /* Device supports NFC-DEP */

          /* Perform NFC-DEP (P2P) activation: ATR and PSL if supported */
          err = RFal_NFC_Dep_Activate(&gNfcDev.devList[devIt], RFAL_NFCDEP_COMM_PASSIVE, NULL, 0);
          if(err < NFC_OK)
          {
            return err;
          }

          gNfcDev.devList[devIt].nfcid = gNfcDev.devList[devIt].proto.nfcDep.activation.Target.ATR_RES.NFCID3;
          gNfcDev.devList[devIt].nfcidLen = RFAL_NFCDEP_NFCID3_LEN;
          gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_NFCDEP; /* NFC-A P2P device activated */
          //gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;       /* No NFC-DEP supported activate using RF interface */
          break;

        /*******************************************************************************/
        case RFAL_NFCA_T4T_NFCDEP:
        default:
          return NFC_WrongState;
      }
      break;

      /*******************************************************************************/
      /* Passive NFC-B Activation                                                    */
      /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_NFCB:

      if (!gNfcDev.isTechInit) {
        RFal_NFCB_PollerInit();
        gNfcDev.isTechInit = true;
        gNfcDev.isOperOngoing = false;

        if (gNfcDev.devList[devIt].dev.nfcb.isSleep) {
          /* Check if desired device is in Sleep */
          /* Wake up all cards. SENSB_RES may return collision but the NFCID0 is available to explicitly select NFC-B card via ATTRIB; so error will be ignored here */
          RFal_NFCB_PollerStartCheckPresence(RFAL_NFCB_SENS_CMD_ALLB_REQ, RFAL_NFCB_SLOT_NUM_1, &gNfcDev.sensbRes, &gNfcDev.sensbResLen);
        }

        return NFC_Busy;
      }

      if (gNfcDev.devList[devIt].dev.nfcb.isSleep) { /* Check if desired device is still in Sleep */
        /* Wake up all cards. SENSB_RES may return collision but the NFCID0 is available to explicitly select NFC-B card via ATTRIB; so error will be ignored here */
        err = RFal_NFCB_PollerGetCheckPresenceStatus();
        if(err == NFC_Busy)
        {
            return NFC_Busy;
        }

        gNfcDev.devList[devIt].dev.nfcb.isSleep = false;
      }

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfcb.sensbRes.nfcid0;
      gNfcDev.devList[devIt].nfcidLen = RFAL_NFCB_NFCID0_LEN;

      /* Check if device supports  ISO-DEP (ISO14443-4) */
      if ((gNfcDev.devList[devIt].dev.nfcb.sensbRes.protInfo.FsciProType & RFAL_NFCB_SENSB_RES_PROTO_ISO_MASK) != 0U) {
        if (!gNfcDev.isOperOngoing) {
          RFal_ISO_Dep_InitWithParams(gNfcDev.disc.compMode, RFAL_ISODEP_MAX_R_RETRYS, RFAL_ISODEP_MAX_WTX_NACK_RETRYS, RFAL_ISODEP_MAX_WTX_RETRYS, RFAL_ISODEP_MAX_DSL_RETRYS, RFAL_ISODEP_MAX_I_RETRYS, RFAL_ISODEP_RATS_RETRIES);
          /* Perform ISO-DEP (ISO14443-4) activation: ATTRIB    */
          err = RFal_ISO_Dep_PollBStartActivation(gNfcDev.disc.isoDepFS, RFAL_ISODEP_NO_DID, gNfcDev.disc.maxBR, 0x00, &gNfcDev.devList[devIt].dev.nfcb, NULL, 0, &gNfcDev.devList[devIt].proto.isoDep);
          if(err < NFC_OK)
          {
            return err;
          }

          gNfcDev.isOperOngoing = true;
          return NFC_Busy;
        }

        err = RFal_ISO_Dep_PollBGetActivationStatus();
        if (err != NFC_OK) {
          return err;
        }

        gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_ISODEP; /* NFC-B T4T device activated */
        break;
      }

      gNfcDev.devList[devIt].rfInterface =  RFAL_NFC_INTERFACE_RF;              /* NFC-B device activated     */
      break;

      /*******************************************************************************/
      /* Passive NFC-F Activation                                                    */
      /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_NFCF:

      RFal_NFCF_PollerInit(gNfcDev.disc.nfcfBR);

      if (RFal_NFCF_IsNfcDepSupported(&gNfcDev.devList[devIt].dev.nfcf)) {
        /* Perform NFC-DEP (P2P) activation: ATR and PSL if supported */
        err = RFal_NFC_Dep_Activate(&gNfcDev.devList[devIt], RFAL_NFCDEP_COMM_PASSIVE, NULL, 0);
        if(err < NFC_OK)
        {
          return err;
        }

        /* Set NFCID */
        gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].proto.nfcDep.activation.Target.ATR_RES.NFCID3;
        gNfcDev.devList[devIt].nfcidLen = RFAL_NFCDEP_NFCID3_LEN;

        gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_NFCDEP;       /* NFC-F P2P device activated */
        break;
      }

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfcf.sensfRes.NFCID2;
      gNfcDev.devList[devIt].nfcidLen = RFAL_NFCF_NFCID2_LEN;

      gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;               /* NFC-F T3T device activated */
      break;

      /*******************************************************************************/
      /* Passive NFC-V Activation                                                    */
      /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_NFCV:

      RFal_NFCV_PollerInit();

      /* No specific activation needed for a T5T */

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.nfcv.InvRes.UID;
      gNfcDev.devList[devIt].nfcidLen = RFAL_NFCV_UID_LEN;

      gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;               /* NFC-V T5T device activated */
      break;

      /*******************************************************************************/
      /* Passive ST25TB Activation                                                   */
      /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_ST25TB:

      RFal_ST25TB_PollerInit();

      /* No specific activation needed for a ST25TB */

      /* Set NFCID */
      gNfcDev.devList[devIt].nfcid    = gNfcDev.devList[devIt].dev.st25tb.UID;
      gNfcDev.devList[devIt].nfcidLen = RFAL_ST25TB_UID_LEN;

      gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;               /* ST25TB device activated */
      break;

    /*******************************************************************************/
    /* Passive Proprietary NFC Activation                                          */
    /*******************************************************************************/
    case RFAL_NFC_LISTEN_TYPE_PROP:

      if (!gNfcDev.isTechInit) {
        err = RFal_NFC_pCbPollerInitialize();
        if(err < NFC_OK)
        {
          return err;
        }
        gNfcDev.isTechInit = true;
        gNfcDev.isOperOngoing = false;
        return NFC_Busy;
      }

      if (!gNfcDev.isOperOngoing) {
        /* Start activation  */
        err = RFal_NFC_pCbStartActivation();
        if(err < NFC_OK)
        {
          return err;
        }

        gNfcDev.isOperOngoing = true;
        return NFC_Busy;
      }

      err = RFal_NFC_pCbGetActivationStatus();
      if (err != NFC_OK) {
        return err;
      }

      /* Clear NFCID */
      gNfcDev.devList[devIt].nfcid = NULL;
      gNfcDev.devList[devIt].nfcidLen = 0;

      gNfcDev.devList[devIt].rfInterface = RFAL_NFC_INTERFACE_RF;
      break;

    /*******************************************************************************/
    default:
      return NFC_WrongState;
  }

  gNfcDev.activeDev = &gNfcDev.devList[devIt];                                      /* Assign active device to be used further on */
  gNfcDev.isOperOngoing = false;
  return NFC_OK;
}

NFC_OpResult RFal_NFC_ListenActivation(void)
{
  bool isDataRcvd;
  volatile NFC_OpResult ret;
  RFal_LmState lmSt;
  RFal_BitRate bitRate;
  uint8_t hdrLen;

  /* Set the header length in NFC-A */
  hdrLen = (RFAL_NFCDEP_SB_LEN + RFAL_NFCDEP_LEN_LEN);

  lmSt = RFal_ListenGetState(&isDataRcvd, &bitRate);
  switch (lmSt) {

    /*******************************************************************************/
    case RFAL_LM_STATE_ACTIVE_A: /* NFC-A CE activation */
    case RFAL_LM_STATE_ACTIVE_Ax:

      if (isDataRcvd) { /* Check if Reader/Initiator has sent some data */
        /* Check if received data is a Sleep request */
        if (RFal_NFCA_ListenerIsSleepReq(gNfcDev.rxBuf.rfBuf, RFal_ConvBitsToBytes(gNfcDev.rxLen))) { /* Check if received data is a SLP_REQ */
          /* Set the Listen Mode in Sleep state */
          ret = RFal_ListenSleepStart(RFAL_LM_STATE_SLEEP_A, gNfcDev.rxBuf.rfBuf, sizeof(gNfcDev.rxBuf.rfBuf), &gNfcDev.rxLen); 
          if(ret < NFC_OK)
          {
            return ret;
          }
        }

        /* Check if received data is a valid RATS */
        else if (RFal_ISO_Dep_IsRats(gNfcDev.rxBuf.rfBuf, (uint8_t)RFal_ConvBitsToBytes(gNfcDev.rxLen))) {
          RFal_ISO_Dep_AtsParam atsParam;
          RFal_ISO_Dep_ListenActvParam rxParam;

          /* Set ATS parameters */
          atsParam.fsci = (uint8_t)RFAL_ISODEP_DEFAULT_FSCI;
          atsParam.fwi = RFAL_ISODEP_DEFAULT_FWI;
          atsParam.sfgi = RFAL_ISODEP_DEFAULT_SFGI;
          atsParam.didSupport = false;
          atsParam.ta = RFAL_ISODEP_ATS_TA_SAME_D;
          atsParam.hb = NULL;
          atsParam.hbLen = 0;

          /* Set Rx parameters */
          rxParam.rxBuf = (RFal_ISO_Dep_BufFormat *)&gNfcDev.rxBuf.isoDepBuf; /*  PRQA S 0310 # MISRA 11.3 - Intentional safe cast to avoiding large buffer duplication */
          rxParam.rxLen = &gNfcDev.rxLen;
          rxParam.isoDepDev = &gNfcDev.devList->proto.isoDep;
          rxParam.isRxChaining = &gNfcDev.isRxChaining;

          RFal_ListenSetState(RFAL_LM_STATE_CARDEMU_4A); /* Set next state CE T4T */
          RFal_ISO_Dep_Init();                       /* Initialize ISO-DEP layer to handle ISO14443-a activation / RATS */

          /* Set ISO-DEP layer to digest RATS and handle activation */
          ret = RFal_ISO_Dep_ListenStartActivation(&atsParam, NULL, gNfcDev.rxBuf.rfBuf, gNfcDev.rxLen, rxParam);
          if(ret < NFC_OK)
          {
              return ret;
          }
        }

        /* Check if received data is a valid ATR_REQ */
        else if (RFal_NFC_Dep_IsAtrReq(&gNfcDev.rxBuf.rfBuf[hdrLen], (RFal_ConvBitsToBytes(gNfcDev.rxLen) - hdrLen), gNfcDev.devList->nfcid)) {
          gNfcDev.devList->type = RFAL_NFC_POLL_TYPE_NFCA;
          ret = RFal_NFC_Dep_Activate(gNfcDev.devList, RFAL_NFCDEP_COMM_PASSIVE, &gNfcDev.rxBuf.rfBuf[hdrLen], (RFal_ConvBitsToBytes(gNfcDev.rxLen) - hdrLen));
          if(ret < NFC_OK)
          {
              return ret;
          }
        }

        else {
          return NFC_ProtocolError;
        }
      }
      return NFC_Busy;

    /*******************************************************************************/
    case RFAL_LM_STATE_CARDEMU_4A: /* T4T ISO-DEP activation */

      ret = RFal_ISO_Dep_ListenGetActivationStatus();
      if (ret == NFC_OK) {
        gNfcDev.devList->type = RFAL_NFC_POLL_TYPE_NFCA;
        gNfcDev.devList->rfInterface = RFAL_NFC_INTERFACE_ISODEP;
        gNfcDev.devList->nfcid = NULL;
        gNfcDev.devList->nfcidLen = 0;
      }
      return ((ret == NFC_LinkLoss) ? NFC_ProtocolError : ret); /* Link loss during protocol activation, reMap error */

    /*******************************************************************************/
    case RFAL_LM_STATE_READY_F: /* NFC-F CE activation */

      if (isDataRcvd) { /* Wait for the first received data */
        /* Set the header length in NFC-F */
        hdrLen = RFAL_NFCDEP_LEN_LEN;

        if (RFal_NFC_Dep_IsAtrReq(&gNfcDev.rxBuf.rfBuf[hdrLen], (RFal_ConvBitsToBytes(gNfcDev.rxLen) - hdrLen), gNfcDev.devList->nfcid)) {
          gNfcDev.devList->type = RFAL_NFC_POLL_TYPE_NFCF;
          ret = RFal_NFC_Dep_Activate(gNfcDev.devList, RFAL_NFCDEP_COMM_PASSIVE, &gNfcDev.rxBuf.rfBuf[hdrLen], (RFal_ConvBitsToBytes(gNfcDev.rxLen) - hdrLen));
          if(ret < NFC_OK)
          {
              return ret;
          }
        } else
        {
          RFal_ListenSetState(RFAL_LM_STATE_CARDEMU_3); /* First data already received - set T3T CE */
        }
      }
      return NFC_Busy;

    /*******************************************************************************/
    case RFAL_LM_STATE_CARDEMU_3: /* T3T activated */

      gNfcDev.devList->type = RFAL_NFC_POLL_TYPE_NFCF;
      gNfcDev.devList->rfInterface = RFAL_NFC_INTERFACE_RF;
      gNfcDev.devList->nfcid = NULL;
      gNfcDev.devList->nfcidLen = 0;

      return NFC_OK;

    /*******************************************************************************/
    case RFAL_LM_STATE_TARGET_A: /* NFC-DEP activation */
    case RFAL_LM_STATE_TARGET_F:

      ret = RFal_NFC_Dep_ListenGetActivationStatus();
      if (ret == NFC_OK) {
        gNfcDev.devList->rfInterface = RFAL_NFC_INTERFACE_NFCDEP;
        gNfcDev.devList->nfcid = gNfcDev.devList->proto.nfcDep.activation.Initiator.ATR_REQ.NFCID3;
        gNfcDev.devList->nfcidLen = RFAL_NFCDEP_NFCID3_LEN;
      }
      return ret;

    /*******************************************************************************/
    case RFAL_LM_STATE_IDLE: /* AP2P activation */
      if (isDataRcvd) {      /* Check if Reader/Initiator has sent some data */
        if ((gNfcDev.lmMask & RFAL_LM_MASK_ACTIVE_P2P) != 0U) { /* Check if AP2P is enabled */

          /* Calculate the header length in NFC-A or NFC-F mode*/
          hdrLen = ((bitRate == RFAL_BR_106) ? (RFAL_NFCDEP_SB_LEN + RFAL_NFCDEP_LEN_LEN) : RFAL_NFCDEP_LEN_LEN);

          if (RFal_NFC_Dep_IsAtrReq(&gNfcDev.rxBuf.rfBuf[hdrLen], (RFal_ConvBitsToBytes(gNfcDev.rxLen) - hdrLen), NULL)) {
            gNfcDev.devList->type = RFAL_NFC_POLL_TYPE_AP2P;
            RFal_SetMode((RFAL_MODE_LISTEN_ACTIVE_P2P), bitRate, bitRate);
            RFal_SetFDTListen(RFAL_FDT_LISTEN_AP2P_LISTENER);
            ret = RFal_NFC_Dep_Activate(gNfcDev.devList, RFAL_NFCDEP_COMM_ACTIVE, &gNfcDev.rxBuf.rfBuf[hdrLen], (RFal_ConvBitsToBytes(gNfcDev.rxLen) - hdrLen));
            if(ret < NFC_OK)
            {
              return ret;
            }
          } else
          {
            return NFC_ProtocolError;
          }
        }
      }
      return NFC_Busy;

    /*******************************************************************************/
    case RFAL_LM_STATE_READY_A:
    case RFAL_LM_STATE_READY_Ax:
    case RFAL_LM_STATE_SLEEP_A:
    case RFAL_LM_STATE_SLEEP_AF:
      return NFC_Busy;

    /*******************************************************************************/
    case RFAL_LM_STATE_POWER_OFF:
      return NFC_LinkLoss;

    default: /* Wait for activation */
      break;
  }

  return NFC_InternalError;
}

NFC_OpResult RFal_NFC_Dep_Activate(RFal_NFC_Device *device, RFal_NFC_Dep_CommMode commMode, const uint8_t *atrReq, uint16_t atrReqLen)
{
  RFal_NFC_Dep_AtrParam          initParam;

  /* Suppress warnings if Listen mode is disabled */
  
  /* If we are in Poll mode */
  if (gNfcDev.state < RFAL_NFC_STATE_LISTEN_TECHDETECT) {
    /*******************************************************************************/
    /* If Passive F use the NFCID2 retrieved from SENSF                            */
    if (device->type == RFAL_NFC_LISTEN_TYPE_NFCF) {
      initParam.nfcid    = device->dev.nfcf.sensfRes.NFCID2;
      initParam.nfcidLen = RFAL_NFCF_NFCID2_LEN;
    } else {
      initParam.nfcid    = gNfcDev.disc.nfcid3;
      initParam.nfcidLen = RFAL_NFCDEP_NFCID3_LEN;
    }

    initParam.BS        = RFAL_NFCDEP_Bx_NO_HIGH_BR;
    initParam.BR        = RFAL_NFCDEP_Bx_NO_HIGH_BR;
    initParam.DID       = RFAL_NFCDEP_DID_NO;
    initParam.NAD       = RFAL_NFCDEP_NAD_NO;
    initParam.LR        = gNfcDev.disc.nfcDepLR;
    initParam.GB        = gNfcDev.disc.GB;
    initParam.GBLen     = gNfcDev.disc.GBLen;
    initParam.commMode  = commMode;
    initParam.operParam = (RFAL_NFCDEP_OPER_FULL_MI_EN | RFAL_NFCDEP_OPER_EMPTY_DEP_DIS | RFAL_NFCDEP_OPER_ATN_EN | RFAL_NFCDEP_OPER_RTOX_REQ_EN);

    RFal_NFC_Dep_Init();
    /* Perform NFC-DEP (P2P) activation: ATR and PSL if supported */
    return RFal_NFC_Dep_InitiatorHandleActivation(&initParam, gNfcDev.disc.maxBR, &device->proto.nfcDep);
  }
  /* If we are in Listen mode */
  else if (RFal_NFC_IsRemDevPoller(device->type) && (gNfcDev.state >= RFAL_NFC_STATE_LISTEN_TECHDETECT)) {
    RFal_NFC_Dep_ListenActvParam   actvParams;
    RFal_NFC_Dep_TargetParam       targetParam;

    memcpy(targetParam.nfcid3, (uint8_t *)gNfcDev.disc.nfcid3, RFAL_NFCDEP_NFCID3_LEN);
    targetParam.bst       = RFAL_NFCDEP_Bx_NO_HIGH_BR;
    targetParam.brt       = RFAL_NFCDEP_Bx_NO_HIGH_BR;
    targetParam.to        = RFAL_NFCDEP_WT_TRG_MAX_L13; /* [LLCP] 1.3 6.2.1 */
    targetParam.ppt       = RFal_NFC_Dep_LR2PP(gNfcDev.disc.nfcDepLR);
    if (gNfcDev.disc.GBLen >= RFAL_NFCDEP_GB_MAX_LEN) {
      return NFC_InvalidParameter;
    }
    targetParam.GBtLen    = gNfcDev.disc.GBLen;
    if (gNfcDev.disc.GBLen > 0U) {
      memcpy(targetParam.GBt, gNfcDev.disc.GB, gNfcDev.disc.GBLen);
    }
    targetParam.operParam = (RFAL_NFCDEP_OPER_FULL_MI_EN | RFAL_NFCDEP_OPER_EMPTY_DEP_DIS | RFAL_NFCDEP_OPER_ATN_EN | RFAL_NFCDEP_OPER_RTOX_REQ_EN);
    targetParam.commMode  = commMode;


    /* Set activation buffer (including header) for NFC-DEP */
    actvParams.rxBuf        = (RFal_NFC_Dep_BufFormat *) &gNfcDev.rxBuf.nfcDepBuf;  /*  PRQA S 0310 # MISRA 11.3 - Intentional safe cast to avoiding large buffer duplication */
    actvParams.rxLen        = &gNfcDev.rxLen;
    actvParams.isRxChaining = &gNfcDev.isRxChaining;
    actvParams.nfcDepDev    = &gNfcDev.devList->proto.nfcDep;

    RFal_ListenSetState(((device->type == RFAL_NFC_POLL_TYPE_NFCA) ? RFAL_LM_STATE_TARGET_A : RFAL_LM_STATE_TARGET_F));

    RFal_NFC_Dep_Init();
    /* Perform NFC-DEP (P2P) activation: send ATR_RES and handle activation */
    return RFal_NFC_Dep_ListenStartActivation(&targetParam, atrReq, atrReqLen, actvParams);
  }
  else {
    return NFC_InternalError;
  }
}


NFC_OpResult RFal_NFC_DeActivation(void)
{
  bool       aux;
  NFC_OpResult ret;

  ret = NFC_OK;
  aux = false;

  /* Check if a device has been activated */
  if (gNfcDev.activeDev != NULL) {
    if (RFal_NFC_IsRemDevListener(gNfcDev.activeDev->type)) {
      switch (gNfcDev.activeDev->rfInterface) {
        /*******************************************************************************/
        case RFAL_NFC_INTERFACE_RF:
          break;                                                                /* No specific deactivation to be performed */

          /*******************************************************************************/
        case RFAL_NFC_INTERFACE_ISODEP:
          if (!gNfcDev.isOperOngoing) {
            ret = RFal_ISO_Dep_StartDeselect();
            if (ret == NFC_OK) { /* Send a Deselect to device */
              gNfcDev.isOperOngoing = true;
              return NFC_Busy;
            }
          } else {
            ret = RFal_ISO_Dep_GetDeselectStatus(); /* Check if deselection has finished */
            if(ret == NFC_Busy)
            {
                return NFC_Busy;
            }

            aux = true; /* Mark device as deselected */
            gNfcDev.isOperOngoing = false;
          } /* Send a Deselect to device */
          break;
          /*******************************************************************************/
        case RFAL_NFC_INTERFACE_NFCDEP:
          switch (gNfcDev.activeDev->type) {
            case RFAL_NFC_LISTEN_TYPE_AP2P:
              RFal_NFC_Dep_RLS(); /* Send a Release to device */
              break;

            default:
              RFal_NFC_Dep_DSL(); /* Send a Deselect to device */
              aux = true;      /* Mark device as deselected */
              break;
          }
          break;

        default:
          return NFC_RequestError;
      }
    }
  }

  /* If deactivation type is only to Sleep, mark it and keep Field On  */
  if ((gNfcDev.deactType == RFAL_NFC_DEACTIVATE_SLEEP) && (gNfcDev.activeDev != NULL) && (aux)) {
    gNfcDev.isOperOngoing = false;


    if (gNfcDev.activeDev->type == RFAL_NFC_LISTEN_TYPE_NFCA) {
      gNfcDev.activeDev->dev.nfca.isSleep = true;
    } else if (gNfcDev.activeDev->type == RFAL_NFC_LISTEN_TYPE_NFCB) {
      gNfcDev.activeDev->dev.nfcb.isSleep = true;
    } else {
      /* MISRA 15.7 - Empty else */
    }
  } else {
    if (!gNfcDev.isDeactivating) {    
      RFal_WakeUpModeStop();

      RFal_ListenStop();
      //RFal_FieldOff();

      if ((gNfcDev.isFieldOn) && RFal_NFC_HasPollerTechs()) {                                /* Check if configured to Poll modes and the Field is On */
        aux = timerIsExpired(gNfcDev.discTmr);                                   /* Check total duration timer is already expired */
        if (((NeonRTOS_Millis() + RFAL_NFC_T_FIELD_OFF) > gNfcDev.discTmr) || (aux)) { /* In case Total Duration has expired or expring in less than tFIELD_OFF */
          gNfcDev.discTmr = (uint32_t)timerCalculateTimer(RFAL_NFC_T_FIELD_OFF);       /* Ensure that Operating Field is in Off condition at least tFIELD_OFF */
        }

        gNfcDev.isDeactivating = true;
        return NFC_Busy;
      }
    } else {                                                                                 /* The Field deactivation has started */
      if (!timerIsExpired(gNfcDev.discTmr)) {
        return NFC_Busy;                                                            /* Ensure Operating Field in Off condition for the time remaining */
      }
    }
  }

  gNfcDev.activeDev      = NULL;                                                               /* Clear Active Device info */
  gNfcDev.isDeactivating = false;
  gNfcDev.isTechInit     = false;
  gNfcDev.isFieldOn      = false;
  return NFC_OK;
}

uint32_t timerCalculateTimer(uint16_t time)
{
  return (NeonRTOS_Millis() + time);
}

bool timerIsExpired(uint32_t timer)
{
  uint32_t uDiff;
  int32_t sDiff;

  uDiff = (timer - NeonRTOS_Millis());   /* Calculate the diff between the timers */
  sDiff = uDiff;                            /* Convert the diff to a signed var      */
  /* Having done this has two side effects:
   * 1) all differences smaller than -(2^31) ms (~25d) will become positive
   *    Signaling not expired: acceptable!
   * 2) Time roll-over case will be handled correctly: super!
   */

  /* Check if the given timer has expired already */
  if (sDiff < 0) {
    return true;
  }

  return false;
}
