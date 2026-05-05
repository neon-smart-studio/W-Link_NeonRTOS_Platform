#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef STM32U5

#ifndef CONFIG_STM32_USE_HSE
#define CONFIG_STM32_USE_HSE 0
#endif

void SysCtrl_Init(void)
{
    HAL_Init();
    
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

#if defined(__HAL_RCC_PWR_CLK_ENABLE)
    __HAL_RCC_PWR_CLK_ENABLE();
#endif

#if defined(PWR_REGULATOR_VOLTAGE_SCALE1)
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif

#if defined(PWR_FLAG_VOSRDY)
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
#endif

#if CONFIG_STM32_USE_HSE

    /*
     * HSE = 8MHz
     * SYSCLK = 160MHz
     * 8 / 1 * 40 / 2 = 160MHz
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;

    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM      = 1;
    RCC_OscInitStruct.PLL.PLLN      = 40;

#else

    /*
     * HSI = 16MHz
     * SYSCLK = 160MHz
     * 16 / 1 * 20 / 2 = 160MHz
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;

#if defined(RCC_HSICALIBRATION_DEFAULT)
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
#endif

    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM      = 1;
    RCC_OscInitStruct.PLL.PLLN      = 20;

#endif

    RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLP      = 2;
    RCC_OscInitStruct.PLL.PLLQ      = 2;
    RCC_OscInitStruct.PLL.PLLR      = 2;

#if defined(RCC_PLL1VCIRANGE_2)
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
#endif

#if defined(RCC_PLL1VCOWIDE)
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
#endif

#if defined(RCC_PLL_FRACN_DISABLE)
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

#if defined(RCC_CLOCKTYPE_PCLK3)
    RCC_ClkInitStruct.ClockType |= RCC_CLOCKTYPE_PCLK3;
#endif

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

#if defined(RCC_SYSCLK_DIV1)
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
#else
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
#endif

    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

#if defined(RCC_CLOCKTYPE_PCLK3)
    RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;
#endif

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        while (1);
    }

    SystemCoreClockUpdate();

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

#endif // STM32U5