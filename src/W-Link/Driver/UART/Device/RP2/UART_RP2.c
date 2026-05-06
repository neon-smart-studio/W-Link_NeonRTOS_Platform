#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "UART/UART.h"

#ifdef DEVICE_RP2

#include "GPIO/Device/RP2/GPIO_RP2.h"

#include "UART/Pin/RP2/UART_Pin_RP2.h"

#include "UART_RP2.h"

bool UART_Init_Status[hwUART_Index_MAX] = {false};

static int  UART_BaudRate[hwUART_Index_MAX] = {0};
static bool UART_FlowControl[hwUART_Index_MAX] = {false};

static NeonRTOS_SyncObj_t UART_Send_SyncHandle[hwUART_Index_MAX];
static NeonRTOS_SyncObj_t UART_Recv_SyncHandle[hwUART_Index_MAX];

static volatile uint8_t *UART_Rx_Byte[hwUART_Index_MAX] = {0};
static volatile uint8_t *UART_Tx_Byte[hwUART_Index_MAX] = {0};

static uart_inst_t *UART_Map_Soc_Base(hwUART_Index index)
{
    switch (index) {
        case hwUART_Index_0: return uart0;
        case hwUART_Index_1: return uart1;
        default: return NULL;
    }
}

static void RP2_UART_IRQ_Process(hwUART_Index index)
{
    if (index >= hwUART_Index_MAX) return;

    uart_inst_t *uart = UART_Map_Soc_Base(index);
    if (!uart) return;

    if (uart_is_readable(uart)) {
        uint8_t ch = uart_getc(uart);

        if (UART_Rx_Byte[index]) {
            *UART_Rx_Byte[index] = ch;
            UART_Rx_Byte[index] = NULL;

            uart_set_irq_enables(uart, false, UART_Tx_Byte[index] != NULL);
            NeonRTOS_SyncObjSignalFromISR(&UART_Recv_SyncHandle[index]);
        }
    }

    if (uart_is_writable(uart)) {
        if (UART_Tx_Byte[index]) {
            uart_putc_raw(uart, *UART_Tx_Byte[index]);
            UART_Tx_Byte[index] = NULL;

            uart_set_irq_enables(uart, UART_Rx_Byte[index] != NULL, false);
            NeonRTOS_SyncObjSignalFromISR(&UART_Send_SyncHandle[index]);
        }
    }
}

static void UART0_IRQ_Handler(void)
{
    RP2_UART_IRQ_Process(hwUART_Index_0);
}

static void UART1_IRQ_Handler(void)
{
    RP2_UART_IRQ_Process(hwUART_Index_1);
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

    uart_inst_t *inst = UART_Map_Soc_Base(index);
    if (!inst) {
        return hwUART_InvalidParameter;
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

    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);

    if (rts_cts) {
        gpio_set_function(rts_pin, GPIO_FUNC_UART);
        gpio_set_function(cts_pin, GPIO_FUNC_UART);
    }

    hwUART_OpResult result;

    uart_init(inst, baudrate);

    uart_set_format(
        inst,
        data_bits,
        stop_bits,
        parity == UART_Parity_None ? UART_PARITY_NONE :
        parity == UART_Parity_Even ? UART_PARITY_EVEN :
                                     UART_PARITY_ODD
    );

    uart_set_hw_flow(inst, rts_cts, rts_cts);
    uart_set_fifo_enabled(inst, true);

    UART_Rx_Byte[index] = NULL;
    UART_Tx_Byte[index] = NULL;

    uart_set_irq_enables(inst, false, false);

    switch(index)
    {
        case hwUART_Index_0:
            irq_set_exclusive_handler(UART0_IRQ, UART0_IRQ_Handler);
            irq_set_enabled(UART0_IRQ, true);
            break;
        case hwUART_Index_1:
            irq_set_exclusive_handler(UART1_IRQ, UART1_IRQ_Handler);
            irq_set_enabled(UART1_IRQ, true);
            break;
    }

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
    
    uart_inst_t *inst = UART_Map_Soc_Base(index);
    if (!inst) {
        return hwUART_InvalidParameter;
    }

    switch(index)
    {
        case hwUART_Index_0:
            irq_set_enabled(UART0_IRQ, false);
            irq_remove_handler(UART0_IRQ, UART0_IRQ_Handler);
            break;
        case hwUART_Index_1:
            irq_set_enabled(UART1_IRQ, false);
            irq_remove_handler(UART1_IRQ, UART1_IRQ_Handler);
            break;
    }

    uart_deinit(inst);

    hwGPIO_Pin tx_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].tx_pin;
    hwGPIO_Pin rx_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].rx_pin;
    hwGPIO_Pin rts_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].rts_pin;
    hwGPIO_Pin cts_pin = UART_Pin_Def_Table[index][UART_Index_Map_Alt[index]].cts_pin;

    gpio_deinit(tx_pin);
    gpio_deinit(rx_pin);

    if(UART_FlowControl[index])
    {
        gpio_deinit(rts_pin);
        gpio_deinit(cts_pin);
    }
    
    UART_Init_Status[index] = false;

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

    uart_inst_t *inst = UART_Map_Soc_Base(index);
    if (!inst) {
        return hwUART_InvalidParameter;
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

        UART_Rx_Byte[index] = &data_rd[recv_bytes];

        uart_set_irq_enables(inst, true, false);

        if (NeonRTOS_SyncObjWait(&UART_Recv_SyncHandle[index], wait_ms) != NeonRTOS_OK) {
            UART_Rx_Byte[index] = NULL;
            uart_set_irq_enables(inst, false, false);

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

    uart_inst_t *inst = UART_Map_Soc_Base(index);
    if (!inst) {
        return hwUART_InvalidParameter;
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

        UART_Tx_Byte[index] = &data_wr[send_bytes];

        uart_set_irq_enables(inst, false, true);

        if (NeonRTOS_SyncObjWait(&UART_Send_SyncHandle[index], wait_ms) != NeonRTOS_OK) {
            UART_Tx_Byte[index] = NULL;
            uart_set_irq_enables(inst, false, false);

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

#endif // DEVICE_RP2