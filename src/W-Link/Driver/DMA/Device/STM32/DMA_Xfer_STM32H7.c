
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "soc.h"

#include "DMA/DMA.h"

#include "CAN/CAN.h"
#include "I2C/I2C_Master.h"
#include "SPI/SPI_Master.h"
#include "UART/UART.h"

#include "NeonRTOS.h"

#ifdef STM32H7

//#include "CAN/Device/STM32/CAN_STM32.h"
#include "I2C/Device/STM32/I2C_Master_STM32.h"
#include "SPI/Device/STM32/SPI_Master_STM32.h"
#include "UART/Device/STM32/UART_STM32.h"

#include "DMA/Device/STM32/DMA_STM32.h"

static bool DMA_Channel_Direction[hwDMA_Stream_Index_MAX] = {false};

static void* DMA_Get_Instance(hwDMA_Stream_Index stream_index)
{
    DMA_Stream_TypeDef *dma_soc_stream_base = NULL;
    BDMA_Channel_TypeDef *bdma_soc_channel_base = NULL;

    if (stream_index >= hwDMA_Stream_Index_MAX)
    {
        return NULL;
    }

    if (stream_index < hwDMA_Stream_Index_BDMA_Start)
    {
        dma_soc_stream_base = DMA_Map_Soc_Stream_Base(stream_index);
        if (dma_soc_stream_base == NULL)
        {
            return NULL;
        }

        return dma_soc_stream_base;
    }
    else if (stream_index >= hwDMA_Stream_Index_BDMA_Start && stream_index <= hwDMA_Stream_Index_BDMA_End)
    {
        bdma_soc_channel_base = BDMA_Map_Soc_Channel_Base(stream_index);
        if (bdma_soc_channel_base == NULL)
        {
            return NULL;
        }

        return bdma_soc_channel_base;
    }
    else
    {
        return NULL;
    }
}

hwDMA_OpResult DMA_Config_UART(hwDMA_Stream_Index stream_index,
                               hwDMA_Peripheral_Direction dir,
                               hwUART_Index index)
{
    if (dir >= hwDMA_Peripheral_Direction_MAX ||
        index >= hwUART_Index_MAX ||
        stream_index >= hwDMA_Stream_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    if (DMA_Stream_Init_Status[stream_index] == false)
    {
        return hwDMA_NotInit;
    }

    g_dma[stream_index].Instance = DMA_Get_Instance(stream_index);
    if (g_dma[stream_index].Instance == NULL)
    {
        return hwDMA_InvalidParameter;
    }

    g_dma[stream_index].Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    g_dma[stream_index].Init.MemBurst            = DMA_MBURST_SINGLE;
    g_dma[stream_index].Init.PeriphBurst         = DMA_PBURST_SINGLE;
    g_dma[stream_index].Init.PeriphInc           = DMA_PINC_DISABLE;
    g_dma[stream_index].Init.MemInc              = DMA_MINC_ENABLE;
    g_dma[stream_index].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    g_dma[stream_index].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    g_dma[stream_index].Init.Mode                = DMA_NORMAL;
    g_dma[stream_index].Init.Priority            = DMA_PRIORITY_HIGH;

    switch (dir)
    {
        case hwDMA_Peripheral_Direction_TX:
            g_dma[stream_index].Init.Direction = DMA_MEMORY_TO_PERIPH;

            switch (index)
            {
                case hwUART_Index_0:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_USART1_TX;
                    break;
                case hwUART_Index_1:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_USART2_TX;
                    break;
                case hwUART_Index_2:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_USART3_TX;
                    break;
                case hwUART_Index_3:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_UART4_TX;
                    break;
                case hwUART_Index_4:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_UART5_TX;
                    break;
                case hwUART_Index_5:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_USART6_TX;
                    break;
                case hwUART_Index_6:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_UART7_TX;
                    break;
                case hwUART_Index_7:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_UART8_TX;
                    break;
                default:
                    return hwDMA_InvalidParameter;
            }
            break;

        case hwDMA_Peripheral_Direction_RX:
            g_dma[stream_index].Init.Direction = DMA_PERIPH_TO_MEMORY;

            switch (index)
            {
                case hwUART_Index_0:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_USART1_RX;
                    break;
                case hwUART_Index_1:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_USART2_RX;
                    break;
                case hwUART_Index_2:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_USART3_RX;
                    break;
                case hwUART_Index_3:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_UART4_RX;
                    break;
                case hwUART_Index_4:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_UART5_RX;
                    break;
                case hwUART_Index_5:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_USART6_RX;
                    break;
                case hwUART_Index_6:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_UART7_RX;
                    break;
                case hwUART_Index_7:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_UART8_RX;
                    break;
                default:
                    return hwDMA_InvalidParameter;
            }
            break;

        default:
            return hwDMA_InvalidParameter;
    }

    if (HAL_DMA_Init(&g_dma[stream_index]) != HAL_OK)
    {
        return hwDMA_HwError;
    }

    switch (dir)
    {
        case hwDMA_Peripheral_Direction_TX:
            __HAL_LINKDMA(&g_uart[index], hdmatx, g_dma[stream_index]);
            break;

        case hwDMA_Peripheral_Direction_RX:
            __HAL_LINKDMA(&g_uart[index], hdmarx, g_dma[stream_index]);
            break;

        default:
            return hwDMA_InvalidParameter;
    }

    DMA_Channel_Direction[stream_index] = dir;

    return hwDMA_OK;
}

hwDMA_OpResult DMA_Config_I2C(hwDMA_Stream_Index stream_index, hwDMA_Peripheral_Direction dir, hwI2C_Index index)
{
    if (dir >= hwDMA_Peripheral_Direction_MAX ||
        index >= hwI2C_Index_MAX ||
        stream_index >= hwDMA_Stream_Index_MAX)
    {
        return hwDMA_InvalidParameter;
    }

    if (DMA_Stream_Init_Status[stream_index] == false)
    {
        return hwDMA_NotInit;
    }

    g_dma[stream_index].Instance = DMA_Get_Instance(stream_index);
    if(g_dma[stream_index].Instance == NULL)
    {
        return hwDMA_InvalidParameter;
    }

    g_dma[stream_index].Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    g_dma[stream_index].Init.MemBurst            = DMA_MBURST_SINGLE;
    g_dma[stream_index].Init.PeriphBurst         = DMA_PBURST_SINGLE;
    g_dma[stream_index].Init.PeriphInc           = DMA_PINC_DISABLE;
    g_dma[stream_index].Init.MemInc              = DMA_MINC_ENABLE;
    g_dma[stream_index].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    g_dma[stream_index].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    g_dma[stream_index].Init.Mode                = DMA_NORMAL;
    g_dma[stream_index].Init.Priority            = DMA_PRIORITY_HIGH;

    switch (dir)
    {
        case hwDMA_Peripheral_Direction_TX:
            g_dma[stream_index].Init.Direction = DMA_MEMORY_TO_PERIPH;

            switch (index)
            {
                case hwI2C_Index_0:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_I2C1_TX;
                    break;

                case hwI2C_Index_1:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_I2C2_TX;
                    break;

                case hwI2C_Index_2:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_I2C3_TX;
                    break;

                case hwI2C_Index_3:
                    g_dma[stream_index].Init.Request = BDMA_REQUEST_I2C4_TX;
                    break;

                default:
                    return hwDMA_InvalidParameter;
            }
            break;

        case hwDMA_Peripheral_Direction_RX:
            g_dma[stream_index].Init.Direction = DMA_PERIPH_TO_MEMORY;

            switch (index)
            {
                case hwI2C_Index_0:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_I2C1_RX;
                    break;

                case hwI2C_Index_1:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_I2C2_RX;
                    break;

                case hwI2C_Index_2:
                    g_dma[stream_index].Init.Request = DMA_REQUEST_I2C3_RX;
                    break;

                case hwI2C_Index_3:
                    g_dma[stream_index].Init.Request = BDMA_REQUEST_I2C4_RX;
                    break;

                default:
                    return hwDMA_InvalidParameter;
            }
            break;

        default:
            return hwDMA_InvalidParameter;
    }

    if (HAL_DMA_Init(&g_dma[stream_index]) != HAL_OK)
    {
        return hwDMA_HwError;
    }

    switch (dir)
    {
        case hwDMA_Peripheral_Direction_TX:
            __HAL_LINKDMA(&g_i2c[index], hdmatx, g_dma[stream_index]);
            break;

        case hwDMA_Peripheral_Direction_RX:
            __HAL_LINKDMA(&g_i2c[index], hdmarx, g_dma[stream_index]);
            break;

        default:
            return hwDMA_InvalidParameter;
    }

    DMA_Channel_Direction[stream_index] = dir;

    return hwDMA_OK;
}

hwDMA_OpResult DMA_Config_SPI(hwDMA_Stream_Index stream_index, hwDMA_Peripheral_Direction dir, hwSPI_Index index)
{
        if(dir>=hwDMA_Peripheral_Direction_MAX || index>=hwSPI_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwSPI_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Stream_Init_Status[stream_index]==false)
        {
                return hwDMA_NotInit;
        }

        g_dma[stream_index].Instance = DMA_Get_Instance(stream_index);
        if(g_dma[stream_index].Instance == NULL)
        {
                return hwDMA_InvalidParameter;
        }

        g_dma[stream_index].Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
        g_dma[stream_index].Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
        g_dma[stream_index].Init.MemBurst            = DMA_MBURST_SINGLE;
        g_dma[stream_index].Init.PeriphBurst         = DMA_PBURST_SINGLE;
        switch(dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        g_dma[stream_index].Init.Direction = DMA_MEMORY_TO_PERIPH;
                        switch(index)
                        {
                                case hwSPI_Index_0:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI1_TX;
                                        break;
                                case hwSPI_Index_1:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI2_TX;
                                        break;
                                case hwSPI_Index_2:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI3_TX;
                                        break;
                                case hwSPI_Index_3:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI4_TX;
                                        break;
                                case hwSPI_Index_4:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI5_TX;
                                        break;
                                case hwSPI_Index_5:
                                        g_dma[stream_index].Init.Request   = BDMA_REQUEST_SPI6_TX;
                                        break;
                        }
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        g_dma[stream_index].Init.Direction = DMA_PERIPH_TO_MEMORY;
                        switch(index)
                        {
                                case hwSPI_Index_0:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI1_RX;
                                        break;
                                case hwSPI_Index_1:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI2_RX;
                                        break;
                                case hwSPI_Index_2:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI3_RX;
                                        break;
                                case hwSPI_Index_3:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI4_RX;
                                        break;
                                case hwSPI_Index_4:
                                        g_dma[stream_index].Init.Request   = DMA_REQUEST_SPI5_RX;
                                        break;
                                case hwSPI_Index_5:
                                        g_dma[stream_index].Init.Request   = BDMA_REQUEST_SPI6_RX;
                                        break;
                        }
                        break;
        }
        g_dma[stream_index].Init.PeriphInc           = DMA_PINC_DISABLE;
        g_dma[stream_index].Init.MemInc              = DMA_MINC_ENABLE;
        g_dma[stream_index].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        g_dma[stream_index].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        g_dma[stream_index].Init.Mode                = DMA_NORMAL;
        g_dma[stream_index].Init.Priority            = DMA_PRIORITY_HIGH;

        HAL_DMA_Init(&g_dma[stream_index]);

        switch(dir)
        {
                case hwDMA_Peripheral_Direction_TX:
                        __HAL_LINKDMA(&g_spi[index], hdmatx, g_dma[stream_index]);
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        __HAL_LINKDMA(&g_spi[index], hdmarx, g_dma[stream_index]);
                        break;
        }

        DMA_Channel_Direction[stream_index] = dir;

        return hwDMA_OK;
}

hwDMA_OpResult DMA_Transfer_UART(hwDMA_Stream_Index stream_index, hwUART_Index index, uint8_t *buf, size_t len)
{
    if (stream_index >= hwDMA_Stream_Index_MAX ||
        index >= hwUART_Index_MAX ||
        buf == NULL ||
        len == 0)
    {
        return hwDMA_InvalidParameter;
    }

    if (DMA_Stream_Init_Status[stream_index] == false)
    {
        return hwDMA_NotInit;
    }

    switch (DMA_Channel_Direction[stream_index])
    {
        case hwDMA_Peripheral_Direction_TX:
            SCB_CleanDCache_by_Addr((uint32_t *)buf, len);

            if (HAL_UART_Transmit_DMA(&g_uart[index], buf, len) != HAL_OK)
            {
                return hwDMA_HwError;
            }
            break;

        case hwDMA_Peripheral_Direction_RX:
            SCB_InvalidateDCache_by_Addr((uint32_t *)buf, len);

            if (HAL_UART_Receive_DMA(&g_uart[index], buf, len) != HAL_OK)
            {
                return hwDMA_HwError;
            }
            break;

        default:
            return hwDMA_InvalidParameter;
    }

    while (HAL_UART_GetState(&g_uart[index]) != HAL_UART_STATE_READY) { }

    HAL_UART_DMAStop(&g_uart[index]);

    return hwDMA_OK;
}

hwDMA_OpResult DMA_Transfer_I2C(hwDMA_Stream_Index stream_index, hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len)
{
    if (stream_index >= hwDMA_Stream_Index_MAX ||
        index >= hwI2C_Index_MAX ||
        buf == NULL ||
        len == 0)
    {
        return hwDMA_InvalidParameter;
    }

    if (DMA_Stream_Init_Status[stream_index] == false)
    {
        return hwDMA_NotInit;
    }

    switch (DMA_Channel_Direction[stream_index])
    {
        case hwDMA_Peripheral_Direction_TX:
            SCB_CleanDCache_by_Addr((uint32_t *)buf, len);

            if (HAL_I2C_Master_Transmit_DMA(&g_i2c[index], dev_addr, buf, len) != HAL_OK)
            {
                return hwDMA_HwError;
            }
            break;

        case hwDMA_Peripheral_Direction_RX:
            SCB_InvalidateDCache_by_Addr((uint32_t *)buf, len);

            if (HAL_I2C_Master_Receive_DMA(&g_i2c[index], dev_addr, buf, len) != HAL_OK)
            {
                return hwDMA_HwError;
            }
            break;

        default:
            return hwDMA_InvalidParameter;
    }

    while (HAL_I2C_GetState(&g_i2c[index]) != HAL_I2C_STATE_READY) { }

    HAL_I2C_DMAStop(&g_i2c[index]);

    return hwDMA_OK;
}

hwDMA_OpResult DMA_Transfer_SPI(hwDMA_Stream_Index stream_index, hwSPI_Index index, uint8_t* buf, size_t len)
{
        if(DMA_Channel_Direction[stream_index]>=hwDMA_Peripheral_Direction_MAX || index>=hwSPI_Index_MAX || buf==NULL)
        {
                return hwDMA_InvalidParameter;
        }

        if(stream_index>=hwDMA_Stream_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(index>=hwSPI_Index_MAX)
        {
                return hwDMA_InvalidParameter;
        }

        if(DMA_Stream_Init_Status[stream_index]==false)
        {
                return hwDMA_NotInit;
        }

        hwDMA_OpResult op_status = hwDMA_OK;
        
        switch(DMA_Channel_Direction[stream_index])
        {
                case hwDMA_Peripheral_Direction_TX:
                        SCB_CleanDCache_by_Addr((uint32_t *)buf, len);
                        HAL_SPI_Transmit_DMA(&g_spi[index], buf, len);
                        break;
                case hwDMA_Peripheral_Direction_RX:
                        SCB_InvalidateDCache_by_Addr((uint32_t *)buf, len);
                        HAL_SPI_Receive_DMA(&g_spi[index], buf, len);
                        break;
        }

        while (HAL_SPI_GetState(&g_spi[index]) != HAL_SPI_STATE_READY) { }
        
        HAL_SPI_DMAStop(&g_spi[index]);

        return op_status;
}

#endif //STM32H7
