#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "Delay/Delay.h"

#ifdef DEVICE_RPI

void Delay_uS(uint32_t us)
{
    busy_wait_us_32(us);
}

void Delay_mS(uint32_t ms)
{
    sleep_ms(ms);
}

void Delay(float sec)
{
    if (sec <= 0.0f) return;

    uint32_t us = (uint32_t)(sec * 1000000.0f);
    Delay_uS(us);
}

#endif