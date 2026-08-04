#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "UART/UART.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSPM0

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"

#include "UART/Pin/TIMSPM0/UART_Pin_TIMSPM0.h"

#include "UART_TIMSPM0.h"

#if defined(UART0_BASE) || defined(UART1_BASE) || \
    defined(UART2_BASE) || defined(UART3_BASE) || \
    defined(UART4_BASE) || defined(UART5_BASE) || \
    defined(UART6_BASE) || defined(UART7_BASE) || \
    defined(UART8_BASE) || defined(UART9_BASE)
#define UART_TIMSPM0_HAS_LEGACY_UART
#endif

#if defined(UC0_UART_BASE) || defined(UC1_UART_BASE) || \
    defined(UC3_UART_BASE) || defined(UC4_UART_BASE) || \
    defined(UC5_UART_BASE) || defined(UC8_UART_BASE) || \
    defined(UC9_UART_BASE) || defined(UC11_UART_BASE)
#define UART_TIMSPM0_HAS_UNICOMM_UART
#endif

#define UART_INTERRUPT_MASK (DL_UART_MAIN_INTERRUPT_RX | DL_UART_MAIN_INTERRUPT_TX)

bool UART_Init_Status[hwUART_Index_MAX] = {false};

static uint32_t UART_BaudRate[hwUART_Index_MAX] = {0U};
static uint8_t UART_FrameBits[hwUART_Index_MAX] = {0U};

static NeonRTOS_SyncObj_t UART_Send_SyncHandle[hwUART_Index_MAX];
static NeonRTOS_SyncObj_t UART_Recv_SyncHandle[hwUART_Index_MAX];

typedef struct
{
    uint8_t *tx_buf;
    size_t tx_size;
    volatile size_t tx_count;
    volatile bool tx_busy;
    uint8_t *rx_buf;
    size_t rx_size;
    volatile size_t rx_count;
    volatile bool rx_busy;
} TIMSPM0_UART_IT_State;

static TIMSPM0_UART_IT_State UART_IT_State[hwUART_Index_MAX];


/*
 * UART 使用一般 hwGPIO_Pin，因此 IOMUX index 由
 * GPIO_Map_Soc_Pin_IOMUX() 統一取得。
 *
 * GPIO_Map_Soc_Int_Pin_IOMUX() 僅供 hwGPIO_Int_Pin 使用，
 * UART TX/RX 不需要呼叫。
 */
static uint32_t UART_Map_Soc_Pin_Function(
    hwUART_Index index,
    hwGPIO_Pin pin
)
{
    switch (index)
    {
#if defined(UART0_BASE)

        case hwUART_Index_0:

#if defined(MSPM0C110x) || defined(MSPM0S003Fx) || \
    defined(MSPS003Fx)

            switch (pin)
            {
                case hwGPIO_Pin_A27:
                    return IOMUX_PINCM28_PF_UART0_TX;

                case hwGPIO_Pin_A26:
                    return IOMUX_PINCM27_PF_UART0_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0C1105) || defined(MSPM0C1106) || \
      defined(MSPM0H321x)

            switch (pin)
            {
                case hwGPIO_Pin_A10:
                    return IOMUX_PINCM15_PF_UART0_TX;

                case hwGPIO_Pin_A11:
                    return IOMUX_PINCM16_PF_UART0_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0L110x) || defined(MSPM0L130x) || \
      defined(MSPM0L134x)

            switch (pin)
            {
                case hwGPIO_Pin_A8:
                    return IOMUX_PINCM9_PF_UART0_TX;

                case hwGPIO_Pin_A9:
                    return IOMUX_PINCM10_PF_UART0_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0L122x) || defined(MSPM0L222x)

            switch (pin)
            {
                case hwGPIO_Pin_A10:
                    return IOMUX_PINCM25_PF_UART0_TX;

                case hwGPIO_Pin_A11:
                    return IOMUX_PINCM26_PF_UART0_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0G110x) || defined(MSPM0G150x) || \
      defined(MSPM0G151x) || defined(MSPM0G310x) || \
      defined(MSPM0G350x) || defined(MSPM0G351x) || \
      defined(MSPM0G352x) || defined(MSPM0L111x)

            switch (pin)
            {
                case hwGPIO_Pin_A10:
                    return IOMUX_PINCM21_PF_UART0_TX;

                case hwGPIO_Pin_A11:
                    return IOMUX_PINCM22_PF_UART0_RX;

                default:
                    return 0U;
            }

#else

            return 0U;

#endif

#elif defined(UC0_UART_BASE)

        case hwUART_Index_0:
            switch (pin)
            {
                case hwGPIO_Pin_A10:
                    return IOMUX_PINCM21_PF_UC0_TX;

                case hwGPIO_Pin_A11:
                    return IOMUX_PINCM22_PF_UC0_RX;

                default:
                    return 0U;
            }

#endif

#if defined(UART1_BASE)

        case hwUART_Index_1:

#if defined(MSPM0C1105) || defined(MSPM0C1106) || \
    defined(MSPM0H321x)

            switch (pin)
            {
                case hwGPIO_Pin_B6:
                    return IOMUX_PINCM17_PF_UART1_TX;

                case hwGPIO_Pin_B7:
                    return IOMUX_PINCM18_PF_UART1_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0L110x) || defined(MSPM0L130x) || \
      defined(MSPM0L134x)

            switch (pin)
            {
                case hwGPIO_Pin_A10:
                    return IOMUX_PINCM11_PF_UART1_TX;

                case hwGPIO_Pin_A11:
                    return IOMUX_PINCM12_PF_UART1_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0L111x)

            switch (pin)
            {
                case hwGPIO_Pin_B6:
                    return IOMUX_PINCM23_PF_UART1_TX;

                case hwGPIO_Pin_B7:
                    return IOMUX_PINCM24_PF_UART1_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0L122x) || defined(MSPM0L222x)

            switch (pin)
            {
                case hwGPIO_Pin_A8:
                    return IOMUX_PINCM19_PF_UART1_TX;

                case hwGPIO_Pin_A9:
                    return IOMUX_PINCM20_PF_UART1_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0G110x) || defined(MSPM0G150x) || \
      defined(MSPM0G151x) || defined(MSPM0G310x) || \
      defined(MSPM0G350x) || defined(MSPM0G351x) || \
      defined(MSPM0G352x)

            switch (pin)
            {
                case hwGPIO_Pin_A17:
                    return IOMUX_PINCM39_PF_UART1_TX;

                case hwGPIO_Pin_A18:
                    return IOMUX_PINCM40_PF_UART1_RX;

                default:
                    return 0U;
            }

#else

            return 0U;

#endif

#elif defined(UC1_UART_BASE)

        case hwUART_Index_1:
            switch (pin)
            {
                case hwGPIO_Pin_B6:
                    return IOMUX_PINCM23_PF_UC1_TX;

                case hwGPIO_Pin_B7:
                    return IOMUX_PINCM24_PF_UC1_RX;

                default:
                    return 0U;
            }

#endif

#if defined(UART2_BASE)

        case hwUART_Index_2:

#if defined(MSPM0C1105) || defined(MSPM0C1106) || \
    defined(MSPM0H321x)

            switch (pin)
            {
                case hwGPIO_Pin_B15:
                    return IOMUX_PINCM22_PF_UART2_TX;

                case hwGPIO_Pin_B16:
                    return IOMUX_PINCM23_PF_UART2_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0L122x) || defined(MSPM0L222x)

            switch (pin)
            {
                case hwGPIO_Pin_B15:
                    return IOMUX_PINCM36_PF_UART2_TX;

                case hwGPIO_Pin_B16:
                    return IOMUX_PINCM37_PF_UART2_RX;

                default:
                    return 0U;
            }

#elif defined(MSPM0G110x) || defined(MSPM0G150x) || \
      defined(MSPM0G310x) || defined(MSPM0G350x)

            switch (pin)
            {
                case hwGPIO_Pin_B15:
                    return IOMUX_PINCM32_PF_UART2_TX;

                case hwGPIO_Pin_B16:
                    return IOMUX_PINCM33_PF_UART2_RX;

                default:
                    return 0U;
            }

#else

            return 0U;

#endif

#endif

#if defined(UART3_BASE)

        case hwUART_Index_3:

#if defined(MSPM0L122x) || defined(MSPM0L222x)

            switch (pin)
            {
                case hwGPIO_Pin_B12:
                    return IOMUX_PINCM33_PF_UART3_TX;

                case hwGPIO_Pin_B13:
                    return IOMUX_PINCM34_PF_UART3_RX;

                default:
                    return 0U;
            }

#else

            switch (pin)
            {
                case hwGPIO_Pin_B12:
                    return IOMUX_PINCM29_PF_UART3_TX;

                case hwGPIO_Pin_B13:
                    return IOMUX_PINCM30_PF_UART3_RX;

                default:
                    return 0U;
            }

#endif

#elif defined(UC3_UART_BASE)

        case hwUART_Index_3:
            switch (pin)
            {
                case hwGPIO_Pin_B12:
                    return IOMUX_PINCM29_PF_UC3_TX;

                case hwGPIO_Pin_B13:
                    return IOMUX_PINCM30_PF_UC3_RX;

                default:
                    return 0U;
            }

#endif

#if defined(UART4_BASE)

        case hwUART_Index_4:

#if defined(MSPM0L122x) || defined(MSPM0L222x)

            switch (pin)
            {
                case hwGPIO_Pin_B10:
                    return IOMUX_PINCM31_PF_UART4_TX;

                case hwGPIO_Pin_B11:
                    return IOMUX_PINCM32_PF_UART4_RX;

                default:
                    return 0U;
            }

#else

            switch (pin)
            {
                case hwGPIO_Pin_B10:
                    return IOMUX_PINCM27_PF_UART4_TX;

                case hwGPIO_Pin_B11:
                    return IOMUX_PINCM28_PF_UART4_RX;

                default:
                    return 0U;
            }

#endif

#elif defined(UC4_UART_BASE)

        case hwUART_Index_4:

#if defined(MSPM0L112x) || defined(MSPM0L211x)

            switch (pin)
            {
                case hwGPIO_Pin_A10:
                    return IOMUX_PINCM21_PF_UC4_TX;

                case hwGPIO_Pin_A11:
                    return IOMUX_PINCM22_PF_UC4_RX;

                default:
                    return 0U;
            }

#else

            switch (pin)
            {
                case hwGPIO_Pin_B8:
                    return IOMUX_PINCM25_PF_UC4_TX;

                case hwGPIO_Pin_B9:
                    return IOMUX_PINCM26_PF_UC4_RX;

                default:
                    return 0U;
            }

#endif

#endif

#if defined(UART5_BASE)

        case hwUART_Index_5:
            switch (pin)
            {
                case hwGPIO_Pin_A1:
                    return IOMUX_PINCM2_PF_UART5_TX;

                case hwGPIO_Pin_A0:
                    return IOMUX_PINCM1_PF_UART5_RX;

                default:
                    return 0U;
            }

#elif defined(UC5_UART_BASE)

        case hwUART_Index_5:
            switch (pin)
            {
                case hwGPIO_Pin_B4:
                    return IOMUX_PINCM17_PF_UC5_TX;

                case hwGPIO_Pin_B5:
                    return IOMUX_PINCM18_PF_UC5_RX;

                default:
                    return 0U;
            }

#endif

#if defined(UART6_BASE)

        case hwUART_Index_6:
            switch (pin)
            {
                case hwGPIO_Pin_B22:
                    return IOMUX_PINCM50_PF_UART6_TX;

                case hwGPIO_Pin_B21:
                    return IOMUX_PINCM49_PF_UART6_RX;

                default:
                    return 0U;
            }

#endif

#if defined(UART7_BASE)

        case hwUART_Index_7:
            switch (pin)
            {
                case hwGPIO_Pin_B15:
                    return IOMUX_PINCM32_PF_UART7_TX;

                case hwGPIO_Pin_B16:
                    return IOMUX_PINCM33_PF_UART7_RX;

                default:
                    return 0U;
            }

#endif

#if defined(UC8_UART_BASE)

        case hwUART_Index_8:
            switch (pin)
            {
                case hwGPIO_Pin_B8:
                    return IOMUX_PINCM25_PF_UC8_TX;

                case hwGPIO_Pin_B9:
                    return IOMUX_PINCM26_PF_UC8_RX;

                default:
                    return 0U;
            }

#endif

#if defined(UC9_UART_BASE)

        case hwUART_Index_9:
            switch (pin)
            {
                case hwGPIO_Pin_B12:
                    return IOMUX_PINCM29_PF_UC9_TX;

                case hwGPIO_Pin_B13:
                    return IOMUX_PINCM30_PF_UC9_RX;

                default:
                    return 0U;
            }

#endif

#if defined(UC11_UART_BASE)

        case hwUART_Index_11:
            switch (pin)
            {
                case hwGPIO_Pin_B15:
                    return IOMUX_PINCM32_PF_UC11_TX;

                case hwGPIO_Pin_B16:
                    return IOMUX_PINCM33_PF_UC11_RX;

                default:
                    return 0U;
            }

#endif

        default:
            return 0U;
    }
}

#ifdef UART_TIMSPM0_HAS_LEGACY_UART
static UART_Regs* UART_Map_Soc_Base(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0:
            return UART0_BASE;
#endif

#if defined(UART1_BASE)
        case hwUART_Index_1:
            return UART1_BASE;
#endif

#if defined(UART2_BASE)
        case hwUART_Index_2:
            return UART2_BASE;
#endif

#if defined(UART3_BASE)
        case hwUART_Index_3:
            return UART3_BASE;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_4:
            return UART4_BASE;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_5:
            return UART5_BASE;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_6:
            return UART6_BASE;
#endif

#if defined(UART7_BASE)
        case hwUART_Index_7:
            return UART7_BASE;
#endif

#if defined(UART8_BASE)
        case hwUART_Index_8:
            return UART8_BASE;
#endif

#if defined(UART9_BASE)
        case hwUART_Index_9:
            return UART9_BASE;
#endif

        default:
            return 0U;
    }
}
#endif // UART_TIMSPM0_HAS_LEGACY_UART


#ifdef UART_TIMSPM0_HAS_UNICOMM_UART
static UNICOMM_Inst_Regs* UART_Map_Soc_Base(hwUART_Index index)
{
    switch (index)
    {
#if defined(UC0_UART_BASE)
        case hwUART_Index_0:
            return UC0_UART_BASE;
#endif

#if defined(UC1_UART_BASE)
        case hwUART_Index_1:
            return UC1_UART_BASE;
#endif

#if defined(UC3_UART_BASE)
        case hwUART_Index_3:
            return UC3_UART_BASE;
#endif

#if defined(UC4_UART_BASE)
        case hwUART_Index_4:
            return UC4_UART_BASE;
#endif

#if defined(UC5_UART_BASE)
        case hwUART_Index_5:
            return UUC5_UART_BASE5;
#endif

#if defined(UC8_UART_BASE)
        case hwUART_Index_8:
            return UC8_UART_BASE;
#endif

#if defined(UC9_UART_BASE)
        case hwUART_Index_9:
            return UC9_UART_BASE;
#endif

#if defined(UC11_UART_BASE)
        case hwUART_Index_11:
            return UC11_UART_BASE;
#endif

        default:
            return 0U;
    }
}
#endif // UART_TIMSPM0_HAS_UNICOMM_UART

#if defined(UART0_BASE)
void UART0_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_0);
}
#elif defined(UC0_UART_BASE)
void UC0_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_0);
}
#endif

#if defined(UART1_BASE)
void UART1_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_1);
}
#elif defined(UC1_UART_BASE)
void UC1_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_1);
}
#endif

#if defined(UART2_BASE)
void UART2_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_2);
}
#elif defined(UC2_UART_BASE)
void UC2_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_2);
}
#endif

#if defined(UART3_BASE)
void UART3_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_3);
}
#elif defined(UC3_UART_BASE)
void UC3_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_3);
}
#endif

#if defined(UART4_BASE)
void UART4_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_4);
}
#elif defined(UC4_UART_BASE)
void UC4_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_4);
}
#endif

#if defined(UART5_BASE)
void UART5_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_5);
}
#elif defined(UC5_UART_BASE)
void UC5_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_5);
}
#endif

#if defined(UART6_BASE)
void UART6_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_6);
}
#elif defined(UC6_UART_BASE)
void UC6_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_6);
}
#endif

#if defined(UART7_BASE)
void UART7_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_7);
}
#elif defined(UC7_UART_BASE)
void UC7_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_7);
}
#endif

#if defined(UART8_BASE)
void UART8_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_8);
}
#elif defined(UC8_UART_BASE)
void UC8_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_8);
}
#endif

#if defined(UART9_BASE)
void UART9_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_9);
}
#elif defined(UC9_UART_BASE)
void UC9_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_9);
}
#endif

#if defined(UART10_BASE)
void UART10_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_10);
}
#elif defined(UC10_UART_BASE)
void UC10_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_10);
}
#endif

#if defined(UART11_BASE)
void UART11_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_11);
}
#elif defined(UC11_UART_BASE)
void UC11_IRQHandler(void)
{
    UART_IRQ_Process(hwUART_Index_11);
}
#endif

static void UART_NVIC_Init(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0:
            NVIC_ClearPendingIRQ(UART0_INT_IRQn);
            NVIC_EnableIRQ(UART0_INT_IRQn);
            break;
#elif defined(UC0_UART_BASE)
        case hwUART_Index_0:
            NVIC_ClearPendingIRQ(UC0_INT_IRQn);
            NVIC_EnableIRQ(UC0_INT_IRQn);
            break;
#endif

#if defined(UART1_BASE)
        case hwUART_Index_1:
            NVIC_ClearPendingIRQ(UART1_INT_IRQn);
            NVIC_EnableIRQ(UART1_INT_IRQn);
            break;
#elif defined(UC1_UART_BASE)
        case hwUART_Index_1:
            NVIC_ClearPendingIRQ(UC1_INT_IRQn);
            NVIC_EnableIRQ(UC1_INT_IRQn);
            break;
#endif

#if defined(UART2_BASE)
        case hwUART_Index_2:
            NVIC_ClearPendingIRQ(UART2_INT_IRQn);
            NVIC_EnableIRQ(UART2_INT_IRQn);
            break;
#elif defined(UC2_UART_BASE)
        case hwUART_Index_2:
            NVIC_ClearPendingIRQ(UC2_INT_IRQn);
            NVIC_EnableIRQ(UC2_INT_IRQn);
            break;
#endif

#if defined(UART3_BASE)
        case hwUART_Index_3:
            NVIC_ClearPendingIRQ(UART3_INT_IRQn);
            NVIC_EnableIRQ(UART3_INT_IRQn);
            break;
#elif defined(UC3_UART_BASE)
        case hwUART_Index_3:
            NVIC_ClearPendingIRQ(UC3_INT_IRQn);
            NVIC_EnableIRQ(UC3_INT_IRQn);
            break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_4:
            NVIC_ClearPendingIRQ(UART4_INT_IRQn);
            NVIC_EnableIRQ(UART4_INT_IRQn);
            break;
#elif defined(UC4_UART_BASE)
        case hwUART_Index_4:
            NVIC_ClearPendingIRQ(UC4_INT_IRQn);
            NVIC_EnableIRQ(UC4_INT_IRQn);
            break;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_5:
            NVIC_ClearPendingIRQ(UART5_INT_IRQn);
            NVIC_EnableIRQ(UART5_INT_IRQn);
            break;
#elif defined(UC5_UART_BASE)
        case hwUART_Index_5:
            NVIC_ClearPendingIRQ(UC5_INT_IRQn);
            NVIC_EnableIRQ(UC5_INT_IRQn);
            break;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_6:
            NVIC_ClearPendingIRQ(UART6_INT_IRQn);
            NVIC_EnableIRQ(UART6_INT_IRQn);
            break;
#elif defined(UC6_UART_BASE)
        case hwUART_Index_6:
            NVIC_ClearPendingIRQ(UC6_INT_IRQn);
            NVIC_EnableIRQ(UC6_INT_IRQn);
            break;
#endif

#if defined(UART7_BASE)
        case hwUART_Index_7:
            NVIC_ClearPendingIRQ(UART7_INT_IRQn);
            NVIC_EnableIRQ(UART7_INT_IRQn);
            break;
#elif defined(UC7_UART_BASE)
        case hwUART_Index_7:
            NVIC_ClearPendingIRQ(UC7_INT_IRQn);
            NVIC_EnableIRQ(UC7_INT_IRQn);
            break;
#endif

#if defined(UART8_BASE)
        case hwUART_Index_8:
            NVIC_ClearPendingIRQ(UART8_INT_IRQn);
            NVIC_EnableIRQ(UART8_INT_IRQn);
            break;
#elif defined(UC8_UART_BASE)
        case hwUART_Index_8:
            NVIC_ClearPendingIRQ(UC8_INT_IRQn);
            NVIC_EnableIRQ(UC8_INT_IRQn);
            break;
#endif

#if defined(UART9_BASE)
        case hwUART_Index_9:
            NVIC_ClearPendingIRQ(UART9_INT_IRQn);
            NVIC_EnableIRQ(UART9_INT_IRQn);
            break;
#elif defined(UC9_UART_BASE)
        case hwUART_Index_9:
            NVIC_ClearPendingIRQ(UC9_INT_IRQn);
            NVIC_EnableIRQ(UC9_INT_IRQn);
            break;
#endif

#if defined(UART10_BASE)
        case hwUART_Index_10:
            NVIC_ClearPendingIRQ(UART10_INT_IRQn);
            NVIC_EnableIRQ(UART10_INT_IRQn);
            break;
#elif defined(UC10_UART_BASE)
        case hwUART_Index_10:
            NVIC_ClearPendingIRQ(UC10_INT_IRQn);
            NVIC_EnableIRQ(UC10_INT_IRQn);
            break;
#endif

#if defined(UART11_BASE)
        case hwUART_Index_11:
            NVIC_ClearPendingIRQ(UART11_INT_IRQn);
            NVIC_EnableIRQ(UART11_INT_IRQn);
            break;
#elif defined(UC11_UART_BASE)
        case hwUART_Index_11:
            NVIC_ClearPendingIRQ(UC11_INT_IRQn);
            NVIC_EnableIRQ(UC11_INT_IRQn);
            break;
#endif

        default:
            break;
    }
}


static void UART_NVIC_DeInit(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART0_BASE)
        case hwUART_Index_0:
            NVIC_DisableIRQ(UART0_INT_IRQn);
            NVIC_ClearPendingIRQ(UART0_INT_IRQn);
            break;
#elif defined(UC0_UART_BASE)
        case hwUART_Index_0:
            NVIC_DisableIRQ(UC0_INT_IRQn);
            NVIC_ClearPendingIRQ(UC0_INT_IRQn);
            break;
#endif

#if defined(UART1_BASE)
        case hwUART_Index_1:
            NVIC_DisableIRQ(UART1_INT_IRQn);
            NVIC_ClearPendingIRQ(UART1_INT_IRQn);
            break;
#elif defined(UC1_UART_BASE)
        case hwUART_Index_1:
            NVIC_DisableIRQ(UC1_INT_IRQn);
            NVIC_ClearPendingIRQ(UC1_INT_IRQn);
            break;
#endif

#if defined(UART2_BASE)
        case hwUART_Index_2:
            NVIC_DisableIRQ(UART2_INT_IRQn);
            NVIC_ClearPendingIRQ(UART2_INT_IRQn);
            break;
#elif defined(UC2_UART_BASE)
        case hwUART_Index_2:
            NVIC_DisableIRQ(UC2_INT_IRQn);
            NVIC_ClearPendingIRQ(UC2_INT_IRQn);
            break;
#endif

#if defined(UART3_BASE)
        case hwUART_Index_3:
            NVIC_DisableIRQ(UART3_INT_IRQn);
            NVIC_ClearPendingIRQ(UART3_INT_IRQn);
            break;
#elif defined(UC3_UART_BASE)
        case hwUART_Index_3:
            NVIC_DisableIRQ(UC3_INT_IRQn);
            NVIC_ClearPendingIRQ(UC3_INT_IRQn);
            break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_4:
            NVIC_DisableIRQ(UART4_INT_IRQn);
            NVIC_ClearPendingIRQ(UART4_INT_IRQn);
            break;
#elif defined(UC4_UART_BASE)
        case hwUART_Index_4:
            NVIC_DisableIRQ(UC4_INT_IRQn);
            NVIC_ClearPendingIRQ(UC4_INT_IRQn);
            break;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_5:
            NVIC_DisableIRQ(UART5_INT_IRQn);
            NVIC_ClearPendingIRQ(UART5_INT_IRQn);
            break;
#elif defined(UC5_UART_BASE)
        case hwUART_Index_5:
            NVIC_DisableIRQ(UC5_INT_IRQn);
            NVIC_ClearPendingIRQ(UC5_INT_IRQn);
            break;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_6:
            NVIC_DisableIRQ(UART6_INT_IRQn);
            NVIC_ClearPendingIRQ(UART6_INT_IRQn);
            break;
#elif defined(UC6_UART_BASE)
        case hwUART_Index_6:
            NVIC_DisableIRQ(UC6_INT_IRQn);
            NVIC_ClearPendingIRQ(UC6_INT_IRQn);
            break;
#endif

#if defined(UART7_BASE)
        case hwUART_Index_7:
            NVIC_DisableIRQ(UART7_INT_IRQn);
            NVIC_ClearPendingIRQ(UART7_INT_IRQn);
            break;
#elif defined(UC7_UART_BASE)
        case hwUART_Index_7:
            NVIC_DisableIRQ(UC7_INT_IRQn);
            NVIC_ClearPendingIRQ(UC7_INT_IRQn);
            break;
#endif

#if defined(UART8_BASE)
        case hwUART_Index_8:
            NVIC_DisableIRQ(UART8_INT_IRQn);
            NVIC_ClearPendingIRQ(UART8_INT_IRQn);
            break;
#elif defined(UC8_UART_BASE)
        case hwUART_Index_8:
            NVIC_DisableIRQ(UC8_INT_IRQn);
            NVIC_ClearPendingIRQ(UC8_INT_IRQn);
            break;
#endif

#if defined(UART9_BASE)
        case hwUART_Index_9:
            NVIC_DisableIRQ(UART9_INT_IRQn);
            NVIC_ClearPendingIRQ(UART9_INT_IRQn);
            break;
#elif defined(UC9_UART_BASE)
        case hwUART_Index_9:
            NVIC_DisableIRQ(UC9_INT_IRQn);
            NVIC_ClearPendingIRQ(UC9_INT_IRQn);
            break;
#endif

#if defined(UART10_BASE)
        case hwUART_Index_10:
            NVIC_DisableIRQ(UART10_INT_IRQn);
            NVIC_ClearPendingIRQ(UART10_INT_IRQn);
            break;
#elif defined(UC10_UART_BASE)
        case hwUART_Index_10:
            NVIC_DisableIRQ(UC10_INT_IRQn);
            NVIC_ClearPendingIRQ(UC10_INT_IRQn);
            break;
#endif

#if defined(UART11_BASE)
        case hwUART_Index_11:
            NVIC_DisableIRQ(UART11_INT_IRQn);
            NVIC_ClearPendingIRQ(UART11_INT_IRQn);
            break;
#elif defined(UC11_UART_BASE)
        case hwUART_Index_11:
            NVIC_DisableIRQ(UC11_INT_IRQn);
            NVIC_ClearPendingIRQ(UC11_INT_IRQn);
            break;
#endif

        default:
            break;
    }
}

void UART_IRQ_Process(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX) {
        return;
    }

    void *base = (void *) (uintptr_t) UART_Map_Soc_Base(index);
    if (base == NULL) {
        return;
    }

    TIMSPM0_UART_IT_State *state = &UART_IT_State[index];

    while (true)
    {
        uint32_t pending = DL_UART_Main_getPendingInterrupt(base);

        switch (pending)
        {
            case DL_UART_MAIN_IIDX_RX:

                if (!state->rx_busy)
                {
                    DL_UART_Main_disableInterrupt(base, DL_UART_MAIN_INTERRUPT_RX);

                    break;
                }

                while (!DL_UART_Main_isRXFIFOEmpty(base) && (state->rx_count < state->rx_size))
                {
                    state->rx_buf[state->rx_count++] = DL_UART_Main_receiveData(base);
                }

                if (state->rx_count >= state->rx_size)
                {
                    DL_UART_Main_disableInterrupt(base, DL_UART_MAIN_INTERRUPT_RX);

                    state->rx_busy = false;

                    NeonRTOS_SyncObjSignalFromISR(&UART_Recv_SyncHandle[index]);
                }

                break;

            case DL_UART_MAIN_IIDX_TX:

                if (!state->tx_busy)
                {
                    DL_UART_Main_disableInterrupt(base, DL_UART_MAIN_INTERRUPT_TX);

                    break;
                }

                while (!DL_UART_Main_isTXFIFOFull(base) &&
                       (state->tx_count < state->tx_size))
                {
                    DL_UART_Main_transmitData(base, state->tx_buf[state->tx_count++]);
                }

                if (state->tx_count >= state->tx_size)
                {
                    DL_UART_Main_disableInterrupt(base, DL_UART_MAIN_INTERRUPT_TX);

                    state->tx_busy = false;

                    NeonRTOS_SyncObjSignalFromISR(&UART_Send_SyncHandle[index]);
                }

                break;

            case DL_UART_MAIN_IIDX_NO_INTERRUPT:
                return;

            default:
                break;
        }
    }
}

hwUART_OpResult UART_Open(hwUART_Index index, uint32_t baudrate, bool rts_cts)
{
    if (rts_cts) {
        return hwUART_Unsupport;
    }

    return UART_Open_Specific_Format(index, baudrate, false, 8U, UART_Parity_None, 1U);
}

hwUART_OpResult UART_Open_Specific_Format(hwUART_Index index, uint32_t baudrate, bool rts_cts, uint8_t data_bits, UART_Parity parity, uint8_t stop_bits)
{
    if ((index >= hwUART_Index_MAX) || (baudrate == 0U)) {
        return hwUART_InvalidParameter;
    }

    if (UART_Init_Status[index]) {
        return hwUART_Busy;
    }

    if (rts_cts) {
        return hwUART_Unsupport;
    }

    if ((parity != UART_Parity_None) &&
        (parity != UART_Parity_Odd) &&
        (parity != UART_Parity_Even))
    {
        return hwUART_InvalidParameter;
    }

    if ((stop_bits != 1U) && (stop_bits != 2U)) {
        return hwUART_InvalidParameter;
    }

    if (data_bits != 8U) {
        return hwUART_Unsupport;
    }

    hwGPIO_Pin tx_pin = UART_Pin_Def_Table[index].tx_pin;
    hwGPIO_Pin rx_pin = UART_Pin_Def_Table[index].rx_pin;

    if ((tx_pin == hwGPIO_Pin_NC) ||
        (rx_pin == hwGPIO_Pin_NC))
    {
        return hwUART_InvalidParameter;
    }

#ifdef UART_TIMSPM0_HAS_LEGACY_UART
    UART_Regs* base = UART_Map_Soc_Base(index);
#endif
#ifdef UART_TIMSPM0_HAS_UNICOMM_UART
    UNICOMM_Inst_Regs* base = UART_Map_Soc_Base(index);
#endif
    if (base == NULL)
    {
        return hwUART_InvalidParameter;
    }

    uint32_t tx_iomux = GPIO_Map_Soc_Pin_IOMUX(tx_pin);
    uint32_t rx_iomux = GPIO_Map_Soc_Pin_IOMUX(rx_pin);

    uint32_t tx_function = UART_Map_Soc_Pin_Function(index, tx_pin);
    uint32_t rx_function = UART_Map_Soc_Pin_Function(index, rx_pin);

    /* IOMUX_PINCM1 is 0, so a zero IOMUX index is not an error. */
    if ((tx_function == 0U) || (rx_function == 0U))
    {
        return hwUART_InvalidParameter;
    }

    if (NeonRTOS_SyncObjCreate(&UART_Send_SyncHandle[index]) != NeonRTOS_OK)
    {
        return hwUART_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&UART_Recv_SyncHandle[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);

        return hwUART_MemoryError;
    }

    DL_UART_Main_reset(base);
    DL_UART_Main_enablePower(base);

    delay_cycles(16U);

    DL_GPIO_initPeripheralOutputFunction(
        tx_iomux,
        tx_function);

    DL_GPIO_initPeripheralInputFunction(
        rx_iomux,
        rx_function);

    const DL_UART_Main_ClockConfig clock_config =
    {
        .clockSel = DL_UART_MAIN_CLOCK_BUSCLK,
        .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
    };

    DL_UART_Main_Config uart_config =
    {
        .mode = DL_UART_MAIN_MODE_NORMAL,
        .direction = DL_UART_MAIN_DIRECTION_TX_RX,
        .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
        .parity = DL_UART_MAIN_PARITY_NONE,
        .wordLength = DL_UART_MAIN_WORD_LENGTH_8_BITS,
        .stopBits = DL_UART_MAIN_STOP_BITS_ONE
    };

    switch (parity)
    {
        case UART_Parity_None:
            uart_config.parity = DL_UART_MAIN_PARITY_NONE;
            break;

        case UART_Parity_Odd:
            uart_config.parity = DL_UART_MAIN_PARITY_ODD;
            break;

        case UART_Parity_Even:
            uart_config.parity = DL_UART_MAIN_PARITY_EVEN;
            break;

        default:
            NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
            NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);

            return hwUART_InvalidParameter;
    }

    if (stop_bits == 2U) {
        uart_config.stopBits = DL_UART_MAIN_STOP_BITS_TWO;
    }

    DL_UART_Main_setClockConfig(base, (DL_UART_Main_ClockConfig *) &clock_config);

    DL_UART_Main_init(base, &uart_config);

    DL_UART_Main_configBaudRate(base, g_sys_clock_hz, baudrate);

    DL_UART_Main_enableFIFOs(base);

    DL_UART_Main_setRXFIFOThreshold(base, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);

    DL_UART_Main_setTXFIFOThreshold(base, DL_UART_TX_FIFO_LEVEL_1_2_EMPTY);

    DL_UART_Main_disableInterrupt(base, UART_INTERRUPT_MASK);

    DL_UART_Main_clearInterruptStatus(base, UART_INTERRUPT_MASK);

    while (!DL_UART_Main_isRXFIFOEmpty(base))
    {
        (void) DL_UART_Main_receiveData(base);
    }

    memset(&UART_IT_State[index], 0, sizeof(UART_IT_State[index]));

    UART_BaudRate[index] = baudrate;
    UART_FrameBits[index] = (uint8_t) (1U + data_bits + ((parity == UART_Parity_None) ? 0U : 1U) + stop_bits);

    DL_UART_Main_enable(base);

    UART_NVIC_Enable(index);

    gpio_pin_init_status[tx_pin] = true;
    gpio_pin_init_status[rx_pin] = true;

    UART_Init_Status[index] = true;

    return hwUART_OK;
}

hwUART_OpResult UART_Close(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_NotInit;
    }

    hwGPIO_Pin tx_pin = UART_Pin_Def_Table[index].tx_pin;
    hwGPIO_Pin rx_pin = UART_Pin_Def_Table[index].rx_pin;

#ifdef UART_TIMSPM0_HAS_LEGACY_UART
    UART_Regs* base = UART_Map_Soc_Base(index);
#endif
#ifdef UART_TIMSPM0_HAS_UNICOMM_UART
    UNICOMM_Inst_Regs* base = UART_Map_Soc_Base(index);
#endif
    if (base == NULL)
    {
        return hwUART_InvalidParameter;
    }

    UART_NVIC_Disable(index);

    DL_UART_Main_disable(base);
    DL_UART_Main_reset(base);
    DL_UART_Main_disablePower(base);

    DL_GPIO_initDigitalInput(GPIO_Map_Soc_Pin_IOMUX(tx_pin));

    DL_GPIO_initDigitalInput(GPIO_Map_Soc_Pin_IOMUX(rx_pin));

    NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);

    memset(&UART_IT_State[index], 0, sizeof(UART_IT_State[index]));

    gpio_pin_init_status[tx_pin] = false;
    gpio_pin_init_status[rx_pin] = false;

    UART_BaudRate[index] = 0U;
    UART_FrameBits[index] = 0U;
    UART_Init_Status[index] = false;

    return hwUART_OK;
}


hwUART_OpResult UART_Read(hwUART_Index index, uint8_t *data_rd, size_t size, uint32_t timeoutMs)
{
    if ((index >= hwUART_Index_MAX) ||
        ((data_rd == NULL) && (size != 0U)))
    {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_NotInit;
    }

    if (size == 0U) {
        return (hwUART_OpResult) 0;
    }

#ifdef UART_TIMSPM0_HAS_LEGACY_UART
    UART_Regs* base = UART_Map_Soc_Base(index);
#endif
#ifdef UART_TIMSPM0_HAS_UNICOMM_UART
    UNICOMM_Inst_Regs* base = UART_Map_Soc_Base(index);
#endif
    if (base == NULL)
    {
        return hwUART_InvalidParameter;
    }

    TIMSPM0_UART_IT_State *state = &UART_IT_State[index];

    if (state->rx_busy)
    {
        return hwUART_Busy;
    }

    if (timeoutMs == NEONRT_WAIT_FOREVER) {
        return NEONRT_WAIT_FOREVER;
    }

    if ((UART_BaudRate[index] == 0U) || (size == 0U)) {
        return (NeonRTOS_Time_t) timeoutMs;
    }

    uint64_t transfer_bits = (uint64_t) size * (uint64_t) UART_FrameBits[index];

    uint64_t transfer_ms = ((transfer_bits * 1000ULL) + UART_BaudRate[index] - 1ULL) / UART_BaudRate[index];

    uint64_t wait_ms = transfer_ms + timeoutMs + 1ULL;

    if (wait_ms >= UINT32_MAX) {
        wait_ms = UINT32_MAX - 1ULL;
    }

    NeonRTOS_SyncObjClear(&UART_Recv_SyncHandle[index]);

    state->rx_buf = data_rd;
    state->rx_size = size;
    state->rx_count = 0U;
    state->rx_busy = true;

    while (!DL_UART_Main_isRXFIFOEmpty(base) && (state->rx_count < state->rx_size))
    {
        state->rx_buf[state->rx_count++] = DL_UART_Main_receiveData(base);
    }

    if (state->rx_count < state->rx_size)
    {
        DL_UART_Main_clearInterruptStatus(base, DL_UART_MAIN_INTERRUPT_RX);

        DL_UART_Main_enableInterrupt(base, DL_UART_MAIN_INTERRUPT_RX);

        if (NeonRTOS_SyncObjWait(&UART_Recv_SyncHandle[index], wait_ms) != NeonRTOS_OK)
        {
            DL_UART_Main_disableInterrupt(base, DL_UART_MAIN_INTERRUPT_RX);

            state->rx_busy = false;

            if (state->rx_count > 0U) {
                return (hwUART_OpResult) state->rx_count;
            }

            return hwUART_Busy;
        }
    }
    else
    {
        state->rx_busy = false;
    }

    return (hwUART_OpResult) state->rx_count;
}


hwUART_OpResult UART_GetChar(hwUART_Index index, uint8_t *char_rd, uint32_t timeoutMs)
{
    return UART_Read(index, char_rd, 1U, timeoutMs);
}

hwUART_OpResult UART_Write(hwUART_Index index, uint8_t *data_wr, size_t size, uint32_t timeoutMs)
{
    if ((index >= hwUART_Index_MAX) || ((data_wr == NULL) && (size != 0U)))
    {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_NotInit;
    }

    if (size == 0U) {
        return (hwUART_OpResult) 0;
    }

#ifdef UART_TIMSPM0_HAS_LEGACY_UART
    UART_Regs* base = UART_Map_Soc_Base(index);
#endif
#ifdef UART_TIMSPM0_HAS_UNICOMM_UART
    UNICOMM_Inst_Regs* base = UART_Map_Soc_Base(index);
#endif
    if (base == NULL)
    {
        return hwUART_InvalidParameter;
    }

    TIMSPM0_UART_IT_State *state = &UART_IT_State[index];

    if (state->tx_busy)
    {
        return hwUART_Busy;
    }

    if (timeoutMs == NEONRT_WAIT_FOREVER)
    {
        return NEONRT_WAIT_FOREVER;
    }

    if ((UART_BaudRate[index] == 0U) || (size == 0U))
    {
        return (NeonRTOS_Time_t) timeoutMs;
    }

    uint64_t transfer_bits = (uint64_t) size * (uint64_t) UART_FrameBits[index];

    uint64_t transfer_ms = ((transfer_bits * 1000ULL) + UART_BaudRate[index] - 1ULL) / UART_BaudRate[index];

    uint64_t wait_ms = transfer_ms + timeoutMs + 1ULL;

    if (wait_ms >= UINT32_MAX) {
        wait_ms = UINT32_MAX - 1ULL;
    }

    NeonRTOS_SyncObjClear(&UART_Send_SyncHandle[index]);

    state->tx_buf = data_wr;
    state->tx_size = size;
    state->tx_count = 0U;
    state->tx_busy = true;

    while (!DL_UART_Main_isTXFIFOFull(base) && (state->tx_count < state->tx_size))
    {
        DL_UART_Main_transmitData(base, state->tx_buf[state->tx_count++]);
    }

    if (state->tx_count < state->tx_size)
    {
        DL_UART_Main_clearInterruptStatus(base, DL_UART_MAIN_INTERRUPT_TX);

        DL_UART_Main_enableInterrupt(base, DL_UART_MAIN_INTERRUPT_TX);

        if (NeonRTOS_SyncObjWait(&UART_Send_SyncHandle[index], wait_ms) != NeonRTOS_OK)
        {
            DL_UART_Main_disableInterrupt(base, DL_UART_MAIN_INTERRUPT_TX);

            state->tx_busy = false;

            if (state->tx_count > 0U) {
                return (hwUART_OpResult) state->tx_count;
            }

            return hwUART_Busy;
        }
    }

    NeonRTOS_Time_t elapsed = 0;

    while (DL_UART_Main_isBusy(base))
    {
        if ((wait_ms != NEONRT_WAIT_FOREVER) &&
            (elapsed >= wait_ms))
        {
            state->tx_busy = false;

            return hwUART_Busy;
        }

        NeonRTOS_Sleep(1);

        if (wait_ms != NEONRT_WAIT_FOREVER) {
            elapsed++;
        }
    }

    state->tx_busy = false;

    return (hwUART_OpResult) state->tx_count;
}

hwUART_OpResult UART_PutChar(hwUART_Index index, uint8_t char_wr, uint32_t timeoutMs)
{
    return UART_Write(index, &char_wr, 1U, timeoutMs);
}

void UART_Printf(const char *format, ...)
{
    if (!UART_Init_Status[LOG_UART_INDEX]) {
        return;
    }

    char buffer[128];
    va_list args;

    va_start(args, format);

    int len = vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    if (len <= 0) {
        return;
    }

    if (len >= (int) sizeof(buffer))
    {
        len = (int) sizeof(buffer) - 1;
    }

    (void) UART_Write(LOG_UART_INDEX, (uint8_t *) buffer, (size_t) len, 1000U);
}


bool UART_is_Init(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX) {
        return false;
    }

    return UART_Init_Status[index];
}

#endif // DEVICE_TIMSPM0
