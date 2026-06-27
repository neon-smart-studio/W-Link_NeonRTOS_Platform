
#ifndef SPI_PIN_TITIVAC_DEF_H
#define SPI_PIN_TITIVAC_DEF_H

#include "GPIO/GPIO.h"

#include "SPI/SPI_Master.h"

typedef struct {
    hwGPIO_Pin io0_pin;   // XDAT0 / MISO
    hwGPIO_Pin io1_pin;   // XDAT1 / MOSI
    hwGPIO_Pin io2_pin;   // XDAT2
    hwGPIO_Pin io3_pin;   // XDAT3
    hwGPIO_Pin sclk_pin;
    hwGPIO_Pin cs_pin;
} SPI_Pin_Def;

#endif //SPI_PIN_TITIVAC_DEF_H