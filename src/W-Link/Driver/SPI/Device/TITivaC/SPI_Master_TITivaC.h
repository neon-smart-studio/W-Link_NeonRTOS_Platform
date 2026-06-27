#ifndef SPI_RP2_H
#define SPI_RP2_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "SPI/SPI_Master.h"
#include "GPIO/GPIO.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern bool Spi_Master_Init_Status[];

uint32_t SPI_Map_Soc_Base(hwSPI_Index index);
uint32_t SPI_Map_Soc_Periph(hwSPI_Index index);
uint32_t SPI_Map_IRQ(hwSPI_Index index);
uint32_t SPI_Map_GPIO_Port(hwSPI_Index index);
uint32_t SPI_Map_GPIO_Periph(hwSPI_Index index);
uint32_t SPI_Map_GPIO_PinMask(hwSPI_Index index, bool cs);
uint32_t SPI_Map_PinConfig(hwSPI_Index index, hwGPIO_Pin pin);

void TITivaC_SPI_IRQ_Process(hwSPI_Index index);

void SPI_NVIC_Init(hwSPI_Index index);
void SPI_NVIC_DeInit(hwSPI_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // SPI_RP2_H