#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "SPI/SPI_Master.h"

#include "DMA/DMA.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSPM0

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"

#include "SPI/Pin/TIMSPM0/SPI_Pin_TIMSPM0.h"

#include "SPI_Master_TIMSPM0.h"

#ifndef SPI_TIMSPM0_POWER_STARTUP_DELAY
#define SPI_TIMSPM0_POWER_STARTUP_DELAY        (16U)
#endif

#define SPI_MASTER_MUTEX_ACCESS_TIMEOUT         (500U)
#define SPI_MASTER_OP_TIMEOUT                   (3000U)
#define SPI_TIMSPM0_RX_OVERFLOW_MASK            (DL_SPI_INTERRUPT_RX_OVERFLOW)

#if defined(MSPM0G120x) || defined(MSPM0G121x) || \
    defined(MSPM0G320x) || defined(MSPM0G321x) || \
    defined(MSPM0G511x) || defined(MSPM0G518x) || \
    defined(MSPM0L112x) || defined(MSPM0L211x)
typedef UNICOMM_Inst_Regs TIMSPM0_SPI_Regs;
#else
typedef SPI_Regs TIMSPM0_SPI_Regs;
#endif

typedef enum {
    TIMSPM0_SPI_PIN_MOSI = 0,
    TIMSPM0_SPI_PIN_MISO,
    TIMSPM0_SPI_PIN_SCLK,
    TIMSPM0_SPI_PIN_CS
} TIMSPM0_SPI_PinSignal;

typedef struct {
    DL_SPI_CLOCK_DIVIDE_RATIO divide_ratio;
    uint32_t serial_divider;
} TIMSPM0_SPI_Clock;

bool Spi_Master_Init_Status[hwSPI_Index_MAX] = {false};

static bool Spi_Master_Use_CS[hwSPI_Index_MAX] = {false};
static uint32_t Spi_Master_Clock_Hz[hwSPI_Index_MAX] = {0U};
static hwSPI_OpMode Spi_Master_Mode[hwSPI_Index_MAX];
static NeonRTOS_LockObj_t Spi_Master_Access_Mutex[hwSPI_Index_MAX];

#define SPI_MASTER_MUTEX_LOCK(index, timeout)                              \
    do {                                                                   \
        if (NeonRTOS_LockObjLock(                                          \
                &Spi_Master_Access_Mutex[(index)], (timeout)) !=            \
            NeonRTOS_OK) {                                                 \
            return hwSPI_MutexTimeout;                                     \
        }                                                                  \
    } while (0)

#define SPI_MASTER_MUTEX_UNLOCK(index)                                     \
    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[(index)])

static TIMSPM0_SPI_Regs *SPI_Map_Soc_Base(hwSPI_Index index)
{
#if defined(MSPM0G120x) || defined(MSPM0G121x) || \
    defined(MSPM0G320x) || defined(MSPM0G321x)
    switch (index)
    {
        case hwSPI_Index_0:
            return UC2;

        case hwSPI_Index_1:
            return UC4;

        default:
            return NULL;
    }
#elif defined(MSPM0G511x) || defined(MSPM0G518x)
    switch (index)
    {
        case hwSPI_Index_0:
            return UC2;

        case hwSPI_Index_1:
            return UC3;

        default:
            return NULL;
    }
#elif defined(MSPM0L112x) || defined(MSPM0L211x)
    switch (index)
    {
        case hwSPI_Index_0:
            return UC4;

        case hwSPI_Index_1:
            return UC8;

        default:
            return NULL;
    }
#else
    switch (index)
    {
#if defined(SPI0_BASE)
        case hwSPI_Index_0:
            return SPI0;
#endif

#if defined(SPI1_BASE)
        case hwSPI_Index_1:
            return SPI1;
#endif

#if defined(SPI2_BASE)
        case hwSPI_Index_2:
            return SPI2;
#endif

        default:
            return NULL;
    }
#endif
}

/*
 * GPIO_Map_Soc_Pin_IOMUX() 由 GPIO_Pin_TIMSPM0_IOMUX.c（附件 2）
 * 提供 PINCM；本函式依 SPI_Pin_Def_Table 的固定路由提供該 PINCM
 * 所需的 peripheral-function 值。
 */
static uint32_t SPI_Map_Soc_Pin_Function(
    hwSPI_Index index,
    TIMSPM0_SPI_PinSignal signal)
{
#if defined(MSPM0C110x) || defined(MSPM0S003Fx)
    if (index != hwSPI_Index_0)
    {
        return 0U;
    }

    switch (signal)
    {
        case TIMSPM0_SPI_PIN_MOSI:
            return IOMUX_PINCM19_PF_SPI0_PICO;
        case TIMSPM0_SPI_PIN_MISO:
            return IOMUX_PINCM5_PF_SPI0_POCI;
        case TIMSPM0_SPI_PIN_SCLK:
            return IOMUX_PINCM7_PF_SPI0_SCLK;
        case TIMSPM0_SPI_PIN_CS:
            return IOMUX_PINCM3_PF_SPI0_CS0;
        default:
            return 0U;
    }

#elif defined(MSPM0C031Cx) || defined(MSPM0G031Cx) || \
      defined(MSPM0C1105) || defined(MSPM0C1106) || \
      defined(MSPM0H321x)
    if (index != hwSPI_Index_0)
    {
        return 0U;
    }

    switch (signal)
    {
        case TIMSPM0_SPI_PIN_MOSI:
            return IOMUX_PINCM8_PF_SPI0_PICO;
        case TIMSPM0_SPI_PIN_MISO:
            return IOMUX_PINCM7_PF_SPI0_POCI;
        case TIMSPM0_SPI_PIN_SCLK:
            return IOMUX_PINCM9_PF_SPI0_SCLK;
        case TIMSPM0_SPI_PIN_CS:
            return IOMUX_PINCM5_PF_SPI0_CS0;
        default:
            return 0U;
    }

#elif defined(MSPM0L110x) || defined(MSPM0L130x) || \
      defined(MSPM0L134x)
    if (index != hwSPI_Index_0)
    {
        return 0U;
    }

    switch (signal)
    {
        case TIMSPM0_SPI_PIN_MOSI:
            return IOMUX_PINCM6_PF_SPI0_PICO;
        case TIMSPM0_SPI_PIN_MISO:
            return IOMUX_PINCM5_PF_SPI0_POCI;
        case TIMSPM0_SPI_PIN_SCLK:
            return IOMUX_PINCM7_PF_SPI0_SCLK;
        case TIMSPM0_SPI_PIN_CS:
            return IOMUX_PINCM3_PF_SPI0_CS0;
        default:
            return 0U;
    }

#elif defined(MSPM0G120x) || defined(MSPM0G121x) || \
      defined(MSPM0G320x) || defined(MSPM0G321x)
    switch (index)
    {
        case hwSPI_Index_0:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM10_PF_UC2_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM9_PF_UC2_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM11_PF_UC2_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM7_PF_UC2_CS0;
                default:
                    return 0U;
            }

        case hwSPI_Index_1:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM25_PF_UC4_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM24_PF_UC4_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM26_PF_UC4_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM23_PF_UC4_CS0;
                default:
                    return 0U;
            }

        default:
            return 0U;
    }

#elif defined(MSPM0G511x) || defined(MSPM0G518x)
    switch (index)
    {
        case hwSPI_Index_0:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM10_PF_UC2_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM9_PF_UC2_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM11_PF_UC2_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM19_PF_UC2_CS0;
                default:
                    return 0U;
            }

        case hwSPI_Index_1:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM25_PF_UC3_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM24_PF_UC3_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM26_PF_UC3_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM23_PF_UC3_CS0;
                default:
                    return 0U;
            }

        default:
            return 0U;
    }

#elif defined(MSPM0L112x) || defined(MSPM0L211x)
    switch (index)
    {
        case hwSPI_Index_0:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM10_PF_UC4_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM9_PF_UC4_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM11_PF_UC4_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM7_PF_UC4_CS0;
                default:
                    return 0U;
            }

        case hwSPI_Index_1:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM25_PF_UC8_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM24_PF_UC8_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM26_PF_UC8_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM23_PF_UC8_CS0;
                default:
                    return 0U;
            }

        default:
            return 0U;
    }

#elif defined(MSPM0L122x) || defined(MSPM0L222x)
    switch (index)
    {
        case hwSPI_Index_0:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM10_PF_SPI0_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM9_PF_SPI0_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM11_PF_SPI0_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM7_PF_SPI0_CS0;
                default:
                    return 0U;
            }

        case hwSPI_Index_1:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM29_PF_SPI1_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM28_PF_SPI1_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM30_PF_SPI1_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM27_PF_SPI1_CS0;
                default:
                    return 0U;
            }

        default:
            return 0U;
    }

#elif defined(MSPM0G151x) || defined(MSPM0G351x) || \
      defined(MSPM0G352x)
    switch (index)
    {
        case hwSPI_Index_0:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM10_PF_SPI0_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM9_PF_SPI0_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM11_PF_SPI0_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM7_PF_SPI0_CS0;
                default:
                    return 0U;
            }

        case hwSPI_Index_1:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM25_PF_SPI1_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM24_PF_SPI1_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM26_PF_SPI1_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM23_PF_SPI1_CS0;
                default:
                    return 0U;
            }

        case hwSPI_Index_2:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM17_PF_SPI2_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM18_PF_SPI2_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM21_PF_SPI2_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM9_PF_SPI2_CS0;
                default:
                    return 0U;
            }

        default:
            return 0U;
    }

#elif defined(MSPM0G110x) || defined(MSPM0G150x) || \
      defined(MSPM0G310x) || defined(MSPM0G350x)
    switch (index)
    {
        case hwSPI_Index_0:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM10_PF_SPI0_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM9_PF_SPI0_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM11_PF_SPI0_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM7_PF_SPI0_CS0;
                default:
                    return 0U;
            }

        case hwSPI_Index_1:
            switch (signal)
            {
                case TIMSPM0_SPI_PIN_MOSI:
                    return IOMUX_PINCM25_PF_SPI1_PICO;
                case TIMSPM0_SPI_PIN_MISO:
                    return IOMUX_PINCM24_PF_SPI1_POCI;
                case TIMSPM0_SPI_PIN_SCLK:
                    return IOMUX_PINCM26_PF_SPI1_SCLK;
                case TIMSPM0_SPI_PIN_CS:
                    return IOMUX_PINCM23_PF_SPI1_CS0;
                default:
                    return 0U;
            }

        default:
            return 0U;
    }

#elif defined(MSPM0L111x)
    if (index != hwSPI_Index_0)
    {
        return 0U;
    }

    switch (signal)
    {
        case TIMSPM0_SPI_PIN_MOSI:
            return IOMUX_PINCM10_PF_SPI0_PICO;
        case TIMSPM0_SPI_PIN_MISO:
            return IOMUX_PINCM9_PF_SPI0_POCI;
        case TIMSPM0_SPI_PIN_SCLK:
            return IOMUX_PINCM11_PF_SPI0_SCLK;
        case TIMSPM0_SPI_PIN_CS:
            return IOMUX_PINCM7_PF_SPI0_CS0;
        default:
            return 0U;
    }

#else
    (void) index;
    (void) signal;
    return 0U;
#endif
}

static bool SPI_Get_Clock_Config(
    uint32_t clock_rate_hz,
    TIMSPM0_SPI_Clock *result)
{
    uint32_t best_hz = 0U;
    uint32_t best_ratio = 0U;
    uint32_t best_serial_divider = 0U;
    const uint32_t source_hz = (uint32_t) F_CPU;

    if ((clock_rate_hz == 0U) ||
        (source_hz == 0U) ||
        (result == NULL))
    {
        return false;
    }

    for (uint32_t ratio = 1U; ratio <= 8U; ratio++)
    {
        uint64_t denominator =
            (uint64_t) ratio * 2ULL * (uint64_t) clock_rate_hz;
        uint32_t serial_divider =
            (uint32_t)
            (((uint64_t) source_hz + denominator - 1ULL) /
             denominator);

        if (serial_divider == 0U)
        {
            serial_divider = 1U;
        }

        if (serial_divider > 1024U)
        {
            continue;
        }

        uint32_t actual_hz =
            source_hz / (ratio * 2U * serial_divider);

        if ((actual_hz <= clock_rate_hz) &&
            (actual_hz > best_hz))
        {
            best_hz = actual_hz;
            best_ratio = ratio;
            best_serial_divider = serial_divider;
        }
    }

    if ((best_ratio == 0U) ||
        (best_serial_divider == 0U))
    {
        return false;
    }

    switch (best_ratio)
    {
        case 1U:
            result->divide_ratio = DL_SPI_CLOCK_DIVIDE_RATIO_1;
            break;
        case 2U:
            result->divide_ratio = DL_SPI_CLOCK_DIVIDE_RATIO_2;
            break;
        case 3U:
            result->divide_ratio = DL_SPI_CLOCK_DIVIDE_RATIO_3;
            break;
        case 4U:
            result->divide_ratio = DL_SPI_CLOCK_DIVIDE_RATIO_4;
            break;
        case 5U:
            result->divide_ratio = DL_SPI_CLOCK_DIVIDE_RATIO_5;
            break;
        case 6U:
            result->divide_ratio = DL_SPI_CLOCK_DIVIDE_RATIO_6;
            break;
        case 7U:
            result->divide_ratio = DL_SPI_CLOCK_DIVIDE_RATIO_7;
            break;
        case 8U:
            result->divide_ratio = DL_SPI_CLOCK_DIVIDE_RATIO_8;
            break;
        default:
            return false;
    }

    result->serial_divider = best_serial_divider - 1U;
    return true;
}

static bool SPI_Map_Frame_Format(
    hwSPI_OpMode op_mode,
    bool use_cs,
    DL_SPI_FRAME_FORMAT *format)
{
    if (format == NULL)
    {
        return false;
    }

    switch (op_mode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            *format = use_cs ?
                DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA0 :
                DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0;
            return true;

        case hwSPI_OpMode_Polarity0_Phase1:
            *format = use_cs ?
                DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA1 :
                DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA1;
            return true;

        case hwSPI_OpMode_Polarity1_Phase0:
            *format = use_cs ?
                DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA0 :
                DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA0;
            return true;

        case hwSPI_OpMode_Polarity1_Phase1:
            *format = use_cs ?
                DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA1 :
                DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA1;
            return true;

        default:
            return false;
    }
}

static hwSPI_OpResult SPI_Config_Module(
    TIMSPM0_SPI_Regs *spi,
    uint32_t clock_rate_hz,
    hwSPI_OpMode op_mode,
    bool use_cs)
{
    TIMSPM0_SPI_Clock spi_clock;
    DL_SPI_FRAME_FORMAT frame_format;

    if ((spi == NULL) ||
        !SPI_Get_Clock_Config(clock_rate_hz, &spi_clock) ||
        !SPI_Map_Frame_Format(op_mode, use_cs, &frame_format))
    {
        return hwSPI_InvalidParameter;
    }

    DL_SPI_disable(spi);

    DL_SPI_Config spi_config = {
        .mode = DL_SPI_MODE_CONTROLLER,
        .frameFormat = frame_format,
        .parity = DL_SPI_PARITY_NONE,
        .dataSize = DL_SPI_DATA_SIZE_8,
        .bitOrder = DL_SPI_BIT_ORDER_MSB_FIRST,
        .chipSelectPin = use_cs ?
            DL_SPI_CHIP_SELECT_0 :
            DL_SPI_CHIP_SELECT_NONE,
    };

    DL_SPI_ClockConfig clock_config = {
        .clockSel = DL_SPI_CLOCK_BUSCLK,
        .divideRatio = spi_clock.divide_ratio,
    };

    DL_SPI_init(spi, &spi_config);
    DL_SPI_setClockConfig(spi, &clock_config);
    DL_SPI_setBitRateSerialClockDivider(
        spi, spi_clock.serial_divider);

    DL_SPI_clearInterruptStatus(
        spi, SPI_TIMSPM0_RX_OVERFLOW_MASK);
    DL_SPI_enable(spi);

    return hwSPI_OK;
}

static hwSPI_OpResult SPI_Config_Pins(
    hwSPI_Index index,
    bool use_cs)
{
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index].mosi_pin;
    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index].miso_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index].cs_pin;

    uint32_t mosi_cm = GPIO_Map_Soc_Pin_IOMUX(mosi_pin);
    uint32_t miso_cm = GPIO_Map_Soc_Pin_IOMUX(miso_pin);
    uint32_t sclk_cm = GPIO_Map_Soc_Pin_IOMUX(sclk_pin);
    uint32_t cs_cm = use_cs ?
        GPIO_Map_Soc_Pin_IOMUX(cs_pin) :
        GPIO_SOC_IOMUX_INVALID;

    uint32_t mosi_function =
        SPI_Map_Soc_Pin_Function(index, TIMSPM0_SPI_PIN_MOSI);
    uint32_t miso_function =
        SPI_Map_Soc_Pin_Function(index, TIMSPM0_SPI_PIN_MISO);
    uint32_t sclk_function =
        SPI_Map_Soc_Pin_Function(index, TIMSPM0_SPI_PIN_SCLK);
    uint32_t cs_function = use_cs ?
        SPI_Map_Soc_Pin_Function(index, TIMSPM0_SPI_PIN_CS) :
        0U;

    if ((mosi_pin == hwGPIO_Pin_NC) ||
        (miso_pin == hwGPIO_Pin_NC) ||
        (sclk_pin == hwGPIO_Pin_NC) ||
        (mosi_cm == GPIO_SOC_IOMUX_INVALID) ||
        (miso_cm == GPIO_SOC_IOMUX_INVALID) ||
        (sclk_cm == GPIO_SOC_IOMUX_INVALID) ||
        (mosi_function == 0U) ||
        (miso_function == 0U) ||
        (sclk_function == 0U))
    {
        return hwSPI_InvalidParameter;
    }

    if (use_cs &&
        ((cs_pin == hwGPIO_Pin_NC) ||
         (cs_cm == GPIO_SOC_IOMUX_INVALID) ||
         (cs_function == 0U)))
    {
        return hwSPI_InvalidParameter;
    }

    DL_GPIO_initPeripheralOutputFunction(
        mosi_cm, mosi_function);
    DL_GPIO_initPeripheralInputFunction(
        miso_cm, miso_function);
    DL_GPIO_initPeripheralOutputFunction(
        sclk_cm, sclk_function);

    if (use_cs)
    {
        DL_GPIO_initPeripheralOutputFunction(
            cs_cm, cs_function);
    }

    return hwSPI_OK;
}

static void SPI_DeConfig_Pins(
    hwSPI_Index index,
    bool use_cs)
{
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index].mosi_pin;
    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index].miso_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index].cs_pin;

    uint32_t mosi_cm = GPIO_Map_Soc_Pin_IOMUX(mosi_pin);
    uint32_t miso_cm = GPIO_Map_Soc_Pin_IOMUX(miso_pin);
    uint32_t sclk_cm = GPIO_Map_Soc_Pin_IOMUX(sclk_pin);

    if (mosi_cm != GPIO_SOC_IOMUX_INVALID)
    {
        DL_GPIO_initDigitalInput(mosi_cm);
    }

    if (miso_cm != GPIO_SOC_IOMUX_INVALID)
    {
        DL_GPIO_initDigitalInput(miso_cm);
    }

    if (sclk_cm != GPIO_SOC_IOMUX_INVALID)
    {
        DL_GPIO_initDigitalInput(sclk_cm);
    }

    if (use_cs)
    {
        uint32_t cs_cm = GPIO_Map_Soc_Pin_IOMUX(cs_pin);

        if (cs_cm != GPIO_SOC_IOMUX_INVALID)
        {
            DL_GPIO_initDigitalInput(cs_cm);
        }
    }
}

static void SPI_Flush_RX(TIMSPM0_SPI_Regs *spi)
{
    while (!DL_SPI_isRXFIFOEmpty(spi))
    {
        (void) DL_SPI_receiveData8(spi);
    }

    DL_SPI_clearInterruptStatus(
        spi, SPI_TIMSPM0_RX_OVERFLOW_MASK);
}

static void SPI_Recover(hwSPI_Index index)
{
    TIMSPM0_SPI_Regs *spi = SPI_Map_Soc_Base(index);

    if (spi == NULL)
    {
        return;
    }

    DL_SPI_disable(spi);
    DL_SPI_reset(spi);
    DL_SPI_enablePower(spi);
    DL_Common_delayCycles(SPI_TIMSPM0_POWER_STARTUP_DELAY);

    (void) SPI_Config_Module(
        spi,
        Spi_Master_Clock_Hz[index],
        Spi_Master_Mode[index],
        Spi_Master_Use_CS[index]);

    SPI_Flush_RX(spi);
}

/*
 * MSPM0 SPI controller 每送出一個 frame 就會收到一個 frame。
 * 先清 RX，再交錯補 TX FIFO 與排空 RX FIFO，避免長資料流 RX overflow。
 * timeout 是「無進度」時間，長資料流不會因總長度超過 3 秒而誤判。
 */
static hwSPI_OpResult SPI_Transfer_Locked(
    hwSPI_Index index,
    const uint8_t *tx_buf,
    uint8_t *rx_buf,
    uint32_t len)
{
    TIMSPM0_SPI_Regs *spi = SPI_Map_Soc_Base(index);

    if ((spi == NULL) || (len == 0U))
    {
        return hwSPI_InvalidParameter;
    }

    uint32_t tx_pos = 0U;
    uint32_t rx_pos = 0U;
    NeonRTOS_Time_t last_progress = NeonRTOS_Millis();

    SPI_Flush_RX(spi);

    while (rx_pos < len)
    {
        bool progress = false;

        while ((rx_pos < tx_pos) &&
               !DL_SPI_isRXFIFOEmpty(spi))
        {
            uint8_t value = DL_SPI_receiveData8(spi);

            if (rx_buf != NULL)
            {
                rx_buf[rx_pos] = value;
            }

            rx_pos++;
            progress = true;
        }

        if (DL_SPI_getRawInterruptStatus(
                spi, SPI_TIMSPM0_RX_OVERFLOW_MASK) != 0U)
        {
            SPI_Recover(index);
            return hwSPI_HwError;
        }

        if ((tx_pos < len) &&
            !DL_SPI_isTXFIFOFull(spi))
        {
            uint8_t value = (tx_buf != NULL) ?
                tx_buf[tx_pos] :
                0xFFU;

            DL_SPI_transmitData8(spi, value);
            tx_pos++;
            progress = true;
        }

        if (progress)
        {
            last_progress = NeonRTOS_Millis();
        }
        else if ((NeonRTOS_Time_t)
                     (NeonRTOS_Millis() - last_progress) >=
                 SPI_MASTER_OP_TIMEOUT)
        {
            SPI_Recover(index);
            return hwSPI_SlaveTimeout;
        }
    }

    while (DL_SPI_isBusy(spi))
    {
        if ((NeonRTOS_Time_t)
                (NeonRTOS_Millis() - last_progress) >=
            SPI_MASTER_OP_TIMEOUT)
        {
            SPI_Recover(index);
            return hwSPI_SlaveTimeout;
        }
    }

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Init(
    hwSPI_Index index,
    uint32_t clock_rate_hz,
    hwSPI_OpMode opMode,
    bool cs)
{
    if ((index >= hwSPI_Index_MAX) ||
        (opMode >= hwSPI_OpMode_MAX) ||
        (clock_rate_hz == 0U))
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index])
    {
        return hwSPI_OK;
    }

    TIMSPM0_SPI_Regs *spi = SPI_Map_Soc_Base(index);
    TIMSPM0_SPI_Clock test_clock;

    if ((spi == NULL) ||
        !SPI_Get_Clock_Config(clock_rate_hz, &test_clock))
    {
        return hwSPI_InvalidParameter;
    }

    if (NeonRTOS_LockObjCreate(
            &Spi_Master_Access_Mutex[index]) != NeonRTOS_OK)
    {
        return hwSPI_MemoryError;
    }

    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[index]);

    hwSPI_OpResult result = SPI_Config_Pins(index, cs);

    if (result != hwSPI_OK)
    {
        NeonRTOS_LockObjDelete(
            &Spi_Master_Access_Mutex[index]);
        return result;
    }

    DL_SPI_reset(spi);
    DL_SPI_enablePower(spi);
    DL_Common_delayCycles(SPI_TIMSPM0_POWER_STARTUP_DELAY);

    result = SPI_Config_Module(
        spi, clock_rate_hz, opMode, cs);

    if (result != hwSPI_OK)
    {
        DL_SPI_reset(spi);
        DL_SPI_disablePower(spi);
        SPI_DeConfig_Pins(index, cs);
        NeonRTOS_LockObjDelete(
            &Spi_Master_Access_Mutex[index]);
        return result;
    }

    SPI_Flush_RX(spi);

    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index].mosi_pin;
    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index].miso_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index].cs_pin;

    gpio_pin_init_status[mosi_pin] = true;
    gpio_pin_init_status[miso_pin] = true;
    gpio_pin_init_status[sclk_pin] = true;

    if (cs)
    {
        gpio_pin_init_status[cs_pin] = true;
    }

    Spi_Master_Use_CS[index] = cs;
    Spi_Master_Clock_Hz[index] = clock_rate_hz;
    Spi_Master_Mode[index] = opMode;
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

    TIMSPM0_SPI_Regs *spi = SPI_Map_Soc_Base(index);

    if (spi == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    bool use_cs = Spi_Master_Use_CS[index];
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index].mosi_pin;
    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index].miso_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index].cs_pin;

    Spi_Master_Init_Status[index] = false;

    DL_SPI_disable(spi);
    DL_SPI_reset(spi);
    DL_SPI_disablePower(spi);

    SPI_DeConfig_Pins(index, use_cs);

    gpio_pin_init_status[mosi_pin] = false;
    gpio_pin_init_status[miso_pin] = false;
    gpio_pin_init_status[sclk_pin] = false;

    if (use_cs)
    {
        gpio_pin_init_status[cs_pin] = false;
    }

    Spi_Master_Use_CS[index] = false;
    Spi_Master_Clock_Hz[index] = 0U;

    SPI_MASTER_MUTEX_UNLOCK(index);
    NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Frequency(
    hwSPI_Index index,
    uint32_t clock_rate_hz)
{
    if ((index >= hwSPI_Index_MAX) ||
        (clock_rate_hz == 0U))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    TIMSPM0_SPI_Regs *spi = SPI_Map_Soc_Base(index);

    if (spi == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    TIMSPM0_SPI_Clock test_clock;

    if (!SPI_Get_Clock_Config(clock_rate_hz, &test_clock))
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    hwSPI_OpResult result = SPI_Config_Module(
        spi,
        clock_rate_hz,
        Spi_Master_Mode[index],
        Spi_Master_Use_CS[index]);

    if (result == hwSPI_OK)
    {
        Spi_Master_Clock_Hz[index] = clock_rate_hz;
        SPI_Flush_RX(spi);
    }

    SPI_MASTER_MUTEX_UNLOCK(index);
    return result;
}

hwSPI_OpResult SPI_Change_Mode(
    hwSPI_Index index,
    hwSPI_OpMode opMode)
{
    if ((index >= hwSPI_Index_MAX) ||
        (opMode >= hwSPI_OpMode_MAX))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    TIMSPM0_SPI_Regs *spi = SPI_Map_Soc_Base(index);

    if (spi == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    hwSPI_OpResult result = SPI_Config_Module(
        spi,
        Spi_Master_Clock_Hz[index],
        opMode,
        Spi_Master_Use_CS[index]);

    if (result == hwSPI_OK)
    {
        Spi_Master_Mode[index] = opMode;
        SPI_Flush_RX(spi);
    }

    SPI_MASTER_MUTEX_UNLOCK(index);
    return result;
}

hwSPI_OpResult SPI_Master_WriteByte(
    hwSPI_Index index,
    uint8_t dat)
{
    return SPI_Master_Stream_Write(index, &dat, 1U);
}

hwSPI_OpResult SPI_Master_ReadByte(
    hwSPI_Index index,
    uint8_t *dat)
{
    return SPI_Master_Stream_Read(index, dat, 1U);
}

hwSPI_OpResult SPI_Master_TransferByte(
    hwSPI_Index index,
    uint8_t wr_dat,
    uint8_t *rd_dat)
{
    return SPI_Master_Stream_Transfer(
        index, &wr_dat, rd_dat, 1U);
}

hwSPI_OpResult SPI_Master_DummyByte(hwSPI_Index index)
{
    uint8_t dummy = 0xFFU;
    return SPI_Master_Stream_Write(index, &dummy, 1U);
}

hwSPI_OpResult SPI_Master_DummyBytes(
    hwSPI_Index index,
    uint32_t len)
{
    if ((index >= hwSPI_Index_MAX) || (len == 0U))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);
    hwSPI_OpResult result =
        SPI_Transfer_Locked(index, NULL, NULL, len);
    SPI_MASTER_MUTEX_UNLOCK(index);

    return result;
}

hwSPI_OpResult SPI_Master_Stream_Write(
    hwSPI_Index index,
    const uint8_t *buf,
    uint16_t len)
{
    if ((index >= hwSPI_Index_MAX) ||
        (buf == NULL) ||
        (len == 0U))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);
    hwSPI_OpResult result =
        SPI_Transfer_Locked(index, buf, NULL, len);
    SPI_MASTER_MUTEX_UNLOCK(index);

    return result;
}

hwSPI_OpResult SPI_Master_Stream_Read(
    hwSPI_Index index,
    uint8_t *buf,
    uint16_t len)
{
    if ((index >= hwSPI_Index_MAX) ||
        (buf == NULL) ||
        (len == 0U))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);
    hwSPI_OpResult result =
        SPI_Transfer_Locked(index, NULL, buf, len);
    SPI_MASTER_MUTEX_UNLOCK(index);

    return result;
}

hwSPI_OpResult SPI_Master_Stream_Transfer(
    hwSPI_Index index,
    const uint8_t *tx_buf,
    uint8_t *rx_buf,
    uint16_t len)
{
    if ((index >= hwSPI_Index_MAX) ||
        (tx_buf == NULL) ||
        (rx_buf == NULL) ||
        (len == 0U))
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);
    hwSPI_OpResult result =
        SPI_Transfer_Locked(index, tx_buf, rx_buf, len);
    SPI_MASTER_MUTEX_UNLOCK(index);

    return result;
}

hwSPI_OpResult SPI_Master_Burst_Write(
    hwSPI_Index index,
    uint8_t *buf,
    uint32_t size)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    if ((size != 0U) && (buf == NULL))
    {
        return hwSPI_InvalidParameter;
    }

    if (size == 0U)
    {
        return hwSPI_OK;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);
    hwSPI_OpResult result =
        SPI_Transfer_Locked(index, buf, NULL, size);
    SPI_MASTER_MUTEX_UNLOCK(index);

    return result;
}

hwSPI_OpResult SPI_Master_Burst_Read(
    hwSPI_Index index,
    uint8_t *buf,
    uint32_t size)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (!Spi_Master_Init_Status[index])
    {
        return hwSPI_NotInit;
    }

    if ((size != 0U) && (buf == NULL))
    {
        return hwSPI_InvalidParameter;
    }

    if (size == 0U)
    {
        return hwSPI_OK;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);
    hwSPI_OpResult result =
        SPI_Transfer_Locked(index, NULL, buf, size);
    SPI_MASTER_MUTEX_UNLOCK(index);

    return result;
}

#endif // DEVICE_TIMSPM0