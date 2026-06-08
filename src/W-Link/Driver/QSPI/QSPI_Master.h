
#ifndef QSPI_MASTER_H
#define QSPI_MASTER_H

#include <stdint.h>

#include "soc.h"  

#include "QSPI_Def.h"

#include "QSPI_Index.h"

#include "Driver_Config.h"

#ifdef	__cplusplus
extern "C" {
#endif

hwQSPI_OpResult QSPI_Master_Init(hwQSPI_Index index, uint32_t clock_rate_hz, hwQSPI_OpMode opMode, bool cs);
hwQSPI_OpResult QSPI_Master_DeInit(hwQSPI_Index index);
hwQSPI_OpResult QSPI_Change_Frequency(hwQSPI_Index index, uint32_t clock_rate_hz);
hwQSPI_OpResult QSPI_Change_Mode(hwQSPI_Index index, hwQSPI_OpMode opMode);
hwQSPI_OpResult QSPI_Master_DummyByte(hwQSPI_Index index);
hwQSPI_OpResult QSPI_Master_DummyBytes(hwQSPI_Index index, uint32_t len);
hwQSPI_OpResult QSPI_Master_WriteByte(hwQSPI_Index index, uint8_t dat);
hwQSPI_OpResult QSPI_Master_ReadByte(hwQSPI_Index index, uint8_t* dat);

hwQSPI_OpResult QSPI_Master_Stream_Write(hwQSPI_Index index, const uint8_t* buf, uint16_t len);
hwQSPI_OpResult QSPI_Master_Stream_Read(hwQSPI_Index index, uint8_t* buf, uint16_t len);

hwQSPI_OpResult QSPI_Master_Burst_Write(hwQSPI_Index index, uint8_t* buf, uint32_t size);
hwQSPI_OpResult QSPI_Master_Burst_Read(hwQSPI_Index index, uint8_t* buf, uint32_t size);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //QSPI_MASTER_H