
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#ifdef STM32H7

#include "DMA_STM32_Index.h"

#include "DMA_STM32.h"

//#include "CAN/Device/STM32/CAN_STM32.h"
#include "I2C/Device/STM32/I2C_Master_STM32.h"
#include "SPI/Device/STM32/SPI_Master_STM32.h"
#include "UART/Device/STM32/UART_STM32.h"

#define DMA_STREAM_LOCK(stream_index) if (NeonRTOS_LockObjLock(&DMA_Stream_Mutex[(stream_index)], DMA_WAIT_ALLOCATED_TIMEOUT) != NeonRTOS_OK) { return hwDMA_MutexTimeout; }
#define DMA_STREAM_UNLOCK(stream_index) if (NeonRTOS_LockObjUnlock(&DMA_Stream_Mutex[(stream_index)]) != NeonRTOS_OK) { return hwDMA_MutexTimeout; }

static const hwDMA_Stream_Index UART_DMA_Channel_Map[hwUART_Index_MAX][hwDMA_Peripheral_Direction_MAX] = {
#if defined (UART1_BASE) || defined(USART1_BASE)
        {hwDMA_Stream_Index_0, hwDMA_Stream_Index_1},
#endif
#if defined (UART2_BASE) || defined(USART2_BASE)
        {hwDMA_Stream_Index_2, hwDMA_Stream_Index_3},
#endif
#if defined (UART3_BASE) || defined(USART3_BASE)
        {hwDMA_Stream_Index_4, hwDMA_Stream_Index_5},
#endif
#if defined (UART4_BASE) || defined(USART4_BASE)
        {hwDMA_Stream_Index_6, hwDMA_Stream_Index_7},
#endif
#if defined (UART5_BASE) || defined(USART5_BASE)
        {hwDMA_Stream_Index_8, hwDMA_Stream_Index_9},
#endif
#if defined (UART6_BASE) || defined(USART6_BASE)
        {hwDMA_Stream_Index_10, hwDMA_Stream_Index_11},
#endif
#if defined (UART7_BASE) || defined(USART7_BASE)
        {hwDMA_Stream_Index_12, hwDMA_Stream_Index_13},
#endif
#if defined (UART8_BASE) || defined(USART8_BASE)
        {hwDMA_Stream_Index_14, hwDMA_Stream_Index_15},
#endif
#if defined (UART9_BASE) || defined(USART9_BASE)
        {hwDMA_Stream_Index_0, hwDMA_Stream_Index_1},
#endif
#if defined (UART10_BASE) || defined(USART10_BASE)
        {hwDMA_Stream_Index_2, hwDMA_Stream_Index_3},
#endif
};

static const hwDMA_Stream_Index SPI_DMA_Channel_Map[hwSPI_Index_MAX][hwDMA_Peripheral_Direction_MAX] = {
#if defined (SPI1_BASE)
        {hwDMA_Stream_Index_0, hwDMA_Stream_Index_1},
#endif
#if defined (SPI2_BASE)
        {hwDMA_Stream_Index_2, hwDMA_Stream_Index_3},
#endif
#if defined (SPI3_BASE)
        {hwDMA_Stream_Index_4, hwDMA_Stream_Index_5},
#endif
#if defined (SPI4_BASE)
        {hwDMA_Stream_Index_6, hwDMA_Stream_Index_7},
#endif
#if defined (SPI5_BASE)
        {hwDMA_Stream_Index_8, hwDMA_Stream_Index_9},
#endif
#if defined (SPI6_BASE)
        {hwDMA_Stream_Index_16, hwDMA_Stream_Index_17},
#endif
};

static const hwDMA_Stream_Index I2C_DMA_Channel_Map[hwI2C_Index_MAX][hwDMA_Peripheral_Direction_MAX] = {
#if defined (I2C1_BASE)
        {hwDMA_Stream_Index_0, hwDMA_Stream_Index_1},
#endif
#if defined (I2C2_BASE)
        {hwDMA_Stream_Index_2, hwDMA_Stream_Index_3},
#endif
#if defined (I2C3_BASE)
        {hwDMA_Stream_Index_4, hwDMA_Stream_Index_5},
#endif
#if defined (I2C4_BASE)
        {hwDMA_Stream_Index_18, hwDMA_Stream_Index_19},
#endif
};

DMA_TypeDef * DMA_Map_Soc_Base(hwDMA_Stream_Index index)
{
    switch(index)
    {
#if defined (DMA1_BASE)
#if defined (DMA1_Stream0)
        case hwDMA_Stream_Index_0:
#endif
#if defined (DMA1_Stream1)
        case hwDMA_Stream_Index_1:
#endif
#if defined (DMA1_Stream2)
        case hwDMA_Stream_Index_2:
#endif
#if defined (DMA1_Stream3)
        case hwDMA_Stream_Index_3:
#endif
#if defined (DMA1_Stream4)
        case hwDMA_Stream_Index_4:
#endif
#if defined (DMA1_Stream5)
        case hwDMA_Stream_Index_5:
#endif
#if defined (DMA1_Stream6)
        case hwDMA_Stream_Index_6:
#endif
#if defined (DMA1_Stream7)
        case hwDMA_Stream_Index_7:
#endif
                return DMA1;
#endif
#if defined (DMA2_BASE)
#if defined (DMA2_Stream0)
        case hwDMA_Stream_Index_8:
#endif
#if defined (DMA2_Stream1)
        case hwDMA_Stream_Index_9:
#endif
#if defined (DMA2_Stream2)
        case hwDMA_Stream_Index_10:
#endif
#if defined (DMA2_Stream3)
        case hwDMA_Stream_Index_11:
#endif
#if defined (DMA2_Stream4)
        case hwDMA_Stream_Index_12:
#endif
#if defined (DMA2_Stream5)
        case hwDMA_Stream_Index_13:
#endif
#if defined (DMA2_Stream6)
        case hwDMA_Stream_Index_14:
#endif
#if defined (DMA2_Stream7)
        case hwDMA_Stream_Index_15:
#endif
                return DMA2;
#endif
        default:
                break;
    }
    return NULL;
}

#if defined (BDMA_BASE)
BDMA_TypeDef * BDMA_Map_Soc_Base(hwDMA_Stream_Index index)
{
    switch(index)
    {
#if defined (BDMA_Channel0)
        case hwDMA_Stream_Index_16:
#endif
#if defined (BDMA_Channel1)
        case hwDMA_Stream_Index_17:
#endif
#if defined (BDMA_Channel2)
        case hwDMA_Stream_Index_18:
#endif
#if defined (BDMA_Channel3)
        case hwDMA_Stream_Index_19:
#endif
#if defined (BDMA_Channel4)
        case hwDMA_Stream_Index_20:
#endif
#if defined (BDMA_Channel5)
        case hwDMA_Stream_Index_21:
#endif
#if defined (BDMA_Channel6)
        case hwDMA_Stream_Index_22:
#endif
#if defined (BDMA_Channel7)
        case hwDMA_Stream_Index_23:
#endif
                return BDMA;
        default:
                break;
    }
    return NULL;
}
#endif //BDMA_BASE

DMA_Stream_TypeDef * DMA_Map_Soc_Stream_Base(hwDMA_Stream_Index index)
{
    switch(index)
    {
#if defined (DMA1_BASE)
#if defined (DMA1_Stream0)
        case hwDMA_Stream_Index_0: return DMA1_Stream0;
#endif
#if defined (DMA1_Stream1)
        case hwDMA_Stream_Index_1: return DMA1_Stream1;
#endif
#if defined (DMA1_Stream2)
        case hwDMA_Stream_Index_2: return DMA1_Stream2;
#endif
#if defined (DMA1_Stream3)
        case hwDMA_Stream_Index_3: return DMA1_Stream3;
#endif
#if defined (DMA1_Stream4)
        case hwDMA_Stream_Index_4: return DMA1_Stream4;
#endif
#if defined (DMA1_Stream5)
        case hwDMA_Stream_Index_5: return DMA1_Stream5;
#endif
#if defined (DMA1_Stream6)
        case hwDMA_Stream_Index_6: return DMA1_Stream6;
#endif
#if defined (DMA1_Stream7)
        case hwDMA_Stream_Index_7: return DMA1_Stream7;
#endif
#endif
#if defined (DMA2_BASE)
#if defined (DMA2_Stream0)
        case hwDMA_Stream_Index_8: return DMA2_Stream0;
#endif
#if defined (DMA2_Stream1)
        case hwDMA_Stream_Index_9: return DMA2_Stream1;
#endif
#if defined (DMA2_Stream2)
        case hwDMA_Stream_Index_10: return DMA2_Stream2;
#endif
#if defined (DMA2_Stream3)
        case hwDMA_Stream_Index_11: return DMA2_Stream3;
#endif
#if defined (DMA2_Stream4)
        case hwDMA_Stream_Index_12: return DMA2_Stream4;
#endif
#if defined (DMA2_Stream5)
        case hwDMA_Stream_Index_13: return DMA2_Stream5;
#endif
#if defined (DMA2_Stream6)
        case hwDMA_Stream_Index_14: return DMA2_Stream6;
#endif
#if defined (DMA2_Stream7)
        case hwDMA_Stream_Index_15: return DMA2_Stream7;
#endif
#endif
        default: break;
    }
    return NULL;
}

#if defined (BDMA_BASE)
BDMA_Channel_TypeDef * BDMA_Map_Soc_Channel_Base(hwDMA_Stream_Index index)
{
    switch(index)
    {
#if defined (BDMA_Channel0)
        case hwDMA_Stream_Index_16: return BDMA_Channel0;
#endif
#if defined (BDMA_Channel1)
        case hwDMA_Stream_Index_17: return BDMA_Channel1;
#endif
#if defined (BDMA_Channel2)
        case hwDMA_Stream_Index_18: return BDMA_Channel2;
#endif
#if defined (BDMA_Channel3)
        case hwDMA_Stream_Index_19: return BDMA_Channel3;
#endif
#if defined (BDMA_Channel4)
        case hwDMA_Stream_Index_20: return BDMA_Channel4;
#endif
#if defined (BDMA_Channel5)
        case hwDMA_Stream_Index_21: return BDMA_Channel5;
#endif
#if defined (BDMA_Channel6)
        case hwDMA_Stream_Index_22: return BDMA_Channel6;
#endif
#if defined (BDMA_Channel7)
        case hwDMA_Stream_Index_23: return BDMA_Channel7;
#endif
        default: break;
    }
    return NULL;
}
#endif //BDMA_BASE

static void DMA_IRQ_Handler(hwDMA_Stream_Index index)
{
        if(index>=hwDMA_Stream_Index_MAX)
        {
                return;
        }

        HAL_DMA_IRQHandler(&g_dma[index]);
}

#if defined (BDMA_BASE)
static void BDMA_IRQ_Handler(hwDMA_Stream_Index index)
{
        if(index>=hwDMA_Stream_Index_MAX)
        {
                return;
        }

        HAL_DMA_IRQHandler(&g_dma[index]);
}
#endif //BDMA_BASE

#if defined (DMA1_BASE)
#if defined (DMA1_Stream0)
void DMA1_Stream0_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_0); }
#endif
#if defined (DMA1_Stream1)
void DMA1_Stream1_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_1); }
#endif
#if defined (DMA1_Stream2)
void DMA1_Stream2_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_2); }
#endif
#if defined (DMA1_Stream3)
void DMA1_Stream3_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_3); }
#endif
#if defined (DMA1_Stream4)
void DMA1_Stream4_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_4); }
#endif
#if defined (DMA1_Stream5)
void DMA1_Stream5_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_5); }
#endif
#if defined (DMA1_Stream6)
void DMA1_Stream6_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_6); }
#endif
#if defined (DMA1_Stream7)
void DMA1_Stream7_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_7); }
#endif
#endif //DMA1_BASE

#if defined (DMA2_BASE)
#if defined (DMA2_Stream0)
void DMA2_Stream0_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_8); }
#endif
#if defined (DMA2_Stream1)
void DMA2_Stream1_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_9); }
#endif
#if defined (DMA2_Stream2)
void DMA2_Stream2_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_10); }
#endif
#if defined (DMA2_Stream3)
void DMA2_Stream3_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_11); }
#endif
#if defined (DMA2_Stream4)
void DMA2_Stream4_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_12); }
#endif
#if defined (DMA2_Stream5)
void DMA2_Stream5_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_13); }
#endif
#if defined (DMA2_Stream6)
void DMA2_Stream6_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_14); }
#endif
#if defined (DMA2_Stream7)
void DMA2_Stream7_IRQHandler(void){ DMA_IRQ_Handler(hwDMA_Stream_Index_15); }
#endif
#endif //DMA2_BASE

#if defined (BDMA_BASE)
#if defined (BDMA_Channel0)
void BDMA_Channel0_IRQHandler(void){ BDMA_IRQ_Handler(hwDMA_Stream_Index_16); }
#endif
#if defined (BDMA_Channel1)
void BDMA_Channel1_IRQHandler(void){ BDMA_IRQ_Handler(hwDMA_Stream_Index_17); }
#endif
#if defined (BDMA_Channel2)
void BDMA_Channel2_IRQHandler(void){ BDMA_IRQ_Handler(hwDMA_Stream_Index_18); }
#endif
#if defined (BDMA_Channel3)
void BDMA_Channel3_IRQHandler(void){ BDMA_IRQ_Handler(hwDMA_Stream_Index_19); }
#endif
#if defined (BDMA_Channel4)
void BDMA_Channel4_IRQHandler(void){ BDMA_IRQ_Handler(hwDMA_Stream_Index_20); }
#endif
#if defined (BDMA_Channel5)
void BDMA_Channel5_IRQHandler(void){ BDMA_IRQ_Handler(hwDMA_Stream_Index_21); }
#endif
#if defined (BDMA_Channel6)
void BDMA_Channel6_IRQHandler(void){ BDMA_IRQ_Handler(hwDMA_Stream_Index_22); }
#endif
#if defined (BDMA_Channel7)
void BDMA_Channel7_IRQHandler(void){ BDMA_IRQ_Handler(hwDMA_Stream_Index_23); }
#endif
#endif //BDMA_BASE

void DMA_Clock_Enable()
{
#if defined (DMA1_BASE)
         __HAL_RCC_DMA1_CLK_ENABLE();
#endif //DMA1_BASE

#if defined (DMA2_BASE)
        __HAL_RCC_DMA2_CLK_ENABLE();
#endif //DMA2_BASE

#if defined (BDMA_BASE)
        __HAL_RCC_BDMA_CLK_ENABLE();
#endif //BDMA_BASE
}

void DMA_Clock_Disable()
{
#if defined (DMA1_BASE)
        __HAL_RCC_DMA1_CLK_DISABLE();
#endif //DMA1_BASE

#if defined (DMA2_BASE)
        __HAL_RCC_DMA2_CLK_DISABLE();
#endif //DMA2_BASE

#if defined (BDMA_BASE)
        __HAL_RCC_BDMA_CLK_DISABLE();
#endif //BDMA_BASE
}

hwDMA_OpResult DMA_NVIC_Init(hwDMA_Stream_Index stream_index)
{
        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Stream_Init_Status[stream_index]==true)
        {
                return hwDMA_OK;
        }

        switch(stream_index)
        {
#if defined (DMA1_BASE)
#if defined (DMA1_Stream0)
                case hwDMA_Stream_Index_0:
                        HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
                        break;
#endif
#if defined (DMA1_Stream1)
                case hwDMA_Stream_Index_1:
                        HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
                        break;
#endif
#if defined (DMA1_Stream2)
                case hwDMA_Stream_Index_2:
                        HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
                        break;
#endif
#if defined (DMA1_Stream3)
                case hwDMA_Stream_Index_3:
                        HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
                        break;
#endif
#if defined (DMA1_Stream4)
                case hwDMA_Stream_Index_4:
                        HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
                        break;
#endif
#if defined (DMA1_Stream5)
                case hwDMA_Stream_Index_5:
                        HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
                        break;
#endif
#if defined (DMA1_Stream6)
                case hwDMA_Stream_Index_6:
                        HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
                        break;
#endif
#if defined (DMA1_Stream7)
                case hwDMA_Stream_Index_7:
                        HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
                        break;
#endif
#endif //DMA1_BASE
#if defined (DMA2_BASE)
#if defined (DMA2_Stream0)
                case hwDMA_Stream_Index_8:
                        HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
                        break;
#endif
#if defined (DMA2_Stream1)
                case hwDMA_Stream_Index_9:
                        HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
                        break;
#endif
#if defined (DMA2_Stream2)
                case hwDMA_Stream_Index_10:
                        HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
                        break;
#endif
#if defined (DMA2_Stream3)
                case hwDMA_Stream_Index_11:
                        HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
                        break;
#endif
#if defined (DMA2_Stream4)
                case hwDMA_Stream_Index_12:
                        HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);
                        break;
#endif
#if defined (DMA2_Stream5)
                case hwDMA_Stream_Index_13:
                        HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
                        break;
#endif
#if defined (DMA2_Stream6)
                case hwDMA_Stream_Index_14:
                        HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
                        break;
#endif
#if defined (DMA2_Stream7)
                case hwDMA_Stream_Index_15:
                        HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
                        break;
#endif
#endif //DMA2_BASE
#if defined (BDMA_BASE)
#if defined (BDMA_Channel0)
                case hwDMA_Stream_Index_16:
                        HAL_NVIC_SetPriority(BDMA_Channel0_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(BDMA_Channel0_IRQn);
                        break;
#endif
#if defined (BDMA_Channel1)
                case hwDMA_Stream_Index_17:
                        HAL_NVIC_SetPriority(BDMA_Channel1_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(BDMA_Channel1_IRQn);
                        break;
#endif
#if defined (BDMA_Channel2)
                case hwDMA_Stream_Index_18:
                        HAL_NVIC_SetPriority(BDMA_Channel2_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(BDMA_Channel2_IRQn);
                        break;
#endif
#if defined (BDMA_Channel3)
                case hwDMA_Stream_Index_19:
                        HAL_NVIC_SetPriority(BDMA_Channel3_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(BDMA_Channel3_IRQn);
                        break;
#endif
#if defined (BDMA_Channel4)
                case hwDMA_Stream_Index_20:
                        HAL_NVIC_SetPriority(BDMA_Channel4_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(BDMA_Channel4_IRQn);
                        break;
#endif
#if defined (BDMA_Channel5)
                case hwDMA_Stream_Index_21:
                        HAL_NVIC_SetPriority(BDMA_Channel5_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(BDMA_Channel5_IRQn);
                        break;
#endif
#if defined (BDMA_Channel6)
                case hwDMA_Stream_Index_22:
                        HAL_NVIC_SetPriority(BDMA_Channel6_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(BDMA_Channel6_IRQn);
                        break;
#endif
#if defined (BDMA_Channel7)
                case hwDMA_Stream_Index_23:
                        HAL_NVIC_SetPriority(BDMA_Channel7_IRQn, DMA_IRQ_NVIC_PRIORITY, DMA_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(BDMA_Channel7_IRQn);
                        break;
#endif
#endif //BDMA_BASE
        }

        return hwDMA_OK;
}

hwDMA_OpResult DMA_NVIC_DeInit(hwDMA_Stream_Index stream_index)
{
        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        switch(stream_index)
        {
#if defined (DMA1_BASE)
#if defined (DMA1_Stream0)
                case hwDMA_Stream_Index_0:
                        HAL_NVIC_DisableIRQ(DMA1_Stream0_IRQn);
                        break;
#endif
#if defined (DMA1_Stream1)
                case hwDMA_Stream_Index_1:
                        HAL_NVIC_DisableIRQ(DMA1_Stream1_IRQn);
                        break;
#endif
#if defined (DMA1_Stream2)
                case hwDMA_Stream_Index_2:
                        HAL_NVIC_DisableIRQ(DMA1_Stream2_IRQn);
                        break;
#endif
#if defined (DMA1_Stream3)
                case hwDMA_Stream_Index_3:
                        HAL_NVIC_DisableIRQ(DMA1_Stream3_IRQn);
                        break;
#endif
#if defined (DMA1_Stream4)
                case hwDMA_Stream_Index_4:
                        HAL_NVIC_DisableIRQ(DMA1_Stream4_IRQn);
                        break;
#endif
#if defined (DMA1_Stream5)
                case hwDMA_Stream_Index_5:
                        HAL_NVIC_DisableIRQ(DMA1_Stream5_IRQn);
                        break;
#endif
#if defined (DMA1_Stream6)
                case hwDMA_Stream_Index_6:
                        HAL_NVIC_DisableIRQ(DMA1_Stream6_IRQn);
                        break;
#endif
#if defined (DMA1_Stream7)
                case hwDMA_Stream_Index_7:
                        HAL_NVIC_DisableIRQ(DMA1_Stream7_IRQn);
                        break;
#endif
#endif //DMA1_BASE
#if defined (DMA2_BASE)
#if defined (DMA2_Stream0)
                case hwDMA_Stream_Index_8:
                        HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
                        break;
#endif
#if defined (DMA2_Stream1)
                case hwDMA_Stream_Index_9:
                        HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);
                        break;
#endif
#if defined (DMA2_Stream2)
                case hwDMA_Stream_Index_10:
                        HAL_NVIC_DisableIRQ(DMA2_Stream2_IRQn);
                        break;
#endif
#if defined (DMA2_Stream3)
                case hwDMA_Stream_Index_11:
                        HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
                        break;
#endif
#if defined (DMA2_Stream4)
                case hwDMA_Stream_Index_12:
                        HAL_NVIC_DisableIRQ(DMA2_Stream4_IRQn);
                        break;
#endif
#if defined (DMA2_Stream5)
                case hwDMA_Stream_Index_13:
                        HAL_NVIC_DisableIRQ(DMA2_Stream5_IRQn);
                        break;
#endif
#if defined (DMA2_Stream6)
                case hwDMA_Stream_Index_14:
                        HAL_NVIC_DisableIRQ(DMA2_Stream6_IRQn);
                        break;
#endif
#if defined (DMA2_Stream7)
                case hwDMA_Stream_Index_15:
                        HAL_NVIC_DisableIRQ(DMA2_Stream7_IRQn);
                        break;
#endif
#endif //DMA2_BASE
#if defined (BDMA_BASE)
#if defined (BDMA_Channel0)
                case hwDMA_Stream_Index_16:
                        HAL_NVIC_DisableIRQ(BDMA_Channel0_IRQn);
                        break;
#endif
#if defined (BDMA_Channel1)
                case hwDMA_Stream_Index_17:
                        HAL_NVIC_DisableIRQ(BDMA_Channel1_IRQn);
                        break;
#endif
#if defined (BDMA_Channel2)
                case hwDMA_Stream_Index_18:
                        HAL_NVIC_DisableIRQ(BDMA_Channel2_IRQn);
                        break;
#endif
#if defined (BDMA_Channel3)
                case hwDMA_Stream_Index_19:
                        HAL_NVIC_DisableIRQ(BDMA_Channel3_IRQn);
                        break;
#endif
#if defined (BDMA_Channel4)
                case hwDMA_Stream_Index_20:
                        HAL_NVIC_DisableIRQ(BDMA_Channel4_IRQn);
                        break;
#endif
#if defined (BDMA_Channel5)
                case hwDMA_Stream_Index_21:
                        HAL_NVIC_DisableIRQ(BDMA_Channel5_IRQn);
                        break;
#endif
#if defined (BDMA_Channel6)
                case hwDMA_Stream_Index_22:
                        HAL_NVIC_DisableIRQ(BDMA_Channel6_IRQn);
                        break;
#endif
#if defined (BDMA_Channel7)
                case hwDMA_Stream_Index_23:
                        HAL_NVIC_DisableIRQ(BDMA_Channel7_IRQn);
                        break;
#endif
#endif //BDMA_BASE
        }

        return hwDMA_OK;
}

static void* DMA_Get_Instance(hwDMA_Stream_Index stream_index)
{
    DMA_Stream_TypeDef *dma_soc_stream_base = NULL;
    BDMA_Channel_TypeDef *bdma_soc_channel_base = NULL;

    if (stream_index >= hwDMA_Stream_Index_MAX)
    {
        return NULL;
    }

    if (stream_index < hwDMA_Stream_Index_16)
    {
        dma_soc_stream_base = DMA_Map_Soc_Stream_Base(stream_index);
        if (dma_soc_stream_base == NULL)
        {
            return NULL;
        }

        return dma_soc_stream_base;
    }
    else if (stream_index >= hwDMA_Stream_Index_16 && stream_index < hwDMA_Stream_Index_MAX)
    {
        bdma_soc_channel_base = BDMA_Map_Soc_Channel_Base(stream_index);
        if (bdma_soc_channel_base == NULL)
        {
            return NULL;
        }

        return bdma_soc_channel_base;
    }
    else
    {
        return NULL;
    }
}

static hwDMA_OpResult DMA_DeConfig(hwDMA_Stream_Index stream_index)
{
    if (stream_index >= hwDMA_Stream_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    if (DMA_Stream_Init_Status[stream_index] == false)
    {
        return hwDMA_NotInit;
    }

    if (g_dma[stream_index].Instance != NULL)
    {
        HAL_DMA_Abort(&g_dma[stream_index]);

        if (HAL_DMA_DeInit(&g_dma[stream_index]) != HAL_OK)
        {
            return hwDMA_HwError;
        }
    }

    memset(&g_dma[stream_index], 0, sizeof(DMA_HandleTypeDef));

    return hwDMA_OK;
}

static hwDMA_OpResult DMA_Config_UART(hwDMA_Stream_Index stream_index, hwDMA_Peripheral_Direction dir, hwUART_Index index)
{
        if(dir>=hwDMA_Peripheral_Direction_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwUART_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if (DMA_Stream_Init_Status[stream_index] == false)
        {
                return hwDMA_NotInit;
        }

        g_dma[stream_index].Instance = DMA_Get_Instance(stream_index);
        if (g_dma[stream_index].Instance == NULL)
        {
                return hwDMA_InvalidParameter;
        }

        g_dma[stream_index].Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        g_dma[stream_index].Init.MemBurst            = DMA_MBURST_SINGLE;
        g_dma[stream_index].Init.PeriphBurst         = DMA_PBURST_SINGLE;
        g_dma[stream_index].Init.PeriphInc           = DMA_PINC_DISABLE;
        g_dma[stream_index].Init.MemInc              = DMA_MINC_ENABLE;
        g_dma[stream_index].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        g_dma[stream_index].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        g_dma[stream_index].Init.Mode                = DMA_NORMAL;
        g_dma[stream_index].Init.Priority            = DMA_PRIORITY_HIGH;

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[stream_index].Init.Direction = DMA_MEMORY_TO_PERIPH;

                        switch (index)
                        {
#if defined (UART1_BASE)
                                case hwUART_Index_0:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART1_TX;
                                        break;
#endif
#if defined(USART1_BASE)
                                case hwUART_Index_0:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART1_TX;
                                        break;
#endif
#if defined (UART2_BASE)
                                case hwUART_Index_1:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART2_TX;
                                        break;
#endif
#if defined(USART2_BASE)
                                case hwUART_Index_1:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART2_TX;
                                        break;
#endif
#if defined (UART3_BASE)
                                case hwUART_Index_2:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART3_TX;
                                        break;
#endif
#if defined(USART3_BASE)
                                case hwUART_Index_2:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART3_TX;
                                        break;
#endif
#if defined (UART4_BASE)
                                case hwUART_Index_3:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART4_TX;
                                        break;
#endif
#if defined(USART4_BASE)
                                case hwUART_Index_3:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART4_TX;
                                        break;
#endif
#if defined (UART5_BASE)
                                case hwUART_Index_4:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART5_TX;
                                        break;
#endif
#if defined(USART5_BASE)
                                case hwUART_Index_4:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART5_TX;
                                        break;
#endif
#if defined (UART6_BASE)
                                case hwUART_Index_5:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART6_TX;
                                        break;
#endif
#if defined(USART6_BASE)
                                case hwUART_Index_5:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART6_TX;
                                        break;
#endif
#if defined (UART7_BASE)
                                case hwUART_Index_6:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART7_TX;
                                        break;
#endif
#if defined(USART7_BASE)
                                case hwUART_Index_6:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART7_TX;
                                        break;
#endif
#if defined (UART8_BASE)
                                case hwUART_Index_7:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART8_TX;
                                        break;
#endif
#if defined(USART8_BASE)
                                case hwUART_Index_7:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART8_TX;
                                        break;
#endif
#if defined (UART9_BASE)
                                case hwUART_Index_8:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART9_TX;
                                        break;
#endif
#if defined(USART9_BASE)
                                case hwUART_Index_8:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART9_TX;
                                        break;
#endif
#if defined (UART10_BASE)
                                case hwUART_Index_9:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART10_TX;
                                        break;
#endif
#if defined(USART10_BASE)
                                case hwUART_Index_9:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART10_TX;
                                        break;
#endif
#if defined(LPUART1_BASE)
                                case hwUART_Index_L1:
                                        g_dma[stream_index].Init.Request = BDMA_REQUEST_LPUART1_TX;
                                        break;
#endif
                        }
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        g_dma[stream_index].Init.Direction = DMA_PERIPH_TO_MEMORY;

                        switch (index)
                        {
#if defined (UART1_BASE)
                                case hwUART_Index_0:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART1_RX;
                                        break;
#endif
#if defined(USART1_BASE)
                                case hwUART_Index_0:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART1_RX;
                                        break;
#endif
#if defined (UART2_BASE)
                                case hwUART_Index_1:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART2_RX;
                                        break;
#endif
#if defined(USART2_BASE)
                                case hwUART_Index_1:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART2_RX;
                                        break;
#endif
#if defined (UART3_BASE)
                                case hwUART_Index_2:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART3_RX;
                                        break;
#endif
#if defined(USART3_BASE)
                                case hwUART_Index_2:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART3_RX;
                                        break;
#endif
#if defined (UART4_BASE)
                                case hwUART_Index_3:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART4_RX;
                                        break;
#endif
#if defined(USART4_BASE)
                                case hwUART_Index_3:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART4_RX;
                                        break;
#endif
#if defined (UART5_BASE)
                                case hwUART_Index_4:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART5_RX;
                                        break;
#endif
#if defined(USART5_BASE)
                                case hwUART_Index_4:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART5_RX;
                                        break;
#endif
#if defined (UART6_BASE)
                                case hwUART_Index_5:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART6_RX;
                                        break;
#endif
#if defined(USART6_BASE)
                                case hwUART_Index_5:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART6_RX;
                                        break;
#endif
#if defined (UART7_BASE)
                                case hwUART_Index_6:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART7_RX;
                                        break;
#endif
#if defined(USART7_BASE)
                                case hwUART_Index_6:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART7_RX;
                                        break;
#endif
#if defined (UART8_BASE)
                                case hwUART_Index_7:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART8_RX;
                                        break;
#endif
#if defined(USART8_BASE)
                                case hwUART_Index_7:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART8_RX;
                                        break;
#endif
#if defined (UART9_BASE)
                                case hwUART_Index_8:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART9_RX;
                                        break;
#endif
#if defined(USART9_BASE)
                                case hwUART_Index_8:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART9_RX;
                                        break;
#endif
#if defined (UART10_BASE)
                                case hwUART_Index_9:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_UART10_RX;
                                        break;
#endif
#if defined(USART10_BASE)
                                case hwUART_Index_9:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_USART10_RX;
                                        break;
#endif
#if defined(LPUART1_BASE)
                                case hwUART_Index_L1:
                                        g_dma[stream_index].Init.Request = BDMA_REQUEST_LPUART1_RX;
                                        break;
#endif
                        }
                        break;
        }

        if (HAL_DMA_Init(&g_dma[stream_index]) != HAL_OK)
        {
                return hwDMA_HwError;
        }

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        __HAL_LINKDMA(&g_uart[index], hdmatx, g_dma[stream_index]);
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        __HAL_LINKDMA(&g_uart[index], hdmarx, g_dma[stream_index]);
                        break;
        }

        return hwDMA_OK;
}

static hwDMA_OpResult DMA_Config_I2C(hwDMA_Stream_Index stream_index, hwDMA_Peripheral_Direction dir, hwI2C_Index index)
{
        if(dir>=hwDMA_Peripheral_Direction_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwI2C_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if (DMA_Stream_Init_Status[stream_index] == false)
        {
                return hwDMA_NotInit;
        }

        g_dma[stream_index].Instance = DMA_Get_Instance(stream_index);
        if(g_dma[stream_index].Instance == NULL)
        {
                return hwDMA_InvalidParameter;
        }

        g_dma[stream_index].Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        g_dma[stream_index].Init.MemBurst            = DMA_MBURST_SINGLE;
        g_dma[stream_index].Init.PeriphBurst         = DMA_PBURST_SINGLE;
        g_dma[stream_index].Init.PeriphInc           = DMA_PINC_DISABLE;
        g_dma[stream_index].Init.MemInc              = DMA_MINC_ENABLE;
        g_dma[stream_index].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        g_dma[stream_index].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        g_dma[stream_index].Init.Mode                = DMA_NORMAL;
        g_dma[stream_index].Init.Priority            = DMA_PRIORITY_HIGH;

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[stream_index].Init.Direction = DMA_MEMORY_TO_PERIPH;

                        switch (index)
                        {
#if defined (I2C1_BASE)
                                case hwI2C_Index_0:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_I2C1_TX;
                                        break;
#endif
#if defined (I2C2_BASE)
                                case hwI2C_Index_1:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_I2C2_TX;
                                        break;
#endif
#if defined (I2C3_BASE)
                                case hwI2C_Index_2:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_I2C3_TX;
                                        break;
#endif
#if defined (I2C4_BASE)
                                case hwI2C_Index_3:
                                        g_dma[stream_index].Init.Request = BDMA_REQUEST_I2C4_TX;
                                        break;
#endif
                        }
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        g_dma[stream_index].Init.Direction = DMA_PERIPH_TO_MEMORY;

                        switch (index)
                        {
#if defined (I2C1_BASE)
                                case hwI2C_Index_0:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_I2C1_RX;
                                        break;
#endif
#if defined (I2C2_BASE)
                                case hwI2C_Index_1:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_I2C2_RX;
                                        break;
#endif
#if defined (I2C3_BASE)
                                case hwI2C_Index_2:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_I2C3_RX;
                                        break;
#endif
#if defined (I2C4_BASE)
                                case hwI2C_Index_3:
                                        g_dma[stream_index].Init.Request = BDMA_REQUEST_I2C4_RX;
                                        break;
#endif
                        }
                        break;
        }

        if (HAL_DMA_Init(&g_dma[stream_index]) != HAL_OK)
        {
                return hwDMA_HwError;
        }

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                __HAL_LINKDMA(&g_i2c[index], hdmatx, g_dma[stream_index]);
                break;

                case hwDMA_Peripheral_Direction_RX:
                __HAL_LINKDMA(&g_i2c[index], hdmarx, g_dma[stream_index]);
                break;

                default:
                return hwDMA_InvalidParameter;
        }

        return hwDMA_OK;
}

static hwDMA_OpResult DMA_Config_SPI(hwDMA_Stream_Index stream_index, hwDMA_Peripheral_Direction dir, hwSPI_Index index)
{
        if(dir>=hwDMA_Peripheral_Direction_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwSPI_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Stream_Init_Status[stream_index]==false)
        {
                return hwDMA_NotInit;
        }

        g_dma[stream_index].Instance = DMA_Get_Instance(stream_index);
        if(g_dma[stream_index].Instance == NULL)
        {
                return hwDMA_InvalidParameter;
        }

        g_dma[stream_index].Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
        g_dma[stream_index].Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
        g_dma[stream_index].Init.MemBurst            = DMA_MBURST_SINGLE;
        g_dma[stream_index].Init.PeriphBurst         = DMA_PBURST_SINGLE;
        g_dma[stream_index].Init.PeriphInc           = DMA_PINC_DISABLE;
        g_dma[stream_index].Init.MemInc              = DMA_MINC_ENABLE;
        g_dma[stream_index].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        g_dma[stream_index].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        g_dma[stream_index].Init.Mode                = DMA_NORMAL;
        g_dma[stream_index].Init.Priority            = DMA_PRIORITY_HIGH;

        switch(dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[stream_index].Init.Direction = DMA_MEMORY_TO_PERIPH;
                        switch(index)
                        {
#if defined (SPI1_BASE)
                                case hwSPI_Index_0:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI1_TX;
                                        break;
#endif
#if defined (SPI2_BASE)
                                case hwSPI_Index_1:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI2_TX;
                                        break;
#endif
#if defined (SPI3_BASE)
                                case hwSPI_Index_2:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI3_TX;
                                        break;
#endif
#if defined (SPI4_BASE)
                                case hwSPI_Index_3:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI4_TX;
                                        break;
#endif
#if defined (SPI5_BASE)
                                case hwSPI_Index_4:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI5_TX;
                                        break;
#endif
#if defined (SPI6_BASE)
                                case hwSPI_Index_5:
                                        g_dma[stream_index].Init.Request = BDMA_REQUEST_SPI6_TX;
                                        break;
#endif
                        }
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        g_dma[stream_index].Init.Direction = DMA_PERIPH_TO_MEMORY;
                        switch(index)
                        {
#if defined (SPI1_BASE)
                                case hwSPI_Index_0:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI1_RX;
                                        break;
#endif
#if defined (SPI2_BASE)
                                case hwSPI_Index_1:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI2_RX;
                                        break;
#endif
#if defined (SPI3_BASE)
                                case hwSPI_Index_2:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI3_RX;
                                        break;
#endif
#if defined (SPI4_BASE)
                                case hwSPI_Index_3:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI4_RX;
                                        break;
#endif
#if defined (SPI5_BASE)
                                case hwSPI_Index_4:
                                        g_dma[stream_index].Init.Request = DMA_REQUEST_SPI5_RX;
                                        break;
#endif
#if defined (SPI6_BASE)
                                case hwSPI_Index_5:
                                        g_dma[stream_index].Init.Request = BDMA_REQUEST_SPI6_RX;
                                        break;
#endif
                        }
                        break;
        }

        if (HAL_DMA_Init(&g_dma[stream_index]) != HAL_OK)
        {
                return hwDMA_HwError;
        }

        switch(dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        __HAL_LINKDMA(&g_spi[index], hdmatx, g_dma[stream_index]);
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        __HAL_LINKDMA(&g_spi[index], hdmarx, g_dma[stream_index]);
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

        hwDMA_Stream_Index stream_index = UART_DMA_Channel_Map[index][dir];
        
        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Stream_Init_Status[stream_index]==false)
        {
                return hwDMA_NotInit;
        }

        hwDMA_OpResult op_status;

        DMA_STREAM_LOCK(stream_index);

        op_status = DMA_Config_UART(stream_index, dir, index);
        if(op_status < hwDMA_OK)
        {
                DMA_STREAM_UNLOCK(stream_index);
                return op_status;
        }

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        SCB_CleanDCache_by_Addr((uint32_t *)buf, len);

                        if (HAL_UART_Transmit_DMA(&g_uart[index], buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(stream_index);
                                DMA_STREAM_UNLOCK(stream_index);
                                return hwDMA_HwError;
                        }
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        SCB_InvalidateDCache_by_Addr((uint32_t *)buf, len);

                        if (HAL_UART_Receive_DMA(&g_uart[index], buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(stream_index);
                                DMA_STREAM_UNLOCK(stream_index);
                                return hwDMA_HwError;
                        }
                        break;
        }

        while (HAL_UART_GetState(&g_uart[index]) != HAL_UART_STATE_READY) { }

        HAL_UART_DMAStop(&g_uart[index]);

        DMA_DeConfig(stream_index);
        
        DMA_STREAM_UNLOCK(stream_index);

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

        hwDMA_Stream_Index stream_index = I2C_DMA_Channel_Map[index][dir];
        
        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Stream_Init_Status[stream_index]==false)
        {
                return hwDMA_NotInit;
        }

        hwDMA_OpResult op_status;

        DMA_STREAM_LOCK(stream_index);

        op_status = DMA_Config_I2C(stream_index, dir, index);
        if(op_status < hwDMA_OK)
        {
                DMA_STREAM_UNLOCK(stream_index);
                return op_status;
        }

        switch (dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        SCB_CleanDCache_by_Addr((uint32_t *)buf, len);

                        if (HAL_I2C_Master_Transmit_DMA(&g_i2c[index], dev_addr, buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(stream_index);
                                DMA_STREAM_UNLOCK(stream_index);
                                return hwDMA_HwError;
                        }
                        break;

                case hwDMA_Peripheral_Direction_RX:
                        SCB_InvalidateDCache_by_Addr((uint32_t *)buf, len);

                        if (HAL_I2C_Master_Receive_DMA(&g_i2c[index], dev_addr, buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(stream_index);
                                DMA_STREAM_UNLOCK(stream_index);
                                return hwDMA_HwError;
                        }
                        break;
        }

        while (HAL_I2C_GetState(&g_i2c[index]) != HAL_I2C_STATE_READY) { }

        HAL_I2C_DMAStop(&g_i2c[index]);

        DMA_DeConfig(stream_index);

        DMA_STREAM_UNLOCK(stream_index);

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

        hwDMA_Stream_Index stream_index = SPI_DMA_Channel_Map[index][dir];
        
        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Stream_Init_Status[stream_index]==false)
        {
                return hwDMA_NotInit;
        }

        hwDMA_OpResult op_status;

        DMA_STREAM_LOCK(stream_index);

        op_status = DMA_Config_SPI(stream_index, dir, index);
        if(op_status < hwDMA_OK)
        {
                DMA_STREAM_UNLOCK(stream_index);
                return op_status;
        }

        switch(dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        SCB_CleanDCache_by_Addr((uint32_t *)buf, len);
                        
                        if (HAL_SPI_Transmit_DMA(&g_spi[index], buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(stream_index);
                                DMA_STREAM_UNLOCK(stream_index);
                                return hwDMA_HwError;
                        }
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        SCB_InvalidateDCache_by_Addr((uint32_t *)buf, len);

                        if (HAL_SPI_Receive_DMA(&g_spi[index], buf, len) != HAL_OK)
                        {
                                DMA_DeConfig(stream_index);
                                DMA_STREAM_UNLOCK(stream_index);
                                return hwDMA_HwError;
                        }
                        break;
        }

        while (HAL_SPI_GetState(&g_spi[index]) != HAL_SPI_STATE_READY) { }
        
        HAL_SPI_DMAStop(&g_spi[index]);

        DMA_DeConfig(stream_index);

        DMA_STREAM_UNLOCK(stream_index);

        return op_status;
}

#endif //STM32H7
