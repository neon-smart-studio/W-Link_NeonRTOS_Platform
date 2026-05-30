#ifndef UART_PIN_NUVOTON_NUC4X2_H
#define UART_PIN_NUVOTON_NUC4X2_H

#include "UART_Pin_Nuvoton.h"

typedef enum {
    UART_Pinset_DEFAULT = 0,
    UART_Pinset_ALT1,
    UART_Pinset_MAX
} UART_Pinset_t;

/*---------------------------------------------------------------------------*/
/* Pinset Config */
/*---------------------------------------------------------------------------*/

#ifndef CONFIG_UART0_PINSET
#define CONFIG_UART0_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART1_PINSET
#define CONFIG_UART1_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART2_PINSET
#define CONFIG_UART2_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART3_PINSET
#define CONFIG_UART3_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART4_PINSET
#define CONFIG_UART4_PINSET UART_Pinset_DEFAULT
#endif

#ifndef CONFIG_UART5_PINSET
#define CONFIG_UART5_PINSET UART_Pinset_DEFAULT
#endif

static const UART_Pinset_t UART_Index_Map_Alt[hwUART_Index_MAX] =
{
#if defined(UART0_BASE)
    CONFIG_UART0_PINSET,
#endif

#if defined(UART1_BASE)
    CONFIG_UART1_PINSET,
#endif

#if defined(UART2_BASE)
    CONFIG_UART2_PINSET,
#endif

#if defined(UART3_BASE)
    CONFIG_UART3_PINSET,
#endif

#if defined(UART4_BASE)
    CONFIG_UART4_PINSET,
#endif

#if defined(UART5_BASE)
    CONFIG_UART5_PINSET,
#endif
};

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX][UART_Pinset_MAX] =
{
#if defined(UART0_BASE)
    {
        /* DEFAULT : PB.12 TXD0 / PB.13 RXD0 */
        {
            hwGPIO_Pin_B12,
            hwGPIO_Pin_B13,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },

        /* ALT1 : PA.14 TXD0 / PA.15 RXD0 */
        {
            hwGPIO_Pin_A14,
            hwGPIO_Pin_A15,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },
    },
#endif

#if defined(UART1_BASE)
    {
        /* DEFAULT : PB.2 TXD1 / PB.3 RXD1 */
        {
            hwGPIO_Pin_B2,
            hwGPIO_Pin_B3,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },

        /* ALT1 : PA.8 TXD1 / PA.9 RXD1 */
        {
            hwGPIO_Pin_A8,
            hwGPIO_Pin_A9,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },
    },
#endif

#if defined(UART2_BASE)
    {
        /* DEFAULT : PB.6 TXD2 / PB.7 RXD2 */
        {
            hwGPIO_Pin_B6,
            hwGPIO_Pin_B7,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },

        /* ALT1 : PD.14 TXD2 / PD.15 RXD2 */
        {
            hwGPIO_Pin_D14,
            hwGPIO_Pin_D15,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },
    },
#endif

#if defined(UART3_BASE)
    {
        /* DEFAULT : PB.10 TXD3 / PB.11 RXD3 */
        {
            hwGPIO_Pin_B10,
            hwGPIO_Pin_B11,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },

        /* ALT1 : PC.10 TXD3 / PC.11 RXD3 */
        {
            hwGPIO_Pin_C10,
            hwGPIO_Pin_C11,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },
    },
#endif

#if defined(UART4_BASE)
    {
        /* DEFAULT : PC.12 TXD4 / PC.13 RXD4 */
        {
            hwGPIO_Pin_C12,
            hwGPIO_Pin_C13,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },

        /* ALT1 : PE.8 TXD4 / PE.9 RXD4 */
        {
            hwGPIO_Pin_E8,
            hwGPIO_Pin_E9,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },
    },
#endif

#if defined(UART5_BASE)
    {
        /* DEFAULT : PE.0 TXD5 / PE.1 RXD5 */
        {
            hwGPIO_Pin_E0,
            hwGPIO_Pin_E1,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },

        /* ALT1 : PC.14 TXD5 / PC.15 RXD5 */
        {
            hwGPIO_Pin_C14,
            hwGPIO_Pin_C15,
            hwGPIO_Pin_NC,
            hwGPIO_Pin_NC
        },
    },
#endif
};

#endif /* UART_PIN_NUVOTON_NUC4X2_H */