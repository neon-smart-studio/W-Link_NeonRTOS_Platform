
#ifndef SPI_MASTER_H
#define SPI_MASTER_H

#include <stdint.h>

#include "soc.h"  

#include "SPI_Def.h"

#include "SPI_Index.h"

#include "Driver_Config.h"

#ifdef	__cplusplus
extern "C" {
#endif

hwSPI_OpResult SPI_Master_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode, bool cs);
hwSPI_OpResult SPI_Master_DeInit(hwSPI_Index index);
hwSPI_OpResult SPI_Change_Frequency(hwSPI_Index index, uint32_t clock_rate_hz);
hwSPI_OpResult SPI_Change_Mode(hwSPI_Index index, hwSPI_OpMode opMode);
hwSPI_OpResult SPI_Master_DummyByte(hwSPI_Index index);
hwSPI_OpResult SPI_Master_DummyBytes(hwSPI_Index index, uint32_t len);
hwSPI_OpResult SPI_Master_WriteByte(hwSPI_Index index, uint8_t dat);
hwSPI_OpResult SPI_Master_ReadByte(hwSPI_Index index, uint8_t* dat);
hwSPI_OpResult SPI_Master_TransferByte(hwSPI_Index index, uint8_t wr_dat, uint8_t* rd_dat);

hwSPI_OpResult SPI_Master_Stream_Write(hwSPI_Index index, const uint8_t* buf, uint16_t len);
hwSPI_OpResult SPI_Master_Stream_Read(hwSPI_Index index, uint8_t* buf, uint16_t len);
hwSPI_OpResult SPI_Master_Stream_Transfer(hwSPI_Index index, const uint8_t* tx_buf, uint8_t* rx_buf, uint16_t len);

hwSPI_OpResult SPI_Master_Burst_Write(hwSPI_Index index, uint8_t* buf, uint32_t size);
hwSPI_OpResult SPI_Master_Burst_Read(hwSPI_Index index, uint8_t* buf, uint32_t size);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //SPI_MASTER_H