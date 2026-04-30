#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef STM32H5

void SysCtrl_Init(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

#if defined(__HAL_RCC_PWR_CLK_ENABLE)
    __HAL_RCC_PWR_CLK_ENABLE();
#endif

#if defined(PWR_REGULATOR_VOLTAGE_SCALE0)
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0);
#elif defined(PWR_REGULATOR_VOLTAGE_SCALE1)
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif

    /*
     * STM32H5 common
     * HSI = 64MHz
     * SYSCLK = 250MHz
     *
     * 64MHz / 4 * 125 / 2 = 1000 / 4? No.
     *
     * Use:
     * HSI = 64MHz
     * PLLM = 4
     * PLLN = 125
     * PLLR = 8
     *
     * 64MHz / 4 * 125 / 8 = 250MHz
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;

#if defined(RCC_HSI_DIV1)
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
#endif

#if defined(RCC_HSICALIBRATION_DEFAULT)
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
#endif

    RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;

#if defined(RCC_PLLM_DIV4)
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
#else
    RCC_OscInitStruct.PLL.PLLM = 4;
#endif

    RCC_OscInitStruct.PLL.PLLN = 125;

#if defined(RCC_PLLP_DIV2)
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
#else
    RCC_OscInitStruct.PLL.PLLP = 2;
#endif

#if defined(RCC_PLLQ_DIV2)
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
#else
    RCC_OscInitStruct.PLL.PLLQ = 2;
#endif

#if defined(RCC_PLLR_DIV8)
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV8;
#else
    RCC_OscInitStruct.PLL.PLLR = 8;
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
                                  RCC_CLOCKTYPE_PCLK2 |
                                  RCC_CLOCKTYPE_PCLK3;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

#if defined(RCC_SYSCLK_DIV1)
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
#else
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
#endif

    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

#if defined(RCC_CLOCKTYPE_PCLK4)
    RCC_ClkInitStruct.ClockType |= RCC_CLOCKTYPE_PCLK4;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_HCLK_DIV1;
#endif

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        while (1);
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

#endif // STM32H5