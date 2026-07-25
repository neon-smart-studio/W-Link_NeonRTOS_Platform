#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef DEVICE_TIMSPM0

/*
 * 目前 MSPM0 SDK 2.11 的所有 G 系列 DeviceFamily parent。
 */
#define TIMSPM0_IS_G_SERIES                                           \
    ((DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G1X0X_G3X0X) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0GX51X)       || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G352X)       || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G511X)       || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G518X)       || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0GX218_GX207))

/*
 * MSPM0C110x、MSPM0C1105/C1106，以及 C031Cx 相容系列。
 */
#define TIMSPM0_IS_C_SERIES                                           \
    ((DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0C110X)       || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0C1105_C1106))

#if TIMSPM0_IS_C_SERIES
#define TIMSPM0_SYSOSC_BASE_HZ    24000000UL
#else
#define TIMSPM0_SYSOSC_BASE_HZ    32000000UL
#endif

#if TIMSPM0_IS_G_SERIES
#define TIMSPM0_CPU_CLOCK_HZ      80000000UL
#define TIMSPM0_BUS_CLOCK_HZ      40000000UL
#else
#define TIMSPM0_CPU_CLOCK_HZ      TIMSPM0_SYSOSC_BASE_HZ
#define TIMSPM0_BUS_CLOCK_HZ      TIMSPM0_SYSOSC_BASE_HZ
#endif

/*
 * 初始化前維持 reset 後的 SYSOSC 頻率；
 * SysCtrl_Init() 完成後更新成實際 CPUCLK。
 */
uint32_t g_sys_clock_hz = TIMSPM0_SYSOSC_BASE_HZ;

#if TIMSPM0_IS_G_SERIES

/*
 * SYSOSC = 32 MHz
 * PLL reference = 32 MHz / 2
 * VCO           = 16 MHz * 10 = 160 MHz
 * CLK0          = 160 MHz / 2 = 80 MHz
 */
static const DL_SYSCTL_SYSPLLConfig g_sys_pll_config =
{
    .inputFreq   = DL_SYSCTL_SYSPLL_INPUT_FREQ_16_32_MHZ,

    .rDivClk2x   = 1,
    .rDivClk1    = 0,
    .rDivClk0    = 0,

    .enableCLK2x = DL_SYSCTL_SYSPLL_CLK2X_DISABLE,
    .enableCLK1  = DL_SYSCTL_SYSPLL_CLK1_DISABLE,
    .enableCLK0  = DL_SYSCTL_SYSPLL_CLK0_ENABLE,

    .sysPLLMCLK  = DL_SYSCTL_SYSPLL_MCLK_CLK0,
    .sysPLLRef   = DL_SYSCTL_SYSPLL_REF_SYSOSC,

    /* Register 9 represents multiplier ×10. */
    .qDiv        = 9,
    .pDiv        = DL_SYSCTL_SYSPLL_PDIV_2
};

#endif

void SysCtrl_Init(void)
{
    __disable_irq();

    /*
     * MDIV 必須先停用，才能安全修改 SYSOSC。
     * 此函式應在 reset 後、其他周邊初始化前呼叫一次。
     */
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);

    while (DL_SYSCTL_getCurrentSYSOSCFreq() !=
           DL_SYSCTL_SYSOSC_FREQ_BASE)
    {
    }

#if TIMSPM0_IS_G_SERIES

    /*
     * 80 MHz 必須先配置 Flash wait state。
     * DL_SYSCTL_configSYSPLL() 會啟動並等待 PLL 穩定。
     */
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2);

    DL_SYSCTL_configSYSPLL(&g_sys_pll_config);

    /*
     * G 系列 CPUCLK/MCLK = 80 MHz；
     * ULPCLK/BUSCLK 除以 2，限制在 40 MHz。
     */
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);

    DL_SYSCTL_switchMCLKfromSYSOSCtoHSCLK(
        DL_SYSCTL_HSCLK_SOURCE_SYSPLL);

#endif

    g_sys_clock_hz = TIMSPM0_CPU_CLOCK_HZ;

    __enable_irq();
}

#endif /* DEVICE_TIMSPM0 */