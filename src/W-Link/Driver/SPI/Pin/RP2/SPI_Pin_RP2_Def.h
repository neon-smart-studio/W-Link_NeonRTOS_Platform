
#ifndef SPI_PIN_RP2_DEF_H
#define SPI_PIN_RP2_DEF_H

#include "GPIO/GPIO.h"

#include "SPI/SPI_Master.h"

typedef struct { 
    hwGPIO_Pin mosi_pin;
    hwGPIO_Pin miso_pin;
    hwGPIO_Pin sclk_pin;
    hwGPIO_Pin cs_pin; // 可為 NC
} SPI_Pin_Def;

#endif //SPI_PIN_RP2_DEF_H