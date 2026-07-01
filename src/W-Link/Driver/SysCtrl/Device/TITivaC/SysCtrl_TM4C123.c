
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef DEVICE_TITIVAC

#if defined(TM4C123)

#define TM4C123_CLOCK_HZ    80000000UL

uint32_t g_sys_clock_hz = TM4C123_CLOCK_HZ;

void SysCtrl_Init(void)
{
    MAP_SysCtlClockSet(
        SYSCTL_SYSDIV_2_5 |   // 400MHz PLL / 2.5 = 80MHz
        SYSCTL_USE_PLL |
        SYSCTL_XTAL_16MHZ |
        SYSCTL_OSC_MAIN
    );

    g_sys_clock_hz = TM4C123_CLOCK_HZ;
}

#endif // TM4C123

#endif // DEVICE_TITIVAC