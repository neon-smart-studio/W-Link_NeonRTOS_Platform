#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef STM32H7RS

void SysCtrl_Init(void)
{
    HAL_Init();
    
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

#if defined(PWR_FLAG_VOSRDY)
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
#endif

    /*
     * STM32H7RS / H7S / H7R
     *
     * HSI = 64MHz
     * SYSCLK = 600MHz
     *
     * 64MHz / 4 * 150 / 4 = 600MHz
     *
     * VCO = 64 / 4 * 150 = 2400MHz
     * PLLP = 4
     */

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;

#if defined(RCC_HSICALIBRATION_DEFAULT)
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
#endif

    RCC_OscInitStruct.PLL1.PLLState  = RCC_PLL_ON;
    RCC_OscInitStruct.PLL1.PLLSource = RCC_PLLSOURCE_HSI;

#if defined(RCC_PLLM_DIV4)
    RCC_OscInitStruct.PLL1.PLLM = RCC_PLLM_DIV4;
#else
    RCC_OscInitStruct.PLL1.PLLM = 4;
#endif

    RCC_OscInitStruct.PLL1.PLLN = 150;

#if defined(RCC_PLLP_DIV4)
    RCC_OscInitStruct.PLL1.PLLP = RCC_PLLP_DIV4;
#else
    RCC_OscInitStruct.PLL1.PLLP = 4;
#endif

#if defined(RCC_PLLQ_DIV4)
    RCC_OscInitStruct.PLL1.PLLQ = RCC_PLLQ_DIV4;
#else
    RCC_OscInitStruct.PLL1.PLLQ = 4;
#endif

#if defined(RCC_PLLR_DIV4)
    RCC_OscInitStruct.PLL1.PLLR = RCC_PLLR_DIV4;
#else
    RCC_OscInitStruct.PLL1.PLLR = 4;
#endif

#if defined(RCC_PLLVCIRANGE_3)
    RCC_OscInitStruct.PLL1.PLLRGE = RCC_PLLVCIRANGE_3;
#endif

#if defined(RCC_PLLVCOWIDE)
    RCC_OscInitStruct.PLL1.PLLVCOSEL = RCC_PLLVCOWIDE;
#endif

#if defined(RCC_PLLFRACN_DISABLE)
    RCC_OscInitStruct.PLL1.PLLFRACN = 0;
#endif

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while (1);
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK  |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

#if defined(RCC_SYSCLK_DIV1)
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
#endif

#if defined(RCC_HCLK_DIV2)
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
#else
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
#endif

    /*
     * SYSCLK = 600MHz
     * HCLK   = 300MHz
     * PCLK1  = 150MHz
     * PCLK2  = 150MHz
     */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

#if defined(FLASH_LATENCY_6)
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
#elif defined(FLASH_LATENCY_5)
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
#else
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
#endif
    {
        while (1);
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

#endif // STM32H7RS