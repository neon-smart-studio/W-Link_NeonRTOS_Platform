
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "PWM/PWM.h"

#include "SysCtrl/SysCtrl.h"

#ifdef DEVICE_TIMSPM0

#include "PWM/Pin/PWM_Pin.h"

#include "GPIO/Device/TIMSPM0/GPIO_TIMSPM0.h"

#include "PWM/Pin/TIMSPM0/PWM_Pin_TIMSPM0.h"
#define PWM_HZ           1000

#define PWM_TIMSPM0_POWER_STARTUP_DELAY (16U)

#define PWM_TIMSPM0_16BIT_PERIOD_MAX   (65536UL)

#ifndef GPIO_SOC_IOMUX_INVALID
#define GPIO_SOC_IOMUX_INVALID         (UINT32_MAX)
#endif

static bool PWM_Channel_Init_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_OnOff_Status[hwPWM_Channel_MAX] = {false};
static bool PWM_Channel_Inverse_Status[hwPWM_Channel_MAX] = {false};
static uint16_t PWM_Channel_Current_Duty[hwPWM_Channel_MAX] = {0};

static bool PWM_Base_Init_Status[hwPWM_Base_MAX] = {false};
static uint32_t PWM_Base_Period[hwPWM_Base_MAX] = {0};

static bool PWM_IsValidChannel(hwPWM_Channel channel)
{
    return ((int32_t) channel >= 0) &&
           (channel < hwPWM_Channel_MAX);
}

static bool PWM_IsValidBase(hwPWM_Base_Index base)
{
    return ((int32_t) base >= 0) &&
           (base < hwPWM_Base_MAX);
}

static GPTIMER_Regs *PWM_Map_Timer_Base(hwPWM_Base_Index base)
{
    switch (base)
    {
#if defined(TIMA0_BASE)
        case hwPWM_Base_TIMA0:
            return TIMA0_BASE;
#endif
#if defined(TIMA1_BASE)
        case hwPWM_Base_TIMA1:
            return TIMA1_BASE;
#endif
#if defined(TIMG0_BASE)
        case hwPWM_Base_TIMG0:
            return TIMG0_BASE;
#endif
#if defined(TIMG1_BASE)
        case hwPWM_Base_TIMG1:
            return TIMG1_BASE;
#endif
#if defined(TIMG2_BASE)
        case hwPWM_Base_TIMG2:
            return TIMG2_BASE;
#endif
#if defined(TIMG4_BASE)
        case hwPWM_Base_TIMG4:
            return TIMG4_BASE;
#endif
#if defined(TIMG5_BASE)
        case hwPWM_Base_TIMG5:
            return TIMG5_BASE;
#endif
#if defined(TIMG6_BASE)
        case hwPWM_Base_TIMG6:
            return TIMG6_BASE;
#endif
#if defined(TIMG7_BASE)
        case hwPWM_Base_TIMG7:
            return TIMG7_BASE;
#endif
#if defined(TIMG8_BASE)
        case hwPWM_Base_TIMG8:
            return TIMG8_BASE;
#endif
#if defined(TIMG9_BASE)
        case hwPWM_Base_TIMG9:
            return TIMG9_BASE;
#endif
#if defined(TIMG12_BASE)
        case hwPWM_Base_TIMG12:
            return TIMG12_BASE;
#endif
#if defined(TIMG14_BASE)
        case hwPWM_Base_TIMG14:
            return TIMG14_BASE;
#endif

        default:
            return NULL;
    }
}

/*
 * PINCM 位址由 GPIO_Map_Soc_Pin_IOMUX() 提供；此函式只回傳屬於
 * PWM Timer/CCP route 的 peripheral-function selector。
 */
static uint32_t PWM_Map_Soc_Pin_Function(hwPWM_Channel channel)
{
#if defined(MSPM0C110x)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM29_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM23_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM18_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM19_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM1_PF_TIMG8_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM3_PF_TIMG8_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM17_PF_TIMG14_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM24_PF_TIMG14_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM25_PF_TIMG14_CCP2;
        case hwPWM_Channel_10: return IOMUX_PINCM26_PF_TIMG14_CCP3;
        default:               return 0U;
    }

#elif defined(MSPM0C1105) || defined(MSPM0C1106)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM5_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM38_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM6_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM24_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM10_PF_TIMG1_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM9_PF_TIMG1_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM14_PF_TIMG2_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM7_PF_TIMG2_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM29_PF_TIMG8_CCP0;
        case hwPWM_Channel_10: return IOMUX_PINCM1_PF_TIMG8_CCP1;
        case hwPWM_Channel_11: return IOMUX_PINCM8_PF_TIMG14_CCP0;
        case hwPWM_Channel_12: return IOMUX_PINCM2_PF_TIMG14_CCP1;
        case hwPWM_Channel_13: return IOMUX_PINCM28_PF_TIMG14_CCP2;
        case hwPWM_Channel_14: return IOMUX_PINCM25_PF_TIMG14_CCP3;
        default:               return 0U;
    }

#elif defined(MSPM0H321x)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM5_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM37_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM6_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM24_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM10_PF_TIMG1_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM9_PF_TIMG1_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM14_PF_TIMG2_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM7_PF_TIMG2_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM29_PF_TIMG8_CCP0;
        case hwPWM_Channel_10: return IOMUX_PINCM1_PF_TIMG8_CCP1;
        case hwPWM_Channel_11: return IOMUX_PINCM8_PF_TIMG14_CCP0;
        case hwPWM_Channel_12: return IOMUX_PINCM2_PF_TIMG14_CCP1;
        case hwPWM_Channel_13: return IOMUX_PINCM28_PF_TIMG14_CCP2;
        case hwPWM_Channel_14: return IOMUX_PINCM25_PF_TIMG14_CCP3;
        default:               return 0U;
    }

#elif defined(MSPM0G110x) || defined(MSPM0G150x) || \
      defined(MSPM0G310x) || defined(MSPM0G350x)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM19_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM8_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM14_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM9_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM21_PF_TIMA1_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM22_PF_TIMA1_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM34_PF_TIMG0_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM35_PF_TIMG0_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM10_PF_TIMG6_CCP0;
        case hwPWM_Channel_10: return IOMUX_PINCM11_PF_TIMG6_CCP1;
        case hwPWM_Channel_11: return IOMUX_PINCM39_PF_TIMG7_CCP0;
        case hwPWM_Channel_12: return IOMUX_PINCM7_PF_TIMG7_CCP1;
        case hwPWM_Channel_13: return IOMUX_PINCM2_PF_TIMG8_CCP0;
        case hwPWM_Channel_14: return IOMUX_PINCM1_PF_TIMG8_CCP1;
        case hwPWM_Channel_15: return IOMUX_PINCM36_PF_TIMG12_CCP0;
        case hwPWM_Channel_16: return IOMUX_PINCM55_PF_TIMG12_CCP1;
        default:               return 0U;
    }

#elif defined(MSPM0G120x) || defined(MSPM0G121x) || \
      defined(MSPM0G320x) || defined(MSPM0G321x)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM7_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM8_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM14_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM9_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM21_PF_TIMA1_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM22_PF_TIMA1_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM10_PF_TIMG0_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM11_PF_TIMG0_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM53_PF_TIMG1_CCP0;
        case hwPWM_Channel_10: return IOMUX_PINCM55_PF_TIMG1_CCP1;
        case hwPWM_Channel_11: return IOMUX_PINCM2_PF_TIMG8_CCP0;
        case hwPWM_Channel_12: return IOMUX_PINCM1_PF_TIMG8_CCP1;
        default:               return 0U;
    }

#elif defined(MSPM0G151x) || defined(MSPM0G351x) || \
      defined(MSPM0G352x)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM7_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM8_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM14_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM34_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM37_PF_TIMA1_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM38_PF_TIMA1_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM10_PF_TIMG0_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM11_PF_TIMG0_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM46_PF_TIMG6_CCP0;
        case hwPWM_Channel_10: return IOMUX_PINCM47_PF_TIMG6_CCP1;
        case hwPWM_Channel_11: return IOMUX_PINCM39_PF_TIMG7_CCP0;
        case hwPWM_Channel_12: return IOMUX_PINCM9_PF_TIMG7_CCP1;
        case hwPWM_Channel_13: return IOMUX_PINCM2_PF_TIMG8_CCP0;
        case hwPWM_Channel_14: return IOMUX_PINCM1_PF_TIMG8_CCP1;
        case hwPWM_Channel_15: return IOMUX_PINCM24_PF_TIMG9_CCP0;
        case hwPWM_Channel_16: return IOMUX_PINCM26_PF_TIMG9_CCP1;
        case hwPWM_Channel_17: return IOMUX_PINCM21_PF_TIMG12_CCP0;
        case hwPWM_Channel_18: return IOMUX_PINCM22_PF_TIMG12_CCP1;
        case hwPWM_Channel_19: return IOMUX_PINCM4_PF_TIMG14_CCP0;
        case hwPWM_Channel_20: return IOMUX_PINCM5_PF_TIMG14_CCP1;
        case hwPWM_Channel_21: return IOMUX_PINCM19_PF_TIMG14_CCP2;
        case hwPWM_Channel_22: return IOMUX_PINCM20_PF_TIMG14_CCP3;
        default:               return 0U;
    }

#elif defined(MSPM0G511x) || defined(MSPM0G518x)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM7_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM2_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM8_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM34_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM1_PF_TIMG0_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM11_PF_TIMG0_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM10_PF_TIMG6_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM47_PF_TIMG6_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM36_PF_TIMG7_CCP0;
        case hwPWM_Channel_10: return IOMUX_PINCM9_PF_TIMG7_CCP1;
        default:               return 0U;
    }

#elif defined(MSPM0L110x) || defined(MSPM0L130x) || \
      defined(MSPM0L134x)
    switch (channel)
    {
        case hwPWM_Channel_1: return IOMUX_PINCM6_PF_TIMG0_CCP0;
        case hwPWM_Channel_2: return IOMUX_PINCM7_PF_TIMG0_CCP1;
        case hwPWM_Channel_3: return IOMUX_PINCM1_PF_TIMG1_CCP0;
        case hwPWM_Channel_4: return IOMUX_PINCM2_PF_TIMG1_CCP1;
        case hwPWM_Channel_5: return IOMUX_PINCM4_PF_TIMG2_CCP0;
        case hwPWM_Channel_6: return IOMUX_PINCM5_PF_TIMG2_CCP1;
        case hwPWM_Channel_7: return IOMUX_PINCM11_PF_TIMG4_CCP0;
        case hwPWM_Channel_8: return IOMUX_PINCM12_PF_TIMG4_CCP1;
        default:              return 0U;
    }

#elif defined(MSPM0L111x)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM7_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM8_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM14_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM9_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM10_PF_TIMG0_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM11_PF_TIMG0_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM37_PF_TIMG1_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM38_PF_TIMG1_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM2_PF_TIMG8_CCP0;
        case hwPWM_Channel_10: return IOMUX_PINCM1_PF_TIMG8_CCP1;
        default:               return 0U;
    }

#elif defined(MSPM0L112x) || defined(MSPM0L211x)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM1_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM8_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM14_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM34_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM46_PF_TIMG1_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM47_PF_TIMG1_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM20_PF_TIMG2_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM9_PF_TIMG2_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM10_PF_TIMG14_CCP0;
        case hwPWM_Channel_10: return IOMUX_PINCM11_PF_TIMG14_CCP1;
        case hwPWM_Channel_11: return IOMUX_PINCM2_PF_TIMG14_CCP2;
        case hwPWM_Channel_12: return IOMUX_PINCM7_PF_TIMG14_CCP3;
        default:               return 0U;
    }

#elif defined(MSPM0L122x) || defined(MSPM0L222x)
    switch (channel)
    {
        case hwPWM_Channel_1:  return IOMUX_PINCM7_PF_TIMA0_CCP0;
        case hwPWM_Channel_2:  return IOMUX_PINCM8_PF_TIMA0_CCP1;
        case hwPWM_Channel_3:  return IOMUX_PINCM14_PF_TIMA0_CCP2;
        case hwPWM_Channel_4:  return IOMUX_PINCM38_PF_TIMA0_CCP3;
        case hwPWM_Channel_5:  return IOMUX_PINCM10_PF_TIMG0_CCP0;
        case hwPWM_Channel_6:  return IOMUX_PINCM11_PF_TIMG0_CCP1;
        case hwPWM_Channel_7:  return IOMUX_PINCM56_PF_TIMG4_CCP0;
        case hwPWM_Channel_8:  return IOMUX_PINCM57_PF_TIMG4_CCP1;
        case hwPWM_Channel_9:  return IOMUX_PINCM20_PF_TIMG5_CCP0;
        case hwPWM_Channel_10: return IOMUX_PINCM9_PF_TIMG5_CCP1;
        case hwPWM_Channel_11: return IOMUX_PINCM2_PF_TIMG8_CCP0;
        case hwPWM_Channel_12: return IOMUX_PINCM1_PF_TIMG8_CCP1;
        case hwPWM_Channel_13: return IOMUX_PINCM25_PF_TIMG12_CCP0;
        case hwPWM_Channel_14: return IOMUX_PINCM26_PF_TIMG12_CCP1;
        default:               return 0U;
    }

#else
    (void) channel;
    return 0U;
#endif
}

static bool PWM_Base_HasFourCC(hwPWM_Base_Index base)
{
    switch (base)
    {
#if defined(TIMA0_BASE)
        case hwPWM_Base_TIMA0:
            return true;
#endif
#if defined(TIMG14_BASE)
        case hwPWM_Base_TIMG14:
            return true;
#endif

        default:
            return false;
    }
}

static uint32_t PWM_CC_OutputMask(uint8_t compare_index)
{
    switch (compare_index)
    {
        case 0:
            return DL_TIMER_CC0_OUTPUT;

        case 1:
            return DL_TIMER_CC1_OUTPUT;

        case 2:
            return DL_TIMER_CC2_OUTPUT;

        case 3:
            return DL_TIMER_CC3_OUTPUT;

        default:
            return 0U;
    }
}

static DL_TIMER_CLOCK_DIVIDE PWM_ClockDivide_FromValue(
    uint32_t divide_value)
{
    switch (divide_value)
    {
        case 1:
            return DL_TIMER_CLOCK_DIVIDE_1;

        case 2:
            return DL_TIMER_CLOCK_DIVIDE_2;

        case 3:
            return DL_TIMER_CLOCK_DIVIDE_3;

        case 4:
            return DL_TIMER_CLOCK_DIVIDE_4;

        case 5:
            return DL_TIMER_CLOCK_DIVIDE_5;

        case 6:
            return DL_TIMER_CLOCK_DIVIDE_6;

        case 7:
            return DL_TIMER_CLOCK_DIVIDE_7;

        case 8:
        default:
            return DL_TIMER_CLOCK_DIVIDE_8;
    }
}

static bool PWM_CalculateTimerConfig(
    DL_TIMER_CLOCK_DIVIDE *clock_divide,
    uint32_t *period)
{
    if ((clock_divide == NULL) ||
        (period == NULL) ||
        (PWM_HZ == 0U) ||
        (g_sys_clock_hz == 0U))
    {
        return false;
    }

    for (uint32_t divide_value = 1U;
         divide_value <= 8U;
         divide_value++)
    {
        uint64_t denominator =
            (uint64_t) PWM_HZ * divide_value;
        uint32_t calculated_period =
            (uint32_t)
                (((uint64_t) g_sys_clock_hz +
                  (denominator / 2U)) /
                 denominator);

        if ((calculated_period > 0U) &&
            (calculated_period <=
             PWM_TIMSPM0_16BIT_PERIOD_MAX))
        {
            *clock_divide =
                PWM_ClockDivide_FromValue(divide_value);
            *period = calculated_period;
            return true;
        }
    }

    return false;
}

static hwPWM_OpResult PWM_Base_Init(hwPWM_Base_Index base)
{
    if (!PWM_IsValidBase(base))
    {
        return hwPWM_InvalidParameter;
    }

    if (PWM_Base_Init_Status[base])
    {
        return hwPWM_OK;
    }

    GPTIMER_Regs *timer = PWM_Map_Timer_Base(base);
    DL_TIMER_CLOCK_DIVIDE clock_divide;
    uint32_t period;

    if ((timer == NULL) ||
        !PWM_CalculateTimerConfig(&clock_divide, &period))
    {
        return hwPWM_InvalidParameter;
    }

    DL_Timer_reset(timer);
    DL_Timer_enablePower(timer);
    DL_Common_delayCycles(PWM_TIMSPM0_POWER_STARTUP_DELAY);

    const DL_Timer_ClockConfig clock_config = {
        .clockSel = DL_TIMER_CLOCK_BUSCLK,
        .divideRatio = clock_divide,
        .prescale = 0U,
    };

    const DL_Timer_PWMConfig pwm_config = {
        .period = period,
        .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
        .isTimerWithFourCC = PWM_Base_HasFourCC(base),
        .startTimer = DL_TIMER_STOP,
    };

    DL_Timer_setClockConfig(timer, &clock_config);
    DL_Timer_initPWMMode(timer, &pwm_config);
    DL_Timer_enableClock(timer);

    PWM_Base_Period[base] = period;
    PWM_Base_Init_Status[base] = true;

    return hwPWM_OK;
}

static void PWM_Base_DeInit(hwPWM_Base_Index base)
{
    if (!PWM_IsValidBase(base) ||
        !PWM_Base_Init_Status[base])
    {
        return;
    }

    GPTIMER_Regs *timer = PWM_Map_Timer_Base(base);

    if (timer != NULL)
    {
        DL_Timer_stopCounter(timer);
        DL_Timer_disableClock(timer);
        DL_Timer_reset(timer);
        DL_Timer_disablePower(timer);
    }

    PWM_Base_Period[base] = 0U;
    PWM_Base_Init_Status[base] = false;
}

static bool PWM_IsBaseUsed(hwPWM_Base_Index base)
{
    for (hwPWM_Channel channel = hwPWM_Channel_1;
         channel < hwPWM_Channel_MAX;
         channel++)
    {
        if (PWM_Channel_Init_Status[channel] &&
            (PWM_Pin_Def_Table[channel].base == base))
        {
            return true;
        }
    }

    return false;
}

static bool PWM_GetPinResources(
    hwPWM_Channel channel,
    GPIO_Regs **gpio,
    uint32_t *gpio_pin,
    uint32_t *iomux)
{
    if (!PWM_IsValidChannel(channel) ||
        (gpio == NULL) ||
        (gpio_pin == NULL) ||
        (iomux == NULL))
    {
        return false;
    }

    const PWM_Pin_Def *definition =
        &PWM_Pin_Def_Table[channel];

    if (definition->channel != channel)
    {
        return false;
    }

    *gpio = GPIO_Map_Soc_Base(definition->pin);
    *gpio_pin = GPIO_Map_Soc_Pin(definition->pin);
    *iomux = GPIO_Map_Soc_Pin_IOMUX(definition->pin);

    return ((*gpio != NULL) &&
            (*gpio_pin != 0U) &&
            (*iomux != GPIO_SOC_IOMUX_INVALID));
}

static uint32_t PWM_Duty_To_Compare(
    hwPWM_Base_Index base,
    uint16_t duty)
{
    if (!PWM_IsValidBase(base) ||
        (PWM_Base_Period[base] == 0U))
    {
        return 0U;
    }

    if (duty > PWM_MAX_DUTY)
    {
        duty = PWM_MAX_DUTY;
    }

    /*
     * DL_TIMER_PWM_MODE_EDGE_ALIGN counts down:
     * duty = 1 - ((compare + 1) / period).
     */
    uint64_t inactive_ticks =
        ((uint64_t) PWM_Base_Period[base] *
         (PWM_MAX_DUTY - duty) +
         (PWM_MAX_DUTY / 2U)) /
        PWM_MAX_DUTY;

    if (inactive_ticks == 0U)
    {
        return 0U;
    }

    return (uint32_t) (inactive_ticks - 1U);
}

static hwPWM_OpResult PWM_SetChannelCompare(
    hwPWM_Channel channel,
    uint16_t duty)
{
    const PWM_Pin_Def *definition =
        &PWM_Pin_Def_Table[channel];
    GPTIMER_Regs *timer =
        PWM_Map_Timer_Base(definition->base);

    if ((timer == NULL) ||
        (definition->compare_index > 3U))
    {
        return hwPWM_InvalidParameter;
    }

    DL_Timer_setCaptureCompareValue(
        timer,
        PWM_Duty_To_Compare(definition->base, duty),
        (DL_TIMER_CC_INDEX) definition->compare_index);

    return hwPWM_OK;
}

static void PWM_ForceChannelOff(hwPWM_Channel channel)
{
    const PWM_Pin_Def *definition =
        &PWM_Pin_Def_Table[channel];
    GPTIMER_Regs *timer =
        PWM_Map_Timer_Base(definition->base);

    if (timer == NULL)
    {
        return;
    }

    /*
     * Software force is before OCTL inversion. Compensate for inversion so
     * PWM_Turn_Off always drives the physical pin low.
     */
    DL_TIMER_FORCE_OUT force =
        PWM_Channel_Inverse_Status[channel] ?
            DL_TIMER_FORCE_OUT_HIGH :
            DL_TIMER_FORCE_OUT_LOW;

    DL_Timer_overrideCCPOut(
        timer,
        force,
        DL_TIMER_FORCE_CMPL_OUT_DISABLED,
        (DL_TIMER_CC_INDEX) definition->compare_index);
}

static void PWM_ReleaseChannelOutput(hwPWM_Channel channel)
{
    const PWM_Pin_Def *definition =
        &PWM_Pin_Def_Table[channel];
    GPTIMER_Regs *timer =
        PWM_Map_Timer_Base(definition->base);

    if (timer == NULL)
    {
        return;
    }

    DL_Timer_overrideCCPOut(
        timer,
        DL_TIMER_FORCE_OUT_DISABLED,
        DL_TIMER_FORCE_CMPL_OUT_DISABLED,
        (DL_TIMER_CC_INDEX) definition->compare_index);
}

hwPWM_OpResult PWM_Channel_Init(
    hwPWM_Channel channel_index,
    bool inverse_PWM)
{
    if (!PWM_IsValidChannel(channel_index))
    {
        return hwPWM_InvalidParameter;
    }

    if (PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_OK;
    }

    const PWM_Pin_Def *definition = &PWM_Pin_Def_Table[channel_index];
    hwPWM_Base_Index base = definition->base;
    GPIO_Regs *gpio;
    uint32_t gpio_pin;
    uint32_t iomux;
    uint32_t iomux_function = PWM_Map_Soc_Pin_Function(channel_index);
    uint32_t ccp_mask = PWM_CC_OutputMask(definition->compare_index);

    if (!PWM_IsValidBase(base) ||
        !PWM_GetPinResources(
            channel_index, &gpio, &gpio_pin, &iomux) ||
        (iomux_function == 0U) ||
        (ccp_mask == 0U) ||
        ((!PWM_Base_HasFourCC(base)) &&
         (definition->compare_index > 1U)))
    {
        return hwPWM_InvalidParameter;
    }

    bool new_base = !PWM_Base_Init_Status[base];
    hwPWM_OpResult result = PWM_Base_Init(base);

    if (result != hwPWM_OK)
    {
        return result;
    }

    GPTIMER_Regs *timer = PWM_Map_Timer_Base(base);

    if (timer == NULL)
    {
        if (new_base)
        {
            PWM_Base_DeInit(base);
        }
        return hwPWM_InvalidParameter;
    }

    PWM_Channel_Inverse_Status[channel_index] = inverse_PWM;
    PWM_Channel_Current_Duty[channel_index] = PWM_MIN_DUTY;

    DL_Timer_setCaptureCompareOutCtl(
        timer,
        inverse_PWM ?
            DL_TIMER_CC_OCTL_INIT_VAL_HIGH :
            DL_TIMER_CC_OCTL_INIT_VAL_LOW,
        inverse_PWM ?
            DL_TIMER_CC_OCTL_INV_OUT_ENABLED :
            DL_TIMER_CC_OCTL_INV_OUT_DISABLED,
        DL_TIMER_CC_OCTL_SRC_FUNCVAL,
        (DL_TIMER_CC_INDEX) definition->compare_index);

    DL_Timer_setCaptCompUpdateMethod(
        timer,
        DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE,
        (DL_TIMER_CC_INDEX) definition->compare_index);

    result = PWM_SetChannelCompare(
        channel_index,
        PWM_MIN_DUTY);

    if (result != hwPWM_OK)
    {
        if (new_base)
        {
            PWM_Base_DeInit(base);
        }
        return result;
    }

    PWM_ForceChannelOff(channel_index);

    DL_Timer_setCCPDirection(
        timer,
        DL_Timer_getCCPDirection(timer) | ccp_mask);

    DL_GPIO_enablePower(gpio);
    DL_Common_delayCycles(PWM_TIMSPM0_POWER_STARTUP_DELAY);
    DL_GPIO_initPeripheralOutputFunction(iomux, iomux_function);
    DL_GPIO_enableOutput(gpio, gpio_pin);

    if (new_base)
    {
        DL_Timer_startCounter(timer);
    }

    gpio_pin_init_status[definition->pin] = true;
    PWM_Channel_Init_Status[channel_index] = true;
    PWM_Channel_OnOff_Status[channel_index] = false;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Channel_DeInit(hwPWM_Channel channel_index)
{
    if (!PWM_IsValidChannel(channel_index))
    {
        return hwPWM_InvalidParameter;
    }

    if (!PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_OK;
    }

    const PWM_Pin_Def *definition =
        &PWM_Pin_Def_Table[channel_index];
    hwPWM_Base_Index base = definition->base;
    GPTIMER_Regs *timer = PWM_Map_Timer_Base(base);
    GPIO_Regs *gpio;
    uint32_t gpio_pin;
    uint32_t iomux;
    uint32_t ccp_mask =
        PWM_CC_OutputMask(definition->compare_index);

    if ((timer == NULL) ||
        !PWM_GetPinResources(
            channel_index, &gpio, &gpio_pin, &iomux) ||
        (ccp_mask == 0U))
    {
        return hwPWM_InvalidParameter;
    }

    PWM_ForceChannelOff(channel_index);
    DL_Timer_setCCPDirection(
        timer,
        DL_Timer_getCCPDirection(timer) & ~ccp_mask);

    DL_GPIO_disableOutput(gpio, gpio_pin);
    DL_GPIO_initDigitalInput(iomux);

    gpio_pin_init_status[definition->pin] = false;
    PWM_Channel_Init_Status[channel_index] = false;
    PWM_Channel_OnOff_Status[channel_index] = false;
    PWM_Channel_Inverse_Status[channel_index] = false;
    PWM_Channel_Current_Duty[channel_index] = PWM_MIN_DUTY;

    if (!PWM_IsBaseUsed(base))
    {
        PWM_Base_DeInit(base);
    }

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On(hwPWM_Channel channel_index)
{
    if (!PWM_IsValidChannel(channel_index))
    {
        return hwPWM_InvalidParameter;
    }

    if (!PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_NotInit;
    }

    hwPWM_OpResult result =
        PWM_SetChannelCompare(
            channel_index,
            PWM_Channel_Current_Duty[channel_index]);

    if (result != hwPWM_OK)
    {
        return result;
    }

    PWM_ReleaseChannelOutput(channel_index);
    PWM_Channel_OnOff_Status[channel_index] = true;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_On_And_Set_Duty(
    hwPWM_Channel channel_index,
    uint16_t duty)
{
    if (!PWM_IsValidChannel(channel_index) ||
        (duty > PWM_MAX_DUTY))
    {
        return hwPWM_InvalidParameter;
    }

    if (!PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_NotInit;
    }

    hwPWM_OpResult result =
        PWM_SetChannelCompare(channel_index, duty);

    if (result != hwPWM_OK)
    {
        return result;
    }

    PWM_Channel_Current_Duty[channel_index] = duty;
    PWM_ReleaseChannelOutput(channel_index);
    PWM_Channel_OnOff_Status[channel_index] = true;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Turn_Off(hwPWM_Channel channel_index)
{
    if (!PWM_IsValidChannel(channel_index))
    {
        return hwPWM_InvalidParameter;
    }

    if (!PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_NotInit;
    }

    PWM_ForceChannelOff(channel_index);
    PWM_Channel_OnOff_Status[channel_index] = false;

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Set_Duty(
    hwPWM_Channel channel_index,
    uint16_t duty)
{
    if (!PWM_IsValidChannel(channel_index) ||
        (duty > PWM_MAX_DUTY))
    {
        return hwPWM_InvalidParameter;
    }

    if (!PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_NotInit;
    }

    if (!PWM_Channel_OnOff_Status[channel_index])
    {
        return hwPWM_NotTurnOn;
    }

    hwPWM_OpResult result =
        PWM_SetChannelCompare(channel_index, duty);

    if (result == hwPWM_OK)
    {
        PWM_Channel_Current_Duty[channel_index] = duty;
    }

    return result;
}

hwPWM_OpResult PWM_Step_Duty(
    hwPWM_Channel channel_index,
    uint16_t step_duty,
    hwPWM_Step_Direction direction)
{
    if (!PWM_IsValidChannel(channel_index) ||
        (direction >= hwPWM_Step_Dir_MAX))
    {
        return hwPWM_InvalidParameter;
    }

    if (!PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_NotInit;
    }

    if (!PWM_Channel_OnOff_Status[channel_index])
    {
        return hwPWM_NotTurnOn;
    }

    uint16_t current_duty =
        PWM_Channel_Current_Duty[channel_index];

    switch (direction)
    {
        case hwPWM_Step_Dir_Up:
            if ((PWM_MAX_DUTY - current_duty) < step_duty)
            {
                current_duty = PWM_MAX_DUTY;
            }
            else
            {
                current_duty += step_duty;
            }
            break;

        case hwPWM_Step_Dir_Down:
            if ((current_duty - PWM_MIN_DUTY) < step_duty)
            {
                current_duty = PWM_MIN_DUTY;
            }
            else
            {
                current_duty -= step_duty;
            }
            break;

        default:
            return hwPWM_InvalidParameter;
    }

    hwPWM_OpResult result =
        PWM_SetChannelCompare(channel_index, current_duty);

    if (result == hwPWM_OK)
    {
        PWM_Channel_Current_Duty[channel_index] =
            current_duty;
    }

    return result;
}

hwPWM_OpResult PWM_Get_Channel_OnOff_Status(
    hwPWM_Channel channel_index,
    bool *onoff_status)
{
    if (!PWM_IsValidChannel(channel_index) ||
        (onoff_status == NULL))
    {
        return hwPWM_InvalidParameter;
    }

    if (!PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_NotInit;
    }

    *onoff_status =
        PWM_Channel_OnOff_Status[channel_index];

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_Current_Duty(
    hwPWM_Channel channel_index,
    uint16_t *current_duty)
{
    if (!PWM_IsValidChannel(channel_index) ||
        (current_duty == NULL))
    {
        return hwPWM_InvalidParameter;
    }

    if (!PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_NotInit;
    }

    *current_duty =
        PWM_Channel_Current_Duty[channel_index];

    return hwPWM_OK;
}

hwPWM_OpResult PWM_Get_Channel_Current_OnOff_Duty(
    hwPWM_Channel channel_index,
    bool *onoff_status,
    uint16_t *current_duty)
{
    if (!PWM_IsValidChannel(channel_index) ||
        (onoff_status == NULL) ||
        (current_duty == NULL))
    {
        return hwPWM_InvalidParameter;
    }

    if (!PWM_Channel_Init_Status[channel_index])
    {
        return hwPWM_NotInit;
    }

    *onoff_status =
        PWM_Channel_OnOff_Status[channel_index];
    *current_duty =
        PWM_Channel_Current_Duty[channel_index];

    return hwPWM_OK;
}

bool PWM_is_Init(hwPWM_Channel channel_index)
{
    if (!PWM_IsValidChannel(channel_index))
    {
        return false;
    }

    return PWM_Channel_Init_Status[channel_index];
}


#endif // DEVICE_TIMSPM0