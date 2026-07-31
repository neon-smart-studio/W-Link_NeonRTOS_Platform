#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "GPIO/GPIO.h"
#include "CAN/CAN.h"
#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSPM0

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"

#include "CAN/Pin/TIMSPM0/CAN_Pin_TIMSPM0.h"

#include "CAN_TIMSPM0.h"

#if defined(CANFD0_BASE) || defined(CANFD1_BASE)

/*
 * This implementation keeps the public API in Classic CAN mode:
 * standard 11-bit ID and up to 8 data bytes.
 */
#define CAN_TX_BUFFER_INDEX       0U
#define CAN_RX_FIFO_NUMBER        DL_MCAN_RX_FIFO_NUM_0
#define CAN_RX_QUEUE_LEN          8U
#define CAN_CLASSIC_MAX_DATA_LEN  8U
#define CAN_STANDARD_ID_MAX       0x7FFU
#define CAN_HW_WAIT_LIMIT         1000000UL

/*
 * CAN_TIMSPM0_CLOCK_HZ must be the actual MCAN_FCLK after the divider.
 * The defaults match TI LaunchPad MCAN examples using a 40 MHz HFXT.
 */
#define CAN_TIMSPM0_BITRATE       500000UL
#define CAN_TIMSPM0_CLOCK_HZ      40000000UL
#define CAN_TIMSPM0_CLOCK_SOURCE  DL_MCAN_FCLK_HFCLK
#define CAN_TIMSPM0_CLOCK_DIV     DL_MCAN_FCLK_DIV_1

#define CAN_MCAN_CORE_INTERRUPTS                                      \
    (DL_MCAN_INTERRUPT_RF0N | DL_MCAN_INTERRUPT_TC |                  \
     DL_MCAN_INTERRUPT_ARA  | DL_MCAN_INTERRUPT_BEU |                 \
     DL_MCAN_INTERRUPT_BO   | DL_MCAN_INTERRUPT_EP  |                 \
     DL_MCAN_INTERRUPT_EW   | DL_MCAN_INTERRUPT_MRAF |                \
     DL_MCAN_INTERRUPT_PEA  | DL_MCAN_INTERRUPT_PED)

static bool CAN_Init_Status[hwCAN_Index_MAX] = {false};

static NeonRTOS_SyncObj_t CAN_TxDone_Sync[hwCAN_Index_MAX];
static NeonRTOS_MsgQ_t CAN_RxQueue[hwCAN_Index_MAX];

static void CAN_IRQ_Process(hwCAN_Index index);

static const DL_MCAN_ClockConfig CAN_ClockConfig =
{
    .clockSel = CAN_TIMSPM0_CLOCK_SOURCE,
    .divider  = CAN_TIMSPM0_CLOCK_DIV,
};

static const DL_MCAN_InitParams CAN_InitParams =
{
    .fdMode          = false,
    .brsEnable       = false,
    .txpEnable       = false,
    .efbi            = false,
    .pxhddisable     = false,
    .darEnable       = false,
    .wkupReqEnable   = false,
    .autoWkupEnable  = false,
    .emulationEnable = true,
    .wdcPreload      = 0xFFU,
    .tdcConfig       = {
        .tdcf = 0U,
        .tdco = 0U,
    },
    .tdcEnable       = false,
};

static const DL_MCAN_ConfigParams CAN_ConfigParams =
{
    .monEnable        = false,
    .asmEnable        = false,
    .tsPrescalar      = 1U,
    .tsSelect         = 0U,
    .timeoutSelect    = DL_MCAN_TIMEOUT_SELECT_CONT,
    .timeoutPreload   = 0U,
    .timeoutCntEnable = false,
    .filterConfig     = {
        .rrfe = true,
        .rrfs = true,
        .anfe = 2U,   /* Reject non-matching extended frames. */
        .anfs = 0U,   /* Route all standard data frames to RX FIFO0. */
    },
};

/*
 * Message RAM layout in bytes:
 *   0x000..0x00F: TX buffer 0 (8-byte payload element)
 *   0x010..0x08F: RX FIFO0, 8 elements x 16 bytes
 */
static const DL_MCAN_MsgRAMConfigParams CAN_MsgRAMConfig =
{
    .flssa                = 0U,
    .lss                  = 0U,
    .flesa                = 0U,
    .lse                  = 0U,
    .txStartAddr          = 0U,
    .txBufNum             = 1U,
    .txFIFOSize           = 0U,
    .txBufMode            = 0U,
    .txBufElemSize        = DL_MCAN_ELEM_SIZE_8BYTES,
    .txEventFIFOStartAddr = 16U,
    .txEventFIFOSize      = 0U,
    .txEventFIFOWaterMark = 0U,
    .rxFIFO0startAddr     = 16U,
    .rxFIFO0size          = CAN_RX_QUEUE_LEN,
    .rxFIFO0waterMark     = 0U,
    .rxFIFO0OpMode        = 0U,
    .rxFIFO1startAddr     = 144U,
    .rxFIFO1size          = 0U,
    .rxFIFO1waterMark     = 0U,
    .rxFIFO1OpMode        = 0U,
    .rxBufStartAddr       = 144U,
    .rxBufElemSize        = DL_MCAN_ELEM_SIZE_8BYTES,
    .rxFIFO0ElemSize      = DL_MCAN_ELEM_SIZE_8BYTES,
    .rxFIFO1ElemSize      = DL_MCAN_ELEM_SIZE_8BYTES,
};

static MCAN_Regs *CAN_Map_Soc_Base(hwCAN_Index index)
{
    switch (index)
    {
        case hwCAN_Index_0:
#if defined(CANFD0_BASE)
            return CANFD0;
#else
            return NULL;
#endif

        case hwCAN_Index_1:
#if defined(CANFD1_BASE)
            return CANFD1;
#else
            return NULL;
#endif

        default:
            return NULL;
    }
}

/*
 * Resolve every CAN-capable pin exposed by the current MSPM0 device header.
 * A pin route not present on the selected MCU is removed by its #if guard.
 */
static uint32_t CAN_Map_Soc_Pin_Function(hwCAN_Index index, hwGPIO_Pin pin)
{
    if (index == hwCAN_Index_0)
    {
        switch(pin)
        {
            case hwGPIO_Pin_A26:
                return IOMUX_PINCM59_PF_CANFD0_CANTX;

            case hwGPIO_Pin_A27:
                return IOMUX_PINCM60_PF_CANFD0_CANRX;
        }
    }

    if (index == hwCAN_Index_1)
    {
        switch(pin)
        {
            case hwGPIO_Pin_B21:
                return IOMUX_PINCM49_PF_CANFD1_CANTX;

            case hwGPIO_Pin_B22:
                return IOMUX_PINCM50_PF_CANFD1_CANRX;
        }
    }

    return 0;
}

static bool CAN_BuildBitTiming(DL_MCAN_BitTimingParams *timing)
{
    uint32_t prescaler;

    if (timing == NULL || CAN_TIMSPM0_BITRATE == 0U ||
        CAN_TIMSPM0_CLOCK_HZ == 0U)
    {
        return false;
    }

    memset(timing, 0, sizeof(*timing));

    /*
     * Prefer the lowest prescaler (largest number of time quanta).
     * The sample point is placed close to 80%.
     */
    for (prescaler = 1U; prescaler <= 512U; ++prescaler)
    {
        uint64_t denominator =
            (uint64_t) CAN_TIMSPM0_BITRATE * (uint64_t) prescaler;
        uint32_t tq;
        uint32_t seg1;
        uint32_t seg2;

        if (((uint64_t) CAN_TIMSPM0_CLOCK_HZ % denominator) != 0U)
        {
            continue;
        }

        tq = (uint32_t) ((uint64_t) CAN_TIMSPM0_CLOCK_HZ / denominator);
        if (tq < 8U || tq > 385U)
        {
            continue;
        }

        seg2 = tq / 5U;
        if (seg2 < 2U)
        {
            seg2 = 2U;
        }
        if (seg2 > 128U)
        {
            seg2 = 128U;
        }

        seg1 = tq - 1U - seg2;
        if (seg1 < 2U || seg1 > 256U)
        {
            continue;
        }

        timing->nomRatePrescalar  = prescaler - 1U;
        timing->nomTimeSeg1       = seg1 - 1U;
        timing->nomTimeSeg2       = seg2 - 1U;
        timing->nomSynchJumpWidth = seg2 - 1U;
        return true;
    }

    return false;
}

static bool CAN_WaitForMemInit(MCAN_Regs *base)
{
    uint32_t wait = CAN_HW_WAIT_LIMIT;

    while (!DL_MCAN_isMemInitDone(base))
    {
        if (--wait == 0U)
        {
            return false;
        }
    }

    return true;
}

static bool CAN_SetOpMode(MCAN_Regs *base, uint32_t mode)
{
    uint32_t wait = CAN_HW_WAIT_LIMIT;

    DL_MCAN_setOpMode(base, mode);
    while (DL_MCAN_getOpMode(base) != mode)
    {
        if (--wait == 0U)
        {
            return false;
        }
    }

    return true;
}

static bool CAN_HardwareInit(MCAN_Regs *base)
{
    DL_MCAN_BitTimingParams bit_timing;

    if (!CAN_BuildBitTiming(&bit_timing))
    {
        return false;
    }

    DL_MCAN_reset(base);
    DL_MCAN_enablePower(base);
    delay_cycles(16U);

    if (!DL_MCAN_isPowerEnabled(base))
    {
        return false;
    }

    DL_MCAN_enableModuleClock(base);
    DL_MCAN_setClockConfig(base, &CAN_ClockConfig);

    if (!CAN_WaitForMemInit(base) ||
        !CAN_SetOpMode(base, DL_MCAN_OPERATION_MODE_SW_INIT))
    {
        return false;
    }

    if (DL_MCAN_init(base, &CAN_InitParams) != 0 ||
        DL_MCAN_config(base, &CAN_ConfigParams) != 0 ||
        DL_MCAN_setBitTime(base, &bit_timing) != 0 ||
        DL_MCAN_msgRAMConfig(base, &CAN_MsgRAMConfig) != 0 ||
        DL_MCAN_setExtIDAndMask(base, 0x1FFFFFFFU) != 0)
    {
        return false;
    }

    if (!CAN_SetOpMode(base, DL_MCAN_OPERATION_MODE_NORMAL))
    {
        return false;
    }

    if (DL_MCAN_TXBufTransIntrEnable(
            base, CAN_TX_BUFFER_INDEX, true) != 0)
    {
        return false;
    }

    DL_MCAN_enableIntr(base, CAN_MCAN_CORE_INTERRUPTS, true);
    DL_MCAN_selectIntrLine(base, CAN_MCAN_CORE_INTERRUPTS, DL_MCAN_INTR_LINE_NUM_1);
    DL_MCAN_enableIntrLine(base, DL_MCAN_INTR_LINE_NUM_1, true);

    DL_MCAN_clearInterruptStatus(base, DL_MCAN_MSP_INTERRUPT_LINE1);
    DL_MCAN_enableInterrupt(base, DL_MCAN_MSP_INTERRUPT_LINE1);

    return true;
}

static void CAN_HardwareDeInit(MCAN_Regs *base)
{
    if (base == NULL || !DL_MCAN_isPowerEnabled(base))
    {
        return;
    }

    DL_MCAN_disableInterrupt(base, DL_MCAN_MSP_INTERRUPT_LINE1);
    DL_MCAN_enableIntrLine(base, DL_MCAN_INTR_LINE_NUM_1, false);
    DL_MCAN_enableIntr(base, CAN_MCAN_CORE_INTERRUPTS, false);
    DL_MCAN_TXBufTransIntrEnable(base, CAN_TX_BUFFER_INDEX, false);

    CAN_SetOpMode(base, DL_MCAN_OPERATION_MODE_SW_INIT);

    DL_MCAN_reset(base);
    DL_MCAN_disableModuleClock(base);
    DL_MCAN_disablePower(base);
}

static void CAN_NVIC_Init(hwCAN_Index index)
{
    switch (index)
    {
#if defined(CANFD0_BASE)
        case hwCAN_Index_0:
            NVIC_ClearPendingIRQ(CANFD0_INT_IRQn);
            NVIC_EnableIRQ(CANFD0_INT_IRQn);
            break;
#endif

#if defined(CANFD1_BASE)
        case hwCAN_Index_1:
            NVIC_ClearPendingIRQ(CANFD1_INT_IRQn);
            NVIC_EnableIRQ(CANFD1_INT_IRQn);
            break;
#endif
    }
}

static void CAN_NVIC_DeInit(hwCAN_Index index)
{
    switch (index)
    {
#if defined(CANFD0_BASE)
        case hwCAN_Index_0:
            NVIC_DisableIRQ(CANFD0_INT_IRQn);
            NVIC_ClearPendingIRQ(CANFD0_INT_IRQn);
            break;
#endif

#if defined(CANFD1_BASE)
        case hwCAN_Index_1:
            NVIC_DisableIRQ(CANFD1_INT_IRQn);
            NVIC_ClearPendingIRQ(CANFD1_INT_IRQn);
            break;
#endif
    }
}

#if defined(CANFD0_BASE)
void CANFD0_IRQHandler(void)
{
    CAN_IRQ_Process(hwCAN_Index_0);
}
#endif

#if defined(CANFD1_BASE)
void CANFD1_IRQHandler(void)
{
    CAN_IRQ_Process(hwCAN_Index_1);
}
#endif

static void CAN_IRQ_Process(hwCAN_Index index)
{
    MCAN_Regs *base;
    uint32_t status;

    if (index >= hwCAN_Index_MAX || !CAN_Init_Status[index])
    {
        return;
    }

    base = CAN_Map_Soc_Base(index);
    if (base == NULL ||
        DL_MCAN_getPendingInterrupt(base) != DL_MCAN_IIDX_LINE1)
    {
        return;
    }

    status = DL_MCAN_getIntrStatus(base);
    DL_MCAN_clearIntrStatus(
        base, status, DL_MCAN_INTR_SRC_MCAN_LINE_1);

    if ((status & DL_MCAN_INTERRUPT_RF0N) != 0U)
    {
        for (;;)
        {
            DL_MCAN_RxFIFOStatus fifo_status = {
                .num = CAN_RX_FIFO_NUMBER,
            };
            DL_MCAN_RxBufElement rx_message;
            uint8_t data[CAN_CLASSIC_MAX_DATA_LEN] = {0};
            uint32_t length;

            DL_MCAN_getRxFIFOStatus(base, &fifo_status);
            if (fifo_status.fillLvl == 0U)
            {
                break;
            }

            memset(&rx_message, 0, sizeof(rx_message));
            DL_MCAN_readMsgRam(base, DL_MCAN_MEM_TYPE_FIFO, 0U,
                fifo_status.num, &rx_message);
            DL_MCAN_writeRxFIFOAck(
                base, fifo_status.num, fifo_status.getIdx);

            if (rx_message.rtr != 0U ||
                rx_message.xtd != 0U ||
                rx_message.fdf != 0U)
            {
                continue;
            }

            length = rx_message.dlc;
            if (length > CAN_CLASSIC_MAX_DATA_LEN)
            {
                length = CAN_CLASSIC_MAX_DATA_LEN;
            }

            memcpy(data, rx_message.data, length);

            /*
             * Keep the original queue contract: one queue item is 8 payload
             * bytes. If NeonRTOS has an ISR-specific queue API, use it here.
             */
            (void) NeonRTOS_MsgQWrite(
                &CAN_RxQueue[index], data, NEONRT_NO_WAIT);
        }
    }

    if ((status & DL_MCAN_INTERRUPT_TC) != 0U)
    {
        NeonRTOS_SyncObjSignalFromISR(&CAN_TxDone_Sync[index]);
    }
}

hwCAN_OpResult CAN_Init(hwCAN_Index index)
{
    MCAN_Regs *base;

    if (index >= hwCAN_Index_MAX)
    {
        return hwCAN_InvalidParameter;
    }

    if (CAN_Init_Status[index])
    {
        return hwCAN_OK;
    }

    base = CAN_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwCAN_InvalidParameter;
    }

    hwGPIO_Pin tx_pin = CAN_Pin_Def_Table[index].tx_pin;
    hwGPIO_Pin rx_pin = CAN_Pin_Def_Table[index].rx_pin;

    GPIO_Regs *tx_port = GPIO_Map_Soc_Port_Base(tx_pin);
    GPIO_Regs *rx_port = GPIO_Map_Soc_Port_Base(rx_pin);

    if (tx_port == NULL || rx_port == NULL)
    {
        return hwCAN_InvalidParameter;
    }

    uint32_t tx_iomux = GPIO_Map_Soc_Pin_IOMUX(tx_pin);
    uint32_t rx_iomux = GPIO_Map_Soc_Pin_IOMUX(rx_pin);

    if (tx_iomux==0 || rx_iomux==0)
    {
        return hwCAN_InvalidParameter;
    }

    uint32_t tx_function = I2C_Map_Soc_Pin_Function(index, tx_pin);
    uint32_t rx_function = I2C_Map_Soc_Pin_Function(index, tx_pin);

    if (tx_function==0 || rx_function==0)
    {
        return hwCAN_InvalidParameter;
    }

    if (NeonRTOS_SyncObjCreate(&CAN_TxDone_Sync[index]) != NeonRTOS_OK)
    {
        return hwCAN_MemoryError;
    }

    if (NeonRTOS_MsgQCreate(&CAN_RxQueue[index], "can_rx", CAN_RX_QUEUE_LEN, CAN_CLASSIC_MAX_DATA_LEN) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        return hwCAN_MemoryError;
    }

    DL_GPIO_enablePower(tx_port);
    DL_Common_delayCycles(16U);

    DL_GPIO_enablePower(rx_port);
    DL_Common_delayCycles(16U);

    DL_GPIO_initPeripheralOutputFunction(tx_iomux, tx_function);
    DL_GPIO_initPeripheralInputFunction(rx_iomux, rx_function);

    if (!CAN_HardwareInit(base))
    {
        CAN_HardwareDeInit(base);
        DL_GPIO_initDigitalInput(tx_iomux);
        DL_GPIO_initDigitalInput(rx_iomux);
        NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
        NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);
        return hwCAN_HwError;
    }

    gpio_pin_init_status[tx_pin] = true;
    gpio_pin_init_status[rx_pin] = true;
    CAN_Init_Status[index] = true;

    CAN_NVIC_Init(index);

    return hwCAN_OK;
}

hwCAN_OpResult CAN_DeInit(hwCAN_Index index)
{
    MCAN_Regs *base;

    if (index >= hwCAN_Index_MAX)
    {
        return hwCAN_InvalidParameter;
    }

    if (!CAN_Init_Status[index])
    {
        return hwCAN_OK;
    }

    base = CAN_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwCAN_InvalidParameter;
    }

    hwGPIO_Pin tx_pin = CAN_Pin_Def_Table[index].tx_pin;
    hwGPIO_Pin rx_pin = CAN_Pin_Def_Table[index].rx_pin;

    uint32_t tx_iomux = GPIO_Map_Soc_Pin_IOMUX(tx_pin);
    uint32_t rx_iomux = GPIO_Map_Soc_Pin_IOMUX(rx_pin);

    if (tx_iomux==0 || rx_iomux==0)
    {
        return hwCAN_InvalidParameter;
    }

    CAN_NVIC_DeInit(index);
    CAN_Init_Status[index] = false;

    CAN_HardwareDeInit(base);

    DL_GPIO_initDigitalInput(tx_iomux);
    DL_GPIO_initDigitalInput(rx_iomux);

    NeonRTOS_SyncObjDelete(&CAN_TxDone_Sync[index]);
    NeonRTOS_MsgQDelete(&CAN_RxQueue[index]);

    gpio_pin_init_status[tx_pin] = false;
    gpio_pin_init_status[rx_pin] = false;

    return hwCAN_OK;
}

hwCAN_OpResult CAN_Read(hwCAN_Index index, uint8_t *buf, uint32_t timeout)
{
    if (index >= hwCAN_Index_MAX || buf == NULL)
    {
        return hwCAN_InvalidParameter;
    }

    if (!CAN_Init_Status[index])
    {
        return hwCAN_NotInit;
    }

    if (NeonRTOS_MsgQRead(&CAN_RxQueue[index], buf, timeout) != NeonRTOS_OK)
    {
        return hwCAN_Timeout;
    }

    return hwCAN_OK;
}

hwCAN_OpResult CAN_Write(hwCAN_Index index, uint32_t id, uint8_t *data, uint8_t len, uint32_t timeout)
{
    MCAN_Regs *base;
    DL_MCAN_TxBufElement tx_message;

    if (index >= hwCAN_Index_MAX || data == NULL ||
        len > CAN_CLASSIC_MAX_DATA_LEN || id > CAN_STANDARD_ID_MAX)
    {
        return hwCAN_InvalidParameter;
    }

    if (!CAN_Init_Status[index])
    {
        return hwCAN_NotInit;
    }

    base = CAN_Map_Soc_Base(index);
    if (base == NULL)
    {
        return hwCAN_InvalidParameter;
    }

    if ((DL_MCAN_getTxBufReqPend(base) &
            (1UL << CAN_TX_BUFFER_INDEX)) != 0U)
    {
        return hwCAN_HwError;
    }

    memset(&tx_message, 0, sizeof(tx_message));
    tx_message.id  = id << 18U;
    tx_message.rtr = 0U;
    tx_message.xtd = 0U;
    tx_message.esi = 0U;
    tx_message.dlc = len;
    tx_message.brs = 0U;
    tx_message.fdf = 0U;
    tx_message.efc = 0U;
    memcpy(tx_message.data, data, len);

    DL_MCAN_writeMsgRam(base, DL_MCAN_MEM_TYPE_BUF, CAN_TX_BUFFER_INDEX, &tx_message);

    if (DL_MCAN_TXBufAddReq(base, CAN_TX_BUFFER_INDEX) != 0)
    {
        return hwCAN_HwError;
    }

    if (NeonRTOS_SyncObjWait(&CAN_TxDone_Sync[index], timeout) != NeonRTOS_OK)
    {
        (void) DL_MCAN_txBufCancellationReq(
            base, CAN_TX_BUFFER_INDEX);
        return hwCAN_Timeout;
    }

    return hwCAN_OK;
}

bool CAN_isInit(hwCAN_Index index)
{
    if (index >= hwCAN_Index_MAX)
    {
        return false;
    }

    return CAN_Init_Status[index];
}

#endif /* CANFD0_BASE || CANFD1_BASE */

#endif /* DEVICE_TIMSPM0 */