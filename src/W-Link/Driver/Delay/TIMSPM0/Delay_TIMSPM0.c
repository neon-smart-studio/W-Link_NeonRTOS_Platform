#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "Delay/Delay.h"

#ifdef DEVICE_TIMSPM0

#define DELAY_US_PER_SECOND   (1000000ULL)
#define DELAY_MS_PER_SECOND   (1000ULL)

static uint32_t s_delay_clock_hz = 0U;
static bool s_delay_has_init = false;

static uint32_t Delay_GetClockHz(void)
{
    return (uint32_t) F_CPU;
}

static void Delay_Init(void)
{
    s_delay_clock_hz = Delay_GetClockHz();

    /*
     * A zero clock definition is invalid. Keep the delay functions safe
     * instead of passing zero to DL_Common_delayCycles(), where zero means
     * the maximum possible cycle delay.
     */
    if (s_delay_clock_hz == 0U)
    {
        s_delay_clock_hz = 1U;
    }

    s_delay_has_init = true;
}

static uint32_t Delay_ClockHz(void)
{
    if (!s_delay_has_init)
    {
        Delay_Init();
    }

    return s_delay_clock_hz;
}

static void Delay_Cycles(uint64_t cycles)
{
    /*
     * DL_Common_delayCycles() accepts only uint32_t. Split long delays and
     * never pass zero because TI defines a zero argument as the maximum delay.
     */
    while (cycles > (uint64_t) UINT32_MAX)
    {
        DL_Common_delayCycles(UINT32_MAX);
        cycles -= (uint64_t) UINT32_MAX;
    }

    if (cycles != 0ULL)
    {
        DL_Common_delayCycles((uint32_t) cycles);
    }
}

static uint64_t Delay_TimeToCycles(
    uint32_t value,
    uint64_t units_per_second)
{
    uint64_t numerator =
        ((uint64_t) value * (uint64_t) Delay_ClockHz());

    /*
     * Round upward so the requested delay is not shortened by integer
     * division. At very low CPU clocks, the minimum delay is one CPU cycle.
     */
    uint64_t cycles =
        (numerator + units_per_second - 1ULL) / units_per_second;

    if ((value != 0U) && (cycles == 0ULL))
    {
        cycles = 1ULL;
    }

    return cycles;
}

void Delay_uS(uint32_t us)
{
    if (us == 0U)
    {
        return;
    }

    Delay_Cycles(Delay_TimeToCycles(us, DELAY_US_PER_SECOND));
}

void Delay_mS(uint32_t ms)
{
    if (ms == 0U)
    {
        return;
    }

    /*
     * Calculate milliseconds directly. Calling Delay_uS(ms * 1000) would
     * overflow uint32_t for long delays.
     */
    Delay_Cycles(Delay_TimeToCycles(ms, DELAY_MS_PER_SECOND));
}

void Delay(float sec)
{
    /*
     * This form also rejects NaN because every comparison with NaN is false.
     */
    if (!(sec > 0.0f))
    {
        return;
    }

    double total_us = (double) sec * (double) DELAY_US_PER_SECOND;
    uint32_t us;

    /*
     * Preserve the uint32_t microsecond range used by the original API and
     * avoid an undefined floating-point-to-integer conversion.
     */
    if (total_us >= (double) UINT32_MAX)
    {
        us = UINT32_MAX;
    }
    else
    {
        us = (uint32_t) total_us;

        /* Round upward so a fractional microsecond is not shortened. */
        if ((double) us < total_us)
        {
            us++;
        }
    }

    Delay_uS(us);
}

#endif //DEVICE_TIMSPM0