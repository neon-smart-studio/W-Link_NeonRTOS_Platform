
#ifndef DMA_STM32_H
#define DMA_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "DMA/DMA.h"

#include "CAN/CAN.h"
#include "I2C/I2C_Master.h"
#include "SPI/SPI_Master.h"
#include "UART/UART.h"

#define DMA_MUTEX_TIMEOUT 1000

extern DMA_HandleTypeDef g_dma[hwDMA_Stream_Index_MAX];
extern bool DMA_Stream_Init_Status[hwDMA_Stream_Index_MAX];
extern NeonRTOS_LockObj_t DMA_Stream_Mutex[hwDMA_Stream_Index_MAX];

DMA_TypeDef * DMA_Map_Soc_Base(hwDMA_Stream_Index index);
DMA_Stream_TypeDef * DMA_Map_Soc_Stream_Base(hwDMA_Stream_Index index);

#if defined (BDMA_BASE)
BDMA_TypeDef * BDMA_Map_Soc_Base(hwDMA_Stream_Index index);
BDMA_Channel_TypeDef * BDMA_Map_Soc_Channel_Base(hwDMA_Stream_Index index);
#endif //BDMA_BASE

hwDMA_OpResult DMA_Instance_Init(hwDMA_Stream_Index stream_index);
hwDMA_OpResult DMA_Instance_DeInit(hwDMA_Stream_Index stream_index);

hwDMA_OpResult DMA_NVIC_Init(hwDMA_Stream_Index stream_index);
hwDMA_OpResult DMA_NVIC_DeInit(hwDMA_Stream_Index stream_index);

hwDMA_OpResult DMA_Config_UART(hwDMA_Stream_Index stream_index, hwDMA_Peripheral_Direction dir, hwUART_Index index);
hwDMA_OpResult DMA_Config_I2C(hwDMA_Stream_Index stream_index, hwDMA_Peripheral_Direction dir, hwI2C_Index index);
hwDMA_OpResult DMA_Config_SPI(hwDMA_Stream_Index stream_index, hwDMA_Peripheral_Direction dir, hwSPI_Index index);
hwDMA_OpResult DMA_DeConfig(hwDMA_Stream_Index stream_index);

hwDMA_OpResult DMA_Transfer_UART(hwDMA_Stream_Index stream_index, hwUART_Index index, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_Transfer_I2C(hwDMA_Stream_Index stream_index, hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_Transfer_SPI(hwDMA_Stream_Index stream_index, hwSPI_Index index, uint8_t* buf, size_t len);

#endif //DMA_STM32_H