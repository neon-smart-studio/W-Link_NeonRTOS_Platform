#ifndef CAN_TITIVAC_H
#define CAN_TITIVAC_H

#include <stdint.h>
#include <stdbool.h>

#include "soc.h"
#include "CAN/CAN.h"

#include "CAN/Pin/TITivaC/CAN_Pin_TITivaC_Def.h"

uint32_t CAN_Map_Soc_Base(hwCAN_Index index);
uint32_t CAN_Map_Soc_Periph(hwCAN_Index index);
uint32_t CAN_Map_Soc_Int(hwCAN_Index index);
uint32_t CAN_Map_PinConfig(hwCAN_Index can, hwGPIO_Pin pin);

void CAN_IRQ_Process(hwCAN_Index index);

void CAN_NVIC_Init(hwCAN_Index index);
void CAN_NVIC_DeInit(hwCAN_Index index);

#endif //CAN_TITIVAC_H