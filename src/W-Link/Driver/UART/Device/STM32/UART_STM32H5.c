#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#include "UART/UART.h"

#ifdef STM32H5

#include "UART_STM32.h"

UART_HandleTypeDef g_uart[hwUART_Index_MAX];

USART_TypeDef *UART_Map_Soc_Base(hwUART_Index index)
{
    switch (index)
    {
#if defined(USART1_BASE)
        case hwUART_Index_0: return USART1;
#elif defined(UART1_BASE)
        case hwUART_Index_0: return UART1;
#endif

#if defined(USART2_BASE)
        case hwUART_Index_1: return USART2;
#elif defined(UART2_BASE)
        case hwUART_Index_1: return UART2;
#endif

#if defined(USART3_BASE)
        case hwUART_Index_2: return USART3;
#elif defined(UART3_BASE)
        case hwUART_Index_2: return UART3;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_3: return UART4;
#elif defined(USART4_BASE)
        case hwUART_Index_3: return USART4;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_4: return UART5;
#elif defined(USART5_BASE)
        case hwUART_Index_4: return USART5;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_5: return UART6;
#elif defined(USART6_BASE)
        case hwUART_Index_5: return USART6;
#endif

#if defined(USART7_BASE)
        case hwUART_Index_6: return USART7;
#elif defined(UART7_BASE)
        case hwUART_Index_6: return UART7;
#endif

#if defined(USART8_BASE)
        case hwUART_Index_7: return USART8;
#elif defined(UART8_BASE)
        case hwUART_Index_7: return UART8;
#endif

#if defined(USART9_BASE)
        case hwUART_Index_8: return USART9;
#elif defined(UART9_BASE)
        case hwUART_Index_8: return UART9;
#endif

#if defined(UART10_BASE)
        case hwUART_Index_9: return UART10;
#elif defined(USART10_BASE)
        case hwUART_Index_9: return USART10;
#endif

#if defined(UART11_BASE)
        case hwUART_Index_10: return UART11;
#elif defined(USART11_BASE)
        case hwUART_Index_10: return USART11;
#endif

#if defined(UART12_BASE)
        case hwUART_Index_11: return UART12;
#elif defined(USART12_BASE)
        case hwUART_Index_11: return USART12;
#endif

#if defined(LPUART1_BASE)
        case hwUART_Index_L1: return LPUART1;
#endif

        default:
            break;
    }

    return NULL;
}

static void UART_EnableClock(hwUART_Index index)
{
    switch (index)
    {
#if defined(USART1_BASE)
        case hwUART_Index_0: __HAL_RCC_USART1_CLK_ENABLE(); break;
#elif defined(UART1_BASE)
        case hwUART_Index_0: __HAL_RCC_UART1_CLK_ENABLE(); break;
#endif

#if defined(USART2_BASE)
        case hwUART_Index_1: __HAL_RCC_USART2_CLK_ENABLE(); break;
#elif defined(UART2_BASE)
        case hwUART_Index_1: __HAL_RCC_UART2_CLK_ENABLE(); break;
#endif

#if defined(USART3_BASE)
        case hwUART_Index_2: __HAL_RCC_USART3_CLK_ENABLE(); break;
#elif defined(UART3_BASE)
        case hwUART_Index_2: __HAL_RCC_UART3_CLK_ENABLE(); break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_3: __HAL_RCC_UART4_CLK_ENABLE(); break;
#elif defined(USART4_BASE)
        case hwUART_Index_3: __HAL_RCC_USART4_CLK_ENABLE(); break;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_4: __HAL_RCC_UART5_CLK_ENABLE(); break;
#elif defined(USART5_BASE)
        case hwUART_Index_4: __HAL_RCC_USART5_CLK_ENABLE(); break;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_5: __HAL_RCC_UART6_CLK_ENABLE(); break;
#elif defined(USART6_BASE)
        case hwUART_Index_5: __HAL_RCC_USART6_CLK_ENABLE(); break;
#endif

#if defined(USART7_BASE)
        case hwUART_Index_6: __HAL_RCC_USART7_CLK_ENABLE(); break;
#elif defined(UART7_BASE)
        case hwUART_Index_6: __HAL_RCC_UART7_CLK_ENABLE(); break;
#endif

#if defined(USART8_BASE)
        case hwUART_Index_7: __HAL_RCC_USART8_CLK_ENABLE(); break;
#elif defined(UART8_BASE)
        case hwUART_Index_7: __HAL_RCC_UART8_CLK_ENABLE(); break;
#endif

#if defined(USART9_BASE)
        case hwUART_Index_8: __HAL_RCC_USART9_CLK_ENABLE(); break;
#elif defined(UART9_BASE)
        case hwUART_Index_8: __HAL_RCC_UART9_CLK_ENABLE(); break;
#endif

#if defined(UART10_BASE)
        case hwUART_Index_9: __HAL_RCC_UART10_CLK_ENABLE(); break;
#elif defined(USART10_BASE)
        case hwUART_Index_9: __HAL_RCC_USART10_CLK_ENABLE(); break;
#endif

#if defined(UART11_BASE)
        case hwUART_Index_10: __HAL_RCC_UART11_CLK_ENABLE(); break;
#elif defined(USART11_BASE)
        case hwUART_Index_10: __HAL_RCC_USART11_CLK_ENABLE(); break;
#endif

#if defined(UART12_BASE)
        case hwUART_Index_11: __HAL_RCC_UART12_CLK_ENABLE(); break;
#elif defined(USART12_BASE)
        case hwUART_Index_11: __HAL_RCC_USART12_CLK_ENABLE(); break;
#endif

#if defined(LPUART1_BASE)
        case hwUART_Index_L1: __HAL_RCC_LPUART1_CLK_ENABLE(); break;
#endif

        default:
            break;
    }
}

static void UART_DisableClock(hwUART_Index index)
{
    switch (index)
    {
#if defined(USART1_BASE)
        case hwUART_Index_0: __HAL_RCC_USART1_CLK_DISABLE(); break;
#elif defined(UART1_BASE)
        case hwUART_Index_0: __HAL_RCC_UART1_CLK_DISABLE(); break;
#endif

#if defined(USART2_BASE)
        case hwUART_Index_1: __HAL_RCC_USART2_CLK_DISABLE(); break;
#elif defined(UART2_BASE)
        case hwUART_Index_1: __HAL_RCC_UART2_CLK_DISABLE(); break;
#endif

#if defined(USART3_BASE)
        case hwUART_Index_2: __HAL_RCC_USART3_CLK_DISABLE(); break;
#elif defined(UART3_BASE)
        case hwUART_Index_2: __HAL_RCC_UART3_CLK_DISABLE(); break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_3: __HAL_RCC_UART4_CLK_DISABLE(); break;
#elif defined(USART4_BASE)
        case hwUART_Index_3: __HAL_RCC_USART4_CLK_DISABLE(); break;
#endif

#if defined(USART5_BASE)
        case hwUART_Index_4: __HAL_RCC_USART5_CLK_DISABLE(); break;
#elif defined(UART5_BASE)
        case hwUART_Index_4: __HAL_RCC_UART5_CLK_DISABLE(); break;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_5: __HAL_RCC_UART6_CLK_DISABLE(); break;
#elif defined(USART6_BASE)
        case hwUART_Index_5: __HAL_RCC_USART6_CLK_DISABLE(); break;
#endif

#if defined(USART7_BASE)
        case hwUART_Index_6: __HAL_RCC_USART7_CLK_DISABLE(); break;
#elif defined(UART7_BASE)
        case hwUART_Index_6: __HAL_RCC_UART7_CLK_DISABLE(); break;
#endif

#if defined(USART8_BASE)
        case hwUART_Index_7: __HAL_RCC_USART8_CLK_DISABLE(); break;
#elif defined(UART8_BASE)
        case hwUART_Index_7: __HAL_RCC_UART8_CLK_DISABLE(); break;
#endif

#if defined(USART9_BASE)
        case hwUART_Index_8: __HAL_RCC_USART9_CLK_DISABLE(); break;
#elif defined(UART9_BASE)
        case hwUART_Index_8: __HAL_RCC_UART9_CLK_DISABLE(); break;
#endif

#if defined(UART10_BASE)
        case hwUART_Index_9: __HAL_RCC_UART10_CLK_DISABLE(); break;
#elif defined(USART10_BASE)
        case hwUART_Index_9: __HAL_RCC_USART10_CLK_DISABLE(); break;
#endif

#if defined(UART11_BASE)
        case hwUART_Index_10: __HAL_RCC_UART11_CLK_DISABLE(); break;
#elif defined(USART11_BASE)
        case hwUART_Index_10: __HAL_RCC_USART11_CLK_DISABLE(); break;
#endif

#if defined(UART12_BASE)
        case hwUART_Index_11: __HAL_RCC_UART12_CLK_DISABLE(); break;
#elif defined(USART12_BASE)
        case hwUART_Index_11: __HAL_RCC_USART12_CLK_DISABLE(); break;
#endif

#if defined(LPUART1_BASE)
        case hwUART_Index_L1: __HAL_RCC_LPUART1_CLK_DISABLE(); break;
#endif

        default:
            break;
    }
}

hwUART_OpResult UART_Instance_Init(
    hwUART_Index index,
    uint32_t baudrate,
    bool rts_cts,
    uint8_t data_bits,
    UART_Parity parity,
    uint8_t stop_bits
)
{
    if (index >= hwUART_Index_MAX)
        return hwUART_InvalidParameter;

    USART_TypeDef *base = UART_Map_Soc_Base(index);
    if (base == NULL)
        return hwUART_InvalidParameter;

    UART_EnableClock(index);

    UART_HandleTypeDef *huart = &g_uart[index];
    memset(huart, 0, sizeof(*huart));

    huart->Instance      = base;
    huart->Init.BaudRate = baudrate;

    switch (data_bits)
    {
#if defined(UART_WORDLENGTH_7B)
        case 7:
            huart->Init.WordLength = UART_WORDLENGTH_7B;
            break;
#endif
        case 8:
            huart->Init.WordLength = UART_WORDLENGTH_8B;
            break;

        case 9:
            huart->Init.WordLength = UART_WORDLENGTH_9B;
            break;

        default:
            return hwUART_InvalidParameter;
    }

    huart->Init.StopBits = (stop_bits == 2) ? UART_STOPBITS_2 : UART_STOPBITS_1;

    switch (parity)
    {
        case UART_Parity_None:
            huart->Init.Parity = UART_PARITY_NONE;
            break;

        case UART_Parity_Even:
            huart->Init.Parity = UART_PARITY_EVEN;
            break;

        case UART_Parity_Odd:
            huart->Init.Parity = UART_PARITY_ODD;
            break;

        default:
            return hwUART_InvalidParameter;
    }

    huart->Init.Mode = UART_MODE_TX_RX;

#if defined(UART_HWCONTROL_RTS_CTS)
    huart->Init.HwFlowCtl = rts_cts ? UART_HWCONTROL_RTS_CTS : UART_HWCONTROL_NONE;
#else
    if (rts_cts)
        return hwUART_InvalidParameter;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
#endif

#if defined(UART_OVERSAMPLING_16)
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
#endif

#if defined(UART_ONE_BIT_SAMPLE_DISABLE)
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
#endif

#if defined(UART_ADVFEATURE_NO_INIT)
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
#endif

    if (HAL_UART_Init(huart) != HAL_OK)
        return hwUART_HwError;

    return hwUART_OK;
}

hwUART_OpResult UART_Instance_DeInit(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX)
        return hwUART_InvalidParameter;

    if (UART_Map_Soc_Base(index) == NULL)
        return hwUART_InvalidParameter;

    if (HAL_UART_DeInit(&g_uart[index]) != HAL_OK)
        return hwUART_HwError;

    UART_DisableClock(index);

    return hwUART_OK;
}

static void UART_HAL_IRQHandler(hwUART_Index index)
{
    if (index < hwUART_Index_MAX)
        HAL_UART_IRQHandler(&g_uart[index]);
}

/* ================= IRQ Handler ================= */

#if defined(USART1_BASE)
void USART1_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_0);
}
#elif defined(UART1_BASE)
void UART1_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_0);
}
#endif

#if defined(USART2_BASE)
void USART2_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_1);
}
#elif defined(UART2_BASE)
void UART2_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_1);
}
#endif

#if defined(UART3_BASE)
void UART3_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_2);
}
#elif defined(USART3_BASE)
void USART3_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_2);
}
#endif

#if defined(UART4_BASE)
void UART4_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_3);
}
#elif defined(USART4_BASE)
void USART4_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_3);
}
#endif

#if defined(UART5_BASE)
void UART5_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_4);
}
#elif defined(USART5_BASE)
void USART5_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_4);
}
#endif

#if defined(UART6_BASE)
void UART6_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_5);
}
#elif defined(USART6_BASE)
void USART6_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_5);
}
#endif

#if defined(UART7_BASE)
void UART7_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_6);
}
#elif defined(USART7_BASE)
void USART7_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_6);
}
#endif

#if defined(UART8_BASE)
void UART8_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_7);
}
#elif defined(USART8_BASE)
void USART8_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_7);
}
#endif

#if defined(UART9_BASE)
void UART9_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_8);
}
#elif defined(USART9_BASE)
void USART9_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_8);
}
#endif

#if defined(UART10_BASE)
void UART10_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_9);
}
#elif defined(USART10_BASE)
void USART10_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_9);
}
#endif

#if defined(UART11_BASE)
void UART11_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_10);
}
#elif defined(USART11_BASE)
void USART11_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_10);
}
#endif

#if defined(UART12_BASE)
void UART12_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_11);
}
#elif defined(USART12_BASE)
void USART12_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_11);
}
#endif

#if defined(LPUART1_BASE)
void LPUART1_IRQHandler(void)
{
    UART_HAL_IRQHandler(hwUART_Index_L1);
}
#endif

/* ================= NVIC ================= */

void UART_NVIC_Init(hwUART_Index index)
{
    switch (index)
    {
#if defined(USART1_BASE)
        case hwUART_Index_0:
            HAL_NVIC_SetPriority(USART1_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART1_IRQn);
            break;
#elif defined(UART1_BASE)
        case hwUART_Index_0:
            HAL_NVIC_SetPriority(UART1_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART1_IRQn);
            break;
#endif

#if defined(USART2_BASE)
        case hwUART_Index_1:
            HAL_NVIC_SetPriority(USART2_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART2_IRQn);
            break;
#elif defined(UART2_BASE)
        case hwUART_Index_1:
            HAL_NVIC_SetPriority(UART2_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART2_IRQn);
            break;
#endif

#if defined(USART3_BASE)
        case hwUART_Index_2:
            HAL_NVIC_SetPriority(USART3_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART3_IRQn);
            break;
#elif defined(UART3_BASE)
        case hwUART_Index_2:
            HAL_NVIC_SetPriority(UART3_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART3_IRQn);
            break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_3:
            HAL_NVIC_SetPriority(UART4_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART4_IRQn);
            break;
#elif defined(USART4_BASE)
        case hwUART_Index_3:
            HAL_NVIC_SetPriority(USART4_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART4_IRQn);
            break;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_4:
            HAL_NVIC_SetPriority(UART5_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART5_IRQn);
            break;
#elif defined(USART5_BASE)
        case hwUART_Index_4:
            HAL_NVIC_SetPriority(USART5_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART5_IRQn);
            break;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_5:
            HAL_NVIC_SetPriority(UART6_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART6_IRQn);
            break;
#elif defined(USART6_BASE)
        case hwUART_Index_5:
            HAL_NVIC_SetPriority(USART6_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART6_IRQn);
            break;
#endif

#if defined(USART7_BASE)
        case hwUART_Index_6:
            HAL_NVIC_SetPriority(USART7_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART7_IRQn);
            break;
#elif defined(UART7_BASE)
        case hwUART_Index_6:
            HAL_NVIC_SetPriority(UART7_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART7_IRQn);
            break;
#endif

#if defined(USART8_BASE)
        case hwUART_Index_7:
            HAL_NVIC_SetPriority(USART8_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART8_IRQn);
            break;
#elif defined(UART8_BASE)
        case hwUART_Index_7:
            HAL_NVIC_SetPriority(UART8_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART8_IRQn);
            break;
#endif

#if defined(USART9_BASE)
        case hwUART_Index_8:
            HAL_NVIC_SetPriority(USART9_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART9_IRQn);
            break;
#elif defined(UART9_BASE)
        case hwUART_Index_8:
            HAL_NVIC_SetPriority(UART9_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART9_IRQn);
            break;
#endif

#if defined(UART10_BASE)
        case hwUART_Index_9:
            HAL_NVIC_SetPriority(UART10_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART10_IRQn);
            break;
#elif defined(USART4_BASE)
        case hwUART_Index_9:
            HAL_NVIC_SetPriority(USART10_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART10_IRQn);
            break;
#endif

#if defined(UART11_BASE)
        case hwUART_Index_10:
            HAL_NVIC_SetPriority(UART11_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART11_IRQn);
            break;
#elif defined(USART5_BASE)
        case hwUART_Index_10:
            HAL_NVIC_SetPriority(USART11_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART11_IRQn);
            break;
#endif

#if defined(UART12_BASE)
        case hwUART_Index_11:
            HAL_NVIC_SetPriority(UART12_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART12_IRQn);
            break;
#elif defined(USART12_BASE)
        case hwUART_Index_11:
            HAL_NVIC_SetPriority(USART12_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART12_IRQn);
            break;
#endif

#if defined(LPUART1_BASE)
        case hwUART_Index_L1:
            HAL_NVIC_SetPriority(LPUART1_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(LPUART1_IRQn);
            break;
#endif

        default:
            break;
    }
}

void UART_NVIC_DeInit(hwUART_Index index)
{
    switch (index)
    {
#if defined(USART1_BASE)
        case hwUART_Index_0:
            HAL_NVIC_DisableIRQ(USART1_IRQn);
            break;
#elif defined(UART1_BASE)
        case hwUART_Index_0:
            HAL_NVIC_DisableIRQ(UART1_IRQn);
            break;
#endif

#if defined(USART2_BASE)
        case hwUART_Index_1:
            HAL_NVIC_DisableIRQ(USART2_IRQn);
            break;
#elif defined(UART2_BASE)
        case hwUART_Index_1:
            HAL_NVIC_DisableIRQ(UART2_IRQn);
            break;
#endif

#if defined(USART3_BASE)
        case hwUART_Index_2:
            HAL_NVIC_DisableIRQ(USART3_IRQn);
            break;
#elif defined(UART3_BASE)
        case hwUART_Index_2:
            HAL_NVIC_DisableIRQ(UART3_IRQn);
            break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_3:
            HAL_NVIC_DisableIRQ(UART4_IRQn);
            break;
#elif defined(USART4_BASE)
        case hwUART_Index_3:
            HAL_NVIC_DisableIRQ(USART4_IRQn);
            break;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_4:
            HAL_NVIC_DisableIRQ(UART5_IRQn);
            break;
#elif defined(USART5_BASE)
        case hwUART_Index_4:
            HAL_NVIC_DisableIRQ(USART5_IRQn);
            break;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_5:
            HAL_NVIC_DisableIRQ(UART6_IRQn);
            break;
#elif defined(USART6_BASE)
        case hwUART_Index_5:
            HAL_NVIC_DisableIRQ(USART6_IRQn);
            break;
#endif

#if defined(USART7_BASE)
        case hwUART_Index_6:
            HAL_NVIC_DisableIRQ(USART7_IRQn);
            break;
#elif defined(UART7_BASE)
        case hwUART_Index_6:
            HAL_NVIC_DisableIRQ(UART7_IRQn);
            break;
#endif

#if defined(USART8_BASE)
        case hwUART_Index_7:
            HAL_NVIC_DisableIRQ(USART8_IRQn);
            break;
#elif defined(UART8_BASE)
        case hwUART_Index_7:
            HAL_NVIC_DisableIRQ(UART8_IRQn);
            break;
#endif

#if defined(USART9_BASE)
        case hwUART_Index_8:
            HAL_NVIC_DisableIRQ(USART9_IRQn);
            break;
#elif defined(UART9_BASE)
        case hwUART_Index_8:
            HAL_NVIC_DisableIRQ(UART9_IRQn);
            break;
#endif

#if defined(UART10_BASE)
        case hwUART_Index_9:
            HAL_NVIC_DisableIRQ(UART10_IRQn);
            break;
#elif defined(USART4_BASE)
        case hwUART_Index_9:
            HAL_NVIC_DisableIRQ(USART10_IRQn);
            break;
#endif

#if defined(UART11_BASE)
        case hwUART_Index_10:
            HAL_NVIC_DisableIRQ(UART11_IRQn);
            break;
#elif defined(USART5_BASE)
        case hwUART_Index_10:
            HAL_NVIC_DisableIRQ(USART11_IRQn);
            break;
#endif

#if defined(UART12_BASE)
        case hwUART_Index_11:
            HAL_NVIC_DisableIRQ(UART12_IRQn);
            break;
#elif defined(USART12_BASE)
        case hwUART_Index_11:
            HAL_NVIC_DisableIRQ(USART12_IRQn);
            break;
#endif

#if defined(LPUART1_BASE)
        case hwUART_Index_L1:
            HAL_NVIC_DisableIRQ(LPUART1_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif // STM32H5