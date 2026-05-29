#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "CAN/CAN.h"

#ifdef DEVICE_NUVOTON

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "CAN/Pin/Nuvoton/CAN_Pin_Nuvoton.h"

#include "CAN_Nuvoton.h"

static bool CAN_Init_Status[hwCAN_Index_MAX] = {false};
static NeonRTOS_SyncObj_t CAN_TxDone_Sync[hwCAN_Index_MAX];
static NeonRTOS_MsgQ_t CAN_RxQueue[hwCAN_Index_MAX];

void CAN_TxMailbox0CompleteCallback(hwCAN_Index index)
{
    if (index < hwCAN_Index_MAX)
        NeonRTOS_SyncObjSignalFromISR(&CAN_TxDone_Sync[index]);
}

void CAN_RxFifo0MsgPendingCallback(hwCAN_Index index, uint8_t data[8])
{
    if (index < hwCAN_Index_MAX)
        NeonRTOS_MsgQWrite(&CAN_RxQueue[index], data, NEONRT_NO_WAIT);
}

hwCAN_OpResult CAN_Init(hwCAN_Index index)
{
    if(index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    if(CAN_Init_Status[index])
        return hwCAN_OK;

    CAN_T *can = CAN_Map_Soc_Base(index);

    if(can == NULL)
        return hwCAN_InvalidParameter;

    GPIO_T *tx_soc_base =
        GPIO_Map_Soc_Base(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin);

    GPIO_T *rx_soc_base =
        GPIO_Map_Soc_Base(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin);

    uint16_t tx_soc_pin =
        GPIO_Map_Soc_Pin(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin);

    uint16_t rx_soc_pin =
        GPIO_Map_Soc_Pin(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin);

    if(tx_soc_pin == 0 || tx_soc_base == NULL ||
       rx_soc_pin == 0 || rx_soc_base == NULL)
    {
        return hwCAN_InvalidParameter;
    }

    if(NeonRTOS_SyncObjCreate(&CAN_TxDone_Sync[index]) != NeonRTOS_OK)
        return hwCAN_MemoryError;

    if(NeonRTOS_MsgQCreate(&CAN_RxQueue[index], "can_rx", 8, 8) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        return hwCAN_MemoryError;
    }

    CAN_GPIO_ConfigAF(index);

    GPIO_SetMode(tx_soc_base, tx_soc_pin, GPIO_MODE_OUTPUT);
    GPIO_SetMode(rx_soc_base, rx_soc_pin, GPIO_MODE_INPUT);

    hwCAN_OpResult result = CAN_Instance_Init(index, 500000);

    if(result != hwCAN_OK)
    {
        CAN_GPIO_DeConfigAF(index);

        GPIO_SetMode(tx_soc_base, tx_soc_pin, GPIO_MODE_INPUT);
        GPIO_SetMode(rx_soc_base, rx_soc_pin, GPIO_MODE_INPUT);

        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);

        return result;
    }

    result = CAN_ConfigFilter(index);

    if(result != hwCAN_OK)
    {
        CAN_Instance_DeInit(index);

        CAN_GPIO_DeConfigAF(index);

        GPIO_SetMode(tx_soc_base, tx_soc_pin, GPIO_MODE_INPUT);
        GPIO_SetMode(rx_soc_base, rx_soc_pin, GPIO_MODE_INPUT);

        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);

        return result;
    }

    result = CAN_StartHardware(index);

    if(result != hwCAN_OK)
    {
        CAN_Instance_DeInit(index);

        CAN_GPIO_DeConfigAF(index);

        GPIO_SetMode(tx_soc_base, tx_soc_pin, GPIO_MODE_INPUT);
        GPIO_SetMode(rx_soc_base, rx_soc_pin, GPIO_MODE_INPUT);

        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);

        return result;
    }

    CAN_NVIC_Init(index);

    gpio_pin_init_status[CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin] = true;
    gpio_pin_init_status[CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin] = true;

    CAN_Init_Status[index] = true;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_DeInit(hwCAN_Index index)
{
    if(index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    if(!CAN_Init_Status[index])
        return hwCAN_OK;

    GPIO_T *tx_soc_base =
        GPIO_Map_Soc_Base(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin);

    GPIO_T *rx_soc_base =
        GPIO_Map_Soc_Base(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin);

    uint16_t tx_soc_pin =
        GPIO_Map_Soc_Pin(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin);

    uint16_t rx_soc_pin =
        GPIO_Map_Soc_Pin(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin);

    if(tx_soc_pin == 0 || tx_soc_base == NULL ||
       rx_soc_pin == 0 || rx_soc_base == NULL)
    {
        return hwCAN_InvalidParameter;
    }

    CAN_NVIC_DeInit(index);

    CAN_StopHardware(index);
    CAN_Instance_DeInit(index);

    CAN_GPIO_DeConfigAF(index);

    GPIO_SetMode(tx_soc_base, tx_soc_pin, GPIO_MODE_INPUT);
    GPIO_SetMode(rx_soc_base, rx_soc_pin, GPIO_MODE_INPUT);

    NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
    NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);

    gpio_pin_init_status[CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin] = false;
    gpio_pin_init_status[CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin] = false;

    CAN_Init_Status[index] = false;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_Read(hwCAN_Index index, uint8_t *buf, uint32_t timeout)
{
    if(index >= hwCAN_Index_MAX || buf == NULL)
        return hwCAN_InvalidParameter;

    if(!CAN_Init_Status[index])
        return hwCAN_NotInit;

    if(NeonRTOS_MsgQRead(&CAN_RxQueue[index], buf, timeout) != NeonRTOS_OK)
        return hwCAN_Timeout;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_Write(hwCAN_Index index,
                         uint32_t id,
                         uint8_t *data,
                         uint8_t len,
                         uint32_t timeout)
{
    if(index >= hwCAN_Index_MAX || data == NULL || len > 8)
        return hwCAN_InvalidParameter;

    if(!CAN_Init_Status[index])
        return hwCAN_NotInit;

    CAN_T *can = CAN_Map_Soc_Base(index);

    if(can == NULL)
        return hwCAN_InvalidParameter;

    STR_CANMSG_T msg = {0};

    msg.FrameType = DATA_FRAME;
    msg.IdType    = CAN_STD_ID;
    msg.Id        = id;
    msg.DLC       = len;

    for(uint8_t i = 0; i < len; i++)
        msg.Data[i] = data[i];

    if(CAN_Transmit(can, 0, &msg) == FALSE)
        return hwCAN_HwError;

    if(NeonRTOS_SyncObjWait(&CAN_TxDone_Sync[index], timeout) != NeonRTOS_OK)
        return hwCAN_Timeout;

    return hwCAN_OK;
}

bool CAN_isInit(hwCAN_Index index)
{
    if(index >= hwCAN_Index_MAX)
        return false;

    return CAN_Init_Status[index];
}

#endif // DEVICE_NUVOTON