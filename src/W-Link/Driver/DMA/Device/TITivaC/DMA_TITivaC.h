
#ifndef DMA_TITIVAC_H
#define DMA_TITIVAC_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "DMA/DMA.h"

#include "DMA_TITivaC_Index.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern bool DMA_Channel_Init_Status[hwDMA_Channel_Index_MAX];
extern NeonRTOS_LockObj_t DMA_Channel_Mutex[hwDMA_Channel_Index_MAX];

void DMA_Clock_Enable();
void DMA_Clock_Disable();

hwDMA_OpResult DMA_HW_Init(void);
hwDMA_OpResult DMA_HW_DeInit(void);

hwDMA_OpResult DMA_Xfer_UART(hwUART_Index index, hwDMA_Peripheral_Direction dir, uint8_t *buf, size_t len);
#if defined(TM4C1294)
hwDMA_OpResult DMA_Xfer_I2C(hwI2C_Index index, hwDMA_Peripheral_Direction dir, uint16_t dev_addr, uint8_t *buf, size_t len);
#endif
hwDMA_OpResult DMA_Xfer_SPI(hwSPI_Index index, hwDMA_Peripheral_Direction dir, uint8_t* buf, size_t len);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif //DMA_TITIVAC_H