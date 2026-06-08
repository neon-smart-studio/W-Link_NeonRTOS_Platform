
#ifndef QSPI_PIN_STM32_DEF_H
#define QSPI_PIN_STM32_DEF_H

#include "GPIO/GPIO.h"

#include "QSPI/QSPI_Master.h"

typedef struct { 
    hwGPIO_Pin io0_pin;
    hwGPIO_Pin io1_pin;
    hwGPIO_Pin io2_pin;
    hwGPIO_Pin io3_pin;
    hwGPIO_Pin sclk_pin;
    hwGPIO_Pin cs_pin; // 可為 NC
} QSPI_Pin_Def;

typedef struct {
    hwQSPI_Index qspi;
    hwGPIO_Pin pin;
    uint32_t af;
} QSPI_AF_Map;

#endif //QSPI_PIN_STM32_DEF_H