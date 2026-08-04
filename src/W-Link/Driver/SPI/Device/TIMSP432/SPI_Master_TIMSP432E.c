#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "SPI/SPI_Master.h"

#include "DMA/DMA.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSP432E

#include "GPIO/Device/TIMSP432/GPIO_TIMSP432E.h"

#include "SPI/Pin/TIMSP432/SPI_Pin_TIMSP432E.h"

#include "SPI_Master_TIMSP432.h"

#define SPI_MASTER_MUTEX_ACCESS_TIMEOUT     500
#define SPI_MASTER_OP_TIMEOUT               3000

typedef enum {
    MSP432E_SPI_IDLE = 0,
    MSP432E_SPI_TX,
    MSP432E_SPI_RX,
    MSP432E_SPI_TXRX,
    MSP432E_SPI_DONE,
    MSP432E_SPI_ERROR
} MSP432E_SPI_State;

typedef struct {
    MSP432E_SPI_State state;
    const uint8_t *tx_buf;
    uint8_t *rx_buf;
    uint32_t len;
    uint32_t tx_pos;
    uint32_t rx_pos;
} MSP432E_SPI_Transfer;

NeonRTOS_LockObj_t Spi_Master_Access_Mutex[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Send_SyncHandle[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Recv_SyncHandle[hwSPI_Index_MAX];

#define SPI_MASTER_MUTEX_LOCK(a, b)  \
    if (NeonRTOS_LockObjLock(&Spi_Master_Access_Mutex[a], b) != NeonRTOS_OK) { return hwSPI_MutexTimeout; }

#define SPI_MASTER_MUTEX_UNLOCK(a)   \
    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[a])

bool Spi_Master_Init_Status[hwSPI_Index_MAX] = {false};

static bool Spi_Master_Use_CS[hwSPI_Index_MAX] = {false};

static MSP432E_SPI_Transfer spi_xfer[hwSPI_Index_MAX];

static uint32_t Spi_Master_Clock_Hz[hwSPI_Index_MAX] = {0};
static hwSPI_OpMode Spi_Master_Mode[hwSPI_Index_MAX];

static void SPI_IRQ_Process(hwSPI_Index index);

static uint32_t SPI_Map_Soc_Base(hwSPI_Index index)
{
    switch (index)
    {
        case hwSPI_Index_0: return SSI0_BASE;
        case hwSPI_Index_1: return SSI1_BASE;
        case hwSPI_Index_2: return SSI2_BASE;
        case hwSPI_Index_3: return SSI3_BASE;
        default: return 0;
    }
}

static uint32_t SPI_Map_Soc_Periph(hwSPI_Index index)
{
    switch (index)
    {
        case hwSPI_Index_0: return SYSCTL_PERIPH_SSI0;
        case hwSPI_Index_1: return SYSCTL_PERIPH_SSI1;
        case hwSPI_Index_2: return SYSCTL_PERIPH_SSI2;
        case hwSPI_Index_3: return SYSCTL_PERIPH_SSI3;
        default: return 0;
    }
}

static uint32_t SPI_Map_IRQ(hwSPI_Index index)
{
    switch (index)
    {
        case hwSPI_Index_0: return INT_SSI0;
        case hwSPI_Index_1: return INT_SSI1;
        case hwSPI_Index_2: return INT_SSI2;
        case hwSPI_Index_3: return INT_SSI3;
        default: return 0;
    }
}

static uint32_t SPI_Map_PinConfig(hwSPI_Index index, hwGPIO_Pin pin)
{
    switch (index) {
        case hwSPI_Index_0:
            if (pin == hwGPIO_Pin_A2) return GPIO_PA2_SSI0CLK;
            if (pin == hwGPIO_Pin_A4) return GPIO_PA4_SSI0XDAT0;
            if (pin == hwGPIO_Pin_A5) return GPIO_PA5_SSI0XDAT1;
            if (pin == hwGPIO_Pin_A3) return GPIO_PA3_SSI0FSS;
            break;
        case hwSPI_Index_1:
            if (pin == hwGPIO_Pin_B5) return GPIO_PB5_SSI1CLK;
            if (pin == hwGPIO_Pin_E4) return GPIO_PE4_SSI1XDAT0;
            if (pin == hwGPIO_Pin_E5) return GPIO_PE5_SSI1XDAT1;
            if (pin == hwGPIO_Pin_B4) return GPIO_PB4_SSI1FSS;
            break;
        case hwSPI_Index_2:
            if (pin == hwGPIO_Pin_D3) return GPIO_PD3_SSI2CLK;
            if (pin == hwGPIO_Pin_D1) return GPIO_PD1_SSI2XDAT0;
            if (pin == hwGPIO_Pin_D0) return GPIO_PD0_SSI2XDAT1;
            if (pin == hwGPIO_Pin_D2) return GPIO_PD2_SSI2FSS;
            break;
        case hwSPI_Index_3:
            if (pin == hwGPIO_Pin_Q0) return GPIO_PQ0_SSI3CLK;
            if (pin == hwGPIO_Pin_Q2) return GPIO_PQ2_SSI3XDAT0;
            if (pin == hwGPIO_Pin_Q3) return GPIO_PQ3_SSI3XDAT1;
            if (pin == hwGPIO_Pin_Q1) return GPIO_PQ1_SSI3FSS;
            break;

        default:
            break;
    }

    return 0;
}

static void SPI0_IRQ_Handler(void)
{
    SPI_IRQ_Process(hwSPI_Index_0);
}

static void SPI1_IRQ_Handler(void)
{
    SPI_IRQ_Process(hwSPI_Index_1);
}

static void SPI2_IRQ_Handler(void)
{
    SPI_IRQ_Process(hwSPI_Index_2);
}

static void SPI3_IRQ_Handler(void)
{
    SPI_IRQ_Process(hwSPI_Index_3);
}

static void NVIC_Init(hwSPI_Index index)
{
    uint32_t base = SPI_Map_Soc_Base(index);
    uint32_t irq = SPI_Map_IRQ(index);

    if (base == 0)
    {
        return;
    }

    switch (index)
    {
        case hwSPI_Index_0:
            SSIIntRegister(base, SPI0_IRQ_Handler);
            break;

        case hwSPI_Index_1:
            SSIIntRegister(base, SPI1_IRQ_Handler);
            break;

        case hwSPI_Index_2:
            SSIIntRegister(base, SPI2_IRQ_Handler);
            break;

        case hwSPI_Index_3:
            SSIIntRegister(base, SPI3_IRQ_Handler);
            break;

        default:
            return;
    }

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    if (irq != 0)
    {
        MAP_IntEnable(irq);
    }
}

static void NVIC_DeInit(hwSPI_Index index)
{
    uint32_t base = SPI_Map_Soc_Base(index);
    uint32_t irq = SPI_Map_IRQ(index);

    if (base != 0)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        SSIIntUnregister(base);
    }

    if (irq != 0)
    {
        MAP_IntDisable(irq);
    }
}

static void SPI_IRQ_Process(hwSPI_Index index)
{
    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return;
    }

    uint32_t status = MAP_SSIIntStatus(base, true);

    if (status != 0)
    {
        MAP_SSIIntClear(base, status);
    }

    MSP432E_SPI_Transfer *t = &spi_xfer[index];

    if (t->state == MSP432E_SPI_IDLE ||
        t->state == MSP432E_SPI_DONE ||
        t->state == MSP432E_SPI_ERROR)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        return;
    }

    if (status & SSI_RXOR)
    {
        t->state = MSP432E_SPI_ERROR;

        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

        NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[index]);
        NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[index]);
        return;
    }

    uint32_t rx;

    while (MAP_SSIDataGetNonBlocking(base, &rx))
    {
        if (t->rx_pos < t->len)
        {
            if ((t->state == MSP432E_SPI_RX || t->state == MSP432E_SPI_TXRX) &&
                t->rx_buf != NULL)
            {
                t->rx_buf[t->rx_pos] = (uint8_t)(rx & 0xFF);
            }

            t->rx_pos++;
        }
    }

    while (t->tx_pos < t->len)
    {
        uint8_t out = 0xFF;

        if ((t->state == MSP432E_SPI_TX || t->state == MSP432E_SPI_TXRX) &&
            t->tx_buf != NULL)
        {
            out = t->tx_buf[t->tx_pos];
        }

        if (!MAP_SSIDataPutNonBlocking(base, out))
        {
            break;
        }

        t->tx_pos++;
    }

    if (t->rx_pos >= t->len && t->tx_pos >= t->len)
    {
        t->state = MSP432E_SPI_DONE;

        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

        NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[index]);
        NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[index]);
    }
}

hwSPI_OpResult SPI_Master_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode, bool cs)
{
    if (index >= hwSPI_Index_MAX || opMode >= hwSPI_OpMode_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == true)
    {
        return hwSPI_OK;
    }

    uint32_t ssi_base = SPI_Map_Soc_Base(index);
    uint32_t ssi_periph = SPI_Map_Soc_Periph(index);

    if (ssi_base == 0 || ssi_periph == 0)
    {
        return hwSPI_InvalidParameter;
    }

    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index].miso_pin;
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index].mosi_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin   = SPI_Pin_Def_Table[index].cs_pin;

    uint32_t miso_pin_cfg = SPI_Map_PinConfig(index, miso_pin);
    uint32_t mosi_pin_cfg = SPI_Map_PinConfig(index, mosi_pin);
    uint32_t sclk_pin_cfg = SPI_Map_PinConfig(index, sclk_pin);
    uint32_t cs_pin_cfg = SPI_Map_PinConfig(index, cs_pin);

    uint32_t miso_port = GPIO_Map_Soc_Port_Base(miso_pin);
    uint32_t mosi_port = GPIO_Map_Soc_Port_Base(mosi_pin);
    uint32_t sclk_port = GPIO_Map_Soc_Port_Base(sclk_pin);
    uint32_t cs_port = GPIO_Map_Soc_Port_Base(cs_pin);

    uint32_t miso_pin_mask = GPIO_Map_Soc_Pin_Mask(miso_pin);
    uint32_t mosi_pin_mask = GPIO_Map_Soc_Pin_Mask(mosi_pin);
    uint32_t sclk_pin_mask = GPIO_Map_Soc_Pin_Mask(sclk_pin);
    uint32_t cs_pin_mask = GPIO_Map_Soc_Pin_Mask(cs_pin);

    if (miso_pin_cfg == 0 || mosi_pin_cfg == 0 || sclk_pin_cfg == 0 || \
        miso_port == 0 || mosi_port == 0 || sclk_port == 0 || \
        miso_pin_mask == 0 || mosi_pin_mask == 0 || sclk_pin_mask == 0)
    {
        return hwSPI_InvalidParameter;
    }

    if(cs)
    {
        if (cs_pin_cfg == 0 || cs_port == 0 || cs_pin_mask == 0)
        {
            return hwSPI_InvalidParameter;
        }
    }

    GPIO_Enable_Port_Clock(miso_port);
    GPIO_Enable_Port_Clock(mosi_port);
    GPIO_Enable_Port_Clock(sclk_port);
    if(cs)
    {
        GPIO_Enable_Port_Clock(cs_port);
    }

    MAP_SysCtlPeripheralEnable(ssi_periph);
    while (!MAP_SysCtlPeripheralReady(ssi_periph));

    MAP_GPIOPinConfigure(miso_pin_cfg);
    MAP_GPIOPinConfigure(mosi_pin_cfg);
    MAP_GPIOPinConfigure(sclk_pin_cfg);
    if(cs)
    {
        MAP_GPIOPinConfigure(cs_pin_cfg);
    }

    MAP_GPIOPinTypeSSI(miso_port, miso_pin_mask);
    MAP_GPIOPinTypeSSI(mosi_port, mosi_pin_mask);
    MAP_GPIOPinTypeSSI(sclk_port, sclk_pin_mask);
    if(cs)
    {
        MAP_GPIOPinTypeSSI(cs_port, cs_pin_mask);
    }

    MAP_SSIDisable(ssi_base);

    uint32_t dl_mode;

    switch (opMode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            dl_mode = SSI_FRF_MOTO_MODE_0;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            dl_mode = SSI_FRF_MOTO_MODE_1;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            dl_mode = SSI_FRF_MOTO_MODE_2;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            dl_mode = SSI_FRF_MOTO_MODE_3;
            break;
    }
    
    MAP_SSIConfigSetExpClk(
        ssi_base,
        g_sys_clock_hz,
        dl_mode,
        SSI_MODE_MASTER,
        clock_rate_hz,
        8
    );

    MAP_SSIEnable(ssi_base);

    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(ssi_base, &dummy));

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Send_SyncHandle[index]) != NeonRTOS_OK)
    {
        MAP_SSIDisable(ssi_base);
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Recv_SyncHandle[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        MAP_SSIDisable(ssi_base);
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_LockObjCreate(&Spi_Master_Access_Mutex[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
        MAP_SSIDisable(ssi_base);
        return hwSPI_MemoryError;
    }

    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[index]);

    Spi_Master_Clock_Hz[index] = clock_rate_hz;
    Spi_Master_Mode[index] = opMode;

    SPI_NVIC_Init(index);

    gpio_pin_init_status[miso_pin] = true;
    gpio_pin_init_status[mosi_pin] = true;
    gpio_pin_init_status[sclk_pin] = true;

    if (cs)
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

    uint32_t ssi_base = SPI_Map_Soc_Base(index);
    if (ssi_base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_NVIC_DeInit(index);

    MAP_SSIDisable(ssi_base);

    NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
    NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);

    memset(&spi_xfer[index], 0, sizeof(spi_xfer[index]));

    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index].miso_pin;
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index].mosi_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin   = SPI_Pin_Def_Table[index].cs_pin;

    uint32_t miso_port = GPIO_Map_Soc_Port_Base(miso_pin);
    uint32_t mosi_port = GPIO_Map_Soc_Port_Base(mosi_pin);
    uint32_t sclk_port = GPIO_Map_Soc_Port_Base(sclk_pin);
    uint32_t cs_port   = GPIO_Map_Soc_Port_Base(cs_pin);
    
    uint32_t miso_pin_mask = GPIO_Map_Soc_Pin_Mask(miso_pin);
    uint32_t mosi_pin_mask = GPIO_Map_Soc_Pin_Mask(mosi_pin);
    uint32_t sclk_pin_mask = GPIO_Map_Soc_Pin_Mask(sclk_pin);
    uint32_t cs_pin_mask   = GPIO_Map_Soc_Pin_Mask(cs_pin);

    if (miso_port == 0 || mosi_port == 0 || sclk_port == 0 || \
        miso_pin_mask == 0 || mosi_pin_mask == 0 || sclk_pin_mask == 0)
    {
        return hwSPI_InvalidParameter;
    }

    if(Spi_Master_Use_CS[index])
    {
        if(cs_port == 0  || cs_pin_mask == 0)
        {
            return hwSPI_InvalidParameter;
        }
    }

    MAP_GPIOPinTypeGPIOInput(miso_port, miso_pin_mask);
    MAP_GPIOPinTypeGPIOInput(mosi_port, mosi_pin_mask);
    MAP_GPIOPinTypeGPIOInput(sclk_port, sclk_pin_mask);
    if(Spi_Master_Use_CS[index])
    {
        MAP_GPIOPinTypeGPIOInput(cs_port, cs_pin_mask);
    }

    gpio_pin_init_status[miso_pin] = false;
    gpio_pin_init_status[mosi_pin] = false;
    gpio_pin_init_status[sclk_pin] = false;

    if (Spi_Master_Use_CS[index] == true)
    {
        gpio_pin_init_status[cs_pin] = false;
        Spi_Master_Use_CS[index] = false;
    }

    Spi_Master_Clock_Hz[index] = 0;
    Spi_Master_Init_Status[index] = false;

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

    uint32_t ssi_base = SPI_Map_Soc_Base(index);

    if (ssi_base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    uint32_t dl_mode;

    switch (Spi_Master_Mode[index])
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            dl_mode = SSI_FRF_MOTO_MODE_0;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            dl_mode = SSI_FRF_MOTO_MODE_1;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            dl_mode = SSI_FRF_MOTO_MODE_2;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            dl_mode = SSI_FRF_MOTO_MODE_3;
            break;
    }
    
    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MAP_SSIIntDisable(ssi_base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    MAP_SSIDisable(ssi_base);

    MAP_SSIConfigSetExpClk(
        ssi_base,
        g_sys_clock_hz,
        dl_mode,
        SSI_MODE_MASTER,
        clock_rate_hz,
        8
    );

    MAP_SSIEnable(ssi_base);
    
    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(ssi_base, &dummy));

    Spi_Master_Clock_Hz[index] = clock_rate_hz;

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Mode(hwSPI_Index index, hwSPI_OpMode opMode)
{
    if (index >= hwSPI_Index_MAX || opMode >= hwSPI_OpMode_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    uint32_t ssi_base = SPI_Map_Soc_Base(index);

    if (ssi_base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    uint32_t dl_mode;

    switch (opMode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            dl_mode = SSI_FRF_MOTO_MODE_0;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            dl_mode = SSI_FRF_MOTO_MODE_1;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            dl_mode = SSI_FRF_MOTO_MODE_2;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            dl_mode = SSI_FRF_MOTO_MODE_3;
            break;
    }
    
    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MAP_SSIIntDisable(ssi_base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    MAP_SSIDisable(ssi_base);

    MAP_SSIConfigSetExpClk(
        ssi_base,
        g_sys_clock_hz,
        dl_mode,
        SSI_MODE_MASTER,
        Spi_Master_Clock_Hz[index],
        8
    );

    MAP_SSIEnable(ssi_base);
    
    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(ssi_base, &dummy));

    Spi_Master_Mode[index] = opMode;

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

    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MSP432E_SPI_Transfer *t = &spi_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = MSP432E_SPI_TX;
    t->tx_buf = &dat;
    t->rx_buf = NULL;
    t->len    = 1;
    t->tx_pos = 0;
    t->rx_pos = 0;

    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(base, &dummy));

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        t->state = MSP432E_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == MSP432E_SPI_ERROR)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_ReadByte(hwSPI_Index index, uint8_t *dat)
{
    if (index >= hwSPI_Index_MAX || dat == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MSP432E_SPI_Transfer *t = &spi_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = MSP432E_SPI_RX;
    t->tx_buf = NULL;
    t->rx_buf = dat;
    t->len    = 1;
    t->tx_pos = 0;
    t->rx_pos = 0;
    
    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(base, &dummy));

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    /*
     * Prime FIFO:
     * SPI read still needs dummy TX clock.
     * ISR will transmit 0xFF because state == MSP432E_SPI_RX.
     */
    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        t->state = MSP432E_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == MSP432E_SPI_ERROR)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_TransferByte(hwSPI_Index index, uint8_t wr_dat, uint8_t *rd_dat)
{
    if (index >= hwSPI_Index_MAX || rd_dat == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MSP432E_SPI_Transfer *t = &spi_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = MSP432E_SPI_TXRX;
    t->tx_buf = &wr_dat;
    t->rx_buf = rd_dat;
    t->len    = 1;
    t->tx_pos = 0;
    t->rx_pos = 0;
    
    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(base, &dummy));

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        t->state = MSP432E_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == MSP432E_SPI_ERROR)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_DummyByte(hwSPI_Index index)
{
    uint8_t dummy = 0xFF;

    return SPI_Master_WriteByte(index, dummy);
}

hwSPI_OpResult SPI_Master_DummyBytes(hwSPI_Index index, uint32_t len)
{
    if (index >= hwSPI_Index_MAX || len == 0)
    {
        return hwSPI_InvalidParameter;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        hwSPI_OpResult ret = SPI_Master_DummyByte(index);

        if (ret != hwSPI_OK)
        {
            return ret;
        }
    }

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Stream_Write(hwSPI_Index index, const uint8_t *buf, uint16_t len)
{
    if (index >= hwSPI_Index_MAX || buf == NULL || len == 0)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MSP432E_SPI_Transfer *t = &spi_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = MSP432E_SPI_TX;
    t->tx_buf = buf;
    t->rx_buf = NULL;
    t->len    = len;
    t->tx_pos = 0;
    t->rx_pos = 0;
    
    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(base, &dummy));

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        t->state = MSP432E_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == MSP432E_SPI_ERROR)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Stream_Read(hwSPI_Index index, uint8_t *buf, uint16_t len)
{
    if (index >= hwSPI_Index_MAX || buf == NULL || len == 0)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MSP432E_SPI_Transfer *t = &spi_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = MSP432E_SPI_RX;
    t->tx_buf = NULL;
    t->rx_buf = buf;
    t->len    = len;
    t->tx_pos = 0;
    t->rx_pos = 0;
    
    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(base, &dummy));

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        t->state = MSP432E_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == MSP432E_SPI_ERROR)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Stream_Transfer(hwSPI_Index index, const uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len)
{
    if (index >= hwSPI_Index_MAX || tx_buf == NULL || rx_buf == NULL || len == 0)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MSP432E_SPI_Transfer *t = &spi_xfer[index];

    memset(t, 0, sizeof(*t));

    t->state  = MSP432E_SPI_TXRX;
    t->tx_buf = tx_buf;
    t->rx_buf = rx_buf;
    t->len    = len;
    t->tx_pos = 0;
    t->rx_pos = 0;
    
    uint32_t dummy;

    while (MAP_SSIDataGetNonBlocking(base, &dummy));

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SSIIntClear(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
    MAP_SSIIntEnable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SSIIntDisable(base, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
        t->state = MSP432E_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == MSP432E_SPI_ERROR)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Burst_Write(hwSPI_Index index, uint8_t* buf, uint32_t size)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }

        if(size==0)
        {
            if(buf==NULL)
            {
                  return hwSPI_OK;
            }
        }
        else{
            if(buf==NULL)
            {
                  return hwSPI_InvalidParameter;
            }
        }
        
        SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

        hwDMA_OpResult dma_op_status = DMA_SPI_Write(index, buf, size);
        if(dma_op_status < hwDMA_OK)
        {
            SPI_MASTER_MUTEX_UNLOCK(index);
            return SPI_Map_DMA_HW_Error_Code(dma_op_status);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Burst_Read(hwSPI_Index index, uint8_t* buf, uint32_t size)
{
        if(index>=hwSPI_Index_MAX)
        {
              return hwSPI_InvalidParameter;
        }
        
        if(Spi_Master_Init_Status[index]==false)
        {
                return hwSPI_NotInit;
        }

        if(size==0)
        {
            if(buf==NULL)
            {
                  return hwSPI_OK;
            }
        }
        else{
            if(buf==NULL)
            {
                  return hwSPI_InvalidParameter;
            }
        }
        
        SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

        hwDMA_OpResult dma_op_status = DMA_SPI_Read(index, buf, size);
        if(dma_op_status < hwDMA_OK)
        {
            SPI_MASTER_MUTEX_UNLOCK(index);
            return SPI_Map_DMA_HW_Error_Code(dma_op_status);
        }
        
        SPI_MASTER_MUTEX_UNLOCK(index);

        return hwSPI_OK;
}

#endif // DEVICE_TIMSP432E