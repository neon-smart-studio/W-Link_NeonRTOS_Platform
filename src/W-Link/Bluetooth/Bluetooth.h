

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>

#include "Bluetooth_Def.h"

#ifdef __cplusplus
extern "C" {
#endif

Bluetooth_OpResult Bluetooth_Init(const char *name, uint8_t addr[BDADDR_SIZE]);
Bluetooth_OpResult Bluetooth_DeInit(void);
Bluetooth_OpResult Bluetooth_Set_TX_Power(Bluetooth_PA_Level pa_level);
Bluetooth_OpResult Bluetooth_Set_Scan_Response_Data(uint8_t* pData);
Bluetooth_OpResult Bluetooth_Set_Discoverable();
Bluetooth_OpResult Bluetooth_Add_Service(
    const char *uuid,
    Bluetooth_Service_Type service_type,
    uint8_t max_attr_records,
    uint16_t *pServiceHandle
);
Bluetooth_OpResult Bluetooth_Service_Add_Char(
    uint16_t serviceHandle,
    const char *uuid,
    uint8_t charValueLen,
    Bluetooth_Char_Property property,
    Bluetooth_Char_Permission permission,
    uint16_t *pCharHandle
);
Bluetooth_OpResult Bluetooth_Service_Add_Char_Desc(
    uint16_t serviceHandle,
    uint16_t charHandle,
    const char *uuid,
    uint8_t descValueMaxLen,
    uint8_t descValueLen,
    const void *pDescValue,
    Bluetooth_Char_Permission permission,
    Bluetooth_Attr_Access access,
    bool requireAuth,
    uint8_t encryptionKeySize,
    bool isVariable,
    uint16_t *pDescHandle
);
Bluetooth_OpResult Bluetooth_Service_Update_Char_Value(
    uint16_t serviceHandle,
    uint16_t charHandle,
    uint8_t offset,
    uint8_t valueLen,
    const uint8_t *pValue
);

#ifdef __cplusplus
}
#endif

#endif // BLUETOOTH_H