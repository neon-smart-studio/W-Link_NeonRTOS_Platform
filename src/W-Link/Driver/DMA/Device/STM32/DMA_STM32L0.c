
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#ifdef STM32L0

#include "DMA_STM32_Index.h"

#include "DMA_STM32.h"

//#include "CAN/Device/STM32/CAN_STM32.h"
#include "I2C/Device/STM32/I2C_Master_STM32.h"
#include "SPI/Device/STM32/SPI_Master_STM32.h"
#include "UART/Device/STM32/UART_STM32.h"

#define DMA_CHANNEL_LOCK(channel_index) if (NeonRTOS_LockObjLock(&DMA_Channel_Mutex[(channel_index)], DMA_WAIT_ALLOCATED_TIMEOUT) != NeonRTOS_OK) { return hwDMA_MutexTimeout; }
#define DMA_CHANNEL_UNLOCK(channel_index) if (NeonRTOS_LockObjUnlock(&DMA_Channel_Mutex[(channel_index)]) != NeonRTOS_OK) { return hwDMA_MutexTimeout; }

static bool DMA_NVIC_Init_Status[hwDMA_Channel_Index_MAX] = {false};

static const hwDMA_Channel_Index UART_DMA_Channel_Map[hwUART_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(USART1_BASE)
    {hwDMA_Channel_Index_4, hwDMA_Channel_Index_5}, // USART1_TX CH4, RX CH5
#endif
#if defined(USART2_BASE)
    {hwDMA_Channel_Index_7, hwDMA_Channel_Index_6}, // USART2_TX CH7, RX CH6
#endif
#if defined(LPUART1_BASE)
    {hwDMA_Channel_Index_7, hwDMA_Channel_Index_6}, // LPUART1 常見 TX CH7, RX CH6
#endif
};

static const hwDMA_Channel_Index SPI_DMA_Channel_Map[hwSPI_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(SPI1_BASE)
    {hwDMA_Channel_Index_3, hwDMA_Channel_Index_2}, // SPI1_TX CH3, RX CH2
#endif
#if defined(SPI2_BASE)
    {hwDMA_Channel_Index_5, hwDMA_Channel_Index_4}, // SPI2_TX CH5, RX CH4
#endif
};

static const hwDMA_Channel_Index I2C_DMA_Channel_Map[hwI2C_Index_MAX][hwDMA_Peripheral_Direction_MAX] =
{
#if defined(I2C1_BASE)
    {hwDMA_Channel_Index_6, hwDMA_Channel_Index_7}, // I2C1_TX CH6, RX CH7
#endif
#if defined(I2C2_BASE)
    {hwDMA_Channel_Index_4, hwDMA_Channel_Index_5}, // I2C2_TX CH4, RX CH5
#endif
#if defined(I2C3_BASE)
    {hwDMA_Channel_Index_2, hwDMA_Channel_Index_3}, // I2C3_TX CH2, RX CH3
#endif
};

DMA_TypeDef * DMA_Map_Soc_Base(hwDMA_Channel_Index index)
{
    switch(index)
    {
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
        case hwDMA_Channel_Index_1:
#endif
#if defined (DMA1_Channel2)
        case hwDMA_Channel_Index_2:
#endif
#if defined (DMA1_Channel3)
        case hwDMA_Channel_Index_3:
#endif
#if defined (DMA1_Channel4)
        case hwDMA_Channel_Index_4:
#endif
#if defined (DMA1_Channel5)
        case hwDMA_Channel_Index_5:
#endif
#if defined (DMA1_Channel6)
        case hwDMA_Channel_Index_6:
#endif
#if defined (DMA1_Channel7)
        case hwDMA_Channel_Index_7:
#endif
                return DMA1;
#endif
        default:
                break;
    }
    return NULL;
}

DMA_Channel_TypeDef * DMA_Map_Soc_Channel_Base(hwDMA_Channel_Index index)
{
    switch(index)
    {
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
        case hwDMA_Channel_Index_1: return DMA1_Channel1;
#endif
#if defined (DMA1_Channel2)
        case hwDMA_Channel_Index_2: return DMA1_Channel2;
#endif
#if defined (DMA1_Channel3)
        case hwDMA_Channel_Index_3: return DMA1_Channel3;
#endif
#if defined (DMA1_Channel4)
        case hwDMA_Channel_Index_4: return DMA1_Channel4;
#endif
#if defined (DMA1_Channel5)
        case hwDMA_Channel_Index_5: return DMA1_Channel5;
#endif
#if defined (DMA1_Channel6)
        case hwDMA_Channel_Index_6: return DMA1_Channel6;
#endif
#if defined (DMA1_Channel7)
        case hwDMA_Channel_Index_7: return DMA1_Channel7;
#endif
#endif
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

#if defined (STM32L010x4) || defined (STM32L011xx) || defined (STM32L021xx)
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
void DMA1_Channel1_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_1); }
#endif
#if defined (DMA1_Channel2) || defined (DMA1_Channel3)
void DMA1_Channel2_3_IRQHandler(void)
{
#if defined (DMA1_Channel2)
        DMA_IRQ_Handler(hwDMA_Channel_Index_2);
#endif
#if defined (DMA1_Channel3)
        DMA_IRQ_Handler(hwDMA_Channel_Index_3);
#endif
}
#endif
#if defined (DMA1_Channel4) || defined (DMA1_Channel5)
void DMA1_Channel4_5_IRQHandler(void)
{
#if defined (DMA1_Channel4)
        DMA_IRQ_Handler(hwDMA_Channel_Index_4);
#endif
#if defined (DMA1_Channel5)
        DMA_IRQ_Handler(hwDMA_Channel_Index_5);
#endif
}
#endif
#endif //DMA1_BASE
#elif defined (STM32L010x6) || defined (STM32L010x8) || defined (STM32L010xB) || \
      defined (STM32L031xx) || defined (STM32L041xx) || \
      defined (STM32L051xx) || defined (STM32L052xx) || defined (STM32L053xx) || \
      defined (STM32L062xx) || defined (STM32L063xx) || \
      defined (STM32L071xx) || defined (STM32L072xx) || defined (STM32L073xx) || \
      defined (STM32L081xx) || defined (STM32L082xx) || defined (STM32L083xx)
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
void DMA1_Channel1_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_1); }
#endif
#if defined (DMA1_Channel2) || defined (DMA1_Channel3)
void DMA1_Channel2_3_IRQHandler(void)
{
#if defined (DMA1_Channel2)
        DMA_IRQ_Handler(hwDMA_Channel_Index_2);
#endif
#if defined (DMA1_Channel3)
        DMA_IRQ_Handler(hwDMA_Channel_Index_3);
#endif
}
#endif
#if defined (DMA1_Channel4) || defined (DMA1_Channel5)
void DMA1_Channel4_5_6_7_IRQHandler(void)
{
#if defined (DMA1_Channel4)
        DMA_IRQ_Handler(hwDMA_Channel_Index_4);
#endif
#if defined (DMA1_Channel5)
        DMA_IRQ_Handler(hwDMA_Channel_Index_5);
#endif
#if defined (DMA1_Channel6)
        DMA_IRQ_Handler(hwDMA_Channel_Index_6);
#endif
#if defined (DMA1_Channel7)
        DMA_IRQ_Handler(hwDMA_Channel_Index_7);
#endif
}
#endif
#endif //DMA1_BASE
#else
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
void DMA1_Channel1_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_1); }
#endif
#if defined (DMA1_Channel2)
void DMA1_Channel2_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_2); }
#endif
#if defined (DMA1_Channel3)
void DMA1_Channel3_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_3); }
#endif
#if defined (DMA1_Channel4)
void DMA1_Channel4_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_4); }
#endif
#if defined (DMA1_Channel5)
void DMA1_Channel5_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_5); }
#endif
#if defined (DMA1_Channel6)
void DMA1_Channel6_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_6); }
#endif
#if defined (DMA1_Channel7)
void DMA1_Channel7_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Channel_Index_7); }
#endif
#endif //DMA1_BASE
#endif

void DMA_Clock_Enable()
{
#if defined (DMA1_BASE)
         __HAL_RCC_DMA1_CLK_ENABLE();
#endif //DMA1_BASE
}

void DMA_Clock_Disable()
{
#if defined (DMA1_BASE)
        __HAL_RCC_DMA1_CLK_DISABLE();
#endif //DMA1_BASE
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
#if defined (STM32L010x4) || defined (STM32L011xx) || defined (STM32L021xx)
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
                case hwDMA_Channel_Index_1:
                        HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
                        break;
#endif
#if defined (DMA1_Channel2)
                case hwDMA_Channel_Index_2:
#endif
#if defined (DMA1_Channel3)
                case hwDMA_Channel_Index_3:
#endif
                        HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
                        break;
#if defined (DMA1_Channel4)
                case hwDMA_Channel_Index_4:
#endif
#if defined (DMA1_Channel5)
                case hwDMA_Channel_Index_5:
#endif
                        HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);
                        break;
#endif
#elif defined (STM32L010x6) || defined (STM32L010x8) || defined (STM32L010xB) || \
      defined (STM32L031xx) || defined (STM32L041xx) || \
      defined (STM32L051xx) || defined (STM32L052xx) || defined (STM32L053xx) || \
      defined (STM32L062xx) || defined (STM32L063xx) || \
      defined (STM32L071xx) || defined (STM32L072xx) || defined (STM32L073xx) || \
      defined (STM32L081xx) || defined (STM32L082xx) || defined (STM32L083xx)
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
                case hwDMA_Channel_Index_1:
                        HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
                        break;
#endif
#if defined (DMA1_Channel2)
                case hwDMA_Channel_Index_2:
#endif
#if defined (DMA1_Channel3)
                case hwDMA_Channel_Index_3:
#endif
                        HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
                        break;
#if defined (DMA1_Channel4)
                case hwDMA_Channel_Index_4:
#endif
#if defined (DMA1_Channel5)
                case hwDMA_Channel_Index_5:
#endif
#if defined (DMA1_Channel6)
                case hwDMA_Channel_Index_6:
#endif
#if defined (DMA1_Channel7)
                case hwDMA_Channel_Index_7:
#endif
                        HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);
                        break;
#endif
#else
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
                case hwDMA_Channel_Index_1:
                        HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
                        break;
#endif
#if defined (DMA1_Channel2)
                case hwDMA_Channel_Index_2:
                        HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
                        break;
#endif
#if defined (DMA1_Channel3)
                case hwDMA_Channel_Index_3:
                        HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
                        break;
#endif
#if defined (DMA1_Channel4)
                case hwDMA_Channel_Index_4:
                        HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
                        break;
#endif
#if defined (DMA1_Channel5)
                case hwDMA_Channel_Index_5:
                        HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
                        break;
#endif
#if defined (DMA1_Channel6)
                case hwDMA_Channel_Index_6:
                        HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
                        break;
#endif
#if defined (DMA1_Channel7)
                case hwDMA_Channel_Index_7:
                        HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
                        break;
#endif
#endif //DMA1_BASE
#endif
        }

        DMA_NVIC_Init_Status[channel_index] = true;

        return hwDMA_OK;
}

hwDMA_OpResult DMA_NVIC_DeInit(hwDMA_Channel_Index channel_index)
{
        if(channel_index>=hwDMA_Channel_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        DMA_NVIC_Init_Status[channel_index] = false;

        switch(channel_index)
        {
#if defined (STM32L010x4) || defined (STM32L011xx) || defined (STM32L021xx)
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
                case hwDMA_Channel_Index_1:
                        HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
                        break;
#endif
#if defined (DMA1_Channel2)
                case hwDMA_Channel_Index_2:
#endif
#if defined (DMA1_Channel3)
                case hwDMA_Channel_Index_3:
#endif
#if defined (DMA1_Channel2) && defined (DMA1_Channel3)
                        if(!DMA_NVIC_Init_Status[hwDMA_Channel_Index_2] && !DMA_NVIC_Init_Status[hwDMA_Channel_Index_3])
#endif
                        {
                                HAL_NVIC_DisableIRQ(DMA1_Channel2_3_IRQn);
                        }
                        break;
#if defined (DMA1_Channel4)
                case hwDMA_Channel_Index_4:
#endif
#if defined (DMA1_Channel5)
                case hwDMA_Channel_Index_5:
#endif
#if defined (DMA1_Channel2) && defined (DMA1_Channel3)
                        if(!DMA_NVIC_Init_Status[hwDMA_Channel_Index_4] && !DMA_NVIC_Init_Status[hwDMA_Channel_Index_5])
#endif
                        {
                                HAL_NVIC_DisableIRQ(DMA1_Channel4_5_IRQn);
                        }
                        break;
#endif
#elif defined (STM32L010x6) || defined (STM32L010x8) || defined (STM32L010xB) || \
      defined (STM32L031xx) || defined (STM32L041xx) || \
      defined (STM32L051xx) || defined (STM32L052xx) || defined (STM32L053xx) || \
      defined (STM32L062xx) || defined (STM32L063xx) || \
      defined (STM32L071xx) || defined (STM32L072xx) || defined (STM32L073xx) || \
      defined (STM32L081xx) || defined (STM32L082xx) || defined (STM32L083xx)
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
                case hwDMA_Channel_Index_1:
                        HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
                        break;
#endif
#if defined (DMA1_Channel2)
                case hwDMA_Channel_Index_2:
#endif
#if defined (DMA1_Channel3)
                case hwDMA_Channel_Index_3:
#endif
#if defined (DMA1_Channel2) && defined (DMA1_Channel3)
                        if(!DMA_NVIC_Init_Status[hwDMA_Channel_Index_2] && !DMA_NVIC_Init_Status[hwDMA_Channel_Index_3])
#endif
                        {
                                HAL_NVIC_DisableIRQ(DMA1_Channel2_3_IRQn);
                        }
                        break;
#if defined (DMA1_Channel4)
                case hwDMA_Channel_Index_4:
#endif
#if defined (DMA1_Channel5)
                case hwDMA_Channel_Index_5:
#endif
#if defined (DMA1_Channel6)
                case hwDMA_Channel_Index_6:
#endif
#if defined (DMA1_Channel7)
                case hwDMA_Channel_Index_7:
#endif
                        {
                                bool dma1_4_7_used = false;

#if defined (DMA1_Channel4)
                                if(DMA_NVIC_Init_Status[hwDMA_Channel_Index_4])
                                {
                                        dma1_4_7_used = true;
                                }
#endif
#if defined (DMA1_Channel5)
                                if(DMA_NVIC_Init_Status[hwDMA_Channel_Index_5])
                                {
                                        dma1_4_7_used = true;
                                }
#endif
#if defined (DMA1_Channel6)
                                if(DMA_NVIC_Init_Status[hwDMA_Channel_Index_6])
                                {
                                        dma1_4_7_used = true;
                                }
#endif
#if defined (DMA1_Channel7)
                                if(DMA_NVIC_Init_Status[hwDMA_Channel_Index_7])
                                {
                                        dma1_4_7_used = true;
                                }
#endif
                                if (!dma1_4_7_used)
                                {
                                        HAL_NVIC_DisableIRQ(DMA1_Channel4_5_6_7_IRQn);
                                }
                        }
                        break;
#endif
#else
#if defined (DMA1_BASE)
#if defined (DMA1_Channel1)
                case hwDMA_Channel_Index_1:
                        HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
                        break;
#endif
#if defined (DMA1_Channel2)
                case hwDMA_Channel_Index_2:
                        HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);
                        break;
#endif
#if defined (DMA1_Channel3)
                case hwDMA_Channel_Index_3:
                        HAL_NVIC_DisableIRQ(DMA1_Channel3_IRQn);
                        break;
#endif
#if defined (DMA1_Channel4)
                case hwDMA_Channel_Index_4:
                        HAL_NVIC_DisableIRQ(DMA1_Channel4_IRQn);
                        break;
#endif
#if defined (DMA1_Channel5)
                case hwDMA_Channel_Index_5:
                        HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
                        break;
#endif
#if defined (DMA1_Channel6)
                case hwDMA_Channel_Index_6:
                        HAL_NVIC_DisableIRQ(DMA1_Channel6_IRQn);
                        break;
#endif
#if defined (DMA1_Channel7)
                case hwDMA_Channel_Index_7:
                        HAL_NVIC_DisableIRQ(DMA1_Channel7_IRQn);
                        break;
#endif
#endif //DMA1_BASE
#endif
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

        g_dma[channel_index].Init.PeriphInc           = DMA_PINC_DISABLE;
        g_dma[channel_index].Init.MemInc              = DMA_MINC_ENABLE;
        g_dma[channel_index].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        g_dma[channel_index].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        g_dma[channel_index].Init.Mode                = DMA_NORMAL;
        g_dma[channel_index].Init.Priority            = DMA_PRIORITY_HIGH;

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[channel_index].Init.Direction = DMA_MEMORY_TO_PERIPH;
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        g_dma[channel_index].Init.Direction = DMA_PERIPH_TO_MEMORY;
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

        g_dma[channel_index].Init.PeriphInc           = DMA_PINC_DISABLE;
        g_dma[channel_index].Init.MemInc              = DMA_MINC_ENABLE;
        g_dma[channel_index].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        g_dma[channel_index].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        g_dma[channel_index].Init.Mode                = DMA_NORMAL;
        g_dma[channel_index].Init.Priority            = DMA_PRIORITY_HIGH;

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[channel_index].Init.Direction = DMA_MEMORY_TO_PERIPH;
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        g_dma[channel_index].Init.Direction = DMA_PERIPH_TO_MEMORY;
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

        g_dma[channel_index].Init.PeriphInc           = DMA_PINC_DISABLE;
        g_dma[channel_index].Init.MemInc              = DMA_MINC_ENABLE;
        g_dma[channel_index].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        g_dma[channel_index].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        g_dma[channel_index].Init.Mode                = DMA_NORMAL;
        g_dma[channel_index].Init.Priority            = DMA_PRIORITY_HIGH;

        switch(dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[channel_index].Init.Direction = DMA_MEMORY_TO_PERIPH;
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        g_dma[channel_index].Init.Direction = DMA_PERIPH_TO_MEMORY;
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

#endif //STM32L0
