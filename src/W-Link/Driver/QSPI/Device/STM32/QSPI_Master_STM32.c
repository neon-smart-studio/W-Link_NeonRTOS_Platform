#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "NeonRTOS.h"

#include "QSPI/QSPI_Master.h"

#ifdef DEVICE_STM32

#if defined(QUADSPI)

#include "GPIO/Device/STM32/GPIO_STM32.h"
#include "QSPI/Pin/STM32/QSPI_Pin_STM32.h"
#include "QSPI_Master_STM32.h"

#define SPI_MASTER_SYNC_TIMEOUT             100
#define SPI_MASTER_MUTEX_ACCESS_TIMEOUT     500
#define SPI_MASTER_OP_TIMEOUT               3000

NeonRTOS_LockObj_t Qspi_Master_Access_Mutex[hwQSPI_Index_MAX];
NeonRTOS_SyncObj_t Qspi_Master_Send_SyncHandle[hwQSPI_Index_MAX];
NeonRTOS_SyncObj_t Qspi_Master_Recv_SyncHandle[hwQSPI_Index_MAX];

#define SPI_MASTER_MUTEX_LOCK(a, b) \
    if (NeonRTOS_LockObjLock(&Qspi_Master_Access_Mutex[a], b) != NeonRTOS_OK) { return hwQSPI_MutexTimeout; }

#define SPI_MASTER_MUTEX_UNLOCK(a) \
    NeonRTOS_LockObjUnlock(&Qspi_Master_Access_Mutex[a])

bool Qspi_Master_Init_Status[hwQSPI_Index_MAX] = {false};

static bool Qspi_Master_Use_CS[hwQSPI_Index_MAX] = {false};

static QSPI_CommandTypeDef qspi_cmd[hwQSPI_Index_MAX];

#ifndef STM32F1
uint32_t STM32_QSPI_GetAF(hwQSPI_Index qspi, hwGPIO_Pin pin)
{
    for (size_t i = 0; i < sizeof(QSPI_Pin_AF_Map) / sizeof(QSPI_Pin_AF_Map[0]); i++) {
        if (QSPI_Pin_AF_Map[i].qspi == qspi &&
            QSPI_Pin_AF_Map[i].pin == pin) {
            return QSPI_Pin_AF_Map[i].af;
        }
    }

    return 0;
}
#endif

hwQSPI_Index QSPI_IndexFromHandle(QSPI_HandleTypeDef *hqspi)
{
    for(int i=0;i<hwQSPI_Index_MAX;i++)
    {
        if(&g_qspi[i] == hqspi)
            return (hwQSPI_Index)i;
    }
    return hwQSPI_Index_MAX;
}

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    hwQSPI_Index idx = QSPI_IndexFromHandle(hqspi);
    if (idx >= hwQSPI_Index_MAX) return;

    NeonRTOS_SyncObjSignalFromISR(&Qspi_Master_Recv_SyncHandle[idx]);
}

void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    hwQSPI_Index idx = QSPI_IndexFromHandle(hqspi);
    if (idx >= hwQSPI_Index_MAX) return;

    NeonRTOS_SyncObjSignalFromISR(&Qspi_Master_Send_SyncHandle[idx]);
}

void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    hwQSPI_Index idx = QSPI_IndexFromHandle(hqspi);
    if (idx >= hwQSPI_Index_MAX) return;

    NeonRTOS_SyncObjSignalFromISR(&Qspi_Master_Send_SyncHandle[idx]);
}

hwQSPI_OpResult QSPI_Master_Init(hwQSPI_Index index, uint32_t clock_rate_hz, hwQSPI_OpMode opMode, bool cs)
{
    if (index >= hwQSPI_Index_MAX) {
        return hwQSPI_InvalidParameter;
    }

    if (opMode >= hwQSPI_OpMode_MAX) {
        return hwQSPI_InvalidParameter;
    }

    if (Qspi_Master_Init_Status[index]) {
        return hwQSPI_OK;
    }

    hwGPIO_Pin io0_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].io0_pin;
    hwGPIO_Pin io1_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].io1_pin;
    hwGPIO_Pin io2_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].io2_pin;
    hwGPIO_Pin io3_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].io3_pin;
    hwGPIO_Pin sclk_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].sclk_pin;
    hwGPIO_Pin cs_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].cs_pin;

    GPIO_TypeDef *io0_soc_base = GPIO_Map_Soc_Base(io0_pin);
    GPIO_TypeDef *io1_soc_base = GPIO_Map_Soc_Base(io1_pin);
    GPIO_TypeDef *io2_soc_base = GPIO_Map_Soc_Base(io2_pin);
    GPIO_TypeDef *io3_soc_base = GPIO_Map_Soc_Base(io3_pin);
    GPIO_TypeDef *sclk_soc_base = GPIO_Map_Soc_Base(sclk_pin);

    uint16_t io0_soc_pin = GPIO_Map_Soc_Pin(io0_pin);
    uint16_t io1_soc_pin = GPIO_Map_Soc_Pin(io1_pin);
    uint16_t io2_soc_pin = GPIO_Map_Soc_Pin(io2_pin);
    uint16_t io3_soc_pin = GPIO_Map_Soc_Pin(io3_pin);
    uint16_t sclk_soc_pin = GPIO_Map_Soc_Pin(sclk_pin);

    GPIO_TypeDef *cs_soc_base = NULL;
    uint16_t cs_soc_pin = 0;

    if (io0_soc_pin == 0 || io0_soc_base == NULL ||
        io1_soc_pin == 0 || io1_soc_base == NULL ||
        io2_soc_pin == 0 || io2_soc_base == NULL ||
        io3_soc_pin == 0 || io3_soc_base == NULL ||
        sclk_soc_pin == 0 || sclk_soc_base == NULL)
    {
        return hwQSPI_InvalidParameter;
    }

    if (cs_pin != hwGPIO_Pin_NC && cs)
    {
        cs_soc_base = GPIO_Map_Soc_Base(cs_pin);
        cs_soc_pin  = GPIO_Map_Soc_Pin(cs_pin);

        if (cs_soc_pin == 0 || cs_soc_base == NULL)
        {
            return hwQSPI_InvalidParameter;
        }
    }

    uint32_t io0_af = STM32_QSPI_GetAF(index, io0_pin);
    uint32_t io1_af = STM32_QSPI_GetAF(index, io1_pin);
    uint32_t io2_af = STM32_QSPI_GetAF(index, io2_pin);
    uint32_t io3_af = STM32_QSPI_GetAF(index, io3_pin);
    uint32_t sclk_af = STM32_QSPI_GetAF(index, sclk_pin);
    uint32_t cs_af   = 0;

    if (io1_af == 0 || io2_af == 0 || sclk_af == 0)
    {
        return hwQSPI_InvalidParameter;
    }

    if (cs_pin != hwGPIO_Pin_NC && cs)
    {
        cs_af = STM32_QSPI_GetAF(index, cs_pin);
        if (cs_af == 0)
        {
            return hwQSPI_InvalidParameter;
        }
    }

    if (NeonRTOS_SyncObjCreate(&Qspi_Master_Send_SyncHandle[index]) != NeonRTOS_OK) {
        return hwQSPI_MemoryError;
    }

    if (NeonRTOS_SyncObjCreate(&Qspi_Master_Recv_SyncHandle[index]) != NeonRTOS_OK) {
        NeonRTOS_SyncObjDelete(&Qspi_Master_Send_SyncHandle[index]);
        return hwQSPI_MemoryError;
    }

    if (NeonRTOS_LockObjCreate(&Qspi_Master_Access_Mutex[index]) != NeonRTOS_OK) {
        NeonRTOS_SyncObjDelete(&Qspi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Qspi_Master_Recv_SyncHandle[index]);
        return hwQSPI_MemoryError;
    }

    NeonRTOS_LockObjUnlock(&Qspi_Master_Access_Mutex[index]);

    GPIO_Enable_RCC_Clock(io0_soc_base);
    GPIO_Enable_RCC_Clock(io1_soc_base);
    GPIO_Enable_RCC_Clock(io2_soc_base);
    GPIO_Enable_RCC_Clock(io3_soc_base);
    GPIO_Enable_RCC_Clock(sclk_soc_base);

    if (cs_pin != hwGPIO_Pin_NC && cs)
    {
        GPIO_Enable_RCC_Clock(cs_soc_base);
    }

    GPIO_InitTypeDef g_spi_io0 = {0};
    g_spi_io0.Pin       = io0_soc_pin;
    g_spi_io0.Mode      = GPIO_MODE_AF_PP;
    g_spi_io0.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_spi_io0.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_spi_io0.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_spi_io0.Alternate = io0_af;
#endif
    HAL_GPIO_Init(io0_soc_base, &g_spi_io0);

    GPIO_InitTypeDef g_spi_io1 = {0};
    g_spi_io1.Pin       = io1_soc_pin;
    g_spi_io1.Mode      = GPIO_MODE_AF_PP;
    g_spi_io1.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_spi_io1.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_spi_io1.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_spi_io1.Alternate = io1_af;
#endif
    HAL_GPIO_Init(io1_soc_base, &g_spi_io1);

    GPIO_InitTypeDef g_spi_io2 = {0};
    g_spi_io2.Pin       = io2_soc_pin;
    g_spi_io2.Mode      = GPIO_MODE_AF_PP;
    g_spi_io2.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_spi_io2.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_spi_io2.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_spi_io2.Alternate = io2_af;
#endif
    HAL_GPIO_Init(io2_soc_base, &g_spi_io2);

    GPIO_InitTypeDef g_spi_io3 = {0};
    g_spi_io3.Pin       = io3_soc_pin;
    g_spi_io3.Mode      = GPIO_MODE_AF_PP;
    g_spi_io3.Pull      = GPIO_NOPULL;
#ifdef GPIO_SPEED_FREQ_VERY_HIGH
    g_spi_io3.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
#else
    g_spi_io3.Speed     = GPIO_SPEED_FREQ_HIGH;
#endif
#ifndef STM32F1
    g_spi_io3.Alternate = io3_af;
#endif
    HAL_GPIO_Init(io3_soc_base, &g_spi_io3);

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
    
    if (cs_pin != hwGPIO_Pin_NC && cs)
    {
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

    hwQSPI_OpResult result = QSPI_Instance_Init(index, clock_rate_hz, opMode);
    if (result != hwQSPI_OK) {
        NeonRTOS_LockObjDelete(&Qspi_Master_Access_Mutex[index]);
        NeonRTOS_SyncObjDelete(&Qspi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Qspi_Master_Recv_SyncHandle[index]);
        return result;
    }

    QSPI_NVIC_Init(index);

    gpio_pin_init_status[io0_pin] = true;
    gpio_pin_init_status[io1_pin] = true;
    gpio_pin_init_status[io2_pin] = true;
    gpio_pin_init_status[io3_pin] = true;
    gpio_pin_init_status[sclk_pin] = true;

    if (cs_pin != hwGPIO_Pin_NC && cs)
    {
        gpio_pin_init_status[cs_pin] = true;
        Qspi_Master_Use_CS[index] = true;
    }

    Qspi_Master_Init_Status[index] = true;

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_DeInit(hwQSPI_Index index)
{
    if (index >= hwQSPI_Index_MAX) {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index]) {
        return hwQSPI_OK;
    }

    hwGPIO_Pin io0_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].io0_pin;
    hwGPIO_Pin io1_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].io1_pin;
    hwGPIO_Pin io2_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].io2_pin;
    hwGPIO_Pin io3_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].io3_pin;
    hwGPIO_Pin sclk_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].sclk_pin;
    hwGPIO_Pin cs_pin = QSPI_Pin_Def_Table[index][QSPI_Index_Map_Alt[index]].cs_pin;

    GPIO_TypeDef *io0_soc_base = GPIO_Map_Soc_Base(io0_pin);
    GPIO_TypeDef *io1_soc_base = GPIO_Map_Soc_Base(io1_pin);
    GPIO_TypeDef *io2_soc_base = GPIO_Map_Soc_Base(io2_pin);
    GPIO_TypeDef *io3_soc_base = GPIO_Map_Soc_Base(io3_pin);
    GPIO_TypeDef *sclk_soc_base = GPIO_Map_Soc_Base(sclk_pin);

    uint16_t io0_soc_pin = GPIO_Map_Soc_Pin(io0_pin);
    uint16_t io1_soc_pin = GPIO_Map_Soc_Pin(io1_pin);
    uint16_t io2_soc_pin = GPIO_Map_Soc_Pin(io2_pin);
    uint16_t io3_soc_pin = GPIO_Map_Soc_Pin(io3_pin);
    uint16_t sclk_soc_pin = GPIO_Map_Soc_Pin(sclk_pin);

    GPIO_TypeDef *cs_soc_base = NULL;
    uint16_t cs_soc_pin = 0;

    if (io0_soc_pin == 0 || io0_soc_base == NULL ||
        io1_soc_pin == 0 || io1_soc_base == NULL ||
        io2_soc_pin == 0 || io2_soc_base == NULL ||
        io3_soc_pin == 0 || io3_soc_base == NULL ||
        sclk_soc_pin == 0 || sclk_soc_base == NULL)
    {
        return hwQSPI_InvalidParameter;
    }

    if (Qspi_Master_Use_CS[index] == true)
    {
        cs_soc_base = GPIO_Map_Soc_Base(cs_pin);
        cs_soc_pin  = GPIO_Map_Soc_Pin(cs_pin);

        if (cs_soc_pin == 0 || cs_soc_base == NULL)
        {
            return hwQSPI_InvalidParameter;
        }
    }

    Qspi_Master_Init_Status[index] = false;

    QSPI_NVIC_DeInit(index);

    QSPI_Instance_DeInit(index);

    HAL_GPIO_DeInit(io0_soc_base, io0_soc_pin);
    HAL_GPIO_DeInit(io1_soc_base, io1_soc_pin);
    HAL_GPIO_DeInit(io2_soc_base, io2_soc_pin);
    HAL_GPIO_DeInit(io3_soc_base, io3_soc_pin);
    HAL_GPIO_DeInit(sclk_soc_base, sclk_soc_pin);

    if (Qspi_Master_Use_CS[index] == true)
    {
        HAL_GPIO_DeInit(cs_soc_base, cs_soc_pin);
    }

    NeonRTOS_LockObjDelete(&Qspi_Master_Access_Mutex[index]);
    NeonRTOS_SyncObjDelete(&Qspi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&Qspi_Master_Recv_SyncHandle[index]);

    gpio_pin_init_status[io0_pin] = false;
    gpio_pin_init_status[io1_pin] = false;
    gpio_pin_init_status[io2_pin] = false;
    gpio_pin_init_status[io3_pin] = false;
    gpio_pin_init_status[sclk_pin] = false;

    if (Qspi_Master_Use_CS[index] == true)
    {
        gpio_pin_init_status[cs_pin] = false;
        Qspi_Master_Use_CS[index] = false;
    }

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Change_Frequency(hwQSPI_Index index, uint32_t clock_rate_hz)
{
    if (index >= hwQSPI_Index_MAX || clock_rate_hz == 0) {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index]) {
        return hwQSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    uint32_t qspi_clk = QSPI_Master_Get_Clock_Freq(index);
    if (qspi_clk == 0) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    uint32_t prescaler = qspi_clk / clock_rate_hz;

    if ((qspi_clk % clock_rate_hz) != 0) {
        prescaler++;
    }

    if (prescaler == 0) {
        prescaler = 1;
    }

    prescaler -= 1;

    if (prescaler > 255) {
        prescaler = 255;
    }

    g_qspi[index].Init.ClockPrescaler = prescaler;

    if (HAL_QSPI_Init(&g_qspi[index]) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Change_Mode(hwQSPI_Index index, hwQSPI_OpMode opMode)
{
    if (index >= hwQSPI_Index_MAX) {
        return hwQSPI_InvalidParameter;
    }

    if (opMode >= hwQSPI_OpMode_MAX) {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index]) {
        return hwQSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    switch (opMode) {
        case hwQSPI_OpMode_Polarity0_Phase0:
            g_qspi[index].Init.ClockMode = QSPI_CLOCK_MODE_0;
            break;

        case hwQSPI_OpMode_Polarity1_Phase1:
            g_qspi[index].Init.ClockMode = QSPI_CLOCK_MODE_3;
            break;
    }

    if (HAL_QSPI_Init(&g_qspi[index]) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_WriteByte(hwQSPI_Index index, const uint8_t dat)
{
    if (index >= hwQSPI_Index_MAX) {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index]) {
        return hwQSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    memset(&qspi_cmd[index], 0, sizeof(QSPI_CommandTypeDef));

    qspi_cmd[index].InstructionMode   = QSPI_INSTRUCTION_NONE;
    qspi_cmd[index].AddressMode       = QSPI_ADDRESS_NONE;
    qspi_cmd[index].AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    qspi_cmd[index].DataMode          = QSPI_DATA_1_LINE;
    qspi_cmd[index].DummyCycles       = 0;
    qspi_cmd[index].NbData            = 1;
    qspi_cmd[index].DdrMode           = QSPI_DDR_MODE_DISABLE;
    qspi_cmd[index].DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    qspi_cmd[index].SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    NeonRTOS_SyncObjClear(&Qspi_Master_Send_SyncHandle[index]);

    if (HAL_QSPI_Command(&g_qspi[index], &qspi_cmd[index], SPI_MASTER_OP_TIMEOUT) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    if (HAL_QSPI_Transmit_IT(&g_qspi[index], (uint8_t *)&dat) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    if (NeonRTOS_SyncObjWait(&Qspi_Master_Send_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_ReadByte(hwQSPI_Index index, uint8_t *dat)
{
    if (index >= hwQSPI_Index_MAX || dat == NULL) {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index]) {
        return hwQSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    memset(&qspi_cmd[index], 0, sizeof(QSPI_CommandTypeDef));

    qspi_cmd[index].InstructionMode   = QSPI_INSTRUCTION_NONE;
    qspi_cmd[index].AddressMode       = QSPI_ADDRESS_NONE;
    qspi_cmd[index].AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    qspi_cmd[index].DataMode          = QSPI_DATA_1_LINE;
    qspi_cmd[index].DummyCycles       = 0;
    qspi_cmd[index].NbData            = 1;
    qspi_cmd[index].DdrMode           = QSPI_DDR_MODE_DISABLE;
    qspi_cmd[index].DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    qspi_cmd[index].SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    NeonRTOS_SyncObjClear(&Qspi_Master_Recv_SyncHandle[index]);

    if (HAL_QSPI_Command(&g_qspi[index], &qspi_cmd[index], SPI_MASTER_OP_TIMEOUT) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    if (HAL_QSPI_Receive_IT(&g_qspi[index], dat) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    if (NeonRTOS_SyncObjWait(&Qspi_Master_Recv_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_DummyByte(hwQSPI_Index index)
{
    return QSPI_Master_WriteByte(index, 0x00);
}

hwQSPI_OpResult QSPI_Master_DummyBytes(hwQSPI_Index index, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        hwQSPI_OpResult ret = QSPI_Master_DummyByte(index);
        if (ret != hwQSPI_OK) {
            return ret;
        }
    }

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_Stream_Write(hwQSPI_Index index, const uint8_t *buf, uint16_t len)
{
    if (index >= hwQSPI_Index_MAX || buf == NULL || len == 0) {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index]) {
        return hwQSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    memset(&qspi_cmd[index], 0, sizeof(QSPI_CommandTypeDef));

    qspi_cmd[index].InstructionMode   = QSPI_INSTRUCTION_NONE;
    qspi_cmd[index].AddressMode       = QSPI_ADDRESS_NONE;
    qspi_cmd[index].AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    qspi_cmd[index].DataMode          = QSPI_DATA_1_LINE;
    qspi_cmd[index].DummyCycles       = 0;
    qspi_cmd[index].NbData            = len;
    qspi_cmd[index].DdrMode           = QSPI_DDR_MODE_DISABLE;
    qspi_cmd[index].DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    qspi_cmd[index].SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    NeonRTOS_SyncObjClear(&Qspi_Master_Send_SyncHandle[index]);

    if (HAL_QSPI_Command(&g_qspi[index], &qspi_cmd[index], SPI_MASTER_OP_TIMEOUT) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    if (HAL_QSPI_Transmit_IT(&g_qspi[index], (uint8_t *)buf) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    if (NeonRTOS_SyncObjWait(&Qspi_Master_Send_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_Stream_Read(hwQSPI_Index index, uint8_t *buf, uint16_t len)
{
    if (index >= hwQSPI_Index_MAX || buf == NULL || len == 0) {
        return hwQSPI_InvalidParameter;
    }

    if (!Qspi_Master_Init_Status[index]) {
        return hwQSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    memset(&qspi_cmd[index], 0, sizeof(QSPI_CommandTypeDef));

    qspi_cmd[index].InstructionMode   = QSPI_INSTRUCTION_NONE;
    qspi_cmd[index].AddressMode       = QSPI_ADDRESS_NONE;
    qspi_cmd[index].AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    qspi_cmd[index].DataMode          = QSPI_DATA_1_LINE;
    qspi_cmd[index].DummyCycles       = 0;
    qspi_cmd[index].NbData            = len;
    qspi_cmd[index].DdrMode           = QSPI_DDR_MODE_DISABLE;
    qspi_cmd[index].DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    qspi_cmd[index].SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    NeonRTOS_SyncObjClear(&Qspi_Master_Recv_SyncHandle[index]);

    if (HAL_QSPI_Command(&g_qspi[index], &qspi_cmd[index], SPI_MASTER_OP_TIMEOUT) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    if (HAL_QSPI_Receive_IT(&g_qspi[index], buf) != HAL_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_HwError;
    }

    if (NeonRTOS_SyncObjWait(&Qspi_Master_Recv_SyncHandle[index], SPI_MASTER_OP_TIMEOUT) != NeonRTOS_OK) {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return hwQSPI_SlaveTimeout;
    }

    SPI_MASTER_MUTEX_UNLOCK(index);

    return hwQSPI_OK;
}

hwQSPI_OpResult QSPI_Master_Burst_Write(hwQSPI_Index index, uint8_t *buf, uint32_t size)
{
    if (size > 0xFFFF) {
        return hwQSPI_InvalidParameter;
    }

    return QSPI_Master_Stream_Write(index, buf, (uint16_t)size);
}

hwQSPI_OpResult QSPI_Master_Burst_Read(hwQSPI_Index index, uint8_t *buf, uint32_t size)
{
    if (size > 0xFFFF) {
        return hwQSPI_InvalidParameter;
    }

    return QSPI_Master_Stream_Read(index, buf, (uint16_t)size);
}

#endif // QUADSPI

#endif // DEVICE_STM32