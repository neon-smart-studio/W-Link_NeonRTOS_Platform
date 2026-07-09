#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#ifdef DEVICE_TIMSP432P

#include "GPIO/Device/TIMSP432/GPIO_TIMSP432P.h"

#include "I2C/Pin/TIMSP432/I2C_Pin_TIMSP432P.h"

typedef enum {
    MSP432P_I2C_IDLE = 0,
    MSP432P_I2C_TX,
    MSP432P_I2C_RX,
    MSP432P_I2C_DONE,
    MSP432P_I2C_ERROR
} MSP432P_I2C_State;

typedef struct {
    MSP432P_I2C_State state;

    uint8_t addr;

    uint8_t *tx_buf;
    uint8_t tx_len;
    uint8_t tx_pos;

    uint8_t *rx_buf;
    uint8_t rx_len;
    uint8_t rx_pos;

    bool stop;
    int error;
} MSP432P_I2C_Transfer;

bool I2C_Master_Init_Status[hwI2C_Index_MAX] = {false};

static hwI2C_Speed_Mode I2C_Clock_Speed_Mode[hwI2C_Index_MAX] = {
    hwI2C_Standard_Mode
};

static NeonRTOS_SyncObj_t I2C_Master_Done_SyncHandle[hwI2C_Index_MAX];
static MSP432P_I2C_Transfer i2c_xfer[hwI2C_Index_MAX];

static uint32_t I2C_Map_Soc_Base(hwI2C_Index index)
{
    switch (index) {
        case hwI2C_Index_0: return EUSCI_B0_BASE;
        case hwI2C_Index_1: return EUSCI_B1_BASE;
        case hwI2C_Index_2: return EUSCI_B2_BASE;
        case hwI2C_Index_3: return EUSCI_B3_BASE;
        default: break;
    }

    return 0;
}

static uint32_t I2C_Map_Soc_Int(hwI2C_Index index)
{
    switch (index) {
        case hwI2C_Index_0: return INT_EUSCIB0;
        case hwI2C_Index_1: return INT_EUSCIB1;
        case hwI2C_Index_2: return INT_EUSCIB2;
        case hwI2C_Index_3: return INT_EUSCIB3;
        default: break;
    }

    return 0;
}

static void I2C0_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_0); }
static void I2C1_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_1); }
static void I2C2_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_2); }
static void I2C3_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_3); }

static hwI2C_OpResult I2C_NVIC_Init(hwI2C_Index index)
{
    uint32_t irq = I2C_Map_Soc_Int(index);
    if (irq == 0) {
        return hwI2C_InvalidParameter;
    }

    switch (index) {
        case hwI2C_Index_0: MAP_Interrupt_registerInterrupt(irq, I2C0_IRQ_Handler); break;
        case hwI2C_Index_1: MAP_Interrupt_registerInterrupt(irq, I2C1_IRQ_Handler); break;
        case hwI2C_Index_2: MAP_Interrupt_registerInterrupt(irq, I2C2_IRQ_Handler); break;
        case hwI2C_Index_3: MAP_Interrupt_registerInterrupt(irq, I2C3_IRQ_Handler); break;
        default: return hwI2C_InvalidParameter;
    }

    MAP_Interrupt_enableInterrupt(irq);

    return hwI2C_OK;
}

static void I2C_NVIC_DeInit(hwI2C_Index index)
{
    uint32_t irq = I2C_Map_Soc_Int(index);
    if (irq == 0) {
        return;
    }

    MAP_Interrupt_disableInterrupt(irq);
}

static void MSP432P_I2C_Finish(hwI2C_Index index)
{
    uint32_t base = I2C_Map_Soc_Base(index);

    MAP_I2C_disableInterrupt(base, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 | EUSCI_B_I2C_RECEIVE_INTERRUPT0 | EUSCI_B_I2C_NAK_INTERRUPT);

    NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
}

static void MSP432P_I2C_StartNext(hwI2C_Index index)
{
    uint32_t base = I2C_Map_Soc_Base(index);
    MSP432P_I2C_Transfer *t = &i2c_xfer[index];

    if (base == 0) {
        return;
    }

    if (t->state == MSP432P_I2C_TX)
    {
        if (t->tx_pos >= t->tx_len)
        {
            if (t->stop) {
                MAP_I2C_masterSendMultiByteStop(base);
            }

            t->state = MSP432P_I2C_DONE;
            MSP432P_I2C_Finish(index);
            return;
        }

        MAP_I2C_masterSendMultiByteNext(base, t->tx_buf[t->tx_pos++]);
        return;
    }

    if (t->state == MSP432P_I2C_RX)
    {
        if (t->rx_pos < t->rx_len)
        {
            t->rx_buf[t->rx_pos++] = MAP_I2C_masterReceiveMultiByteNext(base);
        }

        if (t->rx_pos >= t->rx_len)
        {
            if (t->stop)
            {
                MAP_I2C_masterReceiveMultiByteStop(base);
            }

            t->state = MSP432P_I2C_DONE;
            MSP432P_I2C_Finish(index);
            return;
        }
    }
}

void I2C_IRQ_Process(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX)
    {
        return;
    }

    uint32_t base = I2C_Map_Soc_Base(index);
    if (base == 0)
    {
        return;
    }

    uint_fast16_t status = MAP_I2C_getEnabledInterruptStatus(base);

    MAP_I2C_clearInterruptFlag(base, status);

    if (status & EUSCI_B_I2C_NAK_INTERRUPT)
    {
        i2c_xfer[index].state = MSP432P_I2C_ERROR;
        i2c_xfer[index].error = EUSCI_B_I2C_NAK_INTERRUPT;
        MSP432P_I2C_Finish(index);
        return;
    }

    if (status & (EUSCI_B_I2C_TRANSMIT_INTERRUPT0 | EUSCI_B_I2C_RECEIVE_INTERRUPT0))
    {
        MSP432P_I2C_StartNext(index);
    }
}

hwI2C_OpResult I2C_Master_Init(hwI2C_Index index, hwI2C_Speed_Mode speed_mode)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (I2C_Master_Init_Status[index]) {
        return hwI2C_OK;
    }

    if (speed_mode >= hwI2C_Speed_Mode_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (speed_mode == hwI2C_High_Speed_Mode) {
        return hwI2C_Unsupport;
    }

    uint32_t base = I2C_Map_Soc_Base(index);
    if (base == 0) {
        return hwI2C_InvalidParameter;
    }

    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index].sda_pin;
    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index].scl_pin;

    uint32_t sda_port = GPIO_Map_Soc_Port_Base(sda_pin);
    uint32_t scl_port = GPIO_Map_Soc_Port_Base(scl_pin);
    uint32_t sda_mask = GPIO_Map_Soc_Pin_Mask(sda_pin);
    uint32_t scl_mask = GPIO_Map_Soc_Pin_Mask(scl_pin);

    if (sda_port == 0 || scl_port == 0 ||
        sda_mask == 0 || scl_mask == 0)
    {
        return hwI2C_InvalidParameter;
    }

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(scl_port, scl_mask, GPIO_PRIMARY_MODULE_FUNCTION);

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(sda_port, sda_mask, GPIO_PRIMARY_MODULE_FUNCTION);

    MAP_I2C_disableModule(base);

    eUSCI_I2C_MasterConfig config = {
        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,
        MAP_CS_getSMCLK(),
        EUSCI_B_I2C_SET_DATA_RATE_100KBPS,
        0,
        EUSCI_B_I2C_NO_AUTO_STOP
    };

    switch (speed_mode) {
        case hwI2C_Standard_Mode:
            config.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
            break;

        case hwI2C_Fast_Mode:
            config.dataRate = EUSCI_B_I2C_SET_DATA_RATE_400KBPS;
            break;

        default:
            return hwI2C_InvalidParameter;
    }

    MAP_I2C_initMaster(base, &config);
    MAP_I2C_enableModule(base);

    MAP_I2C_clearInterruptFlag(base, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 | EUSCI_B_I2C_RECEIVE_INTERRUPT0 | EUSCI_B_I2C_NAK_INTERRUPT);

    MAP_I2C_disableInterrupt(base, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 | EUSCI_B_I2C_RECEIVE_INTERRUPT0 | EUSCI_B_I2C_NAK_INTERRUPT);

    if (NeonRTOS_SyncObjCreate(&I2C_Master_Done_SyncHandle[index]) != NeonRTOS_OK)
    {
        return hwI2C_MemoryError;
    }

    memset(&i2c_xfer[index], 0, sizeof(i2c_xfer[index]));
    i2c_xfer[index].state = MSP432P_I2C_IDLE;

    I2C_NVIC_Init(index);

    gpio_pin_init_status[sda_pin] = true;
    gpio_pin_init_status[scl_pin] = true;

    I2C_Clock_Speed_Mode[index] = speed_mode;
    I2C_Master_Init_Status[index] = true;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_DeInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX)
    {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index])
    {
        return hwI2C_OK;
    }

    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index].sda_pin;
    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index].scl_pin;

    uint32_t sda_port = GPIO_Map_Soc_Port_Base(sda_pin);
    uint32_t scl_port = GPIO_Map_Soc_Port_Base(scl_pin);
    uint32_t sda_mask = GPIO_Map_Soc_Pin_Mask(sda_pin);
    uint32_t scl_mask = GPIO_Map_Soc_Pin_Mask(scl_pin);

    uint32_t base = I2C_Map_Soc_Base(index);

    if (base == 0 ||
        sda_port == 0 || scl_port == 0 ||
        sda_mask == 0 || scl_mask == 0)
    {
        return hwI2C_InvalidParameter;
    }

    I2C_Master_Init_Status[index] = false;

    MAP_I2C_disableInterrupt(base, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 | EUSCI_B_I2C_RECEIVE_INTERRUPT0 | EUSCI_B_I2C_NAK_INTERRUPT);

    MAP_I2C_clearInterruptFlag(base, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 | EUSCI_B_I2C_RECEIVE_INTERRUPT0 | EUSCI_B_I2C_NAK_INTERRUPT);

    I2C_NVIC_DeInit(index);

    MAP_I2C_disableModule(base);

    NeonRTOS_SyncObjDelete(&I2C_Master_Done_SyncHandle[index]);

    MAP_GPIO_setAsInputPin(sda_port, sda_mask);
    MAP_GPIO_setAsInputPin(scl_port, scl_mask);

    gpio_pin_init_status[sda_pin] = false;
    gpio_pin_init_status[scl_pin] = false;

    memset(&i2c_xfer[index], 0, sizeof(i2c_xfer[index]));

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_Reset(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index]) {
        return hwI2C_NotInit;
    }

    hwI2C_Speed_Mode speed = I2C_Clock_Speed_Mode[index];

    hwI2C_OpResult ret = I2C_Master_DeInit(index);
    if (ret < hwI2C_OK)
    {
        return ret;
    }

    return I2C_Master_Init(index, speed);
}

hwI2C_OpResult I2C_Master_Read(
    hwI2C_Index index,
    uint8_t address,
    uint8_t *read_dat,
    uint8_t read_len,
    bool stop,
    NeonRTOS_Time_t timeoutMs
)
{
    if (index >= hwI2C_Index_MAX)
    {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index])
    {
        return hwI2C_NotInit;
    }

    if (read_dat == NULL || read_len == 0)
    {
        return hwI2C_InvalidParameter;
    }

    uint32_t base = I2C_Map_Soc_Base(index);
    if (base == 0)
    {
        return hwI2C_InvalidParameter;
    }

    MSP432P_I2C_Transfer *t = &i2c_xfer[index];

    if (t->state == MSP432P_I2C_TX || t->state == MSP432P_I2C_RX)
    {
        return hwI2C_BusError;
    }

    memset(t, 0, sizeof(*t));

    t->state = MSP432P_I2C_RX;
    t->addr = address;
    t->rx_buf = read_dat;
    t->rx_len = read_len;
    t->rx_pos = 0;
    t->stop = stop;

    MAP_I2C_setSlaveAddress(base, address);
    MAP_I2C_setMode(base, EUSCI_B_I2C_RECEIVE_MODE);

    MAP_I2C_clearInterruptFlag(base, EUSCI_B_I2C_RECEIVE_INTERRUPT0 | EUSCI_B_I2C_NAK_INTERRUPT);

    MAP_I2C_enableInterrupt(base, EUSCI_B_I2C_RECEIVE_INTERRUPT0 | EUSCI_B_I2C_NAK_INTERRUPT);

    if (read_len == 1)
    {
        MAP_I2C_masterReceiveStart(base);
    }
    else
    {
        MAP_I2C_masterReceiveStart(base);
    }

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK) {
        MAP_I2C_disableInterrupt(base, EUSCI_B_I2C_RECEIVE_INTERRUPT0 |
                                   EUSCI_B_I2C_NAK_INTERRUPT);

        t->state = MSP432P_I2C_ERROR;
        return hwI2C_SlaveTimeout;
    }

    return (t->state == MSP432P_I2C_DONE) ? hwI2C_OK : hwI2C_BusError;
}

hwI2C_OpResult I2C_Master_Write(
    hwI2C_Index index,
    uint8_t address,
    uint8_t *write_dat,
    uint8_t write_len,
    bool stop,
    NeonRTOS_Time_t timeoutMs
)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index]) {
        return hwI2C_NotInit;
    }

    if (write_dat == NULL || write_len == 0) {
        return hwI2C_InvalidParameter;
    }

    uint32_t base = I2C_Map_Soc_Base(index);
    if (base == 0) {
        return hwI2C_InvalidParameter;
    }

    MSP432P_I2C_Transfer *t = &i2c_xfer[index];

    if (t->state == MSP432P_I2C_TX || t->state == MSP432P_I2C_RX) {
        return hwI2C_BusError;
    }

    memset(t, 0, sizeof(*t));

    t->state = MSP432P_I2C_TX;
    t->addr = address;
    t->tx_buf = write_dat;
    t->tx_len = write_len;
    t->tx_pos = 1;
    t->stop = stop;

    MAP_I2C_setSlaveAddress(base, address);
    MAP_I2C_setMode(base, EUSCI_B_I2C_TRANSMIT_MODE);

    MAP_I2C_clearInterruptFlag(base, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 |
                                 EUSCI_B_I2C_NAK_INTERRUPT);

    MAP_I2C_enableInterrupt(base, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 |
                              EUSCI_B_I2C_NAK_INTERRUPT);

    MAP_I2C_masterSendMultiByteStart(base, write_dat[0]);

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK)
    {
        MAP_I2C_disableInterrupt(base, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 | EUSCI_B_I2C_NAK_INTERRUPT);

        t->state = MSP432P_I2C_ERROR;
        return hwI2C_SlaveTimeout;
    }

    return (t->state == MSP432P_I2C_DONE) ? hwI2C_OK : hwI2C_BusError;
}

bool I2C_Master_isInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return false;
    }

    return I2C_Master_Init_Status[index];
}

#endif //DEVICE_TIMSP432P