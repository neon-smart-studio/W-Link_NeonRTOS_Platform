#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "NeonRTOS.h"
#include "SPI/SPI_Master.h"

#ifdef DEVICE_NUVOTON

#include "GPIO/Device/Nuvoton/GPIO_Nuvoton.h"

#include "SPI/Pin/Nuvoton/SPI_Pin_Nuvoton.h"

#include "SPI_Master_Nuvoton.h"

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

void SPI_RxCpltCallback(hwSPI_Index index)
{
    if (index >= hwSPI_Index_MAX) return;

    NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[index]);
}

void SPI_TxCpltCallback(hwSPI_Index index)
{
    if (index >= hwSPI_Index_MAX) return;

    NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[index]);
}

void SPI_TxRxCpltCallback(hwSPI_Index index)
{
    if (index >= hwSPI_Index_MAX) return;

    NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjSignalFromISR(&Spi_Master_Recv_SyncHandle[index]);
}

hwSPI_OpResult SPI_Master_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode, bool cs)
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

    SPI_Pin_Def def =
        SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]];

    SPI_GPIO_ConfigAF(index, cs);

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

    hwSPI_OpResult result;

    result = SPI_Instance_Init(index, clock_rate_hz, opMode, cs);
    if (result != hwSPI_OK)
    {
        NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
        NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);
        return result;
    }

    SPI_NVIC_Init(index);

    gpio_pin_init_status[def.miso_pin] = true;
    gpio_pin_init_status[def.mosi_pin] = true;
    gpio_pin_init_status[def.sclk_pin] = true;

    if (def.cs_pin != hwGPIO_Pin_NC && cs)
    {
        gpio_pin_init_status[def.cs_pin] = true;
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

    SPI_NVIC_DeInit(index);

    SPI_Instance_DeInit(index, Spi_Master_Use_CS[index]);

    SPI_GPIO_DeConfigAF(index, Spi_Master_Use_CS[index]);

    NeonRTOS_LockObjDelete(&Spi_Master_Access_Mutex[index]);
    NeonRTOS_SyncObjDelete(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjDelete(&Spi_Master_Recv_SyncHandle[index]);

    SPI_Pin_Def def = SPI_Pin_Def_Table[index][SPI_Index_Map_Alt[index]];

    gpio_pin_init_status[def.miso_pin] = false;
    gpio_pin_init_status[def.mosi_pin] = false;
    gpio_pin_init_status[def.sclk_pin] = false;

    if (Spi_Master_Use_CS[index] == true)
    {
        gpio_pin_init_status[def.cs_pin] = false;
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

    SPI_T *spi_soc_base = SPI_Map_Soc_Base(index);
    if (spi_soc_base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    hwSPI_OpResult op_status = SPI_Instance_ChangeFrequency(index, clock_rate_hz);
    if (op_status < hwSPI_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return op_status;
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

    SPI_T *spi_soc_base = SPI_Map_Soc_Base(index);
    if (spi_soc_base == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    hwSPI_OpResult op_status = SPI_Instance_ChangeMode(index, opMode);
    if (op_status < hwSPI_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return op_status;
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

    hwSPI_OpResult op_status = SPI_Instance_Transmit_IT(index, &dat, 1);
    if (op_status < hwSPI_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return op_status;
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

    hwSPI_OpResult op_status = SPI_Instance_Receive_IT(index, dat, 1);
    if (op_status < hwSPI_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return op_status;
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

    hwSPI_OpResult op_status = SPI_Instance_TransmitReceive_IT(index, &wr_dat, rd_dat, 1);
    if (op_status < hwSPI_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return op_status;
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

hwSPI_OpResult SPI_Master_DummyBytes(hwSPI_Index index, uint32_t len)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        hwSPI_OpResult ret;

        ret = SPI_Master_DummyByte(index);

        if (ret != hwSPI_OK)
        {
            return ret;
        }
    }

    return hwSPI_OK;
}

hwSPI_OpResult SPI_Master_Stream_Write(hwSPI_Index index, const uint8_t* buf, uint16_t len)
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

    hwSPI_OpResult op_status = SPI_Instance_Transmit_IT(index, buf, len);
    if (op_status < hwSPI_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return op_status;
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

hwSPI_OpResult SPI_Master_Stream_Read(hwSPI_Index index, uint8_t* buf, uint16_t len)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (buf == NULL)
    {
        return hwSPI_InvalidParameter;
    }

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    hwSPI_OpResult op_status = SPI_Instance_Receive_IT(index, buf, len);
    if (op_status < hwSPI_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return op_status;
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

hwSPI_OpResult SPI_Master_Stream_Transfer(hwSPI_Index index, const uint8_t* tx_buf, uint8_t* rx_buf, uint16_t len)
{
    if (index >= hwSPI_Index_MAX)
    {
        return hwSPI_InvalidParameter;
    }

    if (tx_buf == NULL || rx_buf == NULL || len == 0)
        return hwSPI_InvalidParameter;

    if (Spi_Master_Init_Status[index] == false)
    {
        return hwSPI_NotInit;
    }

    SPI_MASTER_MUTEX_LOCK(index, SPI_MASTER_MUTEX_ACCESS_TIMEOUT);

    NeonRTOS_SyncObjClear(&Spi_Master_Send_SyncHandle[index]);
    NeonRTOS_SyncObjClear(&Spi_Master_Recv_SyncHandle[index]);

    hwSPI_OpResult op_status = SPI_Instance_TransmitReceive_IT(index, tx_buf, rx_buf, len);
    if (op_status < hwSPI_OK)
    {
        SPI_MASTER_MUTEX_UNLOCK(index);
        return op_status;
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

#endif // DEVICE_NUVOTON