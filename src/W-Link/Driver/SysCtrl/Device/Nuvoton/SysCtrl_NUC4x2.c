#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef NUC472

void SysCtrl_Init()
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /*
     * Enable HXT.
     * NUC472/442 常見外部晶振是 12MHz 或 20MHz，
     * 你的 clk.h / system_NUC472_442.h 要一致。
     */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* 先切到 HXT，避免 PLL 切換時不穩 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HXT, CLK_CLKDIV0_HCLK(1));

    /* Set PLL to 84MHz */
    CLK->PLLCTL |= CLK_PLLCTL_PD_Msk;
    CLK->PLLCTL = CLK_PLLCTL_84MHz_HXT;
    CLK_WaitClockReady(CLK_STATUS_PLLSTB_Msk);

    /* HCLK = PLL = 84MHz */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL, CLK_CLKDIV0_HCLK(1));

    /* Update SystemCoreClock */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();


    /* Enable EBI module clock */
    CLK_EnableModuleClock(EBI_MODULE);

    /* 設定 EBI pins multi-function */
    /* 這裡要依你的板子 schematic 設：
       AD0~AD15 / A0~Ax / nCS / nOE / nWE
    */

#ifdef CONFIG_DEVICE_HAS_EXT_SRAM
    /* 開 EBI Bank0，16-bit SRAM，normal mode，CS active low */
    EBI_Open(
        EBI_BANK0,
        EBI_BUSWIDTH_16BIT,
        EBI_TIMING_NORMAL,
        EBI_SEPARATEMODE_DISABLE,
        EBI_CS_ACTIVE_LOW
    );

    /* 如果不穩，可以把 timing 放慢 */
    EBI_SetBusTiming(
        EBI_BANK0,
        EBI_TIMING_NORMAL,
        1
    );
#endif
}

#endif // DEVICE_NUC4x2