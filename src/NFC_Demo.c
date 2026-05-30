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

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "soc.h"

#include "FreeRTOS.h"
#include "task.h"

#include "SysCtrl/SysCtrl.h"
#include "UART/UART.h"
#include "I2C/I2C_Master.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#include "NFC/RFal/RFal_NFC.h"
#include "NFC/NDef/NDef_T5T.h"
#include "NFC/NDef/NDef_Record.h"
#include "NFC/NDef/NDef_Poller.h"
#include "NFC/NDef/NDef_Types.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/* Definition of possible states the demo state machine could have */
#define DEMO_ST_NOTINIT               0  /*!< Demo State:  Not initialized */
#define DEMO_ST_START_DISCOVERY       1  /*!< Demo State:  Start Discovery */
#define DEMO_ST_DISCOVERY             2  /*!< Demo State:  Discovery       */

#define NDEF_DEMO_READ              0U   /*!< NDEF menu read               */
#define NDEF_DEMO_WRITE_MSG1        1U   /*!< NDEF menu write 1 record     */
#define NDEF_DEMO_WRITE_MSG2        2U   /*!< NDEF menu write 2 records    */
#define NDEF_DEMO_FORMAT_TAG        3U   /*!< NDEF menu format tag         */

#define NDEF_DEMO_MAX_FEATURES      4U   /*!< Number of menu items         */

#define NDEF_WRITE_FORMAT_TIMEOUT   10000U /*!< When write or format mode is selected, demo returns back to read mode after a timeout */
#define NDEF_LED_BLINK_DURATION       250U /*!< Led blink duration         */

#define DEMO_RAW_MESSAGE_BUF_LEN      8192 /*!< Raw message buffer len     */

#define DEMO_ST_MANUFACTURER_ID      0x02U /*!< ST Manufacturer ID         */

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

/* P2P communication data */
static uint8_t NFCID3[] = {0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
static uint8_t GB[] = {0x46, 0x66, 0x6d, 0x01, 0x01, 0x11, 0x02, 0x02, 0x07, 0x80, 0x03, 0x02, 0x00, 0x03, 0x04, 0x01, 0x32, 0x07, 0x01, 0x03};

/* P2P communication data */
static uint8_t ndefLLCPSYMM[] = {0x00, 0x00};
static uint8_t ndefInit[] = {0x05, 0x20, 0x06, 0x0F, 0x75, 0x72, 0x6E, 0x3A, 0x6E, 0x66, 0x63, 0x3A, 0x73, 0x6E, 0x3A, 0x73, 0x6E, 0x65, 0x70, 0x02, 0x02, 0x07, 0x80, 0x05, 0x01, 0x02};
static const uint8_t ndefSnepPrefix[] = { 0x13, 0x20, 0x00, 0x10, 0x02, 0x00, 0x00, 0x00 };
static const uint8_t URL[] = "st.com";
static NDef_Const_Buffer bufURL = { URL, sizeof(URL) - 1 };
static uint8_t ndefUriBuffer[255];

static uint8_t *ndefStates[] = {
  (uint8_t *)"INVALID",
  (uint8_t *)"INITIALIZED",
  (uint8_t *)"READ/WRITE",
  (uint8_t *)"READ-ONLY"
};

static const uint8_t *ndefDemoFeatureDescription[NDEF_DEMO_MAX_FEATURES] = {
  (uint8_t *)"1. Tap a tag to read its content",
  (uint8_t *)"2. Present a tag to write a Text record",
  (uint8_t *)"3. Present a tag to write a URI record and an Android Application record",
  (uint8_t *)"4. Present an ST tag to format",
};

static uint8_t ndefURI[]          = "st.com";
static uint8_t ndefTEXT[]         = "Welcome to ST NDEF demo";
static uint8_t ndefTextLangCode[] = "en";

static uint8_t ndefAndroidPackName[] = "com.st.st25nfc";

static RFal_NFC_DiscoverParam discParam;
static uint8_t              state = DEMO_ST_NOTINIT;

static uint8_t              ndefDemoFeature     = NDEF_DEMO_READ;
static uint8_t              ndefDemoPrevFeature = 0xFF;
static bool                 verbose             = false;

static uint8_t              rawMessageBuf[DEMO_RAW_MESSAGE_BUF_LEN];

static uint32_t             timer;
static uint32_t             timerLed;
static bool                 ledOn;

NDef_Context ndef;

#define MAX_HEX_STR         4
#define MAX_HEX_STR_LENGTH  128
char hexStr[MAX_HEX_STR][MAX_HEX_STR_LENGTH];
uint8_t hexStrIdx = 0;

static void demoNdef(RFal_NFC_Device *nfcDevice);
static void ndefCCDump(void);
static void ndefDumpSysInfo(void);
static bool ndefIsSTTag(void);

static void demoP2P(void);
NFC_OpResult  demoTransceiveBlocking(uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxBuf, uint16_t **rcvLen, uint32_t fwt);

static NFC_OpResult NDef_Record_Dump_(const NDef_Record *record, bool verbose);
static NFC_OpResult NDef_Message_Dump(const NDef_Message *message, bool verbose);
static NFC_OpResult ndefEmptyTypeDump(const NDef_Type *empty);
static NFC_OpResult ndefRtdDeviceInfoDump(const NDef_Type *devInfo);
static NFC_OpResult ndefRtdTextDump(const NDef_Type *text);
static NFC_OpResult ndefRtdUriDump(const NDef_Type *uri);
static NFC_OpResult ndefRtdAarDump(const NDef_Type *ext);
static NFC_OpResult ndefMediaVCardDump(const NDef_Type *vCard);
static NFC_OpResult ndefMediaWifiDump(const NDef_Type *wifi);
static NFC_OpResult NDef_Record_Dump_Type(const NDef_Record *record);
static NFC_OpResult NDef_Buffer_Dump(const char *string, const NDef_Const_Buffer *bufPayload, bool verbose);
static NFC_OpResult NDef_Buffer_Print(const char *prefix, const NDef_Const_Buffer *bufPayload, const char *suffix);
static NFC_OpResult NDef_Buffer_8_Print(const char *prefix, const NDef_Const_Buffer_8 *bufPayload, const char *suffix);

/*! Table to associate enums to pointer to function */
typedef struct {
  NDef_Type_ID typeId;                        /*!< NDEF type Id             */
  NFC_OpResult(*dump)(const NDef_Type *type);  /*!< Pointer to dump function */
} NDef_Type_DumpTable;

/*! Type to associate a property and string description */
typedef struct {
  NDef_Const_Buffer bufType;
  char *string;
} ndefVCardTypeTable;

static const NDef_Type_DumpTable typeDumpTable[] = {
  { NDEF_TYPE_ID_EMPTY,           ndefEmptyTypeDump       },
  { NDEF_TYPE_ID_RTD_DEVICE_INFO, ndefRtdDeviceInfoDump   },
  { NDEF_TYPE_ID_RTD_TEXT,        ndefRtdTextDump         },
  { NDEF_TYPE_ID_RTD_URI,         ndefRtdUriDump          },
  { NDEF_TYPE_ID_RTD_AAR,         ndefRtdAarDump          },
  { NDEF_TYPE_ID_MEDIA_VCARD,     ndefMediaVCardDump      },
  { NDEF_TYPE_ID_MEDIA_WIFI,      ndefMediaWifiDump       },
};

static char *hex2Str(unsigned char *data, size_t dataLen);

void demoP2P(void)
{
  uint16_t   *rxLen;
  uint8_t    *rxData;
  NFC_OpResult err;

  NDef_Buffer   bufPayload;
  NDef_Message message;
  NDef_Record  record;
  NDef_Type    uri;

  UART_Printf(" Initialize device .. ");
  err = demoTransceiveBlocking(ndefInit, sizeof(ndefInit), &rxData, &rxLen, RFAL_FWT_NONE);
  if (err != NFC_OK) {
    UART_Printf("failed.");
    return;
  }
  UART_Printf("succeeded.\r\n");

  NDef_RtdURIInit(&uri, NDEF_URI_PREFIX_HTTP_WWW, &bufURL);
  NDef_RtdURIToRecord(&uri, &record);

  NDef_Message_Init(&message);
  NDef_Message_Append(&message, &record);  /* To get MB and ME bits set */

  /* Build the SNEP buffer made of the prefix, the length byte and the record */
  memcpy(ndefUriBuffer, ndefSnepPrefix, sizeof(ndefSnepPrefix));

  /* Skip 1 byte for length byte */
  bufPayload.buffer = ndefUriBuffer + sizeof(ndefSnepPrefix) + 1;
  bufPayload.length = sizeof(ndefUriBuffer) - sizeof(ndefSnepPrefix);
  NDef_Message_Encode(&message, &bufPayload);

  ndefUriBuffer[sizeof(ndefSnepPrefix)] = bufPayload.length;

  bufPayload.buffer = ndefUriBuffer;
  bufPayload.length = sizeof(ndefSnepPrefix) + 1 + bufPayload.length;

  if (err != NFC_OK) {
    UART_Printf("NDEF message creation failed\r\n");
    return;
  }

  NDef_Buffer_Dump("URL converted to SNEP:\r\n", (NDef_Const_Buffer *)&bufPayload, true);

  UART_Printf(" Push NDEF Uri: www.ST.com .. ");
  err = demoTransceiveBlocking(bufPayload.buffer, bufPayload.length, &rxData, &rxLen, RFAL_FWT_NONE);
  if (err != NFC_OK) {
    UART_Printf("failed.");
    return;
  }
  UART_Printf("succeeded.\r\n");


  UART_Printf(" Device present, maintaining connection ");
  while (err == NFC_OK) {
    err = demoTransceiveBlocking(ndefLLCPSYMM, sizeof(ndefLLCPSYMM), &rxData, &rxLen, RFAL_FWT_NONE);
    UART_Printf(".");
    NeonRTOS_Sleep(50);
  }
  UART_Printf("\r\n Device removed.\r\n");
}

NFC_OpResult demoTransceiveBlocking(uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxData, uint16_t **rcvLen, uint32_t fwt)
{
  NFC_OpResult err;

  err = RFal_NFC_DataExchangeStart(txBuf, txBufSize, rxData, rcvLen, fwt);
  if (err == NFC_OK) {
    do {
      RFal_NFC_Worker();
      err = RFal_NFC_DataExchangeGetStatus();
    } while (err == NFC_Busy);
  }
  return err;
}

static void demoNdef(RFal_NFC_Device *pNfcDevice)
{
  NFC_OpResult       err;
  NDef_Message      message;
  uint32_t         rawMessageLen;
  NDef_Info         info;
  NDef_Buffer        bufRawMessage;
  NDef_Const_Buffer  bufConstRawMessage;

  NDef_Record       record1;
  NDef_Record       record2;

  NDef_Type         text;
  NDef_Type         uri;
  NDef_Type         aar;

  NDef_Const_Buffer_8 bufTextLangCode;
  NDef_Const_Buffer bufTextLangText;
  NDef_Const_Buffer bufUri;
  NDef_Const_Buffer bufAndroidPackName;

  /*
   * Perform NDEF Context Initialization
   */
  err = NDef_Poller_ContextInitialization(&ndef, pNfcDevice);
  if (err != NFC_OK) {
    UART_Printf("NDEF NOT DETECTED (ndefPollerContextInitialization returns ");
    UART_Printf("%d",err);
    UART_Printf(")\r\n");
    return;
  }

  if (verbose & (pNfcDevice->type == RFAL_NFC_LISTEN_TYPE_NFCV)) {
    ndefDumpSysInfo();
  }

  /*
   * Perform NDEF Detect procedure
   */
  err = NDef_Poller_NdefDetect(&ndef, &info);
  if (err != NFC_OK) {
    UART_Printf("NDEF NOT DETECTED (ndefPollerNdefDetect returns ");
    UART_Printf("%d",err);
    UART_Printf(")\r\n");
    if (ndefDemoFeature != NDEF_DEMO_FORMAT_TAG) {
      return;
    }
  } else {
    UART_Printf((char *)ndefStates[info.state]);
    UART_Printf(" NDEF detected.\r\n");
    ndefCCDump();

    if (verbose) {
      UART_Printf("NDEF Len: ");
      UART_Printf("%d",ndef.messageLen);
      UART_Printf(", Offset=");
      UART_Printf("%d",ndef.messageOffset);
      UART_Printf("\r\n");
    }
  }

  switch (ndefDemoFeature) {
    /*
     * Demonstrate how to read the NDEF message from the Tag
     */
    case NDEF_DEMO_READ:
      if (info.state == NDEF_STATE_INITIALIZED) {
        /* Nothing to read... */
        return;
      }
      err = NDef_Poller_ReadRawMessage(&ndef, rawMessageBuf, sizeof(rawMessageBuf), &rawMessageLen, true);
      if (err != NFC_OK) {
        UART_Printf("NDEF message cannot be read (ndefPollerReadRawMessage returns ");
        UART_Printf("%d", err);
        UART_Printf(")\r\n");
        return;
      }
      if (verbose) {
        bufRawMessage.buffer = rawMessageBuf;
        bufRawMessage.length = rawMessageLen;
        NDef_Buffer_Dump(" NDEF Content", (NDef_Const_Buffer *)&bufRawMessage, verbose);
      }
      bufConstRawMessage.buffer = rawMessageBuf;
      bufConstRawMessage.length = rawMessageLen;
      err = NDef_Message_Decode(&bufConstRawMessage, &message);
      if (err != NFC_OK) {
        UART_Printf("NDEF message cannot be decoded (NDef_Message_Decode  returns ");
        UART_Printf("%d", err);
        UART_Printf(")\r\n");
        return;
      }
      err = NDef_Message_Dump(&message, verbose);
      if (err != NFC_OK) {
        UART_Printf("NDEF message cannot be displayed (NDef_Message_Dump returns ");
        UART_Printf("%d", err);
        UART_Printf(")\r\n");
        return;
      }
      break;

    /*
     * Demonstrate how to encode a text record and write the message to the tag
     */
    case NDEF_DEMO_WRITE_MSG1:
      ndefDemoFeature = NDEF_DEMO_READ; /* returns to READ mode after write */
      err  = NDef_Message_Init(&message); /* Initialize message structure */
      bufTextLangCode.buffer = ndefTextLangCode;
      bufTextLangCode.length = strlen((char *)ndefTextLangCode);

      bufTextLangText.buffer = ndefTEXT;
      bufTextLangText.length = strlen((char *)ndefTEXT);

      NDef_RtdTextInit(&text, TEXT_ENCODING_UTF8, &bufTextLangCode, &bufTextLangText); /* Initialize Text type structure */
      NDef_RtdTextToRecord(&text, &record1); /* Encode Text Record */
      NDef_Message_Append(&message, &record1); /* Append Text record to message */
      if (err != NFC_OK) {
        UART_Printf("Message creation failed\r\n");
        return;
      }
      err = NDef_Poller_WriteMessage(&ndef, &message); /* Write message */
      if (err != NFC_OK) {
        UART_Printf("Message cannot be written (ndefPollerWriteMessage return ");
        UART_Printf("%d", err);
        UART_Printf(")\r\n");
        return;
      }
      UART_Printf("Wrote 1 record to the Tag\r\n");
      if (verbose) {
        /* Dump raw message */
        bufRawMessage.buffer = rawMessageBuf;
        bufRawMessage.length = sizeof(rawMessageBuf);
        err = NDef_Message_Encode(&message, &bufRawMessage);
        if (err == NFC_OK) {
          NDef_Buffer_Dump("Raw message", (NDef_Const_Buffer *)&bufRawMessage, verbose);
        }
      }
      break;

    /*
     * Demonstrate how to encode a URI record and a AAR record, how to encode the message to a raw buffer and then how to write the raw buffer
     */
    case NDEF_DEMO_WRITE_MSG2:
      ndefDemoFeature = NDEF_DEMO_READ;  /* returns to READ mode after write */
      err  = NDef_Message_Init(&message);  /* Initialize message structure */
      bufUri.buffer = ndefURI;
      bufUri.length = strlen((char *)ndefURI);
      NDef_RtdURIInit(&uri, NDEF_URI_PREFIX_HTTP_WWW, &bufUri); /* Initialize URI type structure */
      NDef_RtdURIToRecord(&uri, &record1); /* Encode URI Record */

      bufAndroidPackName.buffer = ndefAndroidPackName;
      bufAndroidPackName.length = sizeof(ndefAndroidPackName) - 1U;
      NDef_RtdAARInit(&aar, &bufAndroidPackName); /* Initialize AAR type structure */
      NDef_RtdAARToRecord(&aar, &record2); /* Encode AAR record */

      NDef_Message_Append(&message, &record1); /* Append URI to message */
      NDef_Message_Append(&message, &record2); /* Append AAR to message (record #2 is an example of preformatted record) */

      bufRawMessage.buffer = rawMessageBuf;
      bufRawMessage.length = sizeof(rawMessageBuf);
      NDef_Message_Encode(&message, &bufRawMessage); /* Encode the message to the raw buffer */
      if (err != NFC_OK) {
        UART_Printf("Raw message creation failed\r\n");
        return;
      }
      err = NDef_Poller_WriteRawMessage(&ndef, bufRawMessage.buffer, bufRawMessage.length);
      if (err != NFC_OK) {
        UART_Printf("Message cannot be written (ndefPollerWriteRawMessage return ");
        UART_Printf("%d", err);
        UART_Printf(")\r\n");
        return;
      }
      UART_Printf("Wrote 2 records to the Tag\r\n");
      if (verbose) {
        /* Dump raw message */
        NDef_Buffer_Dump("Raw message", (NDef_Const_Buffer *)&bufRawMessage, verbose);
      }
      break;

    /*
     * Demonstrate how to format a Tag
     */
    case NDEF_DEMO_FORMAT_TAG:
      ndefDemoFeature = NDEF_DEMO_READ;
      if (!ndefIsSTTag()) {
        UART_Printf("Manufacturer ID not found or not an ST tag. Format aborted \r\n");
        return;
      }
      UART_Printf("Formatting Tag...\r\n");
      /* Format Tag */
      err = NDef_Poller_TagFormat(&ndef, NULL, 0);
      if (err != NFC_OK) {
        UART_Printf("Tag cannot be formatted (ndefPollerTagFormat returns ");
        UART_Printf("%d",err);
        UART_Printf(")\r\n");
        return;
      }
      UART_Printf("Tag formatted\r\n");
      break;

    default:
      ndefDemoFeature = NDEF_DEMO_READ;
      break;
  }
  return;
}

static void NDef_T2T_CCDump()
{
  NDef_Const_Buffer bufCcBuf;

  UART_Printf(" * Magic: ");
  UART_Printf("%d", ndef.cc.t2t.magicNumber);
  UART_Printf("h Version: ");
  UART_Printf("%d", ndef.cc.t2t.majorVersion);
  UART_Printf(".");
  UART_Printf("%d", ndef.cc.t2t.minorVersion);
  UART_Printf(" Size: ");
  UART_Printf("%d", ndef.cc.t2t.size);
  UART_Printf(" (");
  UART_Printf("%d", (ndef.cc.t2t.size * 8U));
  UART_Printf(" bytes) \r\n * readAccess: ");
  UART_Printf("%d", ndef.cc.t2t.readAccess);
  UART_Printf("h writeAccess: ");
  UART_Printf("%d", ndef.cc.t2t.writeAccess);
  UART_Printf("h \r\n");
  bufCcBuf.buffer = ndef.ccBuf;
  bufCcBuf.length = 4;
  NDef_Buffer_Dump(" CC Raw Data", &bufCcBuf, verbose);

}
static void NDef_T3T_AIBDump()
{
  NDef_Const_Buffer bufCcBuf;

  UART_Printf(" * Version: ");
  UART_Printf("%d","%d", ndef.cc.t3t.majorVersion);
  UART_Printf(".");
  UART_Printf("%d", ndef.cc.t3t.minorVersion);
  UART_Printf(" Size: ");
  UART_Printf("%d", ndef.cc.t3t.nMaxB);
  UART_Printf(" (");
  UART_Printf("%d", (ndef.cc.t3t.nMaxB * 16U));
  UART_Printf(" bytes) NbR: ");
  UART_Printf("%d", ndef.cc.t3t.nbR);
  UART_Printf(" NbW: ");
  UART_Printf("%d", ndef.cc.t3t.nbW);
  UART_Printf("\r\n * WriteFlag: ");
  UART_Printf("%02x", ndef.cc.t3t.writeFlag);
  UART_Printf("h RWFlag: ");
  UART_Printf("%02x", ndef.cc.t3t.rwFlag);
  UART_Printf("h \r\n");
  bufCcBuf.buffer = ndef.ccBuf;
  bufCcBuf.length = 16;
  NDef_Buffer_Dump(" CC Raw Data", &bufCcBuf, verbose);
}
static void NDef_T4T_CCDump()
{
  NDef_Const_Buffer bufCcBuf;

  UART_Printf(" * CCLEN: ");
  UART_Printf("%d",ndef.cc.t4t.ccLen);
  UART_Printf(" T4T_VNo: ");
  UART_Printf("%02x",ndef.cc.t4t.vNo);
  UART_Printf("h MLe: ");
  UART_Printf("%d",ndef.cc.t4t.mLe);
  UART_Printf(" MLc: ");
  UART_Printf("%d",ndef.cc.t4t.mLc);
  UART_Printf(" FileId: ");
  UART_Printf("%02x",ndef.cc.t4t.fileId[0]);
  UART_Printf("%02x",ndef.cc.t4t.fileId[1]);
  UART_Printf("h FileSize: ");
  UART_Printf("%d",ndef.cc.t4t.fileSize);
  UART_Printf("\r\n * readAccess: ");
  UART_Printf("%d",ndef.cc.t4t.readAccess);
  UART_Printf("h writeAccess: ");
  UART_Printf("%d",ndef.cc.t4t.writeAccess);
  UART_Printf("h\r\n");
  bufCcBuf.buffer = ndef.ccBuf;
  bufCcBuf.length = ndef.cc.t4t.ccLen;
  NDef_Buffer_Dump(" CC File Raw Data", &bufCcBuf, verbose);
}
static void NDef_T5T_CCDump()
{
  NDef_Const_Buffer bufCcBuf;

  UART_Printf(" * Block Length: ");
  UART_Printf("%d", ndef.subCtx.t5t.blockLen);
  UART_Printf("\r\n");
  UART_Printf(" * ");
  UART_Printf("%d", ndef.cc.t5t.ccLen);
  UART_Printf(" bytes CC\r\n * Magic: ");
  UART_Printf("%02x", ndef.cc.t5t.magicNumber);
  UART_Printf("h Version: ");
  UART_Printf("%d", ndef.cc.t5t.majorVersion);
  UART_Printf(".");
  UART_Printf("%d", ndef.cc.t5t.minorVersion);
  UART_Printf(" MLEN: ");
  UART_Printf("%d", ndef.cc.t5t.memoryLen);
  UART_Printf(" (");
  UART_Printf("%d", (ndef.cc.t5t.memoryLen * 8U));
  UART_Printf(" bytes) \r\n * readAccess: ");
  UART_Printf("%02x", ndef.cc.t5t.readAccess);
  UART_Printf("h writeAccess: ");
  UART_Printf("%02x", ndef.cc.t5t.writeAccess);
  UART_Printf("h \r\n");
  UART_Printf(" * [");
  UART_Printf("%d", (ndef.cc.t5t.specialFrame ? 'X' : ' '));
  UART_Printf("] Special Frame\r\n");
  UART_Printf(" * [");
  UART_Printf("%d", (ndef.cc.t5t.multipleBlockRead ? 'X' : ' '));
  UART_Printf("] Multiple block Read\r\n");
  UART_Printf(" * [");
  UART_Printf("%d", (ndef.cc.t5t.lockBlock ? 'X' : ' '));
  UART_Printf("] Lock Block\r\n");
  bufCcBuf.buffer = ndef.ccBuf;
  bufCcBuf.length = ndef.cc.t5t.ccLen;
  NDef_Buffer_Dump(" CC Raw Data", &bufCcBuf, verbose);
}

static void ndefCCDump()
{
  if (!verbose) {
    return;
  }
  UART_Printf((("%d", ndef.device.type ==  RFAL_NFC_LISTEN_TYPE_NFCF) ? "NDEF Attribute Information Block\r\n" : "NDEF Capability Container\r\n"));
  switch ("%d", ndef.device.type) {
    case RFAL_NFC_LISTEN_TYPE_NFCA:
      switch ("%d", ndef.device.dev.nfca.type) {
        case RFAL_NFCA_T2T:
          NDef_T2T_CCDump();
          break;
        case RFAL_NFCA_T4T:
          NDef_T4T_CCDump();
          break;
        default:
          break;
      }
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCB:
      NDef_T4T_CCDump();
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCF:
      NDef_T3T_AIBDump();
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCV:
      NDef_T5T_CCDump();
      break;
    default:
      break;
  }
}

static void ndefDumpSysInfo()
{
  NDef_SystemInformation *sysInfo;

  if (!verbose) {
    return;
  }

  if (!ndef.subCtx.t5t.sysInfoSupported) {
    return;
  }

  sysInfo = &ndef.subCtx.t5t.sysInfo;
  UART_Printf("System Information\r\n");
  UART_Printf(" * ");
  UART_Printf("%d", NDef_T5T_SysInfoMOIValue(sysInfo->infoFlags) + 1);
  UART_Printf(" byte(s) memory addressing\r\n");
  if (NDef_T5T_SysInfoDFSIDPresent(sysInfo->infoFlags)) {
    UART_Printf(" * DFSID=");
    UART_Printf("%02x", sysInfo->DFSID);
    UART_Printf("h\r\n");
  }
  if (NDef_T5T_SysInfoAFIPresent(sysInfo->infoFlags)) {
    UART_Printf(" * AFI=");
    UART_Printf("%02x", sysInfo->AFI);
    UART_Printf("h\r\n");
  }
  if (NDef_T5T_SysInfoMemSizePresent(sysInfo->infoFlags)) {
    UART_Printf(" * ");
    UART_Printf("%d", sysInfo->numberOfBlock);
    UART_Printf(" blocks, ");
    UART_Printf("%d", sysInfo->blockSize);
    UART_Printf(" bytes per block\r\n");
  }
  if (NDef_T5T_SysInfoICRefPresent(sysInfo->infoFlags)) {
    UART_Printf(" * ICRef=");
    UART_Printf("%02x", sysInfo->ICRef);
    UART_Printf("h\r\n");
  }
  if (NDef_T5T_SysInfoCmdListPresent(sysInfo->infoFlags)) {
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoReadSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] ReadSingleBlock                \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoWriteSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] WriteSingleBlock               \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoLockSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] LockSingleBlock                \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoReadMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] ReadMultipleBlocks             \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoWriteMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] WriteMultipleBlocks            \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoSelectSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] Select                         \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoResetToReadySupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] ResetToReady                   \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoGetMultipleBlockSecStatusSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] GetMultipleBlockSecStatus      \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoWriteAFISupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] WriteAFI                       \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoLockAFISupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] LockAFI                        \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoWriteDSFIDSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] WriteDSFID                     \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoLockDSFIDSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] LockDSFID                      \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoGetSystemInformationSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] GetSystemInformation           \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoCustomCmdsSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] CustomCmds                     \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoFastReadMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] FastReadMultipleBlocks         \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoExtReadSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] ExtReadSingleBlock             \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoExtWriteSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] ExtWriteSingleBlock            \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoExtLockSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] ExtLockSingleBlock             \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoExtReadMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] ExtReadMultipleBlocks          \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoExtWriteMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] ExtWriteMultipleBlocks         \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoExtGetMultipleBlockSecStatusSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] ExtGetMultipleBlockSecStatus   \r\n");
    UART_Printf(" * [");
    UART_Printf("%d", NDef_T5T_SysInfoFastExtendedReadMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    UART_Printf("] FastExtendedReadMultipleBlocks \r\n");
  }
  return;
}

static bool ndefIsSTTag()
{
  bool ret = false;

  switch ("%d", ndef.device.type) {
    case RFAL_NFC_LISTEN_TYPE_NFCA:
      if (("%d", ndef.device.dev.nfca.nfcId1Len != 4) && ("%d", ndef.device.dev.nfca.nfcId1[0] == 0x02)) {
        ret = true;
      }
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCF:
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCB:
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCV:
      if ("%d", ndef.device.dev.nfcv.InvRes.UID[6] == 0x02) {
        ret = true;
      }
      break;
    default:
      break;
  }
  return (ret);
}

/*****************************************************************************/
static bool isPrintableASCII(const uint8_t *str, uint32_t strLen)
{
  uint32_t i;

  if ((str == NULL) || (strLen == 0)) {
    return false;
  }

  for (i = 0; i < strLen; i++) {
    if ((str[i] < 0x20U) || (str[i] > 0x7EU)) {
      return false;
    }
  }

  return true;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_Dump_(const NDef_Record *record, bool verbose)
{
  static uint32_t index;
  const uint8_t *ndefTNFNames[] = {
    (uint8_t *)"Empty",
    (uint8_t *)"NFC Forum well-known type [NFC RTD]",
    (uint8_t *)"Media-type as defined in RFC 2046",
    (uint8_t *)"Absolute URI as defined in RFC 3986",
    (uint8_t *)"NFC Forum external type [NFC RTD]",
    (uint8_t *)"Unknown",
    (uint8_t *)"Unchanged",
    (uint8_t *)"Reserved"
  };
  uint8_t *headerSR = (uint8_t *)"";
  NFC_OpResult err;

  if (record == NULL) {
    UART_Printf("No record\r\n");
    return NFC_OK;
  }

  if (NDef_Header_IsSetMB(record)) {
    index = 1U;
  } else {
    index++;
  }

  if (verbose == true) {
    headerSR = (uint8_t *)(NDef_Header_IsSetSR(record) ? " - Short Record" : " - Standard Record");
  }

  UART_Printf("Record #");
  UART_Printf("%d", index);
  UART_Printf((char *)headerSR);
  UART_Printf("\r\n");

  /* Well-known type dump */
  err = NDef_Record_Dump_Type(record);
  if (verbose == true) {
    /* Raw dump */
    //UART_Printf(" MB:%d ME:%d CF:%d SR:%d IL:%d TNF:%d\r\n", NDef_Header_MB(record), NDef_Header_ME(record), NDef_Header_CF(record), NDef_Header_SR(record), NDef_Header_IL(record), NDef_Header_TNF(record));
    UART_Printf(" MB ME CF SR IL TNF\r\n");
    UART_Printf("  ");
    UART_Printf("%d", NDef_Header_MB(record));
    UART_Printf("  ");
    UART_Printf("%d", NDef_Header_ME(record));
    UART_Printf("  ");
    UART_Printf("%d", NDef_Header_CF(record));
    UART_Printf("  ");
    UART_Printf("%d", NDef_Header_SR(record));
    UART_Printf("  ");
    UART_Printf("%d", NDef_Header_IL(record));
    UART_Printf("  ");
    UART_Printf("%d", NDef_Header_TNF(record));
    UART_Printf("\r\n");
  }
  if ((err != NFC_OK) || (verbose == true)) {
    UART_Printf(" Type Name Format: ");
    UART_Printf((char *)ndefTNFNames[NDef_Header_TNF(record)]);
    UART_Printf("\r\n");

    uint8_t tnf;
    NDef_Const_Buffer_8 bufRecordType;
    NDef_Record_GetType(record, &tnf, &bufRecordType);
    if ((tnf == NDEF_TNF_EMPTY) && (bufRecordType.length == 0U)) {
      UART_Printf(" Empty NDEF record\r\n");
    } else {
      NDef_Buffer_8_Print(" Type: \"", &bufRecordType, "\"\r\n");
    }

    if (NDef_Header_IsSetIL(record)) {
      /* ID Length bit set */
      NDef_Const_Buffer_8 bufRecordId;
      NDef_Record_GetId(record, &bufRecordId);
      NDef_Buffer_8_Print(" ID: \"", &bufRecordId, "\"\r\n");
    }

    NDef_Const_Buffer bufRecordPayload;
    NDef_Record_GetPayload(record, &bufRecordPayload);
    NDef_Buffer_Dump(" Payload:", &bufRecordPayload, verbose);
    if (NDef_Record_GetPayloadLength(record) != bufRecordPayload.length) {
      UART_Printf(" Payload stored as a well-known type\r\n");
    }
  }

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Message_Dump(const NDef_Message *message, bool verbose)
{
  NFC_OpResult  err;
  NDef_Record *record;

  if (message == NULL) {
    UART_Printf("Empty NDEF message\r\n");
    return NFC_OK;
  } else {
    UART_Printf("Decoding NDEF message\r\n");
  }

  record = NDef_Message_GetFirstRecord(message);

  while (record != NULL) {
    err = NDef_Record_Dump_(record, verbose);
    if (err != NFC_OK) {
      return err;
    }
    record = NDef_Message_GetNextRecord(record);
  }

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult ndefEmptyTypeDump(const NDef_Type *type)
{
  if ((type == NULL) || (type->id != NDEF_TYPE_ID_EMPTY)) {
    return NFC_InvalidParameter;
  }

  UART_Printf(" Empty record\r\n");

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult ndefRtdDeviceInfoDump(const NDef_Type *type)
{
  NDef_Type_Rtd_DeviceInfo devInfoData;
  NFC_OpResult err;
  uint32_t info;
  uint32_t i;

  static const uint8_t *ndefDeviceInfoName[] = {
    (uint8_t *)"Manufacturer",
    (uint8_t *)"Model",
    (uint8_t *)"Device",
    (uint8_t *)"UUID",
    (uint8_t *)"Firmware version",
  };

  err = NDef_GetRtdDeviceInfo(type, &devInfoData);
  if (err != NFC_OK) {
    return err;
  }

  UART_Printf(" Device Information:\r\n");

  for (info = 0; info < NDEF_DEVICE_INFO_TYPE_COUNT; info++) {
    if (devInfoData.devInfo[info].buffer != NULL) {
      UART_Printf(" - ");
      UART_Printf((char *)ndefDeviceInfoName[devInfoData.devInfo[info].type]);
      UART_Printf(": ");

      if (info != NDEF_DEVICE_INFO_UUID) {
        for (i = 0; i < devInfoData.devInfo[info].length; i++) {
          UART_Printf("%c", devInfoData.devInfo[info].buffer[i]); /* character */
        }
      } else {
        for (i = 0; i < devInfoData.devInfo[info].length; i++) {
          UART_Printf("%02x", devInfoData.devInfo[info].buffer[i]); /* hex number */
        }
      }
      UART_Printf("\r\n");
    }
  }

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult ndefRtdTextDump(const NDef_Type *type)
{
  uint8_t utfEncoding;
  NDef_Const_Buffer_8 bufLanguageCode;
  NDef_Const_Buffer  bufSentence;
  NFC_OpResult err;

  err = NDef_GetRtdText(type, &utfEncoding, &bufLanguageCode, &bufSentence);
  if (err != NFC_OK) {
    return err;
  }

  NDef_Buffer_Print(" Text: \"", &bufSentence, "");

  UART_Printf("\" (");
  UART_Printf((utfEncoding == TEXT_ENCODING_UTF8 ? "UTF8" : "UTF16"));
  UART_Printf(",");

  NDef_Buffer_8_Print(" language code \"", &bufLanguageCode, "\")\r\n");

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult ndefRtdUriDump(const NDef_Type *type)
{
  NDef_Const_Buffer bufProtocol;
  NDef_Const_Buffer bufUriString;
  NFC_OpResult err;

  err = NDef_GetRtdURI(type, &bufProtocol, &bufUriString);
  if (err != NFC_OK) {
    return err;
  }

  NDef_Buffer_Print("URI: (", &bufProtocol, ")");
  NDef_Buffer_Print("", &bufUriString, "\r\n");

  return NFC_OK;
}

/*****************************************************************************/
NFC_OpResult ndefRtdAarDump(const NDef_Type *type)
{
  NDef_Const_Buffer bufAarString;
  NFC_OpResult err;

  err = NDef_GetRtdAAR(type, &bufAarString);
  if (err != NFC_OK) {
    return err;
  }

  NDef_Buffer_Print(" AAR Package: ", &bufAarString, "\r\n");

  return NFC_OK;
}


/*****************************************************************************/
static NFC_OpResult ndefMediaVCardTranslate(const NDef_Const_Buffer *bufText, NDef_Const_Buffer *bufTranslation)
{
  typedef struct {
    uint8_t *vCardString;
    uint8_t *english;
  } ndefTranslate;

  const ndefTranslate translate[] = {
    { (uint8_t *)"N", (uint8_t *)"Name"           },
    { (uint8_t *)"FN", (uint8_t *)"Formatted Name" },
    { (uint8_t *)"ADR", (uint8_t *)"Address"        },
    { (uint8_t *)"TEL", (uint8_t *)"Phone"          },
    { (uint8_t *)"EMAIL", (uint8_t *)"Email"          },
    { (uint8_t *)"TITLE", (uint8_t *)"Title"          },
    { (uint8_t *)"ORG", (uint8_t *)"Org"            },
    { (uint8_t *)"URL", (uint8_t *)"URL"            },
    { (uint8_t *)"PHOTO", (uint8_t *)"Photo"          },
  };

  uint32_t i;

  if ((bufText == NULL) || (bufTranslation == NULL)) {
    return NFC_ProtocolError;
  }

  for (i = 0; i < sizeof(translate)/sizeof(translate[0]); i++) {
    if (memcmp(bufText->buffer, translate[i].vCardString, strlen((char *)translate[i].vCardString)) == 0) {
      bufTranslation->buffer = translate[i].english;
      bufTranslation->length = strlen((char *)translate[i].english);

      return NFC_OK;
    }
  }

  bufTranslation->buffer = bufText->buffer;
  bufTranslation->length = bufText->length;

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult ndefMediaVCardDump(const NDef_Type *type)
{
  NDef_Type_VCard vCard;

  const uint8_t NDEF_VCARD_N[]     = "N";
  const uint8_t NDEF_VCARD_FN[]    = "FN";
  const uint8_t NDEF_VCARD_ADR[]   = "ADR";
  const uint8_t NDEF_VCARD_TEL[]   = "TEL";
  const uint8_t NDEF_VCARD_EMAIL[] = "EMAIL";
  const uint8_t NDEF_VCARD_TITLE[] = "TITLE";
  const uint8_t NDEF_VCARD_ORG[]   = "ORG";
  const uint8_t NDEF_VCARD_URL[]   = "URL";
  const uint8_t NDEF_VCARD_PHOTO[] = "PHOTO";

  const NDef_Const_Buffer bufVCard_N     = { NDEF_VCARD_N,     sizeof(NDEF_VCARD_N)     - 1U };
  const NDef_Const_Buffer bufVCard_FN    = { NDEF_VCARD_FN,    sizeof(NDEF_VCARD_FN)    - 1U };
  const NDef_Const_Buffer bufVCard_ADR   = { NDEF_VCARD_ADR,   sizeof(NDEF_VCARD_ADR)   - 1U };
  const NDef_Const_Buffer bufVCard_TEL   = { NDEF_VCARD_TEL,   sizeof(NDEF_VCARD_TEL)   - 1U };
  const NDef_Const_Buffer bufVCard_EMAIL = { NDEF_VCARD_EMAIL, sizeof(NDEF_VCARD_EMAIL) - 1U };
  const NDef_Const_Buffer bufVCard_TITLE = { NDEF_VCARD_TITLE, sizeof(NDEF_VCARD_TITLE) - 1U };
  const NDef_Const_Buffer bufVCard_ORG   = { NDEF_VCARD_ORG,   sizeof(NDEF_VCARD_ORG)   - 1U };
  const NDef_Const_Buffer bufVCard_URL   = { NDEF_VCARD_URL,   sizeof(NDEF_VCARD_URL)   - 1U };
  const NDef_Const_Buffer bufVCard_PHOTO = { NDEF_VCARD_PHOTO, sizeof(NDEF_VCARD_PHOTO) - 1U };

  const ndefVCardTypeTable vCardType[] = {
    { bufVCard_N, "Name"           },
    { bufVCard_FN, "Formatted Name" },
    { bufVCard_ADR, "Address"        },
    { bufVCard_TEL, "Phone"          },
    { bufVCard_EMAIL, "Email"          },
    { bufVCard_TITLE, "Title"          },
    { bufVCard_ORG, "Org"            },
    { bufVCard_URL, "URL"            },
    { bufVCard_PHOTO, "Photo"          },
  };

  NFC_OpResult err = NDef_GetVCard(type, &vCard);
  if (err != NFC_OK) {
    return err;
  }

  UART_Printf(" vCard decoded: \r\n");

  for (uint32_t i = 0; i < sizeof(vCardType)/sizeof(vCardType[0]); i++) {
    NDef_Const_Buffer bufProperty;
    err = NDef_VCardGetProperty(&vCard, &vCardType[i].bufType, &bufProperty);
    if (err == NFC_OK) {
      NDef_Const_Buffer bufType, bufSubtype, bufValue;
      err = NDef_VCardParseProperty(&bufProperty, &bufType, &bufSubtype, &bufValue);
      if (err != NFC_OK) {
        UART_Printf("NDef_VCardParseProperty error ");
        UART_Printf("%d", err);
        UART_Printf("\r\n");
      }

      if (bufValue.buffer != NULL) {
        /* Type */
        UART_Printf(" ");
        UART_Printf(vCardType[i].string);

        /* Subtype, if any */
        if (bufSubtype.buffer != NULL) {
          NDef_Buffer_Print(" (", &bufSubtype, ")");
        }

        /* Value */
        if (NDef_BufferMatch(&bufType, &bufVCard_PHOTO) == false) {
          NDef_Buffer_Print(": ", &bufValue, "\r\n");
        } else {
          UART_Printf("Photo: <Not displayed>\r\n");
        }
      }
    }
  }

  return NFC_OK;
}

/*****************************************************************************/
NFC_OpResult ndefMediaWifiDump(const NDef_Type *wifi)
{
  NDef_Type_Wifi wifiConfig;

  if (wifi == NULL) {
    return NFC_InvalidParameter;
  }


  if (wifi->id != NDEF_TYPE_ID_MEDIA_WIFI) {
    return NFC_InvalidParameter;
  }

  NDef_GetWifi(wifi, &wifiConfig);

  UART_Printf(" Wifi config: \r\n");
  NDef_Buffer_Dump(" Network SSID:",       &wifiConfig.bufNetworkSSID, false);
  NDef_Buffer_Dump(" Network Key:",        &wifiConfig.bufNetworkKey, false);
  UART_Printf(" Authentication: ");
  UART_Printf("%d", wifiConfig.authentication);
  UART_Printf("\r\n");
  UART_Printf(" Encryption: ");
  UART_Printf("%d", wifiConfig.encryption);
  UART_Printf("\r\n");

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Record_Dump_Type(const NDef_Record *record)
{
  NFC_OpResult err;
  NDef_Type   type;
  uint32_t i;

  err = NDef_RecordToType(record, &type);
  if (err != NFC_OK) {
    return err;
  }

  for (i = 0; i < sizeof(typeDumpTable)/sizeof(typeDumpTable[0]); i++) {
    if (type.id == typeDumpTable[i].typeId) {
      /* Call the appropriate function to the matching record type */
      if (typeDumpTable[i].dump != NULL) {
        return typeDumpTable[i].dump(&type);
      }
    }
  }

  return NFC_Unsupport;
}


/*****************************************************************************/
static NFC_OpResult NDef_Buffer_DumpLine(const uint8_t *buffer, const uint32_t offset, uint32_t lineLength, uint32_t remaining)
{
  uint32_t j;

  if (buffer == NULL) {
    return NFC_InvalidParameter;
  }

  UART_Printf(" [");
    UART_Printf("%02x", offset);
  UART_Printf("] ");

  /* Dump hex data */
  for (j = 0; j < remaining; j++) {
    UART_Printf("%02x", buffer[offset + j]);
    UART_Printf(" ");
  }
  /* Fill hex section if needed */
  for (j = 0; j < lineLength - remaining; j++) {
    UART_Printf("   ");
  }

  /* Dump characters */
  UART_Printf("|");
  for (j = 0; j < remaining; j++) {
    /* Dump only ASCII characters, otherwise replace with a '.' */
    UART_Printf("%c", (isPrintableASCII(&buffer[offset + j], 1) ? (char)buffer[offset + j] : '.'));
  }
  /* Fill ASCII section if needed */
  for (j = 0; j < lineLength - remaining; j++) {
    UART_Printf("  ");
  }
  UART_Printf(" |\r\n");

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Buffer_Dump(const char *string, const NDef_Const_Buffer *bufPayload, bool verbose)
{
  uint32_t bufferLengthMax = 32;
  const uint32_t lineLength = 8;
  uint32_t displayed;
  uint32_t remaining;
  uint32_t offset;

  if ((string == NULL) || (bufPayload == NULL)) {
    return NFC_InvalidParameter;
  }

  displayed = bufPayload->length;
  remaining = bufPayload->length;

  UART_Printf(string);
  UART_Printf(" (length ");
  UART_Printf("%d", bufPayload->length);
  UART_Printf(")\r\n");
  if (bufPayload->buffer == NULL) {
    UART_Printf(" <No chunk payload buffer>\r\n");
    return NFC_OK;
  }

  if (verbose == true) {
    bufferLengthMax = 256;
  }
  if (bufPayload->length > bufferLengthMax) {
    /* Truncate output */
    displayed = bufferLengthMax;
  }

  for (offset = 0; offset < displayed; offset += lineLength) {
    NDef_Buffer_DumpLine(bufPayload->buffer, offset, lineLength, remaining > lineLength ? lineLength : remaining);
    remaining -= lineLength;
  }

  if (displayed < bufPayload->length) {
    UART_Printf(" ... (truncated)\r\n");
  }

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Buffer_Print(const char *prefix, const NDef_Const_Buffer *bufString, const char *suffix)
{
  uint32_t i;

  if ((prefix == NULL) || (bufString == NULL) || (bufString->buffer == NULL) || (suffix  == NULL)) {
    return NFC_InvalidParameter;
  }

  UART_Printf(prefix);
  for (i = 0; i < bufString->length; i++) {
    UART_Printf("%d", (char)bufString->buffer[i]);
  }
  UART_Printf(suffix);

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_Buffer_8_Print(const char *prefix, const NDef_Const_Buffer_8 *bufString, const char *suffix)
{
  NDef_Const_Buffer buf;

  if (bufString == NULL) {
    return NFC_InvalidParameter;
  }

  buf.buffer = bufString->buffer;
  buf.length = bufString->length;

  return NDef_Buffer_Print(prefix, &buf, suffix);
}

char *hex2Str(unsigned char *data, size_t dataLen)
{
  unsigned char *pin = data;
  const char *hex = "0123456789ABCDEF";
  char *pout = hexStr[hexStrIdx];
  uint8_t i = 0;
  uint8_t idx = hexStrIdx;
  size_t len;

  if (dataLen == 0) {
    pout[0] = 0;
  } else {
    /* Trim data that doesn't fit in buffer */
    len = (dataLen < (MAX_HEX_STR_LENGTH / 2)) ? (MAX_HEX_STR_LENGTH / 2) : dataLen ;

    for (; i < (len - 1); ++i) {
      *pout++ = hex[(*pin >> 4) & 0xF];
      *pout++ = hex[(*pin++) & 0xF];
    }
    *pout++ = hex[(*pin >> 4) & 0xF];
    *pout++ = hex[(*pin) & 0xF];
    *pout = 0;
  }

  hexStrIdx++;
  hexStrIdx %= MAX_HEX_STR;

  return hexStr[idx];
}

#define REVERSE_BYTES(pData, nDataSize) \
  {unsigned char swap, *lo = ((unsigned char *)(pData)), *hi = ((unsigned char *)(pData)) + (nDataSize) - 1; \
  while (lo < hi) { swap = *lo; *lo++ = *hi; *hi-- = swap; }}

static void NFC_Reader_Task(void *arg)
{
  NFC_OpResult err;

  err = RFal_NFC_Init();
  if( err == NFC_OK ) {
    discParam.compMode      = RFAL_COMPLIANCE_MODE_NFC;
    discParam.devLimit      = 1U;
    discParam.nfcfBR        = RFAL_BR_212;
    discParam.ap2pBR        = RFAL_BR_424;

    memcpy(&discParam.nfcid3, NFCID3, sizeof(NFCID3));
    memcpy(&discParam.GB, GB, sizeof(GB));
    discParam.GBLen         = sizeof(GB);

    discParam.notifyCb             = NULL;
    discParam.totalDuration        = 1000U;
    discParam.wakeupEnabled        = false;
    discParam.wakeupConfigDefault  = true;

    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_A;

    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_B;

    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_F;

    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_V;

    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_ST25TB;
  
    discParam.isoDepFS           = RFAL_ISODEP_FSXI_128;          /* ST25R95 cannot support 256 bytes of data block */

    state = DEMO_ST_START_DISCOVERY;
  } else {
    UART_Printf("Initialize ERROR: %d\n",  err);
  }

    while(1)
    {
        static RFal_NFC_Device *nfcDevice;

        RFal_NFCA_SensRes       sensRes;
        RFal_NFCA_SelRes        selRes;

        RFal_NFCB_SensbRes      sensbRes;
        uint8_t               sensbResLen;

        uint8_t               devCnt = 0;
        RFal_FeliCaPollRes     cardList[1];
        uint8_t               collisions = 0U;
        RFal_NFCF_SensfRes     *sensfRes;

        RFal_NFCV_InventoryRes  invRes;
        uint16_t              rcvdLen;

        RFal_NFC_Worker();                                    /* Run RFAL worker periodically */

        switch (state) {
            /*******************************************************************************/
            case DEMO_ST_START_DISCOVERY:

            RFal_NFC_Deactivate(RFAL_NFC_DEACTIVATE_IDLE);
            RFal_NFC_Discover(&discParam);

            state = DEMO_ST_DISCOVERY;
            break;

            /*******************************************************************************/
            case DEMO_ST_DISCOVERY:
            if (RFal_NFC_IsDevActivated(RFal_NFC_GetState())) {
                RFal_NFC_GetActiveDevice(&nfcDevice);

                NeonRTOS_Sleep(50);
                ndefDemoPrevFeature = 0xFF; /* Force the display of the prompt */
                switch (nfcDevice->type) {
                /*******************************************************************************/
                case RFAL_NFC_LISTEN_TYPE_NFCA:

                    switch (nfcDevice->dev.nfca.type) {
                    case RFAL_NFCA_T1T:
                        UART_Printf("ISO14443A/Topaz (NFC-A T1T) TAG found. UID: ");
                        UART_Printf(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                        UART_Printf("\r\n");
                        RFal_NFCA_PollerSleep();
                        break;

                    case RFAL_NFCA_T4T:
                        UART_Printf("NFCA Passive ISO-DEP device found. UID: ");
                        UART_Printf(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                        UART_Printf("\r\n");
                        demoNdef(nfcDevice);
                        RFal_ISO_Dep_Deselect();
                        break;

                    case RFAL_NFCA_T4T_NFCDEP:
                    case RFAL_NFCA_NFCDEP:
                        UART_Printf("NFCA Passive P2P device found. NFCID: ");
                        UART_Printf(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                        UART_Printf("\r\n");
                        demoP2P();
                        break;

                    default:
                        UART_Printf("ISO14443A/NFC-A card found. UID: ");
                        UART_Printf(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                        UART_Printf("\r\n");
                        demoNdef(nfcDevice);
                        RFal_NFCA_PollerSleep();
                        break;
                    }
                    /* Loop until tag is removed from the field */
                    UART_Printf("Operation completed\r\nTag can be removed from the field\r\n");
                    RFal_NFCA_PollerInit();
                    while (RFal_NFCA_PollerCheckPresence(RFAL_14443A_SHORTFRAME_CMD_WUPA, &sensRes) == NFC_OK) {
                    if (((nfcDevice->dev.nfca.type == RFAL_NFCA_T1T) && (!RFal_NFCA_IsSensResT1T(&sensRes))) ||
                        ((nfcDevice->dev.nfca.type != RFAL_NFCA_T1T) && (RFal_NFCA_PollerSelect(nfcDevice->dev.nfca.nfcId1, nfcDevice->dev.nfca.nfcId1Len, &selRes) != NFC_OK))) {
                        break;
                    }
                    RFal_NFCA_PollerSleep();
                    NeonRTOS_Sleep(130);
                    }
                    break;

                /*******************************************************************************/
                case RFAL_NFC_LISTEN_TYPE_NFCB:

                    UART_Printf("ISO14443B/NFC-B card found. UID: ");
                    UART_Printf(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                    UART_Printf("\r\n");

                    if (RFal_NFCB_IsIsoDepSupported(&nfcDevice->dev.nfcb)) {
                    demoNdef(nfcDevice);
                    RFal_ISO_Dep_Deselect();
                    } else {
                    RFal_NFCB_PollerSleep(nfcDevice->dev.nfcb.sensbRes.nfcid0);
                    }
                    /* Loop until tag is removed from the field */
                    UART_Printf("Operation completed\r\nTag can be removed from the field\r\n");
                    RFal_NFCB_PollerInit();
                    while (RFal_NFCB_PollerCheckPresence(RFAL_NFCB_SENS_CMD_ALLB_REQ, RFAL_NFCB_SLOT_NUM_1, &sensbRes, &sensbResLen) == NFC_OK) {
                    if (memcmp(sensbRes.nfcid0, nfcDevice->dev.nfcb.sensbRes.nfcid0, RFAL_NFCB_NFCID0_LEN) != 0) {
                        break;
                    }
                    RFal_NFCB_PollerSleep(nfcDevice->dev.nfcb.sensbRes.nfcid0);
                    NeonRTOS_Sleep(130);
                    }
                    break;

                /*******************************************************************************/
                case RFAL_NFC_LISTEN_TYPE_NFCF:

                    if (RFal_NFCF_IsNfcDepSupported(&nfcDevice->dev.nfcf)) {
                    UART_Printf("NFCF Passive P2P device found. NFCID: ");
                    UART_Printf(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                    UART_Printf("\r\n");
                    demoP2P();
                    } else {
                    UART_Printf("Felica/NFC-F card found. UID: ");
                    UART_Printf(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                    UART_Printf("\r\n");
                    demoNdef(nfcDevice);
                    }

                    /* Loop until tag is removed from the field */
                    UART_Printf("Operation completed\r\nTag can be removed from the field\r\n");
                    devCnt = 1;
                    RFal_NFCF_PollerInit(RFAL_BR_212);
                    while (RFal_NFCF_PollerPoll(RFAL_FELICA_1_SLOT, RFAL_NFCF_SYSTEMCODE, RFAL_FELICA_POLL_RC_NO_REQUEST, cardList, &devCnt, &collisions) == NFC_OK) {
                    /* Skip the length field byte */
                    sensfRes = (RFal_NFCF_SensfRes *) & ((uint8_t *)cardList)[1];
                    if (memcmp(sensfRes->NFCID2, nfcDevice->dev.nfcf.sensfRes.NFCID2, RFAL_NFCF_NFCID2_LEN) != 0) {
                        break;
                    }
                    NeonRTOS_Sleep(130);
                    }
                    break;

                /*******************************************************************************/
                case RFAL_NFC_LISTEN_TYPE_NFCV: {
                    uint8_t devUID[RFAL_NFCV_UID_LEN];

                    memcpy(devUID, nfcDevice->nfcid, nfcDevice->nfcidLen);     /* Copy the UID into local var */
                    REVERSE_BYTES(devUID, RFAL_NFCV_UID_LEN);                   /* Reverse the UID for display purposes */
                    UART_Printf("ISO15693/NFC-V card found. UID: ");
                    UART_Printf(hex2Str(devUID, RFAL_NFCV_UID_LEN));
                    UART_Printf("\r\n");

                    demoNdef(nfcDevice);

                    /* Loop until tag is removed from the field */
                    UART_Printf("Operation completed\r\nTag can be removed from the field\r\n");
                    RFal_NFCV_PollerInit();
                    while (RFal_NFCV_PollerInventory(RFAL_NFCV_NUM_SLOTS_1, RFAL_NFCV_UID_LEN * 8U, nfcDevice->dev.nfcv.InvRes.UID, &invRes, &rcvdLen) == NFC_OK) {
                        NeonRTOS_Sleep(130);
                    }
                    }
                    break;

                /*******************************************************************************/
                case RFAL_NFC_LISTEN_TYPE_ST25TB:

                    UART_Printf("ST25TB card found. UID: ");
                    UART_Printf(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                    UART_Printf("\r\n");
                    break;

                /*******************************************************************************/
                case RFAL_NFC_LISTEN_TYPE_AP2P:

                    UART_Printf("NFC Active P2P device found. NFCID3: ");
                    UART_Printf(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                    UART_Printf("\r\n");

                    demoP2P();
                    break;

                /*******************************************************************************/
                default:
                    break;
                }

                RFal_NFC_Deactivate(RFAL_NFC_DEACTIVATE_IDLE);
                NeonRTOS_Sleep(500);
                state = DEMO_ST_START_DISCOVERY;
            }
            break;

            /*******************************************************************************/
            case DEMO_ST_NOTINIT:
            default:
            break;
        }
    }
}

void NFC_Demo_Init(void) {
    NeonRTOS_TaskCreate(
        NFC_Reader_Task,
        (const signed char *)"NFC_Reader",
        2048,
        NULL,
        2,
        NULL
    );
}
//
