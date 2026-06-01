
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#ifdef STM32H5

#include "DMA_STM32_Index.h"

#include "DMA_STM32.h"

//#include "CAN/Device/STM32/CAN_STM32.h"
#include "I2C/Device/STM32/I2C_Master_STM32.h"
#include "SPI/Device/STM32/SPI_Master_STM32.h"
#include "UART/Device/STM32/UART_STM32.h"

#define DMA_CHANNEL_LOCK(channel_index) if (NeonRTOS_LockObjLock(&DMA_Channel_Mutex[(channel_index)], DMA_WAIT_ALLOCATED_TIMEOUT) != NeonRTOS_OK) { return hwDMA_MutexTimeout; }
#define DMA_CHANNEL_UNLOCK(channel_index) if (NeonRTOS_LockObjUnlock(&DMA_Channel_Mutex[(channel_index)]) != NeonRTOS_OK) { return hwDMA_MutexTimeout; }

static const hwDMA_Channel_Index UART_DMA_Channel_Map[hwUART_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined (UART1_BASE) || defined(USART1_BASE)
    {hwDMA_Channel_Index_0, hwDMA_Channel_Index_1},
#endif
#if defined (UART2_BASE) || defined(USART2_BASE)
    {hwDMA_Channel_Index_2, hwDMA_Channel_Index_3},
#endif
#if defined (UART3_BASE) || defined(USART3_BASE)
    {hwDMA_Channel_Index_4, hwDMA_Channel_Index_5},
#endif
#if defined (UART4_BASE) || defined(USART4_BASE)
    {hwDMA_Channel_Index_6, hwDMA_Channel_Index_7},
#endif
#if defined (UART5_BASE) || defined(USART5_BASE)
    {hwDMA_Channel_Index_8, hwDMA_Channel_Index_9},
#endif
#if defined (UART6_BASE) || defined(USART6_BASE)
    {hwDMA_Channel_Index_10, hwDMA_Channel_Index_11},
#endif
#if defined (UART7_BASE) || defined(USART7_BASE)
    {hwDMA_Channel_Index_12, hwDMA_Channel_Index_13},
#endif
#if defined (UART8_BASE) || defined(USART8_BASE)
    {hwDMA_Channel_Index_14, hwDMA_Channel_Index_15},
#endif
#if defined (UART9_BASE) || defined(USART9_BASE)
    {hwDMA_Channel_Index_0, hwDMA_Channel_Index_1},
#endif
#if defined (UART10_BASE) || defined(USART10_BASE)
    {hwDMA_Channel_Index_2, hwDMA_Channel_Index_3},
#endif
#if defined (UART11_BASE) || defined(USART11_BASE)
    {hwDMA_Channel_Index_4, hwDMA_Channel_Index_5},
#endif
#if defined (UART12_BASE) || defined(USART12_BASE)
    {hwDMA_Channel_Index_6, hwDMA_Channel_Index_7},
#endif
#if defined (LPUART1_BASE)
    {hwDMA_Channel_Index_8, hwDMA_Channel_Index_9},
#endif
};

static const hwDMA_Channel_Index SPI_DMA_Channel_Map[hwSPI_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(SPI1_BASE)
    {hwDMA_Channel_Index_0, hwDMA_Channel_Index_1},
#endif
#if defined(SPI2_BASE)
    {hwDMA_Channel_Index_2, hwDMA_Channel_Index_3},
#endif
#if defined(SPI3_BASE)
    {hwDMA_Channel_Index_4, hwDMA_Channel_Index_5},
#endif
#if defined(SPI4_BASE)
    {hwDMA_Channel_Index_6, hwDMA_Channel_Index_7},
#endif
#if defined(SPI5_BASE)
    {hwDMA_Channel_Index_8, hwDMA_Channel_Index_9},
#endif
#if defined(SPI6_BASE)
    {hwDMA_Channel_Index_10, hwDMA_Channel_Index_11},
#endif
};

static const hwDMA_Channel_Index I2C_DMA_Channel_Map[hwI2C_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(I2C1_BASE)
    {hwDMA_Channel_Index_0, hwDMA_Channel_Index_1},
#endif
#if defined(I2C2_BASE)
    {hwDMA_Channel_Index_2, hwDMA_Channel_Index_3},
#endif
#if defined(I2C3_BASE)
    {hwDMA_Channel_Index_4, hwDMA_Channel_Index_5},
#endif
#if defined(I2C4_BASE)
    {hwDMA_Channel_Index_6, hwDMA_Channel_Index_7},
#endif
};

DMA_TypeDef * DMA_Map_Soc_Base(hwDMA_Channel_Index index)
{
    switch(index)
    {
#if defined (GPDMA1_BASE)
#if defined (GPDMA1_Channel0)
        case hwDMA_Channel_Index_0:
#endif
#if defined (GPDMA1_Channel1)
        case hwDMA_Channel_Index_1:
#endif
#if defined (GPDMA1_Channel2)
        case hwDMA_Channel_Index_2:
#endif
#if defined (GPDMA1_Channel3)
        case hwDMA_Channel_Index_3:
#endif
#if defined (GPDMA1_Channel4)
        case hwDMA_Channel_Index_4:
#endif
#if defined (GPDMA1_Channel5)
        case hwDMA_Channel_Index_5:
#endif
#if defined (GPDMA1_Channel6)
        case hwDMA_Channel_Index_6:
#endif
#if defined (GPDMA1_Channel7)
        case hwDMA_Channel_Index_7:
#endif
                return GPDMA1;
#endif //GPDMA1_BASE
#if defined (GPDMA2_BASE)
#if defined (GPDMA2_Channel0)
        case hwDMA_Channel_Index_8:
#endif
#if defined (GPDMA2_Channel1)
        case hwDMA_Channel_Index_9:
#endif
#if defined (GPDMA2_Channel2)
        case hwDMA_Channel_Index_10:
#endif
#if defined (GPDMA2_Channel3)
        case hwDMA_Channel_Index_11:
#endif
#if defined (GPDMA2_Channel4)
        case hwDMA_Channel_Index_12:
#endif
#if defined (GPDMA2_Channel5)
        case hwDMA_Channel_Index_13:
#endif
#if defined (GPDMA2_Channel6)
        case hwDMA_Channel_Index_14:
#endif
#if defined (GPDMA2_Channel7)
        case hwDMA_Channel_Index_15:
#endif
                return GPDMA2;
#endif //GPDMA2_BASE
        default:
                break;
    }
    return NULL;
}

DMA_Channel_TypeDef * DMA_Map_Soc_Channel_Base(hwDMA_Channel_Index index)
{
    switch(index)
    {
#if defined (GPDMA1_BASE)
#if defined (GPDMA1_Channel0)
        case hwDMA_Channel_Index_0: return GPDMA1_Channel0;
#endif
#if defined (GPDMA1_Channel1)
        case hwDMA_Channel_Index_1: return GPDMA1_Channel1;
#endif
#if defined (GPDMA1_Channel2)
        case hwDMA_Channel_Index_2: return GPDMA1_Channel2;
#endif
#if defined (GPDMA1_Channel3)
        case hwDMA_Channel_Index_3: return GPDMA1_Channel3;
#endif
#if defined (GPDMA1_Channel4)
        case hwDMA_Channel_Index_4: return GPDMA1_Channel4;
#endif
#if defined (GPDMA1_Channel5)
        case hwDMA_Channel_Index_5: return GPDMA1_Channel5;
#endif
#if defined (GPDMA1_Channel6)
        case hwDMA_Channel_Index_6: return GPDMA1_Channel6;
#endif
#if defined (GPDMA1_Channel7)
        case hwDMA_Channel_Index_7: return GPDMA1_Channel7;
#endif
#endif // GPDMA1_BASE
#if defined (GPDMA2_BASE)
#if defined (GPDMA2_Channel0)
        case hwDMA_Channel_Index_8: return GPDMA2_Channel0;
#endif
#if defined (GPDMA2_Channel1)
        case hwDMA_Channel_Index_9: return GPDMA2_Channel1;
#endif
#if defined (GPDMA2_Channel2)
        case hwDMA_Channel_Index_10: return GPDMA2_Channel2;
#endif
#if defined (GPDMA2_Channel3)
        case hwDMA_Channel_Index_11: return GPDMA2_Channel3;
#endif
#if defined (GPDMA2_Channel4)
        case hwDMA_Channel_Index_12: return GPDMA2_Channel4;
#endif
#if defined (GPDMA2_Channel5)
        case hwDMA_Channel_Index_13: return GPDMA2_Channel5;
#endif
#if defined (GPDMA2_Channel6)
        case hwDMA_Channel_Index_14: return GPDMA2_Channel6;
#endif
#if defined (GPDMA2_Channel7)
        case hwDMA_Channel_Index_15: return GPDMA2_Channel7;
#endif
#endif // GPDMA2_BASE
        default: break;
    }
    return NULL;
}

static void DMA_IRQ_Handler(hwDMA_Channel_Index index)
{
        if(index>=hwDMA_Channel_Index_MAX)
        {
                return;
        }

        HAL_DMA_IRQHandler(&g_dma[index]);
}

#if defined(GPDMA1_BASE)
#if defined(GPDMA1_Channel0)
void GPDMA1_Channel0_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_0); }
#endif
#if defined(GPDMA1_Channel1)
void GPDMA1_Channel1_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_1); }
#endif
#if defined(GPDMA1_Channel2)
void GPDMA1_Channel2_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_2); }
#endif
#if defined(GPDMA1_Channel3)
void GPDMA1_Channel3_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_3); }
#endif
#if defined(GPDMA1_Channel4)
void GPDMA1_Channel4_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_4); }
#endif
#if defined(GPDMA1_Channel5)
void GPDMA1_Channel5_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_5); }
#endif
#if defined(GPDMA1_Channel6)
void GPDMA1_Channel6_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_6); }
#endif
#if defined(GPDMA1_Channel7)
void GPDMA1_Channel7_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_7); }
#endif
#endif // GPDMA1_BASE

#if defined(GPDMA2_BASE)
#if defined(GPDMA2_Channel0)
void GPDMA2_Channel0_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_8); }
#endif
#if defined(GPDMA2_Channel1)
void GPDMA2_Channel1_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_9); }
#endif
#if defined(GPDMA2_Channel2)
void GPDMA2_Channel2_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_10); }
#endif
#if defined(GPDMA2_Channel3)
void GPDMA2_Channel3_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_11); }
#endif
#if defined(GPDMA2_Channel4)
void GPDMA2_Channel4_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_12); }
#endif
#if defined(GPDMA2_Channel5)
void GPDMA2_Channel5_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_13); }
#endif
#if defined(GPDMA2_Channel6)
void GPDMA2_Channel6_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_14); }
#endif
#if defined(GPDMA2_Channel7)
void GPDMA2_Channel7_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_15); }
#endif
#endif // GPDMA2_BASE

void DMA_Clock_Enable()
{
#if defined (GPDMA1_BASE)
         __HAL_RCC_GPDMA1_CLK_ENABLE();
#endif //GPDMA1_BASE

#if defined (GPDMA2_BASE)
         __HAL_RCC_GPDMA2_CLK_ENABLE();
#endif //GPDMA2_BASE

}

void DMA_Clock_Disable()
{
#if defined (GPDMA1_BASE)
        __HAL_RCC_GPDMA1_CLK_DISABLE();
#endif //GPDMA1_BASE

#if defined (GPDMA2_BASE)
         __HAL_RCC_GPDMA2_CLK_DISABLE();
#endif //GPDMA2_BASE
}

hwDMA_OpResult DMA_NVIC_Init(hwDMA_Channel_Index channel_index)
{
        if(channel_index>=hwDMA_Channel_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Channel_Init_Status[channel_index]==true)
        {
                return hwDMA_OK;
        }

        switch(channel_index)
        {
#if defined(GPDMA1_BASE)
#if defined(GPDMA1_Channel0)
                case hwDMA_Channel_Index_0:
                        HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel1)
                case hwDMA_Channel_Index_1:
                        HAL_NVIC_SetPriority(GPDMA1_Channel1_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA1_Channel1_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel2)
                case hwDMA_Channel_Index_2:
                        HAL_NVIC_SetPriority(GPDMA1_Channel2_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA1_Channel2_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel3)
                case hwDMA_Channel_Index_3:
                        HAL_NVIC_SetPriority(GPDMA1_Channel3_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA1_Channel3_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel4)
                case hwDMA_Channel_Index_4:
                        HAL_NVIC_SetPriority(GPDMA1_Channel4_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA1_Channel4_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel5)
                case hwDMA_Channel_Index_5:
                        HAL_NVIC_SetPriority(GPDMA1_Channel5_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA1_Channel5_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel6)
                case hwDMA_Channel_Index_6:
                        HAL_NVIC_SetPriority(GPDMA1_Channel6_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA1_Channel6_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel7)
                case hwDMA_Channel_Index_7:
                        HAL_NVIC_SetPriority(GPDMA1_Channel7_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA1_Channel7_IRQn);
                        break;
#endif
#endif // GPDMA1_BASE

#if defined(GPDMA2_BASE)
#if defined(GPDMA2_Channel0)
                case hwDMA_Channel_Index_8:
                        HAL_NVIC_SetPriority(GPDMA2_Channel0_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA2_Channel0_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel1)
                case hwDMA_Channel_Index_9:
                        HAL_NVIC_SetPriority(GPDMA2_Channel1_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA2_Channel1_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel2)
                case hwDMA_Channel_Index_10:
                        HAL_NVIC_SetPriority(GPDMA2_Channel2_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA2_Channel2_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel3)
                case hwDMA_Channel_Index_11:
                        HAL_NVIC_SetPriority(GPDMA2_Channel3_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA2_Channel3_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel4)
                case hwDMA_Channel_Index_12:
                        HAL_NVIC_SetPriority(GPDMA2_Channel4_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA2_Channel4_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel5)
                case hwDMA_Channel_Index_13:
                        HAL_NVIC_SetPriority(GPDMA2_Channel5_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA2_Channel5_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel6)
                case hwDMA_Channel_Index_14:
                        HAL_NVIC_SetPriority(GPDMA2_Channel6_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA2_Channel6_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel7)
                case hwDMA_Channel_Index_15:
                        HAL_NVIC_SetPriority(GPDMA2_Channel7_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(GPDMA2_Channel7_IRQn);
                        break;
#endif
#endif // GPDMA2_BASE
        }

        return hwDMA_OK;
}

hwDMA_OpResult DMA_NVIC_DeInit(hwDMA_Channel_Index channel_index)
{
        if(channel_index>=hwDMA_Channel_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        switch(channel_index)
        {
#if defined(GPDMA1_BASE)
#if defined(GPDMA1_Channel0)
                case hwDMA_Channel_Index_0:
                        HAL_NVIC_DisableIRQ(GPDMA1_Channel0_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel1)
                case hwDMA_Channel_Index_1:
                        HAL_NVIC_DisableIRQ(GPDMA1_Channel1_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel2)
                case hwDMA_Channel_Index_2:
                        HAL_NVIC_DisableIRQ(GPDMA1_Channel2_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel3)
                case hwDMA_Channel_Index_3:
                        HAL_NVIC_DisableIRQ(GPDMA1_Channel3_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel4)
                case hwDMA_Channel_Index_4:
                        HAL_NVIC_DisableIRQ(GPDMA1_Channel4_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel5)
                case hwDMA_Channel_Index_5:
                        HAL_NVIC_DisableIRQ(GPDMA1_Channel5_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel6)
                case hwDMA_Channel_Index_6:
                        HAL_NVIC_DisableIRQ(GPDMA1_Channel6_IRQn);
                        break;
#endif
#if defined(GPDMA1_Channel7)
                case hwDMA_Channel_Index_7:
                        HAL_NVIC_DisableIRQ(GPDMA1_Channel7_IRQn);
                        break;
#endif
#endif // GPDMA1_BASE

#if defined(GPDMA2_BASE)
#if defined(GPDMA2_Channel0)
                case hwDMA_Channel_Index_8:
                        HAL_NVIC_DisableIRQ(GPDMA2_Channel0_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel1)
                case hwDMA_Channel_Index_9:
                        HAL_NVIC_DisableIRQ(GPDMA2_Channel1_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel2)
                case hwDMA_Channel_Index_10:
                        HAL_NVIC_DisableIRQ(GPDMA2_Channel2_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel3)
                case hwDMA_Channel_Index_11:
                        HAL_NVIC_DisableIRQ(GPDMA2_Channel3_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel4)
                case hwDMA_Channel_Index_12:
                        HAL_NVIC_DisableIRQ(GPDMA2_Channel4_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel5)
                case hwDMA_Channel_Index_13:
                        HAL_NVIC_DisableIRQ(GPDMA2_Channel5_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel6)
                case hwDMA_Channel_Index_14:
                        HAL_NVIC_DisableIRQ(GPDMA2_Channel6_IRQn);
                        break;
#endif
#if defined(GPDMA2_Channel7)
                case hwDMA_Channel_Index_15:
                        HAL_NVIC_DisableIRQ(GPDMA2_Channel7_IRQn);
                        break;
#endif
#endif // GPDMA2_BASE
        }

        return hwDMA_OK;
}

static void* DMA_Get_Instance(hwDMA_Channel_Index channel_index)
{
    if (channel_index >= hwDMA_Channel_Index_MAX)
    {
        return NULL;
    }

    return DMA_Map_Soc_Channel_Base(channel_index);
}

static hwDMA_OpResult DMA_DeConfig(hwDMA_Channel_Index channel_index)
{
    if (channel_index >= hwDMA_Channel_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    if (DMA_Channel_Init_Status[channel_index] == false)
    {
        return hwDMA_NotInit;
    }

    if (g_dma[channel_index].Instance != NULL)
    {
        HAL_DMA_Abort(&g_dma[channel_index]);

        if (HAL_DMA_DeInit(&g_dma[channel_index]) != HAL_OK)
        {
            return hwDMA_HwError;
        }
    }

    memset(&g_dma[channel_index], 0, sizeof(DMA_HandleTypeDef));

    return hwDMA_OK;
}

static hwDMA_OpResult DMA_Config_UART(hwDMA_Channel_Index channel_index, hwDMA_Peripheral_Direction dir, hwUART_Index index)
{
        if(dir>=hwDMA_Peripheral_Direction_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(channel_index>=hwDMA_Channel_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwUART_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if (DMA_Channel_Init_Status[channel_index] == false)
        {
                return hwDMA_NotInit;
        }

        g_dma[channel_index].Instance = DMA_Get_Instance(channel_index);
        if (g_dma[channel_index].Instance == NULL)
        {
                return hwDMA_InvalidParameter;
        }

        g_dma[channel_index].Init.BlkHWRequest          = DMA_BREQ_SINGLE_BURST;
        g_dma[channel_index].Init.SrcInc                = DMA_SINC_INCREMENTED;
        g_dma[channel_index].Init.DestInc               = DMA_DINC_FIXED;
        g_dma[channel_index].Init.SrcDataWidth          = DMA_SRC_DATAWIDTH_BYTE;
        g_dma[channel_index].Init.DestDataWidth         = DMA_DEST_DATAWIDTH_BYTE;
        g_dma[channel_index].Init.Priority              = DMA_LOW_PRIORITY_LOW_WEIGHT;
        g_dma[channel_index].Init.SrcBurstLength        = 1;
        g_dma[channel_index].Init.DestBurstLength       = 1;
        g_dma[channel_index].Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        g_dma[channel_index].Init.TransferEventMode     = DMA_TCEM_BLOCK_TRANSFER;
        g_dma[channel_index].Init.Mode                  = DMA_NORMAL;

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[channel_index].Init.Direction = DMA_MEMORY_TO_PERIPH;
                        g_dma[channel_index].Init.SrcInc    = DMA_SINC_INCREMENTED;
                        g_dma[channel_index].Init.DestInc   = DMA_DINC_FIXED;
                        
                        switch (index)
                        {
#if defined (UART1_BASE)
                                case hwUART_Index_0:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART1_TX;
                                        break;
#endif
#if defined(USART1_BASE)
                                case hwUART_Index_0:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART1_TX;
                                        break;
#endif
#if defined (UART2_BASE)
                                case hwUART_Index_1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART2_TX;
                                        break;
#endif
#if defined(USART2_BASE)
                                case hwUART_Index_1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART2_TX;
                                        break;
#endif
#if defined (UART3_BASE)
                                case hwUART_Index_2:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART3_TX;
                                        break;
#endif
#if defined(USART3_BASE)
                                case hwUART_Index_2:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART3_TX;
                                        break;
#endif
#if defined (UART4_BASE)
                                case hwUART_Index_3:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART4_TX;
                                        break;
#endif
#if defined(USART4_BASE)
                                case hwUART_Index_3:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART4_TX;
                                        break;
#endif
#if defined (UART5_BASE)
                                case hwUART_Index_4:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART5_TX;
                                        break;
#endif
#if defined(USART5_BASE)
                                case hwUART_Index_4:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART5_TX;
                                        break;
#endif
#if defined (UART6_BASE)
                                case hwUART_Index_5:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART6_TX;
                                        break;
#endif
#if defined(USART6_BASE)
                                case hwUART_Index_5:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART6_TX;
                                        break;
#endif
#if defined (UART7_BASE)
                                case hwUART_Index_6:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART7_TX;
                                        break;
#endif
#if defined(USART7_BASE)
                                case hwUART_Index_6:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART7_TX;
                                        break;
#endif
#if defined (UART8_BASE)
                                case hwUART_Index_7:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART8_TX;
                                        break;
#endif
#if defined(USART8_BASE)
                                case hwUART_Index_7:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART8_TX;
                                        break;
#endif
#if defined (UART9_BASE)
                                case hwUART_Index_8:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART9_TX;
                                        break;
#endif
#if defined(USART9_BASE)
                                case hwUART_Index_8:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART9_TX;
                                        break;
#endif
#if defined (UART10_BASE)
                                case hwUART_Index_9:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART10_TX;
                                        break;
#endif
#if defined(USART10_BASE)
                                case hwUART_Index_9:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART10_TX;
                                        break;
#endif
#if defined (UART11_BASE)
                                case hwUART_Index_10:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART11_TX;
                                        break;
#endif
#if defined(USART11_BASE)
                                case hwUART_Index_10:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART11_TX;
                                        break;
#endif
#if defined (UART12_BASE)
                                case hwUART_Index_11:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART12_TX;
                                        break;
#endif
#if defined(USART12_BASE)
                                case hwUART_Index_11:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART12_TX;
                                        break;
#endif
#if defined(LPUART1_BASE)
                                case hwUART_Index_L1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_LPUART1_TX;
                                        break;
#endif
                        }
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        g_dma[channel_index].Init.Direction = DMA_PERIPH_TO_MEMORY;
                        g_dma[channel_index].Init.SrcInc    = DMA_SINC_FIXED;
                        g_dma[channel_index].Init.DestInc   = DMA_DINC_INCREMENTED;
                        
                        switch (index)
                        {
#if defined (UART1_BASE)
                                case hwUART_Index_0:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART1_RX;
                                        break;
#endif
#if defined(USART1_BASE)
                                case hwUART_Index_0:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART1_RX;
                                        break;
#endif
#if defined (UART2_BASE)
                                case hwUART_Index_1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART2_RX;
                                        break;
#endif
#if defined(USART2_BASE)
                                case hwUART_Index_1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART2_RX;
                                        break;
#endif
#if defined (UART3_BASE)
                                case hwUART_Index_2:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART3_RX;
                                        break;
#endif
#if defined(USART3_BASE)
                                case hwUART_Index_2:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART3_RX;
                                        break;
#endif
#if defined (UART4_BASE)
                                case hwUART_Index_3:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART4_RX;
                                        break;
#endif
#if defined(USART4_BASE)
                                case hwUART_Index_3:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART4_RX;
                                        break;
#endif
#if defined (UART5_BASE)
                                case hwUART_Index_4:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART5_RX;
                                        break;
#endif
#if defined(USART5_BASE)
                                case hwUART_Index_4:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART5_RX;
                                        break;
#endif
#if defined (UART6_BASE)
                                case hwUART_Index_5:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART6_RX;
                                        break;
#endif
#if defined(USART6_BASE)
                                case hwUART_Index_5:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART6_RX;
                                        break;
#endif
#if defined (UART7_BASE)
                                case hwUART_Index_6:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART7_RX;
                                        break;
#endif
#if defined(USART7_BASE)
                                case hwUART_Index_6:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART7_RX;
                                        break;
#endif
#if defined (UART8_BASE)
                                case hwUART_Index_7:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART8_RX;
                                        break;
#endif
#if defined(USART8_BASE)
                                case hwUART_Index_7:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART8_RX;
                                        break;
#endif
#if defined (UART9_BASE)
                                case hwUART_Index_8:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART9_RX;
                                        break;
#endif
#if defined(USART9_BASE)
                                case hwUART_Index_8:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART9_RX;
                                        break;
#endif
#if defined (UART10_BASE)
                                case hwUART_Index_9:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART10_RX;
                                        break;
#endif
#if defined(USART10_BASE)
                                case hwUART_Index_9:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART10_RX;
                                        break;
#endif
#if defined (UART11_BASE)
                                case hwUART_Index_10:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART11_RX;
                                        break;
#endif
#if defined(USART11_BASE)
                                case hwUART_Index_10:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART11_RX;
                                        break;
#endif
#if defined (UART12_BASE)
                                case hwUART_Index_11:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_UART12_RX;
                                        break;
#endif
#if defined(USART12_BASE)
                                case hwUART_Index_11:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_USART12_RX;
                                        break;
#endif
#if defined(LPUART1_BASE)
                                case hwUART_Index_L1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_LPUART1_RX;
                                        break;
#endif
                        }
                        break;
        }

        if (HAL_DMA_Init(&g_dma[channel_index]) != HAL_OK)
        {
                return hwDMA_HwError;
        }

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        __HAL_LINKDMA(&g_uart[index], hdmatx, g_dma[channel_index]);
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        __HAL_LINKDMA(&g_uart[index], hdmarx, g_dma[channel_index]);
                        break;
        }

        return hwDMA_OK;
}

static hwDMA_OpResult DMA_Config_I2C(hwDMA_Channel_Index channel_index, hwDMA_Peripheral_Direction dir, hwI2C_Index index)
{
        if(dir>=hwDMA_Peripheral_Direction_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(channel_index>=hwDMA_Channel_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwI2C_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if (DMA_Channel_Init_Status[channel_index] == false)
        {
                return hwDMA_NotInit;
        }

        g_dma[channel_index].Instance = DMA_Get_Instance(channel_index);
        if(g_dma[channel_index].Instance == NULL)
        {
                return hwDMA_InvalidParameter;
        }

        g_dma[channel_index].Init.BlkHWRequest          = DMA_BREQ_SINGLE_BURST;
        g_dma[channel_index].Init.SrcInc                = DMA_SINC_INCREMENTED;
        g_dma[channel_index].Init.DestInc               = DMA_DINC_FIXED;
        g_dma[channel_index].Init.SrcDataWidth          = DMA_SRC_DATAWIDTH_BYTE;
        g_dma[channel_index].Init.DestDataWidth         = DMA_DEST_DATAWIDTH_BYTE;
        g_dma[channel_index].Init.Priority              = DMA_LOW_PRIORITY_LOW_WEIGHT;
        g_dma[channel_index].Init.SrcBurstLength        = 1;
        g_dma[channel_index].Init.DestBurstLength       = 1;
        g_dma[channel_index].Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        g_dma[channel_index].Init.TransferEventMode     = DMA_TCEM_BLOCK_TRANSFER;
        g_dma[channel_index].Init.Mode                  = DMA_NORMAL;

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[channel_index].Init.Direction = DMA_MEMORY_TO_PERIPH;
                        g_dma[channel_index].Init.SrcInc    = DMA_SINC_INCREMENTED;
                        g_dma[channel_index].Init.DestInc   = DMA_DINC_FIXED;
                        
                        switch (index)
                        {
#if defined (I2C1_BASE)
                                case hwI2C_Index_0:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_I2C1_TX;
                                        break;
#endif
#if defined (I2C2_BASE)
                                case hwI2C_Index_1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_I2C2_TX;
                                        break;
#endif
#if defined (I2C3_BASE)
                                case hwI2C_Index_2:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_I2C3_TX;
                                        break;
#endif
#if defined (I2C4_BASE)
                                case hwI2C_Index_3:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_I2C4_TX;
                                        break;
#endif
                        }
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        g_dma[channel_index].Init.Direction = DMA_PERIPH_TO_MEMORY;
                        g_dma[channel_index].Init.SrcInc    = DMA_SINC_FIXED;
                        g_dma[channel_index].Init.DestInc   = DMA_DINC_INCREMENTED;
                        
                        switch (index)
                        {
#if defined (I2C1_BASE)
                                case hwI2C_Index_0:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_I2C1_RX;
                                        break;
#endif
#if defined (I2C2_BASE)
                                case hwI2C_Index_1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_I2C2_RX;
                                        break;
#endif
#if defined (I2C3_BASE)
                                case hwI2C_Index_2:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_I2C3_RX;
                                        break;
#endif
#if defined (I2C4_BASE)
                                case hwI2C_Index_3:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_I2C4_RX;
                                        break;
#endif
                        }
                        break;
        }

        if (HAL_DMA_Init(&g_dma[channel_index]) != HAL_OK)
        {
                return hwDMA_HwError;
        }

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                __HAL_LINKDMA(&g_i2c[index], hdmatx, g_dma[channel_index]);
                break;

                case hwDMA_Peripheral_Direction_RX:
                __HAL_LINKDMA(&g_i2c[index], hdmarx, g_dma[channel_index]);
                break;

                default:
                return hwDMA_InvalidParameter;
        }

        return hwDMA_OK;
}

static hwDMA_OpResult DMA_Config_SPI(hwDMA_Channel_Index channel_index, hwDMA_Peripheral_Direction dir, hwSPI_Index index)
{
        if(dir>=hwDMA_Peripheral_Direction_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(channel_index>=hwDMA_Channel_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwSPI_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Channel_Init_Status[channel_index]==false)
        {
                return hwDMA_NotInit;
        }

        g_dma[channel_index].Instance = DMA_Get_Instance(channel_index);
        if(g_dma[channel_index].Instance == NULL)
        {
                return hwDMA_InvalidParameter;
        }

        g_dma[channel_index].Init.BlkHWRequest          = DMA_BREQ_SINGLE_BURST;
        g_dma[channel_index].Init.SrcInc                = DMA_SINC_INCREMENTED;
        g_dma[channel_index].Init.DestInc               = DMA_DINC_FIXED;
        g_dma[channel_index].Init.SrcDataWidth          = DMA_SRC_DATAWIDTH_BYTE;
        g_dma[channel_index].Init.DestDataWidth         = DMA_DEST_DATAWIDTH_BYTE;
        g_dma[channel_index].Init.Priority              = DMA_LOW_PRIORITY_LOW_WEIGHT;
        g_dma[channel_index].Init.SrcBurstLength        = 1;
        g_dma[channel_index].Init.DestBurstLength       = 1;
        g_dma[channel_index].Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        g_dma[channel_index].Init.TransferEventMode     = DMA_TCEM_BLOCK_TRANSFER;
        g_dma[channel_index].Init.Mode                  = DMA_NORMAL;

        switch(dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[channel_index].Init.Direction = DMA_MEMORY_TO_PERIPH;
                        g_dma[channel_index].Init.SrcInc    = DMA_SINC_INCREMENTED;
                        g_dma[channel_index].Init.DestInc   = DMA_DINC_FIXED;
                        
                        switch(index)
                        {
#if defined (SPI1_BASE)
                                case hwSPI_Index_0:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI1_TX;
                                        break;
#endif
#if defined (SPI2_BASE)
                                case hwSPI_Index_1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI2_TX;
                                        break;
#endif
#if defined (SPI3_BASE)
                                case hwSPI_Index_2:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI3_TX;
                                        break;
#endif
#if defined (SPI4_BASE)
                                case hwSPI_Index_3:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI4_TX;
                                        break;
#endif
#if defined (SPI5_BASE)
                                case hwSPI_Index_4:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI5_TX;
                                        break;
#endif
#if defined (SPI6_BASE)
                                case hwSPI_Index_5:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI6_TX;
                                        break;
#endif
                        }
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        g_dma[channel_index].Init.Direction = DMA_PERIPH_TO_MEMORY;
                        g_dma[channel_index].Init.SrcInc    = DMA_SINC_FIXED;
                        g_dma[channel_index].Init.DestInc   = DMA_DINC_INCREMENTED;
                        
                        switch(index)
                        {
#if defined (SPI1_BASE)
                                case hwSPI_Index_0:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI1_RX;
                                        break;
#endif
#if defined (SPI2_BASE)
                                case hwSPI_Index_1:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI2_RX;
                                        break;
#endif
#if defined (SPI3_BASE)
                                case hwSPI_Index_2:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI3_RX;
                                        break;
#endif
#if defined (SPI4_BASE)
                                case hwSPI_Index_3:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI4_RX;
                                        break;
#endif
#if defined (SPI5_BASE)
                                case hwSPI_Index_4:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI5_RX;
                                        break;
#endif
#if defined (SPI6_BASE)
                                case hwSPI_Index_5:
                                        g_dma[channel_index].Init.Request = GPDMA1_REQUEST_SPI6_RX;
                                        break;
#endif
                        }
                        break;
        }

        if (HAL_DMA_Init(&g_dma[channel_index]) != HAL_OK)
        {
                return hwDMA_HwError;
        }

        switch(dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        __HAL_LINKDMA(&g_spi[index], hdmatx, g_dma[channel_index]);
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        __HAL_LINKDMA(&g_spi[index], hdmarx, g_dma[channel_index]);
                        break;
        }

        return hwDMA_OK;
}

hwDMA_OpResult DMA_Xfer_UART(hwUART_Index index, hwDMA_Peripheral_Direction dir, uint8_t *buf, size_t len)
{
        if(buf==NULL)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwUART_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(dir>=hwDMA_Peripheral_Direction_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        hwDMA_Channel_Index channel_index = UART_DMA_Channel_Map[index][dir];
        
        if(channel_index>=hwDMA_Channel_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Channel_Init_Status[channel_index]==false)
        {
                return hwDMA_NotInit;
        }

        hwDMA_OpResult op_status;

        DMA_CHANNEL_LOCK(channel_index);

        op_status = DMA_Config_UART(channel_index, dir, index);
        if(op_status < hwDMA_OK)
        {
                DMA_CHANNEL_UNLOCK(channel_index);
                return op_status;
        }

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        if (HAL_UART_Transmit_DMA(&g_uart[index], buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(channel_index);
                                DMA_CHANNEL_UNLOCK(channel_index);
                                return hwDMA_HwError;
                        }
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        if (HAL_UART_Receive_DMA(&g_uart[index], buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(channel_index);
                                DMA_CHANNEL_UNLOCK(channel_index);
                                return hwDMA_HwError;
                        }
                        break;
        }

        while (HAL_UART_GetState(&g_uart[index]) != HAL_UART_STATE_READY) { }

        HAL_UART_DMAStop(&g_uart[index]);

        DMA_DeConfig(channel_index);
        
        DMA_CHANNEL_UNLOCK(channel_index);

        return hwDMA_OK;
}

hwDMA_OpResult DMA_Xfer_I2C(hwI2C_Index index, hwDMA_Peripheral_Direction dir, uint16_t dev_addr, uint8_t *buf, size_t len)
{
        if(buf==NULL)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwI2C_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(dir>=hwDMA_Peripheral_Direction_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        hwDMA_Channel_Index channel_index = I2C_DMA_Channel_Map[index][dir];
        
        if(channel_index>=hwDMA_Channel_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Channel_Init_Status[channel_index]==false)
        {
                return hwDMA_NotInit;
        }

        hwDMA_OpResult op_status;

        DMA_CHANNEL_LOCK(channel_index);

        op_status = DMA_Config_I2C(channel_index, dir, index);
        if(op_status < hwDMA_OK)
        {
                DMA_CHANNEL_UNLOCK(channel_index);
                return op_status;
        }

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        if (HAL_I2C_Master_Transmit_DMA(&g_i2c[index], dev_addr, buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(channel_index);
                                DMA_CHANNEL_UNLOCK(channel_index);
                                return hwDMA_HwError;
                        }
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        if (HAL_I2C_Master_Receive_DMA(&g_i2c[index], dev_addr, buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(channel_index);
                                DMA_CHANNEL_UNLOCK(channel_index);
                                return hwDMA_HwError;
                        }
                        break;
        }

        while (HAL_I2C_GetState(&g_i2c[index]) != HAL_I2C_STATE_READY) { }

        HAL_I2C_DMAStop(&g_i2c[index]);

        DMA_DeConfig(channel_index);

        DMA_CHANNEL_UNLOCK(channel_index);

        return hwDMA_OK;
}

hwDMA_OpResult DMA_Xfer_SPI(hwSPI_Index index, hwDMA_Peripheral_Direction dir, uint8_t* buf, size_t len)
{
        if(buf==NULL)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwSPI_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(dir>=hwDMA_Peripheral_Direction_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        hwDMA_Channel_Index channel_index = SPI_DMA_Channel_Map[index][dir];
        
        if(channel_index>=hwDMA_Channel_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Channel_Init_Status[channel_index]==false)
        {
                return hwDMA_NotInit;
        }

        hwDMA_OpResult op_status;

        DMA_CHANNEL_LOCK(channel_index);

        op_status = DMA_Config_SPI(channel_index, dir, index);
        if(op_status < hwDMA_OK)
        {
                DMA_CHANNEL_UNLOCK(channel_index);
                return op_status;
        }

        switch(dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        if (HAL_SPI_Transmit_DMA(&g_spi[index], buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(channel_index);
                                DMA_CHANNEL_UNLOCK(channel_index);
                                return hwDMA_HwError;
                        }
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        if (HAL_SPI_Receive_DMA(&g_spi[index], buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(channel_index);
                                DMA_CHANNEL_UNLOCK(channel_index);
                                return hwDMA_HwError;
                        }
                        break;
        }

        while (HAL_SPI_GetState(&g_spi[index]) != HAL_SPI_STATE_READY) { }
        
        HAL_SPI_DMAStop(&g_spi[index]);

        DMA_DeConfig(channel_index);

        DMA_CHANNEL_UNLOCK(channel_index);

        return op_status;
}

#endif //STM32H5
