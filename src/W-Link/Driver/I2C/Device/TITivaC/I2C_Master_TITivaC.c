#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#ifdef DEVICE_TITIVAC

#include "GPIO/Device/TITivaC/GPIO_TITivaC.h"

#include "I2C/Pin/TITivaC/I2C_Pin_TITivaC.h"

#include "I2C_Master_TITivaC.h"

typedef enum {
    TIVA_I2C_IDLE = 0,
    TIVA_I2C_TX,
    TIVA_I2C_RX,
    TIVA_I2C_DONE,
    TIVA_I2C_ERROR
} TIVA_I2C_State;

typedef struct {
    TIVA_I2C_State state;

    uint8_t addr;

    uint8_t *tx_buf;
    uint8_t tx_len;
    uint8_t tx_pos;

    uint8_t *rx_buf;
    uint8_t rx_len;
    uint8_t rx_pos;

    bool stop;
    int error;
} TIVA_I2C_Transfer;

bool I2C_Master_Init_Status[hwI2C_Index_MAX] = {false};

static hwI2C_Speed_Mode I2C_Clock_Speed_Mode[hwI2C_Index_MAX] = {
    hwI2C_Standard_Mode
};

static NeonRTOS_SyncObj_t I2C_Master_Done_SyncHandle[hwI2C_Index_MAX];
static TIVA_I2C_Transfer i2c_xfer[hwI2C_Index_MAX];

static void TIVA_I2C_StartNext(hwI2C_Index index)
{
    uint32_t base = I2C_Map_Soc_Base(index);
    TIVA_I2C_Transfer *t = &i2c_xfer[index];

    if (base == 0) {
        return;
    }

    if (MAP_I2CMasterErr(base) != I2C_MASTER_ERR_NONE) {
        t->error = MAP_I2CMasterErr(base);
        t->state = TIVA_I2C_ERROR;
        MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_DATA);
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
        return;
    }

    if (t->state == TIVA_I2C_TX) {
        if (t->tx_pos >= t->tx_len) {
            t->state = TIVA_I2C_DONE;
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

    if (t->state == TIVA_I2C_RX) {
        if (t->rx_pos > 0 && t->rx_pos <= t->rx_len) {
            t->rx_buf[t->rx_pos - 1] = (uint8_t)MAP_I2CMasterDataGet(base);
        }

        if (t->rx_pos >= t->rx_len) {
            t->state = TIVA_I2C_DONE;
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

void TIVA_I2C_IRQ_Process(hwI2C_Index index)
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

    TIVA_I2C_StartNext(index);
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
            MAP_I2CMasterInitExpClk(i2c_base, MAP_SysCtlClockGet(), false);
            break;
        case hwI2C_Fast_Mode:
            MAP_I2CMasterInitExpClk(i2c_base, MAP_SysCtlClockGet(), true);
            break;
    }
    
    MAP_I2CMasterEnable(i2c_base);

    MAP_I2CMasterIntDisableEx(i2c_base, I2C_MASTER_INT_DATA);
    MAP_I2CMasterIntClear(i2c_base);

    if (NeonRTOS_SyncObjCreate(&I2C_Master_Done_SyncHandle[index]) != NeonRTOS_OK) {
        return hwI2C_MemoryError;
    }

    memset(&i2c_xfer[index], 0, sizeof(i2c_xfer[index]));
    i2c_xfer[index].state = TIVA_I2C_IDLE;

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

    if (sda_port && sda_mask) {
        MAP_GPIOPinTypeGPIOInput(sda_port, sda_mask);
    }

    if (scl_port && scl_mask) {
        MAP_GPIOPinTypeGPIOInput(scl_port, scl_mask);
    }

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

hwI2C_OpResult I2C_Master_Read(
    hwI2C_Index index,
    uint8_t address,
    uint8_t *read_dat,
    uint8_t read_len,
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

    if (read_dat == NULL || read_len == 0) {
        return hwI2C_InvalidParameter;
    }

    uint32_t base = I2C_Map_Soc_Base(index);
    if (base == 0) {
        return hwI2C_InvalidParameter;
    }

    TIVA_I2C_Transfer *t = &i2c_xfer[index];

    if (t->state == TIVA_I2C_TX || t->state == TIVA_I2C_RX) {
        return hwI2C_BusError;
    }

    memset(t, 0, sizeof(*t));

    t->state = TIVA_I2C_RX;
    t->addr = address;
    t->rx_buf = read_dat;
    t->rx_len = read_len;
    t->rx_pos = 0;
    t->stop = stop;

    MAP_I2CMasterSlaveAddrSet(base, address, true);
    MAP_I2CMasterIntClear(base);
    MAP_I2CMasterIntEnableEx(base, I2C_MASTER_INT_DATA);

    TIVA_I2C_StartNext(index);

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK) {
        MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_DATA);
        t->state = TIVA_I2C_ERROR;
        return hwI2C_SlaveTimeout;
    }

    return (t->state == TIVA_I2C_DONE) ? hwI2C_OK : hwI2C_BusError;
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

    TIVA_I2C_Transfer *t = &i2c_xfer[index];

    if (t->state == TIVA_I2C_TX || t->state == TIVA_I2C_RX) {
        return hwI2C_BusError;
    }

    memset(t, 0, sizeof(*t));

    t->state = TIVA_I2C_TX;
    t->addr = address;
    t->tx_buf = write_dat;
    t->tx_len = write_len;
    t->tx_pos = 0;
    t->stop = stop;

    MAP_I2CMasterSlaveAddrSet(base, address, false);
    MAP_I2CMasterIntClear(base);
    MAP_I2CMasterIntEnableEx(base, I2C_MASTER_INT_DATA);

    TIVA_I2C_StartNext(index);

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK) {
        MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_DATA);
        t->state = TIVA_I2C_ERROR;
        return hwI2C_SlaveTimeout;
    }

    return (t->state == TIVA_I2C_DONE) ? hwI2C_OK : hwI2C_BusError;
}

bool I2C_Master_isInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return false;
    }

    return I2C_Master_Init_Status[index];
}

#endif