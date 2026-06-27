
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "QSPI/QSPI_Master.h"

#ifdef DEVICE_TITIVAC

#include "GPIO/Device/TITivaC/GPIO_TITivaC.h"
#include "QSPI/Pin/TITivaC/QSPI_Pin_TITivaC.h"
#include "QSPI_Master_TITivaC.h"

#define QSPI_MASTER_MUTEX_ACCESS_TIMEOUT    500
#define QSPI_MASTER_OP_TIMEOUT              3000

typedef enum {
    QSPI_STATE_IDLE = 0,
    QSPI_STATE_TX,
    QSPI_STATE_RX,
    QSPI_STATE_DONE,
    QSPI_STATE_ERROR
} QSPI_State;

typedef struct {
    QSPI_State state;
    const uint8_t *tx_buf;
    uint8_t *rx_buf;
    uint32_t len;
    uint32_t tx_pos;
    uint32_t rx_pos;
} QSPI_Transfer;

NeonRTOS_LockObj_t Qspi_Master_Access_Mutex[hwQSPI_Index_MAX];
NeonRTOS_SyncObj_t Qspi_Master_Send_SyncHandle[hwQSPI_Index_MAX];
NeonRTOS_SyncObj_t Qspi_Master_Recv_SyncHandle[hwQSPI_Index_MAX];

#define QSPI_MASTER_MUTEX_LOCK(a, b) \
    if (NeonRTOS_LockObjLock(&Qspi_Master_Access_Mutex[a], b) != NeonRTOS_OK) { return hwQSPI_MutexTimeout; }

#define QSPI_MASTER_MUTEX_UNLOCK(a) \
    NeonRTOS_LockObjUnlock(&Qspi_Master_Access_Mutex[a])

bool Qspi_Master_Init_Status[hwQSPI_Index_MAX] = {false};

static bool Qspi_Master_Use_CS[hwQSPI_Index_MAX] = {false};

static QSPI_Transfer qspi_xfer[hwQSPI_Index_MAX];

static uint32_t Qspi_Master_Clock_Hz[hwQSPI_Index_MAX] = {0};
static hwQSPI_OpMode Qspi_Master_Mode[hwQSPI_Index_MAX];

static uint32_t QSPI_Map_Mode(hwQSPI_OpMode opMode)
{
    switch (opMode)
    {
        case hwQSPI_OpMode_Polarity0_Phase0:
            return SSI_FRF_MOTO_MODE_0;

        case hwQSPI_OpMode_Polarity1_Phase1:
            return SSI_FRF_MOTO_MODE_3;

        default:
            return SSI_FRF_MOTO_MODE_0;
    }
}

static void QSPI_Flush_RX(uint32_t base)
{
    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(base, &dummy));
}

void QSPI_IRQ_Process(hwQSPI_Index index)
{
    uint32_t base = QSPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return;
    }

    uint32_t status = MAP_SSIIntStatus(base, true);

    if (status != 0)
    {
        MAP_SSIIntClear(base, status);
    }

    QSPI_Transfer *t = &qspi_xfer[index];

    if (t->state == QSPI_STATE_IDLE ||
        t->state == QSPI_STATE_DONE ||
        t->state == QSPI_STATE_ERROR)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        return;
    }

    if (status & SSI_RXOR)
    {
        t->state = QSPI_STATE_ERROR;

        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

        NeonRTOS_SyncObjSignalFromISR(&Qspi_Master_Recv_SyncHandle[index]);
        NeonRTOS_SyncObjSignalFromISR(&Qspi_Master_Send_SyncHandle[index]);
        return;
    }

    uint32_t rx;

    while (MAP_SSIDataGetNonBlocking(base, &rx))
    {
        if (t->rx_pos < t->len)
        {
            if (t->state == QSPI_STATE_RX && t->rx_buf != NULL)
            {
                t->rx_buf[t->rx_pos] = (uint8_t)(rx & 0xFF);
            }

            t->rx_pos++;
        }
    }

    while (t->tx_pos < t->len)
    {
        uint8_t out = 0xFF;

        if (t->state == QSPI_STATE_TX && t->tx_buf != NULL)
        {
            out = t->tx_buf[t->tx_pos];
        }

        if ((t->tx_pos + 1) >= t->len)
        {
            if (!MAP_SSIAdvDataPutFrameEndNonBlocking(base, out))
            {
                break;
            }
        }
        else
        {
            if (!MAP_SSIDataPutNonBlocking(base, out))
            {
                break;
            }
        }

        t->tx_pos++;
    }

    if (t->rx_pos >= t->len && t->tx_pos >= t->len)
    {
        t->state = QSPI_STATE_DONE;

        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        MAP_SSIAdvFrameHoldDisable(base);
        MAP_SSIAdvModeSet(base, SSI_ADV_MODE_READ_WRITE);

        NeonRTOS_SyncObjSignalFromISR(&Qspi_Master_Recv_SyncHandle[index]);
        NeonRTOS_SyncObjSignalFromISR(&Qspi_Master_Send_SyncHandle[index]);
    }
}

hwQSPI_OpResult QSPI_Master_Init(hwQSPI_Index index, uint32_t clock_rate_hz, hwQSPI_OpMode opMode, bool cs)
{
    if (index >= hwQSPI_Index_MAX || opMode >= hwQSPI_OpMode_MAX || clock_rate_hz == 0)
    {
        return hwQSPI_InvalidParameter;
    }

    if (Qspi_Master_Init_Status[index])
    {
        return hwQSPI_OK;
    }

    uint32_t ssi_base = SPI_Map_Soc_Base(index);
    uint32_t ssi_periph = SPI_Map_Soc_Periph(index);

    if (ssi_base == 0 || ssi_periph == 0)
    {
        return hwSPI_InvalidParameter;
    }

    hwGPIO_Pin io0_pin  = QSPI_Pin_Def_Table[index].io0_pin;
    hwGPIO_Pin io1_pin  = QSPI_Pin_Def_Table[index].io1_pin;
    hwGPIO_Pin io2_pin  = QSPI_Pin_Def_Table[index].io2_pin;
    hwGPIO_Pin io3_pin  = QSPI_Pin_Def_Table[index].io3_pin;
    hwGPIO_Pin sclk_pin = QSPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin   = QSPI_Pin_Def_Table[index].cs_pin;

    uint32_t io0_cfg  = QSPI_Map_PinConfig(index, io0_pin);
    uint32_t io1_cfg  = QSPI_Map_PinConfig(index, io1_pin);
    uint32_t io2_cfg  = QSPI_Map_PinConfig(index, io2_pin);
    uint32_t io3_cfg  = QSPI_Map_PinConfig(index, io3_pin);
    uint32_t sclk_cfg = QSPI_Map_PinConfig(index, sclk_pin);
    uint32_t cs_cfg   = QSPI_Map_PinConfig(index, cs_pin);

    uint32_t io0_port = GPIO_Map_Soc_Port_Base(io0_pin);
    uint32_t io1_port = GPIO_Map_Soc_Port_Base(io1_pin);
    uint32_t io2_port = GPIO_Map_Soc_Port_Base(io2_pin);
    uint32_t io3_port = GPIO_Map_Soc_Port_Base(io3_pin);
    uint32_t sclk_port = GPIO_Map_Soc_Port_Base(sclk_pin);
    uint32_t cs_port = GPIO_Map_Soc_Port_Base(cs_pin);

    uint32_t io0_pin_mask = GPIO_Map_Soc_Pin_Mask(io0_pin);
    uint32_t io1_pin_mask = GPIO_Map_Soc_Pin_Mask(io1_pin);
    uint32_t io2_pin_mask = GPIO_Map_Soc_Pin_Mask(io2_pin);
    uint32_t io3_pin_mask = GPIO_Map_Soc_Pin_Mask(io3_pin);
    uint32_t sclk_pin_mask = GPIO_Map_Soc_Pin_Mask(sclk_pin);
    uint32_t cs_pin_mask = GPIO_Map_Soc_Pin_Mask(cs_pin);

    if (io0_cfg == 0 || io1_cfg == 0 || io2_cfg == 0 || io3_cfg == 0 || sclk_cfg == 0 || \
        io0_port == 0 || io1_port == 0 || io2_port == 0 || io3_port == 0 || sclk_port == 0 || \
        io0_pin_mask == 0 || io1_pin_mask == 0 || io2_pin_mask == 0 || io3_pin_mask == 0 || sclk_pin_mask == 0)
    {
        return hwQSPI_InvalidParameter;
    }

    if(cs)
    {
        if (cs_cfg == 0 || cs_port == 0 || cs_pin_mask == 0)
        {
            return hwSPI_InvalidParameter;
        }
    }

    GPIO_Enable_Port_Clock(io0_port);
    GPIO_Enable_Port_Clock(io1_port);
    GPIO_Enable_Port_Clock(io2_port);
    GPIO_Enable_Port_Clock(io3_port);
    GPIO_Enable_Port_Clock(sclk_port);
    if(cs)
    {
        GPIO_Enable_Port_Clock(cs_port);
    }

    MAP_SysCtlPeripheralEnable(ssi_periph);
    while (!MAP_SysCtlPeripheralReady(ssi_periph));

    MAP_GPIOPinConfigure(io0_cfg);
    MAP_GPIOPinConfigure(io1_cfg);
    MAP_GPIOPinConfigure(io2_cfg);
    MAP_GPIOPinConfigure(io3_cfg);
    MAP_GPIOPinConfigure(sclk_cfg);
    if (cs)
    {
        MAP_GPIOPinConfigure(cs_cfg);
    }

    MAP_GPIOPinTypeSSI(io0_port, io0_pin_mask);
    MAP_GPIOPinTypeSSI(io1_port, io1_pin_mask);
    MAP_GPIOPinTypeSSI(io2_port, io2_pin_mask);
    MAP_GPIOPinTypeSSI(io3_port, io3_pin_mask);
    MAP_GPIOPinTypeSSI(sclk_port, sclk_pin_mask);
    if (cs)
    {
        MAP_GPIOPinTypeSSI(cs_port, cs_pin_mask);
    }

    MAP_SSIDisable(ssi_base);

    MAP_SSIConfigSetExpClk(
        ssi_base,
        MAP_SysCtlClockGet(),
        QSPI_Map_Mode(opMode),
        SSI_MODE_MASTER,
        clock_rate_hz,
        8
    );

    MAP_SSIAdvModeSet(ssi_base, SSI_ADV_MODE_READ_WRITE);
    MAP_SSIAdvFrameHoldDisable(ssi_base);

    MAP_SSIEnable(ssi_base);
    QSPI_Flush_RX(ssi_base);

    if (NeonRTOS_SyncObjCreate(&Qspi_Master_Send_SyncHandle[index]) != NeonRTOS_OK)
    {
        MAP_SSIDisable(ssi_base);
        return hwQSPI_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&Qspi_Master_Recv_SyncHandle[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Qspi_Master_Send_SyncHandle[index]);
        MAP_SSIDisable(ssi_base);
        return hwQSPI_MemoryError;
    }

    if (NeonRTOS_LockObjCreate(&Qspi_Master_Access_Mutex[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Qspi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Qspi_Master_Recv_SyncHandle[index]);
        MAP_SSIDisable(ssi_base);
        return hwQSPI_MemoryError;
    }

    NeonRTOS_LockObjUnlock(&Qspi_Master_Access_Mutex[index]);

    Qspi_Master_Clock_Hz[index] = clock_rate_hz;
    Qspi_Master_Mode[index] = opMode;

    QSPI_NVIC_Init(index);

    gpio_pin_init_status[io0_pin] = true;
    gpio_pin_init_status[io1_pin] = true;
    gpio_pin_init_status[io2_pin] = true;
    gpio_pin_init_status[io3_pin] = true;
    gpio_pin_init_status[sclk_pin] = true;

    if (cs)
    {
        gpio_pin_init_status[cs_pin] = true;
        Qspi_Master_Use_CS[index] = true;
    }

    Qspi_Master_Init_Status[index] = true;

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_DeInit(hwQSPI_Index index)
{
    if (index >= hwQSPI_Index_MAX)
    {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index])
    {
        return hwQSPI_OK;
    }

    uint32_t base = QSPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return hwQSPI_InvalidParameter;
    }

    QSPI_NVIC_DeInit(index);

    MAP_SSIDisable(base);

    NeonRTOS_LockObjDelete(&Qspi_Master_Access_Mutex[index]);
    NeonRTOS_SyncObjDelete(&Qspi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&Qspi_Master_Recv_SyncHandle[index]);

    memset(&qspi_xfer[index], 0, sizeof(qspi_xfer[index]));

    hwGPIO_Pin io0_pin  = QSPI_Pin_Def_Table[index].io0_pin;
    hwGPIO_Pin io1_pin  = QSPI_Pin_Def_Table[index].io1_pin;
    hwGPIO_Pin io2_pin  = QSPI_Pin_Def_Table[index].io2_pin;
    hwGPIO_Pin io3_pin  = QSPI_Pin_Def_Table[index].io3_pin;
    hwGPIO_Pin sclk_pin = QSPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin   = QSPI_Pin_Def_Table[index].cs_pin;

    uint32_t io0_port  = GPIO_Map_Soc_Port_Base(io0_pin);
    uint32_t io1_port  = GPIO_Map_Soc_Port_Base(io1_pin);
    uint32_t io2_port  = GPIO_Map_Soc_Port_Base(io2_pin);
    uint32_t io3_port  = GPIO_Map_Soc_Port_Base(io3_pin);
    uint32_t sclk_port = GPIO_Map_Soc_Port_Base(sclk_pin);
    uint32_t cs_port   = GPIO_Map_Soc_Port_Base(cs_pin);

    uint32_t io0_pin_mask  = GPIO_Map_Soc_Pin_Mask(io0_pin);
    uint32_t io1_pin_mask  = GPIO_Map_Soc_Pin_Mask(io1_pin);
    uint32_t io2_pin_mask  = GPIO_Map_Soc_Pin_Mask(io2_pin);
    uint32_t io3_pin_mask  = GPIO_Map_Soc_Pin_Mask(io3_pin);
    uint32_t sclk_pin_mask = GPIO_Map_Soc_Pin_Mask(sclk_pin);
    uint32_t cs_pin_mask   = GPIO_Map_Soc_Pin_Mask(cs_pin);

    if (io0_port == 0 || io1_port == 0 || io2_port == 0 || io3_port == 0 || sclk_port == 0 || \
        io0_pin_mask == 0 || io1_pin_mask == 0 || io2_pin_mask == 0 || io3_pin_mask == 0 || sclk_pin_mask == 0)
    {
        return hwQSPI_InvalidParameter;
    }

    if(Qspi_Master_Use_CS[index])
    {
        if(cs_port == 0  || cs_pin_mask == 0)
        {
            return hwSPI_InvalidParameter;
        }
    }

    MAP_GPIOPinTypeGPIOInput(io0_port, io0_pin_mask);
    MAP_GPIOPinTypeGPIOInput(io1_port, io1_pin_mask);
    MAP_GPIOPinTypeGPIOInput(io2_port, io2_pin_mask);
    MAP_GPIOPinTypeGPIOInput(io3_port, io3_pin_mask);
    MAP_GPIOPinTypeGPIOInput(sclk_port, sclk_pin_mask);
    if (Qspi_Master_Use_CS[index])
    {
        MAP_GPIOPinTypeGPIOInput(cs_port, cs_pin_mask);
    }

    gpio_pin_init_status[io0_pin] = false;
    gpio_pin_init_status[io1_pin] = false;
    gpio_pin_init_status[io2_pin] = false;
    gpio_pin_init_status[io3_pin] = false;
    gpio_pin_init_status[sclk_pin] = false;
    if (Qspi_Master_Use_CS[index])
    {
        gpio_pin_init_status[cs_pin] = false;
        Qspi_Master_Use_CS[index] = false;
    }

    Qspi_Master_Clock_Hz[index] = 0;
    Qspi_Master_Init_Status[index] = false;

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Change_Frequency(hwQSPI_Index index, uint32_t clock_rate_hz)
{
    if (index >= hwQSPI_Index_MAX || clock_rate_hz == 0)
    {
        return hwQSPI_InvalidParameter;
    }
    
    if (!Qspi_Master_Init_Status[index]) return hwQSPI_NotInit;

    uint32_t base = QSPI_Map_Soc_Base(index);
    if (base == 0) return hwQSPI_InvalidParameter;

    QSPI_MASTER_MUTEX_LOCK(index, QSPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIDisable(base);

    MAP_SSIConfigSetExpClk(
        base,
        MAP_SysCtlClockGet(),
        QSPI_Map_Mode(Qspi_Master_Mode[index]),
        SSI_MODE_MASTER,
        clock_rate_hz,
        8
    );

    MAP_SSIAdvModeSet(base, SSI_ADV_MODE_READ_WRITE);
    MAP_SSIAdvFrameHoldDisable(base);
    MAP_SSIEnable(base);
    QSPI_Flush_RX(base);

    Qspi_Master_Clock_Hz[index] = clock_rate_hz;

    QSPI_MASTER_MUTEX_UNLOCK(index);
    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Change_Mode(hwQSPI_Index index, hwQSPI_OpMode opMode)
{
    if (index >= hwQSPI_Index_MAX || opMode >= hwQSPI_OpMode_MAX)
    {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index]) return hwQSPI_NotInit;

    uint32_t base = QSPI_Map_Soc_Base(index);
    if (base == 0) return hwQSPI_InvalidParameter;

    QSPI_MASTER_MUTEX_LOCK(index, QSPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIDisable(base);

    MAP_SSIConfigSetExpClk(
        base,
        MAP_SysCtlClockGet(),
        QSPI_Map_Mode(opMode),
        SSI_MODE_MASTER,
        Qspi_Master_Clock_Hz[index],
        8
    );

    MAP_SSIAdvModeSet(base, SSI_ADV_MODE_READ_WRITE);
    MAP_SSIAdvFrameHoldDisable(base);
    MAP_SSIEnable(base);
    QSPI_Flush_RX(base);

    Qspi_Master_Mode[index] = opMode;

    QSPI_MASTER_MUTEX_UNLOCK(index);
    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_WriteByte(hwQSPI_Index index, uint8_t dat)
{
    if (index >= hwQSPI_Index_MAX)
    {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index])
    {
        return hwQSPI_NotInit;
    }

    uint32_t base = QSPI_Map_Soc_Base(index);
    if (base == 0) return hwQSPI_InvalidParameter;

    QSPI_MASTER_MUTEX_LOCK(index, QSPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    QSPI_Transfer *t = &qspi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = QSPI_STATE_TX;
    t->tx_buf = &dat;
    t->rx_buf = NULL;
    t->len    = 1;
    t->tx_pos = 0;
    t->rx_pos = 0;

    QSPI_Flush_RX(base);

    MAP_SSIAdvModeSet(base, SSI_ADV_MODE_QUAD_WRITE);
    MAP_SSIAdvFrameHoldEnable(base);

    NeonRTOS_SyncObjClear(&Qspi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Qspi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    QSPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Qspi_Master_Send_SyncHandle[index],
                             QSPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        MAP_SSIAdvFrameHoldDisable(base);
        MAP_SSIAdvModeSet(base, SSI_ADV_MODE_READ_WRITE);
        t->state = QSPI_STATE_ERROR;
        QSPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_SlaveTimeout;
    }

    if (t->state == QSPI_STATE_ERROR)
    {
        QSPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    QSPI_MASTER_MUTEX_UNLOCK(index);
    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_ReadByte(hwQSPI_Index index, uint8_t *dat)
{
    if (index >= hwQSPI_Index_MAX || dat == NULL)
    {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index])
    {
        return hwQSPI_NotInit;
    }

    uint32_t base = QSPI_Map_Soc_Base(index);
    if (base == 0) return hwQSPI_InvalidParameter;

    QSPI_MASTER_MUTEX_LOCK(index, QSPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    QSPI_Transfer *t = &qspi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = QSPI_STATE_RX;
    t->tx_buf = NULL;
    t->rx_buf = dat;
    t->len    = 1;
    t->tx_pos = 0;
    t->rx_pos = 0;

    QSPI_Flush_RX(base);

    MAP_SSIAdvModeSet(base, SSI_ADV_MODE_QUAD_READ);
    MAP_SSIAdvFrameHoldEnable(base);

    NeonRTOS_SyncObjClear(&Qspi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Qspi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    QSPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Qspi_Master_Recv_SyncHandle[index],
                             QSPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        MAP_SSIAdvFrameHoldDisable(base);
        MAP_SSIAdvModeSet(base, SSI_ADV_MODE_READ_WRITE);
        t->state = QSPI_STATE_ERROR;
        QSPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_SlaveTimeout;
    }

    if (t->state == QSPI_STATE_ERROR)
    {
        QSPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    QSPI_MASTER_MUTEX_UNLOCK(index);
    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_DummyByte(hwQSPI_Index index)
{
    return QSPI_Master_WriteByte(index, 0x00);
}

hwQSPI_OpResult QSPI_Master_DummyBytes(hwQSPI_Index index, uint32_t len)
{
    if (len == 0) return hwQSPI_InvalidParameter;

    for (uint32_t i = 0; i < len; i++)
    {
        hwQSPI_OpResult ret = QSPI_Master_DummyByte(index);
        if (ret != hwQSPI_OK) return ret;
    }

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_Stream_Write(hwQSPI_Index index, const uint8_t *buf, uint16_t len)
{
    if (index >= hwQSPI_Index_MAX || buf == NULL || len == 0)
    {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index])
    {
        return hwQSPI_NotInit;
    }

    uint32_t base = QSPI_Map_Soc_Base(index);
    if (base == 0) return hwQSPI_InvalidParameter;

    QSPI_MASTER_MUTEX_LOCK(index, QSPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    QSPI_Transfer *t = &qspi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = QSPI_STATE_TX;
    t->tx_buf = buf;
    t->rx_buf = NULL;
    t->len    = len;
    t->tx_pos = 0;
    t->rx_pos = 0;

    QSPI_Flush_RX(base);

    MAP_SSIAdvModeSet(base, SSI_ADV_MODE_QUAD_WRITE);
    MAP_SSIAdvFrameHoldEnable(base);

    NeonRTOS_SyncObjClear(&Qspi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Qspi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    QSPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Qspi_Master_Send_SyncHandle[index],
                             QSPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        MAP_SSIAdvFrameHoldDisable(base);
        MAP_SSIAdvModeSet(base, SSI_ADV_MODE_READ_WRITE);
        t->state = QSPI_STATE_ERROR;
        QSPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_SlaveTimeout;
    }

    if (t->state == QSPI_STATE_ERROR)
    {
        QSPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    QSPI_MASTER_MUTEX_UNLOCK(index);
    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_Stream_Read(hwQSPI_Index index, uint8_t *buf, uint16_t len)
{
    if (index >= hwQSPI_Index_MAX || buf == NULL || len == 0)
    {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index])
    {
        return hwQSPI_NotInit;
    }

    uint32_t base = QSPI_Map_Soc_Base(index);
    if (base == 0) return hwQSPI_InvalidParameter;

    QSPI_MASTER_MUTEX_LOCK(index, QSPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    QSPI_Transfer *t = &qspi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = QSPI_STATE_RX;
    t->tx_buf = NULL;
    t->rx_buf = buf;
    t->len    = len;
    t->tx_pos = 0;
    t->rx_pos = 0;

    QSPI_Flush_RX(base);

    MAP_SSIAdvModeSet(base, SSI_ADV_MODE_QUAD_READ);
    MAP_SSIAdvFrameHoldEnable(base);

    NeonRTOS_SyncObjClear(&Qspi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Qspi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    QSPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Qspi_Master_Recv_SyncHandle[index],
                             QSPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        MAP_SSIAdvFrameHoldDisable(base);
        MAP_SSIAdvModeSet(base, SSI_ADV_MODE_READ_WRITE);
        t->state = QSPI_STATE_ERROR;
        QSPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_SlaveTimeout;
    }

    if (t->state == QSPI_STATE_ERROR)
    {
        QSPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    QSPI_MASTER_MUTEX_UNLOCK(index);
    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_Burst_Write(hwQSPI_Index index, uint8_t *buf, uint32_t size)
{
    if (buf == NULL || size == 0 || size > 0xFFFF) return hwQSPI_InvalidParameter;

    return QSPI_Master_Stream_Write(index, buf, (uint16_t)size);
}

hwQSPI_OpResult QSPI_Master_Burst_Read(hwQSPI_Index index, uint8_t *buf, uint32_t size)
{
    if (buf == NULL || size == 0 || size > 0xFFFF) return hwQSPI_InvalidParameter;

    return QSPI_Master_Stream_Read(index, buf, (uint16_t)size);
}

#endif // DEVICE_TITIVAC