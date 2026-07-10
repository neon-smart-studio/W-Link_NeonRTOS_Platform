#ifndef SPI_TITIVAC_H
#define SPI_TITIVAC_H

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
uint32_t SPI_Map_PinConfig(hwSPI_Index index, hwGPIO_Pin pin);

void SPI_IRQ_Process(hwSPI_Index index);

void SPI_NVIC_Init(hwSPI_Index index);
void SPI_NVIC_DeInit(hwSPI_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // SPI_TITIVAC_H