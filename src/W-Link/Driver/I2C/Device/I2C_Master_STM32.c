#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"

#include "NeonRTOS.h"

#include "I2C/I2C_Master.h"

#ifdef DEVICE_STM32

#include "I2C_Master_STM32.h"
#include "I2C/I2C_Pin.h"
#include "GPIO/Device/GPIO_STM32.h"

bool I2C_Master_Init_Status[hwI2C_Index_MAX] = {false};

static hwI2C_Speed_Mode I2C_Clock_Speed_Mode[hwI2C_Index_MAX] = {hwI2C_Standard_Mode};
static NeonRTOS_SyncObj_t I2C_Master_Done_SyncHandle[hwI2C_Index_MAX];

#ifdef STM32F0
#define TIMING_VAL_48M_CLK_100KHZ  0x10805E89  // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_48M_CLK_400KHZ  0x00901850  // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_48M_CLK_1MHZ    0x00700818  // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_48M               48000000    // 48 MHz    
#endif //STM32F0

#ifdef STM32F7
#define TIMING_VAL_54M_CLK_100KHZ  0x10916998  // Standard mode with Rise time = 120ns, Fall time = 120ns
#define TIMING_VAL_54M_CLK_400KHZ  0x00B11B54  // Fast Mode with Rise time = 120ns, Fall time = 120ns
#define TIMING_VAL_54M_CLK_1MHZ    0x0090091B  // Fast Mode Plus with Rise time = 120ns, Fall time = 10ns
#define I2C_PCLK_54M               54000000    // 54 MHz
#endif //STM32F7

#ifdef STM32F3
#define TIMING_VAL_64M_CLK_100KHZ  0x10B17DB4  // Standard mode with Rise time = 120ns, Fall time = 120ns
#define TIMING_VAL_64M_CLK_400KHZ  0x00E22163  // Fast Mode with Rise time = 120ns, Fall time = 120ns
#define TIMING_VAL_64M_CLK_1MHZ    0x00A00D1E  // Fast Mode Plus with Rise time = 120ns, Fall time = 10ns
#define I2C_PCLK_64M               64000000    // 64 MHz

#define TIMING_VAL_72M_CLK_100KHZ  0x10D28DCB  // Standard mode with Rise time = 120ns, Fall time = 120ns
#define TIMING_VAL_72M_CLK_400KHZ  0x00F32571  // Fast Mode with Rise time = 120ns, Fall time = 120ns
#define TIMING_VAL_72M_CLK_1MHZ    0x00C00D24  // Fast Mode Plus with Rise time = 120ns, Fall time = 10ns
#define I2C_PCLK_72M               72000000    // 72 MHz
#endif //STM32F3

#ifdef STM32G0
#define TIMING_VAL_64M_CLK_100KHZ  0xC0311319  // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_64M_CLK_400KHZ  0x10B1102E  // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_64M_CLK_1MHZ    0x00710B1E  // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_64M               64000000    // 64 MHz
#endif //STM32G0

#ifdef STM32G4
#define TIMING_VAL_160M_CLK_100KHZ  0x90A13E56  // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_160M_CLK_400KHZ  0x30D2153A  // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_160M_CLK_1MHZ    0x10830F28  // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_160M               160000000   // 160 MHz
#endif //STM32G4

#ifdef STM32H7
#define TIMING_VAL_140M_CLK_100KHZ 0x70B1445E   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_140M_CLK_400KHZ 0x30C11233   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_140M_CLK_1MHZ   0x00F41B47   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_140M              140000000    // 140 MHz

#define TIMING_VAL_137M5_CLK_100KHZ 0x70B1445B   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_137M5_CLK_400KHZ 0x80510815   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_137M5_CLK_1MHZ   0x00F41A46   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_137M5              137500000    // 137.5 MHz

#define TIMING_VAL_120M_CLK_100KHZ 0x40E15676   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_120M_CLK_400KHZ 0x20C11434   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_120M_CLK_1MHZ   0x00C31536   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_120M              120000000    // 120 MHz

#define TIMING_VAL_112M5_CLK_100KHZ 0x80813142   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_112M5_CLK_400KHZ 0x20D11337   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_112M5_CLK_1MHZ   0x00C31538   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_112M5              112500000    // 112.5 MHz

#define TIMING_VAL_100M_CLK_100KHZ 0x50A14159   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_100M_CLK_400KHZ 0x20B11130   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_100M_CLK_1MHZ   0x00A21232   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_100M              100000000    // 100 MHz
#endif //STM32H7

#ifdef STM32L0
#define TIMING_VAL_32M_CLK_100KHZ  0x20602938  // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_32M_CLK_400KHZ  0x00B0122A  // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_32M_CLK_1MHZ    0x0030040E  // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_32M               32000000    // 32 MHz
#endif //STM32L0

#ifdef STM32L4
#define TIMING_VAL_80M_CLK_100KHZ 0x30C14E6B   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_80M_CLK_400KHZ 0x10D1143A   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_80M_CLK_1MHZ   0x00810E27   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_80M              80000000     // 80 MHz

#define TIMING_VAL_48M_CLK_100KHZ 0x20A03E55   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_48M_CLK_400KHZ 0x10800C21   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_48M_CLK_1MHZ   0x00500816   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_48M              48000000     // 48 MHz

#define TIMING_VAL_120M_CLK_100KHZ 0x107075B0  // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_120M_CLK_400KHZ 0x00501E6C  // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_120M_CLK_1MHZ   0x00200A26  // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_120M              120000000   // 120 MHz
#endif //STM32L4

#ifdef STM32L5
#define TIMING_VAL_110M_CLK_100KHZ 0x40E15676   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_110M_CLK_400KHZ 0x20C11434   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_110M_CLK_1MHZ   0x00C31536   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_110M              110000000    // 110 MHz
#endif //STM32L5

#ifdef STM32U5
#define TIMING_VAL_160M_CLK_100KHZ 0x90A13E56   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_160M_CLK_400KHZ 0x30D2153A   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_160M_CLK_1MHZ   0x10830F28   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_160M              160000000    // 160 MHz
#endif //STM32U5

#ifdef STM32WB
#define TIMING_VAL_64M_CLK_100KHZ 0x10707DBC   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_64M_CLK_400KHZ 0x00602173   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_64M_CLK_1MHZ   0x00300B29   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_64M              64000000     // 64 MHz

#define TIMING_VAL_32M_CLK_100KHZ 0x00707CBB   // Standard mode with Rise Time = 400ns and Fall Time = 100ns
#define TIMING_VAL_32M_CLK_400KHZ 0x00300F38   // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_32M_CLK_1MHZ   0x00100413   // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_32M              32000000     // 32 MHz
#endif //STM32WB

#ifdef STM32WL
#define TIMING_VAL_48M_CLK_100KHZ  0x20E03F53  // Standard mode with Rise Time = 640ns and Fall Time = 20ns
#define TIMING_VAL_48M_CLK_400KHZ  0x20500817  // Fast mode with Rise Time = 250ns and Fall Time = 100ns
#define TIMING_VAL_48M_CLK_1MHZ    0x00500A18  // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
#define I2C_PCLK_48M               48000000    // 48 MHz
#endif //STM32WL

uint32_t I2C_Master_Get_Timing(hwI2C_Index index, hwI2C_Speed_Mode speed_mode)
{
    uint32_t tim = 0;

    uint32_t pclk = I2C_Get_PCLK(index);

    switch (pclk) {
#if defined (I2C_PCLK_32M)
        case I2C_PCLK_32M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_32M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_32M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_32M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_48M)
        case I2C_PCLK_48M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_48M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_48M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_48M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_54M)
        case I2C_PCLK_54M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_54M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_54M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_54M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined(I2C_PCLK_64M)
        case I2C_PCLK_64M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_64M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_64M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_64M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_72M)
        case I2C_PCLK_72M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_72M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_72M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_72M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_80M)
        case I2C_PCLK_80M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_80M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_80M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_80M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_100M)
        case I2C_PCLK_100M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_100M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_100M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_100M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_110M)
        case I2C_PCLK_110M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_110M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_110M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_110M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_112M5)
        case I2C_PCLK_112M5:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_112M5_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_112M5_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_112M5_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_120M)
        case I2C_PCLK_120M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_120M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_120M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_120M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_137M5)
        case I2C_PCLK_137M5:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_137M5_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_137M5_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_137M5_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_140M)
        case I2C_PCLK_140M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_140M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_140M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_140M_CLK_1MHZ;
                    break;
            }
            break;
#endif
#if defined (I2C_PCLK_160M)
        case I2C_PCLK_160M:
            switch (speed_mode) {
                case hwI2C_Standard_Mode:
                    tim = TIMING_VAL_160M_CLK_100KHZ;
                    break;
                case hwI2C_Fast_Mode:
                    tim = TIMING_VAL_160M_CLK_400KHZ;
                    break;
                case hwI2C_High_Speed_Mode:
                    tim = TIMING_VAL_160M_CLK_1MHZ;
                    break;
            }
            break;
#endif
    }
    
    return tim;
}

#ifndef STM32F1
uint32_t STM32_I2C_GetAF(hwI2C_Index i2c, hwGPIO_Pin pin)
{
    for (size_t i = 0; i < sizeof(I2C_Pin_AF_Map) / sizeof(I2C_Pin_AF_Map[0]); i++) {
        if (I2C_Pin_AF_Map[i].i2c == i2c &&
            I2C_Pin_AF_Map[i].pin == pin) {
            return I2C_Pin_AF_Map[i].af;
        }
    }
    return 0;
}
#endif

hwI2C_Index I2C_IndexFromHandle(I2C_HandleTypeDef *hI2C)
{
    for (int i = 0; i < hwI2C_Index_MAX; i++) {
        if (&g_i2c[i] == hI2C) {
            return (hwI2C_Index)i;
        }
    }
    return hwI2C_Index_MAX;
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c_cb)
{
    hwI2C_Index index = I2C_IndexFromHandle(hi2c_cb);
    if (index < hwI2C_Index_MAX) {
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c_cb)
{
    hwI2C_Index index = I2C_IndexFromHandle(hi2c_cb);
    if (index < hwI2C_Index_MAX) {
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c_cb)
{
    hwI2C_Index index = I2C_IndexFromHandle(hi2c_cb);
    if (index < hwI2C_Index_MAX) {
        NeonRTOS_SyncObjSignalFromISR(&I2C_Master_Done_SyncHandle[index]);
    }
}

hwI2C_OpResult I2C_Master_Init(hwI2C_Index index, hwI2C_Speed_Mode speed_mode)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (I2C_Master_Init_Status[index]) {
        return hwI2C_OK;
    }

    if (speed_mode >= hwI2C_Speed_Mode_MAX) {
        return hwI2C_InvalidParameter;
    }

    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]].sda_pin;
    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]].scl_pin;

    GPIO_TypeDef *sda_soc_base = GPIO_Map_Soc_Base(sda_pin);
    GPIO_TypeDef *scl_soc_base = GPIO_Map_Soc_Base(scl_pin);
    
    uint16_t sda_soc_pin = GPIO_Map_Soc_Pin(sda_pin);
    uint16_t scl_soc_pin = GPIO_Map_Soc_Pin(scl_pin);

    if (sda_soc_pin == 0 || sda_soc_base == NULL ||
        scl_soc_pin == 0 || scl_soc_base == NULL) {
        return hwI2C_InvalidParameter;
    }

#ifndef STM32F1
    uint32_t sda_af = STM32_I2C_GetAF(index, sda_pin);
    uint32_t scl_af = STM32_I2C_GetAF(index, scl_pin);

    if (sda_af == 0 || scl_af == 0) {
        return hwI2C_InvalidParameter;
    }
#endif

    if (NeonRTOS_SyncObjCreate(&I2C_Master_Done_SyncHandle[index]) != NeonRTOS_OK) {
        return hwI2C_MemoryError;
    }

    GPIO_Enable_RCC_Clock(sda_soc_base);
    GPIO_Enable_RCC_Clock(scl_soc_base);

    GPIO_InitTypeDef g_i2c_sda = {0};
    g_i2c_sda.Pin       = sda_soc_pin;
    g_i2c_sda.Mode      = GPIO_MODE_AF_OD;
    g_i2c_sda.Pull      = GPIO_PULLUP;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_i2c_sda.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_i2c_sda.Speed     = GPIO_SPEED_HIGH;
#endif
#ifndef STM32F1
    g_i2c_sda.Alternate = sda_af;
#endif
    HAL_GPIO_Init(sda_soc_base, &g_i2c_sda);

    GPIO_InitTypeDef g_i2c_scl = {0};
    g_i2c_scl.Pin       = scl_soc_pin;
    g_i2c_scl.Mode      = GPIO_MODE_AF_OD;
    g_i2c_scl.Pull      = GPIO_PULLUP;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_i2c_scl.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_i2c_scl.Speed     = GPIO_SPEED_HIGH;
#endif
#ifndef STM32F1
    g_i2c_scl.Alternate = scl_af;
#endif
    HAL_GPIO_Init(scl_soc_base, &g_i2c_scl);

    hwI2C_OpResult result;

#ifdef STM32F1
    result = I2C_ApplyRemap(
        index,
        scl_pin,
        sda_pin
    );

    if (result != hwI2C_OK) {
        NeonRTOS_SyncObjDelete(&I2C_Master_Done_SyncHandle[index]);
        return result;
    }
#endif

    result = I2C_Instance_Init(index, speed_mode);
    if(result != hwI2C_OK) {
        NeonRTOS_SyncObjDelete(&I2C_Master_Done_SyncHandle[index]);
        return result;
    }

    I2C_NVIC_Init(index);

    gpio_pin_init_status[sda_pin] = true;
    gpio_pin_init_status[scl_pin] = true;

    I2C_Clock_Speed_Mode[index] = speed_mode;
    I2C_Master_Init_Status[index] = true;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_DeInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (I2C_Master_Init_Status[index] == false) {
        return hwI2C_OK;
    }

    hwGPIO_Pin sda_pin = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]].sda_pin;
    hwGPIO_Pin scl_pin = I2C_Pin_Def_Table[index][I2C_Index_Map_Alt[index]].scl_pin;

    GPIO_TypeDef *sda_soc_base = GPIO_Map_Soc_Base(sda_pin);
    GPIO_TypeDef *scl_soc_base = GPIO_Map_Soc_Base(scl_pin);
    
    uint16_t sda_soc_pin = GPIO_Map_Soc_Pin(sda_pin);
    uint16_t scl_soc_pin = GPIO_Map_Soc_Pin(scl_pin);

    if (sda_soc_pin == 0 || sda_soc_base == NULL ||
        scl_soc_pin == 0 || scl_soc_base == NULL) {
        return hwI2C_InvalidParameter;
    }

    I2C_Master_Init_Status[index] = false;

    I2C_NVIC_DeInit(index);

    I2C_Instance_DeInit(index);

#ifdef STM32F1
    I2C_RestoreRemap(index);
#endif

    NeonRTOS_SyncObjDelete(&I2C_Master_Done_SyncHandle[index]);

    HAL_GPIO_DeInit(sda_soc_base, sda_soc_pin);
    HAL_GPIO_DeInit(scl_soc_base, scl_soc_pin);

    gpio_pin_init_status[sda_pin] = false;
    gpio_pin_init_status[scl_pin] = false;

    return hwI2C_OK;
}

hwI2C_OpResult I2C_Master_Reset(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }
    if (I2C_Master_Init_Status[index] == false) {
        return hwI2C_NotInit;
    }

    hwI2C_OpResult op_status = I2C_Master_DeInit(index);
    if (op_status < hwI2C_OK) {
        return op_status;
    }

    return I2C_Master_Init(index, I2C_Clock_Speed_Mode[index]);
}

hwI2C_OpResult I2C_Master_Read(hwI2C_Index index, uint8_t address, uint8_t *read_dat, uint8_t read_len, bool stop, NeonRTOS_Time_t timeoutMs)
{
    (void)stop;

    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (I2C_Master_Init_Status[index] == false) {
        return hwI2C_NotInit;
    }

    if (read_dat == NULL && read_len > 0) {
        return hwI2C_InvalidParameter;
    }

    if (read_len == 0) {
        return hwI2C_InvalidParameter;
    }

    if (HAL_I2C_Master_Receive_IT(&g_i2c[index], address << 1, read_dat, read_len) != HAL_OK) {
        return hwI2C_BusError;
    }

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK) {
        HAL_I2C_Master_Abort_IT(&g_i2c[index], address << 1);
        return hwI2C_SlaveTimeout;
    }

    return (g_i2c[index].ErrorCode == HAL_I2C_ERROR_NONE) ? hwI2C_OK : hwI2C_BusError;
}

hwI2C_OpResult I2C_Master_Write(hwI2C_Index index, uint8_t address, uint8_t *write_dat, uint8_t write_len, bool stop, NeonRTOS_Time_t timeoutMs)
{
    (void)stop;

    if (index >= hwI2C_Index_MAX) {
        return hwI2C_InvalidParameter;
    }

    if (I2C_Master_Init_Status[index] == false) {
        return hwI2C_NotInit;
    }

    if (write_dat == NULL && write_len > 0) {
        return hwI2C_InvalidParameter;
    }

    if (write_len == 0) {
        return hwI2C_InvalidParameter;
    }

    if (HAL_I2C_Master_Transmit_IT(&g_i2c[index], address << 1, write_dat, write_len) != HAL_OK) {
        return hwI2C_BusError;
    }

    if (NeonRTOS_SyncObjWait(&I2C_Master_Done_SyncHandle[index], timeoutMs) != NeonRTOS_OK) {
        HAL_I2C_Master_Abort_IT(&g_i2c[index], address << 1);
        return hwI2C_SlaveTimeout;
    }

    return (g_i2c[index].ErrorCode == HAL_I2C_ERROR_NONE) ? hwI2C_OK : hwI2C_BusError;
}

bool I2C_Master_isInit(hwI2C_Index index)
{
    if (index >= hwI2C_Index_MAX) {
        return false;
    }
    return I2C_Master_Init_Status[index];
}

#endif // DEVICE_STM32