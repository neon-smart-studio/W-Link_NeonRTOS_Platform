
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "UART/UART.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSP432P

#include "GPIO/Device/TIMSP432/GPIO_TIMSP432P.h"

#include "UART/Pin/TIMSP432/UART_Pin_TIMSP432P.h"

#include "UART_TIMSP432.h"

bool UART_Init_Status[hwUART_Index_MAX] = {false};

static int UART_BaudRate[hwUART_Index_MAX] = {0};
static bool UART_FlowControl[hwUART_Index_MAX] = {false};

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
} MSP432P_UART_IT_State;

static MSP432P_UART_IT_State UART_IT_State[hwUART_Index_MAX];

uint32_t UART_Map_Soc_Base(hwUART_Index index)
{
    switch (index)
    {
        case hwUART_Index_0: return EUSCI_A0_BASE;
        case hwUART_Index_1: return EUSCI_A1_BASE;
        case hwUART_Index_2: return EUSCI_A2_BASE;
        case hwUART_Index_3: return EUSCI_A3_BASE;
        default:             return 0;
    }
}

uint32_t UART_Map_IRQ(hwUART_Index index)
{
    switch (index)
    {
        case hwUART_Index_0: return INT_EUSCIA0;
        case hwUART_Index_1: return INT_EUSCIA1;
        case hwUART_Index_2: return INT_EUSCIA2;
        case hwUART_Index_3: return INT_EUSCIA3;
        default:             return 0;
    }
}

void EUSCIA0_IRQHandler(void) { UART_IRQ_Process(hwUART_Index_0); }
void EUSCIA1_IRQHandler(void) { UART_IRQ_Process(hwUART_Index_1); }
void EUSCIA2_IRQHandler(void) { UART_IRQ_Process(hwUART_Index_2); }
void EUSCIA3_IRQHandler(void) { UART_IRQ_Process(hwUART_Index_3); }

void UART_NVIC_Enable(hwUART_Index index)
{
    uint32_t base = UART_Map_Soc_Base(index);
    uint32_t irq  = UART_Map_IRQ(index);

    if (base == 0 || irq == 0) {
        return;
    }

    MAP_UART_disableInterrupt(base,
        EUSCI_A_UART_RECEIVE_INTERRUPT |
        EUSCI_A_UART_TRANSMIT_INTERRUPT);

    MAP_UART_clearInterruptFlag(base,
        EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG |
        EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG);

    switch (index)
    {
        case hwUART_Index_0:
            MAP_Interrupt_registerInterrupt(INT_EUSCIA0, EUSCIA0_IRQHandler);
            break;

        case hwUART_Index_1:
            MAP_Interrupt_registerInterrupt(INT_EUSCIA1, EUSCIA1_IRQHandler);
            break;

        case hwUART_Index_2:
            MAP_Interrupt_registerInterrupt(INT_EUSCIA2, EUSCIA2_IRQHandler);
            break;

        case hwUART_Index_3:
            MAP_Interrupt_registerInterrupt(INT_EUSCIA3, EUSCIA3_IRQHandler);
            break;

        default:
            return;
    }

    MAP_Interrupt_enableInterrupt(irq);
}

void UART_NVIC_Disable(hwUART_Index index)
{
    uint32_t base = UART_Map_Soc_Base(index);
    uint32_t irq  = UART_Map_IRQ(index);

    if (base != 0)
    {
        MAP_UART_disableInterrupt(base,
            EUSCI_A_UART_RECEIVE_INTERRUPT |
            EUSCI_A_UART_TRANSMIT_INTERRUPT);

        MAP_UART_clearInterruptFlag(base,
            EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG |
            EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG);
    }

    if (irq != 0) {
        MAP_Interrupt_disableInterrupt(irq);
    }
}

void UART_IRQ_Process(hwUART_Index index)
{
    uint32_t base = UART_Map_Soc_Base(index);
    MSP432P_UART_IT_State *s = &UART_IT_State[index];

    uint32_t status = MAP_UART_getEnabledInterruptStatus(base);

    MAP_UART_clearInterruptFlag(base, status);

    if ((status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG) && s->rx_busy)
    {
        while ((MAP_UART_getInterruptStatus(base, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)) &&
               s->rx_count < s->rx_size)
        {
            s->rx_buf[s->rx_count++] = MAP_UART_receiveData(base);
        }

        if (s->rx_count >= s->rx_size)
        {
            s->rx_busy = false;

            MAP_UART_disableInterrupt(base, EUSCI_A_UART_RECEIVE_INTERRUPT);

            NeonRTOS_SyncObjSignalFromISR(&UART_Recv_SyncHandle[index]);
        }
    }

    if ((status & EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG) && s->tx_busy)
    {
        while ((MAP_UART_getInterruptStatus(base, EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG)) &&
               s->tx_count < s->tx_size)
        {
            MAP_UART_transmitData(base, s->tx_buf[s->tx_count++]);
        }

        if (s->tx_count >= s->tx_size)
        {
            s->tx_busy = false;

            MAP_UART_disableInterrupt(base, EUSCI_A_UART_TRANSMIT_INTERRUPT);

            NeonRTOS_SyncObjSignalFromISR(&UART_Send_SyncHandle[index]);
        }
    }
}

hwUART_OpResult UART_Open(hwUART_Index index, uint32_t baudrate, bool rts_cts)
{
    if (rts_cts) {
        return hwUART_Unsupport;
    }

    return UART_Open_Specific_Format(index, baudrate, false, 8, UART_Parity_None, 1);
}

hwUART_OpResult UART_Open_Specific_Format(hwUART_Index index, uint32_t baudrate, bool rts_cts, uint8_t data_bits, UART_Parity parity, uint8_t stop_bits)
{
    if (index >= hwUART_Index_MAX) {
        return hwUART_InvalidParameter;
    }

    if (rts_cts) {
        return hwUART_Unsupport;
    }

    if (parity >= UART_Parity_MAX) {
        return hwUART_InvalidParameter;
    }

    if(stop_bits!=1 && stop_bits!=2)
    {
        return hwUART_InvalidParameter;
    }
    
    if (data_bits != 8) {
        return hwUART_Unsupport;
    }

    hwGPIO_Pin tx_pin = UART_Pin_Def_Table[index].tx_pin;
    hwGPIO_Pin rx_pin = UART_Pin_Def_Table[index].rx_pin;

    uint32_t txPort = GPIO_Map_Soc_Port_Base(tx_pin);
    uint32_t txMask = GPIO_Map_Soc_Pin_Mask(tx_pin);
    uint32_t rxPort = GPIO_Map_Soc_Port_Base(rx_pin);
    uint32_t rxMask = GPIO_Map_Soc_Pin_Mask(rx_pin);

    if (txPort == 0 || txMask == 0 || rxPort == 0 || rxMask == 0) {
        return hwUART_InvalidParameter;
    }

    if (NeonRTOS_SyncObjCreate(&UART_Send_SyncHandle[index]) != NeonRTOS_OK) {
        return hwUART_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&UART_Recv_SyncHandle[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
        return hwUART_MemoryError;
    }

    uint32_t base = UART_Map_Soc_Base(index);

    eUSCI_UART_Config cfg;

    uint32_t clk = g_sys_clock_hz;

    /*
     * 建議你的 SysCtrl 讓 SMCLK = g_sys_clock_hz 或另外改成 g_smclk_hz。
     * 若你的 SMCLK 是 12MHz，這裡請改成 g_smclk_hz。
     */
    uint32_t brw = clk / (baudrate * 16U);

    if (brw == 0) {
        brw = clk / baudrate;
        cfg.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;
    } else {
        cfg.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
    }

    uint32_t remain;
    uint32_t ucbrf = 0;
    uint32_t ucbrs = 0;

    if (cfg.overSampling == EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION)
    {
        remain = clk - (brw * baudrate * 16U);
        ucbrf = (remain * 16U) / (baudrate * 16U);

        ucbrs = 0;
    }
    else
    {
        remain = clk - (brw * baudrate);
        ucbrs = ((remain * 256U) / baudrate) & 0xFFU;
    }

    cfg.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    cfg.clockPrescalar   = brw;
    cfg.firstModReg      = ucbrf;
    cfg.secondModReg     = ucbrs;
    cfg.msborLsbFirst    = EUSCI_A_UART_LSB_FIRST;
    cfg.uartMode         = EUSCI_A_UART_MODE;

    switch (parity)
    {
        case UART_Parity_None:
            cfg.parity = EUSCI_A_UART_NO_PARITY;
            break;

        case UART_Parity_Odd:
            cfg.parity = EUSCI_A_UART_ODD_PARITY;
            break;

        case UART_Parity_Even:
            cfg.parity = EUSCI_A_UART_EVEN_PARITY;
            break;
    }

    switch (stop_bits)
    {
        case 1:
            cfg.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
            break;

        case 2:
            cfg.numberofStopBits = EUSCI_A_UART_TWO_STOP_BITS;
            break;
    }

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
        txPort,
        txMask,
        GPIO_PRIMARY_MODULE_FUNCTION);

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
        rxPort,
        rxMask,
        GPIO_PRIMARY_MODULE_FUNCTION);

    MAP_UART_initModule(base, &cfg);
    MAP_UART_enableModule(base);

    memset(&UART_IT_State[index], 0, sizeof(UART_IT_State[index]));

    UART_NVIC_Enable(index);

    UART_BaudRate[index] = baudrate;

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

    uint32_t txPort = GPIO_Map_Soc_Port_Base(tx_pin);
    uint32_t txMask = GPIO_Map_Soc_Pin_Mask(tx_pin);
    uint32_t rxPort = GPIO_Map_Soc_Port_Base(rx_pin);
    uint32_t rxMask = GPIO_Map_Soc_Pin_Mask(rx_pin);

    uint32_t base = UART_Map_Soc_Base(index);

    UART_NVIC_Disable(index);

    MAP_UART_disableModule(base);

    MAP_GPIO_setAsInputPin(txPort, txMask);
    MAP_GPIO_setAsInputPin(rxPort, rxMask);

    NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);

    memset(&UART_IT_State[index], 0, sizeof(UART_IT_State[index]));

    gpio_pin_init_status[tx_pin] = false;
    gpio_pin_init_status[rx_pin] = false;

    UART_BaudRate[index] = 0;
    UART_Init_Status[index] = false;

    return hwUART_OK;
}

hwUART_OpResult UART_Read(hwUART_Index index, uint8_t *data_rd, size_t size, uint32_t timeoutMs)
{
    if (index >= hwUART_Index_MAX || data_rd == NULL) {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_NotInit;
    }

    uint32_t base = UART_Map_Soc_Base(index);
    MSP432P_UART_IT_State *s = &UART_IT_State[index];

    if (s->rx_busy) {
        return hwUART_Busy;
    }

    float wait_ms_f = ((float)size) / ((float)UART_BaudRate[index] / 8.0f / 1000.0f);

    NeonRTOS_Time_t wait_ms = (timeoutMs == NEONRT_WAIT_FOREVER) ?
                              NEONRT_WAIT_FOREVER :
                              ((int)wait_ms_f) + timeoutMs + 1;

    NeonRTOS_SyncObjClear(&UART_Recv_SyncHandle[index]);

    s->rx_buf = data_rd;
    s->rx_size = size;
    s->rx_count = 0;
    s->rx_busy = true;

    while ((MAP_UART_getInterruptStatus(base, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)) &&
           s->rx_count < s->rx_size)
    {
        s->rx_buf[s->rx_count++] = MAP_UART_receiveData(base);
    }

    if (s->rx_count >= s->rx_size)
    {
        s->rx_busy = false;
    }
    else
    {
        MAP_UART_enableInterrupt(base, EUSCI_A_UART_RECEIVE_INTERRUPT);

        if (NeonRTOS_SyncObjWait(&UART_Recv_SyncHandle[index], wait_ms) != NeonRTOS_OK)
        {
            MAP_UART_disableInterrupt(base, EUSCI_A_UART_RECEIVE_INTERRUPT);

            s->rx_busy = false;

            if (s->rx_count > 0) {
                return (hwUART_OpResult)s->rx_count;
            }

            return hwUART_Busy;
        }
    }

    return (hwUART_OpResult)s->rx_count;
}

hwUART_OpResult UART_GetChar(hwUART_Index index, uint8_t *char_rd, uint32_t timeoutMs)
{
    return UART_Read(index, char_rd, 1, timeoutMs);
}

hwUART_OpResult UART_Write(hwUART_Index index, uint8_t *data_wr, size_t size, uint32_t timeoutMs)
{
    if (index >= hwUART_Index_MAX || data_wr == NULL) {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index]) {
        return hwUART_NotInit;
    }

    uint32_t base = UART_Map_Soc_Base(index);
    MSP432P_UART_IT_State *s = &UART_IT_State[index];

    if (s->tx_busy) {
        return hwUART_Busy;
    }

    float wait_ms_f = ((float)size) / ((float)UART_BaudRate[index] / 8.0f / 1000.0f);

    NeonRTOS_Time_t wait_ms = (timeoutMs == NEONRT_WAIT_FOREVER) ?
                              NEONRT_WAIT_FOREVER :
                              ((int)wait_ms_f) + timeoutMs + 1;

    NeonRTOS_SyncObjClear(&UART_Send_SyncHandle[index]);

    s->tx_buf = data_wr;
    s->tx_size = size;
    s->tx_count = 0;
    s->tx_busy = true;

    while ((MAP_UART_getInterruptStatus(base, EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG)) &&
           s->tx_count < s->tx_size)
    {
        MAP_UART_transmitData(base, s->tx_buf[s->tx_count++]);
    }

    if (s->tx_count >= s->tx_size)
    {
        NeonRTOS_Time_t t = 0;

        while (MAP_UART_queryStatusFlags(base, EUSCI_A_UART_BUSY))
        {
            NeonRTOS_Sleep(1);

            if (timeoutMs != NEONRT_WAIT_FOREVER && ++t >= wait_ms)
            {
                s->tx_busy = false;
                return hwUART_Busy;
            }
        }

        s->tx_busy = false;
    }
    else
    {
        MAP_UART_enableInterrupt(base, EUSCI_A_UART_TRANSMIT_INTERRUPT);

        if (NeonRTOS_SyncObjWait(&UART_Send_SyncHandle[index], wait_ms) != NeonRTOS_OK)
        {
            MAP_UART_disableInterrupt(base, EUSCI_A_UART_TRANSMIT_INTERRUPT);

            s->tx_busy = false;

            if (s->tx_count > 0) {
                return (hwUART_OpResult)s->tx_count;
            }

            return hwUART_Busy;
        }
    }

    return (hwUART_OpResult)s->tx_count;
}

hwUART_OpResult UART_PutChar(hwUART_Index index, uint8_t char_wr, uint32_t timeoutMs)
{
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

#endif //DEVICE_TIMSP432P