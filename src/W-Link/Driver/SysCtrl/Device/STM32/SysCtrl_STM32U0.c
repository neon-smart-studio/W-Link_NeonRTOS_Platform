#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef STM32U0

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

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;

#if defined(RCC_HSICALIBRATION_DEFAULT)
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
#endif

    RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM      = RCC_PLLM_DIV2;
    RCC_OscInitStruct.PLL.PLLN      = 12;
    RCC_OscInitStruct.PLL.PLLP      = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ      = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR      = RCC_PLLR_DIV2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while (1);
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1;

#if defined(RCC_CLOCKTYPE_PCLK2)
    RCC_ClkInitStruct.ClockType |= RCC_CLOCKTYPE_PCLK2;
#endif

    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

#if defined(RCC_CLOCKTYPE_PCLK2)
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
#endif

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        while (1);
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

#endif // STM32U0