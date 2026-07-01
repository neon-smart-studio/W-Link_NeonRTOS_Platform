
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef DEVICE_TITIVAC

#if defined(TM4C1294)

#define TM4C1294_CLOCK_HZ 120000000

uint32_t g_sys_clock_hz = 0;

void SysCtrl_Init()
{
    g_sys_clock_hz = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                            SYSCTL_OSC_MAIN |
                            SYSCTL_USE_PLL |
                            SYSCTL_CFG_VCO_240), TM4C1294_CLOCK_HZ);
                                             
}

#endif //TM4C1294

#endif // DEVICE_TITIVAC