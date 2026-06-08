#ifndef QSPI_PIN_STM32F4_H
#define QSPI_PIN_STM32F4_H

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

static const QSPI_Pinset_t QSPI_Index_Map_Alt[hwQSPI_Index_MAX] = {
    CONFIG_QSPI0_PINSET,
};

/*
 * QSPI_Pin_Def 建議格式：
 * { io0, io1, io2, io3, sclk, ncs }
 *
 * 如果你目前 struct 還是只有 4 個欄位，
 * 要先把 QSPI_Pin_Def 改成 6 個欄位。
 */
static const QSPI_Pin_Def QSPI_Pin_Def_Table[hwQSPI_Index_MAX][QSPI_Pinset_MAX] =
{
    {
        /*
         * 常見 STM32F4 QSPI pin：
         * IO0  = PF8
         * IO1  = PF9
         * IO2  = PF7
         * IO3  = PF6
         * CLK  = PF10
         * NCS  = PB6
         */
        {
            hwGPIO_Pin_F8,
            hwGPIO_Pin_F9,
            hwGPIO_Pin_F7,
            hwGPIO_Pin_F6,
            hwGPIO_Pin_F10,
            hwGPIO_Pin_B6
        },

        /*
         * ALT：
         * IO0  = PE12
         * IO1  = PE13
         * IO2  = PE14
         * IO3  = PE15
         * CLK  = PB2
         * NCS  = PB6
         */
        {
            hwGPIO_Pin_E12,
            hwGPIO_Pin_E13,
            hwGPIO_Pin_E14,
            hwGPIO_Pin_E15,
            hwGPIO_Pin_B2,
            hwGPIO_Pin_B6
        },
    },
};

static const QSPI_AF_Map QSPI_Pin_AF_Map[] =
{
    /* ================= QSPI DEFAULT ================= */
    { hwQSPI_Index_0, hwGPIO_Pin_F6,  GPIO_AF9_QSPI  },  /* BK1_IO3 */
    { hwQSPI_Index_0, hwGPIO_Pin_F7,  GPIO_AF9_QSPI  },  /* BK1_IO2 */
    { hwQSPI_Index_0, hwGPIO_Pin_F8,  GPIO_AF10_QSPI },  /* BK1_IO0 */
    { hwQSPI_Index_0, hwGPIO_Pin_F9,  GPIO_AF10_QSPI },  /* BK1_IO1 */
    { hwQSPI_Index_0, hwGPIO_Pin_F10, GPIO_AF9_QSPI  },  /* CLK */
    { hwQSPI_Index_0, hwGPIO_Pin_B6,  GPIO_AF10_QSPI },  /* NCS */

    /* ================= QSPI ALT ================= */
    { hwQSPI_Index_0, hwGPIO_Pin_E12, GPIO_AF10_QSPI },  /* BK1_IO0 */
    { hwQSPI_Index_0, hwGPIO_Pin_E13, GPIO_AF10_QSPI },  /* BK1_IO1 */
    { hwQSPI_Index_0, hwGPIO_Pin_E14, GPIO_AF10_QSPI },  /* BK1_IO2 */
    { hwQSPI_Index_0, hwGPIO_Pin_E15, GPIO_AF10_QSPI },  /* BK1_IO3 */
    { hwQSPI_Index_0, hwGPIO_Pin_B2,  GPIO_AF9_QSPI  },  /* CLK */
    { hwQSPI_Index_0, hwGPIO_Pin_B6,  GPIO_AF10_QSPI },  /* NCS */
};

#endif

#endif /* QSPI_PIN_STM32F4_H */