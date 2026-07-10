#ifndef QSPI_TIMSP432_H
#define QSPI_TIMSP432_H

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

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // QSPI_TIMSP432_H