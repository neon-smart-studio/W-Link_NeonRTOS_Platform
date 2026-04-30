#ifndef SPI_STM32_H
#define SPI_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "SPI/SPI_Master.h"

#define SPI_IRQ_NVIC_PRIORITY      5
#define SPI_IRQ_NVIC_SUB_PRIORITY  0

#ifdef	__cplusplus
extern "C" {
#endif

extern SPI_HandleTypeDef g_spi[hwSPI_Index_MAX];
extern bool Spi_Master_Init_Status[];

SPI_TypeDef *SPI_Map_Soc_Base(hwSPI_Index index);
hwSPI_Index SPI_IndexFromHandle(SPI_HandleTypeDef *hspi);

hwSPI_OpResult SPI_Instance_Init(
    hwSPI_Index index,
    uint32_t clock_rate_hz,
    hwSPI_OpMode opMode
);

hwSPI_OpResult SPI_Instance_DeInit(hwSPI_Index index);

void SPI_NVIC_Init(hwSPI_Index index);
void SPI_NVIC_DeInit(hwSPI_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif