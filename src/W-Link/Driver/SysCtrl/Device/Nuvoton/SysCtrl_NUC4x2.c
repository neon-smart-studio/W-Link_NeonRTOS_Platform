#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef DEVICE_NUC4x2

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
}

#endif // DEVICE_NUC4x2