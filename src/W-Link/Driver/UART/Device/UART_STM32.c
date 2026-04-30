#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "UART/UART.h"

#ifdef DEVICE_STM32

#include "UART_STM32.h"
#include "UART/UART_Pin.h"

#include "GPIO/Device/GPIO_STM32.h"

bool UART_Init_Status[hwUART_Index_MAX] = {false};

static int  UART_BaudRate[hwUART_Index_MAX] = {0};
static bool UART_FlowControl[hwUART_Index_MAX] = {false};

static NeonRTOS_SyncObj_t UART_Send_SyncHandle[hwUART_Index_MAX];
static NeonRTOS_SyncObj_t UART_Recv_SyncHandle[hwUART_Index_MAX];

#ifndef STM32F1
uint32_t STM32_UART_GetAF(hwUART_Index uart, hwGPIO_Pin pin)
{
    for (size_t i = 0; i < sizeof(UART_Pin_AF_Map) / sizeof(UART_Pin_AF_Map[0]); i++) {
        if (UART_Pin_AF_Map[i].uart == uart &&
            UART_Pin_AF_Map[i].pin  == pin) {
            return UART_Pin_AF_Map[i].af;
        }
    }

    return 0;
}
#endif

hwUART_Index UART_IndexFromHandle(UART_HandleTypeDef *huart)
{
    for (hwUART_Index i = 0; i < hwUART_Index_MAX; i++)
    {
        if (&g_uart[i] == huart)
            return i;
    }

    return hwUART_Index_MAX;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    hwUART_Index idx = UART_IndexFromHandle(huart);
    if (idx >= hwUART_Index_MAX) {
        return;
    }

    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_RXNE);

    huart->RxState = HAL_UART_STATE_READY;

    NeonRTOS_SyncObjSignalFromISR(&UART_Recv_SyncHandle[idx]);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    hwUART_Index idx = UART_IndexFromHandle(huart);
    if (idx >= hwUART_Index_MAX) {
        return;
    }

    NeonRTOS_SyncObjSignalFromISR(&UART_Send_SyncHandle[idx]);
}

hwUART_OpResult UART_Open(hwUART_Index index, uint32_t baudrate, bool rts_cts)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (rts_cts) {
        if (UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].rts_pin == hwGPIO_Pin_NC ||
            UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].cts_pin == hwGPIO_Pin_NC) {
            return hwUART_Unsupport;
        }
    }

    return UART_Open_Specific_Format(index, baudrate, rts_cts, 8, UART_Parity_None, 1);
}

hwUART_OpResult UART_Open_Specific_Format(
    hwUART_Index index,
    uint32_t baudrate,
    bool rts_cts,
    uint8_t data_bits,
    UART_Parity parity,
    uint8_t stop_bits
)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (parity >= UART_Parity_MAX) {
        return hwUART_InvalidParameter;
    }

    if (stop_bits != 1 && stop_bits != 2) {
        return hwUART_InvalidParameter;
    }

#ifdef UART_WORDLENGTH_7B
    if (data_bits != 7 && data_bits != 8 && data_bits != 9)
#else
    if (data_bits != 8 && data_bits != 9)
#endif
    {
        return hwUART_InvalidParameter;
    }

    if (UART_Init_Status[index]) {
        return hwUART_OK;
    }

    if (rts_cts) {
        if (UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].rts_pin == hwGPIO_Pin_NC ||
            UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].cts_pin == hwGPIO_Pin_NC) {
            return hwUART_Unsupport;
        }
    }

    if (NeonRTOS_SyncObjCreate(&UART_Send_SyncHandle[index]) != NeonRTOS_OK) {
        return hwUART_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&UART_Recv_SyncHandle[index]) != NeonRTOS_OK) {
        NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
        return hwUART_MemoryError;
    }

    UART_BaudRate[index] = baudrate;

    hwGPIO_Pin tx_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].tx_pin;
    hwGPIO_Pin rx_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].rx_pin;
    hwGPIO_Pin rts_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].rts_pin;
    hwGPIO_Pin cts_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].cts_pin;

    GPIO_TypeDef *tx_soc_base = GPIO_Map_Soc_Base(tx_pin);
    GPIO_TypeDef *rx_soc_base = GPIO_Map_Soc_Base(rx_pin);
    uint16_t tx_soc_pin = GPIO_Map_Soc_Pin(tx_pin);
    uint16_t rx_soc_pin = GPIO_Map_Soc_Pin(rx_pin);

    GPIO_TypeDef *rts_soc_base = NULL;
    GPIO_TypeDef *cts_soc_base = NULL;
    uint16_t rts_soc_pin = 0;
    uint16_t cts_soc_pin = 0;

    if (tx_soc_pin == 0 || tx_soc_base == NULL ||
        rx_soc_pin == 0 || rx_soc_base == NULL) {
        NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);
        return hwGPIO_InvalidParameter;
    }

    if (rts_cts) {
        rts_soc_base = GPIO_Map_Soc_Base(rts_pin);
        cts_soc_base = GPIO_Map_Soc_Base(cts_pin);
        rts_soc_pin = GPIO_Map_Soc_Pin(rts_pin);
        cts_soc_pin = GPIO_Map_Soc_Pin(cts_pin);

        if (rts_soc_pin == 0 || rts_soc_base == NULL ||
            cts_soc_pin == 0 || cts_soc_base == NULL) {
            NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
            NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);
            return hwGPIO_InvalidParameter;
        }
    }

#ifndef STM32F1
    uint32_t tx_af = STM32_UART_GetAF(index, tx_pin);
    uint32_t rx_af = STM32_UART_GetAF(index, rx_pin);
    uint32_t rts_af = 0;
    uint32_t cts_af = 0;

    if (tx_af == 0 || rx_af == 0) {
        NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);
        return hwGPIO_InvalidParameter;
    }

    if (rts_cts) {
        rts_af = STM32_UART_GetAF(index, rts_pin);
        cts_af = STM32_UART_GetAF(index, cts_pin);

        if (rts_af == 0 || cts_af == 0) {
            NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
            NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);
            return hwGPIO_InvalidParameter;
        }
    }
#endif

    USART_TypeDef *uart_soc_base = UART_Map_Soc_Base(index);
    if (uart_soc_base == NULL) {
        NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);
        return hwGPIO_InvalidParameter;
    }

    GPIO_Enable_RCC_Clock(tx_soc_base);
    GPIO_Enable_RCC_Clock(rx_soc_base);

    if (rts_cts) {
        GPIO_Enable_RCC_Clock(rts_soc_base);
        GPIO_Enable_RCC_Clock(cts_soc_base);
    }

    GPIO_InitTypeDef g_uart_tx = {0};
    g_uart_tx.Pin       = tx_soc_pin;
#ifdef STM32F1
    g_uart_tx.Mode      = GPIO_MODE_AF_PP;
    g_uart_tx.Pull      = GPIO_NOPULL;   // 或 GPIO_NOPULL
#else
    g_uart_tx.Mode      = GPIO_MODE_AF_PP;
    g_uart_tx.Pull      = GPIO_NOPULL;
#endif
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_uart_tx.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_uart_tx.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_uart_tx.Alternate = tx_af;
#endif
    HAL_GPIO_Init(tx_soc_base, &g_uart_tx);

    GPIO_InitTypeDef g_uart_rx = {0};
    g_uart_rx.Pin       = rx_soc_pin;
#ifdef STM32F1
    g_uart_rx.Mode      = GPIO_MODE_AF_INPUT;
    g_uart_rx.Pull      = GPIO_PULLUP;   // 或 GPIO_NOPULL
#else
    g_uart_rx.Mode      = GPIO_MODE_AF_PP;
    g_uart_rx.Pull      = GPIO_NOPULL;
#endif
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_uart_rx.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_uart_rx.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_uart_rx.Alternate = rx_af;
#endif
    HAL_GPIO_Init(rx_soc_base, &g_uart_rx);

    if (rts_cts) {
        GPIO_InitTypeDef g_uart_rts = {0};
        g_uart_rts.Pin       = rts_soc_pin;
        g_uart_rts.Mode      = GPIO_MODE_AF_PP;
        g_uart_rts.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
        g_uart_rts.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
        g_uart_rts.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
        g_uart_rts.Alternate = rts_af;
#endif
        HAL_GPIO_Init(rts_soc_base, &g_uart_rts);

        GPIO_InitTypeDef g_uart_cts = {0};
        g_uart_cts.Pin       = cts_soc_pin;
        g_uart_cts.Mode      = GPIO_MODE_AF_PP;
        g_uart_cts.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
        g_uart_cts.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
        g_uart_cts.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
        g_uart_cts.Alternate = cts_af;
#endif
        HAL_GPIO_Init(cts_soc_base, &g_uart_cts);
    }

    hwUART_OpResult result;

#ifdef STM32F1
    result = UART_ApplyRemap(
        index,
        tx_pin,
        rx_pin,
        rts_pin,
        cts_pin,
        rts_cts
    );

    if (result != hwUART_OK) {
        NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);
        return result;
    }
#endif

    result = UART_Instance_Init(index,
                                baudrate,
                                rts_cts,
                                data_bits,
                                parity,
                                stop_bits);
    if (result != hwUART_OK)
    {
        NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);
        return result;
    }

    UART_NVIC_Init(index);

    UART_FlowControl[index] = rts_cts;
    UART_Init_Status[index] = true;

    return hwUART_OK;
}

hwUART_OpResult UART_Close(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_OK;
    }

    hwGPIO_Pin tx_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].tx_pin;
    hwGPIO_Pin rx_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].rx_pin;
    hwGPIO_Pin rts_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].rts_pin;
    hwGPIO_Pin cts_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].cts_pin;

    GPIO_TypeDef *tx_soc_base = GPIO_Map_Soc_Base(tx_pin);
    GPIO_TypeDef *rx_soc_base = GPIO_Map_Soc_Base(rx_pin);
    uint16_t tx_soc_pin = GPIO_Map_Soc_Pin(tx_pin);
    uint16_t rx_soc_pin = GPIO_Map_Soc_Pin(rx_pin);

    GPIO_TypeDef *rts_soc_base = NULL;
    GPIO_TypeDef *cts_soc_base = NULL;
    uint16_t rts_soc_pin = 0;
    uint16_t cts_soc_pin = 0;

    if (tx_soc_pin == 0 || tx_soc_base == NULL ||
        rx_soc_pin == 0 || rx_soc_base == NULL) {
        return hwGPIO_InvalidParameter;
    }

    if (UART_FlowControl[index]) {
        rts_soc_base = GPIO_Map_Soc_Base(rts_pin);
        cts_soc_base = GPIO_Map_Soc_Base(cts_pin);
        rts_soc_pin = GPIO_Map_Soc_Pin(rts_pin);
        cts_soc_pin = GPIO_Map_Soc_Pin(cts_pin);

        if (rts_soc_pin == 0 || rts_soc_base == NULL ||
            cts_soc_pin == 0 || cts_soc_base == NULL) {
            return hwGPIO_InvalidParameter;
        }
    }
    
    UART_Init_Status[index] = false;

    UART_NVIC_DeInit(index);

    hwUART_OpResult result = UART_Instance_DeInit(index);
    if (result != hwUART_OK)
        return result;

#ifdef STM32F1
    UART_RestoreRemap(index);
#endif

    HAL_GPIO_DeInit(tx_soc_base, tx_soc_pin);
    HAL_GPIO_DeInit(rx_soc_base, rx_soc_pin);

    if (UART_FlowControl[index]) {
        HAL_GPIO_DeInit(rts_soc_base, rts_soc_pin);
        HAL_GPIO_DeInit(cts_soc_base, cts_soc_pin);
    }

    NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);

    gpio_pin_init_status[tx_pin] = false;
    gpio_pin_init_status[rx_pin] = false;

    if (UART_FlowControl[index]) {
        gpio_pin_init_status[rts_pin] = false;
        gpio_pin_init_status[cts_pin] = false;
        UART_FlowControl[index] = false;
    }

    UART_BaudRate[index] = 0;

    return hwUART_OK;
}

hwUART_OpResult UART_Read(hwUART_Index index, uint8_t *data_rd, size_t size, uint32_t timeoutMs)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (!data_rd || size == 0) {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_NotInit;
    }

    float wait_ms_f = ((float)size) / ((float)UART_BaudRate[index] / 8.0f / 1000.0f);
    NeonRTOS_Time_t wait_ms;

    if (timeoutMs == NEONRT_WAIT_FOREVER) {
        wait_ms = NEONRT_WAIT_FOREVER;
    } else if (timeoutMs == NEONRT_NO_WAIT) {
        wait_ms = NEONRT_NO_WAIT;
    } else {
        wait_ms = ((int)wait_ms_f) + timeoutMs + 1;
    }

    UART_HandleTypeDef *huart = &g_uart[index];

    uint16_t recv_bytes = 0;

    for (recv_bytes = 0; recv_bytes < size; recv_bytes++) {
        NeonRTOS_SyncObjClear(&UART_Recv_SyncHandle[index]);

        huart->RxState = HAL_UART_STATE_READY;

        if (HAL_UART_Receive_IT(huart, &data_rd[recv_bytes], 1) != HAL_OK) {
            if (recv_bytes > 0) {
                return (hwUART_OpResult)recv_bytes;
            }
            return hwUART_Busy;
        }

        if (NeonRTOS_SyncObjWait(&UART_Recv_SyncHandle[index], wait_ms) != NeonRTOS_OK) {
            if (recv_bytes > 0) {
                return (hwUART_OpResult)recv_bytes;
            }
            return hwUART_Busy;
        }
    }

    return (hwUART_OpResult)recv_bytes;
}

hwUART_OpResult UART_GetChar(hwUART_Index index, uint8_t *char_rd, uint32_t timeoutMs)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (!char_rd) {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_NotInit;
    }

    return UART_Read(index, char_rd, 1, timeoutMs);
}

hwUART_OpResult UART_Write(hwUART_Index index, uint8_t *data_wr, size_t size, uint32_t timeoutMs)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (!data_wr || size == 0) {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_NotInit;
    }

    float wait_ms_f = ((float)size) / ((float)UART_BaudRate[index] / 8.0f / 1000.0f);
    NeonRTOS_Time_t wait_ms;

    if (timeoutMs == NEONRT_WAIT_FOREVER) {
        wait_ms = NEONRT_WAIT_FOREVER;
    } else if (timeoutMs == NEONRT_NO_WAIT) {
        wait_ms = NEONRT_NO_WAIT;
    } else {
        wait_ms = ((int)wait_ms_f) + timeoutMs + 1;
    }

    UART_HandleTypeDef *huart = &g_uart[index];

    uint16_t send_bytes = 0;

    for (send_bytes = 0; send_bytes < size; send_bytes++) {
        NeonRTOS_SyncObjClear(&UART_Send_SyncHandle[index]);

        if (HAL_UART_Transmit_IT(huart, &data_wr[send_bytes], 1) != HAL_OK) {
            if (send_bytes > 0) {
                return (hwUART_OpResult)send_bytes;
            }
            return hwUART_Busy;
        }

        if (NeonRTOS_SyncObjWait(&UART_Send_SyncHandle[index], wait_ms) != NeonRTOS_OK) {
            if (send_bytes > 0) {
                return (hwUART_OpResult)send_bytes;
            }
            return hwUART_Busy;
        }
    }

    return (hwUART_OpResult)send_bytes;
}

hwUART_OpResult UART_PutChar(hwUART_Index index, uint8_t char_wr, uint32_t timeoutMs)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_NotInit;
    }

    return UART_Write(index, &char_wr, 1, timeoutMs);
}

void UART_Printf(const char *format, ...)
{
    if (UART_Init_Status[LOG_UART_INDEX] == false) {
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

    if (len >= (int)sizeof(buffer)) {
        len = sizeof(buffer) - 1;
    }

    UART_Write(LOG_UART_INDEX, (uint8_t *)buffer, len, 1000);
}

bool UART_is_Init(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX) {
        return false;
    }

    return UART_Init_Status[index];
}

#endif // DEVICE_STM32