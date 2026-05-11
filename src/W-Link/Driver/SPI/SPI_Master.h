
#ifndef SPI_MASTER_H
#define SPI_MASTER_H

#include <stdint.h>

#include "soc.h"  

#include "SPI_Def.h"

#include "SPI_Index.h"

#ifdef	__cplusplus
extern "C" {
#endif

hwSPI_OpResult SPI_Master_Init(hwSPI_Index index, uint32_t clock_rate_hz, hwSPI_OpMode opMode, bool cs);
hwSPI_OpResult SPI_Master_DeInit(hwSPI_Index index);
hwSPI_OpResult SPI_Change_Frequency(hwSPI_Index index, uint32_t clock_rate_hz);
hwSPI_OpResult SPI_Change_Mode(hwSPI_Index index, hwSPI_OpMode opMode);
hwSPI_OpResult SPI_Master_DummyByte(hwSPI_Index index);
hwSPI_OpResult SPI_Master_WriteByte(hwSPI_Index index, uint8_t dat);
hwSPI_OpResult SPI_Master_ReadByte(hwSPI_Index index, uint8_t* dat);
hwSPI_OpResult SPI_Master_TransferByte(hwSPI_Index index, uint8_t wr_dat, uint8_t* rd_dat);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //SPI_MASTER_H