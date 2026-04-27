
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#ifdef STM32F3

#include "UART/UART.h"
#include "UART_STM32.h"

UART_HandleTypeDef g_uart[hwUART_Index_MAX];

USART_TypeDef *UART_Map_Soc_Base(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART1_BASE)
        case hwUART_Index_0: return UART1;
#endif
#if defined(USART1_BASE)
        case hwUART_Index_0: return USART1;
#endif

#if defined(UART2_BASE)
        case hwUART_Index_1: return UART2;
#endif
#if defined(USART2_BASE)
        case hwUART_Index_1: return USART2;
#endif

#if defined(UART3_BASE)
        case hwUART_Index_2: return UART3;
#endif
#if defined(USART3_BASE)
        case hwUART_Index_2: return USART3;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_3: return UART4;
#endif
#if defined(USART4_BASE)
        case hwUART_Index_3: return USART4;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_4: return UART5;
#endif
#if defined(USART5_BASE)
        case hwUART_Index_4: return USART5;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_5: return UART6;
#endif
#if defined(USART6_BASE)
        case hwUART_Index_5: return USART6;
#endif

        default: return NULL;
    }
}

static void UART_EnableClock(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART1_BASE)
        case hwUART_Index_0: __HAL_RCC_UART1_CLK_ENABLE(); break;
#endif
#if defined(USART1_BASE)
        case hwUART_Index_0: __HAL_RCC_USART1_CLK_ENABLE(); break;
#endif
#if defined(UART2_BASE)
        case hwUART_Index_1: __HAL_RCC_UART2_CLK_ENABLE(); break;
#endif
#if defined(USART2_BASE)
        case hwUART_Index_1: __HAL_RCC_USART2_CLK_ENABLE(); break;
#endif
#if defined(UART3_BASE)
        case hwUART_Index_2: __HAL_RCC_UART3_CLK_ENABLE(); break;
#endif
#if defined(USART3_BASE)
        case hwUART_Index_2: __HAL_RCC_USART3_CLK_ENABLE(); break;
#endif
#if defined(UART4_BASE)
        case hwUART_Index_3: __HAL_RCC_UART4_CLK_ENABLE(); break;
#endif
#if defined(USART4_BASE)
        case hwUART_Index_3: __HAL_RCC_USART4_CLK_ENABLE(); break;
#endif
#if defined(UART5_BASE)
        case hwUART_Index_4: __HAL_RCC_UART5_CLK_ENABLE(); break;
#endif
#if defined(USART5_BASE)
        case hwUART_Index_4: __HAL_RCC_USART5_CLK_ENABLE(); break;
#endif
#if defined(UART6_BASE)
        case hwUART_Index_5: __HAL_RCC_UART6_CLK_ENABLE(); break;
#endif
#if defined(USART6_BASE)
        case hwUART_Index_5: __HAL_RCC_USART6_CLK_ENABLE(); break;
#endif
        default: break;
    }
}

static void UART_DisableClock(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART1_BASE)
        case hwUART_Index_0: __HAL_RCC_UART1_CLK_DISABLE(); break;
#endif
#if defined(USART1_BASE)
        case hwUART_Index_0: __HAL_RCC_USART1_CLK_DISABLE(); break;
#endif
#if defined(UART2_BASE)
        case hwUART_Index_1: __HAL_RCC_UART2_CLK_DISABLE(); break;
#endif
#if defined(USART2_BASE)
        case hwUART_Index_1: __HAL_RCC_USART2_CLK_DISABLE(); break;
#endif
#if defined(UART3_BASE)
        case hwUART_Index_2: __HAL_RCC_UART3_CLK_DISABLE(); break;
#endif
#if defined(USART3_BASE)
        case hwUART_Index_2: __HAL_RCC_USART3_CLK_DISABLE(); break;
#endif
#if defined(UART4_BASE)
        case hwUART_Index_3: __HAL_RCC_UART4_CLK_DISABLE(); break;
#endif
#if defined(USART4_BASE)
        case hwUART_Index_3: __HAL_RCC_USART4_CLK_DISABLE(); break;
#endif
#if defined(UART5_BASE)
        case hwUART_Index_4: __HAL_RCC_UART5_CLK_DISABLE(); break;
#endif
#if defined(USART5_BASE)
        case hwUART_Index_4: __HAL_RCC_USART5_CLK_DISABLE(); break;
#endif
#if defined(UART6_BASE)
        case hwUART_Index_5: __HAL_RCC_UART6_CLK_DISABLE(); break;
#endif
#if defined(USART6_BASE)
        case hwUART_Index_5: __HAL_RCC_USART6_CLK_DISABLE(); break;
#endif
        default: break;
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
    USART_TypeDef *base = UART_Map_Soc_Base(index);

    if (base == NULL)
        return hwUART_InvalidParameter;

    UART_EnableClock(index);

    UART_HandleTypeDef *huart = &g_uart[index];
    memset(huart, 0, sizeof(*huart));

    huart->Instance = base;
    huart->Init.BaudRate = baudrate * 3;

    switch (data_bits)
    {
#ifdef UART_WORDLENGTH_7B
        case 7: huart->Init.WordLength = UART_WORDLENGTH_7B; break;
#endif
        case 8: huart->Init.WordLength = UART_WORDLENGTH_8B; break;
        case 9: huart->Init.WordLength = UART_WORDLENGTH_9B; break;
        default: return hwUART_InvalidParameter;
    }

    huart->Init.StopBits = (stop_bits == 2) ? UART_STOPBITS_2 : UART_STOPBITS_1;

    switch (parity)
    {
        case UART_Parity_None: huart->Init.Parity = UART_PARITY_NONE; break;
        case UART_Parity_Even: huart->Init.Parity = UART_PARITY_EVEN; break;
        case UART_Parity_Odd:  huart->Init.Parity = UART_PARITY_ODD;  break;
        default: return hwUART_InvalidParameter;
    }

    huart->Init.Mode       = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl  = rts_cts ? UART_HWCONTROL_RTS_CTS : UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(huart) != HAL_OK)
        return hwUART_HwError;

    return hwUART_OK;
}

hwUART_OpResult UART_Instance_DeInit(hwUART_Index index)
{
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

#if defined(UART1_BASE)
void UART1_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_0); }
#endif
#if defined(USART1_BASE)
void USART1_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_0); }
#endif

#if defined(UART2_BASE)
void UART2_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_1); }
#endif
#if defined(USART2_BASE)
void USART2_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_1); }
#endif

#if defined(UART3_BASE)
void UART3_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_2); }
#endif
#if defined(USART3_BASE)
void USART3_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_2); }
#endif

#if defined(UART4_BASE)
void UART4_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_3); }
#endif
#if defined(USART4_BASE)
void USART4_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_3); }
#endif

#if defined(UART5_BASE)
void UART5_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_4); }
#endif
#if defined(USART5_BASE)
void USART5_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_4); }
#endif

#if defined(UART6_BASE)
void UART6_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_5); }
#endif
#if defined(USART6_BASE)
void USART6_IRQHandler(void) { UART_HAL_IRQHandler(hwUART_Index_5); }
#endif

void UART_NVIC_Init(hwUART_Index index)
{
    switch (index)
    {
#if defined(UART1_BASE)
        case hwUART_Index_0:
            HAL_NVIC_SetPriority(UART1_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART1_IRQn);
            break;
#endif
#if defined(USART1_BASE)
        case hwUART_Index_0:
            HAL_NVIC_SetPriority(USART1_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART1_IRQn);
            break;
#endif

#if defined(UART2_BASE)
        case hwUART_Index_1:
            HAL_NVIC_SetPriority(UART2_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART2_IRQn);
            break;
#endif
#if defined(USART2_BASE)
        case hwUART_Index_1:
            HAL_NVIC_SetPriority(USART2_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART2_IRQn);
            break;
#endif

#if defined(UART3_BASE)
        case hwUART_Index_2:
            HAL_NVIC_SetPriority(UART3_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART3_IRQn);
            break;
#endif
#if defined(USART3_BASE)
        case hwUART_Index_2:
            HAL_NVIC_SetPriority(USART3_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART3_IRQn);
            break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_3:
            HAL_NVIC_SetPriority(UART4_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(UART4_IRQn);
            break;
#endif
#if defined(USART4_BASE)
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
#endif
#if defined(USART5_BASE)
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
#endif
#if defined(USART6_BASE)
        case hwUART_Index_5:
            HAL_NVIC_SetPriority(USART6_IRQn, UART_IRQ_NVIC_PRIORITY, UART_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(USART6_IRQn);
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
#if defined(UART1_BASE)
        case hwUART_Index_0: HAL_NVIC_DisableIRQ(UART1_IRQn); break;
#endif
#if defined(USART1_BASE)
        case hwUART_Index_0: HAL_NVIC_DisableIRQ(USART1_IRQn); break;
#endif

#if defined(UART2_BASE)
        case hwUART_Index_1: HAL_NVIC_DisableIRQ(UART2_IRQn); break;
#endif
#if defined(USART2_BASE)
        case hwUART_Index_1: HAL_NVIC_DisableIRQ(USART2_IRQn); break;
#endif

#if defined(UART3_BASE)
        case hwUART_Index_2: HAL_NVIC_DisableIRQ(UART3_IRQn); break;
#endif
#if defined(USART3_BASE)
        case hwUART_Index_2: HAL_NVIC_DisableIRQ(USART3_IRQn); break;
#endif

#if defined(UART4_BASE)
        case hwUART_Index_3: HAL_NVIC_DisableIRQ(UART4_IRQn); break;
#endif
#if defined(USART4_BASE)
        case hwUART_Index_3: HAL_NVIC_DisableIRQ(USART4_IRQn); break;
#endif

#if defined(UART5_BASE)
        case hwUART_Index_4: HAL_NVIC_DisableIRQ(UART5_IRQn); break;
#endif
#if defined(USART5_BASE)
        case hwUART_Index_4: HAL_NVIC_DisableIRQ(USART5_IRQn); break;
#endif

#if defined(UART6_BASE)
        case hwUART_Index_5: HAL_NVIC_DisableIRQ(UART6_IRQn); break;
#endif
#if defined(USART6_BASE)
        case hwUART_Index_5: HAL_NVIC_DisableIRQ(USART6_IRQn); break;
#endif

        default:
            break;
    }
}

#endif // STM32F3