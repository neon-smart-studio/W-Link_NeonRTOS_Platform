#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "soc.h"

#include "NeonRTOS.h"

#ifdef STM32F1

#include "I2C/I2C_Master.h"
#include "I2C/I2C_Pin.h"
#include "I2C_Master_STM32.h"
#include "GPIO/Device/GPIO_STM32.h"

I2C_HandleTypeDef g_i2c[hwI2C_Index_MAX];

uint32_t I2C_Get_PCLK(hwI2C_Index index)
{
    return 0;
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

        default:
            break;
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

        default:
            break;
    }
}

// UART IRQ 統一入口
static void I2C_HAL_EV_IRQHandler(hwI2C_Index index)
{
    HAL_I2C_EV_IRQHandler(&g_i2c[index]);
}

// UART IRQ 統一入口
static void I2C_HAL_ER_IRQHandler(hwI2C_Index index)
{
    HAL_I2C_ER_IRQHandler(&g_i2c[index]);
}

#if defined (I2C1_BASE)
void I2C1_EV_IRQHandler(void){ I2C_HAL_EV_IRQHandler(hwI2C_Index_0); }
void I2C1_ER_IRQHandler(void){ I2C_HAL_ER_IRQHandler(hwI2C_Index_0); }
#endif
#if defined (I2C2_BASE)
void I2C2_EV_IRQHandler(void){ I2C_HAL_EV_IRQHandler(hwI2C_Index_1); }
void I2C2_ER_IRQHandler(void){ I2C_HAL_ER_IRQHandler(hwI2C_Index_1); }
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

    g_i2c[index].Instance = i2c_soc_base;
    switch(speed_mode)
    {
        case hwI2C_Standard_Mode:
            g_i2c[index].Init.ClockSpeed = I2C_MASTER_STANDARD_MODE_CLK_FREQUENCY;
            break;
        case hwI2C_Fast_Mode:
            g_i2c[index].Init.ClockSpeed = I2C_MASTER_FAST_MODE_CLK_FREQUENCY;
            break;
    }
    g_i2c[index].Init.OwnAddress1 = 0;
    g_i2c[index].Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    g_i2c[index].Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    g_i2c[index].Init.OwnAddress2 = 0;
    g_i2c[index].Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    g_i2c[index].Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&g_i2c[index]) != HAL_OK) {
        return hwI2C_HwError;
    }

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

void I2C_NVIC_Init(hwI2C_Index index)
{
    switch (index)
    {
#if defined (I2C1_BASE)
        case hwI2C_Index_0:
            HAL_NVIC_SetPriority(I2C1_EV_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
            HAL_NVIC_SetPriority(I2C1_ER_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
            break;
#endif
#if defined (I2C2_BASE)
        case hwI2C_Index_1:
            HAL_NVIC_SetPriority(I2C2_EV_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
            HAL_NVIC_SetPriority(I2C2_ER_IRQn, I2C_IRQ_NVIC_PRIORITY, I2C_IRQ_NVIC_SUB_PRIORITY);
            HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
            break;
#endif

        default:
            break;
    }
}

void I2C_NVIC_DeInit(hwI2C_Index index)
{
    switch(index)
    {       
#if defined (I2C1_BASE)
        case hwI2C_Index_0:
            HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
            HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
            break;
#endif
#if defined (I2C2_BASE)
        case hwI2C_Index_1:
            HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
            HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
            break;
#endif

        default:
            break;
    }
}

hwI2C_OpResult I2C_ApplyRemap(
    hwI2C_Index index,
    hwGPIO_Pin scl_pin,
    hwGPIO_Pin sda_pin
)
{
    __HAL_RCC_AFIO_CLK_ENABLE();

    hwGPIO_Pin default_scl_pin = I2C_Pin_Def_Table[index][I2C_Pinset_DEFAULT].scl_pin;
    hwGPIO_Pin default_sda_pin = I2C_Pin_Def_Table[index][I2C_Pinset_DEFAULT].sda_pin;

    hwGPIO_Pin alt_scl_pin = I2C_Pin_Def_Table[index][I2C_Pinset_ALT].scl_pin;
    hwGPIO_Pin alt_sda_pin = I2C_Pin_Def_Table[index][I2C_Pinset_ALT].sda_pin;

    bool is_default =
        scl_pin == default_scl_pin &&
        sda_pin == default_sda_pin;

    bool is_alt =
        scl_pin == alt_scl_pin &&
        sda_pin == alt_sda_pin;

    switch (index)
    {
#if defined(I2C1_BASE)
        case hwI2C_Index_0:
            if (is_default) {
#if defined(__HAL_AFIO_REMAP_I2C1_DISABLE)
                __HAL_AFIO_REMAP_I2C1_DISABLE();
#endif
                return hwI2C_OK;
            }

            if (is_alt) {
#if defined(__HAL_AFIO_REMAP_I2C1_ENABLE)
                __HAL_AFIO_REMAP_I2C1_ENABLE();
                return hwI2C_OK;
#else
                return hwI2C_InvalidParameter;
#endif
            }

            return hwI2C_InvalidParameter;
#endif

#if defined(I2C2_BASE)
        case hwI2C_Index_1:
            if (is_default) {
                return hwI2C_OK;
            }

            /*
             * STM32F1 I2C2 通常沒有 AFIO remap。
             */
            if (is_alt) {
#if defined(__HAL_AFIO_REMAP_I2C2_ENABLE)
                __HAL_AFIO_REMAP_I2C2_ENABLE();
                return hwI2C_OK;
#else
                return hwI2C_InvalidParameter;
#endif
            }

            return hwI2C_InvalidParameter;
#endif

        default:
            return hwI2C_OK;
    }
}

void I2C_RestoreRemap(hwI2C_Index index)
{
    switch (index)
    {
#if defined(I2C1_BASE)
        case hwI2C_Index_0:
#if defined(__HAL_AFIO_REMAP_I2C1_DISABLE)
            __HAL_AFIO_REMAP_I2C1_DISABLE();
#endif
            break;
#endif

#if defined(I2C2_BASE)
        case hwI2C_Index_1:
#if defined(__HAL_AFIO_REMAP_I2C2_DISABLE)
            __HAL_AFIO_REMAP_I2C2_DISABLE();
#endif
            break;
#endif

        default:
            break;
    }

    __HAL_RCC_AFIO_CLK_DISABLE();
}

#endif // STM32F1
