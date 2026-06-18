#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "GPIO/GPIO.h"

#ifdef NUC472

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

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

    /* Enable EBI module clock */
    CLK_EnableModuleClock(EBI_MODULE);

    /* 設定 EBI pins multi-function */
    /* 這裡要依你的板子 schematic 設：
       AD0~AD15 / A0~Ax / nCS / nOE / nWE
    */

#ifdef EXT_SRAM
    /* Configure EBI multi-function pins */

    /* Address lines */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA10MFP_Msk) | SYS_GPA_MFPH_PA10MFP_EBI_A20;
    SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA9MFP_Msk)  | SYS_GPA_MFPH_PA9MFP_EBI_A19;
    SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA8MFP_Msk)  | SYS_GPA_MFPH_PA8MFP_EBI_A18;
    SYS->GPA_MFPL = (SYS->GPA_MFPL & ~SYS_GPA_MFPL_PA7MFP_Msk)  | SYS_GPA_MFPL_PA7MFP_EBI_A17;
    SYS->GPA_MFPL = (SYS->GPA_MFPL & ~SYS_GPA_MFPL_PA6MFP_Msk)  | SYS_GPA_MFPL_PA6MFP_EBI_A16;

    /* AD0 ~ AD15 */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA11MFP_Msk) | SYS_GPA_MFPH_PA11MFP_EBI_AD0;
    SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA12MFP_Msk) | SYS_GPA_MFPH_PA12MFP_EBI_AD1;
    SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA13MFP_Msk) | SYS_GPA_MFPH_PA13MFP_EBI_AD2;
    SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA14MFP_Msk) | SYS_GPA_MFPH_PA14MFP_EBI_AD3;

    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB2MFP_Msk)  | SYS_GPB_MFPL_PB2MFP_EBI_AD4;
    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB3MFP_Msk)  | SYS_GPB_MFPL_PB3MFP_EBI_AD5;
    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB4MFP_Msk)  | SYS_GPB_MFPL_PB4MFP_EBI_AD6;
    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB5MFP_Msk)  | SYS_GPB_MFPL_PB5MFP_EBI_AD7;
    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB6MFP_Msk)  | SYS_GPB_MFPL_PB6MFP_EBI_AD8;
    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB7MFP_Msk)  | SYS_GPB_MFPL_PB7MFP_EBI_AD9;

    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB8MFP_Msk)  | SYS_GPB_MFPH_PB8MFP_EBI_AD10;
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB9MFP_Msk)  | SYS_GPB_MFPH_PB9MFP_EBI_AD11;
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB10MFP_Msk) | SYS_GPB_MFPH_PB10MFP_EBI_AD12;
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB11MFP_Msk) | SYS_GPB_MFPH_PB11MFP_EBI_AD13;
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB12MFP_Msk) | SYS_GPB_MFPH_PB12MFP_EBI_AD14;
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB13MFP_Msk) | SYS_GPB_MFPH_PB13MFP_EBI_AD15;

    /* Control pins */
    SYS->GPE_MFPL = (SYS->GPE_MFPL & ~SYS_GPE_MFPL_PE6MFP_Msk)  | SYS_GPE_MFPL_PE6MFP_EBI_nWR;
    SYS->GPE_MFPL = (SYS->GPE_MFPL & ~SYS_GPE_MFPL_PE7MFP_Msk)  | SYS_GPE_MFPL_PE7MFP_EBI_nRD;
    SYS->GPE_MFPH = (SYS->GPE_MFPH & ~SYS_GPE_MFPH_PE8MFP_Msk)  | SYS_GPE_MFPH_PE8MFP_EBI_ALE;
    SYS->GPE_MFPH = (SYS->GPE_MFPH & ~SYS_GPE_MFPH_PE9MFP_Msk)  | SYS_GPE_MFPH_PE9MFP_EBI_nWRH;
    SYS->GPE_MFPH = (SYS->GPE_MFPH & ~SYS_GPE_MFPH_PE10MFP_Msk) | SYS_GPE_MFPH_PE10MFP_EBI_nWRL;
    SYS->GPE_MFPH = (SYS->GPE_MFPH & ~SYS_GPE_MFPH_PE11MFP_Msk) | SYS_GPE_MFPH_PE11MFP_EBI_nCS0;

    gpio_pin_init_status[hwGPIO_Pin_A6]  = true;   // A16
    gpio_pin_init_status[hwGPIO_Pin_A7]  = true;   // A17
    gpio_pin_init_status[hwGPIO_Pin_A8]  = true;   // A18
    gpio_pin_init_status[hwGPIO_Pin_A9]  = true;   // A19
    gpio_pin_init_status[hwGPIO_Pin_A10] = true;   // A20

    gpio_pin_init_status[hwGPIO_Pin_A11] = true;   // AD0
    gpio_pin_init_status[hwGPIO_Pin_A12] = true;   // AD1
    gpio_pin_init_status[hwGPIO_Pin_A13] = true;   // AD2
    gpio_pin_init_status[hwGPIO_Pin_A14] = true;   // AD3

    gpio_pin_init_status[hwGPIO_Pin_B2]  = true;   // AD4
    gpio_pin_init_status[hwGPIO_Pin_B3]  = true;   // AD5
    gpio_pin_init_status[hwGPIO_Pin_B4]  = true;   // AD6
    gpio_pin_init_status[hwGPIO_Pin_B5]  = true;   // AD7
    gpio_pin_init_status[hwGPIO_Pin_B6]  = true;   // AD8
    gpio_pin_init_status[hwGPIO_Pin_B7]  = true;   // AD9
    gpio_pin_init_status[hwGPIO_Pin_B8]  = true;   // AD10
    gpio_pin_init_status[hwGPIO_Pin_B9]  = true;   // AD11
    gpio_pin_init_status[hwGPIO_Pin_B10] = true;   // AD12
    gpio_pin_init_status[hwGPIO_Pin_B11] = true;   // AD13
    gpio_pin_init_status[hwGPIO_Pin_B12] = true;   // AD14
    gpio_pin_init_status[hwGPIO_Pin_B13] = true;   // AD15

    gpio_pin_init_status[hwGPIO_Pin_E6]  = true;   // nWR
    gpio_pin_init_status[hwGPIO_Pin_E7]  = true;   // nRD
    gpio_pin_init_status[hwGPIO_Pin_E8]  = true;   // ALE
    gpio_pin_init_status[hwGPIO_Pin_E9]  = true;   // nWRH
    gpio_pin_init_status[hwGPIO_Pin_E10] = true;   // nWRL
    gpio_pin_init_status[hwGPIO_Pin_E11] = true;   // nCS0

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
        0x21C,
        EBI_MCLKDIV_2
    );
#endif

    SYS_LockReg();
}

#endif // DEVICE_NUC4x2