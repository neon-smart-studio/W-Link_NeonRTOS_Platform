#ifndef SPI_STM32_H
#define SPI_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "SPI/SPI_Master.h"
#include "GPIO/GPIO.h"

#define SPI_IRQ_NVIC_PRIORITY      5
#define SPI_IRQ_NVIC_SUB_PRIORITY  0

#ifdef	__cplusplus
extern "C" {
#endif

extern bool Spi_Master_Init_Status[];

SPI_T *SPI_Map_Soc_Base(hwSPI_Index index);

void SPI_RxCpltCallback(hwSPI_Index index);
void SPI_TxCpltCallback(hwSPI_Index index);
void SPI_TxRxCpltCallback(hwSPI_Index index);

void SPI_GPIO_ConfigAF(hwSPI_Index index, bool cs);
void SPI_GPIO_DeConfigAF(hwSPI_Index index, bool cs);

hwSPI_OpResult SPI_Instance_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode, bool cs);
hwSPI_OpResult SPI_Instance_DeInit(hwSPI_Index index, bool cs);

hwSPI_OpResult SPI_Instance_ChangeFrequency(hwSPI_Index index, uint32_t clock_rate_hz);
hwSPI_OpResult SPI_Instance_ChangeMode(hwSPI_Index index, hwSPI_OpMode opMode);

hwSPI_OpResult SPI_Instance_Transmit_IT(hwSPI_Index index, const uint8_t *buf, uint16_t len);
hwSPI_OpResult SPI_Instance_Receive_IT(hwSPI_Index index, uint8_t *buf, uint16_t len);
hwSPI_OpResult SPI_Instance_TransmitReceive_IT(hwSPI_Index index, const uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len);

void SPI_NVIC_Init(hwSPI_Index index);
void SPI_NVIC_DeInit(hwSPI_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif