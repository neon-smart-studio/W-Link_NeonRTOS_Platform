
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef DEVICE_TIMSP432P

#define MSP432P_CLOCK_HZ    48000000UL

uint32_t g_sys_clock_hz = MSP432P_CLOCK_HZ;

void SysCtrl_Init(void)
{
    /* 先設 VCORE 到 1，48MHz 必要 */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* 等電源狀態穩定 */
    while (MAP_PCM_getPowerState() != PCM_AM_LDO_VCORE1);

    /* 再設 Flash wait state */
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 1);

    /* 再拉 DCO 到 48MHz */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);

    MAP_CS_initClockSignal(CS_MCLK,   CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
    MAP_CS_initClockSignal(CS_SMCLK,  CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4);
    MAP_CS_initClockSignal(CS_ACLK,   CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    g_sys_clock_hz = MSP432P_CLOCK_HZ;
    
    Interrupt_enableMaster();
}

#endif // DEVICE_TIMSP432P