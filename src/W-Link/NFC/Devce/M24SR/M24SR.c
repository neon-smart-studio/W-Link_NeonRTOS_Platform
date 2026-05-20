/**
  ******************************************************************************
  * @file    m24sr.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions to interface with the M24SR
  *          device.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright(c) 2017 STMicroelectronics.
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

#include <stdbool.h>
#include <stdint.h>

#include "NeonRTOS.h"

#include "NFC/NFC_Def.h"

#include "M24SR_IO.h"
#include "M24SR.h"

/**
  * @brief  APDU-Header command structure
  */
typedef struct
{
  uint8_t CLA;  /* Command class */
  uint8_t INS;  /* Operation code */
  uint8_t P1;   /* Selection Mode */
  uint8_t P2;   /* Selection Option */
} C_APDU_Header;

/**
  * @brief  APDU-Body command structure
  */
typedef struct 
{
  uint8_t LC;                  /* Data field length */  
  uint8_t *pData ;             /* Command parameters */ 
  uint8_t LE;                  /* Expected length of data to be returned */
} C_APDU_Body;

/**
  * @brief  APDU Command structure 
  */
typedef struct
{
  C_APDU_Header Header;
  C_APDU_Body Body;
} C_APDU;

/**
  * @brief  SC response structure
  */
typedef struct
{
  uint8_t *pData ;  /* Data returned from the card */ 
                    /* pointer on the transceiver buffer = ReaderRecBuf[CR95HF_DATA_OFFSET ]; */
  uint8_t SW1;      /* Command Processing status */
  uint8_t SW2;      /* Command Processing qualification */
} R_APDU;

/**
  * @}
  */


/** @defgroup M24SR_Private_Constants  M24SR Driver Private Constants
  * @{
  */

/**
  * @brief  M24SR_Private_Code_Status 
  */
#define UB_STATUS_OFFSET                    4
#define LB_STATUS_OFFSET                    3

#define M24SR_NBBYTE_INVALID                0xFFFE

/**
  * @brief  M24SR_Private_Command_Management
  */  
/* special M24SR command ----------------------------------------------------------------------*/   
#define M24SR_OPENSESSION      0x26
#define M24SR_KILLSESSION      0x52

/* APDU Command: class list -------------------------------------------*/
#define C_APDU_CLA_DEFAULT     0x00
#define C_APDU_CLA_ST          0xA2

/*------------------------ Data Area Management Commands ---------------------*/
#define C_APDU_SELECT_FILE     0xA4
#define C_APDU_GET_RESPONCE    0xC0
#define C_APDU_STATUS          0xF2
#define C_APDU_UPDATE_BINARY   0xD6
#define C_APDU_READ_BINARY     0xB0
#define C_APDU_WRITE_BINARY    0xD0
#define C_APDU_UPDATE_RECORD   0xDC
#define C_APDU_READ_RECORD     0xB2

/*-------------------------- Safety Management Commands ----------------------*/
#define C_APDU_VERIFY          0x20
#define C_APDU_CHANGE          0x24
#define C_APDU_DISABLE         0x26
#define C_APDU_ENABLE          0x28

/*-------------------------- Gpio Management Commands ------------------------*/
#define C_APDU_INTERRUPT       0xD6

/*  Length  ----------------------------------------------------------------------------------*/
#define M24SR_STATUS_NBBYTE                       2
#define M24SR_CRC_NBBYTE                          2
#define M24SR_STATUSRESPONSE_NBBYTE               5
#define M24SR_DESELECTREQUEST_NBBYTE              3
#define M24SR_DESELECTRESPONSE_NBBYTE             3
#define M24SR_WATINGTIMEEXTRESPONSE_NBBYTE        4
#define M24SR_PASSWORD_NBBYTE                  0x10

/*  Command structure  ------------------------------------------------------------------------*/
#define M24SR_CMDSTRUCT_SELECTAPPLICATION         0x01FF
#define M24SR_CMDSTRUCT_SELECTCCFILE              0x017F
#define M24SR_CMDSTRUCT_SELECTNDEFFILE            0x017F
#define M24SR_CMDSTRUCT_READBINARY                0x019F
#define M24SR_CMDSTRUCT_UPDATEBINARY              0x017F
#define M24SR_CMDSTRUCT_VERIFYBINARYWOPWD         0x013F
#define M24SR_CMDSTRUCT_VERIFYBINARYWITHPWD       0x017F
#define M24SR_CMDSTRUCT_CHANGEREFDATA             0x017F
#define M24SR_CMDSTRUCT_ENABLEVERIFREQ            0x011F
#define M24SR_CMDSTRUCT_DISABLEVERIFREQ           0x011F
#define M24SR_CMDSTRUCT_SENDINTERRUPT             0x013F
#define M24SR_CMDSTRUCT_GPOSTATE                  0x017F

/*  Command structure Mask -------------------------------------------------------------------*/
#define M24SR_PCB_NEEDED        0x0001    /* PCB byte present or not */
#define M24SR_CLA_NEEDED        0x0002     /* CLA byte present or not */
#define M24SR_INS_NEEDED        0x0004     /* Operation code present or not*/ 
#define M24SR_P1_NEEDED         0x0008    /* Selection Mode  present or not*/
#define M24SR_P2_NEEDED         0x0010    /* Selection Option present or not*/
#define M24SR_LC_NEEDED         0x0020    /* Data field length byte present or not */
#define M24SR_DATA_NEEDED       0x0040    /* Data present or not */
#define M24SR_LE_NEEDED         0x0080    /* Expected length present or not */
#define M24SR_CRC_NEEDED        0x0100    /* 2 CRC bytes present  or not */

#define M24SR_DID_NEEDED        0x08      /* DID byte present or not */

/**
  * @brief  M24SR_Private_Offset_and_masks
  */

/*  Offset  ----------------------------------------------------------------------------------*/
#define M24SR_OFFSET_PCB                          0
#define M24SR_OFFSET_CLASS                        1
#define M24SR_OFFSET_INS                          2
#define M24SR_OFFSET_P1                           3

/*  mask  ------------------------------------------------------------------------------------*/
#define M24SR_MASK_BLOCK                          0xC0
#define M24SR_MASK_IBLOCK                         0x00
#define M24SR_MASK_RBLOCK                         0x80
#define M24SR_MASK_SBLOCK                         0xC0

/**
  * @}
  */


/** @defgroup M24SR_Private_Variables   M24SR Private Global Variables
  * @{
  */

static C_APDU               Command;
static uint8_t              DataBuffer[0xFF];
uint8_t                     uM24SRbuffer [0xFF];
static uint8_t              uDIDbyte =0x00;

/**
  * @}
  */

/** @defgroup M24SR_Private_Macros   M24SR Private Macros
  * @{
  */  

/** @brief Get Most Significant Byte
  * @param  val: number where MSB must be extracted
  * @retval MSB
  */ 
#define GETMSB(val)    ((uint8_t)((val & 0xFF00 )>>8) ) 

/** @brief Get Least Significant Byte
  * @param  val: number where LSB must be extracted
  * @retval LSB
  */ 
#define GETLSB(val)    ((uint8_t)(val & 0x00FF )) 

/** @brief Used to toggle the block number by adding 0 or 1 to default block number value
  * @param  val: number to know if incrementation is needed
  * @retval  0 or 1 if incrementation needed
  */
#define TOGGLE(val)    ((val != 0x00)? 0x00 : 0x01)

static NeonRTOS_SyncObj_t M24SR_GPO_SyncHandle;
static bool isWaitAnswer = false;

void M24SR_GPO_Callback(void )
{  
  if(isWaitAnswer)
  {
    NeonRTOS_SyncObjSignalFromISR(&M24SR_GPO_SyncHandle);
  }
}

NFC_OpResult M24SR_IsAnswerReady()
{
  isWaitAnswer = true;

  if(NeonRTOS_SyncObjWait(&M24SR_GPO_SyncHandle, M24SR_ANSWER_TIMEOUT)!=NeonRTOS_OK)
  {
    isWaitAnswer = false;
    return NFC_SlaveTimeout;
  }

  isWaitAnswer = false;
  return NFC_OK;
}

  /**
  * @brief  This function updates the CRC 
  * @param  None
  * @retval None
  */
static uint16_t M24SR_UpdateCrc(uint8_t ch, uint16_t *lpwCrc)
{
  ch =(ch^(uint8_t)((*lpwCrc) & 0x00FF));
  ch =(ch^(ch<<4));
  *lpwCrc =(*lpwCrc >> 8)^((uint16_t)ch << 8)^((uint16_t)ch<<3)^((uint16_t)ch>>4);
  
  return(*lpwCrc);
}

/**
  * @brief  This function returns the CRC 16 
  * @param  Data : pointer on the data used to compute the CRC16
  * @param  Length : number of byte of the data
  * @retval CRC16 
  */
static uint16_t M24SR_ComputeCrc(uint8_t *Data, uint8_t Length)
{
  uint8_t chBlock;
  uint16_t wCrc;
  
  wCrc = 0x6363; /* ITU-V.41 */
  
  do {
    chBlock = *Data++;
    M24SR_UpdateCrc(chBlock, &wCrc);
  } while(--Length);
  
  return wCrc ;
}


/**  
* @brief    This function computes the CRC16 residue as defined by CRC ISO/IEC 13239
* @param    DataIn    :  input to data 
* @param    Length     :   Number of bits of DataIn
* @retval   Status(SW1&SW2)    :   CRC16 residue is correct  
* @retval   M24SR_ERROR_CRC    :   CRC16 residue is false
*/
static NFC_OpResult M24SR_IsCorrectCRC16Residue(uint8_t *DataIn, uint8_t Length)
{
  uint16_t ResCRC=0;
  
  /* check the CRC16 Residue */
  if(Length !=0)
  {
    ResCRC= M24SR_ComputeCrc(DataIn, Length);
  }
  
  if(ResCRC == 0x0000)
  {
    /* Good CRC, but error status from M24SR */
    //return(((DataIn[Length-UB_STATUS_OFFSET]<<8) & 0xFF00) |(DataIn[Length-LB_STATUS_OFFSET] & 0x00FF)); 
    return NFC_OK;
  }
  else
  {
    ResCRC=0;
    ResCRC= M24SR_ComputeCrc(DataIn, 5);
    if(ResCRC != 0x0000)
    {
      /* Bad CRC */
      return NFC_CRC_Error;
    }
    else
    {
      /* Good CRC, but error status from M24SR */
      //return(((DataIn[1]<<8) & 0xFF00) |(DataIn[2] & 0x00FF));
      return NFC_OK;
    }
  }  
}


/**
  * @brief     This functions creates an I block command according to the structures CommandStructure and Command. 
  * @param     Command : structue which contains the field of the different parameter
  * @param     CommandStructure : structure that contain the structure of the command(if the different field are presnet or not 
  * @param     NbByte : number of byte of the command
  * @param     pCommand : pointer of the command created
  */
static void M24SR_BuildIBlockCommand(uint16_t CommandStructure, C_APDU Command, uint16_t *NbByte, uint8_t *pCommand)
{
  uint16_t  uCRC16; 
  static uint8_t BlockNumber = 0x01;
  
 (*NbByte) = 0;
  
  /* add the PCD byte */
  if((CommandStructure & M24SR_PCB_NEEDED) !=0)
  {
    /* toggle the block number */
    BlockNumber = TOGGLE(BlockNumber);
    /* Add the I block byte */
    pCommand[(*NbByte)++] = 0x02 |  BlockNumber; 
  }
  
  /* add the DID byte */
  if((BlockNumber & M24SR_DID_NEEDED) !=0)
  {
    /* Add the I block byte */
    pCommand[(*NbByte)++] = uDIDbyte; 
  }
  
  /* add the Class byte */
  if((CommandStructure & M24SR_CLA_NEEDED) !=0)
  {
    pCommand[(*NbByte)++] = Command.Header.CLA ;
  }
  /* add the instruction byte byte */
  if((CommandStructure & M24SR_INS_NEEDED) !=0)
  {
    pCommand[(*NbByte)++] = Command.Header.INS ;
  }
  /* add the Selection Mode byte */
  if((CommandStructure & M24SR_P1_NEEDED) !=0)
  {
    pCommand[(*NbByte)++] = Command.Header.P1 ;
  }
  /* add the Selection Mode byte */
  if((CommandStructure & M24SR_P2_NEEDED) !=0)
  {
    pCommand[(*NbByte)++] = Command.Header.P2 ;
  }
  /* add Data field lengthbyte */
  if((CommandStructure & M24SR_LC_NEEDED) !=0)
  {
    pCommand[(*NbByte)++] = Command.Body.LC ;
  }
  /* add Data field  */
  if((CommandStructure & M24SR_DATA_NEEDED) !=0)
  {
    memcpy(&(pCommand[(*NbByte)]) ,Command.Body.pData,Command.Body.LC ) ;
   (*NbByte) += Command.Body.LC ;
  }
  /* add Le field  */
  if((CommandStructure & M24SR_LE_NEEDED) !=0)
  {
    pCommand[(*NbByte)++] = Command.Body.LE ;
  }
  /* add CRC field  */
  if((CommandStructure & M24SR_CRC_NEEDED) !=0)
  {
    uCRC16 = M24SR_ComputeCrc(pCommand,(uint8_t)(*NbByte));
    /* append the CRC16 */
    pCommand [(*NbByte)++] = GETLSB (uCRC16 ) ;
    pCommand [(*NbByte)++] = GETMSB (uCRC16 ) ;  
  } 
}

static bool M24SR_IsSBlock(uint8_t *pBuffer)
{
  if((pBuffer[M24SR_OFFSET_PCB] & M24SR_MASK_BLOCK) == M24SR_MASK_SBLOCK)
  {
    return true;
  }
  else 
  {  
    return false;
  }
}

static NFC_OpResult M24SR_FWTExtension(uint8_t FWTbyte)
{
  NFC_OpResult op_status;
  
  uint8_t pBuffer[M24SR_STATUSRESPONSE_NBBYTE];
  uint16_t NthByte = 0,
  uCRC16;
  
  /* create the response */
  pBuffer[NthByte++] = 0xF2 ;  
  pBuffer[NthByte++] = FWTbyte ;
  /* compute the CRC */
  uCRC16 = M24SR_ComputeCrc(pBuffer,0x02);
  /* append the CRC16 */
  pBuffer [NthByte++] = GETLSB (uCRC16 ) ;
  pBuffer [NthByte++]=   GETMSB (uCRC16 ) ;  
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NthByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE); 
}

NFC_OpResult M24SR_Init()
{   
  NFC_OpResult op_status;
  
  if (NeonRTOS_SyncObjCreate(&M24SR_GPO_SyncHandle) != NeonRTOS_OK)
  {
      return NFC_MemoryError;
  }

  op_status = M24SR_IO_Init(M24SR_GPO_Callback);
  if(op_status < NFC_OK)
  {
      NeonRTOS_SyncObjDelete(&M24SR_GPO_SyncHandle);
      return op_status;
  }

  /* build the command */
  Command.Header.CLA = 0x00;
  Command.Header.INS = 0x00;
  /* copy the offset */
  Command.Header.P1 = 0x00 ;
  Command.Header.P2 = 0x00 ;
  /* copy the number of byte of the data field */
  Command.Body.LC = 0x00 ;
  /* copy the number of byte to read */
  Command.Body.LE = 0x00 ;
  Command.Body.pData = DataBuffer; 
  
  op_status = M24SR_KillSession();
  if(op_status < NFC_OK)
  {
      return op_status;
  }
  
  op_status = M24SR_ManageI2CGPO(M24SR_GPO_I2C_Answer_Ready);
  if(op_status < NFC_OK)
  {
      return op_status;
  }

  return NFC_OK;
}

NFC_OpResult M24SR_GetSession()
{
  NFC_OpResult op_status;

  uint8_t Buffer = M24SR_OPENSESSION;
  
  op_status = M24SR_IO_WriteMultiple(&Buffer, 1);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  /* Insure no access will be done just after open session */  
  /* The only way here is to poll I2C to know when M24SR is ready */
  /* GPO can not be use with GetSession command */
  NeonRTOS_Sleep(M24SR_ANSWER_TIMEOUT);
  
  return NFC_OK;
}

NFC_OpResult M24SR_KillSession()
{
  NFC_OpResult op_status;

  uint8_t Buffer = M24SR_KILLSESSION;
  
  op_status = M24SR_IO_WriteMultiple(&Buffer, 1);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  /* Insure no access will be done just after open session */  
  /* The only way here is to poll I2C to know when M24SR is ready */
  /* GPO can not be use with KillSession command */
  NeonRTOS_Sleep(M24SR_ANSWER_TIMEOUT);

  return NFC_OK;
}

NFC_OpResult M24SR_Deselect()
{
  NFC_OpResult op_status;

  uint8_t pBuffer[] = {0xC2,0xE0,0xB4};
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, M24SR_DESELECTREQUEST_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  /* flush the M24SR buffer */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_DESELECTREQUEST_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  }  
  
  return NFC_OK;
}

NFC_OpResult M24SR_SelectApplication()
{
  NFC_OpResult op_status;

  uint8_t *pBuffer = uM24SRbuffer ,
  NbByteToRead = M24SR_STATUSRESPONSE_NBBYTE;
  uint8_t uLc = 0x07,
  pData[] = {0xD2,0x76,0x00,0x00,0x85,0x01,0x01},
  uLe = 0x00;
  uint16_t uP1P2 =0x0400,
  NbByte;
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_SELECT_FILE;
  /* copy the offset */
  Command.Header.P1 = GETMSB (uP1P2 ) ;
  Command.Header.P2 = GETLSB (uP1P2 ) ;
  /* copy the number of byte of the data field */
  Command.Body.LC = uLc ;
  /* copy the data */
  memcpy(Command.Body.pData, pData, uLc);
  /* copy the number of byte to read */
  Command.Body.LE = uLe ;
  /* build the I2C command */

  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_SELECTAPPLICATION, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, NbByteToRead);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  return M24SR_IsCorrectCRC16Residue(pBuffer, NbByteToRead);
}

NFC_OpResult M24SR_SelectCCfile()
{
  NFC_OpResult op_status;

  uint8_t *pBuffer = uM24SRbuffer ,
  NbByteToRead = M24SR_STATUSRESPONSE_NBBYTE;
  uint8_t uLc = 0x02;
  uint16_t uP1P2 =0x000C,
  uNbFileId =CC_FILE_ID,
  NbByte;
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_SELECT_FILE;
  /* copy the offset */
  Command.Header.P1 = GETMSB (uP1P2 ) ;
  Command.Header.P2 = GETLSB (uP1P2 ) ;
  /* copy the number of byte of the data field */
  Command.Body.LC = uLc ;
  /* copy the File Id */
  Command.Body.pData[0] = GETMSB (uNbFileId ) ;
  Command.Body.pData[1] = GETLSB (uNbFileId ) ;

  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_SELECTCCFILE, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, NbByteToRead);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  return M24SR_IsCorrectCRC16Residue(pBuffer, NbByteToRead);
}

NFC_OpResult M24SR_SelectSystemfile()
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer ,
  NbByteToRead = M24SR_STATUSRESPONSE_NBBYTE;
  uint8_t    uLc = 0x02;
  uint16_t  uP1P2 =0x000C,
  uNbFileId =SYSTEM_FILE_ID,
  NbByte;
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_SELECT_FILE;
  /* copy the offset */
  Command.Header.P1 = GETMSB (uP1P2 ) ;
  Command.Header.P2 = GETLSB (uP1P2 ) ;
  /* copy the number of byte of the data field */
  Command.Body.LC = uLc ;
  /* copy the File Id */
  Command.Body.pData[0] = GETMSB (uNbFileId ) ;
  Command.Body.pData[1] = GETLSB (uNbFileId ) ;

  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_SELECTCCFILE, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, NbByteToRead);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, NbByteToRead);
}

NFC_OpResult M24SR_SelectNDEFfile(uint16_t NDEFfileId)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer ,
  NbByteToRead = M24SR_STATUSRESPONSE_NBBYTE;
  uint8_t    uLc = 0x02;
  uint16_t  uP1P2 =0x000C,
  NbByte;
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_SELECT_FILE;
  /* copy the offset */
  Command.Header.P1 = GETMSB (uP1P2 ) ;
  Command.Header.P2 = GETLSB (uP1P2 ) ;
  /* copy the number of byte of the data field */
  Command.Body.LC = uLc ;
  /* copy the offset */
  Command.Body.pData[0] = GETMSB (NDEFfileId ) ;
  Command.Body.pData[1] = GETLSB (NDEFfileId ) ;

  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_SELECTNDEFFILE, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, NbByteToRead);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, NbByteToRead);
}

NFC_OpResult M24SR_ReadBinary(uint16_t Offset ,uint8_t NbByteToRead, uint8_t *pBufferRead)
{
  NFC_OpResult op_status;

  uint8_t *pBuffer = uM24SRbuffer ;
  uint16_t  NbByte;
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_READ_BINARY;
  /* copy the offset */
  Command.Header.P1 = GETMSB (Offset ) ;
  Command.Header.P2 = GETLSB (Offset ) ;
  /* copy the number of byte to read */
  Command.Body.LE = NbByteToRead ;
  
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_READBINARY, Command, &NbByte, pBuffer);
  
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IO_ReadMultiple(pBuffer, NbByteToRead + M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  op_status = M24SR_IsCorrectCRC16Residue(pBuffer,NbByteToRead+ M24SR_STATUSRESPONSE_NBBYTE);

  /* retrieve the data without SW1 & SW2 as provided as return value of the function */
  memcpy(pBufferRead ,&pBuffer[1],NbByteToRead);

  return op_status;
}

NFC_OpResult M24SR_STReadBinary(uint16_t Offset, uint8_t NbByteToRead, uint8_t *pBufferRead)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer ;
  uint16_t  NbByte;
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_ST;
  Command.Header.INS = C_APDU_READ_BINARY;
  /* copy the offset */
  Command.Header.P1 = GETMSB (Offset ) ;
  Command.Header.P2 = GETLSB (Offset ) ;
  /* copy the number of byte to read */
  Command.Body.LE = NbByteToRead ;
  
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_READBINARY, Command, &NbByte, pBuffer);
  
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IO_ReadMultiple(pBuffer, NbByteToRead + M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  op_status = M24SR_IsCorrectCRC16Residue(pBuffer,NbByteToRead+ M24SR_STATUSRESPONSE_NBBYTE); 

  /* retrieve the data without SW1 & SW2 as provided as return value of the function */
  memcpy(pBufferRead ,&pBuffer[1],NbByteToRead);

  return op_status;
}

NFC_OpResult M24SR_UpdateBinary(uint16_t Offset ,uint8_t NbByteToWrite,uint8_t *pDataToWrite)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer ;
  uint16_t  NbByte;
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_UPDATE_BINARY;
  /* copy the offset */
  Command.Header.P1 = GETMSB (Offset ) ;
  Command.Header.P2 = GETLSB (Offset ) ;
  /* copy the number of byte of the data field */
  Command.Body.LC = NbByteToWrite ;
  /* copy the File Id */
  memcpy(Command.Body.pData ,pDataToWrite, NbByteToWrite);
  
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_UPDATEBINARY, Command, &NbByte, pBuffer);
  
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* if the response is a Watiting frame extenstion request */ 
  if(M24SR_IsSBlock(pBuffer))
  {
    /*check the CRC */ 
    if(M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_WATINGTIMEEXTRESPONSE_NBBYTE) == NFC_CRC_Error)
    {
        return NFC_CRC_Error;
    }

    /* send the FrameExension response*/ 
    op_status = M24SR_FWTExtension( pBuffer [M24SR_OFFSET_PCB+1]);
  }
  else
  {  
    op_status = M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE); 
  }
  
  return op_status;
}

NFC_OpResult M24SR_Verify(uint16_t uPwdId, uint8_t NbPwdByte ,uint8_t *pPwd)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer ;
  uint16_t  status = 0x0000 ; 
  uint16_t  NbByte;
  
  /*check the parameters */
  if(uPwdId > 0x0003)
  {  
    return NFC_InvalidParameter;
  }
  if((NbPwdByte != 0x00) &&(NbPwdByte != 0x10))
  {  
    return NFC_InvalidParameter;
  }
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_VERIFY;
  /* copy the Password Id */
  Command.Header.P1 = GETMSB (uPwdId ) ;
  Command.Header.P2 = GETLSB (uPwdId ) ;
  /* copy the number of byte of the data field */
  Command.Body.LC = NbPwdByte ;
  
  if(NbPwdByte == 0x10) 
  {
    /* copy the password */
    memcpy(Command.Body.pData, pPwd, NbPwdByte);
    /* build the I2C command */
    M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_VERIFYBINARYWITHPWD, Command, &NbByte, pBuffer);
  }
  else 
  {
    /* build the I2C command */
    M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_VERIFYBINARYWOPWD, Command, &NbByte, pBuffer);
  }
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  /* wait for answer ready */
  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
}

NFC_OpResult M24SR_ChangeReferenceData(uint16_t uPwdId, uint8_t *pPwd)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer;
  uint16_t  NbByte;
  
  /*check the parameters */
  if(uPwdId > 0x0003)
  {  
    return NFC_InvalidParameter;
  }
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_CHANGE;
  /* copy the Password Id */
  Command.Header.P1 = GETMSB (uPwdId ) ;
  Command.Header.P2 = GETLSB (uPwdId ) ;
  /* copy the number of byte of the data field */
  Command.Body.LC = M24SR_PASSWORD_NBBYTE ;
  /* copy the password */
  memcpy(Command.Body.pData, pPwd, M24SR_PASSWORD_NBBYTE);
  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_CHANGEREFDATA, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
}

NFC_OpResult M24SR_EnableVerificationRequirement(uint16_t uReadOrWrite)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer;
  uint16_t  NbByte;
  
  /*check the parameters */
  if((uReadOrWrite != 0x0001) &&(uReadOrWrite != 0x0002))
  {  
    return NFC_InvalidParameter;
  }
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_ENABLE;
  /* copy the Password Id */
  Command.Header.P1 = GETMSB (uReadOrWrite ) ;
  Command.Header.P2 = GETLSB (uReadOrWrite ) ;

  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_ENABLEVERIFREQ, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status =  M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  /* The right access to be updated in EEPROM need at least 6ms */  
  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  /* read the response */ 
  op_status =  M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
}

NFC_OpResult M24SR_DisableVerificationRequirement(uint16_t uReadOrWrite)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer;
  uint16_t  NbByte;
  
  /*check the parameters */
  if((uReadOrWrite != 0x0001) &&(uReadOrWrite != 0x0002))
  {  
    return NFC_InvalidParameter;
  }
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_DEFAULT;
  Command.Header.INS = C_APDU_DISABLE;
  /* copy the Password Id */
  Command.Header.P1 = GETMSB (uReadOrWrite ) ;
  Command.Header.P2 = GETLSB (uReadOrWrite ) ;

  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_DISABLEVERIFREQ, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* The right access to be updated in EEPROM need at least 6ms */    
  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
}

NFC_OpResult M24SR_EnablePermanentState(uint16_t uReadOrWrite)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer;
  uint16_t  NbByte;
  
  /*check the parameters */
  if((uReadOrWrite != 0x0001) &&(uReadOrWrite != 0x0002))
  {  
    return NFC_InvalidParameter;
  }
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_ST;
  Command.Header.INS = C_APDU_ENABLE;
  /* copy the Password Id */
  Command.Header.P1 = GETMSB (uReadOrWrite ) ;
  Command.Header.P2 = GETLSB (uReadOrWrite ) ;

  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_ENABLEVERIFREQ, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
}

NFC_OpResult M24SR_DisablePermanentState(uint16_t uReadOrWrite)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer;
  uint16_t  NbByte;
  
  /*check the parameters */
  if((uReadOrWrite != 0x0001) &&(uReadOrWrite != 0x0002))
  {  
    return NFC_InvalidParameter;
  }
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_ST;
  Command.Header.INS = C_APDU_DISABLE;
  /* copy the Password Id */
  Command.Header.P1 = GETMSB (uReadOrWrite ) ;
  Command.Header.P2 = GETLSB (uReadOrWrite ) ;
  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_DISABLEVERIFREQ, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
}

NFC_OpResult M24SR_SendInterrupt()
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer;
  uint16_t  uP1P2 =0x001E;
  uint16_t  NbByte;
  
  op_status = M24SR_ManageI2CGPO(M24SR_GPO_Interrupt);
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_ST;
  Command.Header.INS = C_APDU_INTERRUPT;
  /* copy the Password Id */
  Command.Header.P1 = GETMSB (uP1P2 ) ;
  Command.Header.P2 = GETLSB (uP1P2 ) ;
  Command.Body.LC = 0x00 ;

  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_SENDINTERRUPT, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
}

NFC_OpResult M24SR_StateControl(uint8_t uSetOrReset)
{
  NFC_OpResult op_status;

  uint8_t   *pBuffer = uM24SRbuffer;
  uint16_t  uP1P2 =0x001F;
  uint16_t  NbByte;
  
  /*check the parameters */
  if((uSetOrReset != 0x01) &&(uSetOrReset != 0x00))
  {  
    return NFC_InvalidParameter;;
  }
  
  op_status = M24SR_ManageI2CGPO(M24SR_GPO_State_Control);
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* build the command */
  Command.Header.CLA = C_APDU_CLA_ST;
  Command.Header.INS = C_APDU_INTERRUPT;
  /* copy the Password Id */
  Command.Header.P1 = GETMSB (uP1P2 ) ;
  Command.Header.P2 = GETLSB (uP1P2 ) ;
  Command.Body.LC = 0x01 ;
  /* copy the data */
  memcpy(Command.Body.pData, &uSetOrReset, 0x01);
  //Command.Body.LE = 0x00 ;

  /* build the I2C command */
  M24SR_BuildIBlockCommand(M24SR_CMDSTRUCT_GPOSTATE, Command, &NbByte, pBuffer);
  
  /* send the request */ 
  op_status = M24SR_IO_WriteMultiple(pBuffer, NbByte);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  op_status = M24SR_IsAnswerReady();
  if(op_status < NFC_OK)
  {
    return op_status;
  } 

  /* read the response */ 
  op_status = M24SR_IO_ReadMultiple(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
  if(op_status < NFC_OK)
  {
    return op_status;
  } 
  
  return M24SR_IsCorrectCRC16Residue(pBuffer, M24SR_STATUSRESPONSE_NBBYTE);
}

NFC_OpResult M24SR_ManageI2CGPO(M24SR_GPO_Management GPO_I2Cconfig)
{
  NFC_OpResult op_status;

  uint8_t GPO_config;
  uint8_t DefaultPassword[16]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
  if(GPO_I2Cconfig >= M24SR_GPO_MAX)
  {  
    return NFC_InvalidParameter;
  }
  
  op_status = M24SR_SelectApplication();
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_SelectSystemfile();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  op_status = M24SR_ReadBinary(0x0004, 0x01, &GPO_config);
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* Update only GPO purpose for I2C */  
  GPO_config =(GPO_config & 0xF0) | GPO_I2Cconfig;

  op_status = M24SR_SelectSystemfile();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  op_status = M24SR_Verify(M24SR_I2C_PWD ,0x10 ,DefaultPassword);
  if(op_status < NFC_OK)
  {
    return op_status;
  }

  op_status = M24SR_UpdateBinary(0x0004 ,0x01, &(GPO_config));
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* if we have set interrupt mode for I2C synchro we can enable interrupt mode */
  if(GPO_I2Cconfig == M24SR_GPO_I2C_Answer_Ready)
  {
    // int mode
  }

  return op_status;
}

NFC_OpResult M24SR_ManageRFGPO(M24SR_GPO_Management GPO_RFconfig)
{
  NFC_OpResult op_status;

  uint8_t GPO_config;
  uint8_t DefaultPassword[16]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
  if(GPO_RFconfig >= M24SR_GPO_MAX)
  {  
    return NFC_InvalidParameter;
  }
  
  op_status = M24SR_SelectApplication();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  op_status = M24SR_SelectSystemfile();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  op_status = M24SR_ReadBinary(0x0004, 0x01, &GPO_config);
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  /* Update only GPO purpose for I2C */  
  GPO_config =(GPO_config & 0x0F) |(GPO_RFconfig<<4);

  op_status = M24SR_SelectSystemfile();
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  op_status = M24SR_Verify(M24SR_I2C_PWD ,0x10 ,DefaultPassword);
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  op_status = M24SR_UpdateBinary(0x0004 ,0x01, &(GPO_config));
  if(op_status < NFC_OK)
  {
    return op_status;
  }
  
  return op_status;
}

NFC_OpResult M24SR_RFConfig(bool OnOffChoice)
{
    /* Disable RF */
    if(OnOffChoice == true )
    {
        return M24SR_IO_RF_Enable(); /* PIN SET */
    }
    else
    {
        return M24SR_IO_RF_Disable(); /* PIN RESET */
    }
}

/*******************(C) COPYRIGHT STMicroelectronics *****END OF FILE****/
