#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "GPIO/GPIO.h"

#include "CAN/CAN.h"
#include "CAN/CAN_Pin.h"

#ifdef DEVICE_STM32

#include "GPIO/Device/GPIO_STM32.h"

#if defined(CAN1_BASE) || defined(CAN2_BASE) || defined(CAN3_BASE)

#include "CAN_STM32.h"

static bool CAN_Init_Status[hwCAN_Index_MAX] = {false};
static NeonRTOS_SyncObj_t CAN_TxDone_Sync[hwCAN_Index_MAX];
static NeonRTOS_MsgQ_t CAN_RxQueue[hwCAN_Index_MAX];

#ifndef STM32F1
uint32_t CAN_GetAF(hwCAN_Index can, hwGPIO_Pin pin)
{
    for (size_t i = 0; i < sizeof(CAN_Pin_AF_Map) / sizeof(CAN_Pin_AF_Map[0]); i++)
    {
        if (CAN_Pin_AF_Map[i].can == can &&
            CAN_Pin_AF_Map[i].pin == pin)
        {
            return CAN_Pin_AF_Map[i].af;
        }
    }

    return 0;
}
#endif

hwCAN_Index CAN_IndexFromHandle(CAN_HandleTypeDef *hcan)
{
    for (hwCAN_Index i = 0; i < hwCAN_Index_MAX; i++)
    {
        if (&g_can[i] == hcan)
            return i;
    }

    return hwCAN_Index_MAX;
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
    hwCAN_Index index = CAN_IndexFromHandle(hcan);

    if (index < hwCAN_Index_MAX)
        NeonRTOS_SyncObjSignalFromISR(&CAN_TxDone_Sync[index]);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef hdr;
    uint8_t data[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &hdr, data) != HAL_OK)
        return;

    hwCAN_Index index = CAN_IndexFromHandle(hcan);

    if (index < hwCAN_Index_MAX)
        NeonRTOS_MsgQWrite(&CAN_RxQueue[index], data, NEONRT_NO_WAIT);
}

hwCAN_OpResult CAN_Init(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    if (CAN_Init_Status[index])
        return hwCAN_OK;

    GPIO_TypeDef *tx_soc_base =
        GPIO_Map_Soc_Base(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin);

    GPIO_TypeDef *rx_soc_base =
        GPIO_Map_Soc_Base(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin);

    uint16_t tx_soc_pin =
        GPIO_Map_Soc_Pin(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin);

    uint16_t rx_soc_pin =
        GPIO_Map_Soc_Pin(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin);

    if (tx_soc_pin == 0 || tx_soc_base == NULL ||
        rx_soc_pin == 0 || rx_soc_base == NULL)
    {
        return hwCAN_InvalidParameter;
    }

#ifndef STM32F1
    uint32_t tx_af =
        CAN_GetAF(index, CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin);

    uint32_t rx_af =
        CAN_GetAF(index, CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin);

    if (tx_af == 0 || rx_af == 0)
        return hwCAN_InvalidParameter;
#endif

    if (NeonRTOS_SyncObjCreate(&CAN_TxDone_Sync[index]) != NeonRTOS_OK)
        return hwCAN_MemoryError;

    if (NeonRTOS_MsgQCreate(&CAN_RxQueue[index], "can_rx", 8, 8) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        return hwCAN_MemoryError;
    }

    GPIO_Enable_RCC_Clock(tx_soc_base);
    GPIO_Enable_RCC_Clock(rx_soc_base);

    GPIO_InitTypeDef g_can_tx = {0};
    g_can_tx.Pin       = tx_soc_pin;
    g_can_tx.Mode      = GPIO_MODE_AF_PP;
    g_can_tx.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_can_tx.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_can_tx.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_can_tx.Alternate = tx_af;
#endif
    HAL_GPIO_Init(tx_soc_base, &g_can_tx);

    GPIO_InitTypeDef g_can_rx = {0};
    g_can_rx.Pin       = rx_soc_pin;
    g_can_rx.Mode      = GPIO_MODE_AF_PP;
    g_can_rx.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_can_rx.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_can_rx.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_can_rx.Alternate = rx_af;
#endif
    HAL_GPIO_Init(rx_soc_base, &g_can_rx);

    hwCAN_OpResult result;

    result = CAN_Instance_Init(index, 500000);
    if (result != hwCAN_OK)
        goto fail;

    result = CAN_ConfigFilter(index);
    if (result != hwCAN_OK)
        goto fail_hw;

    result = CAN_StartHardware(index);
    if (result != hwCAN_OK)
        goto fail_hw;

    CAN_NVIC_Init(index);

    gpio_pin_init_status[CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin] = true;
    gpio_pin_init_status[CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin] = true;

    CAN_Init_Status[index] = true;

    return hwCAN_OK;

fail_hw:
    CAN_Instance_DeInit(index);

fail:
    HAL_GPIO_DeInit(tx_soc_base, tx_soc_pin);
    HAL_GPIO_DeInit(rx_soc_base, rx_soc_pin);
    NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
    NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);
    return result;
}

hwCAN_OpResult CAN_DeInit(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    if (!CAN_Init_Status[index])
        return hwCAN_OK;

    GPIO_TypeDef *tx_soc_base =
        GPIO_Map_Soc_Base(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin);

    GPIO_TypeDef *rx_soc_base =
        GPIO_Map_Soc_Base(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin);

    uint16_t tx_soc_pin =
        GPIO_Map_Soc_Pin(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin);

    uint16_t rx_soc_pin =
        GPIO_Map_Soc_Pin(CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin);

    if (tx_soc_pin == 0 || tx_soc_base == NULL ||
        rx_soc_pin == 0 || rx_soc_base == NULL)
    {
        return hwCAN_InvalidParameter;
    }

    CAN_NVIC_DeInit(index);

    CAN_StopHardware(index);
    CAN_Instance_DeInit(index);

    HAL_GPIO_DeInit(tx_soc_base, tx_soc_pin);
    HAL_GPIO_DeInit(rx_soc_base, rx_soc_pin);

    NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
    NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);

    gpio_pin_init_status[CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].tx_pin] = false;
    gpio_pin_init_status[CAN_Pin_Def_Table[index][CAN_Index_Map_Alt[index]].rx_pin] = false;

    CAN_Init_Status[index] = false;

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

    CAN_TxHeaderTypeDef hdr = {0};
    hdr.StdId = id;
    hdr.IDE   = CAN_ID_STD;
    hdr.RTR   = CAN_RTR_DATA;
    hdr.DLC   = len;

    uint32_t mailbox;

    if (HAL_CAN_AddTxMessage(&g_can[index], &hdr, data, &mailbox) != HAL_OK)
        return hwCAN_HwError;

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

#endif // CAN1_BASE || CAN2_BASE || CAN3_BASE

#endif // DEVICE_STM32