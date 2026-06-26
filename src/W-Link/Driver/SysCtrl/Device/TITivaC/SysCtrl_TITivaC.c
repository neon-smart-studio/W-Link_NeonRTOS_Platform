
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#if defined(TM4C123)

#define TM4C123_CLOCK_HZ    80000000UL

void SysCtrl_Init(void)
{
    MAP_SysCtlClockSet(
        SYSCTL_SYSDIV_2_5 |   // 400 / 2.5 = 80 MHz
        SYSCTL_USE_PLL |
        SYSCTL_XTAL_16MHZ |
        SYSCTL_OSC_MAIN);
}

#endif // TM4C123

#if defined(TM4C1294)

#define TM4C1294_CLOCK_HZ 120000000

void SysCtrl_Init()
{
    MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                            SYSCTL_OSC_MAIN |
                            SYSCTL_USE_PLL |
                            SYSCTL_CFG_VCO_240), TM4C1294_CLOCK_HZ);
                                             
}

#endif //TM4C1294
