#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#ifdef DEVICE_RP2

#include "GPIO/Device/RP2/GPIO_RP2.h"

#include "I2C/Pin/RP2/I2C_Pin_RP2.h"

#include "I2C_Master_RP2.h"

typedef enum {
    RP2_I2C_IDLE = 0,
    RP2_I2C_TX,
    RP2_I2C_RX,
    RP2_I2C_DONE,
    RP2_I2C_ERROR
} RP2_I2C_State;

typedef struct {
    RP2_I2C_State state;

    uint8_t addr;

    uint8_t *tx_buf;
    uint8_t tx_len;
    uint8_t tx_pos;

    uint8_t *rx_buf;
    uint8_t rx_len;
    uint8_t rx_pos;

    bool stop;
    int error;
} RP2_I2C_Transfer;

bool I2C_Master_Init_Status[hwI2C_Index_MAX] = {false};

static hwI2C_Speed_Mode I2C_Clock_Speed_Mode[hwI2C_Index_MAX] = {hwI2C_Standard_Mode};
static NeonRTOS_SyncObj_t I2C_Master_Done_SyncHandle[hwI2C_Index_MAX];

static RP2_I2C_Transfer i2c_xfer[hwI2C_Index_MAX];

static i2c_inst_t *I2C_Map_Soc_Base(hwI2C_Index index)
{
    switch (index)
    {
        case hwI2C_Index_0: return i2c0;
        case hwI2C_Index_1: return i2c1;
        default: break;
    }
    return NULL;
}

static void RP2_I2C_IRQ_Process(hwI2C_Index index)
{
    i2c_inst_t *inst = I2C_Map_Soc_Base(index);
    if(inst==NULL) {
        return;
    }

    i2c_hw_t *hw = i2c_get_hw(inst);
    RP2_I2C_Transfer *t = &i2c_xfer[index];

    uint32_t intr = hw->intr_stat;

    if (intr & I2C_IC_INTR_STAT_R_TX_ABRT_BITS) {
        t->error = hw->tx_abrt_source;
        (void)hw->clr_tx_abrt;
        t->state = RP2_I2C_ERROR;
        hw->intr_mask = 0;
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
        return;
    }

    if (t->state == RP2_I2C_TX) {
        while ((hw->status & I2C_IC_STATUS_TFNF_BITS) &&
               t->tx_pos < t->tx_len) {

            uint32_t cmd = t->tx_buf[t->tx_pos++];

            if (t->tx_pos == t->tx_len && t->stop) {
                cmd |= I2C_IC_DATA_CMD_STOP_BITS;
            }

            hw->data_cmd = cmd;
        }

        if (t->tx_pos >= t->tx_len) {
            hw->intr_mask = I2C_IC_INTR_MASK_M_STOP_DET_BITS |
                            I2C_IC_INTR_MASK_M_TX_ABRT_BITS;
        }
    }

    if (t->state == RP2_I2C_RX) {
        while ((hw->status & I2C_IC_STATUS_TFNF_BITS) &&
               t->tx_pos < t->rx_len) {

            uint32_t cmd = I2C_IC_DATA_CMD_CMD_BITS;

            if (t->tx_pos == t->rx_len - 1 && t->stop) {
                cmd |= I2C_IC_DATA_CMD_STOP_BITS;
            }

            hw->data_cmd = cmd;
            t->tx_pos++;
        }

        while ((hw->status & I2C_IC_STATUS_RFNE_BITS) &&
               t->rx_pos < t->rx_len) {
            t->rx_buf[t->rx_pos++] = (uint8_t)hw->data_cmd;
        }
    }

    if (intr & I2C_IC_INTR_STAT_R_STOP_DET_BITS) {
        (void)hw->clr_stop_det;

        if (t->state == RP2_I2C_RX) {
            while ((hw->status & I2C_IC_STATUS_RFNE_BITS) &&
                   t->rx_pos < t->rx_len) {
                t->rx_buf[t->rx_pos++] = (uint8_t)hw->data_cmd;
            }
        }

        if (t->state == RP2_I2C_RX && t->rx_pos < t->rx_len) {
            t->state = RP2_I2C_ERROR;
        } else {
            t->state = RP2_I2C_DONE;
        }

        hw->intr_mask = 0;
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
    }
}

static void I2C0_IRQ_Handler(void)
{
    RP2_I2C_IRQ_Process(hwI2C_Index_0);
}

static void I2C1_IRQ_Handler(void)
{
    RP2_I2C_IRQ_Process(hwI2C_Index_1);
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

    i2c_inst_t *inst = I2C_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwI2C_InvalidParameter;
    }

    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]].sda_pin;
    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]].scl_pin;

    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    uint baud = I2C_MASTER_STANDARD_MODE_CLK_FREQUENCY;

    switch (speed_mode) {
        case hwI2C_Standard_Mode:
            baud = I2C_MASTER_STANDARD_MODE_CLK_FREQUENCY;
            break;

        case hwI2C_Fast_Mode:
            baud = I2C_MASTER_FAST_MODE_CLK_FREQUENCY;
            break;

        case hwI2C_High_Speed_Mode:
            baud = I2C_MASTER_HIGH_SPEED_MODE_CLK_FREQUENCY;
            break;
    }

    i2c_init(inst, baud);

    if (NeonRTOS_SyncObjCreate(&I2C_Master_Done_SyncHandle[index]) != NeonRTOS_OK) {
        return hwI2C_MemoryError;
    }

    switch(index)
    {
        case hwI2C_Index_0:
            irq_set_exclusive_handler(I2C0_IRQ, I2C0_IRQ_Handler);
            irq_set_enabled(I2C0_IRQ, true);
            break;
        case hwI2C_Index_1:
            irq_set_exclusive_handler(I2C1_IRQ, I2C1_IRQ_Handler);
            irq_set_enabled(I2C1_IRQ, true);
            break;
    }

    i2c_get_hw(inst)->intr_mask = 0;

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

    if (I2C_Master_Init_Status[index] == false) {
        return hwI2C_OK;
    }

    i2c_inst_t *inst = I2C_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwI2C_InvalidParameter;
    }

    I2C_Master_Init_Status[index] = false;

    i2c_get_hw(inst)->intr_mask = 0;

    switch(index)
    {
        case hwI2C_Index_0:
            irq_set_enabled(I2C0_IRQ, false);
            break;
        case hwI2C_Index_1:
            irq_set_enabled(I2C1_IRQ, false);
            break;
    }

    i2c_deinit(inst);

    NeonRTOS_SyncObjDelete(&I2C_Master_Done_SyncHandle[index]);

    memset(&i2c_xfer[index], 0, sizeof(i2c_xfer[index]));

    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]].sda_pin;
    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]].scl_pin;

    gpio_deinit(sda_pin);
    gpio_deinit(scl_pin);

    gpio_pin_init_status[sda_pin] = false;
    gpio_pin_init_status[scl_pin] = false;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_Reset(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }
    if (I2C_Master_Init_Status[index] == false) {
        return hwI2C_NotInit;
    }

    hwI2C_OpResult op_status = I2C_Master_DeInit(index);
    if (op_status < hwI2C_OK) {
        return op_status;
    }

    return I2C_Master_Init(index, I2C_Clock_Speed_Mode[index]);
}

hwI2C_OpResult I2C_Master_Read(hwI2C_Index index, uint8_t address, uint8_t *read_dat, uint8_t read_len, bool stop, NeonRTOS_Time_t timeoutMs)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (I2C_Master_Init_Status[index] == false) {
        return hwI2C_NotInit;
    }

    if (read_dat == NULL && read_len > 0) {
        return hwI2C_InvalidParameter;
    }

    if (read_len == 0) {
        return hwI2C_InvalidParameter;
    }

    i2c_inst_t *inst = I2C_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwI2C_InvalidParameter;
    }

    if (i2c_xfer[index].state == RP2_I2C_TX ||
        i2c_xfer[index].state == RP2_I2C_RX) {
        return hwI2C_BusError;
    }

    i2c_hw_t *hw = i2c_get_hw(inst);

    RP2_I2C_Transfer *t = &i2c_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = RP2_I2C_RX;
    t->addr   = address;
    t->rx_buf = read_dat;
    t->rx_len = read_len;
    t->stop   = stop;

    hw->enable = 0;
    hw->tar = address;
    hw->enable = 1;

    (void)hw->clr_intr;
    hw->intr_mask =
        I2C_IC_INTR_MASK_M_RX_FULL_BITS |
        I2C_IC_INTR_MASK_M_TX_EMPTY_BITS |
        I2C_IC_INTR_MASK_M_STOP_DET_BITS |
        I2C_IC_INTR_MASK_M_TX_ABRT_BITS;

    switch(index)
    {
        case hwI2C_Index_0:
            irq_set_pending(I2C0_IRQ);
            break;
        case hwI2C_Index_1:
            irq_set_pending(I2C1_IRQ);
            break;
    }

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK) {
        hw->intr_mask = 0;
        t->state = RP2_I2C_ERROR;
        
        switch(index)
        {
            case hwI2C_Index_0:
                irq_clear(I2C0_IRQ);
                break;
            case hwI2C_Index_1:
                irq_clear(I2C1_IRQ);
                break;
        }

        return hwI2C_SlaveTimeout;
    }

    return (t->state == RP2_I2C_DONE) ? hwI2C_OK : hwI2C_BusError;
}

hwI2C_OpResult I2C_Master_Write(hwI2C_Index index, uint8_t address, uint8_t *write_dat, uint8_t write_len, bool stop, NeonRTOS_Time_t timeoutMs)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (I2C_Master_Init_Status[index] == false) {
        return hwI2C_NotInit;
    }

    if (write_dat == NULL && write_len > 0) {
        return hwI2C_InvalidParameter;
    }

    if (write_len == 0) {
        return hwI2C_InvalidParameter;
    }

    i2c_inst_t *inst = I2C_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwI2C_InvalidParameter;
    }

    if (i2c_xfer[index].state == RP2_I2C_TX ||
        i2c_xfer[index].state == RP2_I2C_RX) {
        return hwI2C_BusError;
    }
    
    i2c_hw_t *hw = i2c_get_hw(inst);

    RP2_I2C_Transfer *t = &i2c_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = RP2_I2C_TX;
    t->addr   = address;
    t->tx_buf = write_dat;
    t->tx_len = write_len;
    t->stop   = stop;

    hw->enable = 0;
    hw->tar = address;
    hw->enable = 1;

    (void)hw->clr_intr;
    hw->intr_mask =
        I2C_IC_INTR_MASK_M_TX_EMPTY_BITS |
        I2C_IC_INTR_MASK_M_STOP_DET_BITS |
        I2C_IC_INTR_MASK_M_TX_ABRT_BITS;

    switch(index)
    {
        case hwI2C_Index_0:
            irq_set_pending(I2C0_IRQ);
            break;
        case hwI2C_Index_1:
            irq_set_pending(I2C1_IRQ);
            break;
    }

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK) {
        hw->intr_mask = 0;
        t->state = RP2_I2C_ERROR;

        switch(index)
        {
            case hwI2C_Index_0:
                irq_clear(I2C0_IRQ);
                break;
            case hwI2C_Index_1:
                irq_clear(I2C1_IRQ);
                break;
        }

        return hwI2C_SlaveTimeout;
    }

    return (t->state == RP2_I2C_DONE) ? hwI2C_OK : hwI2C_BusError;
}

bool I2C_Master_isInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return false;
    }
    return I2C_Master_Init_Status[index];
}

#endif // DEVICE_RP2