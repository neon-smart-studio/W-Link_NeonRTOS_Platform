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

#ifdef EXT_SRAM
    /* Enable EBI module clock */
    CLK_EnableModuleClock(EBI_MODULE);

    /* 設定 EBI pins multi-function */
    /* 這裡要依你的板子 schematic 設：
       AD0~AD15 / A0~Ax / nCS / nOE / nWE
    */
    /* Configure EBI multi-function pins */
/* Configure EBI multi-function pins */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA10MFP_Msk)) | SYS_GPA_MFPH_PA10MFP_EBI_A20;     /* A20. =   PA10 */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA9MFP_Msk)) | SYS_GPA_MFPH_PA9MFP_EBI_A19;       /* A19. =   PA9 */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA8MFP_Msk)) | SYS_GPA_MFPH_PA8MFP_EBI_A18;       /* A18. =   PA8 */
    SYS->GPA_MFPL = (SYS->GPA_MFPL & (~SYS_GPA_MFPL_PA7MFP_Msk)) | SYS_GPA_MFPL_PA7MFP_EBI_A17;       /* A17. =   PA7 */
    SYS->GPA_MFPL = (SYS->GPA_MFPL & (~SYS_GPA_MFPL_PA6MFP_Msk)) | SYS_GPA_MFPL_PA6MFP_EBI_A16;       /* A16. =   PA6 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB13MFP_Msk)) | SYS_GPB_MFPH_PB13MFP_EBI_AD15;    /* AD15 =   PB13 */

    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB12MFP_Msk)) | SYS_GPB_MFPH_PB12MFP_EBI_AD14;    /* AD14 =   PB12 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB11MFP_Msk)) | SYS_GPB_MFPH_PB11MFP_EBI_AD13;    /* AD13 =   PB11 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB10MFP_Msk)) | SYS_GPB_MFPH_PB10MFP_EBI_AD12;    /* AD12 =   PB10 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB9MFP_Msk)) | SYS_GPB_MFPH_PB9MFP_EBI_AD11;      /* AD11 =   PB9 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB8MFP_Msk)) | SYS_GPB_MFPH_PB8MFP_EBI_AD10;      /* AD10 =   PB8 */

    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB7MFP_Msk)) | SYS_GPB_MFPL_PB7MFP_EBI_AD9;       /* AD9 =    PB7 */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB6MFP_Msk)) | SYS_GPB_MFPL_PB6MFP_EBI_AD8;       /* AD8 =    PB6 */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB5MFP_Msk)) | SYS_GPB_MFPL_PB5MFP_EBI_AD7;       /* AD7 =    PB5 */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB4MFP_Msk)) | SYS_GPB_MFPL_PB4MFP_EBI_AD6;       /* AD6 =    PB4 */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB3MFP_Msk)) | SYS_GPB_MFPL_PB3MFP_EBI_AD5;       /* AD5 =    PB3 */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB2MFP_Msk)) | SYS_GPB_MFPL_PB2MFP_EBI_AD4;       /* AD4 =    PB2 */

    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA14MFP_Msk)) | SYS_GPA_MFPH_PA14MFP_EBI_AD3;     /* AD3. =   PA14 */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA13MFP_Msk)) | SYS_GPA_MFPH_PA13MFP_EBI_AD2;     /* AD2. =   PA13 */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA12MFP_Msk)) | SYS_GPA_MFPH_PA12MFP_EBI_AD1;     /* AD1. =   PA12 */
    SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA11MFP_Msk)) | SYS_GPA_MFPH_PA11MFP_EBI_AD0;     /* AD0. =   PA11 */

    SYS->GPE_MFPL = (SYS->GPE_MFPL & (~SYS_GPE_MFPL_PE6MFP_Msk)) | SYS_GPE_MFPL_PE6MFP_EBI_nWR;       /* PE.6 =   nWR */
    SYS->GPE_MFPL = (SYS->GPE_MFPL & (~SYS_GPE_MFPL_PE7MFP_Msk)) | SYS_GPE_MFPL_PE7MFP_EBI_nRD;       /* PE.7 =   nRD */
    SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE8MFP_Msk)) | SYS_GPE_MFPH_PE8MFP_EBI_ALE;       /* PE.8 =   ALE */
    SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE9MFP_Msk)) | SYS_GPE_MFPH_PE9MFP_EBI_nWRH;      /* PE.9 =   WRH */
    SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE10MFP_Msk)) | SYS_GPE_MFPH_PE10MFP_EBI_nWRL;    /* PE.10 =  WRL */

    SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE11MFP_Msk)) | SYS_GPE_MFPH_PE11MFP_EBI_nCS0;    /* PE.11 = nCS0 */
    SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE12MFP_Msk)) | SYS_GPE_MFPH_PE12MFP_EBI_nCS1;    /* PE.12 = nCS1 */
    SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE13MFP_Msk)) | SYS_GPE_MFPH_PE13MFP_EBI_nCS2;    /* PE.13 = nCS2 */
    SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE14MFP_Msk)) | SYS_GPE_MFPH_PE14MFP_EBI_nCS3;    /* PE.14 = nCS3 */


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
    gpio_pin_init_status[hwGPIO_Pin_E12] = true;   // nCS1
    gpio_pin_init_status[hwGPIO_Pin_E13] = true;   // nCS2
    gpio_pin_init_status[hwGPIO_Pin_E14] = true;   // nCS3
#endif

    SYS_LockReg();

#ifdef EXT_SRAM
    /* 開 EBI Bank0，16-bit SRAM，normal mode，CS active low */
    const uint32_t u32Timing = 0x21C;

    /* Open EBI interface */
    EBI_Open(EBI_BANK0, EBI_BUSWIDTH_16BIT, EBI_TIMING_NORMAL, EBI_SEPARATEMODE_DISABLE, EBI_CS_ACTIVE_LOW);
    EBI_Open(EBI_BANK1, EBI_BUSWIDTH_16BIT, EBI_TIMING_NORMAL, EBI_SEPARATEMODE_DISABLE, EBI_CS_ACTIVE_LOW);
    EBI_Open(EBI_BANK2, EBI_BUSWIDTH_16BIT, EBI_TIMING_NORMAL, EBI_SEPARATEMODE_DISABLE, EBI_CS_ACTIVE_LOW);
    EBI_Open(EBI_BANK3, EBI_BUSWIDTH_16BIT, EBI_TIMING_NORMAL, EBI_SEPARATEMODE_DISABLE, EBI_CS_ACTIVE_LOW);

    /* Configure EBI timing */
    EBI_SetBusTiming(EBI_BANK0, u32Timing, EBI_MCLKDIV_2);
    EBI_SetBusTiming(EBI_BANK1, u32Timing, EBI_MCLKDIV_2);
    EBI_SetBusTiming(EBI_BANK2, u32Timing, EBI_MCLKDIV_2);
    EBI_SetBusTiming(EBI_BANK3, u32Timing, EBI_MCLKDIV_2);
#endif
}

#endif // DEVICE_NUC4x2