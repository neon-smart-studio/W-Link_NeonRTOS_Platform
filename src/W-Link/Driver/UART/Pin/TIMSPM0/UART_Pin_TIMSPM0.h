#ifndef UART_PIN_TIMSPM0_H
#define UART_PIN_TIMSPM0_H

#include "UART_Pin_TIMSPM0_Def.h"

/* ============================================================
 * MSPM0C1105 / C1106
 * MSPM0H321x
 * UART0 / UART1 / UART2
 * ============================================================ */
#if defined(MSPM0C1105) || defined(MSPM0C1106) || \
    defined(MSPM0H321x)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    [hwUART_Index_0] = {hwGPIO_Pin_A10, hwGPIO_Pin_A11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_1] = {hwGPIO_Pin_B6, hwGPIO_Pin_B7, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_2] = {hwGPIO_Pin_B15, hwGPIO_Pin_B16, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};


/* ============================================================
 * MSPM0C1103 / C1104 / MSPS003
 * UART0
 * LP-MSPM0C1104 使用 PA27 TX / PA26 RX
 * ============================================================ */
#elif defined(MSPM0C110x)  || \
      defined(MSPM0S003Fx) || \
      defined(MSPS003Fx)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    [hwUART_Index_0] = {hwGPIO_Pin_A27, hwGPIO_Pin_A26, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};


/* ============================================================
 * MSPM0G110x / G150x / G310x / G350x
 * UART0 / UART1 / UART2 / UART3
 * ============================================================ */
#elif defined(MSPM0G110x) || defined(MSPM0G150x) || \
      defined(MSPM0G310x) || defined(MSPM0G350x)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    [hwUART_Index_0] = {hwGPIO_Pin_A10, hwGPIO_Pin_A11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_1] = {hwGPIO_Pin_A17, hwGPIO_Pin_A18, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_2] = {hwGPIO_Pin_B15, hwGPIO_Pin_B16, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_3] = {hwGPIO_Pin_B12, hwGPIO_Pin_B13, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};


/* ============================================================
 * MSPM0G151x / G351x / G352x
 *
 * UART0 / UART1 / UART3 / UART4 / UART5 / UART6 / UART7
 * 注意：這些系列沒有 UART2_BASE
 * ============================================================ */
#elif defined(MSPM0G151x) || defined(MSPM0G351x) || \
      defined(MSPM0G352x)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    [hwUART_Index_0] = {hwGPIO_Pin_A10, hwGPIO_Pin_A11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_1] = {hwGPIO_Pin_A17, hwGPIO_Pin_A18, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_3] = {hwGPIO_Pin_B12, hwGPIO_Pin_B13, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_4] = {hwGPIO_Pin_B10, hwGPIO_Pin_B11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /*
     * UART5：
     * TX = PA1
     * RX = PA0
     */
    [hwUART_Index_5] = {hwGPIO_Pin_A1, hwGPIO_Pin_A0, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /*
     * UART6：
     * TX = PB22
     * RX = PB21
     */
    [hwUART_Index_6] = {hwGPIO_Pin_B22, hwGPIO_Pin_B21, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_7] = {hwGPIO_Pin_B15, hwGPIO_Pin_B16, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};


/* ============================================================
 * MSPM0G120x / G121x / G320x / G321x
 *
 * UC0 / UC4 / UC5 / UC9 UART
 * ============================================================ */
#elif defined(MSPM0G120x) || defined(MSPM0G121x) || \
      defined(MSPM0G320x) || defined(MSPM0G321x)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    /* UC0 */
    [hwUART_Index_0] = {hwGPIO_Pin_A10, hwGPIO_Pin_A11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* UC4 */
    [hwUART_Index_4] = {hwGPIO_Pin_B8, hwGPIO_Pin_B9, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* UC5 */
    [hwUART_Index_5] = {hwGPIO_Pin_B4, hwGPIO_Pin_B5, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* UC9 */
    [hwUART_Index_9] = {hwGPIO_Pin_B12, hwGPIO_Pin_B13, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};


/* ============================================================
 * MSPM0G511x / G518x
 *
 * UC0 / UC1 / UC3 UART
 * ============================================================ */
#elif defined(MSPM0G511x) || defined(MSPM0G518x)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    /* UC0 */
    [hwUART_Index_0] = {hwGPIO_Pin_A10, hwGPIO_Pin_A11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* UC1 */
    [hwUART_Index_1] = {hwGPIO_Pin_B6, hwGPIO_Pin_B7, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* UC3 */
    [hwUART_Index_3] = {hwGPIO_Pin_B12, hwGPIO_Pin_B13, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};


/* ============================================================
 * MSPM0L110x / L130x / L134x
 * UART0 / UART1
 * ============================================================ */
#elif defined(MSPM0L110x) || defined(MSPM0L130x) || \
      defined(MSPM0L134x)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    [hwUART_Index_0] = {hwGPIO_Pin_A8, hwGPIO_Pin_A9, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_1] = {hwGPIO_Pin_A10, hwGPIO_Pin_A11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};


/* ============================================================
 * MSPM0L111x
 * UART0 / UART1
 * ============================================================ */
#elif defined(MSPM0L111x)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    [hwUART_Index_0] = {hwGPIO_Pin_A10, hwGPIO_Pin_A11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_1] = {hwGPIO_Pin_B6, hwGPIO_Pin_B7, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};


/* ============================================================
 * MSPM0L112x / L211x
 *
 * UC4 / UC8 / UC11 UART
 * ============================================================ */
#elif defined(MSPM0L112x) || defined(MSPM0L211x)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    /* UC4 */
    [hwUART_Index_4] = {hwGPIO_Pin_A10, hwGPIO_Pin_A11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* UC8 */
    [hwUART_Index_8] = {hwGPIO_Pin_B8, hwGPIO_Pin_B9, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    /* UC11 */
    [hwUART_Index_11] = {hwGPIO_Pin_B15, hwGPIO_Pin_B16, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};


/* ============================================================
 * MSPM0L122x / L222x
 * UART0 / UART1 / UART2 / UART3 / UART4
 * ============================================================ */
#elif defined(MSPM0L122x) || defined(MSPM0L222x)

static const UART_Pin_Def UART_Pin_Def_Table[hwUART_Index_MAX] =
{
    [hwUART_Index_0] = {hwGPIO_Pin_A10, hwGPIO_Pin_A11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_1] = {hwGPIO_Pin_A8, hwGPIO_Pin_A9, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_2] = {hwGPIO_Pin_B15, hwGPIO_Pin_B16, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_3] = {hwGPIO_Pin_B12, hwGPIO_Pin_B13, hwGPIO_Pin_NC, hwGPIO_Pin_NC },

    [hwUART_Index_4] = {hwGPIO_Pin_B10, hwGPIO_Pin_B11, hwGPIO_Pin_NC, hwGPIO_Pin_NC },
};

#endif

#undef UART_PIN_ENTRY

#endif // UART_PIN_TIMSPM0_H