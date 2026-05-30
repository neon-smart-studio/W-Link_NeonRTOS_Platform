#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "UART/UART.h"

#ifdef DEVICE_NUVOTON

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "UART/Pin/Nuvoton/UART_Pin_Nuvoton.h"

#include "UART_Nuvoton.h"

bool UART_Init_Status[hwUART_Index_MAX] = {false};

static int  UART_BaudRate[hwUART_Index_MAX] = {0};
static bool UART_FlowControl[hwUART_Index_MAX] = {false};

static NeonRTOS_SyncObj_t UART_Send_SyncHandle[hwUART_Index_MAX];
static NeonRTOS_SyncObj_t UART_Recv_SyncHandle[hwUART_Index_MAX];

void UART_RxCpltCallback(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX)
        return;

    NeonRTOS_SyncObjSignalFromISR(&UART_Recv_SyncHandle[index]);
}

void UART_TxCpltCallback(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX)
        return;

    NeonRTOS_SyncObjSignalFromISR(&UART_Send_SyncHandle[index]);
}

hwUART_OpResult UART_Open(hwUART_Index index, uint32_t baudrate, bool rts_cts)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (rts_cts) {
        if (UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].rts_pin == hwGPIO_Pin_NC ||
            UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].cts_pin == hwGPIO_Pin_NC) {
            return hwUART_InvalidParameter;
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

    UART_Pin_Def def =
        UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]];

    if (rts_cts) {
        if (def.rts_pin == hwGPIO_Pin_NC ||
            def.cts_pin == hwGPIO_Pin_NC) {
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

    UART_GPIO_ConfigAF(index, rts_cts);

    hwUART_OpResult result = UART_Instance_Init(index,
                                baudrate,
                                rts_cts,
                                data_bits,
                                parity,
                                stop_bits);
    if (result < hwUART_OK)
    {
        NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);
        return result;
    }

    UART_NVIC_Init(index);

    UART_FlowControl[index] = rts_cts;
    UART_Init_Status[index] = true;

    gpio_pin_init_status[def.tx_pin] = true;
    gpio_pin_init_status[def.rx_pin] = true;

    if (UART_FlowControl[index]) {
        gpio_pin_init_status[def.rts_pin] = true;
        gpio_pin_init_status[def.cts_pin] = true;
        UART_FlowControl[index] = false;
    }

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

    UART_Pin_Def def =
        UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]];

    UART_Init_Status[index] = false;

    UART_NVIC_DeInit(index);

    hwUART_OpResult result = UART_Instance_DeInit(index);
    if (result < hwUART_OK)
    {
        return result;
    }

    UART_GPIO_DeConfigAF(index, UART_FlowControl[index]);

    NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);

    gpio_pin_init_status[def.tx_pin] = false;
    gpio_pin_init_status[def.rx_pin] = false;

    if (UART_FlowControl[index]) {
        gpio_pin_init_status[def.rts_pin] = false;
        gpio_pin_init_status[def.cts_pin] = false;
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

    uint16_t recv_bytes = 0;

    for (recv_bytes = 0; recv_bytes < size; recv_bytes++) {
        NeonRTOS_SyncObjClear(&UART_Recv_SyncHandle[index]);

        if (UART_Instance_Read_IT(index, &data_rd[recv_bytes], 1) != hwUART_OK) {
            UART_Instance_Stop_Read(index);
            if (recv_bytes > 0) {
                return (hwUART_OpResult)recv_bytes;
            }
            return hwUART_Busy;
        }

        if (NeonRTOS_SyncObjWait(&UART_Recv_SyncHandle[index], wait_ms) != NeonRTOS_OK) {
            UART_Instance_Stop_Read(index);
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

    uint16_t send_bytes = 0;

    for (send_bytes = 0; send_bytes < size; send_bytes++) {
        NeonRTOS_SyncObjClear(&UART_Send_SyncHandle[index]);

        if (UART_Instance_Write_IT(index, &data_wr[send_bytes], 1) != hwUART_OK) {
            UART_Instance_Stop_Write(index);
            if (send_bytes > 0) {
                return (hwUART_OpResult)send_bytes;
            }
            return hwUART_Busy;
        }

        if (NeonRTOS_SyncObjWait(&UART_Send_SyncHandle[index], wait_ms) != NeonRTOS_OK) {
            UART_Instance_Stop_Write(index);
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

#endif // DEVICE_NUVOTON