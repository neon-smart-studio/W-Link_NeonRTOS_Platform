#ifndef UART_PIN_RP2_H
#define UART_PIN_RP2_H

#include "UART_Pin_RP2_Def.h"

typedef enum {
    UART_Pinset_DEFAULT = 0,
    UART_Pinset_ALT1,
    UART_Pinset_ALT2,
    UART_Pinset_ALT3,
    UART_Pinset_MAX
} UART_Pinset_t;

#ifndef CONFIG_UART0_PINSET
#define CONFIG_UART0_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART1_PINSET
#define CONFIG_UART1_PINSET UART_Pinset_DEFAULT
#endif

static const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] = {
    CONFIG_UART0_PINSET,
    CONFIG_UART1_PINSET,
};

/*
 * RP2040 / RP2350 UART pin pattern
 *
 * UART0:
 *   TX  = GP0 / GP12 / GP16 / GP28
 *   RX  = GP1 / GP13 / GP17 / GP29
 *   CTS = GP2 / GP14 / GP18
 *   RTS = GP3 / GP15 / GP19
 *
 * UART1:
 *   TX  = GP4 / GP8  / GP20 / GP24
 *   RX  = GP5 / GP9  / GP21 / GP25
 *   CTS = GP6 / GP10 / GP22 / GP26
 *   RTS = GP7 / GP11 / GP23 / GP27
 *
 * Table order: TX, RX, RTS, CTS
 */
static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
    {
        /* UART0 DEFAULT: TX, RX, RTS, CTS */
        { hwGPIO_Pin_0,  hwGPIO_Pin_1,  hwGPIO_Pin_3,  hwGPIO_Pin_2  },

        /* UART0 ALT1 */
        { hwGPIO_Pin_12, hwGPIO_Pin_13, hwGPIO_Pin_15, hwGPIO_Pin_14 },

        /* UART0 ALT2 */
        { hwGPIO_Pin_16, hwGPIO_Pin_17, hwGPIO_Pin_19, hwGPIO_Pin_18 },

        /* UART0 ALT3 */
        { hwGPIO_Pin_28, hwGPIO_Pin_29, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
    },

    {
        /* UART1 DEFAULT */
        { hwGPIO_Pin_4,  hwGPIO_Pin_5,  hwGPIO_Pin_7,  hwGPIO_Pin_6  },

        /* UART1 ALT1 */
        { hwGPIO_Pin_8,  hwGPIO_Pin_9,  hwGPIO_Pin_11, hwGPIO_Pin_10 },

        /* UART1 ALT2 */
        { hwGPIO_Pin_20, hwGPIO_Pin_21, hwGPIO_Pin_23, hwGPIO_Pin_22 },

        /* UART1 ALT3 */
        { hwGPIO_Pin_24, hwGPIO_Pin_25, hwGPIO_Pin_27, hwGPIO_Pin_26 },
    },
};

#endif // UART_PIN_STM32C0_H