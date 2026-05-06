#ifndef SPI_STM32_H
#define SPI_STM32_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "SPI/SPI_Master.h"
#include "GPIO/GPIO.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern bool Spi_Master_Init_Status[];

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif