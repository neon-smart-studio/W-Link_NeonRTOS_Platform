#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#ifdef STM32G0

void SysCtrl_Init()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /*
     * STM32G0 common
     * HSE = 8MHz
     * SYSCLK = 64MHz
     *
     * 8MHz / 1 * 16 / 2 = 64MHz
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;

    RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

#if defined(RCC_PLLM_DIV1)
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
#endif

    RCC_OscInitStruct.PLL.PLLN = 16;

#if defined(RCC_PLLP_DIV2)
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
#endif

#if defined(RCC_PLLQ_DIV2) && defined(RCC_PLLCFGR_PLLQ)
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
#endif

#if defined(RCC_PLLR_DIV2)
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
#endif

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while (1);
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1;

    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

#if defined(FLASH_LATENCY_2)
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
#else
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
#endif
    {
        while (1);
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

#endif // STM32G0