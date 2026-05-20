
/**
  ******************************************************************************
  * @file           : ndef_type_bluetooth.h
  * @brief          : NDEF Bluetooth type header file
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

#ifndef NDEF_TYPE_BLUETOOTH_H
#define NDEF_TYPE_BLUETOOTH_H

#include "NDef_Record.h"
#include "NDef_Buffer.h"

#include "NFC/NFC_Def.h"

#define NDEF_BT_EIR_DEVICE_ADDRESS_TYPE_OFFSET    6U    /*!< Device Address Type (Public, Random) offset */


/*!< Number of EIRs that can be decoded simultaneously
     Allow to control the number of allocated memory space to store decoded EIRs
     Should be lower or equal to the number of known EIRs (currently 26)
*/
#define NDEF_BT_EIR_COUNT                         8U    /*!< Number of EIRs that can be decoded */

/*! Bluetooth LE address types */
typedef enum {
  NDEF_BLE_PUBLIC_ADDRESS_TYPE = 0x0U, /*!< Public Device Address */
  NDEF_BLE_RANDOM_ADDRESS_TYPE = 0x1U, /*!< Random Device Address */
  NDEF_BLE_UNDEF_ADDRESS_TYPE  = 0xFFU /*!< Device Address is undefined */
} NFC_Bluetooth_LEAddressType;


/*! Bluetooth LE roles */
typedef enum {
  NDEF_BLE_ROLE_PERIPH_ONLY       = 0x0U, /*!< Only Peripheral Role supported */
  NDEF_BLE_ROLE_CENTRAL_ONLY      = 0x1U, /*!< Only Central Role supported */
  NDEF_BLE_ROLE_PERIPH_PREFERRED  = 0x2U, /*!< Peripheral and Central Role supported, Peripheral Role preferred for connection establishment */
  NDEF_BLE_ROLE_CENTRAL_PREFERRED = 0x3U, /*!< Peripheral and Central Role supported, Central Role preferred for connection establishment */
  NDEF_BLE_ROLE_UNDEF             = 0xFFU /*!< LE Role is undefined */
} NFC_Bluetooth_LERole;


/*! Extended Inquiry Responses, as defined in the Bluetooth v4.0 Core Specification */
#define NDEF_BT_EIR_FLAGS                              0x01U   /*!< Bluetooth flags:\n
                                                                    b0: LE limited Discoverable Mode,
                                                                    b1: LE general Discoverable Mode,
                                                                    b2: BR/EDR not supported,
                                                                    b3: Simultaneous LE & BR/EDR Controller,
                                                                    b4: Simultaneous LE & BR/EDR Host */
#define NDEF_BT_EIR_SERVICE_CLASS_UUID_PARTIAL_16      0x02U   /*!< Bluetooth service UUID on 16-bits (partial list) */
#define NDEF_BT_EIR_SERVICE_CLASS_UUID_COMPLETE_16     0x03U   /*!< Bluetooth service UUID on 16-bits (complete list) */
#define NDEF_BT_EIR_SERVICE_CLASS_UUID_PARTIAL_32      0x04U   /*!< Bluetooth service UUID on 32-bits (partial list) */
#define NDEF_BT_EIR_SERVICE_CLASS_UUID_COMPLETE_32     0x05U   /*!< Bluetooth service UUID on 32-bits (complete list) */
#define NDEF_BT_EIR_SERVICE_CLASS_UUID_PARTIAL_128     0x06U   /*!< Bluetooth service UUID on 128-bits (partial list) */
#define NDEF_BT_EIR_SERVICE_CLASS_UUID_COMPLETE_128    0x07U   /*!< Bluetooth service UUID on 128-bits (complete list) */
#define NDEF_BT_EIR_SHORT_LOCAL_NAME                   0x08U   /*!< Shortened local name */
#define NDEF_BT_EIR_COMPLETE_LOCAL_NAME                0x09U   /*!< Complete local name */
#define NDEF_BT_EIR_TX_POWER_LEVEL                     0x0AU   /*!< TX Power Level (1 byte): 0xXX:-127 to +127dBm */
#define NDEF_BT_EIR_DEVICE_CLASS                       0x0DU   /*!< Class of device, Format defined in Assigned Numbers */
#define NDEF_BT_EIR_SIMPLE_PAIRING_HASH                0x0EU   /*!< Simple Pairing Hash C (16 octets), Format defined in [Vol. 2], Part H Section 7.2.2 */
#define NDEF_BT_EIR_SIMPLE_PAIRING_RANDOMIZER          0x0FU   /*!< Simple Pairing Randomizer R (16 octets), Format defined in[Vol. 2], Part H Section 7.2.2 */
#define NDEF_BT_EIR_SECURITY_MANAGER_TK_VALUE          0x10U   /*!< TK Value: Value as used in pairing over LE Physical channel. Format defined in [Vol. 3], Part H Section 2.3 */
#define NDEF_BT_EIR_SECURITY_MANAGER_FLAGS             0x11U   /*!< Flags (1 octet):\n
                                                                    b0: OOB Flags Field (0 = OOB data not present, 1 = OOB data present),
                                                                    b1: LE supported (Host) (i.e. bit 65 of LMP Extended Feature bits Page 1),
                                                                    b2: Simultaneous LE and BR/EDR to Same Device Capable (Host) (i.e. bit 66 of LMP Extended Feature bits Page 1)
                                                                    b3: Address type (0 = Public Address, 1 = Random Address) */
#define NDEF_BT_EIR_SLAVE_CONNECTION_INTERVAL_RANGE    0x12U   /*!< Slave Connection Interval Range: The first 2 octets define the minimum value for the connection interval, The second 2 octets define the maximum value for the connection interval */
#define NDEF_BT_EIR_SERVICE_SOLICITATION_16            0x14U   /*!< Service UUIDs: List of 16 bit Service UUIDs */
#define NDEF_BT_EIR_SERVICE_SOLICITATION_128           0x15U   /*!< Service UUIDs: List of 128 bit Service UUIDs */
#define NDEF_BT_EIR_SERVICE_DATA                       0x16U   /*!< Service Data (2 or more octets): The first 2 octets contain the 16 bit Service UUID followed by additional service data */
#define NDEF_BT_EIR_APPEARANCE                         0x19U   /*!< UUID for `Appearance`: The Appearance characteristic value shall be the enumerated value as defined by Bluetooth Assigned Numbers document */
#define NDEF_BT_EIR_LE_DEVICE_ADDRESS                  0x1BU   /*!< 6 LSB bytes: Device address, 7th byte: Address type (Public/Random) */
#define NDEF_BT_EIR_LE_ROLE                            0x1CU   /*!< Device Role: Periph only, Central only, Periph preferred, Central preferred */
#define NDEF_BT_EIR_LE_SECURE_CONN_CONFIRMATION_VALUE  0x22U   /*!< Secure Connection Confirmation value */
#define NDEF_BT_EIR_LE_SECURE_CONN_RANDOM_VALUE        0x23U   /*!< Secure Connection Random value */
#define NDEF_BT_EIR_MANUFACTURER_DATA                  0xFFU   /*!< Manufacturer Specific Data (2 or more octets): The first 2 octets contain the Company Identifier Code followed by additional manufacturer specific data */


/*! Bluetooth Out-Of-Band data structure
  * Some fields are shared by both Br/Edr & LE Bluetooth, some are specific.
  */
typedef struct {
  /* Mandatory fields:
      bufDeviceAddress
     Optional common fields:
      NDEF_BT_EIR_FLAGS
      NDEF_BT_EIR_SERVICE_CLASS_UUID_PARTIAL_16
      NDEF_BT_EIR_SERVICE_CLASS_UUID_COMPLETE_16
      NDEF_BT_EIR_SERVICE_CLASS_UUID_PARTIAL_32
      NDEF_BT_EIR_SERVICE_CLASS_UUID_COMPLETE_32
      NDEF_BT_EIR_SERVICE_CLASS_UUID_PARTIAL_128
      NDEF_BT_EIR_SERVICE_CLASS_UUID_COMPLETE_128
      NDEF_BT_EIR_SHORT_LOCAL_NAME
      NDEF_BT_EIR_COMPLETE_LOCAL_NAME
      NDEF_BT_EIR_TX_POWER_LEVEL
      NDEF_BT_EIR_DEVICE_CLASS
      NDEF_BT_EIR_SECURITY_MANAGER_FLAGS
      NDEF_BT_EIR_SLAVE_CONNECTION_INTERVAL_RANGE
      NDEF_BT_EIR_SERVICE_SOLICITATION_16
      NDEF_BT_EIR_SERVICE_SOLICITATION_128
      NDEF_BT_EIR_SERVICE_DATA
     For Br/Edr only:
      NDEF_BT_EIR_SIMPLE_PAIRING_HASH
      NDEF_BT_EIR_SIMPLE_PAIRING_RANDOMIZER
      BLE mandatory fields:
      NDEF_BT_EIR_LE_ROLE
     BLE optional fields:
      NDEF_BT_EIR_LE_SECURE_CONN_CONFIRMATION_VALUE
      NDEF_BT_EIR_LE_SECURE_CONN_RANDOM_VALUE
      NDEF_BT_EIR_SECURITY_MANAGER_TK_VALUE
      NDEF_BT_EIR_APPEARANCE
      NDEF_BT_EIR_MANUFACTURER_DATA */

  NDef_Const_Buffer bufDeviceAddress;         /*!< Device address, for BR/EDR only */

  const uint8_t *eir[NDEF_BT_EIR_COUNT];    /*!< Array containing pointer to each EIR */

} NDef_Type_Bluetooth;

extern const NDef_Const_Buffer_8 bufMediaTypeBluetoothBrEdr;       /*! Bluetooth BR/EDR Out-Of-Band Record Type buffer */
extern const NDef_Const_Buffer_8 bufMediaTypeBluetoothLe;          /*! Bluetooth Low Energy Record Type buffer         */
extern const NDef_Const_Buffer_8 bufMediaTypeBluetoothSecureBrEdr; /*! Bluetooth Secure BR/EDR Record Type buffer      */
extern const NDef_Const_Buffer_8 bufMediaTypeBluetoothSecureLe;    /*! Bluetooth Secure Low Energy Record Type buffer  */

uint8_t NFC_Bluetooth_EirLength(const uint8_t *eir);
uint8_t NFC_Bluetooth_EirDataLength(const uint8_t *eir);
uint8_t NFC_Bluetooth_EirType(const uint8_t *eir);
const uint8_t *NFC_Bluetooth_EirData(const uint8_t *eir);
NFC_OpResult NFC_Bluetooth_EirDataToBuffer(const uint8_t *eir, NDef_Const_Buffer *bufEir);
NFC_OpResult NFC_Bluetooth_SetEir(NDef_Type_Bluetooth *bluetooth, const uint8_t *eir);
const uint8_t *NFC_Bluetooth_GetEir(const NDef_Type_Bluetooth *bluetooth, uint8_t eirType);
NFC_OpResult NFC_Bluetooth_GetEirData(const NDef_Type_Bluetooth *bluetooth, uint8_t eirType, NDef_Const_Buffer *bufData);
NFC_OpResult NFC_Bluetooth_GetEirDataReversed(const NDef_Type_Bluetooth *bluetooth, uint8_t eirType, NDef_Buffer *bufDataReversed);
NFC_OpResult NFC_Bluetooth_Reset(NDef_Type_Bluetooth *bluetooth);
NFC_OpResult NFC_Bluetooth_BrEdrInit(NDef_Type *type, const NDef_Type_Bluetooth *bluetooth);
NFC_OpResult NFC_Bluetooth_LeInit(NDef_Type *type, const NDef_Type_Bluetooth *bluetooth);
NFC_OpResult NFC_Bluetooth_SecureBrEdrInit(NDef_Type *type, const NDef_Type_Bluetooth *bluetooth);
NFC_OpResult NFC_Bluetooth_SecureLeInit(NDef_Type *type, const NDef_Type_Bluetooth *bluetooth);
NFC_OpResult NDef_GetBluetooth(const NDef_Type *type, NDef_Type_Bluetooth *bluetooth);

NFC_OpResult NDef_RecordToBluetooth(const NDef_Record *record, NDef_Type *type);
NFC_OpResult NDef_BluetoothToRecord(const NDef_Type *type, NDef_Record *record);

#endif /* NDEF_TYPE_BLUETOOTH_H */

/**
  * @}
  *
  */
