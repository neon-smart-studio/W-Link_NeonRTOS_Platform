/*
 *******************************************************************************
 * Copyright (c) 2017, STMicroelectronics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 */
/*
 * Based on STMicroelectronics SPBTLE driver
 * Modified by Neon Smart Studio for W-Link
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "SPI/SPI_Master.h"

#include "HCI/hci.h"
#include "HCI/hci_le.h"
#include "HCI/Utils/gp_timer.h"

#include "HCI/Controller/bluenrg_gap.h"
#include "HCI/Controller/bluenrg_hal_aci.h"
#include "HCI/Controller/bluenrg_gap_aci.h"
#include "HCI/Controller/bluenrg_gatt_aci.h"
#include "HCI/Controller/bluenrg_updater_aci.h"

#include "sm.h"

#include "Bluetooth/Bluetooth_Def.h"

#include "Bluetooth_SPBTLE.h"

#define HEADER_SIZE      5
#define MAX_BUFFER_SIZE  255

#define SPBTLE_HCI_TASK_STACK_SIZE  1024
#define SPBTLE_HCI_TASK_PRIORITY    2
#define SPBTLE_HCI_TASK_DELAY_MS    1
#define SPBTLE_CMD_QUEUE_SIZE  8
#define SPBTLE_CMD_TIMEOUT_MS  1000

typedef struct
{
    Bluetooth_UUID_Type type;

    union
    {
        uint16_t uuid16;
        uint8_t uuid128[16];
    };
} Bluetooth_UUID;

typedef enum {
    BLE_REQ_SET_DISCOVERABLE,
    BLE_REQ_SET_TX_POWER,
    BLE_REQ_SET_SCAN_RESP,
    BLE_REQ_ADD_SERVICE,
    BLE_REQ_ADD_CHAR,
    BLE_REQ_ADD_CHAR_DESC,
    BLE_REQ_UPDATE_CHAR_VALUE,
} BLE_RequestType;

typedef struct {
    Bluetooth_PA_Level pa_level;
} BLE_SetTxPowerParam;

typedef struct {
    uint8_t *data;
    uint8_t len;
} BLE_SetScanRespParam;

typedef struct {
    Bluetooth_UUID uuid;
    Bluetooth_Service_Type service_type;
    uint8_t max_attr_records;
    uint16_t *service_handle;
} BLE_AddServiceParam;

typedef struct {
    uint16_t service_handle;
    Bluetooth_UUID uuid;
    uint8_t char_value_len;
    Bluetooth_Char_Property property;
    Bluetooth_Char_Permission permission;
    uint16_t *char_handle;
} BLE_AddCharParam;

typedef struct {
    uint16_t service_handle;
    uint16_t char_handle;
    Bluetooth_UUID uuid;
    uint8_t desc_value_max_len;
    uint8_t desc_value_len;
    const void *desc_value;
    Bluetooth_Char_Permission permission;
    Bluetooth_Attr_Access access;
    bool require_auth;
    uint8_t encryption_key_size;
    bool is_variable;
    uint16_t *desc_handle;
} BLE_AddCharDescParam;

typedef struct {
    uint16_t service_handle;
    uint16_t char_handle;
    uint8_t offset;
    uint8_t value_len;
    const uint8_t *value;
} BLE_UpdateCharValueParam;

typedef struct {
    BLE_RequestType type;
    void *param;
    Bluetooth_OpResult result;
    NeonRTOS_SyncObj_t done;
} BLE_Request;

static NeonRTOS_MsgQ_t g_ble_cmd_queue = NULL;

static NeonRTOS_TaskHandle g_hci_task = NULL;

static char SPBTLE_Addr[BDADDR_SIZE];
static char SPBTLE_Broadcast_Device_Name[1+BNAME_LEN+1];

static void SPBTLE_HCI_Task(void *argument);

void SPI_EXTI_Event(void* p1, uint32_t p2)
{
    HCI_Isr();
}

void SPI_EXTI_Callback(hwGPIO_Int_Pin pin, hwGPIO_Interrupt_Action action)
{
    (void)pin;

    if(action == hwGPIO_Interrupt_Action_Rising_Edge)
    {
        NeonRTOS_PendingFunctionCall(SPI_EXTI_Event, NULL, 0);
    }
}

void BlueNRG_RST(void)
{
    GPIO_Pin_Write(SPBTLE_RESET_PIN, false);
    NeonRTOS_Sleep(20);

    GPIO_Pin_Write(SPBTLE_RESET_PIN, true);
    NeonRTOS_Sleep(100);
}

bool BlueNRG_DataPresent(void)
{
    bool level = false;

    GPIO_Interrupt_Pin_Read(SPBTLE_IRQ_PIN, &level);

    return level ? 1 : 0;
}

void BlueNRG_HW_Bootloader(void)
{
    // Reset BlueNRG SPI interface
    BlueNRG_RST();

    // Send an ACI command to reboot BlueNRG in bootloader mode
    // The safest way to get in bootloader mode is keeping high
    // the interrupt pin during reset, but this would require many
    // changes to the current mbed driver
    aci_updater_start();
}

static Bluetooth_OpResult SPBTLE_Map_SPI_Result(hwSPI_OpResult result)
{
    switch(result)
    {
        case hwSPI_OK:
            return Bluetooth_OK;

        case hwSPI_NotInit:
            return Bluetooth_NotInit;

        case hwSPI_InvalidParameter:
            return Bluetooth_InvalidParameter;

        case hwSPI_HwError:
            return Bluetooth_HwError;

        case hwSPI_MemoryError:
            return Bluetooth_MemoryError;

        case hwSPI_MutexTimeout:
        case hwSPI_SlaveTimeout:
            return Bluetooth_Busy;

        case hwSPI_Unsupport:
            return Bluetooth_Unsupport;

        default:
            return Bluetooth_HwError;
    }
}

static Bluetooth_OpResult SPBTLE_Map_GPIO_Result(hwGPIO_OpResult result)
{
    switch(result)
    {
        case hwGPIO_OK:
            return Bluetooth_OK;

        case hwGPIO_InvalidParameter:
            return Bluetooth_InvalidParameter;

        case hwGPIO_PinConflict:
            return Bluetooth_Busy;

        case hwGPIO_HW_Error:
            return Bluetooth_HwError;

        case hwGPIO_Unsupport:
            return Bluetooth_Unsupport;

        default:
            return Bluetooth_HwError;
    }
}

void Enable_SPI_IRQ(void)
{
    GPIO_Interrupt_Enable(SPBTLE_IRQ_PIN);
}

void Disable_SPI_IRQ(void)
{
    GPIO_Interrupt_Disable(SPBTLE_IRQ_PIN);
}

void Clear_SPI_EXTI_Flag(void)
{

}

static void SPBTLE_ProcessPendingEvents(uint32_t timeout_ms)
{
    uint32_t millis = NeonRTOS_Millis();

    while((NeonRTOS_Millis() - millis) < timeout_ms)
    {
        if(BlueNRG_DataPresent())
        {
            HCI_Process();
        }
        else
        {
            break;
        }

        NeonRTOS_Sleep(1);
    }
}

int32_t BlueNRG_SPI_Read_All(uint8_t *buffer, uint8_t buff_size)
{
    uint16_t byte_count = 0;
    uint8_t len = 0;
    uint8_t dummy = 0xFF;
    uint8_t read_char = 0;

    uint8_t header_master[HEADER_SIZE] = {
        0x0B, 0x00, 0x00, 0x00, 0x00
    };

    uint8_t header_slave[HEADER_SIZE] = {0};

    if(buffer == NULL || buff_size == 0)
        return 0;

    GPIO_Pin_Write(SPBTLE_CS_PIN, false);

    for(uint32_t i = 0; i < HEADER_SIZE; i++)
    {
        SPI_Master_TransferByte(
            SPBTLE_SPI_INDEX,
            header_master[i],
            &header_slave[i]
        );
    }

    if(header_slave[0] == 0x02)
    {
        byte_count = ((uint16_t)header_slave[4] << 8) | header_slave[3];

        if(byte_count > buff_size)
            byte_count = buff_size;

        for(len = 0; len < byte_count; len++)
        {
            SPI_Master_TransferByte(
                SPBTLE_SPI_INDEX,
                dummy,
                &read_char
            );

            buffer[len] = read_char;
        }
    }

    GPIO_Pin_Write(SPBTLE_CS_PIN, true);

    return len;
}

static int32_t BlueNRG_SPI_Write(
    uint8_t *data1,
    uint8_t *data2,
    uint8_t Nb_bytes1,
    uint8_t Nb_bytes2
)
{
    int32_t result = 0;

    uint8_t header_master[HEADER_SIZE] = {
        0x0A, 0x00, 0x00, 0x00, 0x00
    };

    uint8_t header_slave[HEADER_SIZE] = {
        0xAA, 0x00, 0x00, 0x00, 0x00
    };

    uint8_t dummy_rx = 0;

    Disable_SPI_IRQ();

    GPIO_Pin_Write(SPBTLE_CS_PIN, false);

    for(uint32_t i = 0; i < HEADER_SIZE; i++)
    {
        SPI_Master_TransferByte(
            SPBTLE_SPI_INDEX,
            header_master[i],
            &header_slave[i]
        );
    }

    if(header_slave[0] == 0x02)
    {
        if(header_slave[1] >= (Nb_bytes1 + Nb_bytes2))
        {
            for(uint32_t i = 0; i < Nb_bytes1; i++)
            {
                SPI_Master_TransferByte(
                    SPBTLE_SPI_INDEX,
                    data1[i],
                    &dummy_rx
                );
            }

            for(uint32_t i = 0; i < Nb_bytes2; i++)
            {
                SPI_Master_TransferByte(
                    SPBTLE_SPI_INDEX,
                    data2[i],
                    &dummy_rx
                );
            }
        }
        else
        {
            result = -2;
        }
    }
    else
    {
        result = -1;
    }

    GPIO_Pin_Write(SPBTLE_CS_PIN, true);

    Enable_SPI_IRQ();

    return result;
}

void Hal_Write_Serial(
    const void *data1,
    const void *data2,
    int32_t n_bytes1,
    int32_t n_bytes2
)
{
    struct timer t;

    Timer_Set(&t, CLOCK_SECOND / 5);

    while(1)
    {
        if(BlueNRG_SPI_Write(
            (uint8_t *)data1,
            (uint8_t *)data2,
            (uint8_t)n_bytes1,
            (uint8_t)n_bytes2
        ) == 0)
        {
            break;
        }

        if(Timer_Expired(&t))
        {
            break;
        }
    }
}

static uint8_t SPBTLE_Map_Char_Property(
    Bluetooth_Char_Property property
)
{
    uint8_t ret = 0;

    if(property & Bluetooth_Char_Property_Broadcast)
        ret |= CHAR_PROP_BROADCAST;

    if(property & Bluetooth_Char_Property_Read)
        ret |= CHAR_PROP_READ;

    if(property & Bluetooth_Char_Property_WriteWithoutResp)
        ret |= CHAR_PROP_WRITE_WITHOUT_RESP;

    if(property & Bluetooth_Char_Property_Write)
        ret |= CHAR_PROP_WRITE;

    if(property & Bluetooth_Char_Property_Notify)
        ret |= CHAR_PROP_NOTIFY;

    if(property & Bluetooth_Char_Property_Indicate)
        ret |= CHAR_PROP_INDICATE;

    if(property & Bluetooth_Char_Property_SignedWrite)
        ret |= CHAR_PROP_SIGNED_WRITE;

    if(property & Bluetooth_Char_Property_Extended)
        ret |= CHAR_PROP_EXT;

    return ret;
}

static uint8_t SPBTLE_Map_Char_Permission(
    Bluetooth_Char_Permission permission
)
{
    uint8_t ret = 0;

    if(permission & Bluetooth_Char_Permission_Authen_Read)
        ret |= ATTR_PERMISSION_AUTHEN_READ;

    if(permission & Bluetooth_Char_Permission_Author_Read)
        ret |= ATTR_PERMISSION_AUTHOR_READ;

    if(permission & Bluetooth_Char_Permission_Encrypt_Read)
        ret |= ATTR_PERMISSION_ENCRY_READ;

    if(permission & Bluetooth_Char_Permission_Authen_Write)
        ret |= ATTR_PERMISSION_AUTHEN_WRITE;

    if(permission & Bluetooth_Char_Permission_Author_Write)
        ret |= ATTR_PERMISSION_AUTHOR_WRITE;

    if(permission & Bluetooth_Char_Permission_Encrypt_Write)
        ret |= ATTR_PERMISSION_ENCRY_WRITE;

    return ret;
}

static uint8_t SPBTLE_Map_Attr_Access(Bluetooth_Attr_Access access)
{
    switch(access)
    {
        case Bluetooth_Attr_Access_ReadOnly:
            return ATTR_ACCESS_READ_ONLY;

        case Bluetooth_Attr_Access_WriteOnly:
            return ATTR_ACCESS_WRITE_REQ_ONLY;

        case Bluetooth_Attr_Access_ReadWrite:
            return ATTR_ACCESS_READ_WRITE;

        default:
            return ATTR_ACCESS_READ_ONLY;
    }
}

static bool SPBTLE_Get_Aci_UUID(
    const Bluetooth_UUID *uuid,
    uint8_t *aci_uuid_type,
    uint8_t **aci_uuid
)
{
    if(uuid == NULL || aci_uuid_type == NULL || aci_uuid == NULL)
    {
        return false;
    }

    switch(uuid->type)
    {
        case Bluetooth_UUID_Type_16:
            *aci_uuid_type = UUID_TYPE_16;
            *aci_uuid = (uint8_t *)&uuid->uuid16;
            return true;

        case Bluetooth_UUID_Type_128:
            *aci_uuid_type = UUID_TYPE_128;
            *aci_uuid = (uint8_t *)uuid->uuid128;
            return true;

        default:
            return false;
    }
}

static void SPBTLE_ExecuteRequest(BLE_Request *req)
{
    int ret = BLE_STATUS_SUCCESS;

    switch(req->type)
    {
        case BLE_REQ_SET_TX_POWER:
        {
            BLE_SetTxPowerParam *p = req->param;

            ret = aci_hal_set_tx_power_level(1, p->pa_level);

            req->result = (ret == BLE_STATUS_SUCCESS)
                        ? Bluetooth_OK
                        : Bluetooth_HCI_Error;
        }
        break;

        case BLE_REQ_SET_SCAN_RESP:
        {
            BLE_SetScanRespParam *p = req->param;

            ret = hci_le_set_scan_resp_data(p->len, p->data);

            req->result = (ret == BLE_STATUS_SUCCESS)
                        ? Bluetooth_OK
                        : Bluetooth_HCI_Error;
        }
        break;

        case BLE_REQ_SET_DISCOVERABLE:
        {
            uint8_t name_len;

            name_len = strlen((char *)&SPBTLE_Broadcast_Device_Name[1]);

            ret = aci_gap_set_discoverable(
                ADV_IND,
                0,
                0,
                PUBLIC_ADDR,
                NO_WHITE_LIST_USE,
                name_len + 1,
                (uint8_t *)SPBTLE_Broadcast_Device_Name,
                0,
                NULL,
                0,
                0
            );

            req->result = (ret == BLE_STATUS_SUCCESS)
                        ? Bluetooth_OK
                        : Bluetooth_HCI_Error;
        }
        break;

        case BLE_REQ_ADD_SERVICE:
        {
            BLE_AddServiceParam *p = req->param;
            uint8_t aci_uuid_type;
            uint8_t aci_service_type;
            uint8_t *aci_uuid;

            if(p == NULL || p->service_handle == NULL)
            {
                req->result = Bluetooth_InvalidParameter;
                return;
            }

            if(!SPBTLE_Get_Aci_UUID(&p->uuid, &aci_uuid_type, &aci_uuid))
            {
                req->result = Bluetooth_InvalidParameter;
                return;
            }

            switch(p->service_type)
            {
                case Bluetooth_Service_Type_Primary:
                    aci_service_type = PRIMARY_SERVICE;
                    break;

                case Bluetooth_Service_Type_Secondary:
                    aci_service_type = SECONDARY_SERVICE;
                    break;

                default:
                    req->result = Bluetooth_InvalidParameter;
                    return;
            }

            ret = aci_gatt_add_serv(
                aci_uuid_type,
                aci_uuid,
                aci_service_type,
                p->max_attr_records,
                p->service_handle
            );

            req->result = (ret == BLE_STATUS_SUCCESS)
                        ? Bluetooth_OK
                        : Bluetooth_HCI_Error;
        }
        break;

        case BLE_REQ_ADD_CHAR:
        {
            BLE_AddCharParam *p = req->param;
            uint8_t aci_uuid_type;
            uint8_t *aci_uuid;
            uint8_t aci_property;
            uint8_t aci_permission;

            if(p == NULL || p->char_handle == NULL)
            {
                req->result = Bluetooth_InvalidParameter;
                return;
            }

            if(!SPBTLE_Get_Aci_UUID(&p->uuid, &aci_uuid_type, &aci_uuid))
            {
                req->result = Bluetooth_InvalidParameter;
                return;
            }

            aci_property = SPBTLE_Map_Char_Property(p->property);
            aci_permission = SPBTLE_Map_Char_Permission(p->permission);

            ret = aci_gatt_add_char(
                p->service_handle,
                aci_uuid_type,
                aci_uuid,
                p->char_value_len,
                aci_property,
                aci_permission,
                0,
                16,
                0,
                p->char_handle
            );

            req->result = (ret == BLE_STATUS_SUCCESS)
                        ? Bluetooth_OK
                        : Bluetooth_HCI_Error;
        }
        break;

        case BLE_REQ_ADD_CHAR_DESC:
        {
            BLE_AddCharDescParam *p = req->param;
            uint8_t aci_uuid_type;
            uint8_t *aci_uuid;
            uint8_t aci_permission;
            uint8_t aci_access;

            if(p == NULL || p->desc_handle == NULL)
            {
                req->result = Bluetooth_InvalidParameter;
                return;
            }

            if(p->desc_value_len > p->desc_value_max_len)
            {
                req->result = Bluetooth_InvalidParameter;
                return;
            }

            if(!SPBTLE_Get_Aci_UUID(&p->uuid, &aci_uuid_type, &aci_uuid))
            {
                req->result = Bluetooth_InvalidParameter;
                return;
            }

            aci_permission = SPBTLE_Map_Char_Permission(p->permission);
            aci_access = SPBTLE_Map_Attr_Access(p->access);

            ret = aci_gatt_add_char_desc(
                p->service_handle,
                p->char_handle,
                aci_uuid_type,
                aci_uuid,
                p->desc_value_max_len,
                p->desc_value_len,
                (void *)p->desc_value,
                aci_permission,
                aci_access,
                p->require_auth ? 1 : 0,
                p->encryption_key_size,
                p->is_variable ? true : false,
                p->desc_handle
            );

            req->result = (ret == BLE_STATUS_SUCCESS)
                        ? Bluetooth_OK
                        : Bluetooth_HCI_Error;
        }
        break;

        case BLE_REQ_UPDATE_CHAR_VALUE:
        {
            BLE_UpdateCharValueParam *p = req->param;

            if(p == NULL)
            {
                req->result = Bluetooth_InvalidParameter;
                return;
            }

            if(p->value == NULL && p->value_len > 0)
            {
                req->result = Bluetooth_InvalidParameter;
                return;
            }

            ret = aci_gatt_update_char_value(
                p->service_handle,
                p->char_handle,
                p->offset,
                p->value_len,
                (uint8_t *)p->value
            );

            req->result = (ret == BLE_STATUS_SUCCESS)
                        ? Bluetooth_OK
                        : Bluetooth_HCI_Error;
        }
        break;

        default:
            req->result = Bluetooth_Unsupport;
            break;
    }

    SPBTLE_ProcessPendingEvents(20);
}

static void SPBTLE_HCI_Task(void *argument)
{
    (void)argument;

    uint8_t bdaddr[BDADDR_SIZE];
    uint16_t service_handle = 0;
    uint16_t dev_name_char_handle = 0;
    uint16_t appearance_char_handle = 0;

    int ret = 0;

    HCI_Init();

    BlueNRG_RST();

    NeonRTOS_Sleep(100);

    SPBTLE_ProcessPendingEvents(50);

    memcpy(bdaddr, SPBTLE_Addr, BDADDR_SIZE);

    /*
     * 這個必須是 reset 後第一個 ACI command。
     */
    ret = aci_hal_write_config_data(
        CONFIG_DATA_PUBADDR_OFFSET,
        CONFIG_DATA_PUBADDR_LEN,
        bdaddr
    );
    if(ret != BLE_STATUS_SUCCESS)
    {
        while(1){NeonRTOS_Sleep(1000);}
    }
    
    SPBTLE_ProcessPendingEvents(20);

    ret = aci_gatt_init();
    if(ret != BLE_STATUS_SUCCESS)
    {
        while(1){NeonRTOS_Sleep(1000);}
    }

    SPBTLE_ProcessPendingEvents(20);

    /*
     * B-L475E-IOT01A 通常是 SPBTLE-RF / BlueNRG-MS，走 IDB05A1。
     * 你如果用 IDB04A1，很多 event structure / GAP init 會不對。
     */ 
#if BNRG_EXPANSION_BOARD==IDB04A1
    ret = aci_gap_init_IDB04A1(
        GAP_PERIPHERAL_ROLE_IDB04A1,
        &service_handle,
        &dev_name_char_handle,
        &appearance_char_handle);
#endif
#if BNRG_EXPANSION_BOARD==IDB05A1
    ret = aci_gap_init_IDB05A1(
        GAP_PERIPHERAL_ROLE_IDB05A1,
        0,
        BNAME_LEN,
        &service_handle,
        &dev_name_char_handle,
        &appearance_char_handle
    );
#endif
    if(ret != BLE_STATUS_SUCCESS)
    {
        while(1){NeonRTOS_Sleep(1000);}
    }

    SPBTLE_ProcessPendingEvents(20);

    ret = aci_gatt_update_char_value(
        service_handle,
        dev_name_char_handle,
        0,
        strlen((char *)&SPBTLE_Broadcast_Device_Name[1]),
        (uint8_t *)&SPBTLE_Broadcast_Device_Name[1]
    );
    if(ret != BLE_STATUS_SUCCESS)
    {
        while(1){NeonRTOS_Sleep(1000);}
    }

    SPBTLE_ProcessPendingEvents(20);

    ret = aci_gap_set_auth_requirement(
        MITM_PROTECTION_NOT_REQUIRED,
        OOB_AUTH_DATA_ABSENT,
        NULL,
        7,
        16,
        DONOT_USE_FIXED_PIN_FOR_PAIRING,
        0,
        NO_BONDING
    );
    if(ret != BLE_STATUS_SUCCESS)
    {
        while(1){NeonRTOS_Sleep(1000);}
    }

    SPBTLE_ProcessPendingEvents(20);

    while(1)
    {
        BLE_Request *req = NULL;

        if(NeonRTOS_MsgQRead(&g_ble_cmd_queue, &req, 1) == NeonRTOS_OK)
        {
            if(req != NULL)
            {
                SPBTLE_ExecuteRequest(req);
                NeonRTOS_SyncObjSignal(&req->done);
            }
        }

        if(BlueNRG_DataPresent())
        {
            HCI_Process();
        }

        NeonRTOS_Sleep(SPBTLE_HCI_TASK_DELAY_MS);
    }
}

void HCI_Event_CB(void *pckt)
{
    //
}

Bluetooth_OpResult Bluetooth_Init(const char *name, uint8_t addr[BDADDR_SIZE])
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    if(addr == NULL)
    {
        return Bluetooth_InvalidParameter;
    }

    memcpy(SPBTLE_Addr, addr, BDADDR_SIZE);

    memset(SPBTLE_Broadcast_Device_Name, 0, sizeof(SPBTLE_Broadcast_Device_Name));
    SPBTLE_Broadcast_Device_Name[0] = AD_TYPE_COMPLETE_LOCAL_NAME;

    if(name == NULL)
    {
        strncpy(&SPBTLE_Broadcast_Device_Name[1], BNAME_DEFAULT, strlen(BNAME_DEFAULT));
    }
    else
    {
        uint8_t name_len = strlen(name);

        if(name_len > BNAME_LEN)
        {
            name_len = BNAME_LEN;
        }

        strncpy(&SPBTLE_Broadcast_Device_Name[1], name, name_len);
    }

    gpio_op_status = GPIO_Pin_Init(SPBTLE_CS_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_None);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }
    gpio_op_status = GPIO_Pin_Write(SPBTLE_CS_PIN, 1);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_Init(SPBTLE_RESET_PIN, hwGPIO_Direction_Output, hwGPIO_Pull_Mode_None);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }
    gpio_op_status = GPIO_Pin_Write(SPBTLE_RESET_PIN, 0);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }

    spi_op_status = SPI_Master_Init(SPBTLE_SPI_INDEX, SPBTLE_SPI_CLOCK_HZ, SPBTLE_SPI_MODE, false);
    if(spi_op_status < hwSPI_OK)
    {
        return SPBTLE_Map_SPI_Result(spi_op_status);
    }

    gpio_op_status = GPIO_Interrupt_Init(SPBTLE_IRQ_PIN, hwGPIO_Interrupt_Mode_Rising_Edge);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }

    gpio_op_status = GPIO_Register_Interrupt_Handler(SPBTLE_IRQ_PIN, SPI_EXTI_Callback);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_Enable(SPBTLE_IRQ_PIN);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }
    
    NeonRTOS_MsgQCreate(
        &g_ble_cmd_queue,
        "BLE CMD",
        SPBTLE_CMD_QUEUE_SIZE,
        sizeof(BLE_Request*)
    );

    if(g_ble_cmd_queue == NULL)
    {
        return Bluetooth_MemoryError;
    }

    NeonRTOS_TaskCreate(
        SPBTLE_HCI_Task,
        "SPBTLE_HCI",
        SPBTLE_HCI_TASK_STACK_SIZE,
        NULL,
        SPBTLE_HCI_TASK_PRIORITY,
        &g_hci_task
    );

    if(g_hci_task == NULL)
    {
        return Bluetooth_MemoryError;
    }

    return Bluetooth_OK;
}

Bluetooth_OpResult Bluetooth_DeInit(void)
{
    hwGPIO_OpResult gpio_op_status;
    hwSPI_OpResult spi_op_status;

    if(g_hci_task != NULL)
    {
        NeonRTOS_TaskDelete(g_hci_task);
        g_hci_task = NULL;
    }

    if(g_ble_cmd_queue != NULL)
    {
        NeonRTOS_MsgQDelete(&g_ble_cmd_queue);
        g_ble_cmd_queue = NULL;
    }

    spi_op_status = SPI_Master_DeInit(SPBTLE_SPI_INDEX);
    if(spi_op_status  < hwSPI_OK)
    {
        return SPBTLE_Map_SPI_Result(spi_op_status );
    }

    gpio_op_status = GPIO_Pin_DeInit(SPBTLE_RESET_PIN);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }

    gpio_op_status = GPIO_Pin_DeInit(SPBTLE_CS_PIN);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_Disable(SPBTLE_IRQ_PIN);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }

    gpio_op_status = GPIO_Unregister_Interrupt_Handler(SPBTLE_IRQ_PIN);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }

    gpio_op_status = GPIO_Interrupt_DeInit(SPBTLE_IRQ_PIN);
    if(gpio_op_status < hwGPIO_OK)
    {
        return SPBTLE_Map_GPIO_Result(gpio_op_status);
    }
    
    return Bluetooth_OK;
}

static Bluetooth_OpResult SPBTLE_SendRequest(
    BLE_RequestType type,
    void *param
)
{
    BLE_Request req;
    BLE_Request *req_ptr = &req;

    memset(&req, 0, sizeof(req));

    req.type = type;
    req.param = param;
    req.result = Bluetooth_Busy;

    if(NeonRTOS_SyncObjCreate(&req.done) != NeonRTOS_OK)
    {
        return Bluetooth_MemoryError;
    }

    if(NeonRTOS_MsgQWrite(
        &g_ble_cmd_queue,
        &req_ptr,
        SPBTLE_CMD_TIMEOUT_MS
    ) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&req.done);
        return Bluetooth_Busy;
    }

    if(NeonRTOS_SyncObjWait(
        &req.done,
        SPBTLE_CMD_TIMEOUT_MS
    ) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&req.done);
        return Bluetooth_Busy;
    }

    NeonRTOS_SyncObjDelete(&req.done);

    return req.result;
}

Bluetooth_OpResult Bluetooth_Set_TX_Power(Bluetooth_PA_Level pa_level)
{
    BLE_SetTxPowerParam param;

    if(pa_level >= Bluetooth_PA_Level_MAX)
    {
        return Bluetooth_InvalidParameter;
    }

    param.pa_level = pa_level;

    return SPBTLE_SendRequest(BLE_REQ_SET_TX_POWER, &param);
}

Bluetooth_OpResult Bluetooth_Set_Scan_Response_Data(uint8_t *pData)
{
    BLE_SetScanRespParam param;

    param.data = pData;
    param.len = 0;

    return SPBTLE_SendRequest(BLE_REQ_SET_SCAN_RESP, &param);
}

Bluetooth_OpResult Bluetooth_Set_Discoverable(void)
{
    return SPBTLE_SendRequest(BLE_REQ_SET_DISCOVERABLE, NULL);
}

static bool Bluetooth_UUID_From_String(
    const char *uuid_str,
    Bluetooth_UUID *uuid
)
{
    uint8_t uuid128[16];
    uint32_t value = 0;
    uint8_t nibble = 0;
    uint8_t count = 0;

    if(uuid_str == NULL || uuid == NULL)
    {
        return false;
    }

    /*
     * UUID16
     * e.g. "180F"
     */
    if(strlen(uuid_str) == 4)
    {
        if(sscanf(uuid_str, "%4hx", &uuid->uuid16) != 1)
        {
            return false;
        }

        uuid->type = Bluetooth_UUID_Type_16;

        return true;
    }

    /*
     * UUID128
     * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
     */
    if(strlen(uuid_str) != 36)
    {
        return false;
    }

    for(uint32_t i = 0; uuid_str[i] != '\0'; i++)
    {
        char c = uuid_str[i];

        if(c == '-')
        {
            continue;
        }

        if(c >= '0' && c <= '9')
        {
            nibble = c - '0';
        }
        else if(c >= 'a' && c <= 'f')
        {
            nibble = c - 'a' + 10;
        }
        else if(c >= 'A' && c <= 'F')
        {
            nibble = c - 'A' + 10;
        }
        else
        {
            return false;
        }

        value = (value << 4) | nibble;

        if((count & 1) != 0)
        {
            uuid128[count >> 1] = value & 0xFF;
            value = 0;
        }

        count++;
    }

    if(count != 32)
    {
        return false;
    }

    uuid->type = Bluetooth_UUID_Type_128;

    /*
     * Convert to BlueNRG byte order
     */
    for(uint32_t i = 0; i < 16; i++)
    {
        uuid->uuid128[i] = uuid128[15 - i];
    }

    return true;
}

Bluetooth_OpResult Bluetooth_Add_Service(
    const char *uuid,
    Bluetooth_Service_Type service_type,
    uint8_t max_attr_records,
    uint16_t *pServiceHandle
)
{
    BLE_AddServiceParam param;

    if(uuid == NULL || pServiceHandle == NULL)
    {
        return Bluetooth_InvalidParameter;
    }

    if(service_type >= Bluetooth_Service_Type_MAX)
    {
        return Bluetooth_InvalidParameter;
    }

    if(!Bluetooth_UUID_From_String(uuid, &param.uuid))
    {
        return Bluetooth_InvalidParameter;
    }

    param.service_type = service_type;
    param.max_attr_records = max_attr_records;
    param.service_handle = pServiceHandle;

    return SPBTLE_SendRequest(BLE_REQ_ADD_SERVICE, &param);
}

Bluetooth_OpResult Bluetooth_Service_Add_Char(
    uint16_t serviceHandle,
    const char *uuid,
    uint8_t charValueLen,
    Bluetooth_Char_Property property,
    Bluetooth_Char_Permission permission,
    uint16_t *pCharHandle
)
{
    BLE_AddCharParam param;

    if(uuid == NULL || pCharHandle == NULL)
    {
        return Bluetooth_InvalidParameter;
    }

    if(!Bluetooth_UUID_From_String(uuid, &param.uuid))
    {
        return Bluetooth_InvalidParameter;
    }

    param.service_handle = serviceHandle;
    param.char_value_len = charValueLen;
    param.property = property;
    param.permission = permission;
    param.char_handle = pCharHandle;

    return SPBTLE_SendRequest(BLE_REQ_ADD_CHAR, &param);
}

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
)
{
    BLE_AddCharDescParam param;

    if(uuid == NULL || pDescHandle == NULL)
    {
        return Bluetooth_InvalidParameter;
    }

    if(descValueLen > descValueMaxLen)
    {
        return Bluetooth_InvalidParameter;
    }

    if(!Bluetooth_UUID_From_String(uuid, &param.uuid))
    {
        return Bluetooth_InvalidParameter;
    }

    param.service_handle = serviceHandle;
    param.char_handle = charHandle;
    param.desc_value_max_len = descValueMaxLen;
    param.desc_value_len = descValueLen;
    param.desc_value = pDescValue;
    param.permission = permission;
    param.access = access;
    param.require_auth = requireAuth;
    param.encryption_key_size = encryptionKeySize;
    param.is_variable = isVariable;
    param.desc_handle = pDescHandle;

    return SPBTLE_SendRequest(BLE_REQ_ADD_CHAR_DESC, &param);
}

Bluetooth_OpResult Bluetooth_Service_Update_Char_Value(
    uint16_t serviceHandle,
    uint16_t charHandle,
    uint8_t offset,
    uint8_t valueLen,
    const uint8_t *pValue
)
{
    BLE_UpdateCharValueParam param;

    if(pValue == NULL && valueLen > 0)
    {
        return Bluetooth_InvalidParameter;
    }

    param.service_handle = serviceHandle;
    param.char_handle = charHandle;
    param.offset = offset;
    param.value_len = valueLen;
    param.value = pValue;

    return SPBTLE_SendRequest(BLE_REQ_UPDATE_CHAR_VALUE, &param);
}