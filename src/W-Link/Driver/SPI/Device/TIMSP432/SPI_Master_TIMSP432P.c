#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "SPI/SPI_Master.h"

#include "DMA/DMA.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSP432P

#include "GPIO/Device/TIMSP432/GPIO_TIMSP432P.h"

#include "SPI/Pin/TIMSP432/SPI_Pin_TIMSP432P.h"

#include "SPI_Master_TIMSP432.h"

#define SPI_MASTER_MUTEX_ACCESS_TIMEOUT     500
#define SPI_MASTER_OP_TIMEOUT               3000

typedef enum {
    TIMSP432P_SPI_IDLE = 0,
    TIMSP432P_SPI_TX,
    TIMSP432P_SPI_RX,
    TIMSP432P_SPI_TXRX,
    TIMSP432P_SPI_DONE,
    TIMSP432P_SPI_ERROR
} TIMSP432P_SPI_State;

typedef struct {
    TIMSP432P_SPI_State state;
    const uint8_t *tx_buf;
    uint8_t *rx_buf;
    uint32_t len;
    uint32_t tx_pos;
    uint32_t rx_pos;
} TIMSP432P_SPI_Transfer;

NeonRTOS_LockObj_t Spi_Master_Access_Mutex[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Send_SyncHandle[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Recv_SyncHandle[hwSPI_Index_MAX];

#define SPI_MASTER_MUTEX_LOCK(a, b)  \
    if (NeonRTOS_LockObjLock(&Spi_Master_Access_Mutex[a], b) != NeonRTOS_OK) { return hwSPI_MutexTimeout; }

#define SPI_MASTER_MUTEX_UNLOCK(a)   \
    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[a])

bool Spi_Master_Init_Status[hwSPI_Index_MAX] = {false};

static bool Spi_Master_Use_CS[hwSPI_Index_MAX] = {false};
static TIMSP432P_SPI_Transfer spi_xfer[hwSPI_Index_MAX];

static uint32_t Spi_Master_Clock_Hz[hwSPI_Index_MAX] = {0};
static hwSPI_OpMode Spi_Master_Mode[hwSPI_Index_MAX];

static uint32_t SPI_Map_Soc_Base(hwSPI_Index index)
{
    switch (index)
    {
        case hwSPI_Index_0: return EUSCI_B0_BASE;
        case hwSPI_Index_1: return EUSCI_B1_BASE;
        case hwSPI_Index_2: return EUSCI_B2_BASE;
        case hwSPI_Index_3: return EUSCI_B3_BASE;
        default: return 0;
    }
}

static IRQn_Type SPI_Map_IRQ(hwSPI_Index index)
{
    switch (index)
    {
        case hwSPI_Index_0: return EUSCIB0_IRQn;
        case hwSPI_Index_1: return EUSCIB1_IRQn;
        case hwSPI_Index_2: return EUSCIB2_IRQn;
        case hwSPI_Index_3: return EUSCIB3_IRQn;
        default: return (IRQn_Type)0;
    }
}

static void SPI0_IRQ_Handler(void){SPI_IRQ_Process(hwSPI_Index_0);}
static void SPI1_IRQ_Handler(void){SPI_IRQ_Process(hwSPI_Index_1);}
static void SPI2_IRQ_Handler(void){SPI_IRQ_Process(hwSPI_Index_2);}
static void SPI3_IRQ_Handler(void){SPI_IRQ_Process(hwSPI_Index_3);}

static void SPI_NVIC_Init(hwSPI_Index index)
{
    IRQn_Type irq = SPI_Map_IRQ(index);

    switch (index)
    {
        case hwSPI_Index_0:
            MAP_Interrupt_registerInterrupt(INT_EUSCIB0, SPI0_IRQ_Handler);
            break;

        case hwSPI_Index_1:
            MAP_Interrupt_registerInterrupt(INT_EUSCIB1, SPI1_IRQ_Handler);
            break;

        case hwSPI_Index_2:
            MAP_Interrupt_registerInterrupt(INT_EUSCIB2, SPI2_IRQ_Handler);
            break;

        case hwSPI_Index_3:
            MAP_Interrupt_registerInterrupt(INT_EUSCIB3, SPI3_IRQ_Handler);
            break;

        default:
            return;
    }

    MAP_SPI_clearInterruptFlag(SPI_Map_Soc_Base(index),
                               EUSCI_B_SPI_RECEIVE_INTERRUPT |
                               EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_SPI_disableInterrupt(SPI_Map_Soc_Base(index),
                             EUSCI_B_SPI_RECEIVE_INTERRUPT |
                             EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_Interrupt_enableInterrupt(irq);
}

static void SPI_NVIC_DeInit(hwSPI_Index index)
{
    uint32_t base = SPI_Map_Soc_Base(index);
    IRQn_Type irq = SPI_Map_IRQ(index);

    if (base != 0)
    {
        MAP_SPI_disableInterrupt(base,
                                 EUSCI_B_SPI_RECEIVE_INTERRUPT |
                                 EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        MAP_SPI_clearInterruptFlag(base,
                                   EUSCI_B_SPI_RECEIVE_INTERRUPT |
                                   EUSCI_B_SPI_TRANSMIT_INTERRUPT);
    }

    switch (index)
    {
        case hwSPI_Index_0:
            MAP_Interrupt_unregisterInterrupt(INT_EUSCIB0);
            break;

        case hwSPI_Index_1:
            MAP_Interrupt_unregisterInterrupt(INT_EUSCIB1);
            break;

        case hwSPI_Index_2:
            MAP_Interrupt_unregisterInterrupt(INT_EUSCIB2);
            break;

        case hwSPI_Index_3:
            MAP_Interrupt_unregisterInterrupt(INT_EUSCIB3);
            break;

        default:
            break;
    }

    if (irq != 0)
    {
        MAP_Interrupt_disableInterrupt(irq);
    }
}

void SPI_IRQ_Process(hwSPI_Index index)
{
    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return;
    }

    TIMSP432P_SPI_Transfer *t = &spi_xfer[index];

    if (t->state == TIMSP432P_SPI_IDLE ||
        t->state == TIMSP432P_SPI_DONE ||
        t->state == TIMSP432P_SPI_ERROR)
    {
        MAP_SPI_disableInterrupt(base,
                                 EUSCI_B_SPI_RECEIVE_INTERRUPT |
                                 EUSCI_B_SPI_TRANSMIT_INTERRUPT);
        return;
    }

    while (MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        uint8_t rx = MAP_SPI_receiveData(base);
        MAP_SPI_clearInterruptFlag(base, EUSCI_B_SPI_RECEIVE_INTERRUPT);

        if (t->rx_pos < t->len)
        {
            if ((t->state == TIMSP432P_SPI_RX ||
                 t->state == TIMSP432P_SPI_TXRX) &&
                t->rx_buf != NULL)
            {
                t->rx_buf[t->rx_pos] = rx;
            }

            t->rx_pos++;
        }
    }

    while ((t->tx_pos < t->len) &&
           MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_TRANSMIT_INTERRUPT))
    {
        uint8_t out = 0xFF;

        if ((t->state == TIMSP432P_SPI_TX ||
             t->state == TIMSP432P_SPI_TXRX) &&
            t->tx_buf != NULL)
        {
            out = t->tx_buf[t->tx_pos];
        }

        MAP_SPI_transmitData(base, out);
        t->tx_pos++;
    }

    if (t->rx_pos >= t->len && t->tx_pos >= t->len)
    {
        while (MAP_SPI_isBusy(base));

        t->state = TIMSP432P_SPI_DONE;

        MAP_SPI_disableInterrupt(base,
                                 EUSCI_B_SPI_RECEIVE_INTERRUPT |
                                 EUSCI_B_SPI_TRANSMIT_INTERRUPT);

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

    if (Spi_Master_Init_Status[index])
    {
        return hwSPI_OK;
    }

    uint32_t spi_base = SPI_Map_Soc_Base(index);

    if (spi_base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index].miso_pin;
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index].mosi_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin   = SPI_Pin_Def_Table[index].cs_pin;

    uint32_t miso_port = GPIO_Map_Soc_Port_Base(miso_pin);
    uint32_t mosi_port = GPIO_Map_Soc_Port_Base(mosi_pin);
    uint32_t sclk_port = GPIO_Map_Soc_Port_Base(sclk_pin);
    uint32_t cs_port   = GPIO_Map_Soc_Port_Base(cs_pin);

    uint32_t miso_mask = GPIO_Map_Soc_Pin_Mask(miso_pin);
    uint32_t mosi_mask = GPIO_Map_Soc_Pin_Mask(mosi_pin);
    uint32_t sclk_mask = GPIO_Map_Soc_Pin_Mask(sclk_pin);
    uint32_t cs_mask   = GPIO_Map_Soc_Pin_Mask(cs_pin);

    if (miso_port == 0 || mosi_port == 0 || sclk_port == 0 ||
        miso_mask == 0 || mosi_mask == 0 || sclk_mask == 0)
    {
        return hwSPI_InvalidParameter;
    }

    if (cs)
    {
        if (cs_port == 0 || cs_mask == 0)
        {
            return hwSPI_InvalidParameter;
        }
    }

    GPIO_Enable_Port_Clock(miso_port);
    GPIO_Enable_Port_Clock(mosi_port);
    GPIO_Enable_Port_Clock(sclk_port);

    if (cs)
    {
        GPIO_Enable_Port_Clock(cs_port);
    }

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(miso_port, miso_mask, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(mosi_port, mosi_mask, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(sclk_port, sclk_mask, GPIO_PRIMARY_MODULE_FUNCTION);

    if (cs)
    {
        MAP_GPIO_setAsPeripheralModuleFunctionInputPin(cs_port, cs_mask, GPIO_PRIMARY_MODULE_FUNCTION);
    }

    uint_fast8_t phase;
    uint_fast8_t polarity;

    switch (opMode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
            phase    = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
            phase    = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
            phase    = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
            phase    = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
            break;
    }

    eUSCI_SPI_MasterConfig spi_config =
    {
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
        g_sys_clock_hz,
        clock_rate_hz,
        EUSCI_B_SPI_MSB_FIRST,
        phase,
        polarity,
        cs ? EUSCI_B_SPI_4PIN_UCxSTE_ACTIVE_LOW : EUSCI_B_SPI_3PIN
    };

    MAP_SPI_disableModule(spi_base);
    MAP_SPI_initMaster(spi_base, &spi_config);
    MAP_SPI_enableModule(spi_base);
    
    while (MAP_SPI_getInterruptStatus(spi_base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        (void)MAP_SPI_receiveData(spi_base);
        MAP_SPI_clearInterruptFlag(spi_base, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    }

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Send_SyncHandle[index]) != NeonRTOS_OK)
    {
        MAP_SPI_disableModule(spi_base);
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Recv_SyncHandle[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        MAP_SPI_disableModule(spi_base);
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_LockObjCreate(&Spi_Master_Access_Mutex[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
        MAP_SPI_disableModule(spi_base);
        return hwSPI_MemoryError;
    }

    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[index]);

    Spi_Master_Clock_Hz[index] = clock_rate_hz;
    Spi_Master_Mode[index] = opMode;
    Spi_Master_Use_CS[index] = cs;

    SPI_NVIC_Init(index);

    gpio_pin_init_status[miso_pin] = true;
    gpio_pin_init_status[mosi_pin] = true;
    gpio_pin_init_status[sclk_pin] = true;

    if (cs)
    {
        gpio_pin_init_status[cs_pin] = true;
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

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_OK;
    }

    uint32_t spi_base = SPI_Map_Soc_Base(index);

    if (spi_base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_NVIC_DeInit(index);

    MAP_SPI_disableInterrupt(spi_base,
                             EUSCI_B_SPI_RECEIVE_INTERRUPT |
                             EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_SPI_clearInterruptFlag(spi_base,
                               EUSCI_B_SPI_RECEIVE_INTERRUPT |
                               EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_SPI_disableModule(spi_base);

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

    uint32_t miso_mask = GPIO_Map_Soc_Pin_Mask(miso_pin);
    uint32_t mosi_mask = GPIO_Map_Soc_Pin_Mask(mosi_pin);
    uint32_t sclk_mask = GPIO_Map_Soc_Pin_Mask(sclk_pin);
    uint32_t cs_mask   = GPIO_Map_Soc_Pin_Mask(cs_pin);

    if (miso_port == 0 || mosi_port == 0 || sclk_port == 0 ||
        miso_mask == 0 || mosi_mask == 0 || sclk_mask == 0)
    {
        return hwSPI_InvalidParameter;
    }

    MAP_GPIO_setAsInputPin(miso_port, miso_mask);
    MAP_GPIO_setAsInputPin(mosi_port, mosi_mask);
    MAP_GPIO_setAsInputPin(sclk_port, sclk_mask);

    if (Spi_Master_Use_CS[index])
    {
        if (cs_port == 0 || cs_mask == 0)
        {
            return hwSPI_InvalidParameter;
        }

        MAP_GPIO_setAsInputPin(cs_port, cs_mask);
        gpio_pin_init_status[cs_pin] = false;
    }

    gpio_pin_init_status[miso_pin] = false;
    gpio_pin_init_status[mosi_pin] = false;
    gpio_pin_init_status[sclk_pin] = false;

    Spi_Master_Use_CS[index] = false;
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

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MAP_SPI_disableInterrupt(base, EUSCI_B_SPI_RECEIVE_INTERRUPT | EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    uint_fast8_t phase;
    uint_fast8_t polarity;

    switch (Spi_Master_Mode[index])
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
            phase    = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
            phase    = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
            phase    = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
            phase    = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
            break;
    }

    eUSCI_SPI_MasterConfig spi_config =
    {
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
        g_sys_clock_hz,
        clock_rate_hz,
        EUSCI_B_SPI_MSB_FIRST,
        phase,
        polarity,
        Spi_Master_Use_CS[index] ? EUSCI_B_SPI_4PIN_UCxSTE_ACTIVE_LOW : EUSCI_B_SPI_3PIN
    };

    MAP_SPI_disableModule(base);
    MAP_SPI_initMaster(base, &spi_config);
    MAP_SPI_enableModule(base);

    while (MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        (void)MAP_SPI_receiveData(base);
        MAP_SPI_clearInterruptFlag(base, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    }

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

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);

    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    MAP_SPI_disableInterrupt(base, EUSCI_B_SPI_RECEIVE_INTERRUPT | EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    uint_fast8_t phase;
    uint_fast8_t polarity;

    switch (opMode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
            phase    = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
            phase    = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
            phase    = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            polarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
            phase    = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
            break;
    }

    eUSCI_SPI_MasterConfig spi_config =
    {
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
        g_sys_clock_hz,
        Spi_Master_Clock_Hz[index],
        EUSCI_B_SPI_MSB_FIRST,
        phase,
        polarity,
        Spi_Master_Use_CS[index] ? EUSCI_B_SPI_4PIN_UCxSTE_ACTIVE_LOW : EUSCI_B_SPI_3PIN
    };

    MAP_SPI_disableModule(base);
    MAP_SPI_initMaster(base, &spi_config);
    MAP_SPI_enableModule(base);
    
    while (MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        (void)MAP_SPI_receiveData(base);
        MAP_SPI_clearInterruptFlag(base, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    }

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

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);
    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSP432P_SPI_Transfer *t = &spi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = TIMSP432P_SPI_TX;
    t->tx_buf = &dat;
    t->rx_buf = NULL;
    t->len    = 1;

    while (MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        (void)MAP_SPI_receiveData(base);
        MAP_SPI_clearInterruptFlag(base, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SPI_clearInterruptFlag(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_SPI_enableInterrupt(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == TIMSP432P_SPI_ERROR)
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

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);
    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSP432P_SPI_Transfer *t = &spi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = TIMSP432P_SPI_RX;
    t->tx_buf = NULL;
    t->rx_buf = dat;
    t->len    = 1;

    while (MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        (void)MAP_SPI_receiveData(base);
        MAP_SPI_clearInterruptFlag(base, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SPI_clearInterruptFlag(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_SPI_enableInterrupt(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == TIMSP432P_SPI_ERROR)
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

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);
    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSP432P_SPI_Transfer *t = &spi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = TIMSP432P_SPI_TXRX;
    t->tx_buf = &wr_dat;
    t->rx_buf = rd_dat;
    t->len    = 1;

    while (MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        (void)MAP_SPI_receiveData(base);
        MAP_SPI_clearInterruptFlag(base, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SPI_clearInterruptFlag(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_SPI_enableInterrupt(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == TIMSP432P_SPI_ERROR)
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

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);
    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSP432P_SPI_Transfer *t = &spi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = TIMSP432P_SPI_TX;
    t->tx_buf = buf;
    t->rx_buf = NULL;
    t->len    = len;

    while (MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        (void)MAP_SPI_receiveData(base);
        MAP_SPI_clearInterruptFlag(base, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SPI_clearInterruptFlag(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_SPI_enableInterrupt(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == TIMSP432P_SPI_ERROR)
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

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);
    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSP432P_SPI_Transfer *t = &spi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = TIMSP432P_SPI_RX;
    t->tx_buf = NULL;
    t->rx_buf = buf;
    t->len    = len;

    while (MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        (void)MAP_SPI_receiveData(base);
        MAP_SPI_clearInterruptFlag(base, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SPI_clearInterruptFlag(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_SPI_enableInterrupt(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == TIMSP432P_SPI_ERROR)
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

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    uint32_t base = SPI_Map_Soc_Base(index);
    if (base == 0)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    TIMSP432P_SPI_Transfer *t = &spi_xfer[index];
    memset(t, 0, sizeof(*t));

    t->state  = TIMSP432P_SPI_TXRX;
    t->tx_buf = tx_buf;
    t->rx_buf = rx_buf;
    t->len    = len;

    while (MAP_SPI_getInterruptStatus(base, EUSCI_B_SPI_RECEIVE_INTERRUPT))
    {
        (void)MAP_SPI_receiveData(base);
        MAP_SPI_clearInterruptFlag(base, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    }

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    MAP_SPI_clearInterruptFlag(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    MAP_SPI_enableInterrupt(base,
        EUSCI_B_SPI_RECEIVE_INTERRUPT |
        EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    SPI_IRQ_Process(index);

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK)
    {
        MAP_SPI_disableInterrupt(base,
            EUSCI_B_SPI_RECEIVE_INTERRUPT |
            EUSCI_B_SPI_TRANSMIT_INTERRUPT);

        t->state = TIMSP432P_SPI_ERROR;
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (t->state == TIMSP432P_SPI_ERROR)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Burst_Write(hwSPI_Index index, uint8_t* buf, uint32_t size)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    if (size != 0 && buf == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (size == 0)
    {
        return hwSPI_OK;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    hwDMA_OpResult dma_op_status = DMA_SPI_Write(index, buf, size);

    if (dma_op_status < hwDMA_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return SPI_Map_DMA_HW_Error_Code(dma_op_status);
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Burst_Read(hwSPI_Index index, uint8_t* buf, uint32_t size)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    if (size != 0 && buf == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (size == 0)
    {
        return hwSPI_OK;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    hwDMA_OpResult dma_op_status = DMA_SPI_Read(index, buf, size);

    if (dma_op_status < hwDMA_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return SPI_Map_DMA_HW_Error_Code(dma_op_status);
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

#endif // DEVICE_TIMSP432P