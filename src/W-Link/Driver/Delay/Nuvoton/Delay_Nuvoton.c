#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "Delay/Delay.h"

#ifdef DEVICE_NUC4x2

void Delay_uS(uint32_t us)
{
    if(us == 0)
        return;

    CLK_SysTickDelay(us);
}

void Delay_mS(uint32_t ms)
{
    while(ms--)
    {
        Delay_uS(1000);
    }
}

void Delay(float sec)
{
    if(sec <= 0.0f)
        return;

    uint32_t us = (uint32_t)(sec * 1000000.0f);

    Delay_uS(us);
}

#endif // DEVICE_NUC4x2