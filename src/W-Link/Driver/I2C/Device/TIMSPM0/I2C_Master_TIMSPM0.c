#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#ifdef DEVICE_TIMSPM0

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"

#include "I2C/Pin/TIMSPM0/I2C_Pin_TIMSPM0.h"

#ifndef I2C_TIMSPM0_POWER_STARTUP_DELAY
#define I2C_TIMSPM0_POWER_STARTUP_DELAY       (16U)
#endif

#define I2C_TIMSPM0_STANDARD_SPEED_HZ          (100000UL)
#define I2C_TIMSPM0_FAST_SPEED_HZ              (400000UL)
#define I2C_TIMSPM0_MAX_7BIT_ADDRESS           (0x7FU)
#define I2C_TIMSPM0_TIMER_CLOCKS_PER_BIT       (10UL)
#define I2C_TIMSPM0_TIMER_DIVISOR_MAX          (128UL)

#define I2C_TIMSPM0_INTERRUPT_MASK                                      \
    (DL_I2C_INTERRUPT_CONTROLLER_RX_DONE |                              \
     DL_I2C_INTERRUPT_CONTROLLER_TX_DONE |                              \
     DL_I2C_INTERRUPT_CONTROLLER_RXFIFO_TRIGGER |                       \
     DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER |                       \
     DL_I2C_INTERRUPT_CONTROLLER_NACK |                                 \
     DL_I2C_INTERRUPT_CONTROLLER_STOP |                                 \
     DL_I2C_INTERRUPT_CONTROLLER_ARBITRATION_LOST)

typedef enum {
    TIMSPM0_I2C_IDLE = 0,
    TIMSPM0_I2C_TX,
    TIMSPM0_I2C_TX_WAIT_STOP,
    TIMSPM0_I2C_RX,
    TIMSPM0_I2C_RX_WAIT_STOP,
    TIMSPM0_I2C_DONE,
    TIMSPM0_I2C_ERROR
} TIMSPM0_I2C_State;

typedef struct {
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

static hwI2C_Speed_Mode
I2C_Clock_Speed_Mode[hwI2C_Index_MAX] = {
    hwI2C_Standard_Mode
};

static NeonRTOS_SyncObj_t
I2C_Master_Done_SyncHandle[hwI2C_Index_MAX];

static TIMSPM0_I2C_Transfer
i2c_xfer[hwI2C_Index_MAX];

static I2C_Regs *I2C_Map_Soc_Base(hwI2C_Index index)
{
    switch (index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            return I2C0;
#endif

#if defined(I2C1_BASE)
        case hwI2C_Index_1:
            return I2C1;
#endif

#if defined(I2C2_BASE)
        case hwI2C_Index_2:
            return I2C2;
#endif

        default:
            return NULL;
    }
}

static bool I2C_Map_Soc_IRQ(
    hwI2C_Index index,
    IRQn_Type *irq)
{
    if (irq == NULL)
    {
        return false;
    }

    switch (index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            *irq = I2C0_INT_IRQn;
            return true;
#endif

#if defined(I2C1_BASE)
        case hwI2C_Index_1:
            *irq = I2C1_INT_IRQn;
            return true;
#endif

#if defined(I2C2_BASE)
        case hwI2C_Index_2:
            *irq = I2C2_INT_IRQn;
            return true;
#endif

        default:
            return false;
    }
}

static uint32_t I2C_Map_Soc_Pin_Function(
    hwI2C_Index index,
    bool scl)
{
    switch (index)
    {
#if defined(I2C0_BASE)
        case hwI2C_Index_0:
            return scl ?
                IOMUX_PINCM2_PF_I2C0_SCL :
                IOMUX_PINCM1_PF_I2C0_SDA;
#endif

#if defined(I2C1_BASE)
        case hwI2C_Index_1:
#if defined(MSPM0L130x) || defined(MSPM0L134x)
            return scl ?
                IOMUX_PINCM5_PF_I2C1_SCL :
                IOMUX_PINCM4_PF_I2C1_SDA;
#elif defined(MSPM0C1105) || defined(MSPM0C1106) || \
      defined(MSPM0H321x)
            return scl ?
                IOMUX_PINCM11_PF_I2C1_SCL :
                IOMUX_PINCM12_PF_I2C1_SDA;
#else
            return scl ?
                IOMUX_PINCM15_PF_I2C1_SCL :
                IOMUX_PINCM16_PF_I2C1_SDA;
#endif
#endif

#if defined(I2C2_BASE)
        case hwI2C_Index_2:
#if defined(MSPM0L122x) || defined(MSPM0L222x)
            return scl ?
                IOMUX_PINCM27_PF_I2C2_SCL :
                IOMUX_PINCM28_PF_I2C2_SDA;
#else
            return scl ?
                IOMUX_PINCM23_PF_I2C2_SCL :
                IOMUX_PINCM24_PF_I2C2_SDA;
#endif
#endif

        default:
            return 0U;
    }
}

static bool I2C_IsTransferActive(TIMSPM0_I2C_State state)
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

    uint32_t denominator = speed_hz * I2C_TIMSPM0_TIMER_CLOCKS_PER_BIT;
    uint32_t bus_clock_hz = (uint32_t) F_CPU;

    if ((bus_clock_hz < denominator) || (bus_clock_hz == 0U))
    {
        return false;
    }

    /*
     * SCL = BUSCLK / ((1 + TPR) * 10)
     *
     * Round the divisor upward so SCL never exceeds the requested speed.
     */
    uint32_t divisor =
        (bus_clock_hz + denominator - 1U) / denominator;

    if ((divisor == 0U) ||
        (divisor > I2C_TIMSPM0_TIMER_DIVISOR_MAX))
    {
        return false;
    }

    *period = (uint8_t) (divisor - 1U);
    return true;
}

static hwI2C_OpResult I2C_NVIC_Init(hwI2C_Index index)
{
    IRQn_Type irq;

    if (!I2C_Map_Soc_IRQ(index, &irq))
    {
        return hwI2C_InvalidParameter;
    }

    NVIC_ClearPendingIRQ(irq);
    NVIC_EnableIRQ(irq);

    return hwI2C_OK;
}

static void I2C_NVIC_DeInit(hwI2C_Index index)
{
    IRQn_Type irq;

    if (!I2C_Map_Soc_IRQ(index, &irq))
    {
        return;
    }

    NVIC_DisableIRQ(irq);
    NVIC_ClearPendingIRQ(irq);
}

static hwI2C_OpResult I2C_ConfigPins(
    hwI2C_Index index,
    uint32_t *scl_iomux,
    uint32_t *sda_iomux)
{
    if ((scl_iomux == NULL) || (sda_iomux == NULL))
    {
        return hwI2C_InvalidParameter;
    }

    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index].scl_pin;
    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index].sda_pin;

    uint32_t scl_cm = GPIO_Map_Soc_Pin_IOMUX(scl_pin);
    uint32_t sda_cm = GPIO_Map_Soc_Pin_IOMUX(sda_pin);
    uint32_t scl_function =
        I2C_Map_Soc_Pin_Function(index, true);
    uint32_t sda_function =
        I2C_Map_Soc_Pin_Function(index, false);

    if ((scl_pin == hwGPIO_Pin_NC) ||
        (sda_pin == hwGPIO_Pin_NC) ||
        (scl_cm == GPIO_SOC_IOMUX_INVALID) ||
        (sda_cm == GPIO_SOC_IOMUX_INVALID) ||
        (scl_function == 0U) ||
        (sda_function == 0U))
    {
        return hwI2C_InvalidParameter;
    }

    DL_GPIO_initPeripheralInputFunctionFeatures(
        scl_cm,
        scl_function,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initPeripheralInputFunctionFeatures(
        sda_cm,
        sda_function,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);

    /*
     * MSPM0 I2C uses the peripheral open-drain output. External pull-up
     * resistors are still required on SCL and SDA.
     */
    DL_GPIO_enableHiZ(scl_cm);
    DL_GPIO_enableHiZ(sda_cm);

    *scl_iomux = scl_cm;
    *sda_iomux = sda_cm;

    return hwI2C_OK;
}

static void I2C_DeConfigPins(hwI2C_Index index)
{
    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index].scl_pin;
    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index].sda_pin;
    uint32_t scl_cm = GPIO_Map_Soc_Pin_IOMUX(scl_pin);
    uint32_t sda_cm = GPIO_Map_Soc_Pin_IOMUX(sda_pin);

    if (scl_cm != GPIO_SOC_IOMUX_INVALID)
    {
        DL_GPIO_initDigitalInput(scl_cm);
    }

    if (sda_cm != GPIO_SOC_IOMUX_INVALID)
    {
        DL_GPIO_initDigitalInput(sda_cm);
    }
}

static void I2C_DrainRXFIFO(
    I2C_Regs *i2c,
    TIMSPM0_I2C_Transfer *transfer)
{
    while (!DL_I2C_isControllerRXFIFOEmpty(i2c))
    {
        uint8_t value = DL_I2C_receiveControllerData(i2c);

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
    I2C_Regs *i2c = I2C_Map_Soc_Base(index);

    if (i2c == NULL)
    {
        return;
    }

    DL_I2C_disableInterrupt(
        i2c, I2C_TIMSPM0_INTERRUPT_MASK);

    i2c_xfer[index].error = error;
    i2c_xfer[index].state = state;

    NeonRTOS_SyncObjSignalFromISR(
        &I2C_Master_Done_SyncHandle[index]);
}

static void I2C_IRQ_Process(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX)
    {
        return;
    }

    I2C_Regs *i2c = I2C_Map_Soc_Base(index);

    if (i2c == NULL)
    {
        return;
    }

    TIMSPM0_I2C_Transfer *transfer =
        &i2c_xfer[index];

    for (;;)
    {
        DL_I2C_IIDX interrupt =
            DL_I2C_getPendingInterrupt(i2c);

        switch (interrupt)
        {
            case DL_I2C_IIDX_NO_INT:
                return;

            case DL_I2C_IIDX_CONTROLLER_RXFIFO_TRIGGER:
            case DL_I2C_IIDX_CONTROLLER_RXFIFO_FULL:
                if ((transfer->state == TIMSPM0_I2C_RX) ||
                    (transfer->state == TIMSPM0_I2C_RX_WAIT_STOP))
                {
                    I2C_DrainRXFIFO(i2c, transfer);
                }
                break;

            case DL_I2C_IIDX_CONTROLLER_TXFIFO_TRIGGER:
                if (transfer->state == TIMSPM0_I2C_TX)
                {
                    if (transfer->tx_pos < transfer->tx_len)
                    {
                        transfer->tx_pos +=
                            DL_I2C_fillControllerTXFIFO(
                                i2c,
                                &transfer->tx_buf[transfer->tx_pos],
                                (uint16_t)
                                    (transfer->tx_len -
                                     transfer->tx_pos));
                    }

                    if (transfer->tx_pos >= transfer->tx_len)
                    {
                        DL_I2C_disableInterrupt(
                            i2c,
                            DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
                    }
                }
                break;

            case DL_I2C_IIDX_CONTROLLER_RX_DONE:
                if (transfer->state == TIMSPM0_I2C_RX)
                {
                    I2C_DrainRXFIFO(i2c, transfer);

                    if (transfer->rx_pos != transfer->rx_len)
                    {
                        DL_I2C_resetControllerTransfer(i2c);
                        I2C_CompleteFromISR(
                            index,
                            TIMSPM0_I2C_ERROR,
                            (uint32_t)
                                DL_I2C_IIDX_CONTROLLER_RX_DONE);
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

            case DL_I2C_IIDX_CONTROLLER_TX_DONE:
                if (transfer->state == TIMSPM0_I2C_TX)
                {
                    DL_I2C_disableInterrupt(
                        i2c,
                        DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);

                    if (transfer->tx_pos != transfer->tx_len)
                    {
                        DL_I2C_resetControllerTransfer(i2c);
                        I2C_CompleteFromISR(
                            index,
                            TIMSPM0_I2C_ERROR,
                            (uint32_t)
                                DL_I2C_IIDX_CONTROLLER_TX_DONE);
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

            case DL_I2C_IIDX_CONTROLLER_STOP:
                if ((transfer->state ==
                        TIMSPM0_I2C_TX_WAIT_STOP) ||
                    (transfer->state ==
                        TIMSPM0_I2C_RX_WAIT_STOP))
                {
                    I2C_CompleteFromISR(
                        index,
                        TIMSPM0_I2C_DONE,
                        0U);
                    return;
                }
                break;

            case DL_I2C_IIDX_CONTROLLER_NACK:
            case DL_I2C_IIDX_CONTROLLER_ARBITRATION_LOST:
                if (I2C_IsTransferActive(transfer->state))
                {
                    DL_I2C_resetControllerTransfer(i2c);
                    I2C_CompleteFromISR(
                        index,
                        TIMSPM0_I2C_ERROR,
                        (uint32_t) interrupt);
                    return;
                }
                break;

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

static void I2C_PrepareTransfer(
    hwI2C_Index index,
    I2C_Regs *i2c)
{
    /*
     * Consume a possible stale completion token before starting a new
     * transaction.
     */
    (void) NeonRTOS_SyncObjWait(
        &I2C_Master_Done_SyncHandle[index],
        NEONRT_NO_WAIT);

    DL_I2C_disableInterrupt(
        i2c, I2C_TIMSPM0_INTERRUPT_MASK);
    DL_I2C_clearInterruptStatus(
        i2c, I2C_TIMSPM0_INTERRUPT_MASK);
    DL_I2C_flushControllerTXFIFO(i2c);
    DL_I2C_flushControllerRXFIFO(i2c);
}

static hwI2C_OpResult I2C_WaitTransfer(
    hwI2C_Index index,
    NeonRTOS_Time_t timeout_ms)
{
    if (NeonRTOS_SyncObjWait(
            &I2C_Master_Done_SyncHandle[index],
            timeout_ms) == NeonRTOS_OK)
    {
        return (i2c_xfer[index].state == TIMSPM0_I2C_DONE) ?
            hwI2C_OK :
            hwI2C_BusError;
    }

    /*
     * Close the IRQ race around a timeout. If the IRQ completed just after
     * the RTOS wait expired, consume its token and return its actual result.
     */
    I2C_NVIC_DeInit(index);

    (void) NeonRTOS_SyncObjWait(
        &I2C_Master_Done_SyncHandle[index],
        NEONRT_NO_WAIT);

    TIMSPM0_I2C_State state =
        i2c_xfer[index].state;

    if (I2C_IsTransferActive(state))
    {
        I2C_Regs *i2c = I2C_Map_Soc_Base(index);

        if (i2c != NULL)
        {
            DL_I2C_disableInterrupt(
                i2c, I2C_TIMSPM0_INTERRUPT_MASK);
            DL_I2C_resetControllerTransfer(i2c);
            DL_I2C_flushControllerTXFIFO(i2c);
            DL_I2C_flushControllerRXFIFO(i2c);
            DL_I2C_clearInterruptStatus(
                i2c, I2C_TIMSPM0_INTERRUPT_MASK);
        }

        i2c_xfer[index].state = TIMSPM0_I2C_ERROR;
        i2c_xfer[index].error = UINT32_MAX;
    }

    (void) I2C_NVIC_Init(index);

    if (state == TIMSPM0_I2C_DONE)
    {
        return hwI2C_OK;
    }

    if (state == TIMSPM0_I2C_ERROR)
    {
        return hwI2C_BusError;
    }

    return hwI2C_SlaveTimeout;
}

hwI2C_OpResult I2C_Master_Init(
    hwI2C_Index index,
    hwI2C_Speed_Mode speed_mode)
{
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

    I2C_Regs *i2c = I2C_Map_Soc_Base(index);
    IRQn_Type irq;
    uint8_t timer_period;

    if ((i2c == NULL) ||
        !I2C_Map_Soc_IRQ(index, &irq) ||
        !I2C_GetTimerPeriod(speed_mode, &timer_period))
    {
        return hwI2C_InvalidParameter;
    }

    if (NeonRTOS_SyncObjCreate(
            &I2C_Master_Done_SyncHandle[index]) !=
        NeonRTOS_OK)
    {
        return hwI2C_MemoryError;
    }

    uint32_t scl_iomux;
    uint32_t sda_iomux;
    hwI2C_OpResult result =
        I2C_ConfigPins(index, &scl_iomux, &sda_iomux);

    if (result != hwI2C_OK)
    {
        NeonRTOS_SyncObjDelete(
            &I2C_Master_Done_SyncHandle[index]);
        return result;
    }

    DL_I2C_reset(i2c);
    DL_I2C_enablePower(i2c);
    DL_Common_delayCycles(
        I2C_TIMSPM0_POWER_STARTUP_DELAY);

    const DL_I2C_ClockConfig clock_config = {
        .clockSel = DL_I2C_CLOCK_BUSCLK,
        .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
    };

    DL_I2C_setClockConfig(i2c, &clock_config);
    DL_I2C_disableAnalogGlitchFilter(i2c);

    DL_I2C_resetControllerTransfer(i2c);
    DL_I2C_setControllerAddressingMode(
        i2c,
        DL_I2C_CONTROLLER_ADDRESSING_MODE_7_BIT);
    DL_I2C_setTimerPeriod(i2c, timer_period);
    DL_I2C_setControllerTXFIFOThreshold(
        i2c,
        DL_I2C_TX_FIFO_LEVEL_BYTES_1);
    DL_I2C_setControllerRXFIFOThreshold(
        i2c,
        DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(i2c);

    DL_I2C_disableInterrupt(
        i2c, I2C_TIMSPM0_INTERRUPT_MASK);
    DL_I2C_clearInterruptStatus(
        i2c, I2C_TIMSPM0_INTERRUPT_MASK);

    memset(
        &i2c_xfer[index],
        0,
        sizeof(i2c_xfer[index]));
    i2c_xfer[index].state = TIMSPM0_I2C_IDLE;

    DL_I2C_enableController(i2c);

    result = I2C_NVIC_Init(index);

    if (result != hwI2C_OK)
    {
        DL_I2C_disableController(i2c);
        DL_I2C_reset(i2c);
        DL_I2C_disablePower(i2c);
        I2C_DeConfigPins(index);
        NeonRTOS_SyncObjDelete(
            &I2C_Master_Done_SyncHandle[index]);
        return result;
    }

    hwGPIO_Pin scl_pin =
        I2C_Pin_Def_Table[index].scl_pin;
    hwGPIO_Pin sda_pin =
        I2C_Pin_Def_Table[index].sda_pin;

    gpio_pin_init_status[scl_pin] = true;
    gpio_pin_init_status[sda_pin] = true;

    I2C_Clock_Speed_Mode[index] = speed_mode;
    I2C_Master_Init_Status[index] = true;

    (void) scl_iomux;
    (void) sda_iomux;
    (void) irq;

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

    I2C_Regs *i2c = I2C_Map_Soc_Base(index);

    if (i2c == NULL)
    {
        return hwI2C_InvalidParameter;
    }

    hwGPIO_Pin scl_pin =
        I2C_Pin_Def_Table[index].scl_pin;
    hwGPIO_Pin sda_pin =
        I2C_Pin_Def_Table[index].sda_pin;

    I2C_Master_Init_Status[index] = false;

    I2C_NVIC_DeInit(index);

    DL_I2C_disableInterrupt(
        i2c, I2C_TIMSPM0_INTERRUPT_MASK);
    DL_I2C_resetControllerTransfer(i2c);
    DL_I2C_flushControllerTXFIFO(i2c);
    DL_I2C_flushControllerRXFIFO(i2c);
    DL_I2C_clearInterruptStatus(
        i2c, I2C_TIMSPM0_INTERRUPT_MASK);
    DL_I2C_disableController(i2c);
    DL_I2C_reset(i2c);
    DL_I2C_disablePower(i2c);

    NeonRTOS_SyncObjDelete(
        &I2C_Master_Done_SyncHandle[index]);

    I2C_DeConfigPins(index);

    gpio_pin_init_status[scl_pin] = false;
    gpio_pin_init_status[sda_pin] = false;

    memset(
        &i2c_xfer[index],
        0,
        sizeof(i2c_xfer[index]));
    i2c_xfer[index].state = TIMSPM0_I2C_IDLE;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_Reset(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX)
    {
        return hwI2C_InvalidParameter;
    }

    if (!I2C_Master_Init_Status[index])
    {
        return hwI2C_NotInit;
    }

    hwI2C_Speed_Mode speed =
        I2C_Clock_Speed_Mode[index];

    hwI2C_OpResult result =
        I2C_Master_DeInit(index);

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

    I2C_Regs *i2c = I2C_Map_Soc_Base(index);

    if (i2c == NULL)
    {
        return hwI2C_InvalidParameter;
    }

    TIMSPM0_I2C_Transfer *transfer =
        &i2c_xfer[index];

    if (I2C_IsTransferActive(transfer->state))
    {
        return hwI2C_BusError;
    }

    I2C_PrepareTransfer(index, i2c);

    memset(transfer, 0, sizeof(*transfer));
    transfer->state = TIMSPM0_I2C_RX;
    transfer->address = address;
    transfer->rx_buf = read_dat;
    transfer->rx_len = read_len;
    transfer->stop = stop;

    uint32_t interrupt_mask =
        DL_I2C_INTERRUPT_CONTROLLER_RXFIFO_TRIGGER |
        DL_I2C_INTERRUPT_CONTROLLER_RX_DONE |
        DL_I2C_INTERRUPT_CONTROLLER_NACK |
        DL_I2C_INTERRUPT_CONTROLLER_ARBITRATION_LOST;

    if (stop)
    {
        interrupt_mask |=
            DL_I2C_INTERRUPT_CONTROLLER_STOP;
    }

    DL_I2C_enableInterrupt(i2c, interrupt_mask);

    DL_I2C_startControllerTransferAdvanced(
        i2c,
        address,
        DL_I2C_CONTROLLER_DIRECTION_RX,
        read_len,
        DL_I2C_CONTROLLER_START_ENABLE,
        stop ?
            DL_I2C_CONTROLLER_STOP_ENABLE :
            DL_I2C_CONTROLLER_STOP_DISABLE,
        DL_I2C_CONTROLLER_ACK_DISABLE);

    return I2C_WaitTransfer(index, timeout_ms);
}

hwI2C_OpResult I2C_Master_Write(
    hwI2C_Index index,
    uint8_t address,
    uint8_t *write_dat,
    uint8_t write_len,
    bool stop,
    NeonRTOS_Time_t timeout_ms)
{
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

    I2C_Regs *i2c = I2C_Map_Soc_Base(index);

    if (i2c == NULL)
    {
        return hwI2C_InvalidParameter;
    }

    TIMSPM0_I2C_Transfer *transfer =
        &i2c_xfer[index];

    if (I2C_IsTransferActive(transfer->state))
    {
        return hwI2C_BusError;
    }

    I2C_PrepareTransfer(index, i2c);

    memset(transfer, 0, sizeof(*transfer));
    transfer->state = TIMSPM0_I2C_TX;
    transfer->address = address;
    transfer->tx_buf = write_dat;
    transfer->tx_len = write_len;
    transfer->stop = stop;

    transfer->tx_pos =
        DL_I2C_fillControllerTXFIFO(
            i2c,
            write_dat,
            write_len);

    uint32_t interrupt_mask =
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE |
        DL_I2C_INTERRUPT_CONTROLLER_NACK |
        DL_I2C_INTERRUPT_CONTROLLER_ARBITRATION_LOST;

    if (transfer->tx_pos < transfer->tx_len)
    {
        interrupt_mask |=
            DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER;
    }

    if (stop)
    {
        interrupt_mask |=
            DL_I2C_INTERRUPT_CONTROLLER_STOP;
    }

    DL_I2C_enableInterrupt(i2c, interrupt_mask);

    DL_I2C_startControllerTransferAdvanced(
        i2c,
        address,
        DL_I2C_CONTROLLER_DIRECTION_TX,
        write_len,
        DL_I2C_CONTROLLER_START_ENABLE,
        stop ?
            DL_I2C_CONTROLLER_STOP_ENABLE :
            DL_I2C_CONTROLLER_STOP_DISABLE,
        DL_I2C_CONTROLLER_ACK_DISABLE);

    return I2C_WaitTransfer(index, timeout_ms);
}

bool I2C_Master_isInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX)
    {
        return false;
    }

    return I2C_Master_Init_Status[index];
}

#endif //DEVICE_TIMSPM0