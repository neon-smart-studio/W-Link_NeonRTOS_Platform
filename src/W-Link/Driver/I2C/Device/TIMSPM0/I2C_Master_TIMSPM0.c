#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "I2C/I2C_Master.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSPM0

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"
#include "I2C/Pin/TIMSPM0/I2C_Pin_TIMSPM0.h"

#if defined(I2C0_BASE) || defined(I2C1_BASE) || defined(I2C2_BASE)
#define I2C_TIMSPM0_HAS_LEGACY_I2C
#endif

#if defined(UC0_I2CC_BASE) || defined(UC1_I2CC_BASE) || \
    defined(UC5_I2CC_BASE) || defined(UC6_I2CC_BASE)
#define I2C_TIMSPM0_HAS_UNICOMM_I2CC
#endif

#ifndef I2C_TIMSPM0_POWER_STARTUP_DELAY
#define I2C_TIMSPM0_POWER_STARTUP_DELAY       (16U)
#endif

#define I2C_TIMSPM0_STANDARD_SPEED_HZ          (100000UL)
#define I2C_TIMSPM0_FAST_SPEED_HZ              (400000UL)
#define I2C_TIMSPM0_MAX_7BIT_ADDRESS           (0x7FU)
#define I2C_TIMSPM0_TIMER_CLOCKS_PER_BIT       (10UL)
#define I2C_TIMSPM0_TIMER_DIVISOR_MAX          (128UL)

/*
 * Backend-independent interrupt flags.
 *
 * Legacy I2C uses DL_I2C_INTERRUPT_CONTROLLER_* while UNICOMM I2CC
 * uses DL_I2CC_INTERRUPT_*.  Keep the transfer state machine independent
 * from those two DriverLib namespaces.
 */
#define I2C_TIMSPM0_INT_RX_DONE                (1UL << 0)
#define I2C_TIMSPM0_INT_TX_DONE                (1UL << 1)
#define I2C_TIMSPM0_INT_RXFIFO_TRIGGER         (1UL << 2)
#define I2C_TIMSPM0_INT_TXFIFO_TRIGGER         (1UL << 3)
#define I2C_TIMSPM0_INT_NACK                   (1UL << 4)
#define I2C_TIMSPM0_INT_STOP                   (1UL << 5)
#define I2C_TIMSPM0_INT_ARBITRATION_LOST       (1UL << 6)

#define I2C_TIMSPM0_ALL_INTERRUPTS                                  \
    (I2C_TIMSPM0_INT_RX_DONE | I2C_TIMSPM0_INT_TX_DONE |            \
     I2C_TIMSPM0_INT_RXFIFO_TRIGGER | I2C_TIMSPM0_INT_TXFIFO_TRIGGER | \
     I2C_TIMSPM0_INT_NACK | I2C_TIMSPM0_INT_STOP |                  \
     I2C_TIMSPM0_INT_ARBITRATION_LOST)

typedef enum
{
    TIMSPM0_I2C_HW_NONE = 0,
    TIMSPM0_I2C_HW_LEGACY,
    TIMSPM0_I2C_HW_UNICOMM
} TIMSPM0_I2C_HwType;

typedef enum
{
    TIMSPM0_I2C_EVENT_NONE = 0,
    TIMSPM0_I2C_EVENT_RX_DONE,
    TIMSPM0_I2C_EVENT_TX_DONE,
    TIMSPM0_I2C_EVENT_RXFIFO,
    TIMSPM0_I2C_EVENT_TXFIFO,
    TIMSPM0_I2C_EVENT_STOP,
    TIMSPM0_I2C_EVENT_NACK,
    TIMSPM0_I2C_EVENT_ARBITRATION_LOST,
    TIMSPM0_I2C_EVENT_OTHER
} TIMSPM0_I2C_Event;

typedef enum
{
    TIMSPM0_I2C_IDLE = 0,
    TIMSPM0_I2C_TX,
    TIMSPM0_I2C_TX_WAIT_STOP,
    TIMSPM0_I2C_RX,
    TIMSPM0_I2C_RX_WAIT_STOP,
    TIMSPM0_I2C_DONE,
    TIMSPM0_I2C_ERROR
} TIMSPM0_I2C_State;

typedef struct
{
    volatile TIMSPM0_I2C_State state;
    uint8_t address;
    const uint8_t *tx_buf;
    uint16_t tx_len;
    volatile uint16_t tx_pos;
    uint8_t *rx_buf;
    uint16_t rx_len;
    volatile uint16_t rx_pos;
    bool stop;
    volatile uint32_t error;
} TIMSPM0_I2C_Transfer;

bool I2C_Master_Init_Status[hwI2C_Index_MAX] = {false};

static hwI2C_Speed_Mode I2C_Clock_Speed_Mode[hwI2C_Index_MAX] = {hwI2C_Standard_Mode};
static NeonRTOS_SyncObj_t I2C_Master_Done_SyncHandle[hwI2C_Index_MAX];

static TIMSPM0_I2C_Transfer i2c_xfer[hwI2C_Index_MAX];

static bool I2C_Map_Soc_Hw(
    hwI2C_Index index,
    TIMSPM0_I2C_Hw *hw)
{
    if (hw == NULL)
    {
        return false;
    }

    hw->type = TIMSPM0_I2C_HW_NONE;
    hw->base = NULL;
    hw->irqn = (IRQn_Type) 0;

    switch (index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            hw->type = TIMSPM0_I2C_HW_LEGACY;
            hw->base = (void *) I2C0;
            hw->irqn = I2C0_INT_IRQn;
            break;
#elif defined(UC0_I2CC_BASE)
        case hwI2C_Index_0:
            hw->type = TIMSPM0_I2C_HW_UNICOMM;
            hw->base = (void *) UC0;
            hw->irqn = UC0_INT_IRQn;
            break;
#endif

#if defined(I2C1_BASE)
        case hwI2C_Index_1:
            hw->type = TIMSPM0_I2C_HW_LEGACY;
            hw->base = (void *) I2C1;
            hw->irqn = I2C1_INT_IRQn;
            break;
#elif defined(UC1_I2CC_BASE)
        case hwI2C_Index_1:
            hw->type = TIMSPM0_I2C_HW_UNICOMM;
            hw->base = (void *) UC1;
            hw->irqn = UC1_INT_IRQn;
            break;
#endif

#if defined(I2C2_BASE)
        case hwI2C_Index_2:
            hw->type = TIMSPM0_I2C_HW_LEGACY;
            hw->base = (void *) I2C2;
            hw->irqn = I2C2_INT_IRQn;
            break;
#endif

#if defined(UC5_I2CC_BASE)
        case hwI2C_Index_5:
            hw->type = TIMSPM0_I2C_HW_UNICOMM;
            hw->base = (void *) UC5;
            hw->irqn = UC5_INT_IRQn;
            break;
#endif

#if defined(UC6_I2CC_BASE)
        case hwI2C_Index_6:
            hw->type = TIMSPM0_I2C_HW_UNICOMM;
            hw->base = (void *) UC6;
            hw->irqn = UC6_INT_IRQn;
            break;
#endif

        default:
            break;
    }

    return (hw->type != TIMSPM0_I2C_HW_NONE) &&
           (hw->base != NULL);
}

static void I2C_HwDisableInterrupt(
    const TIMSPM0_I2C_Hw *hw,
    uint32_t interrupts)
{
    uint32_t mask = 0;
    
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if ((interrupts & I2C_TIMSPM0_INT_RX_DONE) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_RX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TX_DONE) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_TX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_RXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_RXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_NACK) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_NACK;
    }
    if ((interrupts & I2C_TIMSPM0_INT_STOP) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_STOP;
    }
    if ((interrupts & I2C_TIMSPM0_INT_ARBITRATION_LOST) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_ARBITRATION_LOST;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if ((interrupts & I2C_TIMSPM0_INT_RX_DONE) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_RX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TX_DONE) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_TX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_RXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_RXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_TXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_NACK) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_NACK;
    }
    if ((interrupts & I2C_TIMSPM0_INT_STOP) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_STOP;
    }
    if ((interrupts & I2C_TIMSPM0_INT_ARBITRATION_LOST) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_ARBITRATION_LOST;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_disableInterrupt((I2C_Regs *) hw->base, mask);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_disableInterrupt(
            (UNICOMM_Inst_Regs *) hw->base,
            mask);
    }
#endif
}

static void I2C_HwClearInterruptStatus(
    const TIMSPM0_I2C_Hw *hw,
    uint32_t interrupts)
{
    uint32_t mask = 0;

#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if ((interrupts & I2C_TIMSPM0_INT_RX_DONE) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_RX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TX_DONE) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_TX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_RXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_RXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_NACK) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_NACK;
    }
    if ((interrupts & I2C_TIMSPM0_INT_STOP) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_STOP;
    }
    if ((interrupts & I2C_TIMSPM0_INT_ARBITRATION_LOST) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_ARBITRATION_LOST;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if ((interrupts & I2C_TIMSPM0_INT_RX_DONE) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_RX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TX_DONE) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_TX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_RXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_RXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_TXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_NACK) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_NACK;
    }
    if ((interrupts & I2C_TIMSPM0_INT_STOP) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_STOP;
    }
    if ((interrupts & I2C_TIMSPM0_INT_ARBITRATION_LOST) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_ARBITRATION_LOST;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_clearInterruptStatus((I2C_Regs *) hw->base, mask);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_clearInterruptStatus(
            (UNICOMM_Inst_Regs *) hw->base,
            mask);
    }
#endif
}

static void I2C_HwResetTransfer(
    const TIMSPM0_I2C_Hw *hw)
{
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_resetControllerTransfer((I2C_Regs *) hw->base);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_resetTransfer((UNICOMM_Inst_Regs *) hw->base);
    }
#endif
}

static void I2C_HwFlushTXFIFO(
    const TIMSPM0_I2C_Hw *hw)
{
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_flushControllerTXFIFO((I2C_Regs *) hw->base);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_flushTXFIFO((UNICOMM_Inst_Regs *) hw->base);
    }
#endif
}

static void I2C_HwFlushRXFIFO(
    const TIMSPM0_I2C_Hw *hw)
{
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_flushControllerRXFIFO((I2C_Regs *) hw->base);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_flushRXFIFO((UNICOMM_Inst_Regs *) hw->base);
    }
#endif
}

static bool I2C_HwIsRXFIFOEmpty(
    const TIMSPM0_I2C_Hw *hw)
{
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        return DL_I2C_isControllerRXFIFOEmpty(
            (I2C_Regs *) hw->base);
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        return DL_I2CC_isRXFIFOEmpty(
            (UNICOMM_Inst_Regs *) hw->base);
    }
#endif

    return true;
}

static uint8_t I2C_HwReceiveData(
    const TIMSPM0_I2C_Hw *hw)
{
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        return DL_I2C_receiveControllerData(
            (I2C_Regs *) hw->base);
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        return DL_I2CC_receiveData(
            (UNICOMM_Inst_Regs *) hw->base);
    }
#endif

    return 0U;
}

static uint16_t I2C_HwFillTXFIFO(
    const TIMSPM0_I2C_Hw *hw,
    const uint8_t *buffer,
    uint16_t count)
{
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        return DL_I2C_fillControllerTXFIFO(
            (I2C_Regs *) hw->base,
            buffer,
            count);
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        return DL_I2CC_fillTXFIFO(
            (UNICOMM_Inst_Regs *) hw->base,
            (uint8_t *) buffer,
            count);
    }
#endif

    return 0U;
}

static TIMSPM0_I2C_Event I2C_HwGetPendingEvent(
    const TIMSPM0_I2C_Hw *hw,
    uint32_t *raw_interrupt)
{
    if (raw_interrupt != NULL)
    {
        *raw_interrupt = 0U;
    }

#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_IIDX interrupt =
            DL_I2C_getPendingInterrupt((I2C_Regs *) hw->base);

        if (raw_interrupt != NULL)
        {
            *raw_interrupt = (uint32_t) interrupt;
        }

        switch (interrupt)
        {
            case DL_I2C_IIDX_NO_INT:
                return TIMSPM0_I2C_EVENT_NONE;

            case DL_I2C_IIDX_CONTROLLER_RX_DONE:
                return TIMSPM0_I2C_EVENT_RX_DONE;

            case DL_I2C_IIDX_CONTROLLER_TX_DONE:
                return TIMSPM0_I2C_EVENT_TX_DONE;

            case DL_I2C_IIDX_CONTROLLER_RXFIFO_TRIGGER:
            case DL_I2C_IIDX_CONTROLLER_RXFIFO_FULL:
                return TIMSPM0_I2C_EVENT_RXFIFO;

            case DL_I2C_IIDX_CONTROLLER_TXFIFO_TRIGGER:
                return TIMSPM0_I2C_EVENT_TXFIFO;

            case DL_I2C_IIDX_CONTROLLER_STOP:
                return TIMSPM0_I2C_EVENT_STOP;

            case DL_I2C_IIDX_CONTROLLER_NACK:
                return TIMSPM0_I2C_EVENT_NACK;

            case DL_I2C_IIDX_CONTROLLER_ARBITRATION_LOST:
                return TIMSPM0_I2C_EVENT_ARBITRATION_LOST;

            default:
                return TIMSPM0_I2C_EVENT_OTHER;
        }
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_IIDX interrupt =
            DL_I2CC_getPendingInterrupt(
                (UNICOMM_Inst_Regs *) hw->base);

        if (raw_interrupt != NULL)
        {
            *raw_interrupt = (uint32_t) interrupt;
        }

        switch (interrupt)
        {
            case DL_I2CC_IIDX_NO_INT:
                return TIMSPM0_I2C_EVENT_NONE;

            case DL_I2CC_IIDX_RX_DONE:
                return TIMSPM0_I2C_EVENT_RX_DONE;

            case DL_I2CC_IIDX_TX_DONE:
                return TIMSPM0_I2C_EVENT_TX_DONE;

            case DL_I2CC_IIDX_RXFIFO_TRIGGER:
            case DL_I2CC_IIDX_RXFIFO_FULL:
                return TIMSPM0_I2C_EVENT_RXFIFO;

            case DL_I2CC_IIDX_TXFIFO_TRIGGER:
                return TIMSPM0_I2C_EVENT_TXFIFO;

            case DL_I2CC_IIDX_STOP:
                return TIMSPM0_I2C_EVENT_STOP;

            case DL_I2CC_IIDX_NACK:
                return TIMSPM0_I2C_EVENT_NACK;

            case DL_I2CC_IIDX_ARBITRATION_LOST:
                return TIMSPM0_I2C_EVENT_ARBITRATION_LOST;

            default:
                return TIMSPM0_I2C_EVENT_OTHER;
        }
    }
#endif

    return TIMSPM0_I2C_EVENT_NONE;
}

static bool I2C_IsTransferActive(
    TIMSPM0_I2C_State state)
{
    return (state == TIMSPM0_I2C_TX) ||
           (state == TIMSPM0_I2C_TX_WAIT_STOP) ||
           (state == TIMSPM0_I2C_RX) ||
           (state == TIMSPM0_I2C_RX_WAIT_STOP);
}

static bool I2C_GetTimerPeriod(
    hwI2C_Speed_Mode speed_mode,
    uint8_t *period)
{
    uint32_t speed_hz;
    uint32_t denominator;
    uint32_t bus_clock_hz;
    uint32_t divisor;

    if (period == NULL)
    {
        return false;
    }

    switch (speed_mode)
    {
        case hwI2C_Standard_Mode:
            speed_hz = I2C_TIMSPM0_STANDARD_SPEED_HZ;
            break;

        case hwI2C_Fast_Mode:
            speed_hz = I2C_TIMSPM0_FAST_SPEED_HZ;
            break;

        default:
            return false;
    }

    denominator =
        speed_hz * I2C_TIMSPM0_TIMER_CLOCKS_PER_BIT;

    bus_clock_hz = (uint32_t) g_sys_clock_hz;

    if ((bus_clock_hz < denominator) ||
        (bus_clock_hz == 0U))
    {
        return false;
    }

    /*
     * SCL = BUSCLK / ((1 + TPR) * 10)
     *
     * Round upward so the generated SCL never exceeds the requested
     * bus speed.
     */
    divisor =
        (bus_clock_hz + denominator - 1U) /
        denominator;

    if ((divisor == 0U) ||
        (divisor > I2C_TIMSPM0_TIMER_DIVISOR_MAX))
    {
        return false;
    }

    *period = (uint8_t) (divisor - 1U);

    return true;
}

static bool I2C_EnableGPIOPort(
    GPIO_Regs *port)
{
    if (port == NULL)
    {
        return false;
    }

    if (!DL_GPIO_isPowerEnabled(port))
    {
        DL_GPIO_enablePower(port);
        DL_Common_delayCycles(
            I2C_TIMSPM0_POWER_STARTUP_DELAY);
    }

    return DL_GPIO_isPowerEnabled(port);
}

static bool I2C_ConfigurePins(
    hwI2C_Index index)
{
    const I2C_Pin_Def *pins;
    GPIO_Regs *scl_port;
    GPIO_Regs *sda_port;

    if (index >= hwI2C_Index_MAX)
    {
        return false;
    }

    pins = &I2C_Pin_Def_Table[index];

    if ((pins->scl_pin == hwGPIO_Pin_NC) ||
        (pins->sda_pin == hwGPIO_Pin_NC) ||
        (pins->scl_function == 0U) ||
        (pins->sda_function == 0U))
    {
        return false;
    }

    if ((GPIO_Map_Soc_Pin_IOMUX(pins->scl_pin) !=
            pins->scl_iomux) ||
        (GPIO_Map_Soc_Pin_IOMUX(pins->sda_pin) !=
            pins->sda_iomux))
    {
        return false;
    }

    scl_port = GPIO_Map_Soc_Base(pins->scl_pin);
    sda_port = GPIO_Map_Soc_Base(pins->sda_pin);

    if (!I2C_EnableGPIOPort(scl_port) ||
        !I2C_EnableGPIOPort(sda_port))
    {
        return false;
    }

    DL_GPIO_initPeripheralInputFunctionFeatures(
        pins->scl_iomux,
        pins->scl_function,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initPeripheralInputFunctionFeatures(
        pins->sda_iomux,
        pins->sda_function,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);

    /*
     * I2C and UNICOMM I2CC both use peripheral open-drain outputs.
     * External pull-up resistors are still required on SCL and SDA.
     */
    DL_GPIO_enableHiZ(pins->scl_iomux);
    DL_GPIO_enableHiZ(pins->sda_iomux);

    return true;
}

static void I2C_DeConfigurePins(
    hwI2C_Index index)
{
    const I2C_Pin_Def *pins;

    if (index >= hwI2C_Index_MAX)
    {
        return;
    }

    pins = &I2C_Pin_Def_Table[index];

    DL_GPIO_initDigitalInput(pins->scl_iomux);
    DL_GPIO_initDigitalInput(pins->sda_iomux);
}

static void I2C_NVIC_Init(
    const TIMSPM0_I2C_Hw *hw)
{
    NVIC_ClearPendingIRQ(hw->irqn);
    NVIC_EnableIRQ(hw->irqn);
}

static void I2C_NVIC_DeInit(
    const TIMSPM0_I2C_Hw *hw)
{
    NVIC_DisableIRQ(hw->irqn);
    NVIC_ClearPendingIRQ(hw->irqn);
}

static void I2C_DrainRXFIFO(
    const TIMSPM0_I2C_Hw *hw,
    TIMSPM0_I2C_Transfer *transfer)
{
    while (!I2C_HwIsRXFIFOEmpty(hw))
    {
        uint8_t value = I2C_HwReceiveData(hw);

        if ((transfer->rx_buf != NULL) &&
            (transfer->rx_pos < transfer->rx_len))
        {
            transfer->rx_buf[transfer->rx_pos++] = value;
        }
    }
}

static void I2C_CompleteFromISR(
    hwI2C_Index index,
    TIMSPM0_I2C_State state,
    uint32_t error)
{
    TIMSPM0_I2C_Hw hw;

    if (!I2C_Map_Soc_Hw(index, &hw))
    {
        return;
    }

    I2C_HwDisableInterrupt(
        &hw,
        I2C_TIMSPM0_ALL_INTERRUPTS);

    i2c_xfer[index].error = error;
    i2c_xfer[index].state = state;

    NeonRTOS_SyncObjSignalFromISR(
        &I2C_Master_Done_SyncHandle[index]);
}

static void I2C_IRQ_Process(
    hwI2C_Index index)
{
    TIMSPM0_I2C_Hw hw;
    TIMSPM0_I2C_Transfer *transfer;

    if ((index >= hwI2C_Index_MAX) ||
        !I2C_Map_Soc_Hw(index, &hw))
    {
        return;
    }

    transfer = &i2c_xfer[index];

    for (;;)
    {
        uint32_t raw_interrupt;
        TIMSPM0_I2C_Event event =
            I2C_HwGetPendingEvent(
                &hw,
                &raw_interrupt);

        switch (event)
        {
            case TIMSPM0_I2C_EVENT_NONE:
                return;

            case TIMSPM0_I2C_EVENT_RXFIFO:
                if ((transfer->state == TIMSPM0_I2C_RX) ||
                    (transfer->state ==
                        TIMSPM0_I2C_RX_WAIT_STOP))
                {
                    I2C_DrainRXFIFO(&hw, transfer);
                }
                break;

            case TIMSPM0_I2C_EVENT_TXFIFO:
                if (transfer->state == TIMSPM0_I2C_TX)
                {
                    if (transfer->tx_pos < transfer->tx_len)
                    {
                        transfer->tx_pos +=
                            I2C_HwFillTXFIFO(
                                &hw,
                                &transfer->tx_buf[
                                    transfer->tx_pos],
                                (uint16_t)
                                    (transfer->tx_len -
                                     transfer->tx_pos));
                    }

                    if (transfer->tx_pos >= transfer->tx_len)
                    {
                        I2C_HwDisableInterrupt(
                            &hw,
                            I2C_TIMSPM0_INT_TXFIFO_TRIGGER);
                    }
                }
                break;

            case TIMSPM0_I2C_EVENT_RX_DONE:
                if (transfer->state == TIMSPM0_I2C_RX)
                {
                    I2C_DrainRXFIFO(&hw, transfer);

                    if (transfer->rx_pos != transfer->rx_len)
                    {
                        I2C_HwResetTransfer(&hw);
                        I2C_CompleteFromISR(
                            index,
                            TIMSPM0_I2C_ERROR,
                            raw_interrupt);
                        return;
                    }

                    if (transfer->stop)
                    {
                        transfer->state =
                            TIMSPM0_I2C_RX_WAIT_STOP;
                    }
                    else
                    {
                        I2C_CompleteFromISR(
                            index,
                            TIMSPM0_I2C_DONE,
                            0U);
                        return;
                    }
                }
                break;

            case TIMSPM0_I2C_EVENT_TX_DONE:
                if (transfer->state == TIMSPM0_I2C_TX)
                {
                    I2C_HwDisableInterrupt(
                        &hw,
                        I2C_TIMSPM0_INT_TXFIFO_TRIGGER);

                    if (transfer->tx_pos != transfer->tx_len)
                    {
                        I2C_HwResetTransfer(&hw);
                        I2C_CompleteFromISR(
                            index,
                            TIMSPM0_I2C_ERROR,
                            raw_interrupt);
                        return;
                    }

                    if (transfer->stop)
                    {
                        transfer->state =
                            TIMSPM0_I2C_TX_WAIT_STOP;
                    }
                    else
                    {
                        I2C_CompleteFromISR(
                            index,
                            TIMSPM0_I2C_DONE,
                            0U);
                        return;
                    }
                }
                break;

            case TIMSPM0_I2C_EVENT_STOP:
                /*
                 * STOP and RX/TX_DONE can become pending together.
                 * Treat STOP as authoritative completion so the
                 * transfer cannot wait forever if IIDX reports STOP
                 * before the corresponding DONE event.
                 */
                if ((transfer->state == TIMSPM0_I2C_RX) ||
                    (transfer->state ==
                        TIMSPM0_I2C_RX_WAIT_STOP))
                {
                    I2C_DrainRXFIFO(&hw, transfer);

                    if (transfer->rx_pos != transfer->rx_len)
                    {
                        I2C_HwResetTransfer(&hw);
                        I2C_CompleteFromISR(
                            index,
                            TIMSPM0_I2C_ERROR,
                            raw_interrupt);
                        return;
                    }

                    I2C_CompleteFromISR(
                        index,
                        TIMSPM0_I2C_DONE,
                        0U);
                    return;
                }

                if ((transfer->state == TIMSPM0_I2C_TX) ||
                    (transfer->state ==
                        TIMSPM0_I2C_TX_WAIT_STOP))
                {
                    if (transfer->tx_pos != transfer->tx_len)
                    {
                        I2C_HwResetTransfer(&hw);
                        I2C_CompleteFromISR(
                            index,
                            TIMSPM0_I2C_ERROR,
                            raw_interrupt);
                        return;
                    }

                    I2C_CompleteFromISR(
                        index,
                        TIMSPM0_I2C_DONE,
                        0U);
                    return;
                }
                break;

            case TIMSPM0_I2C_EVENT_NACK:
            case TIMSPM0_I2C_EVENT_ARBITRATION_LOST:
                if (I2C_IsTransferActive(transfer->state))
                {
                    I2C_HwResetTransfer(&hw);
                    I2C_CompleteFromISR(
                        index,
                        TIMSPM0_I2C_ERROR,
                        raw_interrupt);
                    return;
                }
                break;

            case TIMSPM0_I2C_EVENT_OTHER:
            default:
                break;
        }
    }
}

#if defined(I2C0_BASE)
void I2C0_IRQHandler(void)
{
    I2C_IRQ_Process(hwI2C_Index_0);
}
#endif

#if defined(I2C1_BASE)
void I2C1_IRQHandler(void)
{
    I2C_IRQ_Process(hwI2C_Index_1);
}
#endif

#if defined(I2C2_BASE)
void I2C2_IRQHandler(void)
{
    I2C_IRQ_Process(hwI2C_Index_2);
}
#endif

#if defined(UC0_I2CC_BASE) && !defined(I2C0_BASE)
void UC0_IRQHandler(void)
{
    I2C_IRQ_Process(hwI2C_Index_0);
}
#endif

#if defined(UC1_I2CC_BASE) && !defined(I2C1_BASE)
void UC1_IRQHandler(void)
{
    I2C_IRQ_Process(hwI2C_Index_1);
}
#endif

#if defined(UC5_I2CC_BASE)
void UC5_IRQHandler(void)
{
    I2C_IRQ_Process(hwI2C_Index_5);
}
#endif

#if defined(UC6_I2CC_BASE)
void UC6_IRQHandler(void)
{
    I2C_IRQ_Process(hwI2C_Index_6);
}
#endif

static bool I2C_HwInit(
    const TIMSPM0_I2C_Hw *hw,
    uint8_t timer_period)
{
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        I2C_Regs *base = (I2C_Regs *) hw->base;
        const DL_I2C_ClockConfig clock_config =
        {
            .clockSel = DL_I2C_CLOCK_BUSCLK,
            .divideRatio = DL_I2C_CLOCK_DIVIDE_1
        };

        DL_I2C_reset(base);
        DL_I2C_enablePower(base);
        DL_Common_delayCycles(
            I2C_TIMSPM0_POWER_STARTUP_DELAY);

        DL_I2C_setClockConfig(base, &clock_config);
        DL_I2C_disableAnalogGlitchFilter(base);
        DL_I2C_resetControllerTransfer(base);
        DL_I2C_setControllerAddressingMode(
            base,
            DL_I2C_CONTROLLER_ADDRESSING_MODE_7_BIT);
        DL_I2C_setTimerPeriod(base, timer_period);
        DL_I2C_setControllerTXFIFOThreshold(
            base,
            DL_I2C_TX_FIFO_LEVEL_BYTES_1);
        DL_I2C_setControllerRXFIFOThreshold(
            base,
            DL_I2C_RX_FIFO_LEVEL_BYTES_1);
        DL_I2C_enableControllerClockStretching(base);

        I2C_HwDisableInterrupt(
            hw,
            I2C_TIMSPM0_ALL_INTERRUPTS);
        I2C_HwClearInterruptStatus(
            hw,
            I2C_TIMSPM0_ALL_INTERRUPTS);

        DL_I2C_enableController(base);

        return true;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        UNICOMM_Inst_Regs *base =
            (UNICOMM_Inst_Regs *) hw->base;
        DL_I2CC_ClockConfig clock_config =
        {
            .clockSel = DL_I2CC_CLOCK_BUSCLK,
            .divideRatio = DL_I2CC_CLOCK_DIVIDE_1
        };

        DL_I2CC_reset(base);
        DL_I2CC_enablePower(base);
        DL_Common_delayCycles(
            I2C_TIMSPM0_POWER_STARTUP_DELAY);

        /*
         * DL_I2CC_enablePower() also selects UNICOMM I2C controller
         * mode for a non-fixed-mode UC instance.
         */
        DL_I2CC_setClockConfig(base, &clock_config);
        DL_I2CC_disableAnalogGlitchFilter(base);
        DL_I2CC_resetTransfer(base);
        DL_I2CC_setAddressingMode(
            base,
            DL_I2CC_ADDRESSING_MODE_7_BIT);
        DL_I2CC_setTimerPeriod(base, timer_period);
        DL_I2CC_setTXFIFOThreshold(
            base,
            DL_I2CC_TX_FIFO_LEVEL_ONE_ENTRY);
        DL_I2CC_setRXFIFOThreshold(
            base,
            DL_I2CC_RX_FIFO_LEVEL_ONE_ENTRY);
        DL_I2CC_enableClockStretching(base);

        I2C_HwDisableInterrupt(
            hw,
            I2C_TIMSPM0_ALL_INTERRUPTS);
        I2C_HwClearInterruptStatus(
            hw,
            I2C_TIMSPM0_ALL_INTERRUPTS);

        DL_I2CC_enable(base);

        return true;
    }
#endif

    return false;
}

static void I2C_HwDeInit(
    const TIMSPM0_I2C_Hw *hw)
{
    I2C_HwDisableInterrupt(
        hw,
        I2C_TIMSPM0_ALL_INTERRUPTS);
    I2C_HwResetTransfer(hw);
    I2C_HwFlushTXFIFO(hw);
    I2C_HwFlushRXFIFO(hw);
    I2C_HwClearInterruptStatus(
        hw,
        I2C_TIMSPM0_ALL_INTERRUPTS);

#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        I2C_Regs *base = (I2C_Regs *) hw->base;

        DL_I2C_disableController(base);
        DL_I2C_reset(base);
        DL_I2C_disablePower(base);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        UNICOMM_Inst_Regs *base =
            (UNICOMM_Inst_Regs *) hw->base;

        DL_I2CC_disable(base);
        DL_I2CC_reset(base);
        DL_I2CC_disablePower(base);
    }
#endif
}

static void I2C_HwPrepareTransfer(
    const TIMSPM0_I2C_Hw *hw)
{
    I2C_HwDisableInterrupt(
        hw,
        I2C_TIMSPM0_ALL_INTERRUPTS);
    I2C_HwClearInterruptStatus(
        hw,
        I2C_TIMSPM0_ALL_INTERRUPTS);
    I2C_HwFlushTXFIFO(hw);
    I2C_HwFlushRXFIFO(hw);
}

static void I2C_HwStartRead(
    const TIMSPM0_I2C_Hw *hw,
    uint8_t address,
    uint16_t length,
    bool stop)
{
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_startControllerTransferAdvanced(
            (I2C_Regs *) hw->base,
            address,
            DL_I2C_CONTROLLER_DIRECTION_RX,
            length,
            DL_I2C_CONTROLLER_START_ENABLE,
            stop ?
                DL_I2C_CONTROLLER_STOP_ENABLE :
                DL_I2C_CONTROLLER_STOP_DISABLE,
            DL_I2C_CONTROLLER_ACK_DISABLE);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_startTransferAdvanced(
            (UNICOMM_Inst_Regs *) hw->base,
            address,
            DL_I2CC_DIRECTION_RX,
            length,
            DL_I2CC_START_ENABLE,
            stop ?
                DL_I2CC_STOP_ENABLE :
                DL_I2CC_STOP_DISABLE,
            DL_I2CC_ACK_DISABLE);
    }
#endif
}

static void I2C_HwStartWrite(
    const TIMSPM0_I2C_Hw *hw,
    uint8_t address,
    uint16_t length,
    bool stop)
{
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_startControllerTransferAdvanced(
            (I2C_Regs *) hw->base,
            address,
            DL_I2C_CONTROLLER_DIRECTION_TX,
            length,
            DL_I2C_CONTROLLER_START_ENABLE,
            stop ?
                DL_I2C_CONTROLLER_STOP_ENABLE :
                DL_I2C_CONTROLLER_STOP_DISABLE,
            DL_I2C_CONTROLLER_ACK_DISABLE);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_startTransferAdvanced(
            (UNICOMM_Inst_Regs *) hw->base,
            address,
            DL_I2CC_DIRECTION_TX,
            length,
            DL_I2CC_START_ENABLE,
            stop ?
                DL_I2CC_STOP_ENABLE :
                DL_I2CC_STOP_DISABLE,
            DL_I2CC_ACK_DISABLE);
    }
#endif
}

static hwI2C_OpResult I2C_WaitTransferDone(
    hwI2C_Index index,
    NeonRTOS_Time_t timeout_ms)
{
    TIMSPM0_I2C_Transfer *transfer =
        &i2c_xfer[index];

    if (NeonRTOS_SyncObjWait(
            &I2C_Master_Done_SyncHandle[index],
            timeout_ms) == NeonRTOS_OK)
    {
        return (transfer->state == TIMSPM0_I2C_DONE) ?
            hwI2C_OK :
            hwI2C_BusError;
    }

    /*
     * Cover a completion racing with the RTOS timeout return.
     */
    if (transfer->state == TIMSPM0_I2C_DONE)
    {
        return hwI2C_OK;
    }

    if (transfer->state == TIMSPM0_I2C_ERROR)
    {
        return hwI2C_BusError;
    }

    if (I2C_IsTransferActive(transfer->state))
    {
        TIMSPM0_I2C_Hw hw;

        if (I2C_Map_Soc_Hw(index, &hw))
        {
            I2C_HwDisableInterrupt(
                &hw,
                I2C_TIMSPM0_ALL_INTERRUPTS);
            I2C_HwResetTransfer(&hw);
            I2C_HwFlushTXFIFO(&hw);
            I2C_HwFlushRXFIFO(&hw);
            I2C_HwClearInterruptStatus(
                &hw,
                I2C_TIMSPM0_ALL_INTERRUPTS);
        }

        transfer->error = UINT32_MAX;
        transfer->state = TIMSPM0_I2C_ERROR;
    }

    return hwI2C_SlaveTimeout;
}

hwI2C_OpResult I2C_Master_Init(
    hwI2C_Index index,
    hwI2C_Speed_Mode speed_mode)
{
    TIMSPM0_I2C_Hw hw;
    uint8_t timer_period;
    const I2C_Pin_Def *pins;

    if ((index >= hwI2C_Index_MAX) ||
        (speed_mode >= hwI2C_Speed_Mode_MAX))
    {
        return hwI2C_InvalidParameter;
    }

    if (I2C_Master_Init_Status[index])
    {
        return hwI2C_OK;
    }

    if (speed_mode == hwI2C_High_Speed_Mode)
    {
        return hwI2C_Unsupport;
    }

    if (!I2C_Map_Soc_Hw(index, &hw) ||
        !I2C_GetTimerPeriod(
            speed_mode,
            &timer_period) ||
        !I2C_ConfigurePins(index))
    {
        return hwI2C_InvalidParameter;
    }

    if (NeonRTOS_SyncObjCreate(
            &I2C_Master_Done_SyncHandle[index]) !=
        NeonRTOS_OK)
    {
        I2C_DeConfigurePins(index);
        return hwI2C_MemoryError;
    }

    if (!I2C_HwInit(&hw, timer_period))
    {
        NeonRTOS_SyncObjDelete(
            &I2C_Master_Done_SyncHandle[index]);
        I2C_DeConfigurePins(index);
        return hwI2C_Unsupport;
    }

    memset(
        &i2c_xfer[index],
        0,
        sizeof(i2c_xfer[index]));
    i2c_xfer[index].state = TIMSPM0_I2C_IDLE;

    I2C_NVIC_Init(&hw);

    pins = &I2C_Pin_Def_Table[index];
    gpio_pin_init_status[pins->scl_pin] = true;
    gpio_pin_init_status[pins->sda_pin] = true;

    I2C_Clock_Speed_Mode[index] = speed_mode;
    I2C_Master_Init_Status[index] = true;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_DeInit(
    hwI2C_Index index)
{
    TIMSPM0_I2C_Hw hw;
    const I2C_Pin_Def *pins;

    if (index >= hwI2C_Index_MAX)
    {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index])
    {
        return hwI2C_OK;
    }

    if (!I2C_Map_Soc_Hw(index, &hw))
    {
        return hwI2C_InvalidParameter;
    }

    pins = &I2C_Pin_Def_Table[index];
    I2C_Master_Init_Status[index] = false;

    I2C_NVIC_DeInit(&hw);
    I2C_HwDeInit(&hw);

    NeonRTOS_SyncObjDelete(
        &I2C_Master_Done_SyncHandle[index]);

    I2C_DeConfigurePins(index);

    gpio_pin_init_status[pins->scl_pin] = false;
    gpio_pin_init_status[pins->sda_pin] = false;

    memset(
        &i2c_xfer[index],
        0,
        sizeof(i2c_xfer[index]));
    i2c_xfer[index].state = TIMSPM0_I2C_IDLE;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_Reset(
    hwI2C_Index index)
{
    hwI2C_Speed_Mode speed;
    hwI2C_OpResult result;

    if (index >= hwI2C_Index_MAX)
    {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index])
    {
        return hwI2C_NotInit;
    }

    speed = I2C_Clock_Speed_Mode[index];
    result = I2C_Master_DeInit(index);

    if (result != hwI2C_OK)
    {
        return result;
    }

    return I2C_Master_Init(index, speed);
}

hwI2C_OpResult I2C_Master_Read(
    hwI2C_Index index,
    uint8_t address,
    uint8_t *read_dat,
    uint8_t read_len,
    bool stop,
    NeonRTOS_Time_t timeout_ms)
{
    TIMSPM0_I2C_Hw hw;
    TIMSPM0_I2C_Transfer *transfer;
    uint32_t interrupts;

    if ((index >= hwI2C_Index_MAX) ||
        (address > I2C_TIMSPM0_MAX_7BIT_ADDRESS) ||
        (read_dat == NULL) ||
        (read_len == 0U))
    {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index])
    {
        return hwI2C_NotInit;
    }

    if (!I2C_Map_Soc_Hw(index, &hw))
    {
        return hwI2C_InvalidParameter;
    }

    transfer = &i2c_xfer[index];

    if (I2C_IsTransferActive(transfer->state))
    {
        return hwI2C_BusError;
    }

    NeonRTOS_SyncObjWait(
        &I2C_Master_Done_SyncHandle[index],
        NEONRT_NO_WAIT);

    I2C_HwPrepareTransfer(&hw);

    memset(transfer, 0, sizeof(*transfer));
    transfer->state = TIMSPM0_I2C_RX;
    transfer->address = address;
    transfer->rx_buf = read_dat;
    transfer->rx_len = read_len;
    transfer->stop = stop;

    interrupts =
        I2C_TIMSPM0_INT_RXFIFO_TRIGGER |
        I2C_TIMSPM0_INT_RX_DONE |
        I2C_TIMSPM0_INT_NACK |
        I2C_TIMSPM0_INT_ARBITRATION_LOST;

    if (stop)
    {
        interrupts |= I2C_TIMSPM0_INT_STOP;
    }

    uint32_t mask = 0;
    
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if ((interrupts & I2C_TIMSPM0_INT_RX_DONE) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_RX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TX_DONE) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_TX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_RXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_RXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_NACK) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_NACK;
    }
    if ((interrupts & I2C_TIMSPM0_INT_STOP) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_STOP;
    }
    if ((interrupts & I2C_TIMSPM0_INT_ARBITRATION_LOST) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_ARBITRATION_LOST;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if ((interrupts & I2C_TIMSPM0_INT_RX_DONE) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_RX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TX_DONE) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_TX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_RXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_RXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_TXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_NACK) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_NACK;
    }
    if ((interrupts & I2C_TIMSPM0_INT_STOP) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_STOP;
    }
    if ((interrupts & I2C_TIMSPM0_INT_ARBITRATION_LOST) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_ARBITRATION_LOST;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_enableInterrupt((I2C_Regs *) hw->base, mask);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_enableInterrupt(
            (UNICOMM_Inst_Regs *) hw->base,
            mask);
    }
#endif
    I2C_HwStartRead(
        &hw,
        address,
        read_len,
        stop);

    return I2C_WaitTransferDone(
        index,
        timeout_ms);
}

hwI2C_OpResult I2C_Master_Write(
    hwI2C_Index index,
    uint8_t address,
    uint8_t *write_dat,
    uint8_t write_len,
    bool stop,
    NeonRTOS_Time_t timeout_ms)
{
    TIMSPM0_I2C_Hw hw;
    TIMSPM0_I2C_Transfer *transfer;
    uint32_t interrupts;

    if ((index >= hwI2C_Index_MAX) ||
        (address > I2C_TIMSPM0_MAX_7BIT_ADDRESS) ||
        (write_dat == NULL) ||
        (write_len == 0U))
    {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index])
    {
        return hwI2C_NotInit;
    }

    if (!I2C_Map_Soc_Hw(index, &hw))
    {
        return hwI2C_InvalidParameter;
    }

    transfer = &i2c_xfer[index];

    if (I2C_IsTransferActive(transfer->state))
    {
        return hwI2C_BusError;
    }

    NeonRTOS_SyncObjWait(
        &I2C_Master_Done_SyncHandle[index],
        NEONRT_NO_WAIT);

    I2C_HwPrepareTransfer(&hw);

    memset(transfer, 0, sizeof(*transfer));
    transfer->state = TIMSPM0_I2C_TX;
    transfer->address = address;
    transfer->tx_buf = write_dat;
    transfer->tx_len = write_len;
    transfer->stop = stop;

    transfer->tx_pos =
        I2C_HwFillTXFIFO(
            &hw,
            write_dat,
            write_len);

    interrupts =
        I2C_TIMSPM0_INT_TX_DONE |
        I2C_TIMSPM0_INT_NACK |
        I2C_TIMSPM0_INT_ARBITRATION_LOST;

    if (transfer->tx_pos < transfer->tx_len)
    {
        interrupts |= I2C_TIMSPM0_INT_TXFIFO_TRIGGER;
    }

    if (stop)
    {
        interrupts |= I2C_TIMSPM0_INT_STOP;
    }

    uint32_t mask = 0;
    
#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if ((interrupts & I2C_TIMSPM0_INT_RX_DONE) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_RX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TX_DONE) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_TX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_RXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_RXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_NACK) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_NACK;
    }
    if ((interrupts & I2C_TIMSPM0_INT_STOP) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_STOP;
    }
    if ((interrupts & I2C_TIMSPM0_INT_ARBITRATION_LOST) != 0U)
    {
        mask |= DL_I2C_INTERRUPT_CONTROLLER_ARBITRATION_LOST;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if ((interrupts & I2C_TIMSPM0_INT_RX_DONE) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_RX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TX_DONE) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_TX_DONE;
    }
    if ((interrupts & I2C_TIMSPM0_INT_RXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_RXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_TXFIFO_TRIGGER) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_TXFIFO_TRIGGER;
    }
    if ((interrupts & I2C_TIMSPM0_INT_NACK) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_NACK;
    }
    if ((interrupts & I2C_TIMSPM0_INT_STOP) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_STOP;
    }
    if ((interrupts & I2C_TIMSPM0_INT_ARBITRATION_LOST) != 0U)
    {
        mask |= DL_I2CC_INTERRUPT_ARBITRATION_LOST;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_LEGACY_I2C)
    if (hw->type == TIMSPM0_I2C_HW_LEGACY)
    {
        DL_I2C_enableInterrupt((I2C_Regs *) hw->base, mask);
        return;
    }
#endif

#if defined(I2C_TIMSPM0_HAS_UNICOMM_I2CC)
    if (hw->type == TIMSPM0_I2C_HW_UNICOMM)
    {
        DL_I2CC_enableInterrupt(
            (UNICOMM_Inst_Regs *) hw->base,
            mask);
    }
#endif
    I2C_HwStartWrite(
        &hw,
        address,
        write_len,
        stop);

    return I2C_WaitTransferDone(
        index,
        timeout_ms);
}

bool I2C_Master_isInit(
    hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX)
    {
        return false;
    }

    return I2C_Master_Init_Status[index];
}

#endif /* DEVICE_TIMSPM0 */