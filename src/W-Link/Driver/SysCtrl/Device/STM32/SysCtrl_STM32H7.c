#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef STM32H7

#ifndef CONFIG_STM32_USE_HSE
#define CONFIG_STM32_USE_HSE 0
#endif

#ifndef CONFIG_STM32_HSE_VALUE
#define CONFIG_STM32_HSE_VALUE 8000000UL
#endif

void SysCtrl_Init(void)
{
    HAL_Init();

    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

#if defined(__HAL_RCC_PWR_CLK_ENABLE)
    __HAL_RCC_PWR_CLK_ENABLE();
#endif

#if defined(PWR_LDO_SUPPLY)
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
#endif

#if defined(PWR_REGULATOR_VOLTAGE_SCALE0)
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0);
#elif defined(PWR_REGULATOR_VOLTAGE_SCALE1)
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif

#if defined(PWR_FLAG_VOSRDY)
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
#endif

#if CONFIG_STM32_USE_HSE

    /*
     * HSE = 8MHz
     * PLL input = 8MHz / 1 = 8MHz
     * VCO = 8MHz * 100 = 800MHz
     * SYSCLK = 800MHz / 2 = 400MHz
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE |
                                       RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;

#if CONFIG_STM32_HSE_VALUE == 8000000UL
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 100;
#else
#error "Unsupported CONFIG_STM32_HSE_VALUE for STM32H7"
#endif

#else

    /*
     * HSI = 64MHz
     * PLL input = 64MHz / 4 = 16MHz
     * VCO = 16MHz * 50 = 800MHz
     * SYSCLK = 800MHz / 2 = 400MHz
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;

#if defined(RCC_HSICALIBRATION_DEFAULT)
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
#endif

    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;

#if defined(RCC_PLLM_DIV4)
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
#else
    RCC_OscInitStruct.PLL.PLLM = 4;
#endif

    RCC_OscInitStruct.PLL.PLLN = 50;

#endif

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;

#if defined(RCC_PLLP_DIV2)
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
#else
    RCC_OscInitStruct.PLL.PLLP = 2;
#endif

#if defined(RCC_PLLQ_DIV4)
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
#elif defined(RCC_PLLQ_DIV2)
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
#else
    RCC_OscInitStruct.PLL.PLLQ = 4;
#endif

#if defined(RCC_PLLR_DIV2)
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
#else
    RCC_OscInitStruct.PLL.PLLR = 2;
#endif

#if defined(RCC_PLLVCIRANGE_3)
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_3;
#endif

#if defined(RCC_PLLVCOWIDE)
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLLVCOWIDE;
#endif

#if defined(RCC_PLLFRACN_DISABLE)
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
#endif

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while (1);
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK  |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;

#if defined(RCC_CLOCKTYPE_D1PCLK1)
    RCC_ClkInitStruct.ClockType |= RCC_CLOCKTYPE_D1PCLK1;
#endif

#if defined(RCC_CLOCKTYPE_D3PCLK1)
    RCC_ClkInitStruct.ClockType |= RCC_CLOCKTYPE_D3PCLK1;
#endif

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

#if defined(RCC_SYSCLK_DIV1)
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
#endif

    /*
     * SYSCLK = 400MHz
     * HCLK   = 200MHz
     * APB    = 100MHz
     */
#if defined(RCC_HCLK_DIV2)
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
#else
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
#endif

    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

#if defined(RCC_APB3_DIV2)
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
#elif defined(RCC_HCLK_DIV2)
    RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV2;
#endif

#if defined(RCC_APB4_DIV2)
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
#elif defined(RCC_HCLK_DIV2)
    RCC_ClkInitStruct.APB4CLKDivider = RCC_HCLK_DIV2;
#endif

#if defined(RCC_D1PCLK1_DIV2)
    RCC_ClkInitStruct.APB3CLKDivider = RCC_D1PCLK1_DIV2;
#endif

#if defined(RCC_D3PCLK1_DIV2)
    RCC_ClkInitStruct.APB4CLKDivider = RCC_D3PCLK1_DIV2;
#endif

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        while (1);
    }

    SystemCoreClockUpdate();

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

#endif // STM32H7