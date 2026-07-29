#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSP432E

#include "GPIO/Device/TIMSP432/GPIO_TIMSP432P.h"

#include "I2C/Pin/TIMSP432/I2C_Pin_TIMSP432P.h"

typedef enum {
    MSP432_I2C_IDLE = 0,
    MSP432_I2C_TX,
    MSP432_I2C_RX,
    MSP432_I2C_DONE,
    MSP432_I2C_ERROR
} MSP432_I2C_State;

typedef struct {
    MSP432_I2C_State state;

    uint8_t addr;

    uint8_t *tx_buf;
    uint8_t tx_len;
    uint8_t tx_pos;

    uint8_t *rx_buf;
    uint8_t rx_len;
    uint8_t rx_pos;

    bool stop;
    int error;
} MSP432E_I2C_Transfer;

bool I2C_Master_Init_Status[hwI2C_Index_MAX] = {false};

static hwI2C_Speed_Mode I2C_Clock_Speed_Mode[hwI2C_Index_MAX] = {
    hwI2C_Standard_Mode
};

static NeonRTOS_SyncObj_t I2C_Master_Done_SyncHandle[hwI2C_Index_MAX];
static MSP432E_I2C_Transfer i2c_xfer[hwI2C_Index_MAX];

static void I2C_IRQ_Process(hwI2C_Index index);

static uint32_t I2C_Map_Soc_Base(hwI2C_Index index)
{
    switch (index) {
        case hwI2C_Index_0: return I2C0_BASE;
        case hwI2C_Index_1: return I2C1_BASE;
        case hwI2C_Index_2: return I2C2_BASE;
        case hwI2C_Index_3: return I2C3_BASE;
        case hwI2C_Index_4: return I2C4_BASE;
        case hwI2C_Index_5: return I2C5_BASE;
        case hwI2C_Index_6: return I2C6_BASE;
        case hwI2C_Index_7: return I2C7_BASE;
        case hwI2C_Index_8: return I2C8_BASE;
        case hwI2C_Index_9: return I2C9_BASE;
        default: break;
    }

    return 0;
}

static uint32_t I2C_Map_Soc_Periph(hwI2C_Index index)
{
    switch (index) {
        case hwI2C_Index_0: return SYSCTL_PERIPH_I2C0;
        case hwI2C_Index_1: return SYSCTL_PERIPH_I2C1;
        case hwI2C_Index_2: return SYSCTL_PERIPH_I2C2;
        case hwI2C_Index_3: return SYSCTL_PERIPH_I2C3;
        case hwI2C_Index_4: return SYSCTL_PERIPH_I2C4;
        case hwI2C_Index_5: return SYSCTL_PERIPH_I2C5;
        case hwI2C_Index_6: return SYSCTL_PERIPH_I2C6;
        case hwI2C_Index_7: return SYSCTL_PERIPH_I2C7;
        case hwI2C_Index_8: return SYSCTL_PERIPH_I2C8;
        case hwI2C_Index_9: return SYSCTL_PERIPH_I2C9;
        default: break;
    }

    return 0;
}

static uint32_t I2C_Map_Soc_Int(hwI2C_Index index)
{
    switch (index) {
        case hwI2C_Index_0: return INT_I2C0;
        case hwI2C_Index_1: return INT_I2C1;
        case hwI2C_Index_2: return INT_I2C2;
        case hwI2C_Index_3: return INT_I2C3;
        case hwI2C_Index_4: return INT_I2C4;
        case hwI2C_Index_5: return INT_I2C5;
        case hwI2C_Index_6: return INT_I2C6;
        case hwI2C_Index_7: return INT_I2C7;
        case hwI2C_Index_8: return INT_I2C8;
        case hwI2C_Index_9: return INT_I2C9;
        default: break;
    }

    return 0;
}

static uint32_t I2C_Map_PinConfig(hwI2C_Index index, hwGPIO_Pin pin)
{
    switch (index) {
        case hwI2C_Index_0:
            if (pin == hwGPIO_Pin_B2) return GPIO_PB2_I2C0SCL;
            if (pin == hwGPIO_Pin_B3) return GPIO_PB3_I2C0SDA;
            break;
        case hwI2C_Index_1:
            if (pin == hwGPIO_Pin_G0) return GPIO_PG0_I2C1SCL;
            if (pin == hwGPIO_Pin_G1) return GPIO_PG1_I2C1SDA;
            break;
        case hwI2C_Index_2:
            if (pin == hwGPIO_Pin_N5) return GPIO_PN5_I2C2SCL;
            if (pin == hwGPIO_Pin_N4) return GPIO_PN4_I2C2SDA;
            break;
        case hwI2C_Index_3:
            if (pin == hwGPIO_Pin_K4) return GPIO_PK4_I2C3SCL;
            if (pin == hwGPIO_Pin_K5) return GPIO_PK5_I2C3SDA;
            break;
        case hwI2C_Index_4:
            if (pin == hwGPIO_Pin_K6) return GPIO_PK6_I2C4SCL;
            if (pin == hwGPIO_Pin_K7) return GPIO_PK7_I2C4SDA;
            break;
        case hwI2C_Index_5:
            if (pin == hwGPIO_Pin_B0) return GPIO_PB0_I2C5SCL;
            if (pin == hwGPIO_Pin_B1) return GPIO_PB1_I2C5SDA;
            break;
        case hwI2C_Index_6:
            if (pin == hwGPIO_Pin_A6) return GPIO_PA6_I2C6SCL;
            if (pin == hwGPIO_Pin_A7) return GPIO_PA7_I2C6SDA;
            break;
        case hwI2C_Index_7:
            if (pin == hwGPIO_Pin_D0) return GPIO_PD0_I2C7SCL;
            if (pin == hwGPIO_Pin_D1) return GPIO_PD1_I2C7SDA;
            break;
        case hwI2C_Index_8:
            if (pin == hwGPIO_Pin_A2) return GPIO_PA2_I2C8SCL;
            if (pin == hwGPIO_Pin_A3) return GPIO_PA3_I2C8SDA;
            break;
        case hwI2C_Index_9:
            if (pin == hwGPIO_Pin_A0) return GPIO_PA0_I2C9SCL;
            if (pin == hwGPIO_Pin_A1) return GPIO_PA1_I2C9SDA;
            break;

        default:
            break;
    }

    return 0;
}

static void I2C0_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_0); }
static void I2C1_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_1); }
static void I2C2_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_2); }
static void I2C3_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_3); }
static void I2C4_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_4); }
static void I2C5_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_5); }
static void I2C6_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_6); }
static void I2C7_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_7); }
static void I2C8_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_8); }
static void I2C9_IRQ_Handler(void) { I2C_IRQ_Process(hwI2C_Index_9); }

static void I2C_NVIC_Init(hwI2C_Index index)
{
    uint32_t irq = I2C_Map_Soc_Int(index);
    if(irq==0)
    {
        return;
    }

    switch (index) {
        case hwI2C_Index_0: IntRegister(irq, I2C0_IRQ_Handler); break;
        case hwI2C_Index_1: IntRegister(irq, I2C1_IRQ_Handler); break;
        case hwI2C_Index_2: IntRegister(irq, I2C2_IRQ_Handler); break;
        case hwI2C_Index_3: IntRegister(irq, I2C3_IRQ_Handler); break;
        case hwI2C_Index_4: IntRegister(irq, I2C4_IRQ_Handler); break;
        case hwI2C_Index_5: IntRegister(irq, I2C5_IRQ_Handler); break;
        case hwI2C_Index_6: IntRegister(irq, I2C6_IRQ_Handler); break;
        case hwI2C_Index_7: IntRegister(irq, I2C7_IRQ_Handler); break;
        case hwI2C_Index_8: IntRegister(irq, I2C8_IRQ_Handler); break;
        case hwI2C_Index_9: IntRegister(irq, I2C9_IRQ_Handler); break;
        default: break;
    }

    MAP_IntEnable(irq);

    return hwI2C_OK;
}

static void I2C_NVIC_DeInit(hwI2C_Index index)
{
    uint32_t irq = I2C_Map_Soc_Int(index);
    if(irq==0)
    {
        return;
    }

    MAP_IntDisable(irq);
}

static void MSP432_I2C_StartNext(hwI2C_Index index)
{
    uint32_t base = I2C_Map_Soc_Base(index);
    MSP432E_I2C_Transfer *t = &i2c_xfer[index];

    if (base == 0) {
        return;
    }

    if (MAP_I2CMasterErr(base) != I2C_MASTER_ERR_NONE) {
        t->error = MAP_I2CMasterErr(base);
        t->state = MSP432_I2C_ERROR;
        MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_DATA);
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
        return;
    }

    if (t->state == MSP432_I2C_TX) {
        if (t->tx_pos >= t->tx_len) {
            t->state = MSP432_I2C_DONE;
            MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_DATA);
            NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
            return;
        }

        MAP_I2CMasterDataPut(base, t->tx_buf[t->tx_pos]);

        if (t->tx_len == 1) {
            if (t->stop) {
                MAP_I2CMasterControl(base, I2C_MASTER_CMD_SINGLE_SEND);
            } else {
                MAP_I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_START);
            }
        } else if (t->tx_pos == 0) {
            MAP_I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_START);
        } else if (t->tx_pos == (uint8_t)(t->tx_len - 1)) {
            if (t->stop) {
                MAP_I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_FINISH);
            } else {
                MAP_I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_CONT);
            }
        } else {
            MAP_I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_CONT);
        }

        t->tx_pos++;
        return;
    }

    if (t->state == MSP432_I2C_RX) {
        if (t->rx_pos > 0 && t->rx_pos <= t->rx_len) {
            t->rx_buf[t->rx_pos - 1] = (uint8_t)MAP_I2CMasterDataGet(base);
        }

        if (t->rx_pos >= t->rx_len) {
            t->state = MSP432_I2C_DONE;
            MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_DATA);
            NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
            return;
        }

        if (t->rx_len == 1) {
            MAP_I2CMasterControl(base, I2C_MASTER_CMD_SINGLE_RECEIVE);
        } else if (t->rx_pos == 0) {
            MAP_I2CMasterControl(base, I2C_MASTER_CMD_BURST_RECEIVE_START);
        } else if (t->rx_pos == (uint8_t)(t->rx_len - 1)) {
            MAP_I2CMasterControl(base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        } else {
            MAP_I2CMasterControl(base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        }

        t->rx_pos++;
        return;
    }
}

static void I2C_IRQ_Process(hwI2C_Index index)
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

    uint32_t status = MAP_I2CMasterIntStatus(base, true);

    if (status) {
        MAP_I2CMasterIntClear(base);
    }

    MSP432_I2C_StartNext(index);
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

    uint32_t i2c_base = I2C_Map_Soc_Base(index);
    uint32_t i2c_periph = I2C_Map_Soc_Periph(index);

    if (i2c_base == 0 || i2c_periph == 0) {
        return hwI2C_InvalidParameter;
    }

    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index].sda_pin;
    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index].scl_pin;

    uint32_t sda_port = GPIO_Map_Soc_Port_Base(sda_pin);
    uint32_t scl_port = GPIO_Map_Soc_Port_Base(scl_pin);
    uint32_t sda_mask = GPIO_Map_Soc_Pin_Mask(sda_pin);
    uint32_t scl_mask = GPIO_Map_Soc_Pin_Mask(scl_pin);

    uint32_t sda_cfg = I2C_Map_PinConfig(index, sda_pin);
    uint32_t scl_cfg = I2C_Map_PinConfig(index, scl_pin);

    if (sda_port == 0 || scl_port == 0 || sda_mask == 0 || scl_mask == 0 || sda_cfg == 0 || scl_cfg == 0) {
        return hwI2C_InvalidParameter;
    }

    GPIO_Enable_Port_Clock(sda_port);
    GPIO_Enable_Port_Clock(scl_port);

    MAP_SysCtlPeripheralEnable(i2c_periph);
    while (!MAP_SysCtlPeripheralReady(i2c_periph));

    MAP_GPIOPinConfigure(scl_cfg);
    MAP_GPIOPinConfigure(sda_cfg);

    MAP_GPIOPinTypeI2CSCL(scl_port, scl_mask);
    MAP_GPIOPinTypeI2C(sda_port, sda_mask);

    MAP_I2CMasterDisable(i2c_base);
    
    switch(speed_mode)
    {
        case hwI2C_Standard_Mode:
            MAP_I2CMasterInitExpClk(i2c_base, g_sys_clock_hz, false);
            break;
        case hwI2C_Fast_Mode:
            MAP_I2CMasterInitExpClk(i2c_base, g_sys_clock_hz, true);
            break;
    }
    
    MAP_I2CMasterEnable(i2c_base);

    MAP_I2CMasterIntDisableEx(i2c_base, I2C_MASTER_INT_DATA);
    MAP_I2CMasterIntClear(i2c_base);

    if (NeonRTOS_SyncObjCreate(&I2C_Master_Done_SyncHandle[index]) != NeonRTOS_OK) {
        return hwI2C_MemoryError;
    }

    memset(&i2c_xfer[index], 0, sizeof(i2c_xfer[index]));
    i2c_xfer[index].state = MSP432_I2C_IDLE;

    I2C_NVIC_Init(index);

    gpio_pin_init_status[sda_pin] = true;
    gpio_pin_init_status[scl_pin] = true;

    I2C_Clock_Speed_Mode[index] = speed_mode;
    I2C_Master_Init_Status[index] = true;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_DeInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index]) {
        return hwI2C_OK;
    }

    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index].sda_pin;
    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index].scl_pin;

    uint32_t sda_port = GPIO_Map_Soc_Port_Base(sda_pin);
    uint32_t scl_port = GPIO_Map_Soc_Port_Base(scl_pin);
    uint32_t sda_mask = GPIO_Map_Soc_Pin_Mask(sda_pin);
    uint32_t scl_mask = GPIO_Map_Soc_Pin_Mask(scl_pin);

    uint32_t i2c_base = I2C_Map_Soc_Base(index);

    if (sda_port == 0 || scl_port == 0 || sda_mask == 0 || scl_mask == 0) {
        return hwI2C_InvalidParameter;
    }

    I2C_Master_Init_Status[index] = false;

    MAP_I2CMasterIntDisableEx(i2c_base, I2C_MASTER_INT_DATA);
    MAP_I2CMasterIntClear(i2c_base);
    
    I2C_NVIC_DeInit(index);

    MAP_I2CMasterDisable(i2c_base);

    NeonRTOS_SyncObjDelete(&I2C_Master_Done_SyncHandle[index]);

    MAP_GPIOPinTypeGPIOInput(sda_port, sda_mask);
    MAP_GPIOPinTypeGPIOInput(scl_port, scl_mask);

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
    if (ret < hwI2C_OK) {
        return ret;
    }

    return I2C_Master_Init(index, speed);
}

hwI2C_OpResult I2C_Master_Read(hwI2C_Index index, uint8_t address, uint8_t *read_dat, uint8_t read_len, bool stop, NeonRTOS_Time_t timeoutMs)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index]) {
        return hwI2C_NotInit;
    }

    if (read_dat == NULL || read_len == 0) {
        return hwI2C_InvalidParameter;
    }

    uint32_t base = I2C_Map_Soc_Base(index);
    if (base == 0) {
        return hwI2C_InvalidParameter;
    }

    MSP432E_I2C_Transfer *t = &i2c_xfer[index];

    if (t->state == MSP432_I2C_TX || t->state == MSP432_I2C_RX) {
        return hwI2C_BusError;
    }

    memset(t, 0, sizeof(*t));

    t->state = MSP432_I2C_RX;
    t->addr = address;
    t->rx_buf = read_dat;
    t->rx_len = read_len;
    t->rx_pos = 0;
    t->stop = stop;

    MAP_I2CMasterSlaveAddrSet(base, address, true);
    MAP_I2CMasterIntClear(base);
    MAP_I2CMasterIntEnableEx(base, I2C_MASTER_INT_DATA);

    MSP432_I2C_StartNext(index);

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK) {
        MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_DATA);
        t->state = MSP432_I2C_ERROR;
        return hwI2C_SlaveTimeout;
    }

    return (t->state == MSP432_I2C_DONE) ? hwI2C_OK : hwI2C_BusError;
}

hwI2C_OpResult I2C_Master_Write(hwI2C_Index index, uint8_t address, uint8_t *write_dat, uint8_t write_len, bool stop, NeonRTOS_Time_t timeoutMs)
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

    MSP432E_I2C_Transfer *t = &i2c_xfer[index];

    if (t->state == MSP432_I2C_TX || t->state == MSP432_I2C_RX) {
        return hwI2C_BusError;
    }

    memset(t, 0, sizeof(*t));

    t->state = MSP432_I2C_TX;
    t->addr = address;
    t->tx_buf = write_dat;
    t->tx_len = write_len;
    t->tx_pos = 0;
    t->stop = stop;

    MAP_I2CMasterSlaveAddrSet(base, address, false);
    MAP_I2CMasterIntClear(base);
    MAP_I2CMasterIntEnableEx(base, I2C_MASTER_INT_DATA);

    MSP432_I2C_StartNext(index);

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK) {
        MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_DATA);
        t->state = MSP432_I2C_ERROR;
        return hwI2C_SlaveTimeout;
    }

    return (t->state == MSP432_I2C_DONE) ? hwI2C_OK : hwI2C_BusError;
}

bool I2C_Master_isInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return false;
    }

    return I2C_Master_Init_Status[index];
}

#endif //DEVICE_TIMSP432E