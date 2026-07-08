
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef DEVICE_TIMSP432E

#define MSP432E_CLOCK_HZ    120000000UL

uint32_t g_sys_clock_hz = 0;

void SysCtrl_Init(void)
{
    g_sys_clock_hz = MAP_SysCtlClockFreqSet(
        SYSCTL_XTAL_25MHZ |
        SYSCTL_OSC_MAIN |
        SYSCTL_USE_PLL |
        SYSCTL_CFG_VCO_480,
        MSP432E_CLOCK_HZ
    );
}

#endif // DEVICE_TIMSP432E