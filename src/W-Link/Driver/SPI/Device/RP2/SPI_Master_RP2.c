#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "SPI/SPI_Master.h"

#ifdef DEVICE_RP2

#include "GPIO/Device/RP2/GPIO_RP2.h"

#include "SPI/Pin/RP2/SPI_Pin_RP2.h"

#include "SPI_Master_RP2.h"

#define SPI_MASTER_SYNC_TIMEOUT             100
#define SPI_MASTER_MUTEX_ACCESS_TIMEOUT     500
#define SPI_MASTER_OP_TIMEOUT               3000

typedef enum {
    RP2_SPI_IDLE = 0,
    RP2_SPI_TX,
    RP2_SPI_RX,
    RP2_SPI_TXRX,
    RP2_SPI_DONE,
    RP2_SPI_ERROR
} RP2_SPI_State;

typedef struct {
    RP2_SPI_State state;
    const uint8_t *tx_buf;
    uint8_t *rx_buf;
    uint32_t len;
    uint32_t tx_pos;
    uint32_t rx_pos;
} RP2_SPI_Transfer;

NeonRTOS_LockObj_t Spi_Master_Access_Mutex[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Send_SyncHandle[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Recv_SyncHandle[hwSPI_Index_MAX];

#define SPI_MASTER_MUTEX_LOCK(a, b)  if (NeonRTOS_LockObjLock(&Spi_Master_Access_Mutex[a], b) != NeonRTOS_OK) { return hwSPI_MutexTimeout; }
#define SPI_MASTER_MUTEX_UNLOCK(a)   NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[a])

bool Spi_Master_Init_Status[hwSPI_Index_MAX] = {false};

static bool Spi_Master_Use_CS[hwSPI_Index_MAX] = {false};

static RP2_SPI_Transfer spi_xfer[hwSPI_Index_MAX];

static spi_inst_t *SPI_Map_Soc_Base(hwSPI_Index index)
{
    switch (index) {
        case hwSPI_Index_0: return spi0;
        case hwSPI_Index_1: return spi1;
        default: return NULL;
    }
}

static void RP2_SPI_IRQ_Process(hwSPI_Index index)
{
    spi_inst_t *inst = SPI_Map_Soc_Base(index);
    if (!inst) return;

    spi_hw_t *hw = spi_get_hw(inst);
    RP2_SPI_Transfer *t = &spi_xfer[index];

    if (t->state == RP2_SPI_IDLE ||
        t->state == RP2_SPI_DONE ||
        t->state == RP2_SPI_ERROR) {
        hw->imsc = 0;
        return;
    }

    while ((hw->sr & SPI_SSPSR_TNF_BITS) && t->tx_pos < t->len) {
        uint8_t out = 0xFF;

        if ((t->state == RP2_SPI_TX || t->state == RP2_SPI_TXRX) &&
            t->tx_buf != NULL) {
            out = t->tx_buf[t->tx_pos];
        }

        hw->dr = out;
        t->tx_pos++;
    }

    while ((hw->sr & SPI_SSPSR_RNE_BITS) && t->rx_pos < t->len) {
        uint8_t in = (uint8_t)hw->dr;

        if ((t->state == RP2_SPI_RX || t->state == RP2_SPI_TXRX) &&
            t->rx_buf != NULL) {
            t->rx_buf[t->rx_pos] = in;
        }

        t->rx_pos++;
    }

    if (t->rx_pos >= t->len && t->tx_pos >= t->len) {
        while (hw->sr & SPI_SSPSR_BSY_BITS) {}

        hw->imsc = 0;
        t->state = RP2_SPI_DONE;

        NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[index]);
        NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[index]);
    }
}

static void SPI0_IRQ_Handler(void)
{
    RP2_SPI_IRQ_Process(hwSPI_Index_0);
}

static void SPI1_IRQ_Handler(void)
{
    RP2_SPI_IRQ_Process(hwSPI_Index_1);
}

hwSPI_OpResult SPI_Master_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode, bool cs)
{
    if (opMode >= hwSPI_OpMode_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == true)
    {
        return hwSPI_OK;
    }

    spi_inst_t *inst = SPI_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwSPI_InvalidParameter;
    }

    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].miso_pin;
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].mosi_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].cs_pin;

    gpio_set_function(miso_pin, GPIO_FUNC_SPI);
    gpio_set_function(mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(sclk_pin, GPIO_FUNC_SPI);

    if (cs_pin != hwGPIO_Pin_NC && cs) {
        gpio_set_function(cs_pin, GPIO_FUNC_SPI);
        Spi_Master_Use_CS[index] = true;
    }

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Send_SyncHandle[index]) != NeonRTOS_OK)
    {
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Recv_SyncHandle[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_LockObjCreate(&Spi_Master_Access_Mutex[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
        return hwSPI_MemoryError;
    }

    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[index]);

    spi_init(inst, clock_rate_hz);

    switch (opMode) {
        case hwSPI_OpMode_Polarity0_Phase0:
            spi_set_format(inst, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
            break;
        case hwSPI_OpMode_Polarity0_Phase1:
            spi_set_format(inst, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
            break;
        case hwSPI_OpMode_Polarity1_Phase0:
            spi_set_format(inst, 8, SPI_CPOL_1, SPI_CPHA_0, SPI_MSB_FIRST);
            break;
        case hwSPI_OpMode_Polarity1_Phase1:
            spi_set_format(inst, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
            break;
    }

    switch(index)
    {
        case hwSPI_Index_0:
            irq_set_exclusive_handler(SPI0_IRQ, SPI0_IRQ_Handler);
            irq_set_enabled(SPI0_IRQ, true);
            break;
        case hwSPI_Index_1:
            irq_set_exclusive_handler(SPI1_IRQ, SPI1_IRQ_Handler);
            irq_set_enabled(SPI1_IRQ, true);
            break;
    }

    spi_get_hw(inst)->imsc = 0;

    gpio_pin_init_status[miso_pin] = true;
    gpio_pin_init_status[mosi_pin] = true;
    gpio_pin_init_status[sclk_pin] = true;

    if (cs_pin != hwGPIO_Pin_NC && cs)
    {
        gpio_pin_init_status[cs_pin] = true;
        Spi_Master_Use_CS[index] = true;
    }

    Spi_Master_Init_Status[index] = true;

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_DeInit(hwSPI_Index index)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_OK;
    }

    spi_inst_t *inst = SPI_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwSPI_InvalidParameter;
    }

    spi_get_hw(inst)->imsc = 0;
    
    switch(index)
    {
        case hwSPI_Index_0:
            irq_set_enabled(SPI0_IRQ, false);
            break;
        case hwSPI_Index_1:
            irq_set_enabled(SPI1_IRQ, false);
            break;
    }

    Spi_Master_Init_Status[index] = false;
    
    spi_deinit(inst);

    NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);
    NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);

    memset(&spi_xfer[index], 0, sizeof(spi_xfer[index]));

    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].miso_pin;
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].mosi_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].cs_pin;

    gpio_deinit(miso_pin);
    gpio_deinit(mosi_pin);
    gpio_deinit(sclk_pin);

    if (Spi_Master_Use_CS[index] == true)
    {
        gpio_deinit(cs_pin);
    }

    gpio_pin_init_status[miso_pin] = false;
    gpio_pin_init_status[mosi_pin] = false;
    gpio_pin_init_status[sclk_pin] = false;

    if (Spi_Master_Use_CS[index] == true)
    {
        gpio_pin_init_status[cs_pin] = false;
        Spi_Master_Use_CS[index] = false;
    }

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Frequency(hwSPI_Index index, uint32_t clock_rate_hz)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    spi_inst_t *inst = SPI_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    spi_set_baudrate(inst, clock_rate_hz);

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Mode(hwSPI_Index index, hwSPI_OpMode opMode)
{
    if (opMode >= hwSPI_OpMode_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    spi_inst_t *inst = SPI_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    switch (opMode) {
        case hwSPI_OpMode_Polarity0_Phase0:
            spi_set_format(inst, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
            break;
        case hwSPI_OpMode_Polarity0_Phase1:
            spi_set_format(inst, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
            break;
        case hwSPI_OpMode_Polarity1_Phase0:
            spi_set_format(inst, 8, SPI_CPOL_1, SPI_CPHA_0, SPI_MSB_FIRST);
            break;
        case hwSPI_OpMode_Polarity1_Phase1:
            spi_set_format(inst, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
            break;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_WriteByte(hwSPI_Index index, uint8_t dat)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    spi_inst_t *inst = SPI_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    spi_hw_t *hw = spi_get_hw(inst);
    RP2_SPI_Transfer *t = &spi_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = RP2_SPI_TX;
    t->tx_buf = &dat;
    t->rx_buf = NULL;
    t->len    = 1;

    while (hw->sr & SPI_SSPSR_RNE_BITS) {
        (void)hw->dr;
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);

    hw->imsc =
        SPI_SSPIMSC_RTIM_BITS |
        SPI_SSPIMSC_TXIM_BITS;

    switch(index)
    {
        case hwSPI_Index_0:
            irq_set_pending(SPI0_IRQ);
            break;
        case hwSPI_Index_1:
            irq_set_pending(SPI1_IRQ);
            break;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_SYNC_TIMEOUT) != NeonRTOS_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_ReadByte(hwSPI_Index index, uint8_t* dat)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (dat == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    spi_inst_t *inst = SPI_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    spi_hw_t *hw = spi_get_hw(inst);
    RP2_SPI_Transfer *t = &spi_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = RP2_SPI_RX;
    t->tx_buf = NULL;
    t->rx_buf = dat;
    t->len    = 1;

    while (hw->sr & SPI_SSPSR_RNE_BITS) {
        (void)hw->dr;
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    hw->imsc =
        SPI_SSPIMSC_RXIM_BITS |
        SPI_SSPIMSC_RTIM_BITS;

    switch(index)
    {
        case hwSPI_Index_0:
            irq_set_pending(SPI0_IRQ);
            break;
        case hwSPI_Index_1:
            irq_set_pending(SPI1_IRQ);
            break;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_SYNC_TIMEOUT) != NeonRTOS_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_TransferByte(hwSPI_Index index, uint8_t wr_dat, uint8_t* rd_dat)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (rd_dat == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    spi_inst_t *inst = SPI_Map_Soc_Base(index);
    if(inst==NULL) {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    spi_hw_t *hw = spi_get_hw(inst);
    RP2_SPI_Transfer *t = &spi_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = RP2_SPI_TXRX;
    t->tx_buf = &wr_dat;
    t->rx_buf = rd_dat;
    t->len    = 1;

    while (hw->sr & SPI_SSPSR_RNE_BITS) {
        (void)hw->dr;
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    hw->imsc =
        SPI_SSPIMSC_RXIM_BITS |
        SPI_SSPIMSC_RTIM_BITS |
        SPI_SSPIMSC_TXIM_BITS;

    switch(index)
    {
        case hwSPI_Index_0:
            irq_set_pending(SPI0_IRQ);
            break;
        case hwSPI_Index_1:
            irq_set_pending(SPI1_IRQ);
            break;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_SYNC_TIMEOUT) != NeonRTOS_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_SYNC_TIMEOUT) != NeonRTOS_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_DummyByte(hwSPI_Index index)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    return SPI_Master_WriteByte(index, 0x00);
}

#endif // DEVICE_RP2