
#ifndef SYSCTRL_H
#define SYSCTRL_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "Driver_Config.h"

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef DEVICE_TITIVAC
extern uint32_t g_sys_clock_hz;
#endif

void SysCtrl_Init();

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //SYSCTRL_H