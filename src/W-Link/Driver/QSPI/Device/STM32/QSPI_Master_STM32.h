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

#if defined(QUADSPI)

extern QSPI_HandleTypeDef g_qspi[hwQSPI_Index_MAX];
extern bool Qspi_Master_Init_Status[];

QUADSPI_TypeDef *QSPI_Map_Soc_Base(hwQSPI_Index index);
hwQSPI_Index QSPI_IndexFromHandle(QSPI_HandleTypeDef *hqspi);

int QSPI_Master_Get_Clock_Freq(hwQSPI_Index index);

hwQSPI_OpResult QSPI_Instance_Init(hwQSPI_Index index, uint32_t clock_rate_hz, hwQSPI_OpMode opMode);
hwQSPI_OpResult QSPI_Instance_DeInit(hwQSPI_Index index);

void QSPI_NVIC_Init(hwQSPI_Index index);
void QSPI_NVIC_DeInit(hwQSPI_Index index);

#endif

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif