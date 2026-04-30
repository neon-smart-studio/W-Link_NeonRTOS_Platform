#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "SPI/SPI_Master.h"

#ifdef DEVICE_STM32

#include "SPI_Master_STM32.h"
#include "SPI/SPI_Pin.h"
#include "GPIO/Device/GPIO_STM32.h"

#define SPI_MASTER_SYNC_TIMEOUT             100
#define SPI_MASTER_MUTEX_ACCESS_TIMEOUT     500
#define SPI_MASTER_OP_TIMEOUT               3000

NeonRTOS_LockObj_t Spi_Master_Access_Mutex[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Send_SyncHandle[hwSPI_Index_MAX];
NeonRTOS_SyncObj_t Spi_Master_Recv_SyncHandle[hwSPI_Index_MAX];

#define SPI_MASTER_MUTEX_LOCK(a, b)  if (NeonRTOS_LockObjLock(&Spi_Master_Access_Mutex[a], b) != NeonRTOS_OK) { return hwSPI_MutexTimeout; }
#define SPI_MASTER_MUTEX_UNLOCK(a)   NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[a])

bool Spi_Master_Init_Status[hwSPI_Index_MAX] = {false};

static bool Spi_Master_Use_CS[hwSPI_Index_MAX] = {false};

static const uint32_t Spi_Master_Baudrate_Prescaler_Table[] = {
    SPI_BAUDRATEPRESCALER_2,
    SPI_BAUDRATEPRESCALER_4,
    SPI_BAUDRATEPRESCALER_8,
    SPI_BAUDRATEPRESCALER_16,
    SPI_BAUDRATEPRESCALER_32,
    SPI_BAUDRATEPRESCALER_64,
    SPI_BAUDRATEPRESCALER_128,
    SPI_BAUDRATEPRESCALER_256
};

#ifndef STM32F1
uint32_t STM32_SPI_GetAF(hwSPI_Index spi, hwGPIO_Pin pin)
{
    for (size_t i = 0; i < sizeof(SPI_Pin_AF_Map) / sizeof(SPI_Pin_AF_Map[0]); i++) {
        if (SPI_Pin_AF_Map[i].spi == spi &&
            SPI_Pin_AF_Map[i].pin == pin) {
            return SPI_Pin_AF_Map[i].af;
        }
    }
    return 0;
}
#endif

hwSPI_Index SPI_IndexFromHandle(SPI_HandleTypeDef *hspi)
{
    for(int i=0;i<hwSPI_Index_MAX;i++)
    {
        if(&g_spi[i] == hspi)
            return (hwSPI_Index)i;
    }
    return hwSPI_Index_MAX;
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    hwSPI_Index idx = SPI_IndexFromHandle(hspi);
    if (idx >= hwSPI_Index_MAX) return;

    NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[idx]);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    hwSPI_Index idx = SPI_IndexFromHandle(hspi);
    if (idx >= hwSPI_Index_MAX) return;

    NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[idx]);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    hwSPI_Index idx = SPI_IndexFromHandle(hspi);
    if (idx >= hwSPI_Index_MAX) return;

    NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[idx]);
    NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[idx]);
}

hwSPI_OpResult SPI_Master_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode)
{
    if (opMode >= hwSPI_OpMode_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == true)
    {
        return hwSPI_OK;
    }

    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].miso_pin;
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].mosi_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].cs_pin;

    GPIO_TypeDef *miso_soc_base = GPIO_Map_Soc_Base(miso_pin);
    GPIO_TypeDef *mosi_soc_base = GPIO_Map_Soc_Base(mosi_pin);
    GPIO_TypeDef *sclk_soc_base = GPIO_Map_Soc_Base(sclk_pin);

    uint16_t miso_soc_pin = GPIO_Map_Soc_Pin(miso_pin);
    uint16_t mosi_soc_pin = GPIO_Map_Soc_Pin(mosi_pin);
    uint16_t sclk_soc_pin = GPIO_Map_Soc_Pin(sclk_pin);

    GPIO_TypeDef *cs_soc_base = NULL;
    uint16_t cs_soc_pin = 0;

    if (miso_soc_pin == 0 || miso_soc_base == NULL ||
        mosi_soc_pin == 0 || mosi_soc_base == NULL ||
        sclk_soc_pin == 0 || sclk_soc_base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (cs_pin != hwGPIO_Pin_NC)
    {
        cs_soc_base = GPIO_Map_Soc_Base(cs_pin);
        cs_soc_pin  = GPIO_Map_Soc_Pin(cs_pin);

        if (cs_soc_pin == 0 || cs_soc_base == NULL)
        {
            return hwSPI_InvalidParameter;
        }
    }

#ifndef STM32F1
    uint32_t mosi_af = STM32_SPI_GetAF(index, mosi_pin);
    uint32_t miso_af = STM32_SPI_GetAF(index, miso_pin);
    uint32_t sclk_af = STM32_SPI_GetAF(index, sclk_pin);
    uint32_t cs_af   = 0;

    if (mosi_af == 0 || miso_af == 0 || sclk_af == 0)
    {
        return hwSPI_InvalidParameter;
    }

    if (cs_pin != hwGPIO_Pin_NC)
    {
        cs_af = STM32_SPI_GetAF(index, cs_pin);
        if (cs_af == 0)
        {
            return hwSPI_InvalidParameter;
        }
    }
#endif

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Send_SyncHandle[index]) != NeonRTOS_OK)
    {
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&Spi_Master_Recv_SyncHandle[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        return hwSPI_MemoryError;
    }

    if (NeonRTOS_LockObjCreate(&Spi_Master_Access_Mutex[index]) != NeonRTOS_OK)
    {
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
        return hwSPI_MemoryError;
    }

    NeonRTOS_LockObjUnlock(&Spi_Master_Access_Mutex[index]);

    GPIO_Enable_RCC_Clock(miso_soc_base);
    GPIO_Enable_RCC_Clock(mosi_soc_base);
    GPIO_Enable_RCC_Clock(sclk_soc_base);

    if (cs_pin != hwGPIO_Pin_NC)
    {
        GPIO_Enable_RCC_Clock(cs_soc_base);
    }

    GPIO_InitTypeDef g_spi_miso = {0};
    g_spi_miso.Pin       = miso_soc_pin;
    g_spi_miso.Mode      = GPIO_MODE_AF_PP;
    g_spi_miso.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_spi_miso.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_spi_miso.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_spi_miso.Alternate = miso_af;
#endif
    HAL_GPIO_Init(miso_soc_base, &g_spi_miso);

    GPIO_InitTypeDef g_spi_mosi = {0};
    g_spi_mosi.Pin       = mosi_soc_pin;
    g_spi_mosi.Mode      = GPIO_MODE_AF_PP;
    g_spi_mosi.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_spi_mosi.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_spi_mosi.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_spi_mosi.Alternate = mosi_af;
#endif
    HAL_GPIO_Init(mosi_soc_base, &g_spi_mosi);

    GPIO_InitTypeDef g_spi_sclk = {0};
    g_spi_sclk.Pin       = sclk_soc_pin;
    g_spi_sclk.Mode      = GPIO_MODE_AF_PP;
    g_spi_sclk.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_spi_sclk.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_spi_sclk.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_spi_sclk.Alternate = sclk_af;
#endif
    HAL_GPIO_Init(sclk_soc_base, &g_spi_sclk);
    
    bool hw_cs = false;

    if (cs_pin != hwGPIO_Pin_NC)
    {
        hw_cs = true;

        GPIO_InitTypeDef g_spi_cs = {0};
        g_spi_cs.Pin       = cs_soc_pin;
        g_spi_cs.Mode      = GPIO_MODE_AF_PP;
        g_spi_cs.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
        g_spi_cs.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
        g_spi_cs.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
        g_spi_cs.Alternate = cs_af;
#endif
        HAL_GPIO_Init(cs_soc_base, &g_spi_cs);
    }

    hwSPI_OpResult result;

#ifdef STM32F1
    result = SPI_ApplyRemap(
        index,
        mosi_pin,
        miso_pin,
        sclk_pin,
        cs_pin,
        hw_cs
    );

    if (result != hwSPI_OK) {
        NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
        return result;
    }
#endif

    result = SPI_Instance_Init(index, clock_rate_hz, opMode);
    if (result != hwSPI_OK)
    {
        NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
        return result;
    }

    SPI_NVIC_Init(index);

    gpio_pin_init_status[miso_pin] = true;
    gpio_pin_init_status[mosi_pin] = true;
    gpio_pin_init_status[sclk_pin] = true;

    if (cs_pin != hwGPIO_Pin_NC)
    {
        gpio_pin_init_status[cs_pin] = true;
        Spi_Master_Use_CS[index] = true;
    }

    Spi_Master_Init_Status[index] = true;

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_DeInit(hwSPI_Index index)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_OK;
    }

    hwGPIO_Pin miso_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].miso_pin;
    hwGPIO_Pin mosi_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].mosi_pin;
    hwGPIO_Pin sclk_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].sclk_pin;
    hwGPIO_Pin cs_pin = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]].cs_pin;

    GPIO_TypeDef *miso_soc_base = GPIO_Map_Soc_Base(miso_pin);
    GPIO_TypeDef *mosi_soc_base = GPIO_Map_Soc_Base(mosi_pin);
    GPIO_TypeDef *sclk_soc_base = GPIO_Map_Soc_Base(sclk_pin);

    uint16_t miso_soc_pin = GPIO_Map_Soc_Pin(miso_pin);
    uint16_t mosi_soc_pin = GPIO_Map_Soc_Pin(mosi_pin);
    uint16_t sclk_soc_pin = GPIO_Map_Soc_Pin(sclk_pin);

    GPIO_TypeDef *cs_soc_base = NULL;
    uint16_t cs_soc_pin = 0;

    if (miso_soc_pin == 0 || miso_soc_base == NULL ||
        mosi_soc_pin == 0 || mosi_soc_base == NULL ||
        sclk_soc_pin == 0 || sclk_soc_base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Use_CS[index] == true)
    {
        cs_soc_base = GPIO_Map_Soc_Base(cs_pin);
        cs_soc_pin  = GPIO_Map_Soc_Pin(cs_pin);

        if (cs_soc_pin == 0 || cs_soc_base == NULL)
        {
            return hwSPI_InvalidParameter;
        }
    }

    Spi_Master_Init_Status[index] = false;

    SPI_NVIC_DeInit(index);

    SPI_Instance_DeInit(index);

#ifdef STM32F1
    SPI_RestoreRemap(index);
#endif

    HAL_GPIO_DeInit(miso_soc_base, miso_soc_pin);
    HAL_GPIO_DeInit(mosi_soc_base, mosi_soc_pin);
    HAL_GPIO_DeInit(sclk_soc_base, sclk_soc_pin);

    if (Spi_Master_Use_CS[index] == true)
    {
        HAL_GPIO_DeInit(cs_soc_base, cs_soc_pin);
    }

    NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);
    NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);

    gpio_pin_init_status[miso_pin] = false;
    gpio_pin_init_status[mosi_pin] = false;
    gpio_pin_init_status[sclk_pin] = false;

    if (Spi_Master_Use_CS[index] == true)
    {
        gpio_pin_init_status[cs_pin] = false;
        Spi_Master_Use_CS[index] = false;
    }

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Frequency(hwSPI_Index index, uint32_t clock_rate_hz)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    SPI_TypeDef *spi_soc_base = SPI_Map_Soc_Base(index);
    if (spi_soc_base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    int spi_hz = 0;
    uint8_t prescaler_rank = 0;
    uint8_t last_index =
        (sizeof(Spi_Master_Baudrate_Prescaler_Table) /
         sizeof(Spi_Master_Baudrate_Prescaler_Table[0])) - 1;

    spi_hz = SPI_Master_Get_Clock_Freq(index) / 2;

    while ((spi_hz > (int)clock_rate_hz) && (prescaler_rank < last_index))
    {
        spi_hz = spi_hz / 2;
        prescaler_rank++;
    }

    g_spi[index].Init.BaudRatePrescaler =
        Spi_Master_Baudrate_Prescaler_Table[prescaler_rank];

    if (HAL_SPI_Init(&g_spi[index]) != HAL_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Change_Mode(hwSPI_Index index, hwSPI_OpMode opMode)
{
    if (opMode >= hwSPI_OpMode_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    SPI_TypeDef *spi_soc_base = SPI_Map_Soc_Base(index);
    if (spi_soc_base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    switch (opMode)
    {
        case hwSPI_OpMode_Polarity0_Phase0:
            g_spi[index].Init.CLKPolarity = SPI_POLARITY_LOW;
            g_spi[index].Init.CLKPhase    = SPI_PHASE_1EDGE;
            break;

        case hwSPI_OpMode_Polarity0_Phase1:
            g_spi[index].Init.CLKPolarity = SPI_POLARITY_LOW;
            g_spi[index].Init.CLKPhase    = SPI_PHASE_2EDGE;
            break;

        case hwSPI_OpMode_Polarity1_Phase0:
            g_spi[index].Init.CLKPolarity = SPI_POLARITY_HIGH;
            g_spi[index].Init.CLKPhase    = SPI_PHASE_1EDGE;
            break;

        case hwSPI_OpMode_Polarity1_Phase1:
            g_spi[index].Init.CLKPolarity = SPI_POLARITY_HIGH;
            g_spi[index].Init.CLKPhase    = SPI_PHASE_2EDGE;
            break;

        default:
            SPI_MASTER_MUTEX_UNLOCK(index);
            return hwSPI_InvalidParameter;
    }

    if (HAL_SPI_Init(&g_spi[index]) != HAL_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_WriteByte(hwSPI_Index index, uint8_t dat)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);

    if (HAL_SPI_Transmit_IT(&g_spi[index], &dat, 1) != HAL_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_SYNC_TIMEOUT) != NeonRTOS_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_ReadByte(hwSPI_Index index, uint8_t* dat)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (dat == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    if (HAL_SPI_Receive_IT(&g_spi[index], dat, 1) != HAL_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_SYNC_TIMEOUT) != NeonRTOS_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_TransferByte(hwSPI_Index index, uint8_t wr_dat, uint8_t* rd_dat)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (rd_dat == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    if (HAL_SPI_TransmitReceive_IT(&g_spi[index], &wr_dat, rd_dat, 1) != HAL_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_HwError;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Send_SyncHandle[index],
                             SPI_MASTER_SYNC_TIMEOUT) != NeonRTOS_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    if (NeonRTOS_SyncObjWait(&Spi_Master_Recv_SyncHandle[index],
                             SPI_MASTER_SYNC_TIMEOUT) != NeonRTOS_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_DummyByte(hwSPI_Index index)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    return SPI_Master_WriteByte(index, 0x00);
}

#endif // DEVICE_STM32