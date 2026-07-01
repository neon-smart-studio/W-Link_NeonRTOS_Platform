
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "UART/UART.h"

#ifdef DEVICE_TITIVAC

#include "GPIO/Device/TITivaC/GPIO_TITivaC.h"

#include "UART/Pin/TITivaC/UART_Pin_TITivaC.h"

#include "UART_TITivaC.h"

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
} TM4C_UART_IT_State;

static TM4C_UART_IT_State UART_IT_State[hwUART_Index_MAX];

void UART_IRQ_Process(hwUART_Index index)
{
    uint32_t base = UART_Map_Soc_Base(index);
    TM4C_UART_IT_State *s = &UART_IT_State[index];

    uint32_t status = MAP_UARTIntStatus(base, true);
    MAP_UARTIntClear(base, status);

    if ((status & (UART_INT_RX | UART_INT_RT)) && s->rx_busy)
    {
        while (MAP_UARTCharsAvail(base) && s->rx_count < s->rx_size)
        {
            int32_t ch = MAP_UARTCharGetNonBlocking(base);

            if (ch >= 0)
            {
                s->rx_buf[s->rx_count++] = (uint8_t)ch;
            }
        }

        if (s->rx_count >= s->rx_size)
        {
            s->rx_busy = false;
            MAP_UARTIntDisable(base, UART_INT_RX | UART_INT_RT);
            NeonRTOS_SyncObjSignalFromISR(&UART_Recv_SyncHandle[index]);
        }
    }

    if ((status & UART_INT_TX) && s->tx_busy)
    {
        while (MAP_UARTSpaceAvail(base) && s->tx_count < s->tx_size)
        {
            MAP_UARTCharPutNonBlocking(base, s->tx_buf[s->tx_count++]);
        }

        if (s->tx_count >= s->tx_size)
        {
            s->tx_busy = false;
            MAP_UARTIntDisable(base, UART_INT_TX);
            NeonRTOS_SyncObjSignalFromISR(&UART_Send_SyncHandle[index]);
        }
    }
}

hwUART_OpResult UART_Open(hwUART_Index index, uint32_t baudrate, bool rts_cts)
{
    if(index>=hwUART_Index_MAX)
    {
        return hwUART_InvalidParameter;
    }
    
#if defined(TM4C123)
    if(rts_cts)
    {
        return hwUART_Unsupport;
    }
#endif
#if defined(TM4C1294)
    if(index>=hwUART_Index_5)
    {
        if(rts_cts)
        {
            return hwUART_Unsupport;
        }
    }
#endif
    
    return UART_Open_Specific_Format(index, baudrate, rts_cts, 8, UART_Parity_None, 1);
}

hwUART_OpResult UART_Open_Specific_Format(hwUART_Index index, uint32_t baudrate, bool rts_cts, uint8_t data_bits, UART_Parity parity, uint8_t stop_bits)
{
    if(index>=hwUART_Index_MAX)
    {
        return hwUART_InvalidParameter;
    }
  
    if(parity>=UART_Parity_MAX)
    {
        return hwUART_InvalidParameter;
    }
    
    if(stop_bits!=1 && stop_bits!=2)
    {
        return hwUART_InvalidParameter;
    }
    
    if(data_bits!=5 && data_bits!=6 && data_bits!=7 && data_bits!=8)
    {
        return hwUART_InvalidParameter;
    }
    
#if defined(TM4C123)
    if(rts_cts)
    {
        return hwUART_Unsupport;
    }
#endif
#if defined(TM4C1294)
    if(index>=hwUART_Index_5)
    {
        if(rts_cts)
        {
            return hwUART_Unsupport;
        }
    }
#endif
    
    hwGPIO_Pin tx_pin = UART_Pin_Def_Table[index].tx_pin;
    hwGPIO_Pin rx_pin = UART_Pin_Def_Table[index].rx_pin;
    hwGPIO_Pin rts_pin = UART_Pin_Def_Table[index].rts_pin;
    hwGPIO_Pin cts_pin = UART_Pin_Def_Table[index].cts_pin;

    uint32_t txPortBase = GPIO_Map_Soc_Port_Base(tx_pin);
    uint32_t txPinMask = GPIO_Map_Soc_Pin_Mask(tx_pin);
    uint32_t rxPortBase = GPIO_Map_Soc_Port_Base(rx_pin);
    uint32_t rxPinMask = GPIO_Map_Soc_Pin_Mask(rx_pin);

    uint32_t rtsPortBase = GPIO_Map_Soc_Port_Base(rts_pin);
    uint32_t rtsPinMask = GPIO_Map_Soc_Pin_Mask(rts_pin);
    uint32_t ctsPortBase = GPIO_Map_Soc_Port_Base(cts_pin);
    uint32_t ctsPinMask = GPIO_Map_Soc_Pin_Mask(cts_pin);

    uint32_t txPinCfg = UART_Map_PinConfig(index, tx_pin);
    uint32_t rxPinCfg = UART_Map_PinConfig(index, rx_pin);
    uint32_t rtsPinCfg = UART_Map_PinConfig(index, rts_pin);
    uint32_t ctsPinCfg = UART_Map_PinConfig(index, cts_pin);

    if(txPortBase==0 || txPinMask==0 || rxPortBase==0 || rxPinMask==0)
    {
      return hwUART_InvalidParameter;
    }
    
    if(rts_cts)
    {
        if(rtsPortBase==0 || rtsPinMask==0 || ctsPortBase==0 || ctsPinMask==0)
        {
            return hwUART_InvalidParameter;
        }

        if(rtsPinCfg==0 || ctsPinCfg==0)
        {
            return hwUART_InvalidParameter;
        }
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

    GPIO_Enable_Port_Clock(txPortBase);
    GPIO_Enable_Port_Clock(rxPortBase);
    if(rts_cts)
    {
        GPIO_Enable_Port_Clock(rtsPortBase);
        GPIO_Enable_Port_Clock(ctsPortBase);
    }

    uint32_t base = UART_Map_Soc_Base(index);
    uint32_t sysCtrlPeriph = UART_Map_Soc_Periph(index);

    MAP_SysCtlPeripheralEnable(sysCtrlPeriph);

    MAP_GPIOPinConfigure(txPinCfg);
    MAP_GPIOPinConfigure(rxPinCfg);
    if(rts_cts)
    {
        MAP_GPIOPinConfigure(rtsPinCfg);
        MAP_GPIOPinConfigure(ctsPinCfg);
    }

    MAP_GPIOPinTypeUART(txPortBase, txPinMask);
    MAP_GPIOPinTypeUART(rxPortBase, rxPinMask);
    if(rts_cts)
    {
        MAP_GPIOPinTypeUART(rtsPortBase, rtsPinMask);
        MAP_GPIOPinTypeUART(ctsPortBase, ctsPinMask);
    }

    uint32_t cfgFlag = 0;

    switch(data_bits)
    {
        case 5:
            cfgFlag |= UART_CONFIG_WLEN_5;
            break;
        case 6:
            cfgFlag |= UART_CONFIG_WLEN_6;
            break;
        case 7:
            cfgFlag |= UART_CONFIG_WLEN_7;
            break;
        case 8:
            cfgFlag |= UART_CONFIG_WLEN_8;
            break;
    }

    switch(parity)
    {
        case UART_Parity_None:
            cfgFlag |= UART_CONFIG_PAR_NONE;
            break;
        case UART_Parity_Odd:
            cfgFlag |= UART_CONFIG_PAR_ODD;
            break;
        case UART_Parity_Even:
            cfgFlag |= UART_CONFIG_PAR_EVEN;
            break;
    }

    switch(stop_bits)
    {
        case 1:
            cfgFlag |= UART_CONFIG_STOP_ONE;
            break;
        case 2:
            cfgFlag |= UART_CONFIG_STOP_TWO;
            break;
    }

    MAP_UARTConfigSetExpClk(base, MAP_SysCtlClockGet(), baudrate, cfgFlag);

    if (rts_cts)
    {
        MAP_UARTFlowControlSet(base, UART_FLOWCONTROL_TX | UART_FLOWCONTROL_RX);
    }
    else
    {
        MAP_UARTFlowControlSet(base, UART_FLOWCONTROL_NONE);
    }

    MAP_UARTEnable(base);
    
    memset(&UART_IT_State[index], 0, sizeof(UART_IT_State[index]));

    MAP_UARTFIFOEnable(base);
    MAP_UARTFIFOLevelSet(base, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    UART_NVIC_Enable(index);

    UART_BaudRate[index] = baudrate;
    
    gpio_pin_init_status[tx_pin] = true;
    gpio_pin_init_status[rx_pin] = true;

    if (rts_cts)
    {
        gpio_pin_init_status[rts_pin] = true;
        gpio_pin_init_status[cts_pin] = true;
        UART_FlowControl[index] = true;
    }

    UART_Init_Status[index] = true;

    return hwUART_OK;
}

hwUART_OpResult UART_Close(hwUART_Index index)
{
    hwGPIO_Pin tx_pin = UART_Pin_Def_Table[index].tx_pin;
    hwGPIO_Pin rx_pin = UART_Pin_Def_Table[index].rx_pin;
    hwGPIO_Pin rts_pin = UART_Pin_Def_Table[index].rts_pin;
    hwGPIO_Pin cts_pin = UART_Pin_Def_Table[index].cts_pin;

    uint32_t txPortBase = GPIO_Map_Soc_Port_Base(tx_pin);
    uint32_t txPinMask = GPIO_Map_Soc_Pin_Mask(tx_pin);
    uint32_t rxPortBase = GPIO_Map_Soc_Port_Base(rx_pin);
    uint32_t rxPinMask = GPIO_Map_Soc_Pin_Mask(rx_pin);

    uint32_t rtsPortBase = GPIO_Map_Soc_Port_Base(rts_pin);
    uint32_t rtsPinMask = GPIO_Map_Soc_Pin_Mask(rts_pin);
    uint32_t ctsPortBase = GPIO_Map_Soc_Port_Base(cts_pin);
    uint32_t ctsPinMask = GPIO_Map_Soc_Pin_Mask(cts_pin);

    if(txPortBase==0 || txPinMask==0 || rxPortBase==0 || rxPinMask==0)
    {
      return hwUART_InvalidParameter;
    }

    if(UART_FlowControl[index])
    {
        if(rtsPortBase==0 || rtsPinMask==0 || ctsPortBase==0 || ctsPinMask==0)
        {
            return hwUART_InvalidParameter;
        }
    }

    uint32_t base = UART_Map_Soc_Base(index);
    uint32_t sysCtrlPeriph = UART_Map_Soc_Periph(index);

    if(UART_FlowControl[index])
    {
        MAP_UARTFlowControlSet(base, UART_FLOWCONTROL_NONE);
    }

    MAP_UARTDisable(base);

    MAP_SysCtlPeripheralDisable(sysCtrlPeriph);

    MAP_GPIOPinTypeGPIOInput(txPortBase, txPinMask);
    MAP_GPIOPinTypeGPIOInput(rxPortBase, rxPinMask);

    if(UART_FlowControl[index])
    {
        MAP_GPIOPinTypeGPIOInput(rtsPortBase, rtsPinMask);
        MAP_GPIOPinTypeGPIOInput(ctsPortBase, ctsPinMask);
    }

    UART_NVIC_Disable(index);

    NeonRTOS_SyncObjDelete(&UART_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&UART_Recv_SyncHandle[index]);

    memset(&UART_IT_State[index], 0, sizeof(UART_IT_State[index]));

    gpio_pin_init_status[tx_pin] = false;
    gpio_pin_init_status[rx_pin] = false;

    if(UART_FlowControl[index])
    {
        gpio_pin_init_status[rts_pin] = false;
        gpio_pin_init_status[cts_pin] = false;
    }

    UART_BaudRate[index] = 0;
    UART_FlowControl[index] = false;
    
    UART_Init_Status[index] = false;

    return hwUART_OK;
}

hwUART_OpResult UART_Read(hwUART_Index index, uint8_t *data_rd, size_t size, uint32_t timeoutMs)
{
    if(index>=hwUART_Index_MAX)
    {
        return hwUART_InvalidParameter;
    }
    
    if(data_rd==NULL)
    {
        return hwUART_InvalidParameter;
    }
    
    if (!UART_Init_Status[index])
    {
        return hwUART_NotInit;
    }

    uint32_t base = UART_Map_Soc_Base(index);
    TM4C_UART_IT_State *s = &UART_IT_State[index];

    if (s->rx_busy)
        return hwUART_Busy;

    float wait_ms_f = ((float)size) / ((float)UART_BaudRate[index] / 8.0f / 1000.0f);
    NeonRTOS_Time_t wait_ms = (timeoutMs == NEONRT_WAIT_FOREVER) ?
                              NEONRT_WAIT_FOREVER :
                              ((int)wait_ms_f) + timeoutMs + 1;

    NeonRTOS_SyncObjClear(&UART_Recv_SyncHandle[index]);

    s->rx_buf = data_rd;
    s->rx_size = size;
    s->rx_count = 0;
    s->rx_busy = true;

    MAP_UARTIntEnable(base, UART_INT_RX | UART_INT_RT);

    if (NeonRTOS_SyncObjWait(&UART_Recv_SyncHandle[index], wait_ms) != NeonRTOS_OK)
    {
        MAP_UARTIntDisable(base, UART_INT_RX | UART_INT_RT);
        s->rx_busy = false;

        if (s->rx_count > 0)
            return (hwUART_OpResult)s->rx_count;

        return hwUART_Busy;
    }

    return (hwUART_OpResult)s->rx_count;
}

hwUART_OpResult UART_GetChar(hwUART_Index index, uint8_t* char_rd, uint32_t timeoutMs)
{
    if(index>=hwUART_Index_MAX)
    {
        return hwUART_InvalidParameter;
    }
    
    if(char_rd==NULL)
    {
        return hwUART_InvalidParameter;
    }
    
    return UART_Read(index, char_rd, 1, timeoutMs);
}

hwUART_OpResult UART_Write(hwUART_Index index, uint8_t *data_wr, size_t size, uint32_t timeoutMs)
{
    if(index>=hwUART_Index_MAX)
    {
        return hwUART_InvalidParameter;
    }

    if (!UART_Init_Status[index])
    {
        return hwUART_NotInit;
    }

    uint32_t base = UART_Map_Soc_Base(index);
    TM4C_UART_IT_State *s = &UART_IT_State[index];

    if (s->tx_busy)
        return hwUART_Busy;

    float wait_ms_f = ((float)size) / ((float)UART_BaudRate[index] / 8.0f / 1000.0f);
    NeonRTOS_Time_t wait_ms = (timeoutMs == NEONRT_WAIT_FOREVER) ?
                              NEONRT_WAIT_FOREVER :
                              ((int)wait_ms_f) + timeoutMs + 1;

    NeonRTOS_SyncObjClear(&UART_Send_SyncHandle[index]);

    s->tx_buf = data_wr;
    s->tx_size = size;
    s->tx_count = 0;
    s->tx_busy = true;

    MAP_UARTIntEnable(base, UART_INT_TX);

    /* 手動先塞一次 FIFO，避免 TX interrupt 沒立刻觸發 */
    UART_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&UART_Send_SyncHandle[index], wait_ms) != NeonRTOS_OK)
    {
        MAP_UARTIntDisable(base, UART_INT_TX);
        s->tx_busy = false;

        if (s->tx_count > 0)
            return s->tx_count;

        return hwUART_Busy;
    }

    return s->tx_count;
}

hwUART_OpResult UART_PutChar(hwUART_Index index, uint8_t char_wr, uint32_t timeoutMs)
{
    if(index>=hwUART_Index_MAX)
    {
        return hwUART_InvalidParameter;
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

#endif //DEVICE_TM4C1294