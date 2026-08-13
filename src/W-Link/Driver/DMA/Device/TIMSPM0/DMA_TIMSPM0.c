
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "NeonRTOS.h"

#if defined(DEVICE_TIMSPM0)

#include "DMA_TIMSPM0.h"

#include "DMA_TIMSPM0_Index.h"

/*
 * MSPM0 devices implement different numbers of DMA channels.  A single shared
 * channel is the portable default; callers are serialized by the channel
 * mutex.  A project can select another implemented channel at build time.
 */
#ifndef DMA_TIMSPM0_CHANNEL
#define DMA_TIMSPM0_CHANNEL 0U
#endif

#ifndef DMA_WAIT_ALLOCATED_TIMEOUT
#define DMA_WAIT_ALLOCATED_TIMEOUT 1000U
#endif

#ifndef DMA_WAIT_TRANSFER_TIMEOUT
#define DMA_WAIT_TRANSFER_TIMEOUT 1000U
#endif

#define DMA_I2C_MAX_TRANSFER_SIZE 0x0FFFU

#if !defined(DMA_SYS_N_DMA_CHANNEL)
#error "The selected MSPM0 device header does not define DMA_SYS_N_DMA_CHANNEL"
#elif (DMA_TIMSPM0_CHANNEL >= DMA_SYS_N_DMA_CHANNEL)
#error "DMA_TIMSPM0_CHANNEL is not implemented by the selected MSPM0 device"
#endif

/* Normalize the trigger names used by released UNICOMM families. */
#if defined(DMA_UC0_TX_BD_TRIG)
#define DMA_TIMSPM0_UC0_TX_TRIG DMA_UC0_TX_BD_TRIG
#define DMA_TIMSPM0_UC0_RX_TRIG DMA_UC0_RX_BD_TRIG
#elif defined(DMA_UC0_TX_TRIG)
#define DMA_TIMSPM0_UC0_TX_TRIG DMA_UC0_TX_TRIG
#define DMA_TIMSPM0_UC0_RX_TRIG DMA_UC0_RX_TRIG
#endif

#if defined(DMA_UC1_TX_BD_TRIG)
#define DMA_TIMSPM0_UC1_TX_TRIG DMA_UC1_TX_BD_TRIG
#define DMA_TIMSPM0_UC1_RX_TRIG DMA_UC1_RX_BD_TRIG
#elif defined(DMA_UC1_TX_TRIG)
#define DMA_TIMSPM0_UC1_TX_TRIG DMA_UC1_TX_TRIG
#define DMA_TIMSPM0_UC1_RX_TRIG DMA_UC1_RX_TRIG
#endif

#if defined(DMA_UC2_TX_BD_TRIG)
#define DMA_TIMSPM0_UC2_TX_TRIG DMA_UC2_TX_BD_TRIG
#define DMA_TIMSPM0_UC2_RX_TRIG DMA_UC2_RX_BD_TRIG
#elif defined(DMA_UC2_TX_TRIG)
#define DMA_TIMSPM0_UC2_TX_TRIG DMA_UC2_TX_TRIG
#define DMA_TIMSPM0_UC2_RX_TRIG DMA_UC2_RX_TRIG
#endif

#if defined(DMA_UC3_TX_BD_TRIG)
#define DMA_TIMSPM0_UC3_TX_TRIG DMA_UC3_TX_BD_TRIG
#define DMA_TIMSPM0_UC3_RX_TRIG DMA_UC3_RX_BD_TRIG
#elif defined(DMA_UC3_TX_TRIG)
#define DMA_TIMSPM0_UC3_TX_TRIG DMA_UC3_TX_TRIG
#define DMA_TIMSPM0_UC3_RX_TRIG DMA_UC3_RX_TRIG
#endif

#if defined(DMA_UC4_TX_BD_TRIG)
#define DMA_TIMSPM0_UC4_TX_TRIG DMA_UC4_TX_BD_TRIG
#define DMA_TIMSPM0_UC4_RX_TRIG DMA_UC4_RX_BD_TRIG
#elif defined(DMA_UC4_TX_TRIG)
#define DMA_TIMSPM0_UC4_TX_TRIG DMA_UC4_TX_TRIG
#define DMA_TIMSPM0_UC4_RX_TRIG DMA_UC4_RX_TRIG
#endif

#if defined(DMA_UC5_TX_BD_TRIG)
#define DMA_TIMSPM0_UC5_TX_TRIG DMA_UC5_TX_BD_TRIG
#define DMA_TIMSPM0_UC5_RX_TRIG DMA_UC5_RX_BD_TRIG
#elif defined(DMA_UC5_TX_TRIG)
#define DMA_TIMSPM0_UC5_TX_TRIG DMA_UC5_TX_TRIG
#define DMA_TIMSPM0_UC5_RX_TRIG DMA_UC5_RX_TRIG
#endif

#if defined(DMA_UC6_TX_BD_TRIG)
#define DMA_TIMSPM0_UC6_TX_TRIG DMA_UC6_TX_BD_TRIG
#define DMA_TIMSPM0_UC6_RX_TRIG DMA_UC6_RX_BD_TRIG
#elif defined(DMA_UC6_TX_TRIG)
#define DMA_TIMSPM0_UC6_TX_TRIG DMA_UC6_TX_TRIG
#define DMA_TIMSPM0_UC6_RX_TRIG DMA_UC6_RX_TRIG
#endif

#if defined(DMA_UC7_TX_BD_TRIG)
#define DMA_TIMSPM0_UC7_TX_TRIG DMA_UC7_TX_BD_TRIG
#define DMA_TIMSPM0_UC7_RX_TRIG DMA_UC7_RX_BD_TRIG
#elif defined(DMA_UC7_TX_TRIG)
#define DMA_TIMSPM0_UC7_TX_TRIG DMA_UC7_TX_TRIG
#define DMA_TIMSPM0_UC7_RX_TRIG DMA_UC7_RX_TRIG
#endif

#if defined(DMA_UC8_TX_BD_TRIG)
#define DMA_TIMSPM0_UC8_TX_TRIG DMA_UC8_TX_BD_TRIG
#define DMA_TIMSPM0_UC8_RX_TRIG DMA_UC8_RX_BD_TRIG
#elif defined(DMA_UC8_TX_TRIG)
#define DMA_TIMSPM0_UC8_TX_TRIG DMA_UC8_TX_TRIG
#define DMA_TIMSPM0_UC8_RX_TRIG DMA_UC8_RX_TRIG
#endif

#if defined(DMA_UC9_TX_BD_TRIG)
#define DMA_TIMSPM0_UC9_TX_TRIG DMA_UC9_TX_BD_TRIG
#define DMA_TIMSPM0_UC9_RX_TRIG DMA_UC9_RX_BD_TRIG
#elif defined(DMA_UC9_TX_TRIG)
#define DMA_TIMSPM0_UC9_TX_TRIG DMA_UC9_TX_TRIG
#define DMA_TIMSPM0_UC9_RX_TRIG DMA_UC9_RX_TRIG
#endif

#define DMA_CHANNEL_LOCK(channel_index)                                      \
    do {                                                                     \
        if (NeonRTOS_LockObjLock(&DMA_Channel_Mutex[(channel_index)],        \
                DMA_WAIT_ALLOCATED_TIMEOUT) != NeonRTOS_OK) {                \
            return hwDMA_MutexTimeout;                                       \
        }                                                                    \
    } while (0)

typedef enum {
    TIMSPM0_DMA_PORT_UART,
    TIMSPM0_DMA_PORT_SPI,
    TIMSPM0_DMA_PORT_I2C
} TIMSPM0_DMA_PortType;

typedef struct {
    volatile uint32_t *txData;
    volatile const uint32_t *rxData;
    volatile uint32_t *txEventMask;
    volatile uint32_t *rxEventMask;
    uint32_t txEventValue;
    uint32_t rxEventValue;
    uint8_t txTrigger;
    uint8_t rxTrigger;
    void *owner;
} TIMSPM0_DMA_PortDef;

bool DMA_Channel_Init_Status[hwDMA_Channel_Index_MAX] = {false};
NeonRTOS_LockObj_t DMA_Channel_Mutex[hwDMA_Channel_Index_MAX] = {NULL};

static hwDMA_OpResult DMA_Unlock_Channel(hwDMA_OpResult result)
{
    if (NeonRTOS_LockObjUnlock(&DMA_Channel_Mutex[DMA_TIMSPM0_CHANNEL])
        != NeonRTOS_OK) {
        return hwDMA_MutexTimeout;
    }

    return result;
}

#if !defined(__MCU_HAS_UNICOMMUART__)
typedef struct {
    UART_Regs *regs;
    uint8_t txTrigger;
    uint8_t rxTrigger;
} TIMSPM0_DMA_UARTDef;

static bool DMA_Map_Standalone_UART(
    uint32_t index, TIMSPM0_DMA_PortDef *port)
{
#if defined(DMA_UART0_TX_TRIG) || defined(DMA_UART1_TX_TRIG) ||             \
    defined(DMA_UART2_TX_TRIG) || defined(DMA_UART3_TX_TRIG) ||             \
    defined(DMA_UART4_TX_TRIG) || defined(DMA_UART5_TX_TRIG) ||             \
    defined(DMA_UART6_TX_TRIG) || defined(DMA_UART7_TX_TRIG)
    const TIMSPM0_DMA_UARTDef defs[] = {
#if defined(UART0_BASE) && defined(DMA_UART0_TX_TRIG) && defined(DMA_UART0_RX_TRIG)
        { UART0, DMA_UART0_TX_TRIG, DMA_UART0_RX_TRIG },
#endif
#if defined(UART1_BASE) && defined(DMA_UART1_TX_TRIG) && defined(DMA_UART1_RX_TRIG)
        { UART1, DMA_UART1_TX_TRIG, DMA_UART1_RX_TRIG },
#endif
#if defined(UART2_BASE) && defined(DMA_UART2_TX_TRIG) && defined(DMA_UART2_RX_TRIG)
        { UART2, DMA_UART2_TX_TRIG, DMA_UART2_RX_TRIG },
#endif
#if defined(UART3_BASE) && defined(DMA_UART3_TX_TRIG) && defined(DMA_UART3_RX_TRIG)
        { UART3, DMA_UART3_TX_TRIG, DMA_UART3_RX_TRIG },
#endif
#if defined(UART4_BASE) && defined(DMA_UART4_TX_TRIG) && defined(DMA_UART4_RX_TRIG)
        { UART4, DMA_UART4_TX_TRIG, DMA_UART4_RX_TRIG },
#endif
#if defined(UART5_BASE) && defined(DMA_UART5_TX_TRIG) && defined(DMA_UART5_RX_TRIG)
        { UART5, DMA_UART5_TX_TRIG, DMA_UART5_RX_TRIG },
#endif
#if defined(UART6_BASE) && defined(DMA_UART6_TX_TRIG) && defined(DMA_UART6_RX_TRIG)
        { UART6, DMA_UART6_TX_TRIG, DMA_UART6_RX_TRIG },
#endif
#if defined(UART7_BASE) && defined(DMA_UART7_TX_TRIG) && defined(DMA_UART7_RX_TRIG)
        { UART7, DMA_UART7_TX_TRIG, DMA_UART7_RX_TRIG },
#endif
    };

    if (index >= (sizeof(defs) / sizeof(defs[0]))) {
        return false;
    }

    port->txData       = &defs[index].regs->TXDATA;
    port->rxData       = &defs[index].regs->RXDATA;
    port->txEventMask  = &defs[index].regs->DMA_TRIG_TX.IMASK;
    port->rxEventMask  = &defs[index].regs->DMA_TRIG_RX.IMASK;
    port->txEventValue = DL_UART_DMA_INTERRUPT_TX;
    port->rxEventValue = DL_UART_DMA_INTERRUPT_RX;
    port->txTrigger    = defs[index].txTrigger;
    port->rxTrigger    = defs[index].rxTrigger;
    port->owner        = defs[index].regs;

    return true;
#else
    (void)index;
    (void)port;
    return false;
#endif
}
#endif

#if !defined(__MCU_HAS_UNICOMMSPI__)
typedef struct {
    SPI_Regs *regs;
    uint8_t txTrigger;
    uint8_t rxTrigger;
} TIMSPM0_DMA_SPIDef;

static bool DMA_Map_Standalone_SPI(
    uint32_t index, TIMSPM0_DMA_PortDef *port)
{
#if defined(DMA_SPI0_TX_TRIG) || defined(DMA_SPI1_TX_TRIG) || \
    defined(DMA_SPI2_TX_TRIG)
    const TIMSPM0_DMA_SPIDef defs[] = {
#if defined(SPI0_BASE) && defined(DMA_SPI0_TX_TRIG) && defined(DMA_SPI0_RX_TRIG)
        { SPI0, DMA_SPI0_TX_TRIG, DMA_SPI0_RX_TRIG },
#endif
#if defined(SPI1_BASE) && defined(DMA_SPI1_TX_TRIG) && defined(DMA_SPI1_RX_TRIG)
        { SPI1, DMA_SPI1_TX_TRIG, DMA_SPI1_RX_TRIG },
#endif
#if defined(SPI2_BASE) && defined(DMA_SPI2_TX_TRIG) && defined(DMA_SPI2_RX_TRIG)
        { SPI2, DMA_SPI2_TX_TRIG, DMA_SPI2_RX_TRIG },
#endif
    };

    if (index >= (sizeof(defs) / sizeof(defs[0]))) {
        return false;
    }

    port->txData       = &defs[index].regs->TXDATA;
    port->rxData       = &defs[index].regs->RXDATA;
    port->txEventMask  = &defs[index].regs->DMA_TRIG_TX.IMASK;
    port->rxEventMask  = &defs[index].regs->DMA_TRIG_RX.IMASK;
    port->txEventValue = DL_SPI_DMA_INTERRUPT_TX;
    port->rxEventValue = DL_SPI_DMA_INTERRUPT_RX;
    port->txTrigger    = defs[index].txTrigger;
    port->rxTrigger    = defs[index].rxTrigger;
    port->owner        = defs[index].regs;

    return true;
#else
    (void)index;
    (void)port;
    return false;
#endif
}
#endif

#if !defined(__MCU_HAS_UNICOMMI2CC__)
typedef struct {
    I2C_Regs *regs;
    uint8_t txTrigger;
    uint8_t rxTrigger;
} TIMSPM0_DMA_I2CDef;

static bool DMA_Map_Standalone_I2C(
    uint32_t index, TIMSPM0_DMA_PortDef *port)
{
#if defined(DMA_I2C0_TX_TRIG) || defined(DMA_I2C1_TX_TRIG) || \
    defined(DMA_I2C2_TX_TRIG)
    const TIMSPM0_DMA_I2CDef defs[] = {
#if defined(I2C0_BASE) && defined(DMA_I2C0_TX_TRIG) && defined(DMA_I2C0_RX_TRIG)
        { I2C0, DMA_I2C0_TX_TRIG, DMA_I2C0_RX_TRIG },
#endif
#if defined(I2C1_BASE) && defined(DMA_I2C1_TX_TRIG) && defined(DMA_I2C1_RX_TRIG)
        { I2C1, DMA_I2C1_TX_TRIG, DMA_I2C1_RX_TRIG },
#endif
#if defined(I2C2_BASE) && defined(DMA_I2C2_TX_TRIG) && defined(DMA_I2C2_RX_TRIG)
        { I2C2, DMA_I2C2_TX_TRIG, DMA_I2C2_RX_TRIG },
#endif
    };

    if (index >= (sizeof(defs) / sizeof(defs[0]))) {
        return false;
    }

    port->txData       = &defs[index].regs->MASTER.MTXDATA;
    port->rxData       = &defs[index].regs->MASTER.MRXDATA;
    port->txEventMask  = &defs[index].regs->DMA_TRIG1.IMASK;
    port->rxEventMask  = &defs[index].regs->DMA_TRIG1.IMASK;
    port->txEventValue = DL_I2C_DMA_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER;
    port->rxEventValue = DL_I2C_DMA_INTERRUPT_CONTROLLER_RXFIFO_TRIGGER;
    port->txTrigger    = defs[index].txTrigger;
    port->rxTrigger    = defs[index].rxTrigger;
    port->owner        = defs[index].regs;

    return true;
#else
    (void)index;
    (void)port;
    return false;
#endif
}
#endif

#if defined(__MCU_HAS_UNICOMMUART__) || defined(__MCU_HAS_UNICOMMSPI__) || \
    defined(__MCU_HAS_UNICOMMI2CC__)
typedef struct {
    UNICOMM_Inst_Regs *regs;
    uint8_t txTrigger;
    uint8_t rxTrigger;
} TIMSPM0_DMA_UnicommDef;

static bool DMA_Unicomm_Mode_Has_DMA(
    UNICOMM_Inst_Regs *regs, TIMSPM0_DMA_PortType type)
{
    /* Newer headers expose per-mode DMA capability flags. */
#if defined(UC0_UART_SYS_DMA)
    if ((regs == UC0) && (type == TIMSPM0_DMA_PORT_UART)) {
        return (UC0_UART_SYS_DMA != 0);
    }
#endif
#if defined(UC1_UART_SYS_DMA)
    if ((regs == UC1) && (type == TIMSPM0_DMA_PORT_UART)) {
        return (UC1_UART_SYS_DMA != 0);
    }
#endif
#if defined(UC3_UART_SYS_DMA)
    if ((regs == UC3) && (type == TIMSPM0_DMA_PORT_UART)) {
        return (UC3_UART_SYS_DMA != 0);
    }
#endif
#if defined(UC4_UART_SYS_DMA)
    if ((regs == UC4) && (type == TIMSPM0_DMA_PORT_UART)) {
        return (UC4_UART_SYS_DMA != 0);
    }
#endif
#if defined(UC8_UART_SYS_DMA)
    if ((regs == UC8) && (type == TIMSPM0_DMA_PORT_UART)) {
        return (UC8_UART_SYS_DMA != 0);
    }
#endif
#if defined(UC11_UART_SYS_DMA)
    if ((regs == UC11) && (type == TIMSPM0_DMA_PORT_UART)) {
        return (UC11_UART_SYS_DMA != 0);
    }
#endif
#if defined(UC0_SPI_SYS_EN_DMA)
    if ((regs == UC0) && (type == TIMSPM0_DMA_PORT_SPI)) {
        return (UC0_SPI_SYS_EN_DMA != 0);
    }
#endif
#if defined(UC2_SPI_SYS_EN_DMA)
    if ((regs == UC2) && (type == TIMSPM0_DMA_PORT_SPI)) {
        return (UC2_SPI_SYS_EN_DMA != 0);
    }
#endif
#if defined(UC3_SPI_SYS_EN_DMA)
    if ((regs == UC3) && (type == TIMSPM0_DMA_PORT_SPI)) {
        return (UC3_SPI_SYS_EN_DMA != 0);
    }
#endif
#if defined(UC4_SPI_SYS_EN_DMA)
    if ((regs == UC4) && (type == TIMSPM0_DMA_PORT_SPI)) {
        return (UC4_SPI_SYS_EN_DMA != 0);
    }
#endif
#if defined(UC8_SPI_SYS_EN_DMA)
    if ((regs == UC8) && (type == TIMSPM0_DMA_PORT_SPI)) {
        return (UC8_SPI_SYS_EN_DMA != 0);
    }
#endif

    (void)regs;
    (void)type;
    return true;
}

static bool DMA_Map_Unicomm(
    uint32_t index, TIMSPM0_DMA_PortType type, TIMSPM0_DMA_PortDef *port)
{
#if defined(DMA_TIMSPM0_UC0_TX_TRIG) || defined(DMA_TIMSPM0_UC1_TX_TRIG) || \
    defined(DMA_TIMSPM0_UC2_TX_TRIG) || defined(DMA_TIMSPM0_UC3_TX_TRIG) || \
    defined(DMA_TIMSPM0_UC4_TX_TRIG) || defined(DMA_TIMSPM0_UC5_TX_TRIG) || \
    defined(DMA_TIMSPM0_UC6_TX_TRIG) || defined(DMA_TIMSPM0_UC7_TX_TRIG) || \
    defined(DMA_TIMSPM0_UC8_TX_TRIG) || defined(DMA_TIMSPM0_UC9_TX_TRIG)
    /* Logical indices follow ascending physical UNICOMM instance number. */
    const TIMSPM0_DMA_UnicommDef defs[] = {
#if defined(UC0_BASE) && defined(DMA_TIMSPM0_UC0_TX_TRIG)
        { UC0, DMA_TIMSPM0_UC0_TX_TRIG, DMA_TIMSPM0_UC0_RX_TRIG },
#endif
#if defined(UC1_BASE) && defined(DMA_TIMSPM0_UC1_TX_TRIG)
        { UC1, DMA_TIMSPM0_UC1_TX_TRIG, DMA_TIMSPM0_UC1_RX_TRIG },
#endif
#if defined(UC2_BASE) && defined(DMA_TIMSPM0_UC2_TX_TRIG)
        { UC2, DMA_TIMSPM0_UC2_TX_TRIG, DMA_TIMSPM0_UC2_RX_TRIG },
#endif
#if defined(UC3_BASE) && defined(DMA_TIMSPM0_UC3_TX_TRIG)
        { UC3, DMA_TIMSPM0_UC3_TX_TRIG, DMA_TIMSPM0_UC3_RX_TRIG },
#endif
#if defined(UC4_BASE) && defined(DMA_TIMSPM0_UC4_TX_TRIG)
        { UC4, DMA_TIMSPM0_UC4_TX_TRIG, DMA_TIMSPM0_UC4_RX_TRIG },
#endif
#if defined(UC5_BASE) && defined(DMA_TIMSPM0_UC5_TX_TRIG)
        { UC5, DMA_TIMSPM0_UC5_TX_TRIG, DMA_TIMSPM0_UC5_RX_TRIG },
#endif
#if defined(UC6_BASE) && defined(DMA_TIMSPM0_UC6_TX_TRIG)
        { UC6, DMA_TIMSPM0_UC6_TX_TRIG, DMA_TIMSPM0_UC6_RX_TRIG },
#endif
#if defined(UC7_BASE) && defined(DMA_TIMSPM0_UC7_TX_TRIG)
        { UC7, DMA_TIMSPM0_UC7_TX_TRIG, DMA_TIMSPM0_UC7_RX_TRIG },
#endif
#if defined(UC8_BASE) && defined(DMA_TIMSPM0_UC8_TX_TRIG)
        { UC8, DMA_TIMSPM0_UC8_TX_TRIG, DMA_TIMSPM0_UC8_RX_TRIG },
#endif
#if defined(UC9_BASE) && defined(DMA_TIMSPM0_UC9_TX_TRIG)
        { UC9, DMA_TIMSPM0_UC9_TX_TRIG, DMA_TIMSPM0_UC9_RX_TRIG },
#endif
    };
    uint32_t logicalIndex = 0U;
    size_t i;

    for (i = 0U; i < (sizeof(defs) / sizeof(defs[0])); ++i) {
        bool hasMode = false;

#if defined(__MCU_HAS_UNICOMMUART__)
        if (type == TIMSPM0_DMA_PORT_UART) {
            hasMode = (defs[i].regs->uart != NULL);
        }
#endif
#if defined(__MCU_HAS_UNICOMMSPI__)
        if (type == TIMSPM0_DMA_PORT_SPI) {
            hasMode = (defs[i].regs->spi != NULL);
        }
#endif
#if defined(__MCU_HAS_UNICOMMI2CC__)
        if (type == TIMSPM0_DMA_PORT_I2C) {
            hasMode = (defs[i].regs->i2cc != NULL);
        }
#endif
        if (!hasMode) {
            continue;
        }

        if (logicalIndex++ != index) {
            continue;
        }

        if (!DMA_Unicomm_Mode_Has_DMA(defs[i].regs, type)) {
            return false;
        }

#if defined(__MCU_HAS_UNICOMMUART__)
        if (type == TIMSPM0_DMA_PORT_UART) {
            port->txData       = &defs[i].regs->uart->TXDATA;
            port->rxData       = &defs[i].regs->uart->RXDATA;
            port->txEventMask  = &defs[i].regs->uart->DMA_TRIG_TX.IMASK;
            port->rxEventMask  = &defs[i].regs->uart->DMA_TRIG_RX.IMASK;
            port->txEventValue = DL_UART_DMA_INTERRUPT_TX;
            port->rxEventValue = DL_UART_DMA_INTERRUPT_RX;
        } else
#endif
#if defined(__MCU_HAS_UNICOMMSPI__)
        if (type == TIMSPM0_DMA_PORT_SPI) {
            port->txData       = &defs[i].regs->spi->TXDATA;
            port->rxData       = &defs[i].regs->spi->RXDATA;
            port->txEventMask  = &defs[i].regs->spi->DMA_TRIG_TX.IMASK;
            port->rxEventMask  = &defs[i].regs->spi->DMA_TRIG_RX.IMASK;
            port->txEventValue = DL_SPI_DMA_INTERRUPT_TX;
            port->rxEventValue = DL_SPI_DMA_INTERRUPT_RX;
        } else
#endif
#if defined(__MCU_HAS_UNICOMMI2CC__)
        if (type == TIMSPM0_DMA_PORT_I2C) {
            port->txData       = &defs[i].regs->i2cc->TXDATA;
            port->rxData       = &defs[i].regs->i2cc->RXDATA;
            port->txEventMask  = &defs[i].regs->i2cc->DMA_TRIG1.IMASK;
            port->rxEventMask  = &defs[i].regs->i2cc->DMA_TRIG1.IMASK;
            port->txEventValue = DL_I2CC_DMA_INTERRUPT_TXFIFO_TRIGGER;
            port->rxEventValue = DL_I2CC_DMA_INTERRUPT_RXFIFO_TRIGGER;
        } else
#endif
        {
            return false;
        }

        port->txTrigger = defs[i].txTrigger;
        port->rxTrigger = defs[i].rxTrigger;
        port->owner     = defs[i].regs;
        return true;
    }

    return false;
#else
    (void)index;
    (void)type;
    (void)port;
    return false;
#endif
}
#endif

static bool DMA_Map_UART(uint32_t index, TIMSPM0_DMA_PortDef *port)
{
#if defined(__MCU_HAS_UNICOMMUART__)
    return DMA_Map_Unicomm(index, TIMSPM0_DMA_PORT_UART, port);
#else
    return DMA_Map_Standalone_UART(index, port);
#endif
}

static bool DMA_Map_SPI(uint32_t index, TIMSPM0_DMA_PortDef *port)
{
#if defined(__MCU_HAS_UNICOMMSPI__)
    return DMA_Map_Unicomm(index, TIMSPM0_DMA_PORT_SPI, port);
#else
    return DMA_Map_Standalone_SPI(index, port);
#endif
}

static bool DMA_Map_I2C(uint32_t index, TIMSPM0_DMA_PortDef *port)
{
#if defined(__MCU_HAS_UNICOMMI2CC__)
    return DMA_Map_Unicomm(index, TIMSPM0_DMA_PORT_I2C, port);
#else
    return DMA_Map_Standalone_I2C(index, port);
#endif
}

static void DMA_Prepare_Channel(const TIMSPM0_DMA_PortDef *port,
    hwDMA_Peripheral_Direction dir, uint8_t *buf, uint16_t len)
{
    const uint8_t channel = (uint8_t)DMA_TIMSPM0_CHANNEL;

    DL_DMA_disableChannel(DMA, channel);

    if (dir == hwDMA_Peripheral_Direction_TX) {
        DL_DMA_configTransfer(DMA, channel, DL_DMA_SINGLE_TRANSFER_MODE,
            DL_DMA_NORMAL_MODE, DL_DMA_WIDTH_BYTE, DL_DMA_WIDTH_BYTE,
            DL_DMA_ADDR_INCREMENT, DL_DMA_ADDR_UNCHANGED);
        DL_DMA_setSrcAddr(DMA, channel, (uint32_t)(uintptr_t)buf);
        DL_DMA_setDestAddr(
            DMA, channel, (uint32_t)(uintptr_t)port->txData);
        DL_DMA_setTrigger(DMA, channel, port->txTrigger,
            DL_DMA_TRIGGER_TYPE_EXTERNAL);
    } else {
        DL_DMA_configTransfer(DMA, channel, DL_DMA_SINGLE_TRANSFER_MODE,
            DL_DMA_NORMAL_MODE, DL_DMA_WIDTH_BYTE, DL_DMA_WIDTH_BYTE,
            DL_DMA_ADDR_UNCHANGED, DL_DMA_ADDR_INCREMENT);
        DL_DMA_setSrcAddr(
            DMA, channel, (uint32_t)(uintptr_t)port->rxData);
        DL_DMA_setDestAddr(DMA, channel, (uint32_t)(uintptr_t)buf);
        DL_DMA_setTrigger(DMA, channel, port->rxTrigger,
            DL_DMA_TRIGGER_TYPE_EXTERNAL);
    }

    DL_DMA_setTransferSize(DMA, channel, len);
}

static hwDMA_OpResult DMA_Wait_Channel_Done(void)
{
    uint32_t timeout = DMA_WAIT_TRANSFER_TIMEOUT;
    const uint8_t channel = (uint8_t)DMA_TIMSPM0_CHANNEL;

    while (DL_DMA_getTransferSize(DMA, channel) != 0U) {
        if (timeout-- == 0U) {
            DL_DMA_disableChannel(DMA, channel);
            return hwDMA_XferTimeout;
        }

        NeonRTOS_DelayMs(1U);
    }

    return hwDMA_OK;
}

static hwDMA_OpResult DMA_Wait_I2C_Idle(void *owner)
{
    uint32_t timeout = DMA_WAIT_TRANSFER_TIMEOUT;

#if defined(__MCU_HAS_UNICOMMI2CC__)
    UNICOMM_Inst_Regs *i2c = (UNICOMM_Inst_Regs *)owner;

    while ((DL_I2CC_getStatus(i2c) & DL_I2CC_STATUS_BUSY) != 0U) {
#else
    I2C_Regs *i2c = (I2C_Regs *)owner;

    while ((DL_I2C_getControllerStatus(i2c)
            & DL_I2C_CONTROLLER_STATUS_BUSY) != 0U) {
#endif
        if (timeout-- == 0U) {
            return hwDMA_XferTimeout;
        }

        NeonRTOS_DelayMs(1U);
    }

    return hwDMA_OK;
}

static void DMA_Start_I2C(void *owner, hwDMA_Peripheral_Direction dir,
    uint16_t devAddr, uint16_t len)
{
#if defined(__MCU_HAS_UNICOMMI2CC__)
    DL_I2CC_startTransfer((UNICOMM_Inst_Regs *)owner, devAddr,
        (dir == hwDMA_Peripheral_Direction_TX) ? DL_I2CC_DIRECTION_TX
                                               : DL_I2CC_DIRECTION_RX,
        len);
#else
    DL_I2C_startControllerTransfer((I2C_Regs *)owner, devAddr,
        (dir == hwDMA_Peripheral_Direction_TX)
            ? DL_I2C_CONTROLLER_DIRECTION_TX
            : DL_I2C_CONTROLLER_DIRECTION_RX,
        len);
#endif
}

static hwDMA_OpResult DMA_Execute_Transfer(const TIMSPM0_DMA_PortDef *port,
    hwDMA_Peripheral_Direction dir, uint8_t *buf, uint16_t len,
    bool startI2C, uint16_t devAddr)
{
    volatile uint32_t *eventMask;
    uint32_t eventValue;
    uint32_t savedEventMask;
    hwDMA_OpResult result;
    const uint8_t channel = (uint8_t)DMA_TIMSPM0_CHANNEL;

    DMA_CHANNEL_LOCK(DMA_TIMSPM0_CHANNEL);

    if (dir == hwDMA_Peripheral_Direction_TX) {
        eventMask  = port->txEventMask;
        eventValue = port->txEventValue;
    } else {
        eventMask  = port->rxEventMask;
        eventValue = port->rxEventValue;
    }

    DMA_Prepare_Channel(port, dir, buf, len);

    /* Preserve a peripheral event setup owned by another driver. */
    savedEventMask = *eventMask;
    *eventMask     = eventValue;

    DL_DMA_enableChannel(DMA, channel);

    if (startI2C) {
        DMA_Start_I2C(port->owner, dir, devAddr, len);
    }

    result = DMA_Wait_Channel_Done();

    if ((result == hwDMA_OK) && startI2C) {
        result = DMA_Wait_I2C_Idle(port->owner);
    }

    DL_DMA_disableChannel(DMA, channel);
    *eventMask = savedEventMask;

    return DMA_Unlock_Channel(result);
}

hwDMA_OpResult DMA_HW_Init(void)
{
    uint8_t channel;

    for (channel = 0U; channel < (uint8_t)DMA_SYS_N_DMA_CHANNEL; ++channel) {
        DL_DMA_disableChannel(DMA, channel);
    }

    return hwDMA_OK;
}

hwDMA_OpResult DMA_HW_DeInit(void)
{
    uint8_t channel;

    for (channel = 0U; channel < (uint8_t)DMA_SYS_N_DMA_CHANNEL; ++channel) {
        DL_DMA_disableChannel(DMA, channel);
    }

    return hwDMA_OK;
}

hwDMA_OpResult DMA_Xfer_UART(hwUART_Index index,
    hwDMA_Peripheral_Direction dir, uint8_t *buf, size_t len)
{
    TIMSPM0_DMA_PortDef port;

    if ((buf == NULL) || (len == 0U) || (len > 0xFFFFU)
        || ((uint32_t)index >= (uint32_t)hwUART_Index_MAX)
        || ((dir != hwDMA_Peripheral_Direction_TX)
            && (dir != hwDMA_Peripheral_Direction_RX))
        || !DMA_Map_UART((uint32_t)index, &port)) {
        return hwDMA_InvalidParameter;
    }

    return DMA_Execute_Transfer(
        &port, dir, buf, (uint16_t)len, false, 0U);
}

hwDMA_OpResult DMA_Xfer_I2C(hwI2C_Index index,
    hwDMA_Peripheral_Direction dir, uint16_t dev_addr, uint8_t *buf,
    size_t len)
{
    TIMSPM0_DMA_PortDef port;

    if ((buf == NULL) || (len == 0U) || (len > DMA_I2C_MAX_TRANSFER_SIZE)
        || (dev_addr > 0x03FFU)
        || ((uint32_t)index >= (uint32_t)hwI2C_Index_MAX)
        || ((dir != hwDMA_Peripheral_Direction_TX)
            && (dir != hwDMA_Peripheral_Direction_RX))
        || !DMA_Map_I2C((uint32_t)index, &port)) {
        return hwDMA_InvalidParameter;
    }

    return DMA_Execute_Transfer(
        &port, dir, buf, (uint16_t)len, true, dev_addr);
}

hwDMA_OpResult DMA_Xfer_SPI(hwSPI_Index index,
    hwDMA_Peripheral_Direction dir, uint8_t *buf, size_t len)
{
    TIMSPM0_DMA_PortDef port;

    if ((buf == NULL) || (len == 0U) || (len > 0xFFFFU)
        || ((uint32_t)index >= (uint32_t)hwSPI_Index_MAX)
        || ((dir != hwDMA_Peripheral_Direction_TX)
            && (dir != hwDMA_Peripheral_Direction_RX))
        || !DMA_Map_SPI((uint32_t)index, &port)) {
        return hwDMA_InvalidParameter;
    }

    return DMA_Execute_Transfer(
        &port, dir, buf, (uint16_t)len, false, 0U);
}

hwDMA_OpResult DMA_Init(void)
{
    for (hwDMA_Channel_Index i = 0; i < hwDMA_Channel_Index_MAX; i++)
    {
        if (NeonRTOS_LockObjCreate(&DMA_Channel_Mutex[i]) != NeonRTOS_OK)
        {
            return hwDMA_MemoryError;
        }

        DMA_Channel_Init_Status[i] = true;
    }

    DMA_HW_Init();

    return hwDMA_OK;
}

hwDMA_OpResult DMA_DeInit(void)
{
    for (hwDMA_Channel_Index i = 0; i < hwDMA_Channel_Index_MAX; i++)
    {
        if (DMA_Channel_Init_Status[i])
        {
            NeonRTOS_LockObjDelete(&DMA_Channel_Mutex[i]);
            DMA_Channel_Init_Status[i] = false;
        }
    }

    DMA_HW_DeInit();

    return hwDMA_OK;
}

hwDMA_OpResult DMA_Uart_Tx(hwUART_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_UART(index, hwDMA_Peripheral_Direction_TX, buf, len);
}

hwDMA_OpResult DMA_Uart_Rx(hwUART_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_UART(index, hwDMA_Peripheral_Direction_RX, buf, len);
}

hwDMA_OpResult DMA_I2C_Write(hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len)
{
    return DMA_Xfer_I2C(index, hwDMA_Peripheral_Direction_TX, dev_addr, buf, len);
}

hwDMA_OpResult DMA_I2C_Read(hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len)
{
    return DMA_Xfer_I2C(index, hwDMA_Peripheral_Direction_RX, dev_addr, buf, len);
}

hwDMA_OpResult DMA_SPI_Write(hwSPI_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_SPI(index, hwDMA_Peripheral_Direction_TX, buf, len);
}

hwDMA_OpResult DMA_SPI_Read(hwSPI_Index index, uint8_t *buf, size_t len)
{
    return DMA_Xfer_SPI(index, hwDMA_Peripheral_Direction_RX, buf, len);
}

#endif //DEVICE_TIMSPM0
