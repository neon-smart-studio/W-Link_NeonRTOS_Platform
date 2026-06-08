#ifndef QSPI_PIN_STM32WB_H
#define QSPI_PIN_STM32WB_H

#include "QSPI_Pin_STM32.h"

#if defined(QUADSPI)

typedef enum {
    QSPI_Pinset_DEFAULT = 0,
    QSPI_Pinset_ALT,
    QSPI_Pinset_MAX
} QSPI_Pinset_t;

#ifndef CONFIG_QSPI0_PINSET
#define CONFIG_QSPI0_PINSET QSPI_Pinset_DEFAULT
#endif

static const QSPI_Pinset_t QSPI_Index_Map_Alt[hwQSPI_Index_MAX] =
{
    CONFIG_QSPI0_PINSET,
};

static const QSPI_Pin_Def QSPI_Pin_Def_Table[hwQSPI_Index_MAX][QSPI_Pinset_MAX] =
{
    {
        /* DEFAULT */
        {
            hwGPIO_Pin_E12,  /* IO0 */
            hwGPIO_Pin_E13,  /* IO1 */
            hwGPIO_Pin_E14,  /* IO2 */
            hwGPIO_Pin_E15,  /* IO3 */
            hwGPIO_Pin_E10,  /* CLK */
            hwGPIO_Pin_E11   /* NCS */
        },

        /* ALT */
        {
            hwGPIO_Pin_F8,   /* IO0 */
            hwGPIO_Pin_F9,   /* IO1 */
            hwGPIO_Pin_F7,   /* IO2 */
            hwGPIO_Pin_F6,   /* IO3 */
            hwGPIO_Pin_F10,  /* CLK */
            hwGPIO_Pin_B11   /* NCS */
        },
    },
};

static const QSPI_AF_Map QSPI_Pin_AF_Map[] =
{
    /* DEFAULT */
    { hwQSPI_Index_0, hwGPIO_Pin_E12, GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_E13, GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_E14, GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_E15, GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_E10, GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_E11, GPIO_AF10_QSPI },

    /* ALT */
    { hwQSPI_Index_0, hwGPIO_Pin_F6,  GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_F7,  GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_F8,  GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_F9,  GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_F10, GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_B11, GPIO_AF10_QSPI },
};

#endif /* QUADSPI */

#endif /* QSPI_PIN_STM32WB_H */