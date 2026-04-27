#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "SPI/SPI_Master.h"

#ifdef STM32H7

#include "SPI/SPI_Pin.h"

#include "GPIO/Device/GPIO_STM32.h"

#define SPI_MASTER_SYNC_TIMEOUT             100
#define SPI_MASTER_MUTEX_ACCESS_TIMEOUT     500
#define SPI_MASTER_OP_TIMEOUT               3000

#define SPI_IRQ_NVIC_PRIORITY 5
#define SPI_IRQ_NVIC_SUB_PRIORITY 0

#define SPI0_MASTER_TX_DMA_CHANNEL hwDMA_Index_0
#define SPI0_MASTER_RX_DMA_CHANNEL hwDMA_Index_1
#define SPI1_MASTER_TX_DMA_CHANNEL hwDMA_Index_2
#define SPI1_MASTER_RX_DMA_CHANNEL hwDMA_Index_3
#define SPI2_MASTER_TX_DMA_CHANNEL hwDMA_Index_4
#define SPI2_MASTER_RX_DMA_CHANNEL hwDMA_Index_5
#define SPI3_MASTER_TX_DMA_CHANNEL hwDMA_Index_6
#define SPI3_MASTER_RX_DMA_CHANNEL hwDMA_Index_7
#define SPI4_MASTER_TX_DMA_CHANNEL hwDMA_Index_8
#define SPI4_MASTER_RX_DMA_CHANNEL hwDMA_Index_9
#define SPI5_MASTER_TX_DMA_CHANNEL hwDMA_Index_16
#define SPI5_MASTER_RX_DMA_CHANNEL hwDMA_Index_17

NeonRTOS_LockObj_t Spi_Master_Access_Mutex[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Send_SyncHandle[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Recv_SyncHandle[hwSPI_Index_MAX];

#define SPI_MASTER_MUTEX_LOCK(a, b)  if (NeonRTOS_LockObjLock(&Spi_Master_Access_Mutex[a], b)!=NeonRTOS_OK) {return hwSPI_MutexTimeout;}
#define SPI_MASTER_MUTEX_UNLOCK(a)   NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[a]);

static bool Spi_Master_Init_Status[hwSPI_Index_MAX] = {false};
static bool Spi_Master_Use_CS[hwSPI_Index_MAX] = {false};

SPI_HandleTypeDef g_spi[hwSPI_Index_MAX];

static const uint32_t Spi_Master_Baudrate_Prescaler_Table[] =  {SPI_BAUDRATEPRESCALER_2,
                                                                SPI_BAUDRATEPRESCALER_4,
                                                                SPI_BAUDRATEPRESCALER_8,
                                                                SPI_BAUDRATEPRESCALER_16,
                                                                SPI_BAUDRATEPRESCALER_32,
                                                                SPI_BAUDRATEPRESCALER_64,
                                                                SPI_BAUDRATEPRESCALER_128,
                                                                SPI_BAUDRATEPRESCALER_256
                                                                };

SPI_TypeDef * SPI_Map_Soc_Base(hwSPI_Index index)
{
    switch(index)
    {
        case hwSPI_Index_0: return SPI1;
        case hwSPI_Index_1: return SPI2;
        case hwSPI_Index_2: return SPI3;
        case hwSPI_Index_3: return SPI4;
        case hwSPI_Index_4: return SPI5;
        case hwSPI_Index_5: return SPI6;
        default: break;
    }
    return NULL;
}

// UART IRQ 統一入口
static void SPI_HAL_IRQHandler(hwSPI_Index index)
{
    HAL_SPI_IRQHandler(&g_spi[index]);
}

// ====== 你的 IRQ handler（請確保對應的向量真的存在）======
void SPI1_IRQHandler(void){ SPI_HAL_IRQHandler(hwSPI_Index_0); }
void SPI2_IRQHandler(void){ SPI_HAL_IRQHandler(hwSPI_Index_1); }
void SPI3_IRQHandler(void){ SPI_HAL_IRQHandler(hwSPI_Index_2); }
void SPI4_IRQHandler(void){ SPI_HAL_IRQHandler(hwSPI_Index_3); }
void SPI5_IRQHandler(void){ SPI_HAL_IRQHandler(hwSPI_Index_4); }
void SPI6_IRQHandler(void){ SPI_HAL_IRQHandler(hwSPI_Index_5); }

static int SPI_Master_Get_Clock_Freq(hwSPI_Index index)
{
    int spi_hz = 0;

    /* Get source clock depending on SPI instance */
    switch (index) {
        case hwSPI_Index_0:
        case hwSPI_Index_1:
        case hwSPI_Index_2:
            spi_hz = LL_RCC_GetSPIClockFreq(LL_RCC_SPI123_CLKSOURCE);
            break;
        case hwSPI_Index_3:
        case hwSPI_Index_4:
            spi_hz = LL_RCC_GetSPIClockFreq(LL_RCC_SPI45_CLKSOURCE);
            break;
        case hwSPI_Index_5:
            spi_hz = LL_RCC_GetSPIClockFreq(LL_RCC_SPI6_CLKSOURCE);
            break;
        default:
            UART_Printf("CLK: SPI instance not set");
            break;
    }
    if (spi_hz == LL_RCC_PERIPH_FREQUENCY_NO) {
        UART_Printf("spi_hz not found\n");
    }
    return spi_hz;
}

hwSPI_OpResult SPI_Master_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode)
{
        if(opMode>=hwSPI_OpMode_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }

        if(Spi_Master_Init_Status[index]==true)
        {
                return hwSPI_OK;
        }

        GPIO_TypeDef * miso_soc_base = GPIO_Map_Soc_Base(SPI_Pin_Def_List[index].miso_pin);
        GPIO_TypeDef * mosi_soc_base = GPIO_Map_Soc_Base(SPI_Pin_Def_List[index].mosi_pin);
        GPIO_TypeDef * sclk_soc_base = GPIO_Map_Soc_Base(SPI_Pin_Def_List[index].sclk_pin);
        uint16_t miso_soc_pin = GPIO_Map_Soc_Pin(SPI_Pin_Def_List[index].miso_pin);
        uint16_t mosi_soc_pin = GPIO_Map_Soc_Pin(SPI_Pin_Def_List[index].mosi_pin);
        uint16_t sclk_soc_pin = GPIO_Map_Soc_Pin(SPI_Pin_Def_List[index].sclk_pin);

        GPIO_TypeDef * cs_soc_base = NULL;
        uint16_t cs_soc_pin = 0;

        if(miso_soc_pin==0 || miso_soc_base==NULL || mosi_soc_pin==0 || mosi_soc_base==NULL || sclk_soc_pin==0 || sclk_soc_base==NULL)
        {
                return hwGPIO_InvalidParameter;
        }

        if(SPI_Pin_Def_List[index].cs_pin!=hwGPIO_Pin_NC)
        {
                cs_soc_base = GPIO_Map_Soc_Base(SPI_Pin_Def_List[index].cs_pin);
                cs_soc_pin = GPIO_Map_Soc_Pin(SPI_Pin_Def_List[index].cs_pin); 
                
                if(cs_soc_pin==0 || cs_soc_base==NULL)
                {
                        return hwGPIO_InvalidParameter;
                }
        }

        SPI_TypeDef * spi_soc_base = SPI_Map_Soc_Base(index);
        if(spi_soc_base==NULL)
        {
                return hwGPIO_InvalidParameter;
        }

        if(NeonRTOS_SyncObjCreate(&Spi_Master_Send_SyncHandle[index])!=NeonRTOS_OK)
        {
                return hwSPI_MemoryError;
        }

        if(NeonRTOS_SyncObjCreate(&Spi_Master_Recv_SyncHandle[index])!=NeonRTOS_OK)
        {
                NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
                return hwSPI_MemoryError;
        }

	if (NeonRTOS_LockObjCreate(&Spi_Master_Access_Mutex[index])!=NeonRTOS_OK)
	{
                NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
                NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
		return hwSPI_MemoryError;
	}
	NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[index]);

        GPIO_Enable_RCC_Clock(miso_soc_base);
        GPIO_Enable_RCC_Clock(mosi_soc_base);
        GPIO_Enable_RCC_Clock(sclk_soc_base);
    
        if(SPI_Pin_Def_List[index].cs_pin!=hwGPIO_Pin_NC)
        {
                GPIO_Enable_RCC_Clock(cs_soc_base);
        }

        // SPI peripheral clock
        switch(index)
        {
                case hwSPI_Index_0: __HAL_RCC_SPI1_CLK_ENABLE(); break;
                case hwSPI_Index_1: __HAL_RCC_SPI2_CLK_ENABLE(); break;
                case hwSPI_Index_2: __HAL_RCC_SPI3_CLK_ENABLE(); break;
                case hwSPI_Index_3: __HAL_RCC_SPI4_CLK_ENABLE(); break;
                case hwSPI_Index_4: __HAL_RCC_SPI5_CLK_ENABLE(); break;
                case hwSPI_Index_5: __HAL_RCC_SPI6_CLK_ENABLE(); break;
                default: break;
        }

        GPIO_InitTypeDef g_spi_miso = {0};
        g_spi_miso.Pin       = miso_soc_pin;
        g_spi_miso.Mode      = GPIO_MODE_AF_PP;
        g_spi_miso.Pull      = GPIO_NOPULL;
        g_spi_miso.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        g_spi_miso.Alternate = SPI_Pin_Def_List[index].miso_af;
        HAL_GPIO_Init(miso_soc_base, &g_spi_miso);

        GPIO_InitTypeDef g_spi_mosi = {0};
        g_spi_mosi.Pin       = mosi_soc_pin;
        g_spi_mosi.Mode      = GPIO_MODE_AF_PP;
        g_spi_mosi.Pull      = GPIO_NOPULL;
        g_spi_mosi.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        g_spi_mosi.Alternate = SPI_Pin_Def_List[index].mosi_af;
        HAL_GPIO_Init(mosi_soc_base, &g_spi_mosi);

        GPIO_InitTypeDef g_spi_sclk = {0};
        g_spi_sclk.Pin       = sclk_soc_pin;
        g_spi_sclk.Mode      = GPIO_MODE_AF_PP;
        g_spi_sclk.Pull      = GPIO_NOPULL;
        g_spi_sclk.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        g_spi_sclk.Alternate = SPI_Pin_Def_List[index].sclk_af;
        HAL_GPIO_Init(sclk_soc_base, &g_spi_sclk);

        if(SPI_Pin_Def_List[index].cs_pin!=hwGPIO_Pin_NC)
        {
                GPIO_InitTypeDef g_spi_cs = {0};
                g_spi_cs.Pin       = cs_soc_pin;
                g_spi_cs.Mode      = GPIO_MODE_AF_PP;
                g_spi_cs.Pull      = GPIO_NOPULL;
                g_spi_cs.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
                g_spi_cs.Alternate = SPI_Pin_Def_List[index].cs_af;
                HAL_GPIO_Init(cs_soc_base, &g_spi_cs);
        }

        /* 填 SPI 參數 */
        g_spi[index].Instance  = spi_soc_base;
        g_spi[index].Init.Mode = SPI_MODE_MASTER;
        g_spi[index].Init.Direction     = SPI_DIRECTION_2LINES;
        g_spi[index].Init.DataSize      = SPI_DATASIZE_8BIT;
        switch(opMode)
        {
                case hwSPI_OpMode_Polarity0_Phase0:
                        g_spi[index].Init.CLKPolarity   = SPI_POLARITY_LOW;   /* CPOL=0 */
                        g_spi[index].Init.CLKPhase      = SPI_PHASE_1EDGE;     /* CPHA=0 */
                        break;
                case hwSPI_OpMode_Polarity0_Phase1:
                        g_spi[index].Init.CLKPolarity   = SPI_POLARITY_LOW;   /* CPOL=0 */
                        g_spi[index].Init.CLKPhase      = SPI_PHASE_2EDGE;     /* CPHA=1 */
                        break;
                case hwSPI_OpMode_Polarity1_Phase0:
                        g_spi[index].Init.CLKPolarity   = SPI_POLARITY_HIGH;   /* CPOL=1 */
                        g_spi[index].Init.CLKPhase      = SPI_PHASE_1EDGE;     /* CPHA=0 */
                        break;
                case hwSPI_OpMode_Polarity1_Phase1:
                        g_spi[index].Init.CLKPolarity   = SPI_POLARITY_HIGH;   /* CPOL=1 */
                        g_spi[index].Init.CLKPhase      = SPI_PHASE_2EDGE;     /* CPHA=1 */
                        break;
        }
        g_spi[index].Init.NSS           = SPI_NSS_SOFT;        /* 用軟 CS */
        g_spi[index].Init.FirstBit      = SPI_FIRSTBIT_MSB;
        g_spi[index].Init.TIMode        = SPI_TIMODE_DISABLE;
        g_spi[index].Init.CRCCalculation= SPI_CRCCALCULATION_DISABLE;

        int spi_hz = 0;
        uint8_t prescaler_rank = 0;
        uint8_t last_index = (sizeof(Spi_Master_Baudrate_Prescaler_Table) / sizeof(Spi_Master_Baudrate_Prescaler_Table[0])) - 1;

        /* Calculate the spi clock for prescaler_rank 0: SPI_BAUDRATEPRESCALER_2 */
        spi_hz = SPI_Master_Get_Clock_Freq(index) / 2;

        /* Define pre-scaler in order to get highest available frequency below requested frequency */
        while ((spi_hz > clock_rate_hz) && (prescaler_rank < last_index)) {
                spi_hz = spi_hz / 2;
                prescaler_rank++;
        }

        /*  Use the best fit pre-scaler */
        g_spi[index].Init.BaudRatePrescaler = Spi_Master_Baudrate_Prescaler_Table[prescaler_rank];

        HAL_SPI_Init(&g_spi[index]);
        
        switch(index)
        {
                case hwSPI_Index_0:
                        HAL_NVIC_SetPriority(SPI1_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(SPI1_IRQn);
                        break;
                case hwSPI_Index_1:
                        HAL_NVIC_SetPriority(SPI2_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(SPI2_IRQn);
                        break;
                case hwSPI_Index_2:
                        HAL_NVIC_SetPriority(SPI3_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(SPI3_IRQn);
                        break;
                case hwSPI_Index_3:
                        HAL_NVIC_SetPriority(SPI4_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(SPI4_IRQn);
                        break;
                case hwSPI_Index_4:
                        HAL_NVIC_SetPriority(SPI5_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(SPI5_IRQn);
                        break;
                case hwSPI_Index_5:
                        HAL_NVIC_SetPriority(SPI6_IRQn, SPI_IRQ_NVIC_PRIORITY, SPI_IRQ_NVIC_SUB_PRIORITY);
                        HAL_NVIC_EnableIRQ(SPI6_IRQn);
                        break;
        }

        hwDMA_Index tx_dma_ch = 0;
        hwDMA_Index rx_dma_ch = 0;

        switch(index)
        {
        case hwSPI_Index_0:
                tx_dma_ch = SPI0_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI0_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_1:
                tx_dma_ch = SPI1_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI1_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_2:
                tx_dma_ch = SPI2_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI2_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_3:
                tx_dma_ch = SPI3_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI3_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_4:
                tx_dma_ch = SPI4_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI4_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_5:
                tx_dma_ch = SPI5_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI5_MASTER_RX_DMA_CHANNEL;
                break;
        }
        
        DMA_Channel_Init(tx_dma_ch);
        DMA_Channel_Init(rx_dma_ch);

        DMA_Config_SPI(tx_dma_ch, hwDMA_Peripheral_Direction_TX, index);
        DMA_Config_SPI(rx_dma_ch, hwDMA_Peripheral_Direction_RX, index);

        gpio_pin_init_status[SPI_Pin_Def_List[index].miso_pin] = true;
        gpio_pin_init_status[SPI_Pin_Def_List[index].mosi_pin] = true;
        gpio_pin_init_status[SPI_Pin_Def_List[index].sclk_pin] = true;
        if(SPI_Pin_Def_List[index].cs_pin!=hwGPIO_Pin_NC)
        {
                gpio_pin_init_status[SPI_Pin_Def_List[index].cs_pin] = true;

                Spi_Master_Use_CS[index] = true;
        }

        Spi_Master_Init_Status[index] = true;

        return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_DeInit(hwSPI_Index index)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }

        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_OK;
        }

        GPIO_TypeDef * miso_soc_base = GPIO_Map_Soc_Base(SPI_Pin_Def_List[index].miso_pin);
        GPIO_TypeDef * mosi_soc_base = GPIO_Map_Soc_Base(SPI_Pin_Def_List[index].mosi_pin);
        GPIO_TypeDef * sclk_soc_base = GPIO_Map_Soc_Base(SPI_Pin_Def_List[index].sclk_pin);
        uint16_t miso_soc_pin = GPIO_Map_Soc_Pin(SPI_Pin_Def_List[index].miso_pin);
        uint16_t mosi_soc_pin = GPIO_Map_Soc_Pin(SPI_Pin_Def_List[index].mosi_pin);
        uint16_t sclk_soc_pin = GPIO_Map_Soc_Pin(SPI_Pin_Def_List[index].sclk_pin);

        GPIO_TypeDef * cs_soc_base = NULL;
        uint16_t cs_soc_pin = 0;

        if(miso_soc_pin==0 || miso_soc_base==NULL || mosi_soc_pin==0 || mosi_soc_base==NULL || sclk_soc_pin==0 || sclk_soc_base==NULL)
        {
                return hwGPIO_InvalidParameter;
        }

        if(Spi_Master_Use_CS[index]==true)
        {
                cs_soc_base = GPIO_Map_Soc_Base(SPI_Pin_Def_List[index].cs_pin);
                cs_soc_pin = GPIO_Map_Soc_Pin(SPI_Pin_Def_List[index].cs_pin); 
                
                if(cs_soc_pin==0 || cs_soc_base==NULL)
                {
                        return hwGPIO_InvalidParameter;
                }
        }

        hwDMA_Index tx_dma_ch = 0;
        hwDMA_Index rx_dma_ch = 0;

        switch(index)
        {
        case hwSPI_Index_0:
                tx_dma_ch = SPI0_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI0_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_1:
                tx_dma_ch = SPI1_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI1_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_2:
                tx_dma_ch = SPI2_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI2_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_3:
                tx_dma_ch = SPI3_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI3_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_4:
                tx_dma_ch = SPI4_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI4_MASTER_RX_DMA_CHANNEL;
                break;
        case hwSPI_Index_5:
                tx_dma_ch = SPI5_MASTER_TX_DMA_CHANNEL;
                rx_dma_ch = SPI5_MASTER_RX_DMA_CHANNEL;
                break;
        }
        
        DMA_Channel_DeInit(tx_dma_ch);
        DMA_Channel_DeInit(rx_dma_ch);

        switch(index)
        {
                case hwSPI_Index_0:
                        HAL_NVIC_DisableIRQ(SPI1_IRQn);
                        break;
                case hwSPI_Index_1:
                        HAL_NVIC_DisableIRQ(SPI2_IRQn);
                        break;
                case hwSPI_Index_2:
                        HAL_NVIC_DisableIRQ(SPI3_IRQn);
                        break;
                case hwSPI_Index_3:
                        HAL_NVIC_DisableIRQ(SPI4_IRQn);
                        break;
                case hwSPI_Index_4:
                        HAL_NVIC_DisableIRQ(SPI5_IRQn);
                        break;
                case hwSPI_Index_5:
                        HAL_NVIC_DisableIRQ(SPI6_IRQn);
                        break;
        }

        HAL_SPI_DeInit(&g_spi[index]);
        
        HAL_GPIO_DeInit(miso_soc_base, miso_soc_pin);
        HAL_GPIO_DeInit(mosi_soc_base, mosi_soc_pin);
        HAL_GPIO_DeInit(sclk_soc_base, sclk_soc_pin);

        if(Spi_Master_Use_CS[index]==true)
        {
                HAL_GPIO_DeInit(cs_soc_base, cs_soc_pin);
        }

        // SPI peripheral clock
        switch(index)
        {
                case hwSPI_Index_0: __HAL_RCC_SPI1_CLK_DISABLE(); break;
                case hwSPI_Index_1: __HAL_RCC_SPI2_CLK_DISABLE(); break;
                case hwSPI_Index_2: __HAL_RCC_SPI3_CLK_DISABLE(); break;
                case hwSPI_Index_3: __HAL_RCC_SPI4_CLK_DISABLE(); break;
                case hwSPI_Index_4: __HAL_RCC_SPI5_CLK_DISABLE(); break;
                case hwSPI_Index_5: __HAL_RCC_SPI6_CLK_DISABLE(); break;
                default: break;
        }

        NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);

        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);

        gpio_pin_init_status[SPI_Pin_Def_List[index].miso_pin] = false;
        gpio_pin_init_status[SPI_Pin_Def_List[index].mosi_pin] = false;
        gpio_pin_init_status[SPI_Pin_Def_List[index].sclk_pin] = false;
        if(Spi_Master_Use_CS[index]==true)
        {
                gpio_pin_init_status[SPI_Pin_Def_List[index].cs_pin] = false;

                Spi_Master_Use_CS[index] = false;
        }

        Spi_Master_Init_Status[index] = false;

        return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Frequency(hwSPI_Index index, uint32_t clock_rate_hz)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }

        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }

        SPI_TypeDef * spi_soc_base = SPI_Map_Soc_Base(index);
        if(spi_soc_base==NULL)
        {
                return hwGPIO_InvalidParameter;
        }

        SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

        int spi_hz = 0;
        uint8_t prescaler_rank = 0;
        uint8_t last_index = (sizeof(Spi_Master_Baudrate_Prescaler_Table) / sizeof(Spi_Master_Baudrate_Prescaler_Table[0])) - 1;

        /* Calculate the spi clock for prescaler_rank 0: SPI_BAUDRATEPRESCALER_2 */
        spi_hz = SPI_Master_Get_Clock_Freq(index) / 2;

        /* Define pre-scaler in order to get highest available frequency below requested frequency */
        while ((spi_hz > clock_rate_hz) && (prescaler_rank < last_index)) {
                spi_hz = spi_hz / 2;
                prescaler_rank++;
        }

        /*  Use the best fit pre-scaler */
        g_spi[index].Init.BaudRatePrescaler = Spi_Master_Baudrate_Prescaler_Table[prescaler_rank];

        HAL_SPI_Init(&g_spi[index]);
        
        __HAL_SPI_DISABLE(&g_spi[index]);                    // 先停
        MODIFY_REG(spi_soc_base->CFG1, SPI_CFG1_FTHLV, 0);   // FTHLV = 0 → 臨界值 1 byte
        __HAL_SPI_ENABLE(&g_spi[index]);                     // 再開

        SPI_MASTER_MUTEX_UNLOCK(index);
        
        return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Mode(hwSPI_Index index, hwSPI_OpMode opMode)
{
        if(opMode>=hwSPI_OpMode_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }

        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }

        SPI_TypeDef * spi_soc_base = SPI_Map_Soc_Base(index);
        if(spi_soc_base==NULL)
        {
                return hwGPIO_InvalidParameter;
        }

        SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

        switch(opMode)
        {
                case hwSPI_OpMode_Polarity0_Phase0:
                        g_spi[index].Init.CLKPolarity   = SPI_POLARITY_LOW;   /* CPOL=0 */
                        g_spi[index].Init.CLKPhase      = SPI_PHASE_1EDGE;     /* CPHA=0 */
                        break;
                case hwSPI_OpMode_Polarity0_Phase1:
                        g_spi[index].Init.CLKPolarity   = SPI_POLARITY_LOW;   /* CPOL=0 */
                        g_spi[index].Init.CLKPhase      = SPI_PHASE_2EDGE;     /* CPHA=1 */
                        break;
                case hwSPI_OpMode_Polarity1_Phase0:
                        g_spi[index].Init.CLKPolarity   = SPI_POLARITY_HIGH;   /* CPOL=1 */
                        g_spi[index].Init.CLKPhase      = SPI_PHASE_1EDGE;     /* CPHA=0 */
                        break;
                case hwSPI_OpMode_Polarity1_Phase1:
                        g_spi[index].Init.CLKPolarity   = SPI_POLARITY_HIGH;   /* CPOL=1 */
                        g_spi[index].Init.CLKPhase      = SPI_PHASE_2EDGE;     /* CPHA=1 */
                        break;
        }

        HAL_SPI_Init(&g_spi[index]);
        
        __HAL_SPI_DISABLE(&g_spi[index]);                    // 先停
        MODIFY_REG(spi_soc_base->CFG1, SPI_CFG1_FTHLV, 0);   // FTHLV = 0 → 臨界值 1 byte
        __HAL_SPI_ENABLE(&g_spi[index]);                     // 再開

        SPI_MASTER_MUTEX_UNLOCK(index);
        
        return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_WriteByte(hwSPI_Index index, uint8_t dat)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }

        SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

        NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
        
        HAL_SPI_Transmit_IT(&g_spi[index], &dat, 1);
        
        if(NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index], SPI_MASTER_SYNC_TIMEOUT)!=NeonRTOS_OK)
        {
                SPI_MASTER_MUTEX_UNLOCK(index);

                return hwSPI_SlaveTimeout;
        }

        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_ReadByte(hwSPI_Index index, uint8_t* dat)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(dat==NULL)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }
        
        SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

        NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);
        
        HAL_SPI_Receive_IT(&g_spi[index], dat, 1);
        
        if(NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index], SPI_MASTER_SYNC_TIMEOUT)!=NeonRTOS_OK)
        {
                SPI_MASTER_MUTEX_UNLOCK(index);
                
                return hwSPI_SlaveTimeout;
        }

        SPI_MASTER_MUTEX_UNLOCK(index);
        
        return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_TransferByte(hwSPI_Index index, uint8_t wr_dat, uint8_t* rd_dat)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(rd_dat==NULL)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }

        SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

        NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);
        
        HAL_SPI_TransmitReceive_IT(&g_spi[index], &wr_dat, rd_dat, 1);
        
        if(NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index], SPI_MASTER_SYNC_TIMEOUT)!=NeonRTOS_OK)
        {
                SPI_MASTER_MUTEX_UNLOCK(index);
                
                return hwSPI_SlaveTimeout;
        }

        if(NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index], SPI_MASTER_SYNC_TIMEOUT)!=NeonRTOS_OK)
        {
                SPI_MASTER_MUTEX_UNLOCK(index);
                
                return hwSPI_SlaveTimeout;
        }

        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_DummyByte(hwSPI_Index index)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }
        
        return SPI_Master_WriteByte(index, 0);
}

hwSPI_OpResult SPI_Master_BurstWrite(hwSPI_Index index, uint8_t* buf, uint32_t size)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }

        if(size==0)
        {
            if(buf==NULL)
            {
                  return hwSPI_OK;
            }
        }
        else{
            if(buf==NULL)
            {
                  return hwSPI_InvalidParameter;
            }
        }
        
        SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

        switch(index)
        {
        case hwSPI_Index_0:
                DMA_Transfer_SPI(SPI0_MASTER_TX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_1:
                DMA_Transfer_SPI(SPI1_MASTER_TX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_2:
                DMA_Transfer_SPI(SPI2_MASTER_TX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_3:
                DMA_Transfer_SPI(SPI3_MASTER_TX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_4:
                DMA_Transfer_SPI(SPI4_MASTER_TX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_5:
                DMA_Transfer_SPI(SPI5_MASTER_TX_DMA_CHANNEL, index, buf, size);
                break;
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_BurstRead(hwSPI_Index index, uint8_t* buf, uint32_t size)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }

        if(size==0)
        {
            if(buf==NULL)
            {
                  return hwSPI_OK;
            }
        }
        else{
            if(buf==NULL)
            {
                  return hwSPI_InvalidParameter;
            }
        }
        
        SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

        switch(index)
        {
        case hwSPI_Index_0:
                DMA_Transfer_SPI(SPI0_MASTER_RX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_1:
                DMA_Transfer_SPI(SPI1_MASTER_RX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_2:
                DMA_Transfer_SPI(SPI2_MASTER_RX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_3:
                DMA_Transfer_SPI(SPI3_MASTER_RX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_4:
                DMA_Transfer_SPI(SPI4_MASTER_RX_DMA_CHANNEL, index, buf, size);
                break;
        case hwSPI_Index_5:
                DMA_Transfer_SPI(SPI5_MASTER_RX_DMA_CHANNEL, index, buf, size);
                break;
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_OK;
}

#endif //STM32H7
