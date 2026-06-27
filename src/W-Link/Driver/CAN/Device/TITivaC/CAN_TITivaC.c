#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "CAN/CAN.h"

#ifdef DEVICE_TITIVAC

#include "GPIO/Device/TITivaC/GPIO_TITivaC.h"

#include "CAN/Pin/TITivaC/CAN_Pin_TITivaC.h"

#include "CAN_TITivaC.h"

#if defined(CAN0_BASE) || defined(CAN1_BASE)

#define CAN_TX_MSG_OBJ      1
#define CAN_RX_MSG_OBJ      2
#define CAN_RX_QUEUE_LEN    8

static bool CAN_Init_Status[hwCAN_Index_MAX] = {false};
static NeonRTOS_SyncObj_t CAN_TxDone_Sync[hwCAN_Index_MAX];
static NeonRTOS_MsgQ_t CAN_RxQueue[hwCAN_Index_MAX];

void CAN_IRQ_Process(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
    {
        return;
    }

    uint32_t base = CAN_Map_Soc_Base(index);
    if(base==0)
    {
        return;
    }

    uint32_t cause = MAP_CANIntStatus(base, CAN_INT_STS_CAUSE);

    if (cause == CAN_INT_INTID_STATUS)
    {
        (void)MAP_CANStatusGet(base, CAN_STS_CONTROL);
        MAP_CANIntClear(base, cause);
        return;
    }

    if (cause == CAN_TX_MSG_OBJ)
    {
        MAP_CANIntClear(base, CAN_TX_MSG_OBJ);
        NeonRTOS_SyncObjSignalFromISR(&CAN_TxDone_Sync[index]);
        return;
    }

    if (cause == CAN_RX_MSG_OBJ)
    {
        tCANMsgObject msg;
        uint8_t data[8] = {0};

        memset(&msg, 0, sizeof(msg));
        msg.pui8MsgData = data;

        MAP_CANMessageGet(base, CAN_RX_MSG_OBJ, &msg, true);

        NeonRTOS_MsgQWrite(&CAN_RxQueue[index], data, NEONRT_NO_WAIT);
        return;
    }

    if (cause != 0)
    {
        MAP_CANIntClear(base, cause);
    }
}

static hwCAN_OpResult CAN_ConfigRxObject(hwCAN_Index index)
{
    uint32_t base = CAN_Map_Soc_Base(index);

    if (base == 0)
        return hwCAN_InvalidParameter;

    tCANMsgObject rx_msg;
    memset(&rx_msg, 0, sizeof(rx_msg));

    rx_msg.ui32MsgID = 0x000;
    rx_msg.ui32MsgIDMask = 0x000;
    rx_msg.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
    rx_msg.ui32MsgLen = 8;

    MAP_CANMessageSet(base, CAN_RX_MSG_OBJ, &rx_msg, MSG_OBJ_TYPE_RX);

    return hwCAN_OK;
}

hwCAN_OpResult CAN_Init(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    if (CAN_Init_Status[index])
        return hwCAN_OK;

    uint32_t can_base = CAN_Map_Soc_Base(index);
    uint32_t can_periph = CAN_Map_Soc_Periph(index);

    if (can_base == 0 || can_periph == 0)
        return hwCAN_InvalidParameter;

    hwGPIO_Pin tx_pin = CAN_Pin_Def_Table[index].tx_pin;
    hwGPIO_Pin rx_pin = CAN_Pin_Def_Table[index].rx_pin;

    uint32_t tx_port = GPIO_Map_Soc_Port_Base(tx_pin);
    uint32_t rx_port = GPIO_Map_Soc_Port_Base(rx_pin);

    uint32_t tx_mask = GPIO_Map_Soc_Pin_Mask(tx_pin);
    uint32_t rx_mask = GPIO_Map_Soc_Pin_Mask(rx_pin);

    uint32_t tx_cfg = CAN_Map_PinConfig(index, tx_pin);
    uint32_t rx_cfg = CAN_Map_PinConfig(index, rx_pin);

    uint32_t irq = CAN_Map_Soc_Int(index);

    if (tx_port == 0 || rx_port == 0 || tx_mask == 0 || rx_mask == 0 || tx_cfg == 0 || rx_cfg == 0 || irq == 0)
    {
        return hwCAN_InvalidParameter;
    }

    if (NeonRTOS_SyncObjCreate(&CAN_TxDone_Sync[index]) != NeonRTOS_OK)
        return hwCAN_MemoryError;

    if (NeonRTOS_MsgQCreate(&CAN_RxQueue[index], "can_rx", CAN_RX_QUEUE_LEN, 8) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        return hwCAN_MemoryError;
    }

    GPIO_Enable_Port_Clock(tx_port);
    GPIO_Enable_Port_Clock(rx_port);

    SysCtlPeripheralEnable(can_periph);

    while (!SysCtlPeripheralReady(can_periph));

    MAP_GPIOPinConfigure(tx_cfg);
    MAP_GPIOPinConfigure(rx_cfg);

    MAP_GPIOPinTypeCAN(tx_port, tx_mask);
    MAP_GPIOPinTypeCAN(rx_port, rx_mask);

    MAP_CANInit(can_base);

    if (MAP_CANBitRateSet(can_base, MAP_SysCtlClockGet(), 500000) == 0)
    {
        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);
        return hwCAN_HwError;
    }

    MAP_CANIntEnable(can_base, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);

    CAN_NVIC_Init(index);

    MAP_CANEnable(can_base);

    if (CAN_ConfigRxObject(index) != hwCAN_OK)
    {
        MAP_CANDisable(can_base);
        MAP_IntDisable(irq);
        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);
        return hwCAN_HwError;
    }

    gpio_pin_init_status[tx_pin] = true;
    gpio_pin_init_status[rx_pin] = true;

    CAN_Init_Status[index] = true;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_DeInit(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    if (!CAN_Init_Status[index])
        return hwCAN_OK;

    uint32_t can_base = CAN_Map_Soc_Base(index);

    if (can_base == 0)
        return hwCAN_InvalidParameter;

    hwGPIO_Pin tx_pin = CAN_Pin_Def_Table[index].tx_pin;
    hwGPIO_Pin rx_pin = CAN_Pin_Def_Table[index].rx_pin;

    uint32_t tx_port = GPIO_Map_Soc_Port_Base(tx_pin);
    uint32_t rx_port = GPIO_Map_Soc_Port_Base(rx_pin);

    uint32_t tx_mask = GPIO_Map_Soc_Pin_Mask(tx_pin);
    uint32_t rx_mask = GPIO_Map_Soc_Pin_Mask(rx_pin);

    uint32_t irq = CAN_Map_Soc_Int(index);

    if (tx_port == 0 || rx_port == 0 || tx_mask == 0 || rx_mask == 0 || irq == 0)
    {
        return hwCAN_InvalidParameter;
    }

    CAN_Init_Status[index] = false;

    MAP_CANIntDisable(can_base, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);

    CAN_NVIC_DeInit(index);

    MAP_CANDisable(can_base);

    if (tx_port && tx_mask)
    {
        MAP_GPIOPinTypeGPIOInput(tx_port, tx_mask);
    }

    if (rx_port && rx_mask)
    {
        MAP_GPIOPinTypeGPIOInput(rx_port, rx_mask);
    }

    NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
    NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);

    gpio_pin_init_status[tx_pin] = false;
    gpio_pin_init_status[rx_pin] = false;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_Read(hwCAN_Index index, uint8_t *buf, uint32_t timeout)
{
    if (index >= hwCAN_Index_MAX || buf == NULL)
        return hwCAN_InvalidParameter;

    if (!CAN_Init_Status[index])
        return hwCAN_NotInit;

    if (NeonRTOS_QueueReceive(&CAN_RxQueue[index], buf, timeout) != NeonRTOS_OK)
        return hwCAN_Timeout;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_Write(hwCAN_Index index, uint32_t id, uint8_t *data, uint8_t len, uint32_t timeout)
{
    if (index >= hwCAN_Index_MAX || data == NULL || len > 8)
        return hwCAN_InvalidParameter;

    if (!CAN_Init_Status[index])
        return hwCAN_NotInit;

    uint32_t base = CAN_Map_Soc_Base(index);

    if (base == 0)
        return hwCAN_InvalidParameter;

    tCANMsgObject tx_msg;
    memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg.ui32MsgID = id;
    tx_msg.ui32MsgIDMask = 0;
    tx_msg.ui32Flags = MSG_OBJ_TX_INT_ENABLE;
    tx_msg.ui32MsgLen = len;
    tx_msg.pui8MsgData = data;

    MAP_CANMessageSet(base, CAN_TX_MSG_OBJ, &tx_msg, MSG_OBJ_TYPE_TX);

    if (NeonRTOS_SyncObjWait(&CAN_TxDone_Sync[index], timeout) != NeonRTOS_OK)
        return hwCAN_Timeout;

    return hwCAN_OK;
}

bool CAN_isInit(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
        return false;

    return CAN_Init_Status[index];
}

#endif

#endif