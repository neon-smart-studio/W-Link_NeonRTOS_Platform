#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#include "NeonRTOS.h"

#ifdef STM32G4

#include "I2C/I2C_Master.h"
#include "I2C_Master_STM32.h"
#include "GPIO/Device/GPIO_STM32.h"

I2C_HandleTypeDef g_i2c[hwI2C_Index_MAX];

uint32_t I2C_Get_PCLK(hwI2C_Index index)
{
    uint32_t pclk = HAL_RCC_GetPCLK1Freq();

#if defined(I2C1_BASE)
    if (index == hwI2C_Index_0) {
        uint32_t clocksource = __HAL_RCC_GET_I2C1_SOURCE();

        #if defined(RCC_I2C1CLKSOURCE_PCLK1)
        if (clocksource == RCC_I2C1CLKSOURCE_PCLK1) {
            return HAL_RCC_GetPCLK1Freq();
        }
        #endif

        #if defined(RCC_I2C1CLKSOURCE_SYSCLK)
        if (clocksource == RCC_I2C1CLKSOURCE_SYSCLK) {
            return HAL_RCC_GetSysClockFreq();
        }
        #endif

        #if defined(RCC_I2C1CLKSOURCE_HSI)
        if (clocksource == RCC_I2C1CLKSOURCE_HSI) {
            return HSI_VALUE;
        }
        #endif

        return HAL_RCC_GetPCLK1Freq();
    }
#endif

#if defined(I2C2_BASE)
    if (index == hwI2C_Index_1) {
        uint32_t clocksource = __HAL_RCC_GET_I2C2_SOURCE();

        #if defined(RCC_I2C2CLKSOURCE_PCLK1)
        if (clocksource == RCC_I2C2CLKSOURCE_PCLK1) {
            return HAL_RCC_GetPCLK1Freq();
        }
        #endif

        #if defined(RCC_I2C2CLKSOURCE_SYSCLK)
        if (clocksource == RCC_I2C2CLKSOURCE_SYSCLK) {
            return HAL_RCC_GetSysClockFreq();
        }
        #endif

        #if defined(RCC_I2C2CLKSOURCE_HSI)
        if (clocksource == RCC_I2C2CLKSOURCE_HSI) {
            return HSI_VALUE;
        }
        #endif

        return HAL_RCC_GetPCLK1Freq();
    }
#endif

#if defined(I2C3_BASE)
    if (index == hwI2C_Index_2) {
        uint32_t clocksource = __HAL_RCC_GET_I2C3_SOURCE();

#if defined(RCC_I2C3CLKSOURCE_PCLK1)
        if (clocksource == RCC_I2C3CLKSOURCE_PCLK1)
            return HAL_RCC_GetPCLK1Freq();
#endif

#if defined(RCC_I2C3CLKSOURCE_SYSCLK)
        if (clocksource == RCC_I2C3CLKSOURCE_SYSCLK)
            return HAL_RCC_GetSysClockFreq();
#endif

#if defined(RCC_I2C3CLKSOURCE_HSI)
        if (clocksource == RCC_I2C3CLKSOURCE_HSI)
            return HSI_VALUE;
#endif

        return HAL_RCC_GetPCLK1Freq();
    }
#endif

    return pclk;
}

hwI2C_Index I2C_IndexFromHandle(I2C_HandleTypeDef *hI2C)
{
    for (int i = 0; i < hwI2C_Index_MAX; i++) {
        if (&g_i2c[i] == hI2C) {
            return (hwI2C_Index)i;
        }
    }
    return hwI2C_Index_MAX;
}

I2C_TypeDef *I2C_Map_Soc_Base(hwI2C_Index index)
{
    switch (index)
    {
#if defined(I2C1_BASE)
        case hwI2C_Index_0: return I2C1;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_1: return I2C2;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_2: return I2C3;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_3: return I2C4;
#endif
        default: break;
    }
    return NULL;
}

static void I2C_EnableClock(hwI2C_Index index)
{
    switch (index)
    {
#if defined(I2C1_BASE)
        case hwI2C_Index_0:
            __HAL_RCC_I2C1_CLK_ENABLE();
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_1:
            __HAL_RCC_I2C2_CLK_ENABLE();
            break;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_2:
            __HAL_RCC_I2C3_CLK_ENABLE();
            break;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_3:
            __HAL_RCC_I2C4_CLK_ENABLE();
            break;
#endif
        default:
            return hwI2C_InvalidParameter;
    }
}

static void I2C_DisableClock(hwI2C_Index index)
{
    switch (index)
    {
#if defined(I2C1_BASE)
        case hwI2C_Index_0:
            __HAL_RCC_I2C1_CLK_DISABLE();
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_1:
            __HAL_RCC_I2C2_CLK_DISABLE();
            break;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_2:
            __HAL_RCC_I2C3_CLK_DISABLE();
            break;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_3:
            __HAL_RCC_I2C4_CLK_DISABLE();
            break;
#endif
        default:
            break;
    }
}

static void I2C_HAL_EV_IRQHandler(hwI2C_Index index)
{
    HAL_I2C_EV_IRQHandler(&g_i2c[index]);
}

static void I2C_HAL_ER_IRQHandler(hwI2C_Index index)
{
    HAL_I2C_ER_IRQHandler(&g_i2c[index]);
}

#if defined(I2C1_BASE)
void I2C1_EV_IRQHandler(void)
{
    I2C_HAL_EV_IRQHandler(hwI2C_Index_0);
}
void I2C1_ER_IRQHandler(void)
{
    I2C_HAL_ER_IRQHandler(hwI2C_Index_0);
}
#endif

#if defined(I2C2_BASE)
void I2C2_EV_IRQHandler(void)
{
    I2C_HAL_EV_IRQHandler(hwI2C_Index_1);
}
void I2C2_ER_IRQHandler(void)
{
    I2C_HAL_ER_IRQHandler(hwI2C_Index_1);
}
#endif

#if defined(I2C3_BASE)
void I2C3_EV_IRQHandler(void)
{
    I2C_HAL_EV_IRQHandler(hwI2C_Index_2);
}
void I2C3_ER_IRQHandler(void)
{
    I2C_HAL_ER_IRQHandler(hwI2C_Index_2);
}
#endif

#if defined(I2C4_BASE)
void I2C4_EV_IRQHandler(void)
{
    I2C_HAL_EV_IRQHandler(hwI2C_Index_3);
}
void I2C4_ER_IRQHandler(void)
{
    I2C_HAL_ER_IRQHandler(hwI2C_Index_3);
}
#endif

hwI2C_OpResult I2C_Instance_Init(hwI2C_Index index, hwI2C_Speed_Mode speed_mode)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    I2C_TypeDef *i2c_soc_base = I2C_Map_Soc_Base(index);
    if (i2c_soc_base == NULL) {
        return hwI2C_InvalidParameter;
    }

    I2C_EnableClock(index);

    g_i2c[index].Instance             = i2c_soc_base;
    g_i2c[index].Init.Timing          = I2C_Master_Get_Timing(index, speed_mode);
    g_i2c[index].Init.OwnAddress1     = 0;
    g_i2c[index].Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    g_i2c[index].Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    g_i2c[index].Init.OwnAddress2     = 0;
    g_i2c[index].Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    g_i2c[index].Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    g_i2c[index].Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    if (g_i2c[index].Init.Timing == 0) {
        return hwI2C_InvalidParameter;
    }

    if (HAL_I2C_Init(&g_i2c[index]) != HAL_OK) {
        return hwI2C_HwError;
    }

    HAL_I2CEx_ConfigAnalogFilter(&g_i2c[index], I2C_ANALOGFILTER_ENABLE);
    HAL_I2CEx_ConfigDigitalFilter(&g_i2c[index], 0);

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Instance_DeInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    HAL_I2C_DeInit(&g_i2c[index]);

    I2C_DisableClock(index);

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Instance_Read(hwI2C_Index index, uint8_t address, uint8_t *read_dat, uint8_t read_len, bool stop)
{
    if (HAL_I2C_Master_Receive_IT(&g_i2c[index], address << 1, read_dat, read_len) != HAL_OK) {
        return hwI2C_HwError;
    }

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Instance_Write(hwI2C_Index index, uint8_t address, uint8_t *write_dat, uint8_t write_len, bool stop)
{
    if (HAL_I2C_Master_Transmit_IT(&g_i2c[index], address << 1, write_dat, write_len) != HAL_OK) {
        return hwI2C_HwError;
    }
    
    return hwI2C_OK;
}

hwI2C_OpResult I2C_Instance_Abort(hwI2C_Index index, uint8_t address)
{
    if (HAL_I2C_Master_Abort_IT(&g_i2c[index], address << 1) != HAL_OK) {
        return hwI2C_HwError;
    }

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Instance_CheckError(hwI2C_Index index)
{
    return (g_i2c[index].ErrorCode == HAL_I2C_ERROR_NONE) ? hwI2C_OK : hwI2C_BusError;
}

void I2C_NVIC_Init(hwI2C_Index index)
{
    switch (index)
    {
#if defined(I2C1_BASE)
        case hwI2C_Index_0:
            HAL_NVIC_SetPriority(I2C1_EV_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
            HAL_NVIC_SetPriority(I2C1_ER_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_1:
            HAL_NVIC_SetPriority(I2C2_EV_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
            HAL_NVIC_SetPriority(I2C2_ER_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_2:
            HAL_NVIC_SetPriority(I2C3_EV_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
            HAL_NVIC_SetPriority(I2C3_ER_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_3:
            HAL_NVIC_SetPriority(I2C4_EV_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
            HAL_NVIC_SetPriority(I2C4_ER_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C4_ER_IRQn);
            break;
#endif
        default:
            break;
    }
}

void I2C_NVIC_DeInit(hwI2C_Index index)
{
    switch (index)
    {
#if defined(I2C1_BASE)
        case hwI2C_Index_0:
            HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
            HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
            break;
#endif
#if defined(I2C2_BASE)
        case hwI2C_Index_1:
            HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
            HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
            break;
#endif
#if defined(I2C3_BASE)
        case hwI2C_Index_2:
            HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
            HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
            break;
#endif
#if defined(I2C4_BASE)
        case hwI2C_Index_3:
            HAL_NVIC_DisableIRQ(I2C4_EV_IRQn);
            HAL_NVIC_DisableIRQ(I2C4_ER_IRQn);
            break;
#endif

        default:
            break;
    }
}

#endif // STM32G4