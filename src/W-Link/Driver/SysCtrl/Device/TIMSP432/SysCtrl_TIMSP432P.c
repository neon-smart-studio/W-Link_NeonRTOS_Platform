
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef DEVICE_TIMSP432P

#define MSP432P_CLOCK_HZ    48000000UL

uint32_t g_sys_clock_hz = MSP432P_CLOCK_HZ;

void SysCtrl_Init(void)
{
    /* Flash wait state：48MHz 需要 */
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 1);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 1);

    /* DCO = 48MHz */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);

    /* MCLK = 48MHz */
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    /* HSMCLK / SMCLK 可依需求設 24MHz 或 12MHz */
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2); // 24MHz
    MAP_CS_initClockSignal(CS_SMCLK,  CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4); // 12MHz

    /* ACLK = REFO 32.768kHz */
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    g_sys_clock_hz = MSP432P_CLOCK_HZ;
    
    Interrupt_enableMaster();
}

#endif // DEVICE_TIMSP432P