#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "Delay/Delay.h"

#ifdef DEVICE_TITIVAC

#define DEMCR_REG       (*((volatile uint32_t *)0xE000EDFCUL))
#define DWT_CTRL_REG    (*((volatile uint32_t *)0xE0001000UL))
#define DWT_CYCCNT_REG  (*((volatile uint32_t *)0xE0001004UL))

#define DEMCR_TRCENA    (1UL << 24)
#define DWT_CYCCNTENA   (1UL << 0)

#ifndef __NOP
#define __NOP() __asm volatile ("nop")
#endif

static uint32_t s_cycles_per_us = 0;
static bool delay_has_init = false;

static void Delay_Init(void)
{
    s_cycles_per_us = SysCtlClockGet() / 1000000U;

    if (s_cycles_per_us == 0) {
        s_cycles_per_us = 1;
    }

    DEMCR_REG |= DEMCR_TRCENA;
    DWT_CYCCNT_REG = 0;
    DWT_CTRL_REG |= DWT_CYCCNTENA;
}

void Delay_uS(uint32_t us)
{
    if (!delay_has_init) {
        Delay_Init();
        delay_has_init = true;
    }

    uint32_t start = DWT_CYCCNT_REG;
    uint32_t ticks = (uint32_t)((uint64_t)us * s_cycles_per_us);

    while ((uint32_t)(DWT_CYCCNT_REG - start) < ticks) {
        __NOP();
    }
}

void Delay_mS(uint32_t ms)
{
    while (ms--) {
        Delay_uS(1000);
    }
}

void Delay(float sec)
{
    if (sec <= 0.0f) return;

    uint32_t us = (uint32_t)(sec * 1000000.0f);
    Delay_uS(us);
}

#endif //DEVICE_TITIVAC