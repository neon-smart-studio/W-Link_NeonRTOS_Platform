#ifndef SPI_STM32_H
#define SPI_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "SPI/SPI_Master.h"
#include "GPIO/GPIO.h"

#define SPI_IRQ_NVIC_PRIORITY      5
#define SPI_IRQ_NVIC_SUB_PRIORITY  0

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

#ifdef	__cplusplus
extern "C" {
#endif

extern SPI_HandleTypeDef g_spi[hwSPI_Index_MAX];
extern bool Spi_Master_Init_Status[];

SPI_TypeDef *SPI_Map_Soc_Base(hwSPI_Index index);
hwSPI_Index SPI_IndexFromHandle(SPI_HandleTypeDef *hspi);

hwSPI_OpResult SPI_Instance_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode);
hwSPI_OpResult SPI_Instance_DeInit(hwSPI_Index index);

void SPI_NVIC_Init(hwSPI_Index index);
void SPI_NVIC_DeInit(hwSPI_Index index);

#ifdef STM32F1
hwSPI_OpResult SPI_ApplyRemap(
    hwSPI_Index index,
    hwGPIO_Pin mosi_pin,
    hwGPIO_Pin miso_pin,
    hwGPIO_Pin sck_pin,
    hwGPIO_Pin nss_pin,
    bool use_hw_nss
);
void SPI_RestoreRemap(hwSPI_Index index);
#endif

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif