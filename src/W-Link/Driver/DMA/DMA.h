
#ifndef DMA_H
#define DMA_H

#include "DMA_Def.h"

#include "CAN/CAN.h"
#include "I2C/I2C_Master.h"
#include "SPI/SPI_Master.h"
#include "UART/UART.h"

#include "Driver_Config.h"

#ifdef	__cplusplus
extern "C" {
#endif

hwDMA_OpResult DMA_Init();
hwDMA_OpResult DMA_DeInit();

hwDMA_OpResult DMA_Uart_Tx(hwUART_Index index, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_Uart_Rx(hwUART_Index index, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_I2C_Write(hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_I2C_Read(hwI2C_Index index, uint16_t dev_addr, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_SPI_Write(hwSPI_Index index, uint8_t *buf, size_t len);
hwDMA_OpResult DMA_SPI_Read(hwSPI_Index index, uint8_t *buf, size_t len);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //DMA_H