#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "soc.h"

#ifdef STM32F0
#if defined(CAN1_BASE)

#include "CAN/CAN.h"
#include "CAN_STM32.h"

CAN_HandleTypeDef g_can[hwCAN_Index_MAX];

static uint32_t CAN_CalcPrescaler(uint32_t can_clk, uint32_t bitrate)
{
    const uint32_t tq = 14; // 1 + 11 + 2

    uint32_t prescaler = can_clk / (bitrate * tq);

    if (prescaler == 0)
        prescaler = 1;

    if (prescaler > 1024)
        prescaler = 1024;

    return prescaler;
}

CAN_TypeDef *CAN_Map_Soc_Base(hwCAN_Index index)
{
    switch (index)
    {
        case hwCAN_Index_0:
            return CAN1;

        default:
            return NULL;
    }
}

uint32_t CAN_GetClock(hwCAN_Index index)
{
    (void)index;
    return HAL_RCC_GetPCLK1Freq();
}

static void CAN_HAL_IRQHandler(hwCAN_Index index)
{
    HAL_CAN_IRQHandler(&g_can[index]);
}

void CEC_CAN_IRQHandler(void)
{
    CAN_HAL_IRQHandler(hwCAN_Index_0);
}

hwCAN_OpResult CAN_Instance_Init(hwCAN_Index index, uint32_t bitrate)
{
    if (index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    if (index != hwCAN_Index_0)
        return hwCAN_InvalidParameter;

    CAN_TypeDef *can_soc_base = CAN_Map_Soc_Base(index);

    if (can_soc_base == NULL)
        return hwCAN_InvalidParameter;

    __HAL_RCC_CAN1_CLK_ENABLE();

    g_can[index].Instance = can_soc_base;

    g_can[index].Init.Prescaler = CAN_CalcPrescaler(CAN_GetClock(index), bitrate);
    g_can[index].Init.Mode = CAN_MODE_NORMAL;
    g_can[index].Init.SyncJumpWidth = CAN_SJW_1TQ;
    g_can[index].Init.TimeSeg1 = CAN_BS1_11TQ;
    g_can[index].Init.TimeSeg2 = CAN_BS2_2TQ;
    g_can[index].Init.TimeTriggeredMode = DISABLE;
    g_can[index].Init.AutoBusOff = ENABLE;
    g_can[index].Init.AutoWakeUp = DISABLE;
    g_can[index].Init.AutoRetransmission = ENABLE;
    g_can[index].Init.ReceiveFifoLocked = DISABLE;
    g_can[index].Init.TransmitFifoPriority = DISABLE;

    if (HAL_CAN_Init(&g_can[index]) != HAL_OK)
        return hwCAN_HwError;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_Instance_DeInit(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    if (index != hwCAN_Index_0)
        return hwCAN_InvalidParameter;

    HAL_CAN_DeInit(&g_can[index]);

    __HAL_RCC_CAN1_CLK_DISABLE();

    return hwCAN_OK;
}

hwCAN_OpResult CAN_ConfigFilter(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    CAN_FilterTypeDef filter = {0};
    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh = 0x0000;
    filter.FilterIdLow = 0x0000;
    filter.FilterMaskIdHigh = 0x0000;
    filter.FilterMaskIdLow = 0x0000;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterActivation = ENABLE;

    if (HAL_CAN_ConfigFilter(&g_can[index], &filter) != HAL_OK)
        return hwCAN_HwError;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_StartHardware(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    if (HAL_CAN_Start(&g_can[index]) != HAL_OK)
        return hwCAN_HwError;

    if (HAL_CAN_ActivateNotification(
            &g_can[index],
            CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        HAL_CAN_Stop(&g_can[index]);
        return hwCAN_HwError;
    }

    return hwCAN_OK;
}

hwCAN_OpResult CAN_StopHardware(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
        return hwCAN_InvalidParameter;

    HAL_CAN_Stop(&g_can[index]);

    return hwCAN_OK;
}

void CAN_NVIC_Init(hwCAN_Index index)
{
    (void)index;

    HAL_NVIC_SetPriority(
        CEC_CAN_IRQn,
        CAN_IRQ_NVIC_PRIORITY,
        CAN_IRQ_NVIC_SUB_PRIORITY
    );

    HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
}

void CAN_NVIC_DeInit(hwCAN_Index index)
{
    (void)index;
    HAL_NVIC_DisableIRQ(CEC_CAN_IRQn);
}

#endif // CAN1_BASE
#endif // STM32F0