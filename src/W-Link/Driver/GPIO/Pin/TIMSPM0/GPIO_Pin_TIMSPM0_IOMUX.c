#include <stdbool.h>
#include <stdint.h>

#include "GPIO_Pin_TIMSPM0.h"

#include "soc.h"

#if defined(MSPM0C110x)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0: return IOMUX_PINCM1;
        case hwGPIO_Pin_A1: return IOMUX_PINCM2;
        case hwGPIO_Pin_A2: return IOMUX_PINCM3;
        case hwGPIO_Pin_A4: return IOMUX_PINCM5;
        case hwGPIO_Pin_A6: return IOMUX_PINCM7;
        case hwGPIO_Pin_A11: return IOMUX_PINCM12;
        case hwGPIO_Pin_A16: return IOMUX_PINCM17;
        case hwGPIO_Pin_A17: return IOMUX_PINCM18;
        case hwGPIO_Pin_A18: return IOMUX_PINCM19;
        case hwGPIO_Pin_A19: return IOMUX_PINCM20;
        case hwGPIO_Pin_A20: return IOMUX_PINCM21;
        case hwGPIO_Pin_A22: return IOMUX_PINCM23;
        case hwGPIO_Pin_A23: return IOMUX_PINCM24;
        case hwGPIO_Pin_A24: return IOMUX_PINCM25;
        case hwGPIO_Pin_A25: return IOMUX_PINCM26;
        case hwGPIO_Pin_A26: return IOMUX_PINCM27;
        case hwGPIO_Pin_A27: return IOMUX_PINCM28;
        case hwGPIO_Pin_A28: return IOMUX_PINCM29;

        /*
         * MSPM0C110x 不存在：
         * PA3、PA5、PA7～PA10、PA12～PA15、
         * PA21、PA29～PA31，以及全部 GPIOB/GPIOC。
         */
        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0C110x */

#if defined(MSPM0C1105) || defined(MSPM0C1106)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0:  return IOMUX_PINCM1;
        case hwGPIO_Pin_A1:  return IOMUX_PINCM2;
        case hwGPIO_Pin_A2:  return IOMUX_PINCM5;
        case hwGPIO_Pin_A3:  return IOMUX_PINCM6;
        case hwGPIO_Pin_A4:  return IOMUX_PINCM7;
        case hwGPIO_Pin_A5:  return IOMUX_PINCM8;
        case hwGPIO_Pin_A6:  return IOMUX_PINCM9;
        case hwGPIO_Pin_A7:  return IOMUX_PINCM10;
        case hwGPIO_Pin_A8:  return IOMUX_PINCM13;
        case hwGPIO_Pin_A9:  return IOMUX_PINCM14;
        case hwGPIO_Pin_A10: return IOMUX_PINCM15;
        case hwGPIO_Pin_A11: return IOMUX_PINCM16;

        case hwGPIO_Pin_A12: return IOMUX_PINCM24;
        case hwGPIO_Pin_A13: return IOMUX_PINCM25;
        case hwGPIO_Pin_A14: return IOMUX_PINCM26;
        case hwGPIO_Pin_A15: return IOMUX_PINCM27;
        case hwGPIO_Pin_A16: return IOMUX_PINCM28;
        case hwGPIO_Pin_A17: return IOMUX_PINCM29;
        case hwGPIO_Pin_A18: return IOMUX_PINCM30;
        case hwGPIO_Pin_A19: return IOMUX_PINCM32;
        case hwGPIO_Pin_A20: return IOMUX_PINCM33;

        case hwGPIO_Pin_A21: return IOMUX_PINCM37;
        case hwGPIO_Pin_A22: return IOMUX_PINCM38;
        case hwGPIO_Pin_A23: return IOMUX_PINCM41;
        case hwGPIO_Pin_A24: return IOMUX_PINCM42;
        case hwGPIO_Pin_A25: return IOMUX_PINCM43;
        case hwGPIO_Pin_A26: return IOMUX_PINCM44;
        case hwGPIO_Pin_A27: return IOMUX_PINCM45;

        case hwGPIO_Pin_A28: return IOMUX_PINCM3;
        case hwGPIO_Pin_A29: return IOMUX_PINCM31;
        case hwGPIO_Pin_A30: return IOMUX_PINCM46;
        case hwGPIO_Pin_A31: return IOMUX_PINCM4;

        /* GPIOB */
        case hwGPIO_Pin_B2:  return IOMUX_PINCM11;
        case hwGPIO_Pin_B3:  return IOMUX_PINCM12;

        case hwGPIO_Pin_B6:  return IOMUX_PINCM17;
        case hwGPIO_Pin_B7:  return IOMUX_PINCM18;
        case hwGPIO_Pin_B8:  return IOMUX_PINCM19;
        case hwGPIO_Pin_B9:  return IOMUX_PINCM20;

        case hwGPIO_Pin_B14: return IOMUX_PINCM21;
        case hwGPIO_Pin_B15: return IOMUX_PINCM22;
        case hwGPIO_Pin_B16: return IOMUX_PINCM23;

        case hwGPIO_Pin_B17: return IOMUX_PINCM34;
        case hwGPIO_Pin_B18: return IOMUX_PINCM35;
        case hwGPIO_Pin_B19: return IOMUX_PINCM36;

        case hwGPIO_Pin_B20: return IOMUX_PINCM39;
        case hwGPIO_Pin_B24: return IOMUX_PINCM40;

        /*
         * MSPM0C1105 / MSPM0C1106 不存在：
         *
         * PB0、PB1、PB4、PB5、
         * PB10～PB13、PB21～PB23、PB25～PB31，
         * 以及全部 GPIOC。
         *
         * 個別封裝不一定會引出以上所有有效 GPIO。
         */
        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0C1105 || MSPM0C1106 */

#if defined(MSPM0G110x) || \
    defined(MSPM0G120x) || \
    defined(MSPM0G121x) || \
    defined(MSPM0G150x) || \
    defined(MSPM0G310x) || \
    defined(MSPM0G320x) || \
    defined(MSPM0G321x) || \
    defined(MSPM0G350x)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0:  return IOMUX_PINCM1;
        case hwGPIO_Pin_A1:  return IOMUX_PINCM2;
        case hwGPIO_Pin_A2:  return IOMUX_PINCM7;
        case hwGPIO_Pin_A3:  return IOMUX_PINCM8;
        case hwGPIO_Pin_A4:  return IOMUX_PINCM9;
        case hwGPIO_Pin_A5:  return IOMUX_PINCM10;
        case hwGPIO_Pin_A6:  return IOMUX_PINCM11;
        case hwGPIO_Pin_A7:  return IOMUX_PINCM14;
        case hwGPIO_Pin_A8:  return IOMUX_PINCM19;
        case hwGPIO_Pin_A9:  return IOMUX_PINCM20;
        case hwGPIO_Pin_A10: return IOMUX_PINCM21;
        case hwGPIO_Pin_A11: return IOMUX_PINCM22;
        case hwGPIO_Pin_A12: return IOMUX_PINCM34;
        case hwGPIO_Pin_A13: return IOMUX_PINCM35;
        case hwGPIO_Pin_A14: return IOMUX_PINCM36;
        case hwGPIO_Pin_A15: return IOMUX_PINCM37;
        case hwGPIO_Pin_A16: return IOMUX_PINCM38;
        case hwGPIO_Pin_A17: return IOMUX_PINCM39;
        case hwGPIO_Pin_A18: return IOMUX_PINCM40;
        case hwGPIO_Pin_A19: return IOMUX_PINCM41;
        case hwGPIO_Pin_A20: return IOMUX_PINCM42;
        case hwGPIO_Pin_A21: return IOMUX_PINCM46;
        case hwGPIO_Pin_A22: return IOMUX_PINCM47;
        case hwGPIO_Pin_A23: return IOMUX_PINCM53;
        case hwGPIO_Pin_A24: return IOMUX_PINCM54;
        case hwGPIO_Pin_A25: return IOMUX_PINCM55;
        case hwGPIO_Pin_A26: return IOMUX_PINCM59;
        case hwGPIO_Pin_A27: return IOMUX_PINCM60;
        case hwGPIO_Pin_A28: return IOMUX_PINCM3;
        case hwGPIO_Pin_A29: return IOMUX_PINCM4;
        case hwGPIO_Pin_A30: return IOMUX_PINCM5;
        case hwGPIO_Pin_A31: return IOMUX_PINCM6;

        /* GPIOB */
        case hwGPIO_Pin_B0:  return IOMUX_PINCM12;
        case hwGPIO_Pin_B1:  return IOMUX_PINCM13;
        case hwGPIO_Pin_B2:  return IOMUX_PINCM15;
        case hwGPIO_Pin_B3:  return IOMUX_PINCM16;
        case hwGPIO_Pin_B4:  return IOMUX_PINCM17;
        case hwGPIO_Pin_B5:  return IOMUX_PINCM18;
        case hwGPIO_Pin_B6:  return IOMUX_PINCM23;
        case hwGPIO_Pin_B7:  return IOMUX_PINCM24;
        case hwGPIO_Pin_B8:  return IOMUX_PINCM25;
        case hwGPIO_Pin_B9:  return IOMUX_PINCM26;
        case hwGPIO_Pin_B10: return IOMUX_PINCM27;
        case hwGPIO_Pin_B11: return IOMUX_PINCM28;
        case hwGPIO_Pin_B12: return IOMUX_PINCM29;
        case hwGPIO_Pin_B13: return IOMUX_PINCM30;
        case hwGPIO_Pin_B14: return IOMUX_PINCM31;
        case hwGPIO_Pin_B15: return IOMUX_PINCM32;
        case hwGPIO_Pin_B16: return IOMUX_PINCM33;
        case hwGPIO_Pin_B17: return IOMUX_PINCM43;
        case hwGPIO_Pin_B18: return IOMUX_PINCM44;
        case hwGPIO_Pin_B19: return IOMUX_PINCM45;
        case hwGPIO_Pin_B20: return IOMUX_PINCM48;
        case hwGPIO_Pin_B21: return IOMUX_PINCM49;
        case hwGPIO_Pin_B22: return IOMUX_PINCM50;
        case hwGPIO_Pin_B23: return IOMUX_PINCM51;
        case hwGPIO_Pin_B24: return IOMUX_PINCM52;
        case hwGPIO_Pin_B25: return IOMUX_PINCM56;
        case hwGPIO_Pin_B26: return IOMUX_PINCM57;
        case hwGPIO_Pin_B27: return IOMUX_PINCM58;

        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0G110x || MSPM0G120x || MSPM0G121x || MSPM0G150x || 
          MSPM0G310x || MSPM0G320x || MSPM0G321x || MSPM0G350x || */

#if defined(MSPM0G151x) || \
    defined(MSPM0G351x) || \
    defined(MSPM0G352x)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0:  return IOMUX_PINCM1;
        case hwGPIO_Pin_A1:  return IOMUX_PINCM2;
        case hwGPIO_Pin_A2:  return IOMUX_PINCM7;
        case hwGPIO_Pin_A3:  return IOMUX_PINCM8;
        case hwGPIO_Pin_A4:  return IOMUX_PINCM9;
        case hwGPIO_Pin_A5:  return IOMUX_PINCM10;
        case hwGPIO_Pin_A6:  return IOMUX_PINCM11;
        case hwGPIO_Pin_A7:  return IOMUX_PINCM14;
        case hwGPIO_Pin_A8:  return IOMUX_PINCM19;
        case hwGPIO_Pin_A9:  return IOMUX_PINCM20;
        case hwGPIO_Pin_A10: return IOMUX_PINCM21;
        case hwGPIO_Pin_A11: return IOMUX_PINCM22;
        case hwGPIO_Pin_A12: return IOMUX_PINCM34;
        case hwGPIO_Pin_A13: return IOMUX_PINCM35;
        case hwGPIO_Pin_A14: return IOMUX_PINCM36;
        case hwGPIO_Pin_A15: return IOMUX_PINCM37;
        case hwGPIO_Pin_A16: return IOMUX_PINCM38;
        case hwGPIO_Pin_A17: return IOMUX_PINCM39;
        case hwGPIO_Pin_A18: return IOMUX_PINCM40;
        case hwGPIO_Pin_A19: return IOMUX_PINCM41;
        case hwGPIO_Pin_A20: return IOMUX_PINCM42;
        case hwGPIO_Pin_A21: return IOMUX_PINCM46;
        case hwGPIO_Pin_A22: return IOMUX_PINCM47;
        case hwGPIO_Pin_A23: return IOMUX_PINCM53;
        case hwGPIO_Pin_A24: return IOMUX_PINCM54;
        case hwGPIO_Pin_A25: return IOMUX_PINCM55;
        case hwGPIO_Pin_A26: return IOMUX_PINCM59;
        case hwGPIO_Pin_A27: return IOMUX_PINCM60;
        case hwGPIO_Pin_A28: return IOMUX_PINCM3;
        case hwGPIO_Pin_A29: return IOMUX_PINCM4;
        case hwGPIO_Pin_A30: return IOMUX_PINCM5;
        case hwGPIO_Pin_A31: return IOMUX_PINCM6;

        /* GPIOB */
        case hwGPIO_Pin_B0:  return IOMUX_PINCM12;
        case hwGPIO_Pin_B1:  return IOMUX_PINCM13;
        case hwGPIO_Pin_B2:  return IOMUX_PINCM15;
        case hwGPIO_Pin_B3:  return IOMUX_PINCM16;
        case hwGPIO_Pin_B4:  return IOMUX_PINCM17;
        case hwGPIO_Pin_B5:  return IOMUX_PINCM18;
        case hwGPIO_Pin_B6:  return IOMUX_PINCM23;
        case hwGPIO_Pin_B7:  return IOMUX_PINCM24;
        case hwGPIO_Pin_B8:  return IOMUX_PINCM25;
        case hwGPIO_Pin_B9:  return IOMUX_PINCM26;
        case hwGPIO_Pin_B10: return IOMUX_PINCM27;
        case hwGPIO_Pin_B11: return IOMUX_PINCM28;
        case hwGPIO_Pin_B12: return IOMUX_PINCM29;
        case hwGPIO_Pin_B13: return IOMUX_PINCM30;
        case hwGPIO_Pin_B14: return IOMUX_PINCM31;
        case hwGPIO_Pin_B15: return IOMUX_PINCM32;
        case hwGPIO_Pin_B16: return IOMUX_PINCM33;
        case hwGPIO_Pin_B17: return IOMUX_PINCM43;
        case hwGPIO_Pin_B18: return IOMUX_PINCM44;
        case hwGPIO_Pin_B19: return IOMUX_PINCM45;
        case hwGPIO_Pin_B20: return IOMUX_PINCM48;
        case hwGPIO_Pin_B21: return IOMUX_PINCM49;
        case hwGPIO_Pin_B22: return IOMUX_PINCM50;
        case hwGPIO_Pin_B23: return IOMUX_PINCM51;
        case hwGPIO_Pin_B24: return IOMUX_PINCM52;
        case hwGPIO_Pin_B25: return IOMUX_PINCM56;
        case hwGPIO_Pin_B26: return IOMUX_PINCM57;
        case hwGPIO_Pin_B27: return IOMUX_PINCM58;
        case hwGPIO_Pin_B28: return IOMUX_PINCM65;
        case hwGPIO_Pin_B29: return IOMUX_PINCM66;
        case hwGPIO_Pin_B30: return IOMUX_PINCM67;
        case hwGPIO_Pin_B31: return IOMUX_PINCM68;

        /* GPIOC */
        case hwGPIO_Pin_C0:  return IOMUX_PINCM74;
        case hwGPIO_Pin_C1:  return IOMUX_PINCM75;
        case hwGPIO_Pin_C2:  return IOMUX_PINCM76;
        case hwGPIO_Pin_C3:  return IOMUX_PINCM77;
        case hwGPIO_Pin_C4:  return IOMUX_PINCM78;
        case hwGPIO_Pin_C5:  return IOMUX_PINCM79;
        case hwGPIO_Pin_C6:  return IOMUX_PINCM84;
        case hwGPIO_Pin_C7:  return IOMUX_PINCM85;
        case hwGPIO_Pin_C8:  return IOMUX_PINCM86;
        case hwGPIO_Pin_C9:  return IOMUX_PINCM87;
        case hwGPIO_Pin_C10: return IOMUX_PINCM88;
        case hwGPIO_Pin_C11: return IOMUX_PINCM89;
        case hwGPIO_Pin_C12: return IOMUX_PINCM61;
        case hwGPIO_Pin_C13: return IOMUX_PINCM62;
        case hwGPIO_Pin_C14: return IOMUX_PINCM63;
        case hwGPIO_Pin_C15: return IOMUX_PINCM64;
        case hwGPIO_Pin_C16: return IOMUX_PINCM69;
        case hwGPIO_Pin_C17: return IOMUX_PINCM70;
        case hwGPIO_Pin_C18: return IOMUX_PINCM71;
        case hwGPIO_Pin_C19: return IOMUX_PINCM72;
        case hwGPIO_Pin_C20: return IOMUX_PINCM73;
        case hwGPIO_Pin_C21: return IOMUX_PINCM80;
        case hwGPIO_Pin_C22: return IOMUX_PINCM81;
        case hwGPIO_Pin_C23: return IOMUX_PINCM82;
        case hwGPIO_Pin_C24: return IOMUX_PINCM83;
        case hwGPIO_Pin_C25: return IOMUX_PINCM90;
        case hwGPIO_Pin_C26: return IOMUX_PINCM91;
        case hwGPIO_Pin_C27: return IOMUX_PINCM92;
        case hwGPIO_Pin_C28: return IOMUX_PINCM93;
        case hwGPIO_Pin_C29: return IOMUX_PINCM94;

        case hwGPIO_Pin_C30:
        case hwGPIO_Pin_C31:
        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0G151x || MSPM0G351x || MSPM0G352x */

#if defined(MSPM0G511x) || \
    defined(MSPM0G518x)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0:  return IOMUX_PINCM1;
        case hwGPIO_Pin_A1:  return IOMUX_PINCM2;
        case hwGPIO_Pin_A2:  return IOMUX_PINCM7;
        case hwGPIO_Pin_A3:  return IOMUX_PINCM8;
        case hwGPIO_Pin_A4:  return IOMUX_PINCM9;
        case hwGPIO_Pin_A5:  return IOMUX_PINCM10;
        case hwGPIO_Pin_A6:  return IOMUX_PINCM11;
        case hwGPIO_Pin_A7:  return IOMUX_PINCM14;
        case hwGPIO_Pin_A8:  return IOMUX_PINCM19;
        case hwGPIO_Pin_A9:  return IOMUX_PINCM20;
        case hwGPIO_Pin_A10: return IOMUX_PINCM21;
        case hwGPIO_Pin_A11: return IOMUX_PINCM22;
        case hwGPIO_Pin_A12: return IOMUX_PINCM34;
        case hwGPIO_Pin_A13: return IOMUX_PINCM35;
        case hwGPIO_Pin_A14: return IOMUX_PINCM36;
        case hwGPIO_Pin_A15: return IOMUX_PINCM37;
        case hwGPIO_Pin_A16: return IOMUX_PINCM38;
        case hwGPIO_Pin_A17: return IOMUX_PINCM39;
        case hwGPIO_Pin_A18: return IOMUX_PINCM40;
        case hwGPIO_Pin_A19: return IOMUX_PINCM41;
        case hwGPIO_Pin_A20: return IOMUX_PINCM42;
        case hwGPIO_Pin_A21: return IOMUX_PINCM46;
        case hwGPIO_Pin_A22: return IOMUX_PINCM47;
        case hwGPIO_Pin_A23: return IOMUX_PINCM53;
        case hwGPIO_Pin_A24: return IOMUX_PINCM54;
        case hwGPIO_Pin_A25: return IOMUX_PINCM55;
        case hwGPIO_Pin_A26: return IOMUX_PINCM58;
        case hwGPIO_Pin_A27: return IOMUX_PINCM59;
        case hwGPIO_Pin_A28: return IOMUX_PINCM3;
        case hwGPIO_Pin_A29: return IOMUX_PINCM4;
        case hwGPIO_Pin_A30: return IOMUX_PINCM5;
        case hwGPIO_Pin_A31: return IOMUX_PINCM6;

        /* GPIOB */
        case hwGPIO_Pin_B0:  return IOMUX_PINCM12;
        case hwGPIO_Pin_B1:  return IOMUX_PINCM13;
        case hwGPIO_Pin_B2:  return IOMUX_PINCM15;
        case hwGPIO_Pin_B3:  return IOMUX_PINCM16;
        case hwGPIO_Pin_B4:  return IOMUX_PINCM17;
        case hwGPIO_Pin_B5:  return IOMUX_PINCM18;
        case hwGPIO_Pin_B6:  return IOMUX_PINCM23;
        case hwGPIO_Pin_B7:  return IOMUX_PINCM24;
        case hwGPIO_Pin_B8:  return IOMUX_PINCM25;
        case hwGPIO_Pin_B9:  return IOMUX_PINCM26;
        case hwGPIO_Pin_B10: return IOMUX_PINCM27;
        case hwGPIO_Pin_B11: return IOMUX_PINCM28;
        case hwGPIO_Pin_B12: return IOMUX_PINCM29;
        case hwGPIO_Pin_B13: return IOMUX_PINCM30;
        case hwGPIO_Pin_B14: return IOMUX_PINCM31;
        case hwGPIO_Pin_B15: return IOMUX_PINCM32;
        case hwGPIO_Pin_B16: return IOMUX_PINCM33;
        case hwGPIO_Pin_B17: return IOMUX_PINCM43;
        case hwGPIO_Pin_B18: return IOMUX_PINCM44;
        case hwGPIO_Pin_B19: return IOMUX_PINCM45;
        case hwGPIO_Pin_B20: return IOMUX_PINCM48;
        case hwGPIO_Pin_B21: return IOMUX_PINCM49;
        case hwGPIO_Pin_B22: return IOMUX_PINCM50;
        case hwGPIO_Pin_B23: return IOMUX_PINCM51;
        case hwGPIO_Pin_B24: return IOMUX_PINCM52;
        case hwGPIO_Pin_B25: return IOMUX_PINCM56;
        case hwGPIO_Pin_B26: return IOMUX_PINCM57;

        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0G511x || MSPM0G518x */

#if defined(MSPM0H321x)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0:  return IOMUX_PINCM1;
        case hwGPIO_Pin_A1:  return IOMUX_PINCM2;
        case hwGPIO_Pin_A2:  return IOMUX_PINCM5;
        case hwGPIO_Pin_A3:  return IOMUX_PINCM6;
        case hwGPIO_Pin_A4:  return IOMUX_PINCM7;
        case hwGPIO_Pin_A5:  return IOMUX_PINCM8;
        case hwGPIO_Pin_A6:  return IOMUX_PINCM9;
        case hwGPIO_Pin_A7:  return IOMUX_PINCM10;
        case hwGPIO_Pin_A8:  return IOMUX_PINCM13;
        case hwGPIO_Pin_A9:  return IOMUX_PINCM14;
        case hwGPIO_Pin_A10: return IOMUX_PINCM15;
        case hwGPIO_Pin_A11: return IOMUX_PINCM16;
        case hwGPIO_Pin_A12: return IOMUX_PINCM24;
        case hwGPIO_Pin_A13: return IOMUX_PINCM25;
        case hwGPIO_Pin_A14: return IOMUX_PINCM26;
        case hwGPIO_Pin_A15: return IOMUX_PINCM27;
        case hwGPIO_Pin_A16: return IOMUX_PINCM28;
        case hwGPIO_Pin_A17: return IOMUX_PINCM29;
        case hwGPIO_Pin_A18: return IOMUX_PINCM30;
        case hwGPIO_Pin_A19: return IOMUX_PINCM31;
        case hwGPIO_Pin_A20: return IOMUX_PINCM32;
        case hwGPIO_Pin_A21: return IOMUX_PINCM36;
        case hwGPIO_Pin_A22: return IOMUX_PINCM37;
        case hwGPIO_Pin_A23: return IOMUX_PINCM40;
        case hwGPIO_Pin_A24: return IOMUX_PINCM41;
        case hwGPIO_Pin_A25: return IOMUX_PINCM42;
        case hwGPIO_Pin_A26: return IOMUX_PINCM43;
        case hwGPIO_Pin_A27: return IOMUX_PINCM44;
        case hwGPIO_Pin_A28: return IOMUX_PINCM3;
        case hwGPIO_Pin_A30: return IOMUX_PINCM45;
        case hwGPIO_Pin_A31: return IOMUX_PINCM4;

        /* GPIOB */
        case hwGPIO_Pin_B2:  return IOMUX_PINCM11;
        case hwGPIO_Pin_B3:  return IOMUX_PINCM12;
        case hwGPIO_Pin_B6:  return IOMUX_PINCM17;
        case hwGPIO_Pin_B7:  return IOMUX_PINCM18;
        case hwGPIO_Pin_B8:  return IOMUX_PINCM19;
        case hwGPIO_Pin_B9:  return IOMUX_PINCM20;
        case hwGPIO_Pin_B14: return IOMUX_PINCM21;
        case hwGPIO_Pin_B15: return IOMUX_PINCM22;
        case hwGPIO_Pin_B16: return IOMUX_PINCM23;
        case hwGPIO_Pin_B17: return IOMUX_PINCM33;
        case hwGPIO_Pin_B18: return IOMUX_PINCM34;
        case hwGPIO_Pin_B19: return IOMUX_PINCM35;
        case hwGPIO_Pin_B20: return IOMUX_PINCM38;
        case hwGPIO_Pin_B24: return IOMUX_PINCM39;

        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0H321x */

#if defined(MSPM0L110x) || \
    defined(MSPM0L130x) || \
    defined(MSPM0L134x)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0:  return IOMUX_PINCM1;
        case hwGPIO_Pin_A1:  return IOMUX_PINCM2;
        case hwGPIO_Pin_A2:  return IOMUX_PINCM3;
        case hwGPIO_Pin_A3:  return IOMUX_PINCM4;
        case hwGPIO_Pin_A4:  return IOMUX_PINCM5;
        case hwGPIO_Pin_A5:  return IOMUX_PINCM6;
        case hwGPIO_Pin_A6:  return IOMUX_PINCM7;
        case hwGPIO_Pin_A7:  return IOMUX_PINCM8;
        case hwGPIO_Pin_A8:  return IOMUX_PINCM9;
        case hwGPIO_Pin_A9:  return IOMUX_PINCM10;
        case hwGPIO_Pin_A10: return IOMUX_PINCM11;
        case hwGPIO_Pin_A11: return IOMUX_PINCM12;
        case hwGPIO_Pin_A12: return IOMUX_PINCM13;
        case hwGPIO_Pin_A13: return IOMUX_PINCM14;
        case hwGPIO_Pin_A14: return IOMUX_PINCM15;
        case hwGPIO_Pin_A15: return IOMUX_PINCM16;
        case hwGPIO_Pin_A16: return IOMUX_PINCM17;
        case hwGPIO_Pin_A17: return IOMUX_PINCM18;
        case hwGPIO_Pin_A18: return IOMUX_PINCM19;
        case hwGPIO_Pin_A19: return IOMUX_PINCM20;
        case hwGPIO_Pin_A20: return IOMUX_PINCM21;
        case hwGPIO_Pin_A21: return IOMUX_PINCM22;
        case hwGPIO_Pin_A22: return IOMUX_PINCM23;
        case hwGPIO_Pin_A23: return IOMUX_PINCM24;
        case hwGPIO_Pin_A24: return IOMUX_PINCM25;
        case hwGPIO_Pin_A25: return IOMUX_PINCM26;
        case hwGPIO_Pin_A26: return IOMUX_PINCM27;
        case hwGPIO_Pin_A27: return IOMUX_PINCM28;

        /*
         * MSPM0L110x / L130x / L134x：
         * PA28～PA31 不存在；
         * GPIOB、GPIOC 不存在。
         */
        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0L110x || MSPM0L130x || MSPM0L134x */

#if defined(MSPM0L111x)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0:  return IOMUX_PINCM1;
        case hwGPIO_Pin_A1:  return IOMUX_PINCM2;

        case hwGPIO_Pin_A2:  return IOMUX_PINCM7;
        case hwGPIO_Pin_A3:  return IOMUX_PINCM8;
        case hwGPIO_Pin_A4:  return IOMUX_PINCM9;
        case hwGPIO_Pin_A5:  return IOMUX_PINCM10;
        case hwGPIO_Pin_A6:  return IOMUX_PINCM11;

        case hwGPIO_Pin_A7:  return IOMUX_PINCM14;

        case hwGPIO_Pin_A8:  return IOMUX_PINCM19;
        case hwGPIO_Pin_A9:  return IOMUX_PINCM20;
        case hwGPIO_Pin_A10: return IOMUX_PINCM21;
        case hwGPIO_Pin_A11: return IOMUX_PINCM22;

        case hwGPIO_Pin_A12: return IOMUX_PINCM34;
        case hwGPIO_Pin_A13: return IOMUX_PINCM35;
        case hwGPIO_Pin_A14: return IOMUX_PINCM36;
        case hwGPIO_Pin_A15: return IOMUX_PINCM37;
        case hwGPIO_Pin_A16: return IOMUX_PINCM38;
        case hwGPIO_Pin_A17: return IOMUX_PINCM39;
        case hwGPIO_Pin_A18: return IOMUX_PINCM40;
        case hwGPIO_Pin_A19: return IOMUX_PINCM41;
        case hwGPIO_Pin_A20: return IOMUX_PINCM42;

        case hwGPIO_Pin_A21: return IOMUX_PINCM46;
        case hwGPIO_Pin_A22: return IOMUX_PINCM47;

        case hwGPIO_Pin_A23: return IOMUX_PINCM53;
        case hwGPIO_Pin_A24: return IOMUX_PINCM54;
        case hwGPIO_Pin_A25: return IOMUX_PINCM55;

        case hwGPIO_Pin_A26: return IOMUX_PINCM59;
        case hwGPIO_Pin_A27: return IOMUX_PINCM60;

        case hwGPIO_Pin_A28: return IOMUX_PINCM3;

        /*
         * MSPM0L111x 沒有 PA29、PA30，
         * 但有 PA31。
         */
        case hwGPIO_Pin_A31: return IOMUX_PINCM6;

        /* GPIOB */
        case hwGPIO_Pin_B2:  return IOMUX_PINCM15;
        case hwGPIO_Pin_B3:  return IOMUX_PINCM16;

        case hwGPIO_Pin_B6:  return IOMUX_PINCM23;
        case hwGPIO_Pin_B7:  return IOMUX_PINCM24;
        case hwGPIO_Pin_B8:  return IOMUX_PINCM25;
        case hwGPIO_Pin_B9:  return IOMUX_PINCM26;

        case hwGPIO_Pin_B14: return IOMUX_PINCM31;
        case hwGPIO_Pin_B15: return IOMUX_PINCM32;
        case hwGPIO_Pin_B16: return IOMUX_PINCM33;

        case hwGPIO_Pin_B17: return IOMUX_PINCM43;
        case hwGPIO_Pin_B18: return IOMUX_PINCM44;
        case hwGPIO_Pin_B19: return IOMUX_PINCM45;

        case hwGPIO_Pin_B20: return IOMUX_PINCM48;
        case hwGPIO_Pin_B24: return IOMUX_PINCM52;

        /*
         * MSPM0L111x 不存在：
         *
         * PA29、PA30
         *
         * PB0、PB1、PB4、PB5、
         * PB10～PB13、
         * PB21～PB23、
         * PB25～PB31
         *
         * 以及全部 GPIOC。
         */
        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0L111x */

#if defined(MSPM0L112x) || \
    defined(MSPM0L211x)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0:  return IOMUX_PINCM1;
        case hwGPIO_Pin_A1:  return IOMUX_PINCM2;

        case hwGPIO_Pin_A2:  return IOMUX_PINCM7;
        case hwGPIO_Pin_A3:  return IOMUX_PINCM8;
        case hwGPIO_Pin_A4:  return IOMUX_PINCM9;
        case hwGPIO_Pin_A5:  return IOMUX_PINCM10;
        case hwGPIO_Pin_A6:  return IOMUX_PINCM11;
        case hwGPIO_Pin_A7:  return IOMUX_PINCM14;

        case hwGPIO_Pin_A8:  return IOMUX_PINCM19;
        case hwGPIO_Pin_A9:  return IOMUX_PINCM20;
        case hwGPIO_Pin_A10: return IOMUX_PINCM21;
        case hwGPIO_Pin_A11: return IOMUX_PINCM22;

        case hwGPIO_Pin_A12: return IOMUX_PINCM34;
        case hwGPIO_Pin_A13: return IOMUX_PINCM35;
        case hwGPIO_Pin_A14: return IOMUX_PINCM36;
        case hwGPIO_Pin_A15: return IOMUX_PINCM37;
        case hwGPIO_Pin_A16: return IOMUX_PINCM38;
        case hwGPIO_Pin_A17: return IOMUX_PINCM39;
        case hwGPIO_Pin_A18: return IOMUX_PINCM40;
        case hwGPIO_Pin_A19: return IOMUX_PINCM41;
        case hwGPIO_Pin_A20: return IOMUX_PINCM42;

        case hwGPIO_Pin_A21: return IOMUX_PINCM46;
        case hwGPIO_Pin_A22: return IOMUX_PINCM47;

        case hwGPIO_Pin_A23: return IOMUX_PINCM53;
        case hwGPIO_Pin_A24: return IOMUX_PINCM54;
        case hwGPIO_Pin_A25: return IOMUX_PINCM55;

        case hwGPIO_Pin_A26: return IOMUX_PINCM59;
        case hwGPIO_Pin_A27: return IOMUX_PINCM60;

        case hwGPIO_Pin_A28: return IOMUX_PINCM3;
        case hwGPIO_Pin_A29: return IOMUX_PINCM4;
        case hwGPIO_Pin_A30: return IOMUX_PINCM5;
        case hwGPIO_Pin_A31: return IOMUX_PINCM6;

        /* GPIOB */
        case hwGPIO_Pin_B0:  return IOMUX_PINCM12;
        case hwGPIO_Pin_B1:  return IOMUX_PINCM13;

        case hwGPIO_Pin_B2:  return IOMUX_PINCM15;
        case hwGPIO_Pin_B3:  return IOMUX_PINCM16;
        case hwGPIO_Pin_B4:  return IOMUX_PINCM17;
        case hwGPIO_Pin_B5:  return IOMUX_PINCM18;

        case hwGPIO_Pin_B6:  return IOMUX_PINCM23;
        case hwGPIO_Pin_B7:  return IOMUX_PINCM24;
        case hwGPIO_Pin_B8:  return IOMUX_PINCM25;
        case hwGPIO_Pin_B9:  return IOMUX_PINCM26;
        case hwGPIO_Pin_B10: return IOMUX_PINCM27;
        case hwGPIO_Pin_B11: return IOMUX_PINCM28;
        case hwGPIO_Pin_B12: return IOMUX_PINCM29;
        case hwGPIO_Pin_B13: return IOMUX_PINCM30;
        case hwGPIO_Pin_B14: return IOMUX_PINCM31;
        case hwGPIO_Pin_B15: return IOMUX_PINCM32;
        case hwGPIO_Pin_B16: return IOMUX_PINCM33;

        case hwGPIO_Pin_B17: return IOMUX_PINCM43;
        case hwGPIO_Pin_B18: return IOMUX_PINCM44;
        case hwGPIO_Pin_B19: return IOMUX_PINCM45;

        case hwGPIO_Pin_B20: return IOMUX_PINCM48;
        case hwGPIO_Pin_B21: return IOMUX_PINCM49;
        case hwGPIO_Pin_B22: return IOMUX_PINCM50;
        case hwGPIO_Pin_B23: return IOMUX_PINCM51;
        case hwGPIO_Pin_B24: return IOMUX_PINCM52;

        case hwGPIO_Pin_B25: return IOMUX_PINCM56;
        case hwGPIO_Pin_B26: return IOMUX_PINCM57;
        case hwGPIO_Pin_B27: return IOMUX_PINCM58;

        /*
         * MSPM0L112x / MSPM0L211x 不存在：
         *
         * PB28～PB31
         * GPIOC 全部
         */
        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0L112x || MSPM0L211x */

#if defined(MSPM0L122x) || \
    defined(MSPM0L222x)

uint32_t GPIO_Map_Soc_Pin_IOMUX(hwGPIO_Pin pin)
{
    switch (pin)
    {
        /* GPIOA */
        case hwGPIO_Pin_A0:  return IOMUX_PINCM1;
        case hwGPIO_Pin_A1:  return IOMUX_PINCM2;

        case hwGPIO_Pin_A2:  return IOMUX_PINCM7;
        case hwGPIO_Pin_A3:  return IOMUX_PINCM8;
        case hwGPIO_Pin_A4:  return IOMUX_PINCM9;
        case hwGPIO_Pin_A5:  return IOMUX_PINCM10;
        case hwGPIO_Pin_A6:  return IOMUX_PINCM11;
        case hwGPIO_Pin_A7:  return IOMUX_PINCM14;

        case hwGPIO_Pin_A8:  return IOMUX_PINCM19;
        case hwGPIO_Pin_A9:  return IOMUX_PINCM20;
        case hwGPIO_Pin_A10: return IOMUX_PINCM25;
        case hwGPIO_Pin_A11: return IOMUX_PINCM26;

        case hwGPIO_Pin_A12: return IOMUX_PINCM38;
        case hwGPIO_Pin_A13: return IOMUX_PINCM39;
        case hwGPIO_Pin_A14: return IOMUX_PINCM40;
        case hwGPIO_Pin_A15: return IOMUX_PINCM41;
        case hwGPIO_Pin_A16: return IOMUX_PINCM42;

        case hwGPIO_Pin_A17: return IOMUX_PINCM49;
        case hwGPIO_Pin_A18: return IOMUX_PINCM50;
        case hwGPIO_Pin_A19: return IOMUX_PINCM51;
        case hwGPIO_Pin_A20: return IOMUX_PINCM52;

        case hwGPIO_Pin_A21: return IOMUX_PINCM56;
        case hwGPIO_Pin_A22: return IOMUX_PINCM57;

        case hwGPIO_Pin_A23: return IOMUX_PINCM67;
        case hwGPIO_Pin_A24: return IOMUX_PINCM68;
        case hwGPIO_Pin_A25: return IOMUX_PINCM69;

        case hwGPIO_Pin_A26: return IOMUX_PINCM73;
        case hwGPIO_Pin_A27: return IOMUX_PINCM74;

        case hwGPIO_Pin_A28: return IOMUX_PINCM3;
        case hwGPIO_Pin_A29: return IOMUX_PINCM4;
        case hwGPIO_Pin_A30: return IOMUX_PINCM5;
        case hwGPIO_Pin_A31: return IOMUX_PINCM6;

        /* GPIOB */
        case hwGPIO_Pin_B0:  return IOMUX_PINCM12;
        case hwGPIO_Pin_B1:  return IOMUX_PINCM13;

        case hwGPIO_Pin_B2:  return IOMUX_PINCM15;
        case hwGPIO_Pin_B3:  return IOMUX_PINCM16;
        case hwGPIO_Pin_B4:  return IOMUX_PINCM17;
        case hwGPIO_Pin_B5:  return IOMUX_PINCM18;

        case hwGPIO_Pin_B6:  return IOMUX_PINCM27;
        case hwGPIO_Pin_B7:  return IOMUX_PINCM28;
        case hwGPIO_Pin_B8:  return IOMUX_PINCM29;
        case hwGPIO_Pin_B9:  return IOMUX_PINCM30;
        case hwGPIO_Pin_B10: return IOMUX_PINCM31;
        case hwGPIO_Pin_B11: return IOMUX_PINCM32;
        case hwGPIO_Pin_B12: return IOMUX_PINCM33;
        case hwGPIO_Pin_B13: return IOMUX_PINCM34;
        case hwGPIO_Pin_B14: return IOMUX_PINCM35;
        case hwGPIO_Pin_B15: return IOMUX_PINCM36;
        case hwGPIO_Pin_B16: return IOMUX_PINCM37;

        case hwGPIO_Pin_B17: return IOMUX_PINCM53;
        case hwGPIO_Pin_B18: return IOMUX_PINCM54;
        case hwGPIO_Pin_B19: return IOMUX_PINCM55;

        case hwGPIO_Pin_B20: return IOMUX_PINCM62;
        case hwGPIO_Pin_B21: return IOMUX_PINCM63;
        case hwGPIO_Pin_B22: return IOMUX_PINCM64;
        case hwGPIO_Pin_B23: return IOMUX_PINCM65;
        case hwGPIO_Pin_B24: return IOMUX_PINCM66;

        case hwGPIO_Pin_B25: return IOMUX_PINCM70;
        case hwGPIO_Pin_B26: return IOMUX_PINCM71;
        case hwGPIO_Pin_B27: return IOMUX_PINCM72;

        case hwGPIO_Pin_B28: return IOMUX_PINCM21;
        case hwGPIO_Pin_B29: return IOMUX_PINCM22;
        case hwGPIO_Pin_B30: return IOMUX_PINCM23;
        case hwGPIO_Pin_B31: return IOMUX_PINCM24;

        /* GPIOC */
        case hwGPIO_Pin_C0: return IOMUX_PINCM43;
        case hwGPIO_Pin_C1: return IOMUX_PINCM44;
        case hwGPIO_Pin_C2: return IOMUX_PINCM45;
        case hwGPIO_Pin_C3: return IOMUX_PINCM46;
        case hwGPIO_Pin_C4: return IOMUX_PINCM47;
        case hwGPIO_Pin_C5: return IOMUX_PINCM48;

        case hwGPIO_Pin_C6: return IOMUX_PINCM58;
        case hwGPIO_Pin_C7: return IOMUX_PINCM59;
        case hwGPIO_Pin_C8: return IOMUX_PINCM60;
        case hwGPIO_Pin_C9: return IOMUX_PINCM61;

        /*
         * PC10～PC31 不存在。
         */
        case hwGPIO_Pin_NC:
        default: return GPIO_SOC_IOMUX_INVALID;
    }
}

#endif /* MSPM0L122x || MSPM0L222x */