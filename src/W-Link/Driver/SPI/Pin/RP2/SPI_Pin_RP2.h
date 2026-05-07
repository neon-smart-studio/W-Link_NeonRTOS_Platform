#ifndef SPI_PIN_RP2_H
#define SPI_PIN_RP2_H

#include "SPI_Pin_RP2_Def.h"

#ifdef DEVICE_RP2

typedef enum {
    SPI_Pinset_DEFAULT = 0,
    SPI_Pinset_ALT1,
    SPI_Pinset_ALT2,
    SPI_Pinset_MAX
} SPI_Pinset_t;

#ifndef CONFIG_SPI0_PINSET
#define CONFIG_SPI0_PINSET SPI_Pinset_DEFAULT
#endif

#ifndef CONFIG_SPI1_PINSET
#define CONFIG_SPI1_PINSET SPI_Pinset_DEFAULT
#endif

static const SPI_Pinset_t SPI_Index_Map_Alt[hwSPI_Index_MAX] = {
    CONFIG_SPI0_PINSET,
    CONFIG_SPI1_PINSET,
};

static const SPI_Pin_Def SPI_Pin_Def_Table[hwSPI_Index_MAX][SPI_Pinset_MAX] =
{
    /*
     * SPI0
     */
    {
        /*
         * DEFAULT
         * SCK  = GPIO18
         * TX   = GPIO19
         * RX   = GPIO16
         * CS   = GPIO17
         */
        {
            hwGPIO_Pin_19,   // MOSI/TX
            hwGPIO_Pin_16,   // MISO/RX
            hwGPIO_Pin_18,   // SCK
            hwGPIO_Pin_17    // NSS/CS
        },

        /*
         * ALT1
         * SCK  = GPIO2
         * TX   = GPIO3
         * RX   = GPIO4
         * CS   = GPIO5
         */
        {
            hwGPIO_Pin_3,
            hwGPIO_Pin_4,
            hwGPIO_Pin_2,
            hwGPIO_Pin_5
        },

        /*
         * ALT2
         * SCK  = GPIO6
         * TX   = GPIO7
         * RX   = GPIO4
         * CS   = GPIO5
         */
        {
            hwGPIO_Pin_7,
            hwGPIO_Pin_4,
            hwGPIO_Pin_6,
            hwGPIO_Pin_5
        },
    },

    /*
     * SPI1
     */
    {
        /*
         * DEFAULT
         * SCK  = GPIO10
         * TX   = GPIO11
         * RX   = GPIO12
         * CS   = GPIO13
         */
        {
            hwGPIO_Pin_11,
            hwGPIO_Pin_12,
            hwGPIO_Pin_10,
            hwGPIO_Pin_NC, //hwGPIO_Pin_13
        },

        /*
         * ALT1
         * SCK  = GPIO14
         * TX   = GPIO15
         * RX   = GPIO8
         * CS   = GPIO9
         */
        {
            hwGPIO_Pin_15,
            hwGPIO_Pin_8,
            hwGPIO_Pin_14,
            hwGPIO_Pin_9
        },

        /*
         * ALT2
         * SCK  = GPIO26
         * TX   = GPIO27
         * RX   = GPIO24
         * CS   = GPIO25
         */
        {
            hwGPIO_Pin_27,
            hwGPIO_Pin_24,
            hwGPIO_Pin_26,
            hwGPIO_Pin_25
        },
    }
};

#endif // DEVICE_RP2

#endif // SPI_PIN_RP2_H