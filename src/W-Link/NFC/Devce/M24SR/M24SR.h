/**
  ******************************************************************************
  * @file    m24sr.h
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage M24SR
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/*
 * Based on STMicroelectronics M24SR driver
 * Modified by Neon Smart Studio for W-Link
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __M24SR_H
#define __M24SR_H

#include <stdbool.h>
#include <stdint.h>

#include "NFC/NFC_Def.h"

/* NFC IO specific config parameters */
#define NFC_IO_TRIALS              (uint32_t) 1 /* In case M24SR will reply ACK failed allow to perform retry */
    
#define M24SR_ANSWER_TIMEOUT   (uint32_t)  80 /* Timeout used by the component function NFC_IO_IsDeviceReady() */
#define M24SR_ANSWER_STABLE    (uint8_t)    5 /* Loop repetition used by the component function NFC_IO_IsDeviceReady() */

/*-------------------------- Password_Management ----------------------------*/
#define M24SR_READ_PWD         (uint16_t) 0x0001
#define M24SR_WRITE_PWD        (uint16_t) 0x0002
#define M24SR_I2C_PWD          (uint16_t) 0x0003

/*-------------------------- Verify command answer ----------------------------*/
#define M24SR_PWD_NOT_NEEDED   (uint16_t) 0x9000
#define M24SR_PWD_NEEDED       (uint16_t) 0x6300
#define M24SR_PWD_CORRECT      (uint16_t) 0x9000

/**
  * @brief  M24SR_Private_File_Identifier
  */
#define SYSTEM_FILE_ID          0xE101   
#define CC_FILE_ID              0xE103
#define NDEF_FILE_ID            0x0001   
  
/**
  * @brief  GPO state structure 
  */
typedef enum{
  M24SR_GPO_High_Impedance= 0,
  M24SR_GPO_Session_Opened,
  M24SR_GPO_WIP,
  M24SR_GPO_I2C_Answer_Ready,
  M24SR_GPO_Interrupt,
  M24SR_GPO_State_Control,
  M24SR_GPO_MAX
}M24SR_GPO_Management;

#ifdef __cplusplus
 extern "C" {
#endif

NFC_OpResult M24SR_Init();
NFC_OpResult M24SR_GetSession();
NFC_OpResult M24SR_KillSession();
NFC_OpResult M24SR_Deselect();
NFC_OpResult M24SR_SelectApplication();
NFC_OpResult M24SR_SelectCCfile();
NFC_OpResult M24SR_SelectSystemfile();
NFC_OpResult M24SR_SelectNDEFfile(uint16_t NDEFfileId);
NFC_OpResult M24SR_ReadBinary(uint16_t Offset ,uint8_t NbByteToRead, uint8_t *pBufferRead);
NFC_OpResult M24SR_STReadBinary(uint16_t Offset, uint8_t NbByteToRead, uint8_t *pBufferRead);
NFC_OpResult M24SR_UpdateBinary(uint16_t Offset ,uint8_t NbByteToWrite,uint8_t *pDataToWrite);
NFC_OpResult M24SR_Verify(uint16_t uPwdId, uint8_t NbPwdByte ,uint8_t *pPwd);
NFC_OpResult M24SR_ChangeReferenceData(uint16_t uPwdId, uint8_t *pPwd);
NFC_OpResult M24SR_EnableVerificationRequirement(uint16_t uReadOrWrite);
NFC_OpResult M24SR_DisableVerificationRequirement(uint16_t uReadOrWrite);
NFC_OpResult M24SR_EnablePermanentState(uint16_t uReadOrWrite);
NFC_OpResult M24SR_DisablePermanentState(uint16_t uReadOrWrite);
NFC_OpResult M24SR_SendInterrupt();
NFC_OpResult M24SR_StateControl(uint8_t uSetOrReset);
NFC_OpResult M24SR_ManageI2CGPO(M24SR_GPO_Management GPO_I2Cconfig);
NFC_OpResult M24SR_ManageRFGPO(M24SR_GPO_Management GPO_RFconfig);
NFC_OpResult M24SR_RFConfig(bool OnOffChoice);


#ifdef __cplusplus
}
#endif

#endif /* __M24SR_H */




/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
