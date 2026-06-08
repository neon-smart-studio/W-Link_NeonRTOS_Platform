#ifndef QSPI_PIN_STM32H7_H
#define QSPI_PIN_STM32H7_H

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
            hwGPIO_Pin_F8,   /* IO0 */
            hwGPIO_Pin_F9,   /* IO1 */
            hwGPIO_Pin_F7,   /* IO2 */
            hwGPIO_Pin_F6,   /* IO3 */
            hwGPIO_Pin_F10,  /* CLK */
            hwGPIO_Pin_B6    /* NCS */
        },

        /* ALT */
        {
            hwGPIO_Pin_D11,  /* IO0 */
            hwGPIO_Pin_D12,  /* IO1 */
            hwGPIO_Pin_E2,   /* IO2 */
            hwGPIO_Pin_D13,  /* IO3 */
            hwGPIO_Pin_B2,   /* CLK */
            hwGPIO_Pin_G6    /* NCS */
        },
    },
};

static const QSPI_AF_Map QSPI_Pin_AF_Map[] =
{
    /* DEFAULT */
    { hwQSPI_Index_0, hwGPIO_Pin_F6,  GPIO_AF9_QSPI  },
    { hwQSPI_Index_0, hwGPIO_Pin_F7,  GPIO_AF9_QSPI  },
    { hwQSPI_Index_0, hwGPIO_Pin_F8,  GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_F9,  GPIO_AF10_QSPI },
    { hwQSPI_Index_0, hwGPIO_Pin_F10, GPIO_AF9_QSPI  },
    { hwQSPI_Index_0, hwGPIO_Pin_B6,  GPIO_AF10_QSPI },

    /* ALT */
    { hwQSPI_Index_0, hwGPIO_Pin_D11, GPIO_AF9_QSPI  },
    { hwQSPI_Index_0, hwGPIO_Pin_D12, GPIO_AF9_QSPI  },
    { hwQSPI_Index_0, hwGPIO_Pin_E2,  GPIO_AF9_QSPI  },
    { hwQSPI_Index_0, hwGPIO_Pin_D13, GPIO_AF9_QSPI  },
    { hwQSPI_Index_0, hwGPIO_Pin_B2,  GPIO_AF9_QSPI  },
    { hwQSPI_Index_0, hwGPIO_Pin_G6,  GPIO_AF10_QSPI },
};

#endif

#endif /* QSPI_PIN_STM32H7_H */