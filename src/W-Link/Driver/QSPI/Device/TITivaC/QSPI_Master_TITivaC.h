#ifndef SPI_STM32_H
#define SPI_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "QSPI/QSPI_Master.h"
#include "GPIO/GPIO.h"

#define SPI_IRQ_NVIC_PRIORITY      5
#define SPI_IRQ_NVIC_SUB_PRIORITY  0

#ifdef	__cplusplus
extern "C" {
#endif

extern bool Qspi_Master_Init_Status[];

uint32_t QSPI_Map_Soc_Base(hwQSPI_Index index);
uint32_t QSPI_Map_Soc_Periph(hwQSPI_Index index);
uint32_t QSPI_Map_IRQ(hwQSPI_Index index);
uint32_t QSPI_Map_PinConfig(hwQSPI_Index index, hwGPIO_Pin pin);

void QSPI_IRQ_Process(hwQSPI_Index index);

void QSPI_NVIC_Init(hwQSPI_Index index);
void QSPI_NVIC_DeInit(hwQSPI_Index index);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif