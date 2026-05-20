
/**
  ******************************************************************************
  * @file           : ndef_type_bluetooth.cpp
  * @brief          : NDEF Bluetooth type
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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "NDef_Record.h"
#include "NDef_Types.h"
#include "NDef_Type_Bluetooth.h"

#include "NFC/NFC_Def.h"

#if NDEF_TYPE_BLUETOOTH_SUPPORT

/*! Bluetooth Payload minimal length */
#define NDEF_BLUETOOTH_BREDR_PAYLOAD_LENGTH_MIN            8U
#define NDEF_BLUETOOTH_SECURE_LE_PAYLOAD_LENGTH_MIN        2U
#define NDEF_BLUETOOTH_PAYLOAD_LENGTH_MIN                  (MIN(NDEF_BLUETOOTH_BREDR_PAYLOAD_LENGTH_MIN, NDEF_BLUETOOTH_SECURE_LE_PAYLOAD_LENGTH_MIN))


/*! EIR length */
#define NDEF_BT_EIR_DEVICE_ADDRESS_SIZE                    6U
#define NDEF_BT_EIR_BLE_DEVICE_ADDRESS_SIZE                6U
#define NDEF_BT_EIR_DEVICE_CLASS_SIZE                      3U
#define NDEF_BT_EIR_SIMPLE_PAIRING_HASH_SIZE              16U
#define NDEF_BT_EIR_SIMPLE_PAIRING_RANDOMIZER_SIZE        16U
#define NDEF_BT_EIR_SECURE_CO_CONFIRMATION_VALUE_SIZE     16U
#define NDEF_BT_EIR_SECURE_CO_RANDOM_VALUE_SIZE           16U
#define NDEF_BT_EIR_SECURITY_MANAGER_TK_SIZE              16U
#define NDEF_BT_EIR_SLAVE_CONNECTION_INTERVAL_RANGE_SIZE  (2U * sizeof(uint16_t))


/*! Enable EIR length check while decoding payload */
#define NDEF_BLUETOOTH_CHECK_REFERENCE_LENGTH

/*! Encode 0-length data EIRs */
#define NDEF_BLUETOOTH_ENCODE_EMPTY_DATA_EIR

/*! EIR Length-Type-Data fields offsets */
#define NDEF_BT_EIR_LENGTH_OFFSET                   0U
#define NDEF_BT_EIR_TYPE_OFFSET                     1U
#define NDEF_BT_EIR_DATA_OFFSET                     2U

/*! Bluetooth Type strings */
static const uint8_t NDef_Type_BluetoothBrEdr[]       = "application/vnd.bluetooth.ep.oob";        /*!< Bluetooth BR/EDR Out-Of-Band Record Type            */
static const uint8_t NDef_Type_BluetoothLe[]          = "application/vnd.bluetooth.le.oob";        /*!< Bluetooth Low Energy Out-Of-Band Record Type        */
static const uint8_t NDef_Type_BluetoothSecureBrEdr[] = "application/vnd.bluetooth.secure.ep.oob"; /*!< Bluetooth Secure BR/EDR Out-Of-Band Record Type     */
static const uint8_t NDef_Type_BluetoothSecureLe[]    = "application/vnd.bluetooth.secure.le.oob"; /*!< Bluetooth Secure Low Energy Out-Of-Band Record type */

const NDef_Const_Buffer_8 bufMediaTypeBluetoothBrEdr       = { NDef_Type_BluetoothBrEdr,       sizeof(NDef_Type_BluetoothBrEdr) - 1U };       /*!< Bluetooth BR/EDR Record Type buffer            */
const NDef_Const_Buffer_8 bufMediaTypeBluetoothLe          = { NDef_Type_BluetoothLe,          sizeof(NDef_Type_BluetoothLe) - 1U };          /*!< Bluetooth Low Energy Record Type buffer        */
const NDef_Const_Buffer_8 bufMediaTypeBluetoothSecureBrEdr = { NDef_Type_BluetoothSecureBrEdr, sizeof(NDef_Type_BluetoothSecureBrEdr) - 1U }; /*!< Bluetooth Secure BR/EDR Record Type buffer     */
const NDef_Const_Buffer_8 bufMediaTypeBluetoothSecureLe    = { NDef_Type_BluetoothSecureLe,    sizeof(NDef_Type_BluetoothSecureLe) - 1U };    /*!< Bluetooth Secure Low Energy Record Type buffer */

/*****************************************************************************/
uint8_t NFC_Bluetooth_EirLength(const uint8_t *eir)
{
  if (eir == NULL) {
    return 0;
  }

  uint8_t length = eir[NDEF_BT_EIR_LENGTH_OFFSET];
  /* Check the EIR contains something */
  if (length != 0U) {
    /* Add the Length byte that is not included in the EIR length */
    length += (uint8_t)sizeof(uint8_t);
  }

  return length;
}


/*****************************************************************************/
uint8_t NFC_Bluetooth_EirDataLength(const uint8_t *eir)
{
  if (eir == NULL) {
    return 0;
  }

  uint8_t dataLength = eir[NDEF_BT_EIR_LENGTH_OFFSET];
  /* Check the EIR contains a type */
  if (dataLength > 0U) {
    dataLength -= (uint8_t)sizeof(uint8_t); /* Remove the EIR Type byte */
  }

  return dataLength;
}


/*****************************************************************************/
uint8_t NFC_Bluetooth_EirType(const uint8_t *eir)
{
  uint8_t type = 0;

  if (NFC_Bluetooth_EirLength(eir) != 0U) {
    type = eir[NDEF_BT_EIR_TYPE_OFFSET];
  }

  return type;
}


/*****************************************************************************/
const uint8_t *NFC_Bluetooth_EirData(const uint8_t *eir)
{
  const uint8_t *data = NULL;

  if (NFC_Bluetooth_EirDataLength(eir) != 0U) {
    data = &eir[NDEF_BT_EIR_DATA_OFFSET];
  }

  return data;
}


/*****************************************************************************/
NFC_OpResult NFC_Bluetooth_EirDataToBuffer(const uint8_t *eir, NDef_Const_Buffer *bufEir)
{
  if ((eir == NULL) || (bufEir == NULL)) {
    return NFC_InvalidParameter;
  }

  bufEir->buffer = NFC_Bluetooth_EirData(eir);
  bufEir->length = NFC_Bluetooth_EirDataLength(eir);

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NFC_Bluetooth_SetEir(NDef_Type_Bluetooth *bluetooth, const uint8_t *eir)
{
  if ((bluetooth == NULL) || (eir == NULL)) {
    return NFC_InvalidParameter;
  }

  /* Find first free EIR */
  for (uint32_t i = 0; i < (uint32_t)SIZEOF_ARRAY(bluetooth->eir); i++) {
    /* Append it */
    if (bluetooth->eir[i] == NULL) {
      bluetooth->eir[i] = eir;
      return NFC_OK;
    }
    /* Or update existing one */
    else if (NFC_Bluetooth_EirType(bluetooth->eir[i]) == NFC_Bluetooth_EirType(eir)) {
      bluetooth->eir[i] = eir;
      return NFC_OK;
    } else {
      /* MISRA 15.7 - Empty else */
    }
  }

  return NFC_MemoryError;
}


/*****************************************************************************/
const uint8_t *NFC_Bluetooth_GetEir(const NDef_Type_Bluetooth *bluetooth, uint8_t eirType)
{
  if (bluetooth == NULL) {
    return NULL;
  }

  /* Find EIR with this type */
  for (uint32_t i = 0; i < (uint32_t)SIZEOF_ARRAY(bluetooth->eir); i++) {
    if (NFC_Bluetooth_EirType(bluetooth->eir[i]) == eirType) {
      return bluetooth->eir[i];
    }
  }

  return NULL;
}


/*****************************************************************************/
/* This function copies an array, changing its endianness, useful to convert data to BLE endianness */
static uint8_t *NDEF_BluetoothReverse(uint8_t *dst, const uint8_t *src, uint32_t length)
{
  if ((dst == NULL) || (src == NULL)) {
    return NULL;
  }

  for (uint32_t i = 0; i < length; i++) {
    dst[i] = src[length - i - 1U];
  }

  return dst;
}


/*****************************************************************************/
NFC_OpResult NFC_Bluetooth_GetEirData(const NDef_Type_Bluetooth *bluetooth, uint8_t eirType, NDef_Const_Buffer *bufData)
{
  if ((bluetooth == NULL) || (bufData == NULL)) {
    return NFC_InvalidParameter;
  }

  const uint8_t *eir = NFC_Bluetooth_GetEir(bluetooth, eirType);

  return NFC_Bluetooth_EirDataToBuffer(eir, bufData);
}


/*****************************************************************************/
NFC_OpResult NFC_Bluetooth_GetEirDataReversed(const NDef_Type_Bluetooth *bluetooth, uint8_t eirType, NDef_Buffer *bufDataReversed)
{
  if ((bluetooth == NULL) || (bufDataReversed == NULL)) {
    return NFC_InvalidParameter;
  }

  const uint8_t *eir = NFC_Bluetooth_GetEir(bluetooth, eirType);

  uint32_t data_length = NFC_Bluetooth_EirDataLength(eir);
  if (data_length > bufDataReversed->length) {
    bufDataReversed->length = data_length;
    return NFC_MemoryError;
  }
  bufDataReversed->length = data_length;

  const uint8_t *eir_data = NFC_Bluetooth_EirData(eir);
  (void)NDEF_BluetoothReverse(bufDataReversed->buffer, eir_data, data_length);

  return NFC_OK;
}


/*****************************************************************************/
static uint32_t NFC_Bluetooth_PayloadGetLength(const NDef_Type *type)
{
  const NDef_Type_Bluetooth *ndefData;
  uint32_t length = 0;

  if ((type == NULL) ||
      ((type->id != NDEF_TYPE_ID_BLUETOOTH_BREDR)        &&
       (type->id != NDEF_TYPE_ID_BLUETOOTH_LE)           &&
       (type->id != NDEF_TYPE_ID_BLUETOOTH_SECURE_BREDR) &&
       (type->id != NDEF_TYPE_ID_BLUETOOTH_SECURE_LE))) {
    return 0;
  }

  ndefData = &type->data.bluetooth;

  /* For both BR/EDR and Secure LE */
  if ((type->id == NDEF_TYPE_ID_BLUETOOTH_BREDR) ||
      (type->id == NDEF_TYPE_ID_BLUETOOTH_SECURE_LE)) {
    length  = sizeof(uint16_t); /* 2 bytes for length */
  }

  /* For BR/EDR only, but no test needed because length is 0 in that case */
  length += ndefData->bufDeviceAddress.length;

  /* Go through all EIRs */
  for (uint32_t i = 0; i < (uint32_t)SIZEOF_ARRAY(ndefData->eir); i++) {
#ifdef NDEF_BLUETOOTH_ENCODE_EMPTY_DATA_EIR
    /* Send all/valid EIRs (even EIRs with data length == 0) */
#else
    /* Send EIRs with data length != 0U only */
    if (NFC_Bluetooth_GetEirDataLength(ndefDtata->eir[i]) != 0U)
#endif
    {
      length += NFC_Bluetooth_EirLength(ndefData->eir[i]);
    }
  }

  return length;
}


/*****************************************************************************/
NFC_OpResult NFC_Bluetooth_Reset(NDef_Type_Bluetooth *bluetooth)
{
  NDef_Const_Buffer bufEmpty = { NULL, 0 };

  if (bluetooth == NULL) {
    return NFC_InvalidParameter;
  }

  /* Initialize NDef_Buffer */
  bluetooth->bufDeviceAddress = bufEmpty;

  /* Initialize all EIRs */
  for (uint32_t i = 0; i < (uint32_t)SIZEOF_ARRAY(bluetooth->eir); i++) {
    bluetooth->eir[i] = NULL;
  }

  return NFC_OK;
}


/*****************************************************************************/
static const uint8_t *NFC_Bluetooth_ToPayloadItem(const NDef_Type *type, NDef_Const_Buffer *bufItem, bool begin)
{
  static uint32_t item = 0;
  static uint32_t eirId = 0;

  const NDef_Type_Bluetooth *ndefData;

  if ((type == NULL) || (bufItem == NULL) ||
      ((type->id != NDEF_TYPE_ID_BLUETOOTH_BREDR)        &&
       (type->id != NDEF_TYPE_ID_BLUETOOTH_LE)           &&
       (type->id != NDEF_TYPE_ID_BLUETOOTH_SECURE_BREDR) &&
       (type->id != NDEF_TYPE_ID_BLUETOOTH_SECURE_LE))) {
    return NULL;
  }

  ndefData = &type->data.bluetooth;

  bufItem->buffer = NULL;
  bufItem->length = 0;

  /* Initialization */
  if (begin == true) {
    item = 0;
    eirId = 0;
  }

  /* BR/EDR or Secure Low Energy */
  if ((type->id == NDEF_TYPE_ID_BLUETOOTH_BREDR) ||
      (type->id == NDEF_TYPE_ID_BLUETOOTH_SECURE_LE)) {
    if (item == 0U) {
      /* for BR-EDR and Secure LE, Device address & length are managed outside EIR */
      /* First item for NDEF_TYPE_ID_BLUETOOTH_BREDR: Payload length */
      static uint16_t len;
      len = (uint16_t)NFC_Bluetooth_PayloadGetLength(type);
      bufItem->buffer = (const uint8_t *)&len;
      bufItem->length = sizeof(uint16_t);

      item++;
      return bufItem->buffer;
    }
  }
  if (type->id == NDEF_TYPE_ID_BLUETOOTH_BREDR) {
    if (item == 1U) {
      /* for BR-EDR Device address & length are managed outside EIR */
      /* Second item for NDEF_TYPE_ID_BLUETOOTH_BREDR: Device Address */
      bufItem->buffer = ndefData->bufDeviceAddress.buffer;
      bufItem->length = ndefData->bufDeviceAddress.length;

      item++;
      return bufItem->buffer;
    }
  }

  /* Go through all EIRs */
  while (eirId < (uint32_t)SIZEOF_ARRAY(ndefData->eir)) {
#ifdef NDEF_BLUETOOTH_ENCODE_EMPTY_DATA_EIR
    /* Send all/valid EIRs (even EIRs with data length == 0) */
    if (NFC_Bluetooth_EirLength(ndefData->eir[eirId]) != 0U)
#else
    /* Send EIRs with data length != 0U only */
    if (NFC_Bluetooth_EirDataLength(ndefData->eir[eirId]) != 0U)
#endif
    {
      bufItem->buffer = (const uint8_t *)ndefData->eir[eirId];
      bufItem->length = NFC_Bluetooth_EirLength(ndefData->eir[eirId]);

      eirId++;
      return bufItem->buffer;
    }
    eirId++;
  }

  return bufItem->buffer;
}


/*****************************************************************************/
static NFC_OpResult NFC_Bluetooth_Init(NDef_Type *type, const NDef_Type_Bluetooth *bluetooth)
{
  NDef_Type_Bluetooth *ndefData;

  if ((type == NULL) || (bluetooth == NULL)) {
    return NFC_InvalidParameter;
  }

  /* type->id set by the caller */
  type->getPayloadLength = NFC_Bluetooth_PayloadGetLength;
  type->getPayloadItem   = NFC_Bluetooth_ToPayloadItem;
  type->typeToRecord     = NDef_BluetoothToRecord;
  ndefData               = &type->data.bluetooth;

  /* Copy in a bulk */
  (void)memcpy(ndefData, bluetooth, sizeof(NDef_Type_Bluetooth));

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NFC_Bluetooth_BrEdrInit(NDef_Type *type, const NDef_Type_Bluetooth *bluetooth)
{
  if (type == NULL) {
    return NFC_InvalidParameter;
  }

  /* Initialize a Basic Rate/Enhanced Data Rate type */
  type->id = NDEF_TYPE_ID_BLUETOOTH_BREDR;

  return NFC_Bluetooth_Init(type, bluetooth);
}


/*****************************************************************************/
NFC_OpResult NFC_Bluetooth_LeInit(NDef_Type *type, const NDef_Type_Bluetooth *bluetooth)
{
  if (type == NULL) {
    return NFC_InvalidParameter;
  }

  /* Initialize a Bluetooth Low Energy type */
  type->id = NDEF_TYPE_ID_BLUETOOTH_LE;

  return NFC_Bluetooth_Init(type, bluetooth);
}


/*****************************************************************************/
NFC_OpResult NFC_Bluetooth_SecureBrEdrInit(NDef_Type *type, const NDef_Type_Bluetooth *bluetooth)
{
  if (type == NULL) {
    return NFC_InvalidParameter;
  }

  /* Initialize a Secure BR/EDR type */
  type->id = NDEF_TYPE_ID_BLUETOOTH_SECURE_BREDR;

  return NFC_Bluetooth_Init(type, bluetooth);
}


/*****************************************************************************/
NFC_OpResult NFC_Bluetooth_SecureLeInit(NDef_Type *type, const NDef_Type_Bluetooth *bluetooth)
{
  if (type == NULL) {
    return NFC_InvalidParameter;
  }

  /* Initialize a Secure Low Energy type */
  type->id = NDEF_TYPE_ID_BLUETOOTH_SECURE_LE;

  return NFC_Bluetooth_Init(type, bluetooth);
}


/*****************************************************************************/
NFC_OpResult NDef_GetBluetooth(const NDef_Type *type, NDef_Type_Bluetooth *bluetooth)
{
  const NDef_Type_Bluetooth *ndefData;

  if ((type == NULL) || (bluetooth == NULL) ||
      ((type->id != NDEF_TYPE_ID_BLUETOOTH_BREDR)        &&
       (type->id != NDEF_TYPE_ID_BLUETOOTH_LE)           &&
       (type->id != NDEF_TYPE_ID_BLUETOOTH_SECURE_BREDR) &&
       (type->id != NDEF_TYPE_ID_BLUETOOTH_SECURE_LE))) {
    return NFC_InvalidParameter;
  }

  ndefData = &type->data.bluetooth;

  /* Copy in a bulk */
  (void)memcpy(bluetooth, ndefData, sizeof(NDef_Type_Bluetooth));

  return NFC_OK;
}


/*****************************************************************************/
#ifdef NDEF_BLUETOOTH_CHECK_REFERENCE_LENGTH
static uint32_t NFC_Bluetooth_EirRefLength(uint8_t eirType)
{
  uint32_t length;
  switch (eirType) {
    case NDEF_BT_EIR_FLAGS                            : length = sizeof(uint8_t)                                      ; break;
    case NDEF_BT_EIR_TX_POWER_LEVEL                   : length = sizeof(uint8_t)                                      ; break;
    case NDEF_BT_EIR_DEVICE_CLASS                     : length = NDEF_BT_EIR_DEVICE_CLASS_SIZE                        ; break;
    case NDEF_BT_EIR_SIMPLE_PAIRING_HASH              : length = NDEF_BT_EIR_SIMPLE_PAIRING_HASH_SIZE                 ; break;
    case NDEF_BT_EIR_SIMPLE_PAIRING_RANDOMIZER        : length = NDEF_BT_EIR_SIMPLE_PAIRING_RANDOMIZER_SIZE           ; break;
    case NDEF_BT_EIR_SECURITY_MANAGER_TK_VALUE        : length = NDEF_BT_EIR_SECURITY_MANAGER_TK_SIZE                 ; break;
    case NDEF_BT_EIR_SECURITY_MANAGER_FLAGS           : length = sizeof(uint8_t)                                      ; break;
    case NDEF_BT_EIR_SLAVE_CONNECTION_INTERVAL_RANGE  : length = NDEF_BT_EIR_SLAVE_CONNECTION_INTERVAL_RANGE_SIZE     ; break;
    case NDEF_BT_EIR_LE_DEVICE_ADDRESS                : length = NDEF_BT_EIR_BLE_DEVICE_ADDRESS_SIZE + sizeof(uint8_t); break;
    case NDEF_BT_EIR_LE_ROLE                          : length = sizeof(uint8_t)                                      ; break;
    case NDEF_BT_EIR_LE_SECURE_CONN_CONFIRMATION_VALUE: length = NDEF_BT_EIR_SECURE_CO_CONFIRMATION_VALUE_SIZE        ; break;
    case NDEF_BT_EIR_LE_SECURE_CONN_RANDOM_VALUE      : length = NDEF_BT_EIR_SECURE_CO_RANDOM_VALUE_SIZE              ; break;
    default:
      /* No length constraint on the following EIRs:   */
      /* NDEF_BT_EIR_SERVICE_CLASS_UUID_PARTIAL_16     */
      /* NDEF_BT_EIR_SERVICE_CLASS_UUID_COMPLETE_16    */
      /* NDEF_BT_EIR_SERVICE_CLASS_UUID_PARTIAL_32     */
      /* NDEF_BT_EIR_SERVICE_CLASS_UUID_COMPLETE_32    */
      /* NDEF_BT_EIR_SERVICE_CLASS_UUID_PARTIAL_128    */
      /* NDEF_BT_EIR_SERVICE_CLASS_UUID_COMPLETE_128   */
      /* NDEF_BT_EIR_SHORT_LOCAL_NAME                  */
      /* NDEF_BT_EIR_COMPLETE_LOCAL_NAME               */
      /* NDEF_BT_EIR_SERVICE_SOLICITATION_16           */
      /* NDEF_BT_EIR_SERVICE_SOLICITATION_128          */
      /* NDEF_BT_EIR_SERVICE_DATA                      */
      /* NDEF_BT_EIR_APPEARANCE                        */
      /* NDEF_BT_EIR_MANUFACTURER_DATA                 */
      length = 0;
      break;
  }

  return length;
}
#endif


/*****************************************************************************/
static NFC_OpResult ndefPayloadToBluetooth(const NDef_Const_Buffer *bufPayload, NDef_Type_ID typeId, NDef_Type *type)
{
  NDef_Type_Bluetooth *ndefData;

  if ((bufPayload == NULL) || (bufPayload->buffer == NULL) ||
      (type       == NULL)) {
    return NFC_InvalidParameter;
  }

  if (bufPayload->length < NDEF_BLUETOOTH_PAYLOAD_LENGTH_MIN) { /* Bluetooth Payload Min */
    return NFC_ProtocolError;
  }

  /* MISRA complains that this conditional expression is always false, so comment it out */
  /* if ( (typeId != NDEF_TYPE_ID_BLUETOOTH_BREDR)  &&
       (typeId != NDEF_TYPE_ID_BLUETOOTH_BLE)    &&
       (typeId != NDEF_TYPE_ID_BLUETOOTH_SECURE_LE) )
  {
      return NFC_InvalidParameter;
  } */

  type->id               = typeId;
  type->getPayloadLength = NFC_Bluetooth_PayloadGetLength;
  type->getPayloadItem   = NFC_Bluetooth_ToPayloadItem;
  type->typeToRecord     = NDef_BluetoothToRecord;
  ndefData               = &type->data.bluetooth;

  /* Reset every field */
  if (NFC_Bluetooth_Reset(ndefData) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  uint32_t offset = 0;

  /* Extract data from the payload */
  if ((typeId == NDEF_TYPE_ID_BLUETOOTH_BREDR) ||
      (typeId == NDEF_TYPE_ID_BLUETOOTH_SECURE_LE)) {
    uint16_t length = bufPayload->buffer[offset];
    NO_WARNING(length);
    offset += sizeof(uint16_t);

    /* Could check length and bufPayload->length match */
  }
  if (typeId == NDEF_TYPE_ID_BLUETOOTH_BREDR) {
    ndefData->bufDeviceAddress.buffer = &bufPayload->buffer[offset];
    ndefData->bufDeviceAddress.length = NDEF_BT_EIR_DEVICE_ADDRESS_SIZE;
    offset += NDEF_BT_EIR_DEVICE_ADDRESS_SIZE;
  }

  while (offset < bufPayload->length) {
    const uint8_t *eir = &bufPayload->buffer[offset];
    uint8_t eir_length = NFC_Bluetooth_EirLength(eir);

    if (eir_length == 0U) {
      break;  /* Leave when find an empty EIR */
    }

    offset += eir_length;

#ifdef NDEF_BLUETOOTH_CHECK_REFERENCE_LENGTH
    uint8_t  eir_data_length = NFC_Bluetooth_EirDataLength(eir);
    uint8_t  eir_type        = NFC_Bluetooth_EirType(eir);
    uint32_t refLength       = NFC_Bluetooth_EirRefLength(eir_type);
    /* Check length match */
    if ((refLength != 0U) &&
        (refLength != eir_data_length)) {
      return NFC_ProtocolError;
    }
#endif
    NFC_OpResult err = NFC_Bluetooth_SetEir(ndefData, eir);
    if (err != NFC_OK) {
      return err;
    }
  }

  /* The client is in charge to check that the mandatory fields are there */

  return NFC_OK;
}


/*****************************************************************************/
NFC_OpResult NDef_RecordToBluetooth(const NDef_Record *record, NDef_Type *type)
{
  const NDef_Type *ndefData;
  NDef_Type_ID typeId;

  if ((record == NULL) || (type == NULL)) {
    return NFC_InvalidParameter;
  }

  /* "application/vnd.bluetooth.ep.oob" */
  if (NDef_RecordTypeMatch(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothBrEdr)) {
    typeId = NDEF_TYPE_ID_BLUETOOTH_BREDR;
  }
  /* "application/vnd.bluetooth.le.oob" */
  else if (NDef_RecordTypeMatch(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothLe)) {
    typeId = NDEF_TYPE_ID_BLUETOOTH_LE;
  }
  /* "application/vnd.bluetooth.secure.ep.oob" */
  else if (NDef_RecordTypeMatch(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothSecureBrEdr)) {
    typeId = NDEF_TYPE_ID_BLUETOOTH_SECURE_BREDR;
  }
  /* "application/vnd.bluetooth.secure.le.oob" */
  else if (NDef_RecordTypeMatch(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothSecureLe)) {
    typeId = NDEF_TYPE_ID_BLUETOOTH_SECURE_LE;
  } else {
    return NFC_ProtocolError;
  }

  ndefData = NDef_RecordGetNdefType(record);
  if ((ndefData != NULL) && ((ndefData->id == NDEF_TYPE_ID_BLUETOOTH_BREDR)        ||
                             (ndefData->id == NDEF_TYPE_ID_BLUETOOTH_LE)           ||
                             (ndefData->id == NDEF_TYPE_ID_BLUETOOTH_SECURE_BREDR) ||
                             (ndefData->id == NDEF_TYPE_ID_BLUETOOTH_SECURE_LE))
     ) {
    (void)memcpy(type, ndefData, sizeof(NDef_Type));
    return NFC_OK;
  }

  return ndefPayloadToBluetooth(&record->bufPayload, typeId, type);
}


/*****************************************************************************/
NFC_OpResult NDef_BluetoothToRecord(const NDef_Type *type, NDef_Record *record)
{
  if ((type == NULL) || (record == NULL)) {
    return NFC_InvalidParameter;
  }

  (void)NDef_RecordReset(record);

  if (type->id == NDEF_TYPE_ID_BLUETOOTH_BREDR) {
    /* "application/vnd.bluetooth.ep.oob" */
    (void)NDef_RecordSetType(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothBrEdr);
  } else if (type->id == NDEF_TYPE_ID_BLUETOOTH_LE) {
    /* "application/vnd.bluetooth.le.oob" */
    (void)NDef_RecordSetType(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothLe);
  } else if (type->id == NDEF_TYPE_ID_BLUETOOTH_SECURE_BREDR) {
    /* "application/vnd.bluetooth.secure.ep.oob" */
    (void)NDef_RecordSetType(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothSecureBrEdr);
  } else if (type->id == NDEF_TYPE_ID_BLUETOOTH_SECURE_LE) {
    /* "application/vnd.bluetooth.secure.le.oob" */
    (void)NDef_RecordSetType(record, NDEF_TNF_MEDIA_TYPE, &bufMediaTypeBluetoothSecureLe);
  } else {
    return NFC_ProtocolError;
  }

  if (NDef_RecordSetNdefType(record, type) != NFC_OK) {
    return NFC_InvalidParameter;
  }

  return NFC_OK;
}

#endif
